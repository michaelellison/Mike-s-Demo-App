/// \file CATStringTable.cpp
/// \brief String table for internationalization
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATStringTable.h"

CATStringTable::CATStringTable()
{
}

CATStringTable::~CATStringTable()
{
}

CATResult CATStringTable::AddSubTable( CATStringTable*  childTable )
{
    fSubTables.push_back(childTable);
    return CAT_SUCCESS;
}

CATResult CATStringTable::RemoveSubTable( CATStringTable*  childTable )
{
    std::vector<CATStringTable*>::iterator iter;

    for (iter = fSubTables.begin(); iter != fSubTables.end(); ++iter)
    {
        if ( *iter == childTable)
        {
            fSubTables.erase(iter);
            return CAT_SUCCESS;
        }
    }
    return CAT_ERR_STRINGTABLE_NOT_FOUND;
}

CATString CATStringTable::GetString(CATUInt32     stringId)
{
    CATString theString;
    // Scan subtables for string
    for (size_t i = 0; i < fSubTables.size(); i++)
    {
        theString = fSubTables[i]->GetString(stringId);
        if (!theString.IsEmpty())
            return theString;
    }

    return theString;
}
