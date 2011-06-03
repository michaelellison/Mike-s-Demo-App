//---------------------------------------------------------------------------
/// \file CATApp.cpp
/// \brief Application parent class - derive from this for each app made.
/// \ingroup CATGUI
/// 
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//
//---------------------------------------------------------------------------
#include <memory.h>
#include <stdio.h>

#include "CATApp.h"
#include "CATPlatform.h"
#include "CATFileSystem.h"
#include "CATPrefs.h"
#include "CATXMLParser.h"
#include "CATGuiFactory.h"
#include "CATSkin.h"
#include "CATEventDefs.h"

#include "CATWindow.h"
#include "CATControl.h"
#include "CATWaitDlg.h"


CATApp* gApp = 0;

//---------------------------------------------------------------------------
// App construct - requires you to specify a runmode in the 
// constructor.
//
// \param runMode - current runmode. See CATRunMode
//
CATApp::CATApp(  CATINSTANCE        instance, 
                 CATRunMode         runMode, 
                 const CATString&   appName)               
{
    try
    {
        fGUIFactory          = 0;   
        fSkin                = 0;
        fWaitDlg					= 0;

        fAppName             = appName;
        this->fRunMode       = runMode;
        this->fAppInstance   = instance;
        gPlatform            = new CATPlatform();   
        fGlobalFileSystem    = gPlatform->GetFileSystem();
        fExiting             = false;	

        // Finds base app paths and the like that are system-specific	
        OSOnAppCreate();

        //----------------------------------------------------------------
        /// SkinDir
        CATString skinDirName = L"Skin";
        fSkinDir = fGlobalFileSystem->BuildPath(fBaseDir,skinDirName,true);
        if (CATFAILED(fGlobalFileSystem->DirExists(fSkinDir)))
        {
            fGlobalFileSystem->CreateDir(fSkinDir);
        }

        // Help dir
        fHelpDir = fGlobalFileSystem->BuildPath(fBaseDir,"Help",true);

        // Console mode will rarely, if ever, want creation of sub dirs.
        if (runMode != CAT_CONSOLE)
        {
            if (CATFAILED(fGlobalFileSystem->DirExists(fHelpDir)))
            {
                fGlobalFileSystem->CreateDir(fHelpDir);
            }
        }

        //----------------------------------------------------------------
        // If data directory is empty, use system default
        //----------------------------------------------------------------
        if (fDataDir.IsEmpty())
        {
            fDataDir = fGlobalFileSystem->BuildPath(fBaseDir,"Data",true);
        }

        if (runMode != CAT_CONSOLE)
        {
            if (CATFAILED(fGlobalFileSystem->DirExists(fDataDir)))
            {
                fGlobalFileSystem->CreateDir(fDataDir);
            }
        }

        //----------------------------------------------------------------
        // Init prefs
        CATString prefsFilename = fGlobalFileSystem->BuildPath(this->GetDataDir(), fAppName);
        prefsFilename << ".cfg";
        fPrefs     = new CATPrefs(prefsFilename); 
        fPrefs->Load(fGlobalFileSystem);
        fTempPrefs = new CATPrefs();

    }
    catch (...)
    {
        gApp->DisplayMessage("Error initializing app directories and file system. Exiting");
        exit(-1);
    }
}


//---------------------------------------------------------------------------
CATApp::~CATApp()
{
    if (CATFAILED(fAppLock.Wait()))
    {
        CATASSERT(false,"Failed to get app lock!");
        return;
    }

    if (fSkin != 0)
    {      
        delete fSkin;
        fSkin = 0;
    }

    if (fWaitDlg != 0)
    {
        delete fWaitDlg;
        fWaitDlg = 0;
    }

    if (fTempPrefs)
    {
        delete fTempPrefs;
        fTempPrefs = 0;
    }

    /// Everything that might save prefs must be deleted above here!
    if (fPrefs != 0)
    {
        CATResult result = fPrefs->Save();
        if (CATFAILED(result))
        {
            gApp->DisplayError(result);
        }
        delete fPrefs;
        fPrefs = 0;
    }

    if (fGlobalFileSystem)
    {
        gPlatform->Release(fGlobalFileSystem);
        CATASSERT(fGlobalFileSystem == 0, "Global based file systems still allocated...");
    }

    if (fGUIFactory)
    {
        delete fGUIFactory;
        fGUIFactory = 0;
    }

    FlushResourceCache();

    if (gPlatform != 0)
    {
        delete gPlatform;
        gPlatform = 0;
    }

    fAppLock.Release();
}

