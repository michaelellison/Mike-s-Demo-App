//---------------------------------------------------------------------------
/// \file CATCommand.h
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

#ifndef CATCommand_H_
#define CATCommand_H_

#include "CATInternal.h"
#include "CATString.h"

/// \class CATCommand
/// \brief Generalized command construct for tracking commands
/// \ingroup CATGUI
///
/// Commands represent an action requested by the user.
/// CATCommand represents a command by a command string associated with
/// a floating point value and a string parameter.
///
class CATCommand
{
    public:
        CATCommand();

        /// CATCommand constructor
        ///
        /// Typically, all of these will come from a control and its
        /// skin script.
        ///
        /// \param cmdString - the command string to be sent
        /// \param cmdVal - the value from the control or 
        /// other source sending the command
        /// \param cmdStrParam - a string parameter for the command.
        /// \param cmdTarget - used to target another control specifically 
        CATCommand( const CATString& cmdString, 
                    CATFloat32       cmdVal,
                    const CATString& cmdStrParam = "",
                    const CATString& cmdTarget   = "",
                    const CATString& cmdType     = "");

        /// CATCommand destructor
        virtual ~CATCommand();

        /// CATCommand copy operator
        CATCommand& operator=(const CATCommand& cmd);
        //-------------------------------------------------------------------
        // Command execution info
        //-------------------------------------------------------------------

        /// GetValue() retrieves the value in the command
        /// \return CATFloat32 - floating point value 
        /// (generally of the control sending it)
        CATFloat32     GetValue();

        /// GetCmdString() retrieves the command string itself.
        ///
        /// This string is used to identify the action the application 
        /// should take.
        ///
        /// \return CATString - command string.
        CATString    GetCmdString();

        /// GetStringParam() retrieves the string parameter of the command.
        ///
        /// \return CATString - parameter string for the command, if any.
        CATString    GetStringParam();

        /// GetTarget() retrieves the target of the command.
        /// If it's empty, then the command is generally targetted (all)
        CATString    GetTarget();

        /// GetType() retrieves the type of command.
        /// Generally, this is an empty string.  
        /// In VST apps, "VSTParam" indicates that it is a VST parameter.
        CATString    GetType();

    protected:
        /// Command string to execute
        CATString          fCmdString;

        /// Value for command
        CATFloat32           fVal;

        /// String parameter for command
        CATString          fStrParam;

        CATString          fCmdType;

        /// Optional target of command.
        /// Assumed to be all if not set.
        CATString          fTarget;
};

#endif // CATCommand_H_
