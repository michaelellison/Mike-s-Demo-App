/// \file CATVideoMode_Win32.cpp
/// \brief Video Mode handling for Win32 by Michael Ellison
/// \ingroup CAT
///
///
// The MIT License
//
// Copyright (c) 2008 by Michael Ellison
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
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

#include <algorithm>
#include <windows.h>
#include <time.h>
#include "CATVideoMode.h"
#include "CATVideoMode_NVidia.h"

// The following could be static members of the class, but I don't
// want to clutter the header with platform-specific code and
// it's not yet enough to justify inheriting per platform.

/// Internal callback for enumeration of displays.  
BOOL CALLBACK GetDisplayInfoCB(HMONITOR hMonitor, 
                               HDC      hdcMonitor, 
                               LPRECT   lprcMonitor, 
                               LPARAM   dwData);


// Retrieves information on all the displays.  Mainly just calls
// EnumDisplayMonitors() with GetDisplayInfoCB for the callback.
///
void CATVideoMode::GetAllDisplays(
            std::vector<CVM_INFO_ADAPTER_CURRENT>& displayList,
            bool                                   monitorModes)
{
    RefreshNVidia(1);

    displayList.clear();
    ::EnumDisplayMonitors(0,0,GetDisplayInfoCB,(LPARAM)&displayList);
    for (CVMUInt32 i = 0; i < displayList.size(); i++)
    {
        displayList[i].RawModeList = !monitorModes;
    }
}

// Set the display mode.
CVM_RESULT CATVideoMode::SetDisplayMode(CVMInt32  display,
                                        CVM_INFO* info,
                                        bool      queueSet)                                        
    
{
    CVM_RESULT result = CVM_SUCCESS;

    if (info == 0)
        return CVM_ERR_INVALID_PARAM;

    if (!fInitializedAndClean)
    {
        // For sets, don't force initialization prior to set.
        result = Initialize(fMonitorModesOnly);
        if (CVM_FAILED(result))
            return result;
    }

    wchar_t dbg[100];
    swprintf(dbg,100,L"setting mode on display %d: %dx%d\n",display,info->WidthPixels,info->HeightPixels);
    ::OutputDebugStr(dbg);

    CVMUInt32 startDisplay = (CVMUInt32)display;
    CVMUInt32 endDisplay   = (CVMUInt32)display;

    if (display == -1)
    {
        startDisplay = 0;
        endDisplay = (CVMUInt32)this->fDisplays.size() - 1;
    }
    else if (display >= (CVMInt32)this->fDisplays.size())
    {
        return CVM_ERR_INVALID_DISPLAY;
    }


    for (CVMUInt32 curDisplay =  startDisplay;
                   curDisplay <= endDisplay;
                   curDisplay++)
    {
        DEVMODE devMode;
        memset(&devMode,0,sizeof(DEVMODE));
        devMode.dmSize = sizeof(DEVMODE);

        if (info->BitsPerPixel)
        {
            devMode.dmBitsPerPel        = info->BitsPerPixel;
            devMode.dmFields           |= DM_BITSPERPEL;
        }

        if (info->HeightPixels)
        {
            devMode.dmFields           |= DM_PELSHEIGHT;
            devMode.dmPelsHeight        = info->HeightPixels;
        }

        if (info->WidthPixels)
        {
            devMode.dmFields           |= DM_PELSWIDTH;
            devMode.dmPelsWidth         = info->WidthPixels;
        }

        if (info->RefreshRateHz)
        {
            devMode.dmFields           |= DM_DISPLAYFREQUENCY;
            devMode.dmDisplayFrequency  = info->RefreshRateHz;
        }

        if (info->Orientation != CVM_ROT_ANY)
        {
            devMode.dmFields           |= DM_DISPLAYORIENTATION;

            switch (info->Orientation)
            {
                case CVM_ROT_90:   devMode.dmDisplayOrientation = DMDO_90;
                    break;
                case CVM_ROT_180:  devMode.dmDisplayOrientation = DMDO_180;
                    break;
                case CVM_ROT_270:  devMode.dmDisplayOrientation = DMDO_270;
                    break;
                default:
                case CVM_ROT_NONE: devMode.dmDisplayOrientation = DMDO_DEFAULT;
                    break;
            }
        }
        
        DWORD flags = 0;
        
        if (queueSet)
        {
            // Save settings but don't switch yet if queueSet is true.
            // Rotation is performed first on NVidia (you'll get the mode
            // too if doing multiple channels and they aren't rotated at
            // the same angle yet).
            flags = CDS_UPDATEREGISTRY | CDS_NORESET;
        }
        else
        {
            fInitializedAndClean = false;
        }        

        if (info->Orientation != CVM_ROT_ANY)
        {
            CVM_RESULT tmpResult;
            
            if (this->fDisplays[curDisplay].CardBrand == CVMCARD_NVIDIA)
            {
                // Right now, just use NVidia-specific calls to set the rotation.
                if (!CVM_FAILED(tmpResult = SetNVidiaRot(&fDisplays[curDisplay],
                                                         &devMode)))
                {
                    fInitializedAndClean = false;
                }
                else
                {
                    // Bail with error if we're in the danger area for NVidia,
                    // otherwise continue and try normal method of mode switch.
                    if (tmpResult == CVM_ERR_NVIDIA_WONT_ROTATE_BELOW_1024_768)
                        return tmpResult;
                }
            }
        }
        
        LONG cres = ::ChangeDisplaySettingsEx( 
                                fDisplays[curDisplay].DisplayName,
                                &devMode,
                                0,
                                flags,
                                0);   
       
        switch (cres)
        {
            case DISP_CHANGE_BADDUALVIEW:
                result = CVM_ERR_SET_MODE_DUAL_VIEW;
                break;

            case DISP_CHANGE_BADFLAGS:
                result = CVM_ERR_SET_MODE_BAD_FLAGS;
                break;
            
            case DISP_CHANGE_BADMODE:
                result = CVM_ERR_SET_MODE_INVALID_MODE;
                break;
            
            case DISP_CHANGE_BADPARAM:
                result = CVM_ERR_SET_MODE_BAD_PARAM;
                break;
                    
            case DISP_CHANGE_NOTUPDATED:
                result = CVM_ERR_SET_MODE_REGISTRY;
                break;

            case DISP_CHANGE_RESTART:
                result = CVM_ERR_SET_MODE_REBOOT;
                break;

            case DISP_CHANGE_FAILED:
                result = CVM_ERR_SET_MODE_FAILED;
                break;
        }
    }

    return result;
}