//---------------------------------------------------------------------------
// GetBaseDir() retrieves the base directory for the application.
//
// This should generally be the directory the .exe is in.
// \return CATString - base directory path
CATString CATApp::GetBaseDir()
{
    CATResult result = CAT_SUCCESS;

    if (CATFAILED(result = fAppLock.Wait()))
    {
        CATASSERT(false,"Failed to get app lock!");
        return "";
    }
    CATString baseDir = fBaseDir;

    fAppLock.Release();

    return baseDir;
}

//---------------------------------------------------------------------------
// GetSkinDir() retrieves the directory skins are located in.
//
// This should generally be BaseDir + "Skins/"
// \return CATString - skin directory path
CATString CATApp::GetSkinDir()
{
    return fSkinDir;
}

CATString CATApp::GetHelpDir()
{
    return fHelpDir;
}

//---------------------------------------------------------------------------
// GetDataDir() retrieves the directory misc data (like prefs) are located in.
//
// This should generally be BaseDir + "Data/"
// \return CATString - skin directory path
CATString CATApp::GetDataDir()
{
    return fDataDir;
}


//---------------------------------------------------------------------------
// GetRunMode() returns the run mode as defined in CATRunMode.
//
// \return CATRunMode - current run mode of the application
CATRunMode CATApp::GetRunMode()
{
    return fRunMode;
}

//---------------------------------------------------------------------------
// GetString() retrieves the string for the current language (set in SetLanguage)
// associated with the stringId.
//
// SetLanguage() should be called before calling GetString().
// GetString() is threadsafe, and may be called from any thread in the application.
//
// \param stringId - ID for the string - these will be defined in the 
// CATResult_* files.
// \return CATString - requested string, fallback string, or an empty string
// if none was found.
// \sa SetLanguage()
CATString CATApp::GetString(CATUInt32 stringId)
{
    CATResult result = CAT_SUCCESS;

    if (CATFAILED(result = fAppLock.Wait()))
    {
        CATASSERT(false,"Failed to get app lock!");
        return "";
    }

    CATString theString = this->fStringTable.GetString(stringId);

    fAppLock.Release();

    return theString;
}
//---------------------------------------------------------------------
// Overrideables
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Display an error box for a given return value.
//
// Since these may be classes or integers, it needs
// to account for both and dump as much information
// as possible.
// \param resultCode - result code to display info from
void CATApp::DisplayError(const CATResult& resultCode, CATWindow* wnd)
{
    if (resultCode == CAT_SUCCESS)
        return;

    CATString errorString = gApp->GetString(resultCode);

    // If the result strings are a class, provide additional
    // information
#ifdef CAT_RESULT_AS_CLASS
    CATString additionalInfo = resultCode.GetDescription();
    if (additionalInfo.IsEmpty() == false)
    {
        errorString << kCRLF << additionalInfo;
    }
    additionalInfo = resultCode.GetFileError();
    if (additionalInfo.IsEmpty() == false)
    {
        errorString << kCRLF << "File: " << additionalInfo;
    }

    errorString << kCRLF << "Source: " << resultCode.GetFilename() << " ( Line " << resultCode.GetLineNumber() << " )";

#endif

    DisplayMessage(errorString,wnd);   
}


void CATApp::DisplayMessage(const CATString& message, CATWindow* wnd)
{
    (void)this->DisplayPrompt(message, CATPROMPT_OK, wnd);
}

