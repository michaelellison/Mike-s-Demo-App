/// \file CATCmdLine.cpp
/// \ingroup CAT
/// \brief Utility class for command-line parsing
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATCmdLine.h"

CATCmdArg::CATCmdArg(CATWChar cmdSwitch)
{
    fSwitch = cmdSwitch;
    fArg    = 0;
}

CATCmdArg::~CATCmdArg()
{
    if (fArg)
    {
        delete [] fArg;
        fArg = 0;
    }
}

void CATCmdArg::SetArg( const CATWChar* argument )
{
    if (fArg)
    {
        delete [] fArg;
        fArg = 0;
    }

    if (argument)
    {
        fArg = new CATWChar[wcslen(argument)+1];
        wcscpy(fArg,argument);
    }
}

const CATWChar* CATCmdArg::GetArg()
{
    return fArg;
}

CATWChar CATCmdArg::GetSwitch()
{
    return fSwitch;
}


CATCmdLine::CATCmdLine()
{
    fSwitchFlags = 0;
    fStringTable = 0;
    fPrefaceId   = CAT_STR_EMPTY;
    fInitialized = false;
}

CATCmdLine::~CATCmdLine()
{
    if (fInitialized)
    {
        Uninitialize();
    }
}

CATResult CATCmdLine::Initialize( CATResult             prefaceId,                                         
                                  const CATCMDLINEARG*  cmdTable,                                  
                                  CATStringTable*       stringTable,
                                  const char*           languageId)
{
    if (fInitialized)
    {
        Uninitialize();
    }

    fStringTable = stringTable;
    fPrefaceId   = prefaceId;

    // Copy command table into local vector
    if (cmdTable != 0)
    {
        CATCMDLINEARG* cmdTablePtr = (CATCMDLINEARG*)&cmdTable[0];
        while (cmdTablePtr->CmdSwitch != (CATWChar)-1)
        {
            fCmdTable.push_back(*cmdTablePtr);
            // Add verification here
            ++cmdTablePtr;
        }
    }

    fInitialized = true;
    return CATRESULT(CAT_SUCCESS);
}

void CATCmdLine::Uninitialize()
{
    fCmdTable.clear();

    fStringTable = 0;
    fPrefaceId   = CAT_STR_EMPTY;

    // Clear operands list
    CATInt32 i;
    CATInt32 numEntries = (CATInt32)fOperands.size();        
    for (i = 0; i < numEntries; i++)
    {
        delete [] fOperands[i];
    }
    fOperands.clear();

    // Clear arguments list
    numEntries = (CATInt32)fArguments.size();
    for (i = 0; i < numEntries; i++)
    {
        delete fArguments[i];
    }
    fArguments.clear();    


    fInitialized = false;
}

