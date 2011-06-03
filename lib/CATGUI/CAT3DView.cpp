// ------------------------------------------------------------------
// CAT3DView.cpp
// Copyright (c) 2002-2011 Mike Ellison. 
// Quick hack to wedge it from old GUI to new.... needs work.
// ------------------------------------------------------------------

#include "CAT3DView.h"
#include "CATWindow.h"
#include "CATCursor.h"
#include "CATUtil.h"
#include "CATEventDefs.h"

// Include opengl libs for 3D
#include "gl\gl.h"
#include "gl\glu.h"

CATFloat32 kMinRotate = 0.0001f;
int kRotateTimer = 100;

CAT3DView::CAT3DView(   const CATString&             element, 
								const CATString&             rootDir)
							: CATControlWnd(element,  rootDir)
{   
	// Defaults
	// Register a window class for the 3D view
	fWindowAtom = 0;
	fDisplayList = -1;
	fAxisList	 = -1;
	fRotateSpeed  = 0.0f;
	f3DXYTranslating	=	false;		// Are we moving the view?
	f3DXYRotating		=	false;		// Rotating it?
	f3DZTranslating	=	false;		// Translating on Z?
	f3DZRotating		=	false;		// Rotating on Z?
	fStereoView			=	false;		// Stereo mode?
	fFullScreen			=	false;		// Full screen mode?
	fTracking = false;

	fLastMouseX			= 0;				// Last known position of the mouse with a button down
	fLastMouseY			= 0;

	memset(&fOrgRect,0,sizeof(RECT));
	// Start out at origin for view
	fViewX	 = fViewY	 = fViewZ	 = 0.0;
	fViewRotX = fViewRotY = fViewRotZ = 0.0;
	fCursor.SetType(CATCURSOR_HAND);
}

// Destructor
CAT3DView::~CAT3DView()
{		
	// Kill the display list if we got one
	if (fDisplayList != -1)
	{
		glDeleteLists(fDisplayList,1);	
	}

	if (fAxisList != -1)
	{
		glDeleteLists(fAxisList,1);
	}
}

// We draw the 3D stuff after the main CodeRock GUI draws, so that
// it won't be scaled with the other stuff.
//
void CAT3DView::PostDraw(	CATDRAWCONTEXT		hdc, 
									const CATRect&		updateRect)
{
	HDC odc;
	PAINTSTRUCT ps;
	// Begin a paint operation
	odc = BeginPaint(fHwnd, &ps);

	// Make the OpenGL RC current for drawing
	::wglMakeCurrent(odc,fHRC);	

	// Clear the window - we should double buffer here....
	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Store the current matrix - we should be in modelview mode right now
	::glPushMatrix();

	// Translate first, then rotate Z/Y/X to our current camera view
	::glTranslatef(fViewX, fViewY, fViewZ);
	::glRotatef(fViewRotZ,0,0,1);	
	::glRotatef(fViewRotY,0,1,0);
	::glRotatef(fViewRotX,1,0,0);


	 GLfloat ambient[] = {1.6f, 1.6f, 1.6f, 1.0f};
	 glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

	 GLfloat lightColor1[] = {1.0f, 1.0f, 1.0f, 1.0f};
	 GLfloat lightPos1[] = {fViewX, fViewY, fMaxZ, 0.0f};
	 glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor1);
	 glLightfv(GL_LIGHT0, GL_POSITION, lightPos1);


	// Draw anything in our display list
	if (fDisplayList != -1)
	{
		::glCallList(fDisplayList);
	}

	if (fAxisList != -1)
	{
		::glCallList(fAxisList);
	}

	// Restore the default view matrix
	::glPopMatrix();

	// Flush out all drawing commands
	::glFlush();

	::SwapBuffers(odc);

	// End the session with the OpenGL RC
	::wglMakeCurrent(NULL,NULL);

	// End Paint session
	EndPaint(fHwnd,&ps);
}

