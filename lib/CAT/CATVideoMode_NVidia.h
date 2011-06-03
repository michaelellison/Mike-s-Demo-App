/// \file CATVideoMode_Nvidia.h
/// \brief NVidia specific functions
/// \ingroup CAT
///
///
// The MIT License
//
// Copyright (c) 2008 by Michael Ellison
//
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATVideoMode_NVidia_H_
#define _CATVideoMode_NVidia_H_

#include "CATVideoMode.h"


/// Retrieves a handle to the NVidia dll (nvcpl.dll)
HMODULE GetNVidiaModule();

/// Retrieve NVidia specific mode information regarding the display
/// if available.  
///
/// NOTE: current does NOT retrieve the modes, and always returns 
/// false. It does, however, pull additional information about
/// NVidia cards.
///
/// This is because NVIdia does not currently allow enumeration of
/// video modes via it's enumeration function if the displays are 
/// in DualView mode - which, of course, just happens to be the only
/// one I use at the moment.
///
/// \param  info     Primary display information
/// \param  modeList List of available modes to fill
///
bool GetNVidiaModes(CVM_INFO_ADAPTER&              info,                     
                    std::vector<CVM_INFO_ADAPTER>& modeList);

/// Set video rotation for NVidia adapters.
CVM_RESULT SetNVidiaRot(  CVM_INFO_ADAPTER* info,
                          DEVMODE*          devMode);                           
/// Refresh NVidia info
void RefreshNVidia(int val);



#endif //_CATVideoMode_NVidia_H_