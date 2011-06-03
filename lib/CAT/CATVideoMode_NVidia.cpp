/// \file CATVideoMode_Nvidia.cpp
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
#ifdef _WIN32

// Get rid of non-ANSI compatible warnings.
#ifndef _CRT_SECURE_NO_WARNINGS
	#if _MSC_VER >= 13
	    #define _CRT_SECURE_NO_WARNINGS
	#endif
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501
#endif 

#include <windows.h>
#include "CATVideoMode_Nvidia.h"



// If you need more data / information about this struct, 
// get NVidia's SDK. We're currently only using a tiny 
// fraction of the fields, however, so it seems like 
// severe overkill.
struct DispInfo_NVidia
{
    DWORD Size;                     ///< sizeof(DispInfo_NVidia)                                    
    DWORD InputMask1;               ///< Input field mask (1)
    DWORD OutputMask1;              ///< Output field mask (1)
    DWORD InputMask2;               ///< Input field mask (2)
    DWORD OutputMask2;              ///< Output field mask (2)    
    char  DisplayName[MAX_PATH];    ///< Windows display name.
    char  AdapterName[256];         ///< Adapter name
    char  DriverVersion[64];        ///< Version string
    DWORD NVidiaLinkMode;           ///< NVidia link mode (single, dual, 
                                    ///< clone, hspan, vspan)
    DWORD DisplayNumber;            ///< Display index + 1
    int   DisplayHead;              ///< Display number on adapter
    BOOL  Primary;                  ///< TRUE if primary monitor
    char  FriendlyName[256];        ///< Friendly name (usually monitor type)
    char  Vendor[256];              ///< Display Vendor
    char  Model[256];               ///< Display Model
    char  Generic[256];             ///< Generic name
    
    DWORD UniqueId;
    DWORD DisplayType;
    DWORD DispWidth_mm;
    DWORD DispHeight_mm;
    float DisplayGamma;
    
    DWORD OptimalWidth;             ///< Optimal width
    DWORD OptimalHeight;            ///< Optimal height
    DWORD OptimalRefresh;           ///< Optimal refresh rate
    DWORD MaxSafeWidth;             ///< Max safe width
    DWORD MaxSafeHeight;            ///< Max safe height
    DWORD MaxSafeRefresh;           ///< Max safe refresh rate
    DWORD BitPerPixel;              ///< Current Color depth
    DWORD Width;                    ///< Current Width (including pannable)
    DWORD Height;                   ///< Current Height (including pannable)
    DWORD Refresh;                  ///< Current Refresh rate
    RECT  DisplayRect;              ///< Current Display Rect 
    
    DWORD VisibleWidth;             ///< Current Visible Width 
                                    ///< (excludes pannable)

    DWORD VisibleHeight;            ///< Current Visible Height

    DWORD Rotation;                 ///< Current Rotation in degrees
    
    unsigned char OtherFields[588]; ///< Additional information fields that
                                    ///< we're not currently using.
};

// Haven't found a clean C api for rotation - think it may have to be done in
// dtcfgex().  Yuck.  If anyone finds differently, let me know!
typedef DWORD (WINAPI* FuncNvdtcfgex)( LPSTR lpszCmdLine);

typedef BOOL  (WINAPI *FuncNvGetDisplayInfo)   
                  ( char*            userDisplay,
                    DispInfo_NVidia* nvDispInfo);

typedef BOOL  (_cdecl *FuncNvSelectDispDev)
                  ( CVMUInt32        nWindowsMonitorNumber);

typedef BOOL  (WINAPI* FuncNvCplRefresh)
                  ( CVMUInt32        flags);


static HMODULE gNVidiaCPL = 0;

FuncNvdtcfgex           nvidia_dtcfgex        = 0;
FuncNvGetDisplayInfo    nvidia_GetDisplayInfo = 0;
FuncNvSelectDispDev     nvidia_SelectDisplay  = 0;
FuncNvCplRefresh        nvidia_Refresh        = 0;

void RefreshNVidia(int val)
{
   if (GetNVidiaModule() && nvidia_Refresh)
        nvidia_Refresh(val);
}

/// Had some problems unloading the NVidia DLL each call (e.g. it would crash).
/// Might be because I was also trying to rotate low res modes at the time,
/// which also crashes. At anyrate, need to revisit this. Right now we're
/// loading the DLL and never disposing of it.
///
/// This leaks one module handle @ exit of app on NVidia machines as a result.
HMODULE GetNVidiaModule()
{
    if (gNVidiaCPL)
        return gNVidiaCPL;

    gNVidiaCPL = LoadLibrary(L"nvcpl.dll");

    nvidia_dtcfgex = (FuncNvdtcfgex)
        ::GetProcAddress(gNVidiaCPL,"dtcfgex");
    
    nvidia_GetDisplayInfo = (FuncNvGetDisplayInfo)
        ::GetProcAddress(gNVidiaCPL,"NvGetDisplayInfo");
    
    nvidia_SelectDisplay  = (FuncNvSelectDispDev)                          
        ::GetProcAddress(gNVidiaCPL,"NvSelectDisplayDevice");
    
    nvidia_Refresh = (FuncNvCplRefresh)
        ::GetProcAddress(gNVidiaCPL,"NvCplRefreshConnectedDevices");

    return gNVidiaCPL;
}