// Parse a command line.
// Parsed data will overwrite previously parsed information.
CATResult CATCmdLine::Parse( CATInt32              argc, 
                             CATWChar**            argvw)                           
{    
    fSwitchFlags = 0;
    CATResult  parseValid = CATRESULT(CAT_SUCCESS);
    CATCmdArg* curCmd = 0;

    for (CATInt32 i = 0; i < argc; i++)
    {   
        if (curCmd)
        {
            // Last was a switch with an argument.
            curCmd->SetArg(argvw[i]);
            curCmd = 0;
        }
        else if ((*argvw[i] == '/') || (*argvw[i] == '-'))
        {
            // It's a switch (or multiple switches)
            CATInt32 numSwitches = (CATInt32)wcslen(argvw[i]);
            if (numSwitches > 1)
            {
                // allow multiple switches w/o arguments
                for (CATInt32 switchIdx = 1; switchIdx < numSwitches; switchIdx++)
                {
                    CATWChar curSwitch = argvw[i][switchIdx];
                    curCmd = new CATCmdArg(curSwitch);
                    fArguments.push_back(curCmd);

                    // If this switch has an argument, then leave it active.
                    // Otherwise, reset.  Also OR in switch flags, if present.
                    for (CATInt32 cmdNum = 0; 
                         cmdNum < (CATInt32)fCmdTable.size(); ++cmdNum)
                    {
                        if (fCmdTable[cmdNum].CmdSwitch == curSwitch)
                        {
                            fSwitchFlags |= fCmdTable[cmdNum].SwitchFlag;

                            if (!fCmdTable[cmdNum].TakesArg)
                                curCmd = 0;
                            break;
                        }
                    }
                }
            }
            else
            {
                // just a slash or minus by itself.
                curCmd = 0;
                if (CATSUCCEEDED(parseValid))
                    parseValid = CATRESULT(CAT_ERR_CMD_INVALID_SWITCH);
            }
        }
        else
        {
            // It's an operand. Just push it on list.
            CATWChar* newOperand = new CATWChar[wcslen(argvw[i])+1];
            wcscpy(newOperand,argvw[i]);
            fOperands.push_back(newOperand);

            // Look for operand in table, set switch flags if it 
            // has them.
            CATInt32 opNum    = (CATInt32)fOperands.size() - 1; // Current operand index
                                                                // from command line.
            CATInt32 curOpNum = 0;
            
            for (CATInt32 cmdNum = 0; 
                 cmdNum < (CATInt32)fCmdTable.size(); ++cmdNum)
            {
                if (fCmdTable[cmdNum].CmdSwitch == 0)
                {
                    if (opNum == curOpNum)
                    {
                        fSwitchFlags |= fCmdTable[cmdNum].SwitchFlag;
                        break;
                    }
                    else
                    {
                        curOpNum++;
                    }
                }
            }
            
        }     
    }
    
    if (curCmd != 0)
    {
        // Command didn't receive argument.
        parseValid = CATRESULT(CAT_ERR_CMD_SWITCH_NO_ARG);
    }

    if (CATFAILED(parseValid))
        return parseValid;

    // Validate parameters
    //  Check all required parameters to be sure they're there
    //  Check all exclusive sets to be sure we've got just one (or none if optional)
    //  in each.
    //  Check received switches for unknown ones
    
    /// \todo finish validation of parameters for CATCmdLine
    size_t    numCmds  = fCmdTable.size();
    size_t    cmdIndex = 0;    
    CATUInt32 curOperand = 0;
    CATUInt32 lastValidCmdGroup = 0;

    for (cmdIndex = 0; cmdIndex < numCmds; cmdIndex++)
    {
        if (fCmdTable[cmdIndex].Required)
        {
            if (fCmdTable[cmdIndex].CmdGroup != 0)
            {
                CATInt32 curGroup = fCmdTable[cmdIndex].CmdGroup;
                // If we haven't already validated this command group...
                if (curGroup != lastValidCmdGroup)
                {
                    // we can assume we're at the first entry in the command
                    // group.
                    CATUInt32 numFound = 0;
                    for (size_t i = 0; i < numCmds; i++)
                    {
                        if (fCmdTable[i].CmdGroup == curGroup)
                        {
                            if (fCmdTable[i].CmdSwitch != 0)
                            {
                                if (this->IsSwitchSet(fCmdTable[i].CmdSwitch))
                                    numFound++;
                            }
                            else
                            {    
                                // Operands really don't make sense to be
                                // in exclusive groups, since there's
                                // no way to differentiate. 
                                if (this->GetOpByIndex(curOperand))
                                {
                                    if (numFound == 0)
                                        numFound++;
                                }
                            }
                        }
                    }

                    if (numFound == 1)
                    {
                        lastValidCmdGroup = curGroup;
                    }
                    else if (numFound == 0)
                    {
                        parseValid = CATRESULT(CAT_ERR_CMDREQ_GROUP);
                    }
                    else // numFound > 1
                    {
                        parseValid = CATRESULT(CAT_ERR_CMDREQ_EXCLUSIVE_GROUP);
                    }
                }                
            }
            else
            {
                if (fCmdTable[cmdIndex].CmdSwitch == 0)
                {
                    // it's an operand. Do we have it?
                    if (0 == this->GetOpByIndex(curOperand))
                    {
                        parseValid = CATRESULT(CAT_ERR_CMDREQ_OP);
                        break;
                    }
                }
                else
                {
                    // It's a required switch, check it.
                    if (!this->IsSwitchSet(fCmdTable[cmdIndex].CmdSwitch))
                    {
                        parseValid = CATRESULT(CAT_ERR_CMDREQ_SWITCH);
                        break;
                    }
                }
            }
        }

        // Track operand index
        if (fCmdTable[cmdIndex].CmdSwitch == 0)
            curOperand++;
    }

    return parseValid;
}


