/// \file CATCmdLine.h
/// \brief Utility class for command-line parsing
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATCmdLine_H_
#define _CATCmdLine_H_

#include "CATInternal.h"
#include "CATStringTable.h"

class CATCmdLine;

/// Callback prototype for command line arguments.
///
/// \param  cmdLinePtr   Ptr to calling command line object
/// \param  userParam    User defined param (this-> or similar context)
/// \return CATResult    If a callback returns an error, other callbacks
///                      will not be called.
typedef CATResult (*CATCMDFUNC)(CATCmdLine*      cmdLinePtr,
                                void*            userParam);

/// CATCMDLINEARG is the entry format used for the table that must be created
/// and passed to CATCmdLine::Initialize to enable it to parse a command line.
///
/// Each command entry is one of the following:
///  -# Operand - A command with no switch preceeding it.
///  -# Switch  - A command that starts with a '/' or '-' and is proceeded by
///               a single character. Multiple switches may follow a single
///               '/' or '-' on the command line.
///  -# Switch w/Arg - A switch, like the above, but with an argument after it.
///               The switch and argument must be seperated by a space.
///
/// Tables should begin with an operand for the executable itself, and end 
/// with an entry where CmdSwitch is -1.
///
/// Operands MUST have the following:
///  -# CmdSwitch must be 0
///  -# TakesArg must be false.
///  -# ArgOptDescId must be a short (single word) description of the operand.
///
/// If a switch takes and argument, ArgOptDescId should be valid. Otherwise it should
/// be 0.
///
/// CmdGroup specifies a group of entries that are mutually exclusive.
/// A CmdGroup of 0 indicates that the command is *not* part of a group.
/// Any entries sharing the same CmdGroup value are in the same group. 
///
/// Entries and CmdGroup IDs should be in order.
///
/// If a CmdGroup is required, set the Required flag on all entries. Otherwise,
/// do not set the flag on *any* entries.
///
struct CATCMDLINEARG
{
    CATWChar        CmdSwitch;     ///< 0 indicates operand, -1 indicates end of table.
    CATInt32        CmdGroup;      ///< Specifies mutually exclusive cmd group.
    CATBool         Required;      ///< If true, the argument is required.
    CATBool         TakesArg;      ///< must be false for operands, otherwise true if switch takes arg
    CATCMDFUNC      Callback;      ///< optional callback function
    CATUInt32       SwitchFlag;    ///< optional flag to set for argument, \see CATCmdLine::GetSwitchFlags()
    CATResult       ArgOptDescId;  ///< ID of string for argument/operand description
    CATResult       DescriptionId; ///< ID of string for verbose description of operand/argument
};

/// \class CATCmdArg
/// \brief Helper class for \ref CATCmdLine to hold individual parsed command line
///        arguments and switches.
/// \ingroup CAT
class CATCmdArg
{
    public:
        /// Constructor for CATCmdArg.
        /// \param cmdSwitch Parsed switch value.
        CATCmdArg  (CATWChar cmdSwitch);
       ~CATCmdArg  ( );
        
        /// Sets the argument string value.
        /// \param argument The parsed argument string for the switch.        
        void                SetArg     ( const CATWChar*       argument );

        /// Retrieves the argument string value, if any.
        /// \return  const CATWChar*   String value of argument, or 0 if none.
        const CATWChar*     GetArg     ();

        /// Retrieves the switch character for the argument.
        /// \return CATWChar switch character.
        CATWChar            GetSwitch  ();
    
    protected:
        CATWChar            fSwitch;    ///< Parsed switch
        CATWChar*           fArg;       ///< Argument for switch, if any. 0 if not.
};

/// \class CATCmdLine
/// \brief Generic command line parser
/// \ingroup CAT
///
/// CATCmdLine takes the command line (in argc/argvw format - using Wide Characters!)
/// and parses it out into switches, arguments, and operands based on the passed-in table.
///
/// You can query it after parsing, or just set it up to call your callback functions
/// during the parse.
///
/// To use:
///   -# First call Initialize(), passing the table and app information in.
///   -# Call Parse() with the command information to parse.
///   -# Check the return value from Parse(). If all is good, it will be 0.
///   -# Query the parsed data if desired.
///   -# Optionally, call RunParsedCallbacks() to execute any callbacks
///      for commands received on the command line.  If a callback returns an
///      error, no other callbacks will be called and RunParsedCallbacks() will
///      return that error.
///   -# You may go back to step 1 or 2 if desired to parse additional command lines.
///   -# Call Uninitialize 
///      (optional - will be called on initialized objects during destructor)
///
/// See \CATCMDLINEARG for the format of the table.
class CATCmdLine
{
    public:
                           CATCmdLine ( );
        virtual           ~CATCmdLine ( );

        /// \brief Initialize the command parser.
        ///
        /// \param prefaceId    Preface string to print at start of usage.
        /// \param cmdTable     Application's command line parsing table.
        ///                     This may be built on the fly and deleted after 
        ///                     initialization if desired.        
        /// \param stringTable  Application's string table. This is expected to
        ///                     be around for the lifetime of the command parser object.
        /// \param languageId   Current language Id to use
        /// \return CATResult   CAT_SUCCESS on success.
        CATResult          Initialize  ( CATResult            prefaceId,                                         
                                         const CATCMDLINEARG* cmdTable,                                         
                                         CATStringTable*      stringTable,
                                         const char*          langId = "eng");
        
