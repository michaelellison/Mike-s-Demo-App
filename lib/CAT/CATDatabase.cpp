/// \file CATDatabase.cpp
/// \brief SQL database wrapper
/// \ingroup CAT
///
/// Copyright (c) 2010 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
#include "CATDatabase.h"

#include "sqlite3.h"
#include <vector>
#include <set>

// Convert an SQLite error to our error codes.
CATResult SQLiteErrToCATResult(int sqliteErr);

struct CATDB_INTERNAL 
{
    sqlite3*                             SQLiteHandle;
    const CATDBInfo*                     DBInfo;
    CATDBQuery                           BeginTrans;
    CATDBQuery                           CommitTrans;
    CATDBQuery                           RollbackTrans;
    std::map<CATUInt32, CATDBQuery>      QueryMap;
    std::map<CATUInt32, CATDBQueryInfo*> QueryInfoMap;
    std::map<CATColId,  CATString>       ColumnNames;
    bool                                 InTransaction;
};

CATResult CATDatabase::GlobalDBInit()
{
    static bool sCATDBInit = false;
    if (!sCATDBInit)
    {
        sCATDBInit = true;
        sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
    }
    return CAT_SUCCESS;
}

CATDatabase::CATDatabase()
{
    fInternal = new CATDB_INTERNAL;
    fInternal->SQLiteHandle  = 0;
    fInternal->DBInfo        = 0;
    fInternal->BeginTrans    = 0;
    fInternal->CommitTrans   = 0;
    fInternal->RollbackTrans = 0;
    fInternal->InTransaction = false;
}

CATDatabase::~CATDatabase()
{
    if (fInternal)
    {
        if (fInternal->SQLiteHandle)
            Close();

        delete fInternal;
        fInternal = 0;
    }
}


CATResult CATDatabase::Open(const CATString& path, const CATDBInfo* dbInfo)
{
    if (fInternal->SQLiteHandle != 0)
        Close();

    int sqliteRes = sqlite3_open_v2(path, &fInternal->SQLiteHandle,SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE,0);
    CATResult res = SQLiteErrToCATResult(sqliteRes);
    // If we failed to open an existing database, create one and call our OnCreate().
    if (CATFAILED(res))
    {
        if (CATFAILED(res))
            return res;
    }
    fInternal->DBInfo = dbInfo;        
    res = CreateUpdateDB();
    if (CATFAILED(res))
    {
        Close();
    }
    else
    {
        CreateQuery("BEGIN TRANSACTION",&fInternal->BeginTrans);
        CreateQuery("COMMIT TRANSACTION",&fInternal->BeginTrans);
        CreateQuery("ROLLBACK TRANSACTION",&fInternal->BeginTrans);
    }

    sqlite3_commit_hook(fInternal->SQLiteHandle, OnCommitCB, this);
    return res;
}

bool CheckTableCB(CATUInt32 curRow, CATUInt32 numCol, CATVariant* rowResults, const CATColId* colIdArray, void* context)
{
    std::set<CATString>* names = (std::set<CATString>*)context;
    names->insert(rowResults[1].GetString());    
    return true;
}

