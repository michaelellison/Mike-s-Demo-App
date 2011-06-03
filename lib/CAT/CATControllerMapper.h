/// \file    CATControllerMapper.h
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
#ifndef  CATControllerMapper_H_
#define  CATControllerMapper_H_

#include "CATInternal.h"
#include "CATJoystick.h"
#include "CATControlMap.h"
#include "CATString.h"
#include "CATMsgThread.h"

// move these to win32 implementation once we've got class developed.
class CATDirectInput;

/// \class CATControllerMapper
/// \brief Maps inputs from a controller to various outputs
/// \ingroup CAT
class CATControllerMapper : public CATMsgThread
{
    public:
        CATControllerMapper();
        virtual ~CATControllerMapper();

        virtual CATResult Initialize();
        virtual CATResult Uninitialize();

        virtual CATResult GetControllerList( std::vector<CATString>& controllerList);
        
        virtual CATResult GetMappingList(    const CATString&        controllerName,
                                             std::vector<CATString>& mappingList);
        
        virtual CATResult StartMapping(      const CATString&        controllerName,
                                             const CATString&        mappingName);
        
        virtual CATResult StopMapping();

    protected:
        virtual  void OnThreadIdle();
        virtual  CATUInt32 OnThreadMessage(CATUInt32 msg, CATUInt32 wParam, CATUInt32 lParam);
        void     OnAxis(CATInt32 val, CATJoystick::AXIS_TYPE axisType);
        void     OnButtonChange(bool state, CATUInt32 button);

        bool            fInitialized;
        CATUInt32       fPollFreq;
        CATJoystick*    fController;    ///< active input controller (eventually needs to be parent class of joystick)
        CATControlMap*  fMap;           ///< active mapping information

        // Move these to win32 implementation once we've got the class developed
        CATDirectInput*  fDirectInput;
        CATJOYSTICKSTRUCT fLastStatus;
        CATUInt32         fValidAxis;
};

#endif // CATControllerMapper_H_
