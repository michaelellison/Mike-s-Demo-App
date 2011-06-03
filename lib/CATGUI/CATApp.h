//---------------------------------------------------------------------------
/// \file CATApp.h
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
#ifndef CATApp_H_
#define CATApp_H_

#include "CATTypes.h"
#include "CATString.h"
#include "CATResult.h"
#include "CATStringTableCore.h"
#include "CATMutex.h"
#include "CATSkin.h"

class CATPrefs;
class CATWaitDlg;
class CATFileSystem;
class CATGuiFactory;

/// CATRunMode defines the type of app the code is running as.
/// It allows classes to perform differently in different environments
/// without having to #ifdef stuff.
///
/// Use CATApp::GetRunMode() to retrieve the current run mode.
enum CATRunMode
{
    /// Basic console app
    CAT_CONSOLE,
    /// Windowed GUI app
    CAT_GUI,
    /// VST plug-in environment
    CAT_VST
};

/// \class CATApp
/// \brief Application parent class - derive from this for each app made.
/// \ingroup CATGUI
///
/// CATApp is an attempt at virtualizing enough of the idea of an application
/// that we don't have to rewrite our entrypoint code each time or #ifdef
/// code depending on whether we're in a VST, a console app, or whatever.
///
class CATApp
{
public:
    /// App construct - requires you to specify a runmode in the 
    /// constructor.
    ///
    /// \param instance - instance of app (HINSTANCE for windows, does mac have equiv?)
    /// \param runMode - current runmode. See CATRunMode
    /// \param appName - application name
    ///
    CATApp(     CATINSTANCE        instance, 
                CATRunMode         runMode, 
                const CATString&   appName);                           

    virtual    ~CATApp();

    /// GetBaseDir() retrieves the base directory for the application.
    ///
    /// This should generally be the directory the .exe is in.
    /// \return CATString - base directory path
    virtual CATString    GetBaseDir();

    /// GetDataDir() retrieves the directory misc. data (like the prefs)
    /// are in.
    ///
    /// This should generally be BaseDir + "Data/"
    /// \return CATString - skin directory path
    virtual CATString    GetDataDir();

    /// GetSkinDir() retrieves the directory skins are located in.
    ///
    /// This should generally be BaseDir + "Skin/"
    /// \return CATString - skin directory path
    virtual CATString    GetSkinDir();

    /// GetSkinDir() retrieves the directory the help files are located in.
    ///
    /// This should generally be BaseDir + "Help/"
    /// \return CATString - help directory path
    virtual CATString    GetHelpDir();

    /// InitWaitDlg() initializes the wait dialog for use. Call
    /// before retrieving the wait dialog.  Generally initialized
    /// in OnStart() from derived classes.
    virtual CATResult	  InitWaitDlg(  CATInt32 bmpId, 
                                        CATRect& textRect,
                                        CATInt32 progressOnId  = 0,
                                        CATInt32 progressOffId = 0,
                                        CATInt32 progressLeft  = 0,
                                        CATInt32 progressTop   = 0);

    /// GetWaitDlg() retrieves a pointer to the app's wait dialog.
    /// WARNING: this may be null. Call InitWaitDlg() to create the
    /// wait dialog first.
    virtual CATWaitDlg*  GetWaitDlg();

    virtual void		 SetWaitString(   const CATString&  waitStr);

    virtual void		 SetWaitProgress( CATFloat32        progress );

    virtual void		 SetWaitStatus(   const CATString&  waitStr, 
                                          CATFloat32        progress);

    /// GetAppFileSystem() retrieves a file system object for the 
    /// app's base directory.
    ///
    /// \return CATFileSystem* - pointer to app file system.
    /// Don't delete this - only one per app...
    CATFileSystem* GetAppFileSystem();

    /// GetGlobalFileSystem() retrieves a file system object for
    /// the system.
    ///
    /// \return CATFileSystem* - pointer to global file system.
    /// Don't delete this - only one per app...
    CATFileSystem* GetGlobalFileSystem();

    /// GetRunMode() returns the run mode as defined in CATRunMode.
    ///
    /// \return CATRunMode - current run mode of the application
    CATRunMode   GetRunMode();


    //---------------------------------------------------------------------
    // Overrideables
    //---------------------------------------------------------------------
    /// Run() begins the app.
    ///
    /// You should load a stringtable first via SetLanguage().
    /// Run() does not return until the app is exiting. It calls
    /// the overrideables for OnStart(), OnEnd(), and MainLoop()
    /// however so your derived class gets control.
    ///
    /// \return CATResult - final result for application
    virtual CATResult Run();

