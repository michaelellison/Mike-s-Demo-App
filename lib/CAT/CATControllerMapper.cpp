/// \file    CATControllerMapper.cpp
/// \brief   Maps inputs from a controller to various outputs
/// \ingroup CAT
///
/// Copyright (c) 2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATControllerMapper.h"
#include "CATDirectInput.h"
#include "CATDIJoystick.h"

CATControllerMapper::CATControllerMapper()
{
    fDirectInput = 0;
    fController  = 0;
    fMap         = 0;
    fPollFreq    = 50;
    fValidAxis   = 0;
    fInitialized = false;
}

CATControllerMapper::~CATControllerMapper()
{
    if (fInitialized)
        (void)Uninitialize();
}

CATResult CATControllerMapper::Initialize()
{
    if (fInitialized)
        return CAT_SUCCESS;
    
    CATResult result;

    fDirectInput = new CATDirectInput();
    result = fDirectInput->Init();
    if (CATFAILED(result))
        Uninitialize();
    else
        fInitialized = true;

    return result;
}

CATResult CATControllerMapper::Uninitialize()
{
    StopMapping();

    if (fDirectInput)
    {
        delete fDirectInput;
        fDirectInput = 0;
    }
    fInitialized = false;
    return CAT_SUCCESS;
}


void JoystickListCB(const CATWChar* name, void* userParam)
{
    std::vector<CATString>* joystickList = (std::vector<CATString>*)userParam;
    joystickList->push_back(name);
}


CATResult CATControllerMapper::GetControllerList( std::vector<CATString>& controllerList )
{
    if (!fInitialized)
        return CATRESULT(CAT_ERR_NOT_INITIALIZED);

    fDirectInput->EnumJoysticks(JoystickListCB,&controllerList);    
    
    return CAT_SUCCESS;
}

CATResult CATControllerMapper::GetMappingList(    const CATString&        controllerName,
                                                  std::vector<CATString>& mappingList)
{
    return CAT_SUCCESS;
}

CATResult CATControllerMapper::StartMapping(      const CATString&        controllerName,
                                                  const CATString&        mappingName)
{
    fController = fDirectInput->CreateJoystick(controllerName);    
    
    if (!fController)
        return CAT_ERROR;

    fValidAxis = fController->GetValidAxis();

    fController->GetStatus(fLastStatus);
    fMap = new CATControlMap();

    this->Start(fPollFreq);

    return CAT_SUCCESS;
}

CATResult CATControllerMapper::StopMapping()
{
    this->Stop();

    if (fController)
    {
        delete fController;
        fController = 0;
    }

    if (fMap)
    {
        delete fMap;
        fMap = 0;
    }
    return CAT_SUCCESS;
}


void CATControllerMapper::OnThreadIdle()
{
    if (fController)
    {
        CATJOYSTICKSTRUCT status;

        if (fController->GetStatus(status))
		{
			for (CATUInt32 i = 0; i < 12; i++)
			{
				if ((status.buttonMap & (1 << i)) != (fLastStatus.buttonMap & (1 << i)))	
				{
					OnButtonChange( 0 != (status.buttonMap & (1 << i)), i);
				}
			}

			// Oh yeah... for mice, axis doesn't need to change for us
			// to update position. doh. Just call direct.
			OnAxis(status.xAxis,    CATJoystick::X_AXIS);
			OnAxis(status.xHigh,    CATJoystick::X_AXIS_HIGH);
			OnAxis(status.xLow,     CATJoystick::X_AXIS_LOW);
			OnAxis(status.xRot,     CATJoystick::X_ROT);
			OnAxis(status.xRotLow,  CATJoystick::X_ROT_LOW);
			OnAxis(status.xRotHigh, CATJoystick::X_ROT_HIGH);
			OnAxis(status.yAxis,    CATJoystick::Y_AXIS);
			OnAxis(status.yHigh,    CATJoystick::Y_AXIS_HIGH);
			OnAxis(status.yLow,     CATJoystick::Y_AXIS_LOW);
			OnAxis(status.yRot,     CATJoystick::Y_ROT);
			OnAxis(status.yRotLow,  CATJoystick::Y_ROT_LOW);
			OnAxis(status.yRotHigh, CATJoystick::Y_ROT_HIGH);
			OnAxis(status.zAxis,    CATJoystick::Z_AXIS);
			OnAxis(status.zHigh,    CATJoystick::Z_AXIS_HIGH);
			OnAxis(status.zLow,     CATJoystick::Z_AXIS_LOW);
			OnAxis(status.zRot,     CATJoystick::Z_ROT);
			OnAxis(status.zRotLow,  CATJoystick::Z_ROT_LOW);
			OnAxis(status.zRotHigh, CATJoystick::Z_ROT_HIGH);

            fLastStatus = status;			
		}

    }
}

CATUInt32 CATControllerMapper::OnThreadMessage(CATUInt32 msg, CATUInt32 wParam, CATUInt32 lParam)
{
    return 0;
}

void CATControllerMapper::OnAxis(CATInt32 val, CATJoystick::AXIS_TYPE axisType)
{
}


void CATControllerMapper::OnButtonChange(bool state, CATUInt32 button)
{		
    
}
