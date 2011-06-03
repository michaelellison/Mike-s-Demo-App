//---------------------------------------------------------------------------
/// \file    CATOverlayFactory.h
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
//---------------------------------------------------------------------------
#ifndef CATOverlayFactory_H_
#define CATOverlayFactory_H_

#include "CATXMLObject.h"
#include "CATXMLFactory.h"

/// \class CATOverlayFactory
/// \brief   Object factory for creating a game overlay from XML
/// \ingroup CATGUI
///
/// \sa CATXMLParser, CATXMLFactory
class CATOverlayFactory : public CATXMLFactory
{
    public:
        /// CATOverlayFactory constructor.
        ///
        /// CATOverlayFactory is an overridden version of CATXMLFactory specifically
        /// for creating game overlays and interfaces from an .XML skin.
        ///
        /// \param skinRoot - the base directory of the skin
        /// \param skinPath - full path to the skin
        CATOverlayFactory(const CATString& skinRoot, const CATString& skinPath);

        /// CATOverlayFactory destructor
        virtual ~CATOverlayFactory();

        virtual CATXMLObject* CreateObject( const CATWChar* objectType);

    protected:
        /// fSkinRoot is the base directory of the skin. This is used
        /// to find supporting files referenced by the skin
        CATString    fSkinRoot;

        /// fSkinPath holds the full path for the skin's .XML file.
        CATString    fSkinPath;
};

#endif // CATOverlayFactory_H_


