/// \file CATDatabase.h
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
#ifndef __CATDatabase_H__
#define __CATDatabase_H__

#include "CATInternal.h"
#include "CATString.h"
#include "CATVariant.h"

#include <vector>
#include <map>

// Opaque query type
typedef void*     CATDBQuery;

// 64-bit database record identifier
typedef CATUInt64 CATDBId;

// Column Id type
typedef CATUInt32  CATColId;

typedef bool (*CATDBRowCallback)(CATUInt32 curRow, CATUInt32 numCol, CATVariant* rowResults, const CATColId* colIdArray, void* context);

enum CATDBCOLFLAGS
{
    CATDBCOL_NONE      = 0,
    CATDBCOL_PRIMARY   = 0x01,
    CATDBCOL_AUTOINC   = 0x02,
    CATDBCOL_PRIMEAUTO = 0x03
};

enum CATDBQUERYFLAGS 
{
    CATDBQUERY_NONE     = 0,
    CATDBQUERY_ONCREATE = 0x1
};

// Column definition
struct CATDBColumnInfo
{
    CATColId        id;
    const char*     name;
    CATVARIANT_TYPE dataType;
    CATDBCOLFLAGS   flags;
    CATUInt32       fieldLen;
};

// Table = collection of columns
struct CATDBTableInfo
{
    const char*            name;
    const CATDBColumnInfo* columns;
};

// Queries
struct CATDBQueryInfo
{
    CATUInt32       id;
    const char*     queryString;
    const CATColId* responseColumns;
    CATDBQUERYFLAGS flags;
};

// Database = collection of tables and queries
struct CATDBInfo
{
    CATString                   name;
    const CATDBTableInfo*       tables;
    CATDBQueryInfo*             queries;
};


class CATDatabase
{
    public:
        static CATResult GlobalDBInit();

        CATDatabase();
        virtual ~CATDatabase();

        virtual CATResult Open              (   const CATString&    path,
                                                const CATDBInfo*    dbInfo);
        virtual CATResult Close();
                      
        virtual CATDBId   GetLastDBId       ();

        virtual CATResult CustomQuery       (   const CATString&        query,
                                                CATColId*               colArray,
                                                CATDBRowCallback        callback,
                                                void*                   context);
        
        virtual CATResult Query             (   CATUInt32                queryId,
                                                std::vector<CATVariant>* params,
                                                CATDBRowCallback         callback,
                                                void*                    context);

        virtual CATResult BeginTransaction    ( );
        virtual CATResult CommitTransaction   ( );
        virtual CATResult RollbackTransaction ( );
        // --------------------------------------------------------------------
        // Low level query functions
        // --------------------------------------------------------------------
        virtual CATString GetColumnName     (   CATColId            columnId);

        virtual CATResult SetParam          (   CATDBQuery          query,
                                                CATInt32            paramIndex,
                                                const CATVariant&   paramValue);

        virtual CATResult SetNamedParam     (   CATDBQuery          query,
                                                const CATString&    name,
                                                const CATVariant&   paramValue);

        virtual CATResult CreateQuery       (   const CATString&    sqlQuery,
                                                CATDBQuery*         queryOut);
        
        virtual CATResult ExecQuery         (   CATDBQuery          query,
                                                const CATColId*     colArray,
                                                CATDBRowCallback    callback,
                                                void*               context);

        virtual CATResult FreeQuery         (   CATDBQuery          query);

        static CATString GetSQLiteTypeString(CATVARIANT_TYPE catType, CATDBCOLFLAGS flags);

    protected:
        // Sets up a new database / makes sure the opened database conforms.
        virtual CATResult CreateUpdateDB();

        // Called when a commit occurs
        virtual int OnCommitComplete();

        // Callback from sqlite on commit.
        static int OnCommitCB(void* context);

        // Keep the messy details out of the header.
        struct CATDB_INTERNAL *fInternal;        
};

#endif // __CATDatabase_H__
