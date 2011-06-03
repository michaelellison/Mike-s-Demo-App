// ------------------------------------------------------------------
// CAT3DView.h
// Copyright (c) 2002-2011 Mike Ellison. 
// Quick hack to wedge it from old GUI to new.... needs work.
// ------------------------------------------------------------------
#ifndef CAT3DView_H
#define CAT3DView_H
#include "CATPoint.h"
#include "CATControlWnd.h"

#include <gl\gl.h>

class CAT3DView : public CATControlWnd
{
public:
	CAT3DView(	const CATString&               element, 
					const CATString&               rootDir);

	virtual ~CAT3DView();

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

	void GetMinMaxBox(CATFloat32& minX, CATFloat32& minY, CATFloat32& minZ,
		CATFloat32& maxX, CATFloat32& maxY, CATFloat32& maxZ)
	{
		minX = fMinX; minY = fMinY; minZ = fMinZ; maxX = fMaxX; maxY = fMaxY; maxZ = fMaxZ;
	}

	void GetViewPos(  CATFloat32& viewX, CATFloat32& viewY, CATFloat32& viewZ,
		CATFloat32& rotX, CATFloat32& rotY, CATFloat32& rotZ)
	{
		viewX = fViewX; viewY = fViewY; viewZ = fViewZ; rotX = fViewRotX; rotY = fViewRotY; rotZ = fViewRotZ;
	}

	virtual bool OnControlEvent(const CATEvent& event, CATInt32& returnVal);

protected:
	void On3DMouseMove(CATFloat32 xOff, CATFloat32 yOff, bool leftBtn, bool midBtn, bool rightBtn);
	ATOM		fWindowAtom;
	HWND		fHwnd;
	HDC		fHDC;
	HGLRC		fHRC;
	int		fPixelFormat;
	GLuint	fDisplayList;
	GLuint	fAxisList;

	bool		fTracking;

	CATFloat32  fRotateSpeed;
	// Viewport positions
	CATFloat32	fViewX;
	CATFloat32	fViewY;
	CATFloat32	fViewZ;
	CATFloat32	fViewRotX;
	CATFloat32	fViewRotY;
	CATFloat32	fViewRotZ;

	CATFloat32  fMinX;
	CATFloat32  fMaxX;
	CATFloat32  fMinY;
	CATFloat32  fMaxY;
	CATFloat32  fMinZ;
	CATFloat32  fMaxZ;

	bool		f3DXYTranslating;
	bool		f3DXYRotating;
	bool		f3DZTranslating;
	bool		f3DZRotating;

	bool		fFullScreen;
	bool		fStereoView;

	RECT		fOrgRect;

	CATInt32	fLastMouseX;
	CATInt32	fLastMouseY;


	void CenterViewport();

	// Window procedure for 3D views
	static LRESULT CALLBACK CAT3DViewProc(   HWND hwnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam);
};

#endif //CAT3DView_H