// This is more or less our OnSize() event, 'cept it's different
// for Post-drawn controls as they don't have the scaling and such
// applied to them like all the other controls do...
CATResult CAT3DView::RectFromAttribs()
{	
	CATResult res = CATControlWnd::RectFromAttribs();
	
	CATRect rect = this->GetRectAbs();
	// Move the 3D window to our new location - don't redraw it yet
	MoveWindow(	fHwnd,		
		rect.left,
		rect.top, 
		rect.Width(),
		rect.Height(),
		FALSE);

	// Make the OpenGL RC current for our window
	HDC hdc = GetDC(fHwnd);		
	::wglMakeCurrent(hdc,fHRC);
	GLsizei glw, glh;

	glw = rect.Width();
	glh = rect.Height();

	// Reset the viewport, persepective, and matrix
	::glViewport(0,0,glw,glh);	
	::glMatrixMode(GL_PROJECTION);
	::glLoadIdentity();
	::gluPerspective(45.0,
		(CATFloat64)glw/(CATFloat64)glh,
		fabs(fMaxZ-fMinZ)/10,			// 1 to 100 for viewing. Hey, sounds good to me ;)
		fabs(fMaxZ-fMinZ)*10);		// might actually calc this or something later, but for objects
	// with a 12" z-depth, should work pretty well...

	// Switch to model view for our next draw, and reset the matrix
	// We perform the view translations/rotations during the draw.
	::glMatrixMode(GL_MODELVIEW);
	::glLoadIdentity();
	 

	// Release the RC and DC
	::wglMakeCurrent(NULL,NULL);	
	ReleaseDC(fHwnd, hdc);
	return res;
}


bool CAT3DView::OnControlEvent(const CATEvent& eventMsg, CATInt32& returnVal)
{
	switch (eventMsg.fEventCode)
	{
		case CATEVENT_WINDOWS_EVENT:
			{
				returnVal = (CATInt32)CAT3DViewProc(
							(HWND)eventMsg.fIntParam1,
							(UINT)eventMsg.fIntParam2,
							(WPARAM)eventMsg.fIntParam3,
							(LPARAM)eventMsg.fIntParam4);
			}
			break;

		default:
			break;
	}
	return false;
}
// This is the raw Win32 window proc for the 3D view window.
// Handle mousing and stuff here...
LRESULT CALLBACK CAT3DView::CAT3DViewProc(   HWND hwnd,
														UINT uMsg,
														WPARAM wParam,
														LPARAM lParam)
{
	// Get our "this" pointer from the window. WARNING: will not be true on WM_CREATE
	// (it is set on WM_CREATE though from the datastructure in lParam for window creation)
	CAT3DView* wnd3d = (CAT3DView*)GetWindowLongPtr(hwnd,GWLP_USERDATA);


	switch (uMsg)
	{
		// Mouse has left the building.
		// Bail out of any mouse-down stuff we were doing.
	case WM_KILLFOCUS:
		wnd3d->f3DXYTranslating		= false;
		wnd3d->f3DXYRotating			= false;
		wnd3d->f3DZTranslating		= false;
		wnd3d->f3DZRotating			= false;
		break;
		// This gets called anytime the mouse moves within our window
		// check button-down motions
	case WM_MOUSEMOVE:
		{
			// Find the XY offsets from the last location of the mouse
			CATFloat32 xOff = (CATFloat32)(((CATInt16)LOWORD(lParam) - wnd3d->fLastMouseX));
			CATFloat32 yOff = (CATFloat32)(((CATInt16)HIWORD(lParam) - wnd3d->fLastMouseY));

			// Figure out what to do
			wnd3d->On3DMouseMove(
				xOff,
				yOff, 
				(wParam & MK_LBUTTON) > 0, 
				(wParam & MK_MBUTTON) > 0, 
				(wParam & MK_RBUTTON) > 0);

			// Save the last mouse position
			wnd3d->fLastMouseX = (CATInt16)LOWORD(lParam);
			wnd3d->fLastMouseY = (CATInt16)HIWORD(lParam);
			return TRUE;
		}
		break;
	case WM_TIMER:
		if ((wParam == kRotateTimer) && (wnd3d->fRotateSpeed > kMinRotate))
		{
			// This time, the ratios are based from a full circle / window size
			CATFloat32 ratio = (360.0f) / (CATFloat32)(wnd3d->fRect.right - wnd3d->fRect.left);
			wnd3d->fViewRotY += wnd3d->fRotateSpeed * ratio;
			while (wnd3d->fViewRotY > 360)	
				wnd3d->fViewRotY -= 360;
			wnd3d->MarkDirty();
		}
		break;
	case WM_CREATE:
		// Store our "this" pointer with the window
		{
			LPCREATESTRUCT lpc = (LPCREATESTRUCT)lParam;
			SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG)lpc->lpCreateParams);

			// SetWindowLong's don't take effect until the next SetWindowPos, so flush it now
			// before we get a message that might want to use the this pointer...
			SetWindowPos(hwnd,0,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOZORDER|SWP_NOSIZE);
			::SetTimer(hwnd,kRotateTimer,40,0);
		}			
		return 0;
		break;


		// Yukky blinkies go away.
	case WM_ERASEBKGND:
		return TRUE;
		break;

	case WM_DESTROY:
		// When the window is destroyed, we should destroy our
		// OpenGL context as well.
		::KillTimer(hwnd,kRotateTimer);
		::wglMakeCurrent(0,0);
		::wglDeleteContext(wnd3d->fHRC);
		wnd3d->fHRC = 0;
		return 0;
		break;
	}

	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