        /// Uninitializes the command parser.        
        /// This may be done manually, or will be called on destruction if needed.
        void               Uninitialize( );

        /// \brief Parse a command line.
        /// Parsed data will overwrite previously parsed information.
        ///
        /// \param  argc      Number of arguments on command line
        /// \param  argvw     Command line in array form.                
        ///
        /// \todo
        /// Need to add verification of results to Parsing
        CATResult          Parse       ( CATInt32             argc, 
                                         CATWChar**           argvw);
                                         
        /// \brief Execute any callbacks specified by the table on
        /// the most recently parsed data.
        ///
        /// You *must* call Parse() prior to using this function!
        ///
        /// \param  cbParam   Parameter to be sent on function callbacks, if any.
        /// \return CATResult CAT_SUCCESS on success.
        CATResult          RunParsedCallbacks(void*           cbParam = 0);

        /// \brief Prints the usage information for the app based on the table
        /// provided during initialization.
        /// \param maxColumns  Max columns to print (e.g. screen width)
        CATResult          PrintUsage   ( CATInt32            maxColumns = 80);

        /// \brief Returns true if the switch was present on the command line
        ///
        /// \param cmdSwitch   Character switch to check
        /// \return CATBool    true if set on cmd line, false otherwise.
        CATBool            IsSwitchSet  ( CATWChar            cmdSwitch);

        /// \brief Returns the argument (if any) for the switch specified on 
        /// the command line.
        ///
        /// \param  cmdSwitch        Character switch to check
        /// \return const CATWChar*  Ptr to argument or 0 if none.
        const CATWChar*    GetArgument  ( CATWChar            cmdSwitch);

        /// \brief Returns the argument (if any) for the switch specified on
        /// the command line after converting to an unsigned integer.
        ///
        /// Note: will accept hex or octal, per wcstoul.
        ///
        /// \param  cmdSwitch    Character switch to check
        /// \return CATUInt32    Unsigned integer argument. 0 if not found.
        CATUInt32          GetArgUInt   ( CATWChar            cmdSwitch);

        /// \brief Returns the argument (if any) for the switch specified on
        /// the command line after converting to a signed integer.
        ///
        /// Note: will accept hex or octal, per wcstol.
        ///
        /// \param  cmdSwitch    Character switch to check
        /// \return CATInt32     Signed integer argument. 0 if not found.
        CATInt32           GetArgInt    ( CATWChar            cmdSwitch);

        /// \brief Retrieves the number of operands received on the command line.
        /// 
        /// \return CATUInt32    Number of operands (unswitched commands)
        CATUInt32          GetNumOps    ( );

        /// \brief Retrieves an operand's string value by index
        ///
        /// \param  index               Operand index
        /// \return const CATWChar*     Operand value from command line
        const CATWChar*    GetOpByIndex ( CATUInt32           index);


        /// \brief Retrieves an operand's string value by it's DescriptionId
        /// in the command table.
        ///
        /// \param  descId           DescriptionId from command table
        /// \return const CATWChar*  Operand value from command line
        const CATWChar*    GetOpByDescId( CATResult           descId);

        /// \brief Retrieves the flags set by switches/operands that
        /// are present. 
        ///
        /// Values of flags are set in the table in SwitchFlag.
        /// 
        /// \return CATUInt32 All switch flags from command line OR'd together.
        CATUInt32          GetSwitchFlags();
    protected:
        /// \brief Internal function to get the printed length of a command
        /// while printing usage information.
        ///
        /// \param  index     Command index in table
        /// \return CATInt32  Length of text to print command
        /// \sa PrintUsage()
        CATInt32           GetCmdLength ( CATUInt32           index);

        /// \brief Internal function to print a command from the command table.        
        ///
        /// \param  index     Command index in table
        /// \return CATInt32  Length of text to print command
        /// \sa PrintUsage()
        CATInt32           PrintCmdEntry( CATUInt32           index);

        /// \brief Internal function to determine if a command is in a 
        /// mutually exclusive command group, and if so - if it
        /// is the first or last in the group.
        ///
        /// \param   index   Index of the command
        /// \param   isFirst Set to true on return if it is the first
        ///                  command in a group with > 1 members.
        /// \param   isLast  Set to true on return if it is the last
        ///                  command in a group with > 1 members.
        /// \return  CATBool Returns true if it is in a group with > 1 members.
        CATBool            IsCmdInGroup (  CATUInt32          index,
                                           CATBool&           isFirst,
                                           CATBool&           isLast);               
    protected:
        CATUInt32           fSwitchFlags; ///< OR'd combination of all switch flags found during parse
        CATBool             fInitialized; ///< Has Initialize() been called?
        CATStringTable*     fStringTable; ///< String table to get strings from
        CATString           fLangId;      ///< Language Id (CATLANG_EN default)
        CATResult           fPrefaceId;   ///< String id of preface for PrintUsage()
        
        std::vector<CATCMDLINEARG> fCmdTable;    ///< List of commands
        std::vector<CATCmdArg*>    fArguments;   ///< List of parsed arguments
        std::vector<CATWChar*>     fOperands;    ///< List of parsed operands
};

#endif //_CATCmdLine_H_
