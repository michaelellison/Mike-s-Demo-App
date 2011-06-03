/// \file  CATDirectInput.h
/// \brief Basic DirectInput wrapper
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATInternal.h"
#include "CATDirectInput.h"
#include "CATDIJoystick.h"

// Constructor
CATDirectInput::CATDirectInput()
{
	fDirectInput	= 0;	
	fInitialized	= false;
	fUserCallback	= 0;
	fUserParam		= 0;
}

// Initialize direct input
bool CATDirectInput::Init()
{
    if (DI_OK == DirectInput8Create((HINSTANCE)::GetModuleHandle(0) ,DIRECTINPUT_VERSION,IID_IDirectInput8W, (void**)&fDirectInput, NULL))
	{
		fInitialized = true;
	}
	return fInitialized;
}

CATDirectInput::~CATDirectInput()
{
    if (fDirectInput)
	    fDirectInput->Release();
	
    fDirectInput = 0;

	while (fJoystickList.size())
	{
		DIDEVICEINSTANCE* inputDevInfo = fJoystickList[0];
        delete inputDevInfo;
        fJoystickList.erase(fJoystickList.begin());
	}

}

// Start the control panel for direct input
void CATDirectInput::DoControlPanel(HWND parent)
{
	if (fDirectInput)
	{
		fDirectInput->RunControlPanel(parent, 0);
	}
}

// Enumerate joysticks and callback the caller
void CATDirectInput::EnumJoysticks(EnumJoysticksCallback callback, void *userParam)
{
	fUserCallback = callback;
	fUserParam = userParam;

	this->fDirectInput->EnumDevices(
						DI8DEVCLASS_GAMECTRL, 
						DIJoysticksCB, 
						this, 
						DIEDFL_ATTACHEDONLY);
}


// Static direct input callback
BOOL CALLBACK CATDirectInput::DIJoysticksCB(	LPCDIDEVICEINSTANCE lpddi,	LPVOID pvRef )
{
	CATDirectInput* cdi = (CATDirectInput*)pvRef;
	
	// Copy device instance data and add to list
	DIDEVICEINSTANCE* inputDevInfo = new DIDEVICEINSTANCE;
	memcpy(inputDevInfo,lpddi,sizeof(DIDEVICEINSTANCE));
	cdi->fJoystickList.push_back(inputDevInfo);
	
	// Call user callback
	if (cdi->fUserCallback)
	{
		cdi->fUserCallback(inputDevInfo->tszInstanceName, cdi->fUserParam);
	}

	return TRUE;
}


// Create a joystick interface
CATJoystick* CATDirectInput::CreateJoystick(const CATWChar* name)
{	
	for (CATUInt32 i = 0; i < fJoystickList.size(); i++)
	{
		DIDEVICEINSTANCE* inputDevInfo;
		if (inputDevInfo = fJoystickList[i])
		{
			if (0 == _wcsicmp(inputDevInfo->tszInstanceName,name))
			{
				CATDIJoystick* joystick = 0;
				joystick	= new CATDIJoystick(this->fDirectInput);
				if (joystick->Init(inputDevInfo))
				{
					return joystick;
				}				
				delete joystick;
				joystick = 0;
			}
		}
	}

	return 0;
}