void CAT3DView::SetRotateSpeed(CATFloat32 speed)
{
	fRotateSpeed = speed;
}


// Handle mouse movements / buttons in 3D window
void CAT3DView::On3DMouseMove(CATFloat32 xOff, CATFloat32 yOff, bool leftBtn, bool midBtn, bool rightBtn)
{
	CATFloat32 xRatio;	// x-motion ratio, generally a ratio of "What we're adjusting" / "Width of Window Mouse is in"
	CATFloat32 yRatio;	// y-motion ratio, generally a ratio of "What we're adjusting" / "Height of Window Mouse is in"
	CATFloat32 ratio;		// If x and y adjust similar stuff, we might want to pick a speed that is somewhere between
	// the two but equal so things work as expected for the operator...

	bool wasTracking = fTracking;

	// Calc width/height of the view rectangle
	CATInt32  wndWidth	= this->fRect.right - this->fRect.left;
	CATInt32  wndHeight = this->fRect.bottom - this->fRect.top;
	// right mouse button controls normal translation on XY axis
	if (rightBtn)
	{
		// If we just put the button down, then don't translate anything yet,
		// just set the location for the last mouse position and start the
		// translation process...
		if (!f3DXYTranslating) 
		{
			f3DXYTranslating = true;			
		}
		else
		{
			// xRatio is the (width of the object) / (width of view window)
			// yRatio is the (height of object) / (height of view window)
			//
			// With these ratios, we can move the mouse exactly one screen to move the object across the screen.
			// However, if we got a long skinny object, this "feels" weird, so we take the average of the two.
			xRatio = (fMaxX - fMinX) / (CATFloat32)(wndWidth);
			yRatio = (fMaxY - fMinY) / (CATFloat32)(wndHeight);

			// Average ratio
			ratio = (xRatio + yRatio)/2;

			// Translate XY by our mouse offsets * ratio.
			// Note that for Y, the view's (0,0) point is in top-left, so we invert motion.
			fViewX += xOff*ratio;						
			fViewY -= yOff*ratio;				
		}					
	}
	else
	{
		// No more mouse button = no more translating.
		f3DXYTranslating = false;		
	}

	// Middle button - Z translation
	// Z translation is performed similarly to XY translation,
	// only it's just on the Z axis.
	//
	// Note that if both the middle AND the left button are down, we're going to do
	// Z-rotation instead (further down in dah code...)
	if (midBtn && !(leftBtn))
	{
		if (!f3DZTranslating) 
		{
			f3DZTranslating = true;
			f3DZRotating	  = false;
		}
		else
		{											
			// We allow motion on either X or Y by the mouse to change Z
			xRatio = (fMaxZ - fMinZ) / (CATFloat32)(wndWidth);
			yRatio = (fMaxZ - fMinZ) / (CATFloat32)(wndHeight);

			// Translation on Z = xMovement*xRatio + yMovement*yRatio
			fViewZ += ((xOff * xRatio) + (yOff*yRatio));
		}					
	}
	else
	{
		f3DZTranslating = false;
	}

	// Now for rotational fun - 
	// Left button (alone) performs rotation on the X and Y Axis
	// Note: if both left and middle are down, we do Z-Rotation (see farther below...)
	if (leftBtn && !(midBtn))
	{
		if (!f3DXYRotating) 
		{						
			f3DXYRotating	  = true;
			f3DZRotating	  = false;
		}
		else
		{					
			// This time, the ratios are based from a full circle / window size
			xRatio = (360.0f) / (CATFloat32)(wndWidth);
			yRatio = (360.0f) / (CATFloat32)(wndHeight);
			ratio = (xRatio + yRatio)/2;

			// Add rotations to our view
			fViewRotX += yOff * ratio;
			fViewRotY += xOff * ratio;

			// Keep the rotations legal in degrees
			// We use while in case for some reason the mouse movement
			// was really really big - probably not necessary, but not
			// particularly costly either.
			while (fViewRotX > 360)	fViewRotX -= 360;
			while (fViewRotX < 0)	fViewRotX += 360;
			while (fViewRotY > 360)	fViewRotY -= 360;
			while (fViewRotY < 0)	fViewRotY += 360;
		}
	}
	else
	{
		// End rotating stuff.
		f3DXYRotating = false;
	}

	// Z-Rotation
	// Left button with Middle button performs rotation on the Z Axis				
	if (leftBtn && midBtn)
	{
		if (!f3DZRotating) 
		{
			f3DZTranslating = false;
			f3DXYRotating	  = false;
			f3DZRotating	  = true;
		}
		else
		{					
			// This time, the ratios are based from a full circle / window size
			xRatio = (360.0f) / (CATFloat32)(wndWidth);
			yRatio = (360.0f) / (CATFloat32)(wndHeight);

			// Z rotation is a combination of y movement and x movement
			fViewRotZ += (yOff * yRatio) + (xOff * xRatio);						

			// Keep the rotations legal in degrees
			while (fViewRotZ > 360)	fViewRotZ -= 360;
			while (fViewRotZ < 0)		fViewRotZ += 360;
		}
	}
	else
	{
		// End rotating stuff.
		f3DZRotating = false;
	}


	// If we're doing something, force a redraw.
	if (f3DXYRotating || f3DXYTranslating || f3DZTranslating || f3DZRotating)
	{
		MarkDirty();
	}

	if (!wasTracking)
	{
		if (leftBtn || rightBtn || midBtn)
		{
			fTracking = true;
			::SetCapture(fControlWnd);
		}
	}
	else
	{
		if (fTracking)
		{
			if (!(leftBtn || rightBtn || midBtn))
			{
				::ReleaseCapture();
				fTracking = false;
			}
		}
	}

}



