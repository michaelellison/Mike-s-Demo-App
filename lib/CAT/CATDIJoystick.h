/// \file  CATDIJoystick.h
/// \brief DirectInput joystick wrapper
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
#ifndef _CATDIJoystick_H_
#define _CATDIJoystick_H_

#include "CATInternal.h"
#include "CATDirectInput.h"
#include "CATJoystick.h"

class CATDIJoystick : public CATJoystick
{
	public:
		CATDIJoystick(IDirectInput8* diInput);		
		virtual ~CATDIJoystick();


		virtual bool Configure(HWND hwnd);


		bool Init(DIDEVICEINSTANCE* devInstance);
		
		virtual bool GetStatus(CATJOYSTICKSTRUCT& status);

	protected:
		static BOOL CALLBACK AxesCallback(LPCDIDEVICEOBJECTINSTANCE	lpddoi,	LPVOID pvRef );

		IDirectInput8*				fDirectInput;
		IDirectInputDevice8*	    fInputDevice;
		DIDEVCAPS					fInputDevCaps;
};

#endif // _CATDIJoystick_H_