CATResult CATCmdLine::PrintUsage(CATInt32 maxColumns)
{    
    CATResult result = CAT_SUCCESS;

    if (!fStringTable)
    {
        return CATRESULT(CAT_ERR_CMD_NO_STRINGTABLE);
    }

    const CATWChar* preface = fStringTable->GetString(fPrefaceId);
    
    if (preface)
        wprintf(L"%s\n",preface);

    wprintf(L"\n%s\n\n",fStringTable->GetString(CAT_STR_USAGE));

    int curColumn   = 0;
    int startColumn = 4;

    // Validity checks?


    size_t    numCommands = fCmdTable.size();
    CATInt32* cmdLengths  = new CATInt32[numCommands+1];
    CATUInt32          i  = 0;

    // Retrieve command print lengths
    for (i = 0; i < (CATUInt32)numCommands; ++i)
        cmdLengths[i] = GetCmdLength(i);
    
    // Print each command in the command line table
    for (i = 0; i < (CATUInt32)numCommands; ++i)
    {
        // Wrap now if we need to
        if (curColumn + cmdLengths[i] >= 80)
        {
            curColumn = startColumn;
            wprintf(L"\n");
            wprintf(L"%*c",startColumn-1,0x20);
        }
        
        // Print command, track column
        CATInt32 cmdLength = PrintCmdEntry(i);
        CATASSERT(cmdLength == cmdLengths[i], "ERROR: GetCmdLength() and PrintCmdEntry() don't match!");
        curColumn += cmdLength;

        if (curColumn < 79)
        {
            curColumn++;
            wprintf(L" ");
        }
    }
    
    if (cmdLengths)
    {
        delete [] cmdLengths;
        cmdLengths = 0;
    }
    
    wprintf(L"\n\n");
    
    // find max length of operands and switch/args
    CATInt32 maxArgLen = 0;
    for (i = 1; i < numCommands; i++)
    {
        const CATWChar* opDesc = fStringTable->GetString (fCmdTable[i].ArgOptDescId);
        
//        const CATWChar* cmdDesc = fStringTable->GetString(fCmdTable[i].DescriptionId);
        
        CATInt32 argLen = 0;
        if (fCmdTable[i].CmdSwitch == 0)
        {
            argLen = startColumn + (CATInt32)(opDesc?wcslen(opDesc):5);
        }
        else
        {
            if (!fCmdTable[i].TakesArg)
                argLen = startColumn+2;
            else
            {
                argLen = startColumn + 3 + (CATInt32)(opDesc?wcslen(opDesc):5);
            }
        }

        if (argLen > maxArgLen)
            maxArgLen = argLen;
    }

    // print switch documentation
    for (i = 1; i < numCommands; ++i)
    {
        const CATWChar* cmdDesc = fStringTable->GetString(fCmdTable[i].DescriptionId);

        const CATWChar* argDesc = fStringTable->GetString(fCmdTable[i].ArgOptDescId);

        if (fCmdTable[i].CmdSwitch != 0)
        {
            CATASSERT(cmdDesc != 0, "Commands MUST be documented.");
            if (fCmdTable[i].TakesArg == 0)
            {
                curColumn = wprintf(L"%*c-%c",startColumn-1, 0x20,
                                              fCmdTable[i].CmdSwitch);
                
                if (curColumn < maxArgLen)
                    wprintf(L"%*c",(maxArgLen - curColumn),0x20);

                wprintf(L" %s\n", cmdDesc?cmdDesc:L"EMPTY");

            }
            else
            {
                CATASSERT(argDesc != 0, "Switch arguments MUST be documented if present.");
                curColumn = wprintf(L"%*c-%c %s",startColumn-1, 0x20,
                                                 fCmdTable[i].CmdSwitch, 
                                                 argDesc?argDesc:L"EMPTY");                                         

                if (curColumn < maxArgLen)
                    wprintf(L"%*c",(maxArgLen - curColumn),0x20);

                wprintf(L" %s\n", cmdDesc?cmdDesc:L"EMPTY");
            }
        }
    }

    // print operand documentation
    for (i = 1; i < numCommands; ++i)
    {
        const CATWChar* opDesc = fStringTable->GetString (fCmdTable[i].ArgOptDescId);
        
        const CATWChar* cmdDesc = fStringTable->GetString(fCmdTable[i].DescriptionId);

        if (fCmdTable[i].CmdSwitch == 0)
        {
            CATASSERT(opDesc  != 0, "Operands MUST have short description name.");
            CATASSERT(cmdDesc != 0, "Operands MUST be documented if present.");
            
            curColumn = wprintf(L"%*c%s", startColumn-1, 0x20,
                                          opDesc?opDesc:L"EMPTY");
            
            if (curColumn < maxArgLen)
                wprintf(L"%*c",(maxArgLen - curColumn),0x20);
            
            wprintf(L" %s\n", cmdDesc?cmdDesc:L"EMPTY");
        }
    }

    wprintf(L"\n");

    return result;
}

