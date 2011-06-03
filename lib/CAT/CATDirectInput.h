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
#ifndef CATDirectInput_H_
#define CATDirectInput_H_

#include "CATInternal.h"
#ifdef CAT_CONFIG_WIN32

#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>
#include "CATJoystick.h"
#include <vector>

class CATDirectInput
{
	public:
		CATDirectInput();		
		
        virtual ~CATDirectInput();
		
		// Must call init after creation before use
		bool Init();
		
		void DoControlPanel(HWND parent = 0);

		typedef void (*EnumJoysticksCallback)(const CATWChar* name, void* userParam);
		
        void EnumJoysticks(EnumJoysticksCallback callback, void *userParam);
		
		CATJoystick* CreateJoystick(const CATWChar* name);
        

	protected:
		bool							fInitialized;
		IDirectInput8*				    fDirectInput;

        std::vector<DIDEVICEINSTANCE*>  fJoystickList;
		EnumJoysticksCallback	        fUserCallback;
		void*							fUserParam;
		
		// Direct Input joystick callback
		static BOOL CALLBACK DIJoysticksCB(	LPCDIDEVICEINSTANCE lpddi,	LPVOID pvRef );
};

#endif // CAT_CONFIG_WIN32
#endif //CATDirectInput_H_
