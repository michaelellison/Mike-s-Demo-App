//---------------------------------------------------------------------------
/// \file CATComboBox.h
/// \brief GUI Layer object - contains sub-controls
/// \ingroup CATGUI
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------
#ifndef CATComboBox_H_
#define CATComboBox_H_

#include "CATLayer.h"

class CATEditBox;
class CATMenu;

/// \class CATComboBox CATComboBox.h
/// \brief GUI Layer object
/// \ingroup CATGUI
class CATComboBox : public CATLayer
{
public:
    /// Constructor - mirrors CATXMLObject() constructor for now.
    CATComboBox( const CATString&               element, 
        const CATString&               rootDir);

    virtual ~CATComboBox();

    /// Load() loads the skin in
    virtual        CATResult Load(		// Optional progress callback information
        CATPROGRESSCB				progressCB   = 0,
        void*							progressParam= 0,
        CATFloat32						progMin      = 0.0f,
        CATFloat32						progMax		 = 1.0f);

    virtual void   OnParentCreate();
    virtual void   OnParentDestroy();

    // Process commands from children for combo.
    virtual void   OnCommand( CATCommand& command,
        CATControl* ctrl);         

    /// Event handler
    virtual CATResult OnEvent(const CATEvent& event, CATInt32& retVal);

    virtual CATEditBox*     GetEditBox();
    virtual CATMenu*        GetMenu();

#ifdef _WIN32
    virtual CATUInt32     GetAccessRole() {return CAT_ROLE_SYSTEM_COMBOBOX;}
#endif

protected:
    /// ParseAttributes() parses the known attributes for an object.
    virtual CATResult ParseAttributes();

    void              RebuildCombo();
    void              AddComboString( const CATString& string);

    // Sub controls
    CATEditBox*        fComboEdit;
    CATMenu*           fComboMenu;
    std::vector<CATString>  fMenuList;
    CATUInt32           fMaxMenuLength;
    CATString          fPrefName;     
};

#endif // CATComboBox_H_