void CATVideoMode::GetAllModes(CVM_INFO_ADAPTER&              info, 
                               std::vector<CVM_INFO_ADAPTER>& modeList,
                               bool                           monitorModes)
{
    CVMInt32 curModeNum = 0;
        
    DEVMODE devMode;
    memset(&devMode,0,sizeof(DEVMODE));
    devMode.dmSize = sizeof(DEVMODE);

    if (info.CardBrand == CVMCARD_NVIDIA)
    {
        // Currently does not actually snag modes - just tweaks the display 
        // info with additional parameters from NVidia API.  Modes won't 
        // enum with NVidia API if you're in DualView mode.
        if (GetNVidiaModes(info, modeList))
            return;
    }

    CVMUInt32 startMode = (CVMUInt32)modeList.size();
    bool      rotModes  = false;

    while (::EnumDisplaySettingsEx(info.DisplayName,
                                   curModeNum,
                                   &devMode,
                                   monitorModes?0:EDS_RAWMODE))
    {
        CVM_INFO_ADAPTER mode = info;

        mode.WidthPixels     = devMode.dmPelsWidth;
        mode.HeightPixels    = devMode.dmPelsHeight;
        mode.BitsPerPixel    = devMode.dmBitsPerPel;
        mode.RefreshRateHz   = devMode.dmDisplayFrequency;        
        mode.RawModeList     = !monitorModes;
        
        switch (devMode.dmDisplayOrientation)
        {
            case DMDO_DEFAULT:  
                mode.Orientation = CVM_ROT_NONE; 
                break;
            
            case DMDO_270:
                mode.Orientation = CVM_ROT_270;  
                rotModes         = true; 
                break;

            case DMDO_180:
                mode.Orientation = CVM_ROT_180;  
                rotModes         = true; 
                break;

            case DMDO_90:       
                mode.Orientation = CVM_ROT_90;   
                rotModes         = true; 
                break;

            default:            
                mode.Orientation = CVM_ROT_ANY;  
                rotModes = true; 
                break;
        }

        modeList.push_back(mode);

        memset(&devMode,0,sizeof(DEVMODE));
        devMode.dmSize = sizeof(DEVMODE);
        curModeNum++;
    }

    // If we didn't get any rotation modes reported, may need to spoof them
    // for brand-specific rotation interfaces.
    if (!rotModes)
    {
        if (info.CardBrand == CVMCARD_NVIDIA)
        {
            CVM_ORIENT curRot = info.Orientation;
            bool whSwapped = ((curRot == CVM_ROT_90) || 
                              (curRot == CVM_ROT_270));

            CVMUInt32 lastModePlusOne = (CVMUInt32)modeList.size();
            for (CVMUInt32 i = startMode; i < lastModePlusOne; i++)
            {
                // Fix original record to match its actual rotation
                modeList[i].Orientation = curRot;

                // Add rotations we're not currently in to list.
                CVM_INFO_ADAPTER unrotInfo = modeList[i];

                if (curRot != CVM_ROT_NONE)
                {
                    if (whSwapped)
                        CVMSwap(unrotInfo.WidthPixels,unrotInfo.HeightPixels);

                    unrotInfo.Orientation = CVM_ROT_NONE;

                    // NVidia reports its lower modes even when rotated, 
                    // despite the fact that it gets unhappy when you actually 
                    // try to do it. In this case, we overwrite the mode with
                    // the unrotated version.  Otherwise, we add the unrotated 
                    // version to the list.
                    if ((unrotInfo.WidthPixels < 1024) || 
                        (unrotInfo.HeightPixels < 768 ))
                        modeList[i] = unrotInfo;
                    else
                        modeList.push_back(unrotInfo);
                }

                if (curRot != CVM_ROT_90)
                {
                    unrotInfo = modeList[i];
                    if (!whSwapped)
                        CVMSwap(unrotInfo.WidthPixels,unrotInfo.HeightPixels);
                    unrotInfo.Orientation = CVM_ROT_90;

                    if ((unrotInfo.WidthPixels >= 768) && 
                        (unrotInfo.HeightPixels >= 1024))
                    {
                        modeList.push_back(unrotInfo);
                    }
                }

                if (curRot != CVM_ROT_180)
                {
                    unrotInfo = modeList[i];

                    if (whSwapped)
                        CVMSwap(unrotInfo.WidthPixels,unrotInfo.HeightPixels);

                    unrotInfo.Orientation = CVM_ROT_180;

                    if ((unrotInfo.WidthPixels >= 1024) && 
                        (unrotInfo.HeightPixels >= 768))
                    {
                        modeList.push_back(unrotInfo);
                    }
                }

                if (curRot != CVM_ROT_270)
                {
                    unrotInfo = modeList[i];
                    if (!whSwapped)
                        CVMSwap(unrotInfo.WidthPixels,unrotInfo.HeightPixels);

                    unrotInfo.Orientation = CVM_ROT_270;

                    if ((unrotInfo.WidthPixels >= 768) && 
                        (unrotInfo.HeightPixels >= 1024))
                    {
                        modeList.push_back(unrotInfo);
                    }
                }
            }
        }
    }
}