CATResult CATDatabase::CreateUpdateDB()
{
    CATResult tmpResult;
    CATResult finalResult = CAT_SUCCESS;
    CATString creationQuery;
    const CATDBTableInfo*  tables   = 0;
    const CATDBColumnInfo* columns  = 0;

    if (fInternal->DBInfo != 0)
    {
        tables = fInternal->DBInfo->tables;
        while (tables && (tables->name != 0))
        {
            // Create the table and its columns if it doesn't exist.
            creationQuery = "CREATE TABLE IF NOT EXISTS ";
            creationQuery << tables->name << "(";
            
            columns = tables->columns;
            while ((columns) && (columns->name != 0))
            {
                creationQuery << columns->name << " " << GetSQLiteTypeString(columns->dataType, columns->flags);
                if (columns->fieldLen > 1)
                {
                    creationQuery << "(" << columns->fieldLen << ")";
                }

                fInternal->ColumnNames[columns->id] = columns->name;

                ++columns;
                if (columns->name != 0)
                {
                    creationQuery << ",";
                }
            }
            creationQuery << ")";            
            
            if (CATFAILED(tmpResult = CustomQuery(creationQuery,0,0,0)))
            {
                CATTRACE("FAILED Creating Table!\n");
                if (CATSUCCEEDED(finalResult))
                    tmpResult = finalResult;
            }                          

            // Ok, the table should be created now.  
            // Now double-check the columns and add any missing ones.
            creationQuery = "PRAGMA table_info(";
            creationQuery << tables->name << ");";
            std::set<CATString> columnNames;
            if (CATFAILED(tmpResult = CustomQuery(creationQuery,0,CheckTableCB,&columnNames)))
            {
                CATTRACE("FAILED Creating Table!\n");
                if (CATSUCCEEDED(finalResult))
                    tmpResult = finalResult;
            }

            columns = tables->columns;
            while ((columns) && (columns->name != 0))
            {
                std::set<CATString>::iterator cit = columnNames.find(columns->name);
                if (cit == columnNames.end())
                {
                    // Column not present. add it.
                    creationQuery = "ALTER TABLE ";
                    creationQuery << tables->name << " ADD COLUMN " << columns->name << " "
                                  << GetSQLiteTypeString(columns->dataType, columns->flags);
                    if (columns->fieldLen > 1)
                    {
                        creationQuery << "(" << columns->fieldLen << ")";
                    }
                    if (CATFAILED(tmpResult = CustomQuery(creationQuery,0,0,0)))
                    {
                        CATTRACE("FAILED Altering table!\n");
                        if (CATSUCCEEDED(finalResult))
                            tmpResult = finalResult;
                    }
                }
                ++columns;
            }

            ++tables;
        }
    }

    return finalResult;
}

CATResult CATDatabase::Close()
{
    if (fInternal->SQLiteHandle)
    {
        // Free all precompiled queries.
        std::map<CATUInt32, CATDBQuery>::iterator it = fInternal->QueryMap.begin();
        while (it != fInternal->QueryMap.end())
        {
            this->FreeQuery(it->second);
            ++it;
        }
        fInternal->QueryMap.clear();

        fInternal->QueryInfoMap.clear();
        fInternal->ColumnNames.clear();

        FreeQuery(fInternal->BeginTrans);
        FreeQuery(fInternal->CommitTrans);
        FreeQuery(fInternal->RollbackTrans);

        // Close db
        sqlite3_close(fInternal->SQLiteHandle);
        
        fInternal->SQLiteHandle  = 0;
        fInternal->DBInfo        = 0;
        fInternal->BeginTrans    = 0;
        fInternal->CommitTrans   = 0;
        fInternal->RollbackTrans = 0;
        fInternal->InTransaction = false;
    }
    return CAT_SUCCESS;
}

int CATDatabase::OnCommitCB(void* context)
{
    CATDatabase* catDb = (CATDatabase*)context;
    if (!catDb)
        return 0;
    return catDb->OnCommitComplete();
}

int CATDatabase::OnCommitComplete()
{
    return 0;
}

CATResult CATDatabase::CreateQuery( const CATString&    sqlQuery,
                                    CATDBQuery*         queryOut)
{
    sqlite3_stmt* queryHandle = 0;
    int sqlResult = sqlite3_prepare_v2(fInternal->SQLiteHandle, sqlQuery,-1,&queryHandle,0);
    CATResult res = SQLiteErrToCATResult(sqlResult);

    if (CATSUCCEEDED(res) && (queryOut != 0))
    {
        *queryOut = (CATDBQuery)queryHandle;
        return CAT_SUCCESS;
    }
    
    CATString errMsg = sqlite3_errmsg(fInternal->SQLiteHandle);
    CATTRACE(errMsg);
    return res;   
}

