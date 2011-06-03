//---------------------------------------------------------------------------
/// \file CATBrowser.h
/// \brief Browser GUI control
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
#ifndef _CATBrowser_H_
#define _CATBrowser_H_

#include "CATControlWnd.h"

//----------------------------------------------------------------------------
class CATBrowser;
class CATBrowser : public CATControlWnd
{
   public:
      CATBrowser(  	const CATString&               element, 
							const CATString&               rootDir);


      virtual ~CATBrowser();          
      
      virtual void OnParentCreate();
      virtual void OnParentDestroy();            

      /// Event handler
      virtual CATResult OnEvent(const CATEvent& event, CATInt32& retVal);

      // Public browser interface      
      virtual CATResult     Refresh();
      virtual CATResult     Go(const CATString& url);
      virtual CATResult     Stop();
      virtual CATResult     Back();
      virtual CATResult     Forward();
      virtual CATString     GetTitle();
      virtual CATString     GetURL();
      virtual CATWND        GetWnd();
      virtual CATResult     Print();
      virtual CATResult     Zoom(int zoomLevel);
      virtual int          GetZoom();
      virtual int          GetZoomRange();
      virtual void         PostDraw(CATDRAWCONTEXT drawContext, const CATRect& dirtyRect);
      virtual CATResult     Show(bool show);

      
      // escape a string for use in a url or similar...
      virtual CATString     EscapeString( const CATString& inputStr);

   protected:

      /// RectFromAttribs() recalculates the control's rectangle from
      /// the attributes.  This can only be called after TranslateAttribs() has
      /// loaded the images.
      virtual CATResult    RectFromAttribs();

      /// TranslateAttribs() parses the known attributes for an object.
      virtual CATResult ParseAttributes();
      CATString fDefaultURL;
};

#endif // _CATBrowser_H_


