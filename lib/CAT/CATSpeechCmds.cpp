//---------------------------------------------------------------------------
/// \class   CATSpeechCmds.cpp
/// \brief   Speech command input support
/// \ingroup CATGUI
/// 
/// Copyright (c) 2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//
//---------------------------------------------------------------------------

#include "CATSpeechCmds.h"

CATSpeechCmds::CATSpeechCmds()
{
    fRecognizer  = 0;
    fRecoContext = 0;
    fRecoGrammar = 0;
}

CATSpeechCmds::~CATSpeechCmds()
{
}

void CATSpeechCmds::ThreadFunction()
{
    CATResult result = InitializeSAPI();
    if (CATSUCCEEDED(result))
    {

        CATMsgThread::ThreadFunction();

        UninitializeSAPI();
    }    
}

CATResult CATSpeechCmds::InitializeSAPI()
{
    CATResult result = CAT_SUCCESS;
    ::CoInitialize(0);
    /*
    if (CATFAILED( result = ::CoCreateInstance(CLSID_SpSharedRecognizer, 
                                               0,
                                               CLSCTX_INPROC,
                                               IID_SpSharedRecognizer,
                                               &fRecognizer)))
    {
        CoUninitialize();        
        return CATRESULT(CAT_ERR_SPEECH_INIT);
    }

    if (FAILED(fRecognizer->CreateRecoContext(&fRecoContext)))
    {
        Uninitialize();
        return CATRESULT(CAT_ERR_SPEECH_INIT);
    }
    */
    return result;
}

void CATSpeechCmds::UninitializeSAPI()
{
    if (fRecoGrammar)
        fRecoGrammar->Release();
    fRecoGrammar = 0;

    if (fRecoContext)
        fRecoContext->Release();
    fRecoContext = 0;

    if (fRecognizer)
        fRecognizer->Release();
    fRecognizer = 0;

    CoUninitialize();
}