bool GetNVidiaModes(CVM_INFO_ADAPTER&              info,                     
                    std::vector<CVM_INFO_ADAPTER>& /*modeList*/)
{
    HMODULE nvidiaDLL = GetNVidiaModule();
    if (!nvidiaDLL)
        return false;

    // Note: old drivers may not support... for my needs, assuming
    //       user has current (or at least modern) drivers. -mae
    if (!nvidia_SelectDisplay( info.DisplayNumber))
    {
        return false;
    }

    char userDisplay[32];
    _itoa(info.DisplayNumber+1,userDisplay,10);

    DispInfo_NVidia dispInfo;
    memset(&dispInfo,0,sizeof(DispInfo_NVidia));
    
    dispInfo.Size = sizeof(DispInfo_NVidia);
    dispInfo.InputMask1 = (DWORD)-1;
    dispInfo.InputMask2 = (DWORD)-1;
    
    if (!nvidia_GetDisplayInfo(userDisplay,&dispInfo))
    {
        return false;
    }

    // Copy out information that NVidia has for current mode
    ::MultiByteToWideChar( 0,
                           0,
                           dispInfo.DriverVersion,
                           -1,
                           info.DriverVersion,
                           kCVM_VER_MAX);

    ::MultiByteToWideChar( 0,
                           0,
                           dispInfo.DisplayName,
                           -1,
                           info.DisplayType,
                           kCVM_DISPLAY_MAX);
    
    switch (dispInfo.NVidiaLinkMode)
    {
        case 1: // clone
        case 2: // hspan
        case 3: // vspan
            info.LinkedDisplay = true;
            break;
            break;

        default: // unknown
        case 0:  // single
        case 4:  // dualview
            break;
    }     

    switch (dispInfo.Rotation)
    {
        case 0:
            info.Orientation = CVM_ROT_NONE;
            break;
        case 90:
            info.Orientation = CVM_ROT_90;
            break;
        case 180:
            info.Orientation = CVM_ROT_180;
            break;
        case 270:
            info.Orientation = CVM_ROT_270;
            break;
    }
   
    return false;
}

/// Rotates the screen to requested position
CVM_RESULT SetNVidiaRot(CVM_INFO_ADAPTER* info, DEVMODE* devMode)                        
{
    CVMWChar* deviceName  = info->DisplayName;
    CVMInt32  displayNum  = info->DisplayNumber;
    CVM_ORIENT orient     = CVM_ROT_NONE;

    if (devMode == 0)
        return CVM_ERR_INVALID_PARAM;

    if (0 == GetNVidiaModule())
        return CVM_ERROR;

        
    int rotDegrees = 0;
    switch (devMode->dmDisplayOrientation)
    {
        case DMDO_90:
            rotDegrees = 90;
            orient = CVM_ROT_90;
            if ((devMode->dmPelsHeight < 1024) || (devMode->dmPelsWidth < 768))
                return CVM_ERR_NVIDIA_WONT_ROTATE_BELOW_1024_768;
            break;

        case DMDO_180:
            rotDegrees = 180;
            orient = CVM_ROT_180;
            if ((devMode->dmPelsWidth < 1024) || (devMode->dmPelsHeight < 768))
                return CVM_ERR_NVIDIA_WONT_ROTATE_BELOW_1024_768;
            break;

        case DMDO_270:
            rotDegrees = 270;
            orient = CVM_ROT_270;
            if ((devMode->dmPelsHeight < 1024) || (devMode->dmPelsWidth < 768))
                return CVM_ERR_NVIDIA_WONT_ROTATE_BELOW_1024_768;
            break;

        default:
            break;
    }

    devMode->dmDisplayOrientation = 0;
    devMode->dmFields &= ~DM_DISPLAYORIENTATION;

    HMODULE nvcplDLL = GetNVidiaModule();
    if (!nvcplDLL)
    {
        return CVM_ERROR;
    }
    
    wcscpy(devMode->dmDeviceName,deviceName);
    devMode->dmDisplayOrientation = 0;

    if (info->Orientation != orient)
    {
        // Got it. Now rotate display.
        ::OutputDebugString(L"Rotating display...\n");            
            
        char* nvidiaCfg = new char[256];
        sprintf(nvidiaCfg,"rotate %d %d",displayNum+1, rotDegrees);
        nvidia_dtcfgex(nvidiaCfg);

        // Refresh nvidia handler
        nvidia_Refresh(1);

        delete [] nvidiaCfg;

        Sleep(1000);
    }

    return CVM_SUCCESS;
}



#endif //_WIN32