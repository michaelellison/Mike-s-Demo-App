//---------------------------------------------------------------------------
/// \class   CATSpeechCmds.h
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
#ifndef _CATSPEECHCMDS_H_
#define _CATSPEECHCMDS_H_

#include "CATInternal.h"
#include "CATMsgThread.h"
#include <sapi.h>

class CATSpeechCmds : public CATMsgThread
{
    public:
        CATSpeechCmds();
        virtual ~CATSpeechCmds();


    protected:
        virtual	void ThreadFunction();
        CATResult InitializeSAPI();
        void      UninitializeSAPI();

        // Mostly windows specific - should back this out into child class
        ISpRecognizer*  fRecognizer;
        ISpRecoContext* fRecoContext;
        ISpRecoGrammar* fRecoGrammar;

};

#endif //_CATSPEECHCMDS_H_