    //---------------------------------------------------------------------
    /// Display an error box for a given return value.
    ///
    /// Since these may be classes or integers, it needs
    /// to account for both and dump as much information
    /// as possible.
    /// \param resultCode - result code to display info from
    virtual void         DisplayError(	const CATResult& resultCode, 
                                        CATWindow*		 wnd = 0);

    /// Display an message box with the specified string
    ///
    /// \param message - string to display
    virtual void         DisplayMessage(const CATString& message,
                                        CATWindow*		 wnd = 0);

    /// CATPROMPTTYPE defines the types of prompts that are available 
    /// from DisplayPrompt
    enum CATPROMPTTYPE
    {
        CATPROMPT_YESNO,
        CATPROMPT_OKCANCEL,
        CATPROMPT_OK,
        CATPROMPT_YESNOCANCEL
    };

    /// DisplayPrompt() displays a message and retrieves a response 
    /// from the user.
    ///
    /// The user is prompted with a message, and the response from the 
    /// prompt is returned.
    ///
    /// \param message - message to send to user
    /// \param prompt - type of prompt to use (see CATPROMPTTYPE)
    /// \param wnd - parent window of display box (GUI only)
    /// \return CATResult - CAT_STAT_PROMPT_[YES|NO|OK|CANCEL]
    virtual CATResult DisplayPrompt( const CATString&   message, 
                                     CATPROMPTTYPE      prompt, 
                                     CATWindow*         wnd = 0);



    /// OpenFileDialog() displays an open file dialog and returns the
    /// selected file path, if any.
    ///
    /// \param title - title string
    /// \param filetypeList - list of file type / extension pairs. std::map<filename, filemask>,
    ///        e.g. std::map.Insert("Session files","*.drum_sess")
    /// \param returnPath - ref to string to fill with received file path, if any.
    ///        when called, this is also used as the default file name.
    /// \param parentWnd - parent window ofof dialog
    virtual CATResult OpenFileDialog(   const CATString&               title,
                                        std::vector<CATString>&        filetypeList,
                                        CATString&                     returnPath,
                                        CATWindow*                     parentWnd = 0);

    /// SaveFileDialog() displays a save file dialog and returns the
    /// selected file path, if any.
    ///
    /// \param title - title string
    /// \param filetypeList - list of file type / extension pairs. std::map<filename, filemask>,
    ///        e.g. std::map.Insert("Session files","*.drum_sess")
    /// \param returnPath - ref to string to fill with received file path, if any.
    ///        when called, this is also used as the default file name.
    /// \param parentWnd - parent window
    /// \param promptOverwrite - if true, prompts if the file already exists...
    /// \param fileExtension - default file extension to add to file name
    /// 
    virtual CATResult SaveFileDialog(   const CATString&               title,
                                        std::vector<CATString>&        filetypeList,
                                        CATString&                     returnPath,
                                        CATWindow*                     parentWnd = 0,
                                        bool                           promptOverwrite = true,
                                        const CATString&               fileExtension = "");


    /// GetString() retrieves the string for the current language (set in SetLanguage)
    /// associated with the stringId.
    ///
    /// SetLanguage() should be called before calling GetString().
    /// GetString() is threadsafe, and may be called from any thread in the application.
    ///
    /// \param stringId - ID for the string - these will be defined in the 
    /// CATResult_* files.
    /// \return CATString - requested string, fallback string, or an empty string
    /// if none was found.
    /// \sa SetLanguage()
    CATString             GetString(CATUInt32 stringId);

    /// GetAppName() retrieves the application name
    ///
    /// \return CATString - name of application
    CATString             GetAppName();

    /// GetAppName() retrieves the full path of the application.
    ///
    /// \return CATString - path to application
    CATString             GetAppExePath();

    /// GetPrefs() retrieves a pointer to the preferences object for the app.
    ///
    /// Do NOT delete this. It is owned by the app.
    /// \return CATPrefs - application preferences
    /// \sa CATPrefs
    CATPrefs*             GetPrefs();

    /// GetTempPrefs() retrieves a pointer to the runtime prefs.
    ///
    /// Do NOT delete this. It is owned by the app.
    /// \return CATPrefs - application preferences
    /// \sa CATPrefs
    CATPrefs*             GetTempPrefs();

