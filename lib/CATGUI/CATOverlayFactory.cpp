//---------------------------------------------------------------------------
/// \file CATOverlayFactory.cpp
/// \brief   Object factory for creating a game overlay from XML
/// \ingroup CATGUI
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//
//
//---------------------------------------------------------------------------
#include "CATOverlayFactory.h"

//---------------------------------------------------------------------------
CATOverlayFactory::CATOverlayFactory(const CATString& skinRoot, const CATString& skinPath)
: CATXMLFactory()
{
    fSkinRoot = skinRoot;
    fSkinPath = skinPath;
}

//---------------------------------------------------------------------------
CATOverlayFactory::~CATOverlayFactory()
{
}


CATXMLObject* CATOverlayFactory::CreateObject( const CATWChar* objType)
{
    CATXMLObject* newObject = 0;
    return newObject;
}

