/// \file  CATJoystick.h
/// \brief Base Joystick class
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
#ifndef _CATJoystick_H_
#define _CATJoystick_H_

#include "CATInternal.h"

const CATUInt32 kCATJOYSTICKBTNMAX = 32;
// Right now, each axis goes from 0 to 100.
struct CATJOYSTICKSTRUCT
{
	CATInt32  xAxis;
	CATInt32  xLow;
	CATInt32  xHigh;
	CATInt32  yAxis;
	CATInt32  yLow;
	CATInt32  yHigh;
	CATInt32  zAxis;
	CATInt32  zLow;
	CATInt32  zHigh;
	CATInt32  xRot;
	CATInt32  xRotLow;
	CATInt32  xRotHigh;
	CATInt32  yRot;
	CATInt32  yRotLow;
	CATInt32  yRotHigh;
	CATInt32  zRot;
	CATInt32  zRotLow;
	CATInt32  zRotHigh;
    CATInt32  xPov;
    CATInt32  xPovLow;
    CATInt32  xPovHigh;
    CATInt32  yPov;
    CATInt32  yPovLow;
    CATInt32  yPovHigh;
	CATUInt32 buttonMap;	
};

class CATJoystick
{
	public:
				
		// Axis types for input device
		enum AXIS_TYPE
		{
			X_AXIS		= 0x00000001,
			X_AXIS_LOW	= 0x00000002,
			X_AXIS_HIGH = 0x00000004,
			Y_AXIS		= 0x00000008,
			Y_AXIS_LOW	= 0x00000010,
			Y_AXIS_HIGH = 0x00000020,
			Z_AXIS		= 0x00000040,
			Z_AXIS_LOW	= 0x00000080,
			Z_AXIS_HIGH = 0x00000100,
			X_ROT	    = 0x00000200,
            X_ROT_LOW   = 0x00000400,
            X_ROT_HIGH  = 0x00000800,
			Y_ROT	    = 0x00001000,
            Y_ROT_LOW   = 0x00002000,
            Y_ROT_HIGH  = 0x00004000,
            Z_ROT       = 0x00008000,
			Z_ROT_LOW   = 0x00010000,
			Z_ROT_HIGH  = 0x00020000,
            X_POV       = 0x00040000,
            X_POV_LOW   = 0x00080000,
            X_POV_HIGH  = 0x00100000,
            Y_POV       = 0x00200000,
            Y_POV_LOW   = 0x00400000,
            Y_POV_HIGH  = 0x00800000
		};

		CATJoystick();
		virtual ~CATJoystick();		


		// Retrieve bitmapped valid axis
		virtual CATUInt32 GetValidAxis();

		// Configure the joystick(s)
		virtual bool		    GetStatus(CATJOYSTICKSTRUCT& status) = 0;
        virtual bool            Configure(HWND hwnd)                 = 0;
		
		static  const CATWChar*  GetAxisName(AXIS_TYPE axisType);

	protected:
		CATUInt32	fValidAxis;
		
};

#endif // _CATJoystick_H_