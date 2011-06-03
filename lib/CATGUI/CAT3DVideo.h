// ------------------------------------------------------------------
// CAT3DVideo.h
// Copyright (c) 2002-2011 Mike Ellison. 
// Quick hack to wedge it from old GUI to new.... needs work.
// ------------------------------------------------------------------
#ifndef CAT3DVideo_H
#define CAT3DVideo_H
#include "CATInternal.h"
#include "CATPoint.h"
#include "CATControlWnd.h"
#include "CATCritSec.h"
#include "CATVideoCapture.h"
#include "CATMutex.h"
#include <gl\gl.h>


class CAT3DVideo : public CATControlWnd
{
public:
	CAT3DVideo(	const CATString&               element, 
					const CATString&               rootDir);

	virtual ~CAT3DVideo();

	/// RectFromAttribs() recalculates the control's rectangle from
	/// the attributes.  This can only be called after ParseAttributes() has
	/// loaded the images.
//	virtual CATResult    RectFromAttribs();


	/// PostDraw() draws any stuff that requires an OS-specific draw
	/// context.
	virtual void   PostDraw(	CATDRAWCONTEXT			drawContext, 
										const CATRect&			dirtyRect);

	virtual CATResult RectFromAttribs();
	virtual void OnParentCreate();

	// Auto-facet for scanned information. triangles == true uses triangles, otherwise uses quads
	virtual void Set3dFacets(	CATC3DPoint* pointArray, CATInt32 numScans, CATInt32 height, bool triangles = true);

	virtual void SetRotateSpeed(CATFloat32 speed);

	// Turn the axis display on or off. If on, and corner == true, then it displays in corner.
	// Otherwise, it displays inside the object at the origin.
	virtual void SetAxisDisplay(bool on, bool corner = false);
	virtual bool OnControlEvent(const CATEvent& event, CATInt32& returnVal);

	virtual CATResult	Start();
	virtual CATResult Stop();

	virtual void OnMouseWheel(    const CATPOINT& point,
										   CATFloat32        wheelMove,
											CATMODKEY			modKey);

protected:
	void CenterViewport();
	void On3DMouseMove(CATFloat32 xOff, CATFloat32 yOff, bool leftBtn, bool midBtn, bool rightBtn);

#ifdef _WIN32
	RECT		fOrgRect;
	// Window procedure for 3D views
	static LRESULT CALLBACK CAT3DVideoProc(   HWND hwnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam);
#endif

	// Video
	static void OnFrame(CATImage* frame, void* context);
	CATMutex				 fMutex;
	CATImage* fCurImage;
	CATVideoCapture	 fCapture;


	// 3D window / OpenGL
	ATOM		fWindowAtom;
	HWND		fHwnd;
	HDC		fHDC;
	HGLRC		fHRC;
	int		fPixelFormat;
	GLuint	fDisplayList;
	GLuint	fAxisList;


	// Viewport positions
	CATFloat32  fRotateSpeed;
	CATFloat32	fViewX;
	CATFloat32	fViewY;
	CATFloat32	fViewZ;
	CATFloat32	fViewRotX;
	CATFloat32	fViewRotY;
	CATFloat32	fViewRotZ;

	// Item min/max
	CATFloat32  fMinX;
	CATFloat32  fMaxX;
	CATFloat32  fMinY;
	CATFloat32  fMaxY;
	CATFloat32  fMinZ;
	CATFloat32  fMaxZ;

	// Flags for rotation
	CATInt32	fLastMouseX;
	CATInt32	fLastMouseY;
	bool		fTracking;
	bool		f3DXYTranslating;
	bool		f3DXYRotating;
	bool		f3DZTranslating;
	bool		f3DZRotating;
};

#endif //CAT3DVideo_H