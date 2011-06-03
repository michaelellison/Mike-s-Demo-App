//---------------------------------------------------------------------------
/// \file CATCommand.cpp
/// \brief Generalized command construct for tracking commands
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
#include <memory.h>

#include "CATCommand.h"

CATCommand::CATCommand()
{
    fVal              = 1.0f;
    fCmdString        = "";
    fCmdType          = "";
    fTarget           = "";
    fStrParam         = "";
}

CATCommand::CATCommand(  const CATString& cmdString, 
                       CATFloat32         cmdVal,
                       const CATString& cmdStrParam,
                       const CATString& cmdTarget,
                       const CATString& cmdType)
{
    fCmdType          = cmdType;
    fCmdString        = cmdString;
    fVal              = cmdVal;
    fStrParam         = cmdStrParam;
    fTarget           = cmdTarget;
}

CATCommand::~CATCommand()
{
}

CATCommand& CATCommand::operator=(const CATCommand& cmd)
{
    fCmdType    = cmd.fCmdType;
    fTarget     = cmd.fTarget;
    fCmdString  = cmd.fCmdString;
    fVal        = cmd.fVal;
    fStrParam   = cmd.fStrParam;
    return *this;
}

CATFloat32  CATCommand::GetValue()
{
    return fVal;
}

CATString CATCommand::GetCmdString()
{
    return fCmdString;
}

CATString CATCommand::GetStringParam()
{
    return fStrParam;
}


CATString CATCommand::GetTarget()
{
    return fTarget;
}

CATString CATCommand::GetType()
{
    return fCmdType;
}