CATResult CATApp::DisplayPrompt( const CATString& message, CATPROMPTTYPE prompt, CATWindow* wnd)
{
    CATResult result = CAT_SUCCESS;

    if (this->GetRunMode() == CAT_CONSOLE)
    {
        printf("%s\n",(const char*)message);
        if (prompt == CATPROMPT_OK)
        {
            return CAT_STAT_PROMPT_OK;
        }

        result = CAT_STAT_IN_PROGRESS;
        do
        {
            char resChar = 0;

            switch (prompt)
            {
            case CATPROMPT_YESNO:
                printf("[Y]es or [N]o? ");
                resChar = getchar();
                if ((resChar | (char) 0x20) == 'y')
                {
                    result = CAT_STAT_PROMPT_YES;
                }
                else if ((resChar | (char) 0x20) == 'n')
                {
                    result = CAT_STAT_PROMPT_NO;
                }                  
                break;

            case CATPROMPT_YESNOCANCEL:
                printf("[Y]es, [N]o, or [C]ancel? ");
                resChar = getchar();
                if ((resChar | (char) 0x20) == 'y')
                {
                    result = CAT_STAT_PROMPT_YES;
                }
                else if ((resChar | (char) 0x20) == 'n')
                {
                    result = CAT_STAT_PROMPT_NO;
                }                  
                else if ((resChar | (char) 0x20) == 'c')
                {
                    result = CAT_STAT_PROMPT_CANCEL;
                }
                break;

            case CATPROMPT_OKCANCEL:
                printf("[O]kay or [C]ancel? ");
                resChar = getchar();
                if ((resChar | (char) 0x20) == 'o')
                {
                    result = CAT_STAT_PROMPT_OK;
                }
                else if ((resChar | (char) 0x20) == 'c')
                {
                    result = CAT_STAT_PROMPT_CANCEL;
                }                  
                break;
            default:
                CATASSERT(false,"Unknown prompt type in console mode!");
                break;
            }

            printf("%s\n", (const char*)this->fStringTable.GetString(result));
        } while (result == CAT_STAT_IN_PROGRESS);
    }
    else
    {
        DWORD resid = 0;
        DWORD promptType;

        switch (prompt)
        {
        case CATPROMPT_YESNO:
            promptType = MB_YESNO;
            break;
        case CATPROMPT_OKCANCEL:
            promptType = MB_OKCANCEL;
            break;
        case CATPROMPT_YESNOCANCEL:
            promptType = MB_YESNOCANCEL;
            break;
        case CATPROMPT_OK:
        default:
            promptType = MB_OK;
            break;
        }

        HWND parentWnd = 0;
        if (wnd)         
        {
            parentWnd = wnd->OSGetWnd();
        }
        else
        {
            parentWnd = ::GetActiveWindow();
        }

        resid = ::MessageBox(parentWnd,message,this->GetAppName(),promptType);

        switch (resid)
        {
        case IDYES:
            result = CAT_STAT_PROMPT_YES;
            break;
        case IDNO:
            result = CAT_STAT_PROMPT_NO;
            break;
        case IDOK:
            result = CAT_STAT_PROMPT_OK;
            break;
        case IDCANCEL:
            result = CAT_STAT_PROMPT_CANCEL;
            break;
        default:
            CATASSERT(false,"Unknown prompt type result returned!");
            result = CAT_STAT_PROMPT_OK;
        }                  
    }
    return result;
}

//---------------------------------------------------------------------------
CATResult CATApp::Run()
{
    CATResult result = CAT_SUCCESS;

    result = OnStart();

    // If starting failed, then bail without entering main loop.
    if (CATFAILED(result))
    {
        return result;
    }

    result = MainLoop();

    // Pass the result from the MainLoop into OnEnd.
    result = OnEnd(result);

    return result;
}

