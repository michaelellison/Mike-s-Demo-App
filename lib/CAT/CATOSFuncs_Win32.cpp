//---------------------------------------------------------------------------
/// \file CATOSFuncs_Win32.cpp
/// \brief Functions that can be abstracted between platforms directly
/// \ingroup CAT
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------
#include "CATOSFuncs.h"
#include "CATInternal.h"
#include "CATStringTableCore.h"

#include <shellapi.h>

// CATInvalidateRect invalidates a rectangular region within a
// window so that it is drawn on the next paint.
//
// \param window - OS defined window to invalidate
// \param rect   - Rectangle in window to mark as invalid.
void CATInvalidateRect( CATWND window,
                       const CATRect& rect )
{
   RECT wRect = *(RECT*)&(CATRECT)rect;
   if (::IsWindow(window))
   {   
      ::InvalidateRect(window,&wRect,FALSE);   
   }
}

BOOL CALLBACK FindFirstMonitor( HMONITOR hMonitor,
                                HDC      /*hdcMonitor*/,
                                LPRECT   /*lprcMonitor*/,
                                LPARAM   dwData)
{
    HMONITOR *monitor = (HMONITOR*)dwData;
    *monitor = hMonitor;
    return FALSE;
}

CATRect GetPrimaryMonitorRect()
{
    HMONITOR primaryMonitor = 0;
    EnumDisplayMonitors(0,0,FindFirstMonitor,LPARAM(&primaryMonitor));

    MONITORINFO monInfo;   
    memset(&monInfo,0,sizeof(MONITORINFO));
    monInfo.cbSize = sizeof(MONITORINFO);
    ::GetMonitorInfo(primaryMonitor,&monInfo);

    CATRect primaryRect = *(CATRECT*)&monInfo.rcWork;
    return primaryRect;
}

// CATPostQuit() posts a quit message to a GUI interface...
void CATPostQuit( CATInt32 exitCode)
{
   ::PostQuitMessage(exitCode);
}

// CATExecute() executes a shell command.
CATResult CATExecute(const CATWChar* shellCommand, CATWND wnd)
{   
   SHELLEXECUTEINFO si;
   memset(&si,0,sizeof(SHELLEXECUTEINFO));
   si.cbSize = sizeof(SHELLEXECUTEINFO);

   si.hwnd = wnd;
   si.lpVerb = L"open";
   si.lpFile = shellCommand;
   si.nShow = SW_SHOW;
   CATInt32 result = (CATInt32)(UINT_PTR)ShellExecuteEx(&si);
   if (result > 32)
   {
      return CAT_SUCCESS;
   }

   switch (result)
   {
      case SE_ERR_OOM:
      case 0:
         return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
      
      //case SE_ERR_FNF:
      case ERROR_FILE_NOT_FOUND:
         return CATRESULTFILE(CAT_ERR_FILE_DOES_NOT_EXIST, shellCommand);
      
      case ERROR_BAD_FORMAT:
         return CATRESULTFILE(CAT_ERR_FILE_CORRUPTED,shellCommand);

      case SE_ERR_ACCESSDENIED:
         return CATRESULTFILE(CAT_ERR_FILE_ACCESS_DENIED,shellCommand);

      case SE_ERR_PNF:
         return CATRESULTFILE(CAT_ERR_PATH_DOES_NOT_EXIST, shellCommand);
               
      default:
         return CATRESULTFILE(CAT_ERR_EXECUTE_FAILED,shellCommand);      
   }      
}

CATMODKEY GetModifierKeys()
{
	CATMODKEY modKey = (((::GetKeyState(VK_CONTROL) & 0x8000 ) ? CATMODKEY_CTRL : CATMODKEY_NONE) |
					    ((::GetKeyState(VK_SHIFT)   & 0x8000 ) ? CATMODKEY_SHIFT: CATMODKEY_NONE) |
					    ((::GetKeyState(VK_MENU)	& 0x8000 ) ? CATMODKEY_ALT  : CATMODKEY_NONE));
	return modKey;
};