// Center the viewport to our object
void CAT3DView::CenterViewport()
{
	// Zero rotations
	this->fViewRotX = 0.0;
	this->fViewRotY = 0.0;
	this->fViewRotZ = 0.0;

	// Look at center, a little back on Z
	this->fViewX = (fMaxX + fMinX)/2;
	this->fViewY = (fMaxY + fMinY)/2;
	this->fViewZ = (CATFloat32)((fMinZ - fabs((fMaxZ - fMinZ))*2 ));
}

void CAT3DView::SetAxisDisplay(bool on, bool corner)
{
	// Connect to the OpenGL RC
	HDC hdc = GetDC(this->fHwnd);	
	::wglMakeCurrent(hdc,fHRC);

	// Delete old axis display
	if (fAxisList != -1)
	{
		::glDeleteLists(fAxisList,1);	
		fAxisList = -1;
	}

	// If we are just turning it off, bail now.
	if (on == false)
	{
		return;
	}

	// Create a new list
	fAxisList = ::glGenLists(1);

	// Start list compiling
	::glNewList(fAxisList,GL_COMPILE);
	{		
		::glBegin(GL_LINES);
		// Draw vertex lines

		// X-axis
		glColor3f(1,0,0);
		glVertex3f(0,0,0);
		glVertex3f(12,0,0);

		// Y - axis
		glColor3f(0,1,0);
		glVertex3f(0,0,0);
		glVertex3f(0,12,0);

		// Z - axis
		glColor3f(0,0,1);
		glVertex3f(0,0,0);
		glVertex3f(0,0,12);

		::glEnd();
		// End the list operations
	} ::glEndList();

	// Release OpenGL and DC
	::wglMakeCurrent(NULL,NULL);
	ReleaseDC(this->fHwnd, hdc);
}

