/// \file    CATWindowHook.h
/// \brief   Hooks windows messages in the current app's top-level window
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATWindowHook_H_
#define _CATWindowHook_H_

#include "CATInternal.h"
#ifdef CAT_CONFIG_WIN32

/// \class CATWindowHook
/// \brief Hooks windows messages in the current app's top-level window
/// \ingroup CAT
///
class CATWindowHook
{
    public:
        CATWindowHook();
        virtual ~CATWindowHook();

    protected:  

};

#endif //CAT_CONFIG_WIN32
#endif //_CATWindowHook_H_