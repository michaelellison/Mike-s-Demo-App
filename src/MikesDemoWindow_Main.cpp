/// \file MikesDemoWindow_Main.cpp
/// \brief Main Window class for MikesDemo
/// \ingroup MikesDemo
///
/// Copyright (c) 2011 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///

#include "MikesDemoWindow_Main.h"
#include "CATEventDefs.h"
#include "CATSwitch.h"
#include "CAT3DVideo.h"
#include "CATFileSystem.h"
#include "CATMenu.h"
#include "CATVideoCtrl.h"
#include "CATTab.h"
#include "CATTreeCtrl.h"
#include "CATPictureMulti.h"
#include "CATLabel.h"


// Joystick scan from old scanner (in skin directory)
const char* k3dScanFile = "GoodStick.3sc";

MikesDemoWindow_Main::MikesDemoWindow_Main( const CATString& element, 
														  const CATString& rootDir)
:CATWindow(element,rootDir)
{
	// Controls that we grab
	fView3d  = 0;
	fTabCtrl = 0;
	fProTree = 0;
	fProPic  = 0;
	fProLogo = 0;
	fProText = 0;

	// 3D data
	fNum3dPoints	  = 0;
	f3dPoints		  = 0;
	fPointScanArray  = 0;
	fPointScanHeight = 0;
	fPointScanScans  = 0;
} 

MikesDemoWindow_Main::~MikesDemoWindow_Main()
{
	Clear3dPoints();
}

void MikesDemoWindow_Main::OnCreate()
{
	 CATResult result = CAT_SUCCESS;

	 CATWindow::OnCreate();

	 // Grab controls we'll be accessing
	 fView3d = (CAT3DVideo*)this->FindControlAndVerify("Video3D","Video3D");	 
	 fTabCtrl= (CATTab*)this->Find(L"Tabs",L"Tab");
	 fProTree= (CATTreeCtrl*)this->FindControlAndVerify("ProTree","Tree");
	 fProLogo= (CATPictureMulti*)FindControlAndVerify("ProLogos","PictureMulti");
	 fProPic = (CATPictureMulti*)FindControlAndVerify("ProPics","PictureMulti");
	 fProText = (CATLabel*)FindControlAndVerify("ProText","Label");

	 if (fProTree)
		fProTree->SetCurItem(fProTree->GetRootItem(0),true);

	 // Load in old 3D scan to display
	 if (fView3d)
	 {
		 CATString pointFile = 
			 gApp->GetGlobalFileSystem()->BuildPath(gApp->GetSkinDir(), k3dScanFile);	 
		 if (!pointFile.IsEmpty())
		 {
				if (CATSUCCEEDED(result = LoadRawScan(pointFile)))
				{
					// Display it and rotate it.
					fView3d->Set3dFacets(this->f3dPoints, fPointScanScans,fPointScanHeight);
					fView3d->SetRotateSpeed(2.0f);
				}
		 }
	 }	 
}


void MikesDemoWindow_Main::OnDestroy()
{
	 if (fView3d)
		 fView3d->Stop();
    CATWindow::OnDestroy();
}

void MikesDemoWindow_Main::OnCommand(CATCommand& command, CATControl* ctrl)
{
    CATString commandString = command.GetCmdString();

	 // Custom commands for the window here...
	 if (0 == commandString.Compare(L"DoLogoLink"))
	 {
		 CATUInt32 linkIndex = 0;
		 if (fProLogo)
			 linkIndex = CATRound(fProLogo->GetValue());

		 switch(linkIndex)
		 {
			case 0:
				CATExecute(L"http://www.line6.com/podfarm");
				break;
			case 1:
				CATExecute(L"http://www.reflectsystems.com");
				break;
			case 2:
				CATExecute(L"http://www.freepatentsonline.com/6597381.html");
				break;
			case 3:
				CATExecute(L"http://www.nadatech.com");
				break;
		 }

	 }
	 else if (0 == commandString.Compare(L"TabSelect"))
	 {		 
		 if (fTabCtrl)
			fTabCtrl->SetCurTab((CATUInt32)command.GetValue());		
		 CATWindow::OnCommand(command,ctrl);
	 }
	 else if (0 == commandString.Compare(L"ProSelect"))
	 {
		   if (fProTree)
			{
				CATTREEINFO* curItem = fProTree->GetCurItem();
				if (curItem)
					OnProTreeChange((CATUInt32)curItem->DataPtr - 1);
			}
	 }
	 else if (0 == commandString.Compare(L"Go3d"))
	 {
		 if (fTabCtrl)
			 fTabCtrl->SetCurTab(2);
		 CATWindow::OnCommand(CATCommand("TabSelect",2),ctrl);
	 }
	 else
	 {
		CATWindow::OnCommand(command,ctrl);
	 }
}

void MikesDemoWindow_Main::OnProTreeChange(CATUInt32 index)
{
	
	if (fProLogo)
		fProLogo->SetValue((CATFloat32)index,false);
	if (fProPic)
		fProPic->SetValue((CATFloat32)index,false);					
	if (fProText)
		fProText->SetString(gApp->GetString(CAT_STR_MIKE_RESUME_LINE6 + index));
}

void MikesDemoWindow_Main::OnShow()
{
	 // Start the video
	if (fSkin)
	{
		if (fView3d)
			fView3d->Start();
	}
}

CATResult MikesDemoWindow_Main::OnEvent  (   const CATEvent&        eventStruct, 
															CATInt32&              retVal)
{
	switch (eventStruct.fEventCode)
	{
		case CATEVENT_WINDOW_SHOWN:
			if (eventStruct.fVoidParam == this)
			{
				this->OnShow();
			}
			break;
		
		default:
			break;
	}
   return CATWindow::OnEvent(eventStruct,retVal);
}




