//---------------------------------------------------------------------------
/// \file CATBrowser_Win32.cpp
/// \brief Browser GUI control (windows)
/// 
/// Copyright (c) 2003 by Michael Ellison
/// All Rights Reserved.
///
/// $Author: mike $
/// $Revision: 4 $
/// $RCSFile: $
/// $Date: 2011-06-01 07:28:30 -0500 (Wed, 01 Jun 2011) $
//
//---------------------------------------------------------------------------
#include "CATBrowser.h"
#include "CATWindow.h"
#include "CATEventDefs.h"
#include "CATApp.h"

CATBrowser::CATBrowser( const CATString&               element, 
								const CATString&               rootDir)
: CATControlWnd(element, rootDir)
{
   fDefaultURL          =  "";
}

//---------------------------------------------------------------------------
/// CATBrowser destructor
//---------------------------------------------------------------------------
CATBrowser::~CATBrowser()
{

}


void CATBrowser::OnParentCreate()
{
	
}

void CATBrowser::OnParentDestroy()
{
   if (fControlWnd)
   {
      // Already destroyed from browser...
      fControlWnd = 0;
   }
      
}

CATResult CATBrowser::RectFromAttribs()
{
   // Browser shouldn't have border currently...
   CATResult result = CATControl::RectFromAttribs(); 
   fWndRect = this->GetRectAbs();

   if (this->fControlWnd)
   {
      this->GetWindow()->OSMoveWnd(fWndRect, fControlWnd);      
   }
   
   this->MarkDirty();   
	
   return result;
}

CATResult CATBrowser::Refresh()
{
   return CAT_SUCCESS;
}

CATResult CATBrowser::Go(const CATString& url)
{
   return CAT_SUCCESS;
}

CATResult CATBrowser::Stop()
{
   return CAT_SUCCESS;
}

CATResult CATBrowser::Back()
{
   return CAT_SUCCESS;
}

CATResult CATBrowser::Forward()
{
   return CAT_SUCCESS;
}

CATString CATBrowser::GetTitle()
{
   CATString title;
   return title;
}

CATString CATBrowser::GetURL()
{
   CATString url;
   return url;
}

CATWND CATBrowser::GetWnd()
{
	return 0;
}

CATResult CATBrowser::Print()
{
   return CAT_SUCCESS;
}

CATResult CATBrowser::Zoom(int zoomLevel)
{
   return CAT_SUCCESS;
}

CATResult CATBrowser::Show(bool show)
{
   return CAT_SUCCESS;
}

int CATBrowser::GetZoom()
{
	return 0;
}

int CATBrowser::GetZoomRange()
{
	return 0;
}