// Auto-facet for scanned information
void CAT3DView::Set3dFacets(	CATC3DPoint* pointArray, CATInt32 numScans, CATInt32 height, bool triangles)
{
	// Connect to the OpenGL RC
	HDC hdc = GetDC(this->fHwnd);	
	::wglMakeCurrent(hdc,fHRC);

	// Nuke the old display list if there is one
	if (fDisplayList != -1)
	{
		::glDeleteLists(fDisplayList,1);	
		fDisplayList = -1;
	}

	if ((pointArray == 0) || (numScans == 0) || (height == 0))
	{
		return;
	}

	// Create a new list
	fDisplayList = ::glGenLists(1);

	CATC3DPoint* curPoint = pointArray;

	// We want to track our min/max positions of the object - 
	// start 'em out here or bail if we can't get the point.
	fMinX = fMaxX = (CATFloat32)curPoint->x;
	fMinY = fMaxY = -(CATFloat32)curPoint->y;
	fMinZ = fMaxZ = (CATFloat32)curPoint->z;

	// Start list compiling
	::glNewList(fDisplayList,GL_COMPILE);
	{		
		::glBegin(GL_QUADS);
		{		
			// Loop through all the points in the list and
			// store 'em into the display list. Also
			// compile our min/max locations for the object.
			for (CATInt32 x = 0; x < numScans - 1; x++)
			{
				for (CATInt32 y = 0; y < height - 1; y++)
				{
					CATC3DPoint *tlPoint, *trPoint, *blPoint, *brPoint;
					tlPoint = &pointArray[x*height + y];
					trPoint = &pointArray[(x + 1)*height + y];
					blPoint = &pointArray[x*height + y + 1];
					brPoint = &pointArray[(x + 1)*height + y + 1];

					if ((tlPoint->x == 0) && (tlPoint->y == 0) && (tlPoint->z == 0))	continue;
					if ((trPoint->x == 0) && (trPoint->y == 0) && (trPoint->z == 0))	continue;
					if ((blPoint->x == 0) && (blPoint->y == 0) && (blPoint->z == 0))	continue;
					if ((brPoint->x == 0) && (brPoint->y == 0) && (brPoint->z == 0))	continue;

					::glColor3f( ((GLfloat)(tlPoint->color.r)/255.0f),
						((GLfloat)(tlPoint->color.g)/255.0f),
						((GLfloat)(tlPoint->color.b)/255.0f));

					// Set location
					::glVertex3f(	 (CATFloat32)tlPoint->x, 
						-(CATFloat32)tlPoint->y, 
						(CATFloat32)tlPoint->z );

					::glColor3f( ((GLfloat)(trPoint->color.r)/255.0f),
						((GLfloat)(trPoint->color.g)/255.0f),
						((GLfloat)(trPoint->color.b)/255.0f));

					// Set location
					::glVertex3f(	 (CATFloat32)trPoint->x, 
						-(CATFloat32)trPoint->y, 
						(CATFloat32)trPoint->z );

					::glColor3f( ((GLfloat)(brPoint->color.r)/255.0f),
						((GLfloat)(brPoint->color.g)/255.0f),
						((GLfloat)(brPoint->color.b)/255.0f));

					// Set location
					::glVertex3f(	 (CATFloat32)brPoint->x, 
						-(CATFloat32)brPoint->y, 
						(CATFloat32)brPoint->z );

					::glColor3f( ((GLfloat)(blPoint->color.r)/255.0f),
						((GLfloat)(blPoint->color.g)/255.0f),
						((GLfloat)(blPoint->color.b)/255.0f));

					// Set location
					::glVertex3f(	 (CATFloat32)blPoint->x, 
						-(CATFloat32)blPoint->y, 
						(CATFloat32)blPoint->z );


					// Track min/max
					fMinX = min(fMinX,(CATFloat32)tlPoint->x);
					fMaxX = max(fMaxX,(CATFloat32)tlPoint->x);
					fMinY = min(fMinY,-(CATFloat32)tlPoint->y);
					fMaxY = max(fMaxY,-(CATFloat32)tlPoint->y);
					fMinZ = min(fMinZ,(CATFloat32)tlPoint->z);
					fMaxZ = max(fMaxZ,(CATFloat32)tlPoint->z);

				}				
			}			
		}
		// End the points operations
		::glEnd();
		// End the list operations
	} ::glEndList();

	// Center the viewport to look at the object
	RectFromAttribs();
	CenterViewport();

	// Release OpenGL and DC
	::wglMakeCurrent(NULL,NULL);
	ReleaseDC(this->fHwnd, hdc);
}

