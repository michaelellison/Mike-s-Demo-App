/// \file CATStringTable.h
/// \brief String table for internationalization
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-06-02 02:47:03 -0500 (Thu, 02 Jun 2011) $
// $Revision: 7 $
// $NoKeywords: $

#ifndef _CATStringTable_H_
#define _CATStringTable_H_

#include "CATInternal.h"
#include "CATString.h"

/// \class CATStringTable
/// \brief Base class for string tables for internationalization
/// \ingroup CAT
///
/// See CATStringTableCore.h and CATStringsCore.xml for current usage.
class CATStringTable
{
    public:
                                CATStringTable   ();
        virtual                 ~CATStringTable  ();

        CATResult               AddSubTable     ( CATStringTable*  childTable );
        CATResult               RemoveSubTable  ( CATStringTable*  childTable );
        virtual CATString       GetString       ( CATUInt32        stringId   );
    protected:
        std::vector<CATStringTable*>              fSubTables;
};

#endif // _CATStringTable_H_