//---------------------------------------------------------------------------
// 3D Scanner functions

// 3D constants from old scanner
const CATFloat64 kMinZ			= -6.0;
const CATFloat64 kMaxZ			= 6.0;
const CATFloat64 kBottomLine	= 4.0;			// Y - location of bottom line.

CATResult MikesDemoWindow_Main::LoadRawScan(const CATString& fname)
{	
	CATResult result = CAT_SUCCESS;
	CATTRACE((CATString)"Loading Raw Scan: " << fname << "...");
	CATInt32   i,j;
	CATStreamFile	lastScan;
	CATInt32	numScans,height;

	if (CATSUCCEEDED(result = lastScan.Open(fname,CATStream::READ_ONLY)))
	{
		CATUInt32 wout;
		// Read height of scan
		wout = sizeof(CATInt32);
		if (CATFAILED(result = lastScan.Read(&height,wout)))
		{			
			CATTRACE("Error loading raw scan - couldn't read height.");
			lastScan.Close();
			result;
		}

		// Read number of scans
		wout = sizeof(CATInt32);
		if (CATFAILED(result = lastScan.Read(&numScans,wout)))
		{			
			CATTRACE("Error loading raw scan - couldn't read number of scans.");
			lastScan.Close();
			return result;
		}
		
		if (fPointScanArray != 0)
		{
			delete [] fPointScanArray;
			fPointScanArray = 0;
			fPointScanHeight = 0;
			fPointScanScans = 0;
		}

		fPointScanArray = new CATScanPoint[numScans*height];		
		fPointScanHeight = height;
		fPointScanScans = numScans;

		if (0 == fPointScanArray)
		{
			CATTRACE("Error: Couldn't allocate enough memory to load scan!");
			lastScan.Close();
			return CAT_ERR_OUT_OF_MEMORY;
		}

		// Read data directly into a raw buffer....		
		unsigned char* rawBuf = new unsigned char[numScans*height*(sizeof(CATFloat64)*3 + sizeof(CATUInt32))];
		if (!rawBuf)
		{
			CATTRACE("Not enough memory to load scan!");
			lastScan.Close();
			return CAT_ERR_OUT_OF_MEMORY;
		}

		wout = numScans*height*(sizeof(CATFloat64)*3 + sizeof(CATUInt32));
		if (CATSUCCEEDED(result = lastScan.Read(rawBuf, wout)))
		{
			// Now read data
			for (i = 0; i < numScans; i++)
			{
				for (j=0; j < height; j++)
				{
					CATInt32 step = ((sizeof(CATFloat64)*3) + sizeof(CATUInt32));
					CATInt32 curPos = (i * height) + j;

					// Read points from memory buffer
					fPointScanArray[curPos].y = *((CATFloat64*)&rawBuf[curPos*step]);
					fPointScanArray[curPos].z = *((CATFloat64*)&rawBuf[curPos*step + sizeof(CATFloat64)]);
					fPointScanArray[curPos].rotation = *((CATFloat64*)&rawBuf[curPos*step + sizeof(CATFloat64)*2]);
					fPointScanArray[curPos].color = *(CATUInt32*)((CATFloat64*)&rawBuf[curPos*step + sizeof(CATFloat64)*3]);
					// woops, r/b flipped.
					CATSwap(((CATCOLOR*)(&fPointScanArray[curPos].color))->r,
							  ((CATCOLOR*)(&fPointScanArray[curPos].color))->b);
				}
			}
		}
		else
		{
			result = CAT_ERROR;
		}

		delete [] rawBuf;
		lastScan.Close();		
		if (CATFAILED(result))
		{
			return result;
		}

		if (!(GetPointsFromRawScan(fPointScanScans,fPointScanHeight,fPointScanArray)))
		{
			return CAT_ERROR;
		}

		CATTRACE("Raw scan loaded.");		
		return CAT_SUCCESS;
	}
	return result;
}

bool MikesDemoWindow_Main::GetPointsFromRawScan(CATInt32 numScans, CATInt32 height, CATScanPoint* pointArray)
{
	if (pointArray == 0)
	{
		return false;
	}

	// calculate the rotated 3D points	from a raw scan array
	Clear3dPoints();
	CATInt32 i,j;

	fNum3dPoints = numScans * height;
	f3dPoints    = new CATC3DPoint[fNum3dPoints];


	for (i = 0; i < numScans; i++)
	{
		for (j=0; j < height; j++)
		{
			CATC3DPoint* point3d	= 0;
			CATInt32		curPos	= (i * height) + j;

			// Skip blank points
			if ((pointArray[curPos].y == 0) && (pointArray[curPos].z == 0))
			{
				continue;
			}

			// Check for points below stage and ignore
			if (pointArray[curPos].y > kBottomLine)
			{
				continue;
			}

			// Check for points outside of table diameter
			if ((pointArray[curPos].z > kMaxZ) ||
				 (pointArray[curPos].z < kMinZ))
			{
				continue;
			}

			f3dPoints[curPos].FromScannedPolar(	pointArray[curPos].y, 
															pointArray[curPos].z, 
															pointArray[curPos].rotation, 
															pointArray[curPos].color);				
		}
	}

	CATTRACE((CATString)"Got " << fNum3dPoints << " points from raw scan.");	
	return true;
}

void MikesDemoWindow_Main::Clear3dPoints()
{
	if (f3dPoints != 0)
	{
		delete [] f3dPoints;
		f3dPoints = 0;
	}
	fNum3dPoints = 0;
}
