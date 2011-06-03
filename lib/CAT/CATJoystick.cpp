/// \file  CATJoystick.cpp
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
#include "CATJoystick.h"


CATJoystick::CATJoystick()
{
	fValidAxis = 0;
}


CATJoystick::~CATJoystick()
{


}

CATUInt32 CATJoystick::GetValidAxis()
{
	return fValidAxis;
}

const CATWChar* CATJoystick::GetAxisName(CATJoystick::AXIS_TYPE axisType)
{
	CATWChar* axisName = L"Undefined Axis";
	switch (axisType)
	{
		case X_AXIS:		axisName  = L"X"; break;
		case X_AXIS_LOW:	axisName  = L"X (L)";break;
		case X_AXIS_HIGH:   axisName  = L"X (H)";break;
		case Y_AXIS:		axisName  = L"Y";break;
		case Y_AXIS_LOW:	axisName  = L"Y (L)";break;
		case Y_AXIS_HIGH:   axisName  = L"Y (H)";break;
		case Z_AXIS:		axisName  = L"Z";break;
		case Z_AXIS_LOW:	axisName  = L"Z (L)";break;
		case Z_AXIS_HIGH:   axisName  = L"Z (H)";break;
		case X_ROT:	        axisName  = L"rX";break;
		case X_ROT_LOW:	    axisName  = L"rX (L)";break;
		case X_ROT_HIGH:	axisName  = L"rX (H)";break;
		case Y_ROT:     	axisName  = L"rY";break;
		case Y_ROT_LOW: 	axisName  = L"rY (L)";break;
		case Y_ROT_HIGH:	axisName  = L"rY (H)";break;
		case Z_ROT:     	axisName  = L"rZ";break;
		case Z_ROT_LOW: 	axisName  = L"rZ (L)";break;
		case Z_ROT_HIGH:	axisName  = L"rZ (H)";break;
        case X_POV_LOW:     axisName  = L"povX (L)";break;
        case X_POV_HIGH:    axisName  = L"povX (H)";break;
        case Y_POV_LOW:     axisName  = L"povY (L)";break;
        case Y_POV_HIGH:    axisName  = L"povY (H)";break;
	}
	return axisName;
}