    /// GetInstance() retrieves the application instance.
    /// For windows, this is your HINSTANCE handle.
    /// \return CATINSTANCE - instance of application
    CATINSTANCE           GetInstance();

    /// LoadSkin() loads a skin
    /// WARNING: this is not really tested for loading more than
    ///          one yet per run.
    /// \param skinPath - path to skin to load
    /// \return CATResult - CAT_SUCCESS on success.
    CATResult             LoadSkin(const CATString& skinPath);
	 
	 /// Loads the xml string table given the path
	 CATResult				  LoadStrings(const CATString& stringPath);

    CATGuiFactory*        GetGUIFactory() {return fGUIFactory;}

    /// GetSkin() retrieves a ptr to the base skin object.
    /// Do NOT delete it.
    /// \return CATSkin* - pointer to the current skin
    CATSkin*              GetSkin();

    //---------------------------------------------------------------------
    // You should override the following functions in your application.
    //---------------------------------------------------------------------      

    /// OnStart() is called before the primary event loop is started. This would
    /// be a good place to load your skin and perform other initialization functions.
    /// \return CAT_SUCCESS on success.
    virtual CATResult     OnStart();

    /// OnEnd() is called after the main loop finishes, with the result code of
    /// the loop.
    ///
    virtual CATResult     OnEnd(const CATResult& result);

    /// OnEvent() is called as events are received that need to be processed by the application.
    virtual CATResult     OnEvent(const CATEvent& event, CATInt32& retVal);

    /// OnCommand() is called each time a command is received by the application.
    ///
    /// Most often, commands will come up from the Skin from controls. Each time
    /// a button is clicked or a knob turned, for example.
    ///
    /// The commands are a class, primarily containing a command string that the
    /// application should check to see if it supports. If so, the command should 
    /// be performed.
    ///
    /// \param command - the command to execute
    /// \param ctrl - the control that sent the command, or 0
    /// \param wnd - the window of the control, or 0
    /// \param skin - the skin the command came from, or 0
    virtual void         OnCommand(  CATCommand& command, 
                                     CATControl* ctrl, 
                                     CATWindow* wnd, 
                                     CATSkin* skin);

    /// MainLoop() is the message pump. You'll generally want to do things like
    /// a GetMessage() / DispatchMEssage() loop in here.
    virtual CATResult     MainLoop();

    /// ExitApp() requests an immediate exit
    virtual void          ExitApp();

    /// IsExiting() returns true if the application is in the process of exiting.
    virtual bool          IsExiting();

    /// OnHelp() is called when the user requests help.
    virtual void          OnHelp();

    virtual CATString     GetVersionString();


    /// Resource image request functions - these allow us to reuse a single
    /// image w/o wasting more space...
    CATResult             AddResourceImage( const CATString& path, CATImage* image);
    CATResult             GetResourceImage( const CATString& path, CATImage*& image);
    CATResult             FlushResourceCache();

    virtual void OSOnAppCreate();
    //---------------------------------------------------------------------
    // Global objects
    //---------------------------------------------------------------------

protected:
    static void SkinLoadCB( CATFloat32 progress, const CATString& status, void* userParam);

    bool                    fExiting;            // Flag for inherited classes to check in their main loop
    // to see if we're exiting.

    CATPrefs*               fPrefs;              // Global preferences
    CATPrefs*			    fTempPrefs;		     // Runtime preferences

    CATMutex                fAppLock;            // Thread lock for app object
    // to serialize access
    CATStringTableCore      fStringTable;        // Application string table
    CATRunMode              fRunMode;            // Current run mode

    CATFileSystem*          fGlobalFileSystem;   // Global file system for app framework.

    CATString               fBaseDir;            // Base directory for app    
    CATString				fDataDir;			 // Base data directory

    CATString				fHelpDir;			 // Help directory

    CATString               fProgramPath;        // Full path of program
    CATString               fAppName;            // friendly application name
    CATINSTANCE             fAppInstance;        // Application instance       

    CATString				fSkinDir;			 // Base skin directory    
    CATGuiFactory*          fGUIFactory;         // Factory for GUI (overrideable)
    CATSkin*                fSkin;               // Application's skin
    CATWaitDlg*				fWaitDlg;

    std::map<CATString, CATImage*>  fImageCache; // Image cache
};

extern CATApp* gApp;

#endif // CATApp_H_
