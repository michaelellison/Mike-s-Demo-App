/// \file  CATOverlayOpenGL.cpp
/// \brief OpenGL image overlay
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-06-02 02:47:03 -0500 (Thu, 02 Jun 2011) $
// $Revision: 7 $
// $NoKeywords: $

#include "CATOverlayOpenGL.h"
#include "CATString.h"

#ifdef CAT_CONFIG_WIN32
#include <gl/gl.h>

/// This table contains all of the functions we wish to intercept
/// from IDirect3DDevice9 and the targets for those functions.
CATINTERCEPT_DLL_TABLE_ENTRY CATOverlayOpenGL::kOpenGLInterceptTable[] =
{
    { "wglSwapBuffers",        CATOverlayOpenGL::OnSwapBuffers,  5},

    // End marker
    {0,0,0}
};

CATOverlayOpenGL::CATOverlayOpenGL()
: CATOverlay()
{
    fTexScaleX       = 1.0f;
    fTexScaleY       = 1.0f;
    fTexture         = 0;
    fOpenGLDLL = ::LoadLibrary(L"opengl32.dll");
}

CATOverlayOpenGL::~CATOverlayOpenGL()
{
    RestoreAll();
    if (fOpenGLDLL)
        FreeLibrary(fOpenGLDLL);
    fOpenGLDLL = 0;
}


void CATOverlayOpenGL::DrawToScene(HDC hdc)
{  

    fLock.Wait();

    // Snag current screen size
    CATInt32 retVals[4];
    glGetIntegerv(GL_VIEWPORT,&retVals[0]);
    fRect.Set(retVals[0],retVals[1],retVals[2]+retVals[0],retVals[3]+retVals[1]);

    // If no overlay, just bail now.
    if (!fOverlay)
    {
        // Clean up if we've got an old texture.
        if (fOverlayDirty)
        {
            if (fTexture)
                glDeleteTextures(1,&fTexture);
            fTexture = 0;
            fOverlayDirty = false;
        }
        fLock.Release();
        return;
    }

    if (fOverlayDirty)
    {
        if (fTexture)
            glDeleteTextures(1,&fTexture);
        fTexture = 0;

        // calc min power of 2 that'll fit the image into a texture.        
        CATInt32 tw = 2;
        while (tw < fOverlay->Width())
            tw*= 2;
        CATInt32 th = 2;
        while (th < fOverlay->Height())
            th*=2;

        // Create our overlay texture
        glGenTextures(1,&fTexture);
        glBindTexture(GL_TEXTURE_2D,fTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        // Create a buffer to copy image into that's a power of 2 for texture happiness.
        CATUInt8* buffer = new CATUInt8[tw*th*4];
        CATUInt8* srcBuf = fOverlay->GetRawDataPtr();
        CATUInt32 imgWidth = fOverlay->Width();
        memset(buffer,0,tw*th*4);        
        for (int y = 0; y < fOverlay->Height(); y++)
        {
            memcpy(&buffer[y*tw*4],&srcBuf[y*imgWidth*4],4*imgWidth);
        }        
        // dump image into texture
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,tw,th,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
        
        // Preserve image/texture scale for drawing
        fTexScaleX = (CATFloat32)fOverlay->Width() /  (CATFloat32)tw;
        fTexScaleY = (CATFloat32)fOverlay->Height() / (CATFloat32)th;
        
        delete [] buffer;
    }

    // Save all the attributes and disable everything that would affect our drawing.
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Go to a nice, simple drawing mode.
    // Make sure to save out the matricies and restore them later.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, fRect.Width(), fRect.Height(), 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Disable anything that'd affect our drawing, enable 2D texture 
    glDisable(GL_DEPTH_TEST);
 	 glDisable(GL_LIGHTING);
	 glDisable(GL_TEXTURE_1D);
    glShadeModel(GL_FLAT);
    glEnable (GL_TEXTURE_2D);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_ONE, GL_ZERO);
    glEnable(GL_BLEND);

    // Calc scale of current display vs. overlay's virtual rect
    CATFloat32 scaleX = (CATFloat32)fRect.Width()  / (CATFloat32)this->fRefScreenWidth;
    CATFloat32 scaleY = (CATFloat32)fRect.Height() / (CATFloat32)this->fRefScreenHeight;
    
    if (fKeepAspect)
    {
        scaleY = scaleX = CATMin(scaleX,scaleY);
    }

    // Draw scaled overlay to requested location
    glBegin(GL_QUADS);
    {
        glColor4f (1.0f,1.0f,1.0f, 1.0f);//fAlpha);
        
        glTexCoord2f(0, 0);        
        
        glVertex3f(  fOverlayRect.left   * scaleX,  
                     fOverlayRect.top    * scaleY, -1);

        glTexCoord2f(fTexScaleX, 0); 
        
        glVertex3f(  fOverlayRect.right  * scaleX,  
                     fOverlayRect.top    * scaleY, -1);

        glTexCoord2f(fTexScaleX, fTexScaleY); 

        glVertex3f(  fOverlayRect.right  * scaleX,  
                     fOverlayRect.bottom * scaleY, -1);

        glTexCoord2f(0, fTexScaleY); 

        glVertex3f(  fOverlayRect.left   * scaleX,  
                     fOverlayRect.bottom * scaleY, -1);
    }
    glEnd();
    glFlush();


    // Restore matricies and attributes
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();

    fLock.Release();
}


CATResult CATOverlayOpenGL::HookFunctions()
{
    return this->InterceptDLL(fOpenGLDLL,&kOpenGLInterceptTable[0],this);
}

CATHOOKFUNC void CATOverlayOpenGL::OnSwapBuffers(    CATHOOK*             hookInst,
                                                       HDC                 hdc)
{
    CATHOOK_PROLOGUE(1);
    ((CATOverlayOpenGL*)(hookInst->InterceptObj))->DrawToScene(hdc);
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 1);
    CATHOOK_EPILOGUE_WINAPI(1);
}

#endif // CAT_CONFIG_WIN32