CATInt32 CATCmdLine::PrintCmdEntry( CATUInt32 index)
{
    CATInt32 length = 0;

    bool isFirst    = false;
    bool isLast     = false;        
    bool inGroup    = IsCmdInGroup(index,isFirst,isLast);
    bool isRequired = fCmdTable[index].Required;

    if (isFirst)
    {
        // First in group, mark group
        length += wprintf(L"%c",isRequired?'(':'[');
    }
    else if ((!inGroup) && (!isRequired))
    {
        // Mark as optional if not in group
        length += wprintf(L"[");
    }

    if (fCmdTable[index].CmdSwitch != 0)
    { 
        // It's a switch
        length += wprintf(L"-%c",fCmdTable[index].CmdSwitch);
        if (fCmdTable[index].TakesArg)
        {
            // With an argument
            const CATWChar* argstr = fStringTable->GetString(fCmdTable[index].ArgOptDescId);
            CATASSERT(argstr != 0, "If a command takes an argument, you must provide a valid string ID for it.");
            length += wprintf(L" %s",(argstr)?argstr:L"EMPTY");
        }
    }
    else
    { 
        // It's an operand
        const CATWChar* argstr = fStringTable->GetString(fCmdTable[index].ArgOptDescId);
        CATASSERT(argstr != 0, "If a command takes an argument, you must provide a valid string ID for it.");
        length += wprintf(L"%s",(argstr)?argstr:L"EMPTY");
    }

    if (isLast)
    {
        // First in group, mark group
        length += wprintf(L"%c",isRequired?')':']');
    }
    else if (inGroup)
    {
        length += wprintf(L" |");
    }
    else if ((!inGroup) && (!isRequired))
    {
        // Mark as optional if not in group
        length += wprintf(L"]");
    }

    return length;
}
CATInt32 CATCmdLine::GetCmdLength( CATUInt32 index)
{
    CATInt32 cmdLength = 0;

    bool isFirst    = false;
    bool isLast     = false;        
    bool inGroup    = IsCmdInGroup(index,isFirst,isLast);
    bool isRequired = fCmdTable[index].Required;

    if (isFirst)
    {
        cmdLength++;
        // curColumn += wprintf(L"%c",isRequired?'(':'[');
    }
    else if ((!inGroup) && (!isRequired))
    {
        cmdLength++;
        // curColumn += wprintf(L"[");
    }

    if (fCmdTable[index].CmdSwitch != 0)
    { 
        // It's a switch
        
        cmdLength += 2;        
        // curColumn += wprintf(L"-%c",fCmdTable[index].CmdSwitch);
        
        if (fCmdTable[index].TakesArg)
        {
            // With an argument
            const CATWChar* argstr = fStringTable->GetString(fCmdTable[index].ArgOptDescId);
            CATASSERT(argstr != 0, "If a command takes an argument, you must provide a valid string ID for it.");
            
            cmdLength += (CATInt32)((argstr)?(wcslen(argstr) + 1):5);
            //curColumn += wprintf(L" %s",(argstr)?argstr:L"EMPTY");
        }
    }
    else
    { 
        // It's an operand
        const CATWChar* argstr = fStringTable->GetString(fCmdTable[index].ArgOptDescId);
        CATASSERT(argstr != 0, "If a command takes an argument, you must provide a valid string ID for it.");
        
        cmdLength += (CATInt32)((argstr)?(wcslen(argstr)):5);
        //curColumn += wprintf(L"%s",(argstr)?argstr:L"EMPTY");
    }

    if (isLast)
    {
        // First in group, mark group
        cmdLength++;
        //curColumn += wprintf(L"%c",isRequired?')':']');
    }
    else if (inGroup)
    {
        cmdLength+=2;
        //curColumn += wprintf(L" |");
    }
    else if ((!inGroup) && (!isRequired))
    {
        // Mark as optional if not in group
        cmdLength++;
        //curColumn += wprintf(L"]");
    }


    return cmdLength;
}

