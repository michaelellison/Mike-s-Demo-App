//---------------------------------------------------------------------------
/// \file CATControlWnd.h
/// \brief Window / OS based control class - base object for OS dependant 
/// controls.
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
#ifndef _CATControlWnd_H_
#define _CATControlWnd_H_

#include "CATControl.h"

class CATWindow;
class CATControlWnd;

/// \class CATControlWnd
/// \brief Window / OS based control class - base object for OS dependant 
/// controls.
/// \ingroup CATGUI
class CATControlWnd : public CATControl
{
   public:
      /// Constructor - mirrors CATXMLObject() constructor for now.
      CATControlWnd(  const CATString&               element, 
                      const CATString&               rootDir);

      virtual ~CATControlWnd();

      virtual CATWND GetControlWndHndl()
      {
         return fControlWnd;
      }
      
      /// SetFocused() sets the control's focused state.
      ///
      /// \param focused - if true, then the control is given focus.
      ///                  if false, focus is removed.      
		virtual void        SetFocused(bool focused);

      /// MarkDirty() marks the control as dirty (needing to be redrawn)
      ///
      /// \param dirtyRect - if specified, marks only part of the rectangle
		/// \param force - if true, forces invalidation even if the control isn't visible
      virtual void        MarkDirty(CATRect* dirtyRect = 0, bool force = false);

      /// Draw() draws the control into the parent's image
      /// \param image - image to draw into
      /// \param dirtyRect - portion of control (in window coordinates)
      ///        that requires redrawing.
      virtual void Draw(CATImage* image, const CATRect& dirtyRect);

      /// Event handler
      virtual CATResult OnEvent(const CATEvent& event, CATInt32& retVal);

      /// PostDraw() draws any stuff that requires an OS-specific draw
      /// context.
      virtual void   PostDraw(CATDRAWCONTEXT drawContext, const CATRect& dirtyRect);

      // Retrieve post-draw exclusion rects
      virtual bool         GetPostRects(CATStack<CATRect>& rectStack);

      /// OnParentCreate() is called when the parent window is created.
      /// 
      /// Most controls won't need this, but any that create their own
      /// windows should do so at this point.
      virtual void   OnParentCreate();

      /// OnParentDestroy() is called as the parent window is destroyed.
      ///
      /// Controls that create their own windows during OnParentCreate()
      /// should destroy them during OnParentDestroy()
      virtual void   OnParentDestroy();

		virtual void   SetVisible(bool visible);

      /// RectFromAttribs() recalculates the control's rectangle from
      /// the attributes.  This can only be called after ParseAttributes() has
      /// loaded the images.
      virtual CATResult    RectFromAttribs();
      
      /// GetColorFore() retrieves the foreground color for the control
      /// \return CATColor - foreground color
      virtual CATColor     GetColorFore() const;

      /// GetColorBack() retrieves the background color for the control
      /// \return CATColor - background color
      virtual CATColor     GetColorBack() const;


      /// Win-32 specific control stuff
      #ifdef CAT_CONFIG_WIN32
         HBRUSH   GetBGBrush();

         static LRESULT WINAPI CustomControlProc( HWND hWnd, 
                                                   UINT message, 
                                                   WPARAM wParam, 
                                                   LPARAM lParam);

         // Foreground brush
         HBRUSH   fFGBrush;
         HBRUSH   fFGFocBrush;
         HBRUSH   fFGDisBrush;

         // Background brush
         HBRUSH   fBGBrush;         
         HBRUSH   fBGFocBrush;
         HBRUSH   fBGDisBrush;

         


         /// Old window proc
         void* fOldWndProc;

         CATString fWindowType;
         CATUInt32  fWindowStyle;
      
      #endif // CAT_CONFIG_WIN32

      /// Load() loads the skin in
      virtual        CATResult Load(	CATPROGRESSCB				progressCB   = 0,
												void*							progressParam= 0,
												CATFloat32						progMin      = 0.0f,
												CATFloat32						progMax		 = 1.0f);   

      /// OnEscapeChange() is called if a control looses focus by
      /// the user pressing the ESC key.  If the control supports it,
      /// it should reset the value to its previous one.
      virtual void            OnEscapeChange() {}

      virtual void   SetEnabled(bool enabled);
      virtual void   OSSetText(const CATString& newText);
      virtual void   OSGetText(CATString& getText);

      virtual CATString GetHint() const;

   protected:            
      CATRect   fWndRect;
      
      /// Draw a border around the control
      virtual void OSDrawBorderRect(CATDRAWCONTEXT drawContext, CATRect& rect);

      /// ParseAttributes() parses the known attributes for an object.
      virtual CATResult ParseAttributes();

      /// OS-specific control creation / cleanup
      virtual CATResult OSCreate();
      virtual void OSCleanup();

      CATWND       fControlWnd;

      // member event handler that can be overridden.
      // returns true if the event is handled by the control.
      virtual bool OnControlEvent(const CATEvent& event, CATInt32& returnVal);

      // Create the control's window
      CATResult CreateControlWnd( const CATString&   windowClass, 
                                 CATUInt32           style );

            
      /// Foreground color - selected
      CATColor     fFGColor_focus;
      CATColor     fBGColor_focus;

      /// Foreground color - border
      CATColor     fFGColor_border;
      CATColor     fBGColor_border;

      bool        fFocusSet;
      bool        fBorderSet;

      bool        fCaptured;
};

#endif // _CATControlWnd_H_


