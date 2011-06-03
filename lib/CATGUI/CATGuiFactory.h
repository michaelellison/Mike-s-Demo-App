//---------------------------------------------------------------------------
/// \file CATGuiFactory.h
/// \brief Object factory for creating a GUI from XML
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
#ifndef CATGUIFactory_H_
#define CATGUIFactory_H_

#include "CATXMLObject.h"
#include "CATXMLFactory.h"

/// \class CATGuiFactory
/// \brief Object factory for creating a GUI from XML
/// \ingroup CATGUI
///
/// \sa CATXMLParser, CATXMLFactory
class CATGuiFactory : public CATXMLFactory
{
    public:
        /// CATGuiFactory constructor.
        ///
        /// CATGuiFactory is an overridden version of CATXMLFactory specifically
        /// for creating the GUI framework from an .XML skin.
        ///
        /// \param skinRoot - the base directory of the skin
        /// \param skinPath - full path to the skin
        CATGuiFactory(const CATString& skinRoot, const CATString& skinPath);

        /// CATGuiFactory destructor
        virtual ~CATGuiFactory();

        virtual CATXMLObject* CreateObject( const CATWChar* objectType);

    protected:
        /// fSkinRoot is the base directory of the skin. This is used
        /// to find supporting files referenced by the skin
        CATString    fSkinRoot;

        /// fSkinPath holds the full path for the skin's .XML file.
        CATString    fSkinPath;
};

#endif // CATGUIFactory_H_


