// Demo app, Copyright (c) 2011 by Mike Ellison
#include "MikesDemo.h"

#include "CATWindow.h"
#include "CATAppButton.h"
#include "CATEventDefs.h"
#include "CATFileSystem.h"

#include <psapi.h>

MikesDemo::MikesDemo(CATINSTANCE appInstance, CATRunMode runMode, const CATString& appName) 
: CATApp(appInstance, runMode, appName)
{
    fWindow         = 0;
}

MikesDemo::~MikesDemo()
{
}

CATResult MikesDemo::OnStart()
{
	 // Find our skin and load it
	 CATResult result = CAT_SUCCESS;
    CATString skinPath = this->GetSkinDir();
	 CATString stPath = skinPath;

	 skinPath << "Skin.xml";
	 stPath   << "Strings.xml";

	 result = this->LoadStrings(stPath);

    CATString skinDir, skinFile;
    this->fGlobalFileSystem->SplitPath(skinPath, skinDir, skinFile);
    this->fGUIFactory = new MikesDemoGuiFactory(skinDir,skinPath);

    if (CATFAILED(result = this->LoadSkin(skinPath)))
    {
        return result;
    }

	 // Open the window
    result = fSkin->OpenWindow(L"MikesDemo", fWindow);

    return result;
}

CATResult MikesDemo::OnEnd(const CATResult& result)
{
    return result;
}

CATResult MikesDemo::OnEvent(const CATEvent& event, CATInt32& retVal)
{
    return CATApp::OnEvent(event, retVal);
}

CATResult MikesDemo::MainLoop()
{
 	 MSG msg;
    CATResult result = CAT_SUCCESS;
    while (::GetMessage(&msg,0,0,0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }   
    return result;
}


void MikesDemo::OnCommand(    CATCommand& command, 
                              CATControl* ctrl, 
                              CATWindow*  wnd, 
                              CATSkin*    skin)
{
    CATString commandString = command.GetCmdString();
    CATResult result = CAT_SUCCESS;

	 // Check custom app-wide commands here, 
	 // otherwise send to framework.
    if (0 == commandString.Compare(L"Execute"))
    {
       CATExecute(command.GetStringParam(),0);
    }
	 else if (0 == commandString.Compare(L"Install"))
	 {
		 CATString installer =
			GetGlobalFileSystem()->BuildPath(GetSkinDir(),command.GetStringParam());
		 CATExecute(installer,0);
	 }
	 else
	 {
		CATApp::OnCommand(command,ctrl,wnd,skin);
	 }

    if (CATFAILED(result))
    {
        DisplayMessage( this->GetString(result),fWindow);
    }
}


void MikesDemo::OnHelp()
{
	 // Popup html help
    CATString helpPath = this->GetHelpDir();    
    helpPath << L"index.html";
    CATResult result = CATExecute(helpPath);
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   if (FAILED(::OleInitialize(0)))
   {
      ::MessageBox(NULL,L"Could not initialize COM.\n",L"MikesDemo",MB_OK);
      return -1;
   }

   CATResult result = CAT_SUCCESS;
   try
   {
      gApp = new MikesDemo(hInstance, CAT_GUI,L"MikesDemoWindow");
   }
   catch(...)
   {
      gApp = 0;
   }

   if (gApp == 0)
   {
      ::MessageBox(NULL,L"Could not create app object. Terminating.",L"MikesDemo",MB_OK);
      ::CoUninitialize();
      return -1;
   }
   
	// Main run loop
   result = gApp->Run();
   

	// Exit app
   if (CATFAILED(result))
   {
      gApp->DisplayError(result);
   }
   
   delete gApp;
   gApp = 0;
	::OleUninitialize();
   return result;
}

#endif