CATBool CATCmdLine::IsCmdInGroup( CATUInt32 index,
                                  CATBool& isFirst,
                                  CATBool& isLast)
{
    isFirst = false;
    isLast  = false;

    CATUInt32 numCmds = (CATUInt32)fCmdTable.size();
    if (numCmds == 1)
        return false;

    CATASSERT(index < numCmds,"Invalid command index.");
    if (index >= numCmds)
        return false;

    CATInt32 cmdGroup = fCmdTable[index].CmdGroup;
    if (cmdGroup == 0)
        return false;

    if (index == 0)
    {
        if (fCmdTable[index+1].CmdGroup == cmdGroup)
        {
            isFirst = true;
            return true;
        }
        return false;
    }

    if (index == numCmds - 1)
    {
        if (fCmdTable[index-1].CmdGroup == cmdGroup)
        {
            isLast = true;
            return true;
        }
        return false;
    }
    
    if (fCmdTable[index-1].CmdGroup == cmdGroup)
    {
        isFirst = false;
        isLast  = (fCmdTable[index+1].CmdGroup != cmdGroup);
        return true;
    }
    else
    {
        if (fCmdTable[index+1].CmdGroup == cmdGroup)
        {
            isFirst = true;
            isLast  = false;
            return true;
        }
    }

    return false;
}


CATBool CATCmdLine::IsSwitchSet(CATWChar cmdSwitch)
{
    size_t numArgs = fArguments.size();
    for (size_t i = 0; i < numArgs; i++)
    {
        if (fArguments[i]->GetSwitch() == cmdSwitch)
            return true;
    }
    return false;
}

const CATWChar* CATCmdLine::GetArgument(CATWChar cmdSwitch)
{
    size_t numArgs = fArguments.size();
    for (size_t i = 0; i < numArgs; i++)
    {
        if (fArguments[i]->GetSwitch() == cmdSwitch)
            return fArguments[i]->GetArg();
    }
    return 0;
}

CATUInt32 CATCmdLine::GetArgUInt(CATWChar cmdSwitch)
{
    size_t numArgs = fArguments.size();
    for (size_t i = 0; i < numArgs; i++)
    {
        if (fArguments[i]->GetSwitch() == cmdSwitch)
        {
            const CATWChar* arg = fArguments[i]->GetArg();
            if (!arg)
                return 0;

            CATWChar* endPtr = 0;
            return (CATUInt32)wcstoul(arg,&endPtr,0);
        }
    }
    return 0;
}

CATInt32 CATCmdLine::GetArgInt(CATWChar cmdSwitch)
{
    size_t numArgs = fArguments.size();
    for (size_t i = 0; i < numArgs; i++)
    {
        if (fArguments[i]->GetSwitch() == cmdSwitch)
        {
            const CATWChar* arg = fArguments[i]->GetArg();
            if (!arg)
                return 0;

            CATWChar* endPtr = 0;
            return (CATUInt32)wcstol(arg,&endPtr,0);
        }
    }
    return 0;
}

CATUInt32 CATCmdLine::GetNumOps()
{
    return (CATUInt32)fOperands.size();
}

const CATWChar* CATCmdLine::GetOpByIndex(CATUInt32 index)
{
    if (index >= (CATUInt32)fOperands.size())
        return 0;

    return fOperands[index];
}

const CATWChar* CATCmdLine::GetOpByDescId(CATResult descId)
{
    // Find operand index
    size_t index = (size_t)-1;
    size_t numCmds = fCmdTable.size();
    size_t opNum   = 0;
    for (size_t i = 0; i < numCmds; ++i)
    {
        if (fCmdTable[i].DescriptionId == descId)
        {
            index = i;
            break;
        }

        if (fCmdTable[i].CmdSwitch == 0)
            opNum++;
    }
    
    // Description not found?
    if (index == (size_t)-1)
        return 0;
    
    if (opNum >= (CATUInt32)fOperands.size())
        return 0;

    return fOperands[opNum];
}

CATUInt32 CATCmdLine::GetSwitchFlags()
{
    return fSwitchFlags;
}

CATResult CATCmdLine::RunParsedCallbacks(void* cbParam)
{
    CATResult result = CAT_SUCCESS;

    // Call callbacks if validated for all present switches / operands
    size_t cmdIndex = 0;
    size_t numCmds  = fCmdTable.size();

    for (cmdIndex = 0; cmdIndex < numCmds; cmdIndex++)
    {            
        if (fCmdTable[cmdIndex].Callback != 0)
        {
            CATWChar cmdSwitch = fCmdTable[cmdIndex].CmdSwitch;
            if (cmdSwitch != 0)
            {
                if (IsSwitchSet(cmdSwitch))
                {
                    result = fCmdTable[cmdIndex].Callback(this,cbParam);
                    if (CATFAILED(result))
                        return result;
                }
            }
            else
            {
                const CATWChar* operand = GetOpByDescId(fCmdTable[cmdIndex].DescriptionId);
                if (operand)
                {
                    result = fCmdTable[cmdIndex].Callback(this,cbParam);

                    if (CATFAILED(result))
                        return result;
                }
            }
        }
    }
    
    return result;
}