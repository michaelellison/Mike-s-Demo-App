//---------------------------------------------------------------------------
/// \file CATGuiFactory.cpp
/// \brief Base Object factory for creating objects from XML
/// \ingroup CATGUI
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-06-02 02:47:03 -0500 (Thu, 02 Jun 2011) $
// $Revision: 7 $
// $NoKeywords: $
//
//
//---------------------------------------------------------------------------
#include "CATGuiFactory.h"

// Skin class
#include "CATSkin.h"

// Window class
#include "CATWindow.h"

// Controls
#include "CATControl.h"
#include "CATButton.h"
#include "CATAppButton.h"
#include "CATIconButton.h"
#include "CATSwitch.h"
#include "CATIconSwitch.h"
#include "CATSlider.h"
#include "CATKnob.h"
#include "CATLabel.h"
#include "CATPicture.h"
#include "CATEditBox.h"
#include "CATListBox.h"
#include "CATTreeCtrl.h"
#include "CATProgress.h"
#include "CATMenu.h"
#include "CATLayer.h"
#include "CATTab.h"
#include "CATRadioButton.h"
#include "CATComboBox.h"
#include "CATSwitchMulti.h"
#include "CATPictureMulti.h"
#include "CAT3DView.h"
#include "CAT3DVideo.h"
//---------------------------------------------------------------------------
CATGuiFactory::CATGuiFactory(const CATString& skinRoot, const CATString& skinPath)
: CATXMLFactory()
{
    fSkinRoot = skinRoot;
    fSkinPath = skinPath;
}

//---------------------------------------------------------------------------
CATGuiFactory::~CATGuiFactory()
{
}


CATXMLObject* CATGuiFactory::CreateObject( const CATWChar* objType)
{
    CATXMLObject* newObject = 0;

    if (wcscmp(objType, L"Skin"     ) == 0 )  newObject = new CATSkin    ( objType, fSkinRoot, fSkinPath);   
    else if (wcscmp(objType, L"Window"   ) == 0 )  newObject = new CATWindow  ( objType, fSkinRoot);   
    else if (wcscmp(objType, L"Button"   ) == 0 )  newObject = new CATButton  ( objType, fSkinRoot);
    else if (wcscmp(objType, L"AppButton") == 0 )  newObject = new CATAppButton( objType, fSkinRoot);
    else if (wcscmp(objType, L"IconButton")== 0 )  newObject = new CATIconButton( objType, fSkinRoot);
    else if (wcscmp(objType, L"Switch"   ) == 0 )  newObject = new CATSwitch  ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"IconSwitch") == 0 )  newObject = new CATIconSwitch( objType,  fSkinRoot);
    else if (wcscmp(objType, L"SwitchMulti") == 0 )  newObject = new CATSwitchMulti  ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"RadioButton") == 0 )newObject = new CATRadioButton  ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"Slider"   ) == 0 )  newObject = new CATSlider  ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"Knob"     ) == 0 )  newObject = new CATKnob    ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"Label"    ) == 0 )  newObject = new CATLabel   ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"Picture"  ) == 0 )  newObject = new CATPicture ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"PictureMulti"  ) == 0 )  newObject = new CATPictureMulti ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"EditBox"  ) == 0 )  newObject = new CATEditBox ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"ListBox"  ) == 0 )  newObject = new CATListBox ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"Tree"     ) == 0 )  newObject = new CATTreeCtrl( objType,  fSkinRoot);
    else if (wcscmp(objType, L"Progress" ) == 0 )  newObject = new CATProgress( objType,  fSkinRoot);
    else if (wcscmp(objType, L"Menu"     ) == 0 )  newObject = new CATMenu    ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"Layer"    ) == 0 )  newObject = new CATLayer   ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"Tab"      ) == 0 )  newObject = new CATTab     ( objType,  fSkinRoot);
    else if (wcscmp(objType, L"ComboBox" ) == 0 )  newObject = new CATComboBox( objType,  fSkinRoot);
	 else if (wcscmp(objType, L"View3D"   ) == 0 )  newObject = new CAT3DView  ( objType,  fSkinRoot);
	 else if (wcscmp(objType, L"Video3D"  ) == 0 )  newObject = new CAT3DVideo ( objType,  fSkinRoot);
    else
    {        
        CATASSERT(false,"Unknown GUI type! Check the element name.");
        return new CATControl(objType, fSkinRoot);
    }

    return newObject;
}