//---------------------------------------------------------------------------
CATResult CATApp::OnStart()
{
    return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
CATResult CATApp::OnEnd(const CATResult& result)
{
    return result;
}
//---------------------------------------------------------------------------

void CATApp::OnCommand(CATCommand& command, CATControl* ctrl, CATWindow* wnd, CATSkin* skin)
{   
    /// App level commands

    CATString cmdString = command.GetCmdString();

    CATResult result = CAT_SUCCESS;

    /*
    if (cmdString.Compare("OpenURI") == 0)
    {
        // Opens a URI or executeable. This is a security risk, as it could be used to
        // execute C:\winnt\system32\format.com, but it also gives us a lot of power and
        // extensibility. Use with care, disable on secure apps.
        CATWND whndl = 0;
        if (wnd)
        {
            whndl = wnd->OSGetWnd();
        }
        result = CATExecute(command.GetStringParam(), whndl);
    }
    else 
    if (cmdString.Compare("OpenSkinBinary") == 0)
    {
        // ShellExecute a binary from the skin.  This will spawn HTML, executeables, etc.      
        CATString skinExec = this->fGlobalFileSystem->BuildPath(fSkin->GetRootDir(),command.GetStringParam());
        if (CATSUCCEEDED(result = fGlobalFileSystem->FileExists(skinExec)))
        {
            CATWND whndl = 0;
            if (wnd)
            {
                whndl = wnd->OSGetWnd();
            }
            result = CATExecute(skinExec, whndl);         
        }
    }
    else 
    */
    if (cmdString.Compare("Help") == 0)
    {
        this->OnHelp();
    }


    if (CATFAILED(result))
    {
        gApp->DisplayError(result);
    }
}
//---------------------------------------------------------------------------
CATResult CATApp::OnEvent(const CATEvent& event, CATInt32& retVal)
{
    switch (event.fEventCode)
    {
        // Window close event - set retVal to 1 to allow close, 0 otherwise.
    case CATEVENT_WINDOW_CLOSE:
        {
            CATWindow* theWnd = (CATWindow*)event.fVoidParam;
            // By default, allow.
            retVal = 1;
            return CAT_SUCCESS;
        }
        break;  

    case CATEVENT_WINDOW_HIDDEN:
        // If the window is primary, close the app.
        {
            CATWindow* theWnd = (CATWindow*)event.fVoidParam;
            if (theWnd->IsPrimary())
            {
                CATResult result = this->GetSkin()->OnEvent(event, retVal);
                // Now close...
                this->ExitApp();
                return result;
            }
        }
        break;


    }   
    // Pass it to skin to distribute through windows by default.
    return this->GetSkin()->OnEvent(event, retVal);
}
//---------------------------------------------------------------------------
CATResult CATApp::MainLoop()
{
    return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// GetAppName() retrieves the application name
//---------------------------------------------------------------------------
CATString CATApp::GetAppName()
{
    return this->fAppName;
}

//---------------------------------------------------------------------------
// GetAppExePath() retrieves the application path
//---------------------------------------------------------------------------
CATString CATApp::GetAppExePath()
{
    this->GetBaseDir();
    return this->fProgramPath;
}

//---------------------------------------------------------------------------
// GetPrefs() retrieves a ptr to the app's preferences object.
//---------------------------------------------------------------------------
CATPrefs* CATApp::GetPrefs()
{
    return this->fPrefs;
}

//---------------------------------------------------------------------------
// GetTempPrefs() retrieves a ptr to the app's runtime preferences object.
//---------------------------------------------------------------------------
CATPrefs* CATApp::GetTempPrefs()
{
    return this->fTempPrefs;
}

//---------------------------------------------------------------------------
// GetGlobalFileSystem() retrieves a file system object for
// the system.
//
// \return CATFileSystem* - pointer to global file system.
// Don't delete this - only one per app...
CATFileSystem* CATApp::GetGlobalFileSystem()
{
    return this->fGlobalFileSystem;
}


// GetInstance() retrieves the application instance.
// For windows, this is your HINSTANCE handle.
// \return CATINSTANCE - instance of application
CATINSTANCE CATApp::GetInstance()
{
    return fAppInstance;
}

//---------------------------------------------------------------------------
// LoadSkin() loads in a skin
//---------------------------------------------------------------------------
CATResult CATApp::LoadSkin(const CATString& skinPath)
{
    CATResult result = CAT_SUCCESS;

    if (CATFAILED(result = fAppLock.Wait()))
    {
        return result;
    }

    if (fSkin != 0)
    {
        delete fSkin;
        fSkin = 0;
    }

    CATString skinDir, skinFile;
    this->fGlobalFileSystem->SplitPath(skinPath, skinDir, skinFile);


    if (fGUIFactory == 0)
    {
        fGUIFactory = new CATGuiFactory(skinDir, skinPath);
    }

    result = CATXMLParser::Parse(  skinPath,
        fGUIFactory, 
        (CATXMLObject*&)fSkin);


    if (CATFAILED(result))
    {
        if (fSkin != 0)
        {
            delete fSkin;
            fSkin = 0;
        }
        fAppLock.Release();
        return result;
    }

    // Load it in
    result = fSkin->Load(SkinLoadCB, this, 0.0, 1.0f);
    if (CATFAILED(result))
    {      
        delete fSkin;
        fSkin = 0;      
    }

    fAppLock.Release();

    return result;
}

void CATApp::SkinLoadCB( CATFloat32 progress, const CATString& status, void* userParam)
{
    CATApp* app = (CATApp*)userParam;
    CATString statString = "Loading Skin: ";
    statString << status;
    app->SetWaitStatus( statString, progress);
}

//---------------------------------------------------------------------------
// GetSkin() just returns our skin pointer
//---------------------------------------------------------------------------
CATSkin* CATApp::GetSkin()
{
    return fSkin;
}

//---------------------------------------------------------------------------
void CATApp::ExitApp()
{
    fExiting = true;

    // MAEDEBUG
    // This shouldn't go here long term...   
    CATPostQuit(0);  
}
//---------------------------------------------------------------------------
bool CATApp::IsExiting()
{
    return fExiting;
}

void CATApp::OnHelp()
{
    this->DisplayError(CATRESULT(CAT_ERR_NO_HELP_AVAILABLE));
}

/// Resource image request functions - these allow us to reuse a single
/// image w/o wasting more space...
CATResult CATApp::AddResourceImage( const CATString& path, CATImage* image)
{
    CATImage* oldImage = 0;
    std::map<CATString, CATImage*>::iterator iter = fImageCache.find(path);
    if (iter != fImageCache.end())
    {
        return CATRESULT(CAT_STAT_IMAGE_ALREADY_LOADED);
    }


    // For now, we add a ref just for this list. Call FlushResourceCache() to
    // remove this additional reference and free otherwise unused images...
    //
    // AFAIK, this would only be done if switching skins at runtime anyway,
    // or on exit.
    //
    // This does prevent the possibility of us having old deleted images
    // in our list though...
    image->AddRef();
    fImageCache.insert(std::make_pair(path,image));   
    return CAT_SUCCESS;
}

// Find image if available and increment reference count.
CATResult CATApp::GetResourceImage( const CATString& path, CATImage*& image)
{       
    std::map<CATString, CATImage*>::iterator iter = fImageCache.find(path);
    if (iter != fImageCache.end())
    {
        image = iter->second;
        image->AddRef();
        return CAT_SUCCESS;
    }

    return CATRESULT(CAT_ERR_IMAGE_NULL);
}


// Flush unused images from the cache.
CATResult CATApp::FlushResourceCache()
{
    std::map<CATString, CATImage*>::iterator iter = fImageCache.begin();
    while (iter != fImageCache.end())
    {
        if (iter->second->GetRefCount() == 1)
        {
            CATImage::ReleaseImage(iter->second);
            iter = fImageCache.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    return CAT_SUCCESS;
}


CATResult CATApp::InitWaitDlg(CATInt32 bmpId, CATRect& textRect, CATInt32 progOnId, CATInt32 progOffId, CATInt32 progLeft, CATInt32 progTop)
{
    if (fWaitDlg != 0)
        delete fWaitDlg;

    fWaitDlg = new CATWaitDlg(this->fAppInstance, bmpId, textRect, progOnId, progOffId, progLeft, progTop);

    if (fWaitDlg != 0)
        return CAT_SUCCESS;	

    return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
}

CATWaitDlg* CATApp::GetWaitDlg()
{
    CATASSERT(fWaitDlg != 0, "Calling GetWaitDlg() prior to initializing it with InitWaitDlg()!");
    return fWaitDlg;
}


void CATApp::SetWaitString(const CATString& waitStr)
{
    if (fWaitDlg != 0)
        fWaitDlg->SetWaitLabelText(waitStr);
}

void CATApp::SetWaitProgress(CATFloat32 progress)
{
    if (fWaitDlg != 0)
        fWaitDlg->SetProgress(progress);
}

void CATApp::SetWaitStatus( const CATString& waitStr, CATFloat32 progress)
{
    if (fWaitDlg)
    {
        fWaitDlg->SetWaitLabelText(waitStr);
        fWaitDlg->SetProgress(progress);
    }
}

CATString CATApp::GetVersionString()
{
    CATString versionString;
    versionString = this->GetAppName();
    versionString << (CATWChar)0x2122; // Add trademark


    // Retrieve version info from file
    CATString ourFile = this->GetAppExePath();

    DWORD dummyZero = 0;
    CATUInt32 infoSize = ::GetFileVersionInfoSize(ourFile,&dummyZero);  

    if (infoSize > 0)
    {
        CATUInt8* dataBuffer = new CATUInt8[infoSize*sizeof(CATWChar)];   
        if (::GetFileVersionInfo(ourFile,dummyZero,infoSize,dataBuffer))
        {
            VS_FIXEDFILEINFO*  prodVerPtr = 0;
            unsigned int     prodVerLen = 0;

            if (::VerQueryValue(dataBuffer,L"\\",(void**)&prodVerPtr,&prodVerLen))
            {
                CATUInt8 majorVersion  = (CATUInt8)((prodVerPtr->dwProductVersionMS & 0xFF0000) >> 16);
                CATUInt8 minorVersion  = (CATUInt8)((prodVerPtr->dwProductVersionLS & 0xFF0000) >> 16);
                CATUInt8 subVersion    = (CATUInt8)((prodVerPtr->dwProductVersionLS & 0xFF));
                versionString << L" Version: " << (char)(majorVersion + '0') << L"." 
                    << (char)(minorVersion + '0') << (char)(subVersion + '0');
            }
        }

        delete [] dataBuffer;
    }

    return versionString;
}


CATResult CATApp::LoadStrings(const CATString& stringPath)
{
	return fStringTable.LoadXMLStringTable(stringPath);
}