BOOL CALLBACK GetDisplayInfoCB(HMONITOR hMonitor, 
                               HDC      /*hdcMonitor*/, 
                               LPRECT   lprcMonitor, 
                               LPARAM   dwData)
{
    std::vector<CVM_INFO_ADAPTER_CURRENT>* displayList = 
        (std::vector<CVM_INFO_ADAPTER_CURRENT>*) dwData;

    CVM_INFO_ADAPTER_CURRENT cvmInfo;
    memset(&cvmInfo,0,sizeof(CVM_INFO_ADAPTER_CURRENT));

    // Get monitor information
    MONITORINFOEX mi;   
    memset(&mi,0,sizeof(MONITORINFOEX));
    mi.cbSize = sizeof(MONITORINFOEX);
    ::GetMonitorInfo(hMonitor,&mi);
 
    wcsncpy(cvmInfo.DisplayName,mi.szDevice,kCVM_DISPLAY_MAX);

    cvmInfo.CVMVersion  = kCVMVersion_Current;
    cvmInfo.CardBrand   = CVMCARD_GENERIC;
    cvmInfo.Left        = lprcMonitor->left;
    cvmInfo.Top         = lprcMonitor->top;
    cvmInfo.WorkLeft    = mi.rcWork.left;
    cvmInfo.WorkTop     = mi.rcWork.top;
    cvmInfo.WorkWidth   = mi.rcWork.right  - mi.rcWork.left;
    cvmInfo.WorkHeight  = mi.rcWork.bottom - mi.rcWork.top;
    

    cvmInfo.LinkedDisplay = false;
    cvmInfo.Primary = (mi.dwFlags & MONITORINFOF_PRIMARY)?true:false;

    DEVMODE activeMode;
    memset(&activeMode,0,sizeof(DEVMODE));
    
    activeMode.dmSize   = sizeof(DEVMODE);
    activeMode.dmFields = DM_DISPLAYORIENTATION | 
                          DM_PELSWIDTH | 
                          DM_PELSHEIGHT|
                          DM_BITSPERPEL|
                          DM_DISPLAYFREQUENCY;

    // Get current settings of the display

    // Windows CE requires the extra flag to get orientation, 
    // and doesn't have EnumDisplaySettingsEx().

#ifdef _WIN32_WCE
    activeMode.dmFields |= DM_DISPLAYQUERYORIENTATION;
    ::EnumDisplaySettings(mi.szDevice,ENUM_CURRENT_SETTINGS,&activeMode);
#else  // _WIN32_WCE
    ::EnumDisplaySettingsEx(mi.szDevice,
                            ENUM_CURRENT_SETTINGS,
                            &activeMode,
                            EDS_RAWMODE);
#endif // _WIN32_WCE

    cvmInfo.WidthPixels     = activeMode.dmPelsWidth;
    cvmInfo.HeightPixels    = activeMode.dmPelsHeight;
    cvmInfo.BitsPerPixel    = activeMode.dmBitsPerPel;
    cvmInfo.RefreshRateHz   = activeMode.dmDisplayFrequency;
    
    switch (activeMode.dmDisplayOrientation)
    {
        case DMDO_DEFAULT:  
            cvmInfo.Orientation = CVM_ROT_NONE; 
            break;
        
        case DMDO_270:
            cvmInfo.Orientation = CVM_ROT_270;              
            break;

        case DMDO_180:
            cvmInfo.Orientation = CVM_ROT_180;              
            break;

        case DMDO_90:       
            cvmInfo.Orientation = CVM_ROT_90;               
            break;

        default:            
            cvmInfo.Orientation = CVM_ROT_ANY;              
            break;
    }

    
    
    // Meh, this is always 0x0401
    // _itow(activeMode.dmDriverVersion,cvmInfo.DriverVersion,0x10);
    
    // Get adapter information
    CVMInt32 curDevice = 0;
    DISPLAY_DEVICE displayDevice;
    memset(&displayDevice,0,sizeof(DISPLAY_DEVICE));
    displayDevice.cb = sizeof(DISPLAY_DEVICE);

    while (::EnumDisplayDevices(0,curDevice,&displayDevice,0))
    {

        if (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
        {
            curDevice++;
            continue;
        }

        if (0 == wcscmp(displayDevice.DeviceName, mi.szDevice))
        {
            wcsncpy( cvmInfo.AdapterName,
                     displayDevice.DeviceString, kCVM_ADAPTER_MAX);
            
            CVMCARDTYPE cardType = 
                CATVideoMode::CheckVendorId(displayDevice.DeviceID);
            
            if (cardType != CVMCARD_UNKNOWN)
            {
                cvmInfo.CardBrand = cardType;
            }

            break;
        }

        curDevice++;
    }

    // Get additional info for active mode from device context for driver
    cvmInfo.BitPlanes = 0;

    HDC curDC = ::CreateDC(0,cvmInfo.DisplayName,0,0);
    if (curDC)
    {
        cvmInfo.BitPlanes = ::GetDeviceCaps(curDC,PLANES);
        DeleteDC(curDC);
    }

    cvmInfo.DisplayNumber = (CVMInt32)displayList->size();    
    displayList->push_back(cvmInfo);

    return TRUE;
}

#define CVM_VIDEO_CHANGE_WM WM_APP + 123
LRESULT CALLBACK VideoChangeWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DISPLAYCHANGE:
            ::PostMessage(hWnd, CVM_VIDEO_CHANGE_WM,0,0);
            break;
        default:
          return DefWindowProc(hWnd,message,wParam,lParam);
    }
    return 0;    
}