CATResult CATDatabase::ExecQuery(   CATDBQuery          query,
                                    const CATColId*           colArray,
                                    CATDBRowCallback    callback,
                                    void*               context)
{
    sqlite3_stmt* queryHandle = (sqlite3_stmt*)query;

    int numRows    = 0;
    int numCols    = 0;
    CATResult res;

    // Step each row in the query result
    for (;;)
    {
        int sqlResult  = sqlite3_step(queryHandle);
        res            = SQLiteErrToCATResult(sqlResult);

        if (CAT_STAT_SQL_ROW != res)
            break;

        if (!numCols)
        {
            if (0 == (numCols = sqlite3_column_count(queryHandle)))
                return CAT_SUCCESS;
        }

        // Create row data and fill
        CATVariant* rowResults = new CATVariant[numCols];
        for (int col = 0; col < numCols; col++)
        {
            rowResults[col].Clear();
            int valType = sqlite3_column_type(queryHandle,col);
            switch (valType)
            {
                case SQLITE_INTEGER:
                    rowResults[col].SetInt64(sqlite3_column_int64(queryHandle,col));
                    break;
                case SQLITE_FLOAT:
                    rowResults[col].SetDouble(sqlite3_column_double(queryHandle,col));
                    break;
                case SQLITE3_TEXT:
                    rowResults[col].SetString((const char*)sqlite3_column_text(queryHandle,col));
                    break;
                case SQLITE_BLOB:
                case SQLITE_NULL:
                default:
                    break;
            }
        }
        
        if (callback)
            res = callback(numRows,numCols,rowResults,colArray,context);
    
        delete [] rowResults;

        ++numRows;
      
        if (res == CAT_ERR_SQL_ABORT)
            break;
    }

    sqlite3_reset(queryHandle);

    if (CATFAILED(res))
    {
        CATString errMsg = sqlite3_errmsg(fInternal->SQLiteHandle);
        CATTRACE(errMsg);
    }

    return res;
}

CATResult CATDatabase::FreeQuery( CATDBQuery query)
{
    int sqlRes = sqlite3_finalize((sqlite3_stmt*)query);
    return SQLiteErrToCATResult(sqlRes);
}

CATResult CATDatabase::SetNamedParam(   CATDBQuery          query,
                                        const CATString&    name,
                                        const CATVariant&   paramValue)
{
    sqlite3_stmt* queryHandle = (sqlite3_stmt*)query;
    int paramIndex = sqlite3_bind_parameter_index(queryHandle,name);
    return SetParam(query,paramIndex,paramValue);
}

CATResult CATDatabase::SetParam( CATDBQuery          query,
                                 CATInt32            paramIndex,
                                 const CATVariant&   paramValue)
{
    sqlite3_stmt* queryHandle = (sqlite3_stmt*)query;
    
    int sqlRes = SQLITE_MISMATCH;
    switch (paramValue.GetType())
    {
        case CATVARIANT_INT64:
            sqlRes = sqlite3_bind_int64( queryHandle,
                                         paramIndex+1,
                                         paramValue.GetInt64());
            break;
        case CATVARIANT_DOUBLE:
            sqlRes = sqlite3_bind_double(queryHandle,
                                         paramIndex+1,
                                         paramValue.GetDouble());
            break;
        case CATVARIANT_TEXT:
            sqlRes = sqlite3_bind_text( queryHandle,
                                        paramIndex+1,
                                        paramValue.GetString(),
                                        -1,SQLITE_TRANSIENT);
            break;
        case CATVARIANT_NULL:
            sqlRes = sqlite3_bind_null( queryHandle,paramIndex+1);
            break;
        default:
            break;
    }
    return SQLiteErrToCATResult(sqlRes);
}

CATDBId CATDatabase::GetLastDBId()
{
    return (CATDBId)sqlite3_last_insert_rowid(fInternal->SQLiteHandle);
}



struct SQLToCAT
{
    int         sqlErr;
    CATResult   catErr;
};

