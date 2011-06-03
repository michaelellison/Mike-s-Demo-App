/// \file  CATDIJoystick.cpp
/// \brief Joystick class for DirectInput
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
#include "CATDIJoystick.h"
#include "CATString.h"
const int kMaxProp = 1000;

CATDIJoystick::CATDIJoystick(IDirectInput8* dInput) : 
CATJoystick()
{
	fDirectInput = dInput;
	fInputDevice = 0;
	memset(&fInputDevCaps,0,sizeof(DIDEVCAPS));
}


CATDIJoystick::~CATDIJoystick()
{
	if (fInputDevice != 0)
	{
		fInputDevice->Unacquire();
		fInputDevice->Release();
		fInputDevice = 0;
	}
}


bool CATDIJoystick::Init(DIDEVICEINSTANCE* devInstance)
{
	// Check if we've pre-allocated the device (we should have in the menu callback)
	if (fInputDevice)
	{
		fInputDevice->Unacquire();
		fInputDevice->Release();
		fInputDevice = 0;
	}
	
	// Allocate direct input device
	if (FAILED(fDirectInput->CreateDevice( devInstance->guidInstance, &fInputDevice, NULL )))
	{
		return false;
	}
	
	// Get capabilities
	memset(&fInputDevCaps,0,sizeof(DIDEVCAPS));
	fInputDevCaps.dwSize = sizeof(DIDEVCAPS);
	if (FAILED(fInputDevice->GetCapabilities(&fInputDevCaps)))
	{
		fInputDevice->Release();
		fInputDevice = 0;
		return false;
	}
	
	// Set device format
	if (FAILED(fInputDevice->SetDataFormat(&c_dfDIJoystick)))
	{
		fInputDevice->Release();
		fInputDevice = 0;
		return false;
	}

	// Set device behaviour
	HWND hwnd = 0;

	// Set cooperative level so we'll always be able to receive data 
	if (FAILED(fInputDevice->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
	{
		fInputDevice->Release();
		fInputDevice = 0;
		return false;
	}

	// Determine valid axes
	fValidAxis = 0;
	fInputDevice->EnumObjects(AxesCallback, this, DIDFT_AXIS);
	
	if (FAILED(fInputDevice->Acquire()))
	{
		fInputDevice->Release();
		fInputDevice = 0;
		return false;
	}

	return true;
}

// Callback from direct input for valid axis
BOOL CALLBACK CATDIJoystick::AxesCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi,	LPVOID pvRef )
{	
	CATDIJoystick* jstick = (CATDIJoystick*)pvRef;
	if (jstick == 0)
	{
		return TRUE;
	}

	CATUInt32* validAxis = (CATUInt32*)&jstick->fValidAxis;
	if (validAxis == 0)
	{
		return TRUE;
	}


	// Save valid axis
	if (0 == memcmp(&lpddoi->guidType,&GUID_XAxis, sizeof(GUID)))
	{
		*validAxis |= X_AXIS | X_AXIS_LOW | X_AXIS_HIGH;
	}
	else if (0 == memcmp(&lpddoi->guidType,&GUID_YAxis,sizeof(GUID)))
	{
		*validAxis |= Y_AXIS | Y_AXIS_LOW | Y_AXIS_HIGH;
	}
	else if (0 == memcmp(&lpddoi->guidType,&GUID_ZAxis,sizeof(GUID)))
	{
		*validAxis |= Z_AXIS;
	}
	else if (0 == memcmp(&lpddoi->guidType,&GUID_RxAxis,sizeof(GUID)))
	{
		*validAxis |= X_ROT | X_ROT_LOW | X_ROT_HIGH;
	}
	else if (0 == memcmp(&lpddoi->guidType,&GUID_RyAxis,sizeof(GUID)))
	{
		*validAxis |= Y_ROT | Y_ROT_LOW | Y_ROT_HIGH;
	}
	else if (0 == memcmp(&lpddoi->guidType,&GUID_RzAxis,sizeof(GUID)))
	{
		*validAxis |= Z_ROT | Z_ROT_LOW | Z_ROT_HIGH;;
	}

	
	// Set min/max for axis
	DIPROPRANGE diprg; 	
	diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 	
	diprg.diph.dwHow        = DIPH_BYID; 
	diprg.diph.dwObj        = lpddoi->dwType;
	diprg.lMin              = 0;
	diprg.lMax              = kMaxProp;
	
	if (FAILED(jstick->fInputDevice->SetProperty(DIPROP_RANGE, &diprg.diph)))
	{
	}
	
	return TRUE;
}

bool CATDIJoystick::Configure(HWND hwnd)
{
	if (fInputDevice)
	{

		fInputDevice->RunControlPanel(hwnd,0);
	}
	else
	{
		fDirectInput->RunControlPanel(hwnd,0);
	}

	return true;
}

bool CATDIJoystick::GetStatus(CATJOYSTICKSTRUCT& status)
{
	if (this->fInputDevice)
	{
		if (FAILED(fInputDevice->Poll()))
		{
			if (FAILED(fInputDevice->Acquire()))
			{
				return false;
			}
			fInputDevice->Poll();
		}

		DIJOYSTATE curState;
		if (FAILED(fInputDevice->GetDeviceState(sizeof(DIJOYSTATE), &curState)))
		{
			return false;
		}

		status.xAxis = (curState.lX)/10;
		status.yAxis = (curState.lY)/10;
		status.zAxis = (curState.lZ)/10;
		status.xRot  = (curState.lRx)/10;
		status.yRot  = (curState.lRy)/10;
		status.zRot  = (curState.lRz)/10;
        
        // rgdwPOV - 100ths of degrees clockwise from north, mostly.        
        // Per msdn, some drivers only set low word for centered.
        if (LOWORD(curState.rgdwPOV[0]) == 0xffff)
        {
            status.xPov = status.yPov = 50;
        }
        else
        {
            CATFloat32 rads = (CATFloat32)((curState.rgdwPOV[0]/100.0)*kCATDEGTORAD);
            status.xPov = (CATInt32)(-sin(rads)*50 + 50);
            status.yPov = (CATInt32)(-cos(rads)*50 + 50);
        }        
        
        // Half axis
        status.xLow  = (curState.lX <  kMaxProp/2) ? ((kMaxProp/2  - curState.lX)/5) : 0;
        status.xHigh = (curState.lX >= kMaxProp/2) ? ((curState.lX - kMaxProp/2)/5)  : 0;
        status.yLow  = (curState.lY <  kMaxProp/2) ? ((kMaxProp/2  - curState.lY)/5) : 0;
        status.yHigh = (curState.lY >= kMaxProp/2) ? ((curState.lY - kMaxProp/2)/5)  : 0;
        status.zLow  = (curState.lZ <  kMaxProp/2) ? ((kMaxProp/2  - curState.lZ)/5) : 0;
        status.zHigh = (curState.lZ >= kMaxProp/2) ? ((curState.lZ - kMaxProp/2)/5)  : 0;

        status.xRotLow  = (curState.lRx <  kMaxProp/2) ? ((kMaxProp/2   - curState.lRx)/5) : 0;
        status.xRotHigh = (curState.lRx >= kMaxProp/2) ? ((curState.lRx - kMaxProp/2)/5)   : 0;
        status.yRotLow  = (curState.lRy <  kMaxProp/2) ? ((kMaxProp/2   - curState.lRy)/5) : 0;
        status.yRotHigh = (curState.lRy >= kMaxProp/2) ? ((curState.lRy - kMaxProp/2)/5)   : 0;
        status.zRotLow  = (curState.lRz <  kMaxProp/2) ? ((kMaxProp/2   - curState.lRz)/5) : 0;
        status.zRotHigh = (curState.lRz >= kMaxProp/2) ? ((curState.lRz - kMaxProp/2)/5)   : 0;
        
            

		status.buttonMap = 0;		
		for (CATInt32 i = kCATJOYSTICKBTNMAX - 1; i >= 0; i--)
		{
			status.buttonMap <<= 1;
			status.buttonMap |= (curState.rgbButtons[i] > 0)?1:0;			
		}
	}
	return true;
}