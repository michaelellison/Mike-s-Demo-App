// ------------------------------------------------------------------
/// \file CATVideoCtrl.cpp
/// \brief Simple control for video display
///
// Copyright (c) 2011 Mike Ellison. See COPYING.txt
// ------------------------------------------------------------------

#include "CATVideoCtrl.h"
#include "CATWindow.h"
#include "CATCursor.h"
#include "CATUtil.h"
#include "CATEventDefs.h"
 
CATVideoCtrl::CATVideoCtrl(   const CATString&             element, 
										const CATString&             rootDir)
							: CATControl(element,  rootDir)
{   	
	fCurImage = 0;
}

// Destructor
CATVideoCtrl::~CATVideoCtrl()
{
	this->Stop();	
}

void CATVideoCtrl::OnParentDestroy()
{
	Stop();
	CATControl::OnParentDestroy();
}


void CATVideoCtrl::Draw(CATImage* image, const CATRect& dirtyRect)
{
    if (this->IsVisible() == false)
        return;

    // sanity check parent image / dirty rectangle
    CATRect imgRect(0,0,image->Width(), image->Height());
    CATASSERT(imgRect.Inside(dirtyRect), "Update rect is outside of img rect!");

    // Find intersection between dirty rect and us
    CATRect drawRect;
    bool   drawn = false;

    if (this->fRect.Intersect(dirtyRect, &drawRect))
    {  
        CATRect ourRect;

		  fCritSec.Wait();
		   if (!fCurImage)
			{
				image->FillRect(drawRect,CATColor(0,0,0,255));
			}
         else if (drawRect.Intersect(CATRect( fRect.left, 
                                         fRect.top, 
                                         fRect.left + fCurImage->Width(),
                                         fRect.top  + fCurImage->Height()),
													  &ourRect))
         {
             ourRect.Offset(-fRect.left, -fRect.top);

             image->CopyOver( fCurImage,
                             drawRect.left, 
                             drawRect.top, 
                             ourRect.left,
                             ourRect.top,
                             ourRect.Width(),
                             ourRect.Height());
		  }
		  fCritSec.Release();
	 }
}


void CATVideoCtrl::OnParentCreate()
{
	CATControl::OnParentCreate();
	fCapture.Init(GetWindow()->GetBaseWindow(),fRect, OnFrame,this);
}

CATResult CATVideoCtrl::Start()
{
	return fCapture.Start();
}

CATResult CATVideoCtrl::Stop()
{
	return fCapture.Stop();
}

void CATVideoCtrl::OnFrame(CATImage* frame, void* context)
{
	CATVideoCtrl* ctrl = (CATVideoCtrl*)context;
	ctrl->fCritSec.Wait();

	ctrl->fCurImage = frame;
	
	ctrl->fCritSec.Release();
	ctrl->MarkDirty();
}