const SQLToCAT kSQLToCAT[] =
{
    // Status codes
    { SQLITE_ROW,		    CAT_STAT_SQL_ROW},       /* sqlite3_step() has another row ready */          
    { SQLITE_DONE,		    CAT_STAT_SQL_DONE},      /* sqlite3_step() has finished executing */
    // Errors
    { SQLITE_ERROR,         CAT_ERR_SQL_ERROR},     /* SQL error or missing database */
    { SQLITE_INTERNAL,		CAT_ERR_SQL_INTERNAL},  /* Internal logic error in SQLite */          
    { SQLITE_PERM,		    CAT_ERR_SQL_PERM},      /* Access permission denied */          
    { SQLITE_ABORT,		    CAT_ERR_SQL_ABORT},     /* Callback routine requested an abort */          
    { SQLITE_BUSY,		    CAT_ERR_SQL_BUSY},      /* The database file is locked */
    { SQLITE_LOCKED,        CAT_ERR_SQL_LOCKED},    /* A table in the database is locked */
    { SQLITE_NOMEM,	        CAT_ERR_SQL_NOMEM},     /* A malloc() failed */
    { SQLITE_READONLY,		CAT_ERR_SQL_READONLY},  /* Attempt to write a readonly database */
    { SQLITE_INTERRUPT,		CAT_ERR_SQL_INTERRUPT}, /* Operation terminated by sqlite3_interrupt()*/
    { SQLITE_IOERR,		    CAT_ERR_SQL_IOERR},     /* Some kind of disk I/O error occurred */          
    { SQLITE_CORRUPT,		CAT_ERR_SQL_CORRUPT},   /* The database disk image is malformed */          
    { SQLITE_NOTFOUND,		CAT_ERR_SQL_NOTFOUND},  /* NOT USED. Table or record not found */          
    { SQLITE_FULL,		    CAT_ERR_SQL_FULL},      /* Insertion failed because database is full */          
    { SQLITE_CANTOPEN,		CAT_ERR_SQL_CANTOPEN},  /* Unable to open the database file */          
    { SQLITE_PROTOCOL,		CAT_ERR_SQL_PROTOCOL},  /* NOT USED. Database lock protocol error */          
    { SQLITE_EMPTY,		    CAT_ERR_SQL_EMPTY},     /* Database is empty */          
    { SQLITE_SCHEMA,		CAT_ERR_SQL_SCHEMA},    /* The database schema changed */          
    { SQLITE_TOOBIG,		CAT_ERR_SQL_TOOBIG},    /* String or BLOB exceeds size limit */          
    { SQLITE_CONSTRAINT,	CAT_ERR_SQL_CONSTRAINT},/* Abort due to constraint violation */          
    { SQLITE_MISMATCH,		CAT_ERR_SQL_MISMATCH},  /* Data type mismatch */          
    { SQLITE_MISUSE,		CAT_ERR_SQL_MISUSE},    /* Library used incorrectly */          
    { SQLITE_NOLFS,		    CAT_ERR_SQL_NOLFS},     /* Uses OS features not supported on host */          
    { SQLITE_AUTH,		    CAT_ERR_SQL_AUTH},      /* Authorization denied */          
    { SQLITE_FORMAT,		CAT_ERR_SQL_FORMAT},    /* Auxiliary database format error */          
    { SQLITE_RANGE,		    CAT_ERR_SQL_RANGE},     /* 2nd parameter to sqlite3_bind out of range */          
    { SQLITE_NOTADB,		CAT_ERR_SQL_NOTADB},    /* File opened that is not a database file */          
    //----
    { SQLITE_OK,CAT_SUCCESS}
};

CATResult SQLiteErrToCATResult(int sqliteErr)
{
    CATResult result = CAT_SUCCESS;
    if (sqliteErr == 0)
        return result;
    
    if ((sqliteErr & 0xff) >= 100)
        result = CAT_STATUS;
    else
        result = CAT_ERR_SQL_ERROR;


    const SQLToCAT* curCheck = &kSQLToCAT[0];
    while (curCheck->sqlErr != SQLITE_OK)
    {
        if (curCheck->sqlErr == sqliteErr)
            return curCheck->catErr;
        ++curCheck;
    }
    return result;
}

