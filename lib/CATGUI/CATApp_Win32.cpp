//---------------------------------------------------------------------------
/// \file  CATApp_Win32.cpp
/// \brief Application parent class - win32-specific functions
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
#include "CATApp.h"
#include "CATWindow.h"
#include "CATFileSystem.h"

#include <shlobj.h>

//---------------------------------------------------------------------------
// OpenFileDialog() displays an open file dialog and returns the
// selected file path, if any.
//
// \param title - title string
// \param filetypeList - list of file type / extension pairs.
//        Format should be: "Description of file (*.ext)|*.ext|"
//        Any other format may cause unexpected results.
// \param returnPath - ref to string to fill with received file path, if any.
//        when called, this is also used as the default file name.
//
//---------------------------------------------------------------------------
CATResult CATApp::OpenFileDialog( const CATString&           title,
                                 std::vector<CATString>&     filetypeList,
                                 CATString&                  returnPath,
                                 CATWindow*                  wnd)
{
    OPENFILENAME ofn;


    // Build up the filter / mask strings
    CATString filterString;   

    CATUInt32 i;
    CATUInt32 numEntries = (CATUInt32)filetypeList.size();
    for (i = 0; i < numEntries; i++)
    {
        CATString curString = filetypeList[i];
        filterString << curString;
        if (filterString.GetWChar(filterString.Length() - 1) != '|')
            filterString << "|";
    }

    // Convert filter string into filter buffer for OFN's.
    // We used |'s to hold the places of the nulls, so need to convert.   
    CATUInt32 filterBufLen = filterString.Length();
    CATWChar* filterBuf = filterString.GetUnicodeBuffer();
    for (i = 0; i < filterBufLen; i++)
    {
        if (filterBuf[i] == '|')
        {
            filterBuf[i] = 0;
        }
    }

    // Pick a parent window   
    HWND parentWnd = 0;

    if (wnd == 0)
    {
        parentWnd = ::GetActiveWindow();
    }
    else
    {
        parentWnd = wnd->OSGetWnd();
    }

    CATString titleBufStr = title;

    // Fill the open filename struct
    memset(&ofn,0,sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFilter = filterBuf;
    ofn.lpstrTitle  = titleBufStr.GetUnicodeBuffer();
    ofn.lpstrFile   = returnPath.GetUnicodeBuffer(_MAX_PATH+1);
    ofn.nMaxFile    = _MAX_PATH;	
    ofn.hwndOwner   = parentWnd;	
    ofn.Flags		 = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    BOOL dlgRes = ::GetOpenFileName(&ofn);

    // Release allocated buffers.
    filterString.ReleaseBuffer();
    titleBufStr.ReleaseBuffer();
    returnPath.ReleaseBuffer();

    if (!dlgRes)
    {
        returnPath = L"";
        return CATRESULT(CAT_ERR_FILEOPEN_CANCELLED);
    }

    return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// SaveFileDialog() displays a save file dialog and returns the
// selected file path, if any.
//
// \param title - title string
// \param filetypeList - list of file type / extension pairs.
//        Format should be: "Description of file (*.ext)|*.ext|"
//        Any other format may cause unexpected results.
// \param returnPath - ref to string to fill with received file path, if any.
//        when called, this is also used as the default file name.
//      
//---------------------------------------------------------------------------
CATResult CATApp::SaveFileDialog( const CATString&           title,
                                 std::vector<CATString>&     filetypeList,
                                 CATString&                  returnPath,
                                 CATWindow*                  wnd,
                                 bool                        promptOverwrite,
                                 const CATString&            fileExtension)
{
    OPENFILENAME ofn;


    // Build up filter list.
    CATString filterString;
    CATUInt32 i;
    CATUInt32 numEntries = filetypeList.size();
    for (i = 0; i < numEntries; i++)
    {
        CATString curString = filetypeList[i];
        filterString << curString;
        if (filterString.GetWChar(filterString.Length() - 1) != '|')
            filterString << L"|";
    }

    // Convert filter string into filter buffer for OFN's.
    // We used |'s to hold the places of the nulls, so need to convert.   
    CATUInt32 filterBufLen = filterString.Length();
    CATWChar* filterBuf = filterString.GetUnicodeBuffer();
    for (i = 0; i < filterBufLen; i++)
    {
        if (filterBuf[i] == '|')
        {
            filterBuf[i] = 0;
        }
    }

    // Pick a parent window   
    HWND parentWnd = 0;
    if (wnd == 0)
    {
        parentWnd = ::GetActiveWindow();
    }
    else
    {
        parentWnd = wnd->OSGetWnd();
    }

    CATString titleBufStr = title;

    // Fill the open filename struct
    memset(&ofn,0,sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFilter = filterBuf;
    ofn.lpstrTitle  = titleBufStr.GetUnicodeBuffer();
    ofn.lpstrFile   = returnPath.GetUnicodeBuffer(_MAX_PATH+1);
    ofn.nMaxFile    = _MAX_PATH;	
    ofn.hwndOwner   = parentWnd;	
    ofn.Flags		 = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;


    CATString defExtStr;
    if (fileExtension.IsEmpty() == false)
    {
        defExtStr = fileExtension;
        ofn.lpstrDefExt = defExtStr.GetUnicodeBuffer();
    }

    BOOL dlgRes = ::GetSaveFileName(&ofn);

    // Release allocated buffers.
    filterString.ReleaseBuffer();
    titleBufStr.ReleaseBuffer();
    returnPath.ReleaseBuffer();

    if (fileExtension.IsEmpty() == false)
        defExtStr.ReleaseBuffer();

    if (!dlgRes)
    {
        returnPath = "";
        return CATRESULT(CAT_ERR_FILESAVE_CANCELLED);
    }

    return CAT_SUCCESS;

}


//---------------------------------------------------------------------------
// OSOnAppCreate() does OS-specific app initialization such as pulling the 
// startup path of the app and the like.
//---------------------------------------------------------------------------
void CATApp::OSOnAppCreate()
{
    //---------------------
    // Get path to program
    //---------------------
    CATWChar* buffer = fProgramPath.GetUnicodeBuffer(_MAX_PATH+1);
    if (buffer != 0)
    {
        memset(buffer,0,_MAX_PATH + 1);
        // Note - GetModuleFileNameW not supported by Win98, so use ASCII version
        // and rely on CATString's conversion for funky paths.
        GetModuleFileName(fAppInstance,buffer,_MAX_PATH);
        fProgramPath.ReleaseBuffer();     
    }

    //------------------------------------------
    // Get app base dir from registry
    //------------------------------------------
    CATTRACE("Reading application path from registry, if available...");
    CATString regPath = L"Software";      
    regPath << '\\' << this->GetAppName();
    HKEY hKey = 0;
    if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_CURRENT_USER,regPath,0,KEY_READ,&hKey))
    {
        DWORD type = 0;
        DWORD size = 1024*2;

        if (ERROR_SUCCESS != ::RegQueryValueEx(hKey,L"",0,&type,(LPBYTE)fBaseDir.GetUnicodeBuffer(size/2+2),&size))
        {
            CATTRACE("Error reading base directory key. Using defaults");
        }

        type = 0;
        size = 1024*2;

        if (ERROR_SUCCESS != ::RegQueryValueEx(hKey,L"Data_Dir",0,&type,(LPBYTE)fDataDir.GetUnicodeBuffer(size/2+2),&size))
        {
            CATTRACE("Error reading data directory key. Using defaults.");
        }

        ::RegCloseKey(hKey);
        fBaseDir.ReleaseBuffer();
        fDataDir.ReleaseBuffer();

        if (CATFAILED(fGlobalFileSystem->DirExists(fBaseDir)))
        {
            // Registry key is bogus. Crap.  Empty string - we'll use program path if possible.
            fBaseDir = "";
        }
    }

    //------------------------------------------
    // If registry failed, use program folder
    //------------------------------------------
    if (fBaseDir.IsEmpty())
    {
        CATString progName;
        fGlobalFileSystem->SplitPath(fProgramPath, fBaseDir, progName,true);
    } 

    if (fDataDir.IsEmpty())
    {
#ifdef _WIN32
			int folder = CSIDL_FLAG_CREATE | CSIDL_APPDATA;			
			CATString tmpStr;
         ::SHGetFolderPath(NULL, folder, NULL, SHGFP_TYPE_CURRENT, tmpStr.GetUnicodeBuffer(_MAX_PATH));
			tmpStr.ReleaseBuffer();
			fDataDir = fGlobalFileSystem->BuildPath(tmpStr,fAppName,true);
#else
        fDataDir = fGlobalFileSystem->BuildPath(fBaseDir,L"Data",true);
#endif
    }

    fGlobalFileSystem->EnsureTerminator(fBaseDir);
    fGlobalFileSystem->EnsureTerminator(fDataDir);

}