void CAT3DView::OnParentCreate()
{
	fWindowType = "CAT3DView";

	// Register a window class for us..
	// Set up the Win32 structure for creating a window for our 3D OpenGL view
	WNDCLASS wc;
	wc.style				= CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc		= CAT3DViewProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= gApp->GetInstance();
	wc.hIcon				= NULL;
	wc.hCursor			= 0;
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName	= 0;
	wc.lpszClassName	= fWindowType;
	fWindowAtom			= RegisterClass(&wc);

	CATControlWnd::OnParentCreate();
	fHwnd = CATControlWnd::GetControlWndHndl();

	// Now create the OpenGL objects needed for this window....
	// Why telling it I want a damned color pixel takes so many zeros, 
	// I may never understand...
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0,0,0,
		0,0,0,
		0,0,
		0,0,0,0,0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0,
		0,
		0};

		// Set pixel format and get an openGL RC
		HDC hdc = GetDC(fHwnd);
		// Pick our pixel format for the DC and set it
		fPixelFormat = ::ChoosePixelFormat(hdc,&pfd);
		SetPixelFormat(hdc,fPixelFormat,&pfd);

		// Create the openGL context and store
		fHRC = ::wglCreateContext(hdc);

		// Make it current
		::wglMakeCurrent(hdc,fHRC);

		// Set our clear color to 0
		::glClearColor(0.0f,0.0f,0.0f,0.0f);
		::glClearDepth(1.0f);


		// Enable depth checking and smooth points at size 2
		::glEnable(GL_DEPTH_TEST);
		::glEnable(GL_POINT_SMOOTH);

 		 glEnable(GL_COLOR_MATERIAL);
		 glEnable(GL_DEPTH_TEST);
		 glEnable(GL_BLEND);
		 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		 glEnable(GL_LIGHTING);
		 glEnable(GL_LIGHT0);
		 glShadeModel(GL_SMOOTH);

		::glPointSize(2);

		// Unset the context
		::wglMakeCurrent(0,0);

		ReleaseDC(fHwnd, hdc);
}