// Makes previously set display modes that had the queueSet flag
// set to true take effect simultaneously.
CVM_RESULT CATVideoMode::RealizeDisplayModes(int secondsTimeoutMax)
{
    ::OutputDebugString(L"Entered RealizeDisplayModes.\n");
    // Register a window to get the display change
    MSG msg;
    WNDCLASS msgWndClass;
    memset(&msgWndClass,0,sizeof(msgWndClass));
    HINSTANCE instance = ::GetModuleHandle(0);
    msgWndClass.style           = 0;
    msgWndClass.lpfnWndProc     = VideoChangeWndProc;
    msgWndClass.cbClsExtra      = 0;
    msgWndClass.cbWndExtra      = 0;
    msgWndClass.hInstance       = instance;
    msgWndClass.hIcon           = 0;
    msgWndClass.hbrBackground	= 0;
    msgWndClass.lpszMenuName    = 0;	
    msgWndClass.lpszClassName	= L"ResolutionChangeWnd";	
    RegisterClass(&msgWndClass);

    HWND wnd = ::CreateWindowEx(0,L"ResolutionChangeWnd",
                                  L"EventWnd",0,0,0,0,0,0,0,instance,this);

    if (wnd == 0)
    {
        ::OutputDebugString(L"Failed to create message window for RealizeDisplayModes.\n");
        Sleep(secondsTimeoutMax);
        fInitializedAndClean = false;
        return CVM_SUCCESS;
    }

    while (PeekMessage(&msg, wnd, 0, 0,PM_REMOVE)) 
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    // Realize display changes on each monitor. Allow up to 15 seconds / monitor or until
    // we get back a video changed message.
    HANDLE waitEvent = ::CreateEvent(0,TRUE,FALSE,0);

    //for (CVMUInt32 i = 0; i < this->fDisplays.size(); i++)    
    {
        ::ResetEvent(waitEvent);
        clock_t changeTime = clock();
      
        // Seems to reset on all devices, not just primary... needs more testing to see if we
        // need to loop this on some display setups.
        ::ChangeDisplaySettingsEx(0 /*fDisplays[i].DisplayName*/,0,0,0,0);        
        
        while (::MsgWaitForMultipleObjects(1,&waitEvent,FALSE,1000,QS_ALLEVENTS) != WAIT_OBJECT_0)
        {
            if (clock() - changeTime  > secondsTimeoutMax*CLOCKS_PER_SEC)
            {
                ::OutputDebugStr(L"Timed out waiting for resolution message.\n");
                SetEvent(waitEvent);
                break;
            }
            
            while (PeekMessage(&msg, wnd, 0, 0, PM_REMOVE)) 
            {
                if (msg.message == CVM_VIDEO_CHANGE_WM)                    
                {
                    ::OutputDebugStr(L"Got resolution change message.\n");
                    SetEvent(waitEvent);
                }
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }
    }

    CloseHandle(waitEvent);
    DestroyWindow(wnd);
    UnregisterClass(L"ResolutionChangeWnd",instance);

    // Refresh nvidia info if available.
    RefreshNVidia(2);

    fInitializedAndClean = false;
    return CVM_SUCCESS;
}

// Retrieves a sorted list of display(s) under the specified
// window. Displays containing the largest portion of the
// window will be sorted to the beginning of the list.
CVM_RESULT CATVideoMode::GetDisplaysFromWindow( 
            CVM_WND window,
            std::vector<CVM_INFO_ADAPTER_CURRENT>&  displayList)
{

    RECT rect;
    if (!::GetWindowRect((HWND)window, &rect))
        return CVM_ERR_INVALID_WND;

    CVMRECT cvmRect;
    cvmRect.left   = rect.left;
    cvmRect.top    = rect.top;
    cvmRect.bottom = rect.bottom;
    cvmRect.right  = rect.right;

    return GetDisplaysFromRect(cvmRect, displayList);
}




#endif //_WIN32