CATString CATDatabase::GetSQLiteTypeString(CATVARIANT_TYPE catType, CATDBCOLFLAGS flags)
{
    CATString sqlString;
    switch (catType)
    {
        case CATVARIANT_INT64:
            sqlString = "INTEGER";
            if (flags & CATDBCOL_PRIMARY)
                sqlString << " PRIMARY KEY";
            if (flags & CATDBCOL_AUTOINC)
                sqlString << " AUTOINCREMENT";
            break;
        case CATVARIANT_DOUBLE:
            sqlString = "REAL";
            break;
        case CATVARIANT_TEXT:
            sqlString = "TEXT";
            break;
        case CATVARIANT_NULL:
        default:
            sqlString = "NULL";
            break;
    }
    return sqlString;
}

CATResult CATDatabase::CustomQuery(   const CATString&        query,
                                      CATColId*               colArray,
                                      CATDBRowCallback        callback,
                                      void*                   context)
{
    CATDBQuery queryHandle;
    
    CATResult result = CreateQuery(query,&queryHandle);
    if (CATFAILED(result))
        return result;
    
    result = ExecQuery(queryHandle,colArray,callback,context);
    
    FreeQuery(queryHandle);
    return result;
}
        
CATResult CATDatabase::Query(   CATUInt32                queryId,
                                std::vector<CATVariant>* params,
                                CATDBRowCallback         callback,
                                void*                    context)
{
    CATResult   result      = CAT_SUCCESS;
    CATDBQuery  queryHandle = 0;

    std::map<CATUInt32,CATDBQuery>::iterator it = fInternal->QueryMap.find(queryId);
    const CATDBQueryInfo* curQuery = 0;
    
    if (it != fInternal->QueryMap.end())
    {
        queryHandle = it->second;
        std::map<CATUInt32,CATDBQueryInfo*>::iterator it2 = fInternal->QueryInfoMap.find(queryId);
        if (it2 != fInternal->QueryInfoMap.end())
        {
            curQuery = it2->second;
        }
    }
    else
    {
        // Find the query
        curQuery = fInternal->DBInfo->queries;
        while (curQuery->id != 0)
        {
            if (curQuery->id == queryId)
            {
                // found it.
                break;
            }
            ++curQuery;
        }
        // Compile it
        if (curQuery->id != 0)
        {
            result = this->CreateQuery(curQuery->queryString,&queryHandle);        
            if (!CATFAILED(result))
            {
                fInternal->QueryMap[curQuery->id] = queryHandle;
            }
            else
            {
                return result;
            }
        }
        else
        {
            return CAT_ERR_QUERY_NOT_FOUND;
        }

        if (params)
        {
            for (CATUInt32 i = 0; i < params->size(); i++)
            {
                result = this->SetParam(queryHandle,i,(*params)[i]);
                if (CATFAILED(result))
                    return result;
            }
        }

        result = this->ExecQuery(queryHandle,curQuery->responseColumns,callback,context);
    }

    return result;
}

CATString CATDatabase::GetColumnName(CATColId   column)
{
    CATString colName;
    std::map<CATColId,CATString>::iterator it = fInternal->ColumnNames.find(column);
    if (it != fInternal->ColumnNames.end())
        colName = it->second;
    return colName;
}

CATResult CATDatabase::BeginTransaction    ( )
{
    if (fInternal->InTransaction)
        return CAT_ERR_ALREADY_IN_TRANSACTION;
    
    CATResult result;
    if (CATSUCCEEDED(result = ExecQuery(fInternal->BeginTrans,0,0,0)))
        fInternal->InTransaction = true;
    
    return result;
}

CATResult CATDatabase::CommitTransaction   ( )
{
    if (!fInternal->InTransaction)
        return CAT_ERR_NOT_IN_TRANSACTION;
    
    CATResult result;
    if (CATSUCCEEDED(result = ExecQuery(fInternal->CommitTrans,0,0,0)))
        fInternal->InTransaction = false;
    
    return result;
}

CATResult CATDatabase::RollbackTransaction ( )
{
    if (!fInternal->InTransaction)
        return CAT_ERR_NOT_IN_TRANSACTION;
    
    CATResult result;
    if (CATSUCCEEDED(result = ExecQuery(fInternal->RollbackTrans,0,0,0)))
        fInternal->InTransaction = false;
    
    return result;
}