CATResult OSLoadIconImage(const CATWChar* appPath,CATICON* icon, CATString& appName)
{
    SHFILEINFO shfi;
    if (!SHGetFileInfo( appPath, 0, &shfi, sizeof( SHFILEINFO ),SHGFI_DISPLAYNAME|SHGFI_ICON | SHGFI_LARGEICON))
    {
        return CAT_ERR_FILE_NOT_FOUND;
    }
    
    if (icon != 0)
    {
        *icon = shfi.hIcon;        
    }
    
    appName = shfi.szDisplayName;
    
    DWORD dummyZero = 0;
#ifdef _WIN32_WCE
    CATString aPath = appPath;
    CATUInt32 infoSize = ::GetFileVersionInfoSize((LPWSTR)(const wchar_t*)aPath,&dummyZero);  
#else
    CATUInt32 infoSize = ::GetFileVersionInfoSize(appPath,&dummyZero);  
#endif

    struct LANGANDCODEPAGE {
      WORD wLanguage;
      WORD wCodePage;
    } *lpTranslate;

    UINT cbTranslate = sizeof(lpTranslate);

    if (infoSize > 0)
    {
        CATUInt8* dataBuffer = new CATUInt8[infoSize*sizeof(CATWChar)];   
        if (::GetFileVersionInfo((LPWSTR)appPath,dummyZero,infoSize,dataBuffer))
        {
            
            VerQueryValue(dataBuffer,
                          L"\\VarFileInfo\\Translation",
                          (LPVOID*)&lpTranslate,
                          &cbTranslate);

            CATUInt32 langCode = (((CATUInt32)lpTranslate[0].wLanguage) << 16) | lpTranslate[0].wCodePage;
            CATString verPath = L"\\StringFileInfo\\";
            verPath.AppendHex(langCode,false);
            verPath << L"\\ProductName";

            CATWChar*          prodVerString = 0;
            unsigned int       prodVerLen = 0;
            
            VerQueryValueW(  dataBuffer, 
                            (LPWSTR)(const CATWChar*)verPath, 
                            (LPVOID*)&prodVerString, 
                            &prodVerLen);             

            if (prodVerString)
                appName = prodVerString;
        }

        delete [] dataBuffer;
    }

    
    return CAT_SUCCESS;
}

CATResult OSGetWindowIcon(const CATWChar* windowName,CATICON* icon)
{
    HWND hwnd = ::FindWindow(0,windowName);
    if (hwnd = 0)
        return CAT_ERR_FILE_NOT_FOUND;

#ifdef _WIN32_WCE
    (*icon) = (CATICON)(DWORD_PTR)::GetClassLong(hwnd,GCL_HICON);
#else
    (*icon) = (CATICON)(DWORD_PTR)::GetClassLongPtr(hwnd,GCLP_HICON);
#endif
        
    return CAT_SUCCESS;
}

CATString GetInstallLoc(const CATWChar* uninstKey)
{
    CATString loc;

    CATString regPath;
    regPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
    regPath << uninstKey;
    
    HKEY hKey = 0;
    if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,regPath,0,KEY_READ,&hKey))
    {
        DWORD type = 0;
        DWORD size = 1024*2;

        RegQueryValueEx(hKey,
                        L"InstallLocation",
                        0,
                        &type,
                        (LPBYTE)loc.GetUnicodeBuffer(size/2+2),&size);
        loc.ReleaseBuffer();

        ::RegCloseKey(hKey);
    }
   

    return loc;
}

CATString GetSoftwareReg(const CATWChar* regString)
{
    CATString loc;

    CATString tmpStr = regString;
    CATString regName = tmpStr.GetFilenameExt();
    CATString regPath = tmpStr.GetDriveDirectory();

    HKEY hKey = 0;
    if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,regPath,0,KEY_READ,&hKey))
    {
        DWORD type = 0;
        DWORD size = 1024*2;

        RegQueryValueEx(hKey,
                        regName,
                        0,
                        &type,
                        (LPBYTE)loc.GetUnicodeBuffer(size/2+2),&size);
        loc.ReleaseBuffer();

        ::RegCloseKey(hKey);
    }
   
    return loc;
}