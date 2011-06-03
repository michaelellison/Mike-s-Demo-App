/// \file CATStringTableCore.h
/// \brief Core String table for CAT library
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATStringTableCore_H_
#define _CATStringTableCore_H_

#include "CATStringTable.h"
#include <map>
/// \class CATStringTableCore
/// \brief Core string table for CAT library
/// \ingroup CAT
///
/// \todo
/// Currently implemented as a bunch of hardcoded strings. This
/// needs to be changed to dynamically loaded from XML or similar.
class CATStringTableCore : public CATStringTable
{
    public:
                                CATStringTableCore  ();
        virtual                 ~CATStringTableCore ();

        virtual CATResult       LoadXMLStringTable  (const CATString& path, const char *langId = "eng");
        virtual CATResult       GenHeaderForXML     (const CATString& path);
        virtual CATResult       GenHTML             (const CATString& path);
        virtual CATString       GetString           (CATUInt32        stringId);
    
protected:
        std::map<CATUInt32, CATString> fStringMap;
};

#endif // _CATStringTableCore_H_
