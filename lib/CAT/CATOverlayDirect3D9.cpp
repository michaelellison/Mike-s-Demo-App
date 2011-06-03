/// \file  CATOverlayDirect3D9.cpp
/// \brief Direct3D function interception
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATOverlayDirect3D9.h"
#include "CATString.h"

#ifdef CAT_CONFIG_WIN32
#include <d3d9.h>

/// This table contains all of the functions we wish to intercept
/// from IDirect3DDevice9 and the targets for those functions.
CATINTERCEPT_COM_TABLE_ENTRY CATOverlayDirect3D9::kDirect3DDeviceInterceptTable9[] =
{
    // VTableIndex  HookFunction                                 StubLength
    {  16,          CATOverlayDirect3D9::OnReset9,            5},  
    {  17,          CATOverlayDirect3D9::OnPresent9,          5},  
    {  42,          CATOverlayDirect3D9::OnEndScene,          5},  
    // End marker
    {(CATUInt32)-1,0,-1}
};
CATOverlayDirect3D9::CATOverlayDirect3D9()
{
    fDeviceLost  = true;
    fTexture     = 0;
    fBackTexture = 0;
    fTexScaleX   = 1.0f;
    fTexScaleY   = 1.0f;
    fD3d9DLL     = ::LoadLibrary(L"d3d9.dll");
}

CATOverlayDirect3D9::~CATOverlayDirect3D9()
{
    RestoreAll();
    if (fD3d9DLL)
        FreeLibrary(fD3d9DLL);
    fD3d9DLL = 0;
}

void CATOverlayDirect3D9::DrawToScene(IDirect3DDevice9* device)
{
    fLock.Wait();
    
    if (fDeviceLost)
    {
        fLock.Release();
        return;
    }

    // If the overlay is dirty, regen the texture
    if (fOverlayDirty)
    {
        //::OutputDebugString(L"DX9: Refreshing overlay.");
        if (fTexture)
            fTexture->Release();
        if (fBackTexture)
            fBackTexture->Release();
        
        fTexture     = 0;
        fBackTexture = 0;
        
        if (fOverlay == 0)
        {
            fLock.Release();
            return;
        }

        CATInt32 tw = 2;
        while (tw < fOverlay->Width())
            tw*= 2;
        CATInt32 th = 2;
        while (th < fOverlay->Height())
            th*=2;

        RECT textureRect;
        textureRect.left   = textureRect.top = 0;
        textureRect.right  = tw;
        textureRect.bottom = th;

        HRESULT hr =  device->CreateTexture(tw,th,1,0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&fBackTexture,0);
        hr |= device->CreateTexture(tw,th,1,0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&fTexture,0);
        
        if (FAILED(hr))
        {
            ::OutputDebugString(L"Failed creating texture");
            fLock.Release();
            return;
        }
        
        D3DSURFACE_DESC surfaceDesc,sd2;        
        fTexture->GetLevelDesc(0,&surfaceDesc);
        fBackTexture->GetLevelDesc(0,&sd2);
        if ((surfaceDesc.Format != D3DFMT_A8R8G8B8) || (sd2.Format != D3DFMT_A8R8G8B8))
        {
            CATString dbg;
            dbg.Format(L"Surface format mismatch. %d",surfaceDesc.Format);
            ::OutputDebugString(dbg);
            fTexture->Release();
            fTexture = 0;
            fBackTexture->Release();
            fBackTexture = 0;
            fLock.Release();
            return;
        }            
        
        D3DLOCKED_RECT lockedRect;                       
        if (!FAILED(fTexture->LockRect(0,&lockedRect,&textureRect,0)))
        {
            // Create a buffer to copy image into that's a power of 2 for texture happiness.
            CATUInt8* buffer = (CATUInt8*)lockedRect.pBits;
            CATUInt8* srcBuf = fOverlay->GetRawDataPtr();
            CATUInt32 imgWidth = fOverlay->Width();
            
            
            // Yuck. our current format is swapped from what DX likes to give me.
            int uh = fOverlay->Height();
            int uw = fOverlay->Width();
            
            memset(buffer,0,tw*th*4);

            int dstLinePad = (tw - uw)*4;
            for (int y = 0; y < uh; y++)
            {
                for (int x = 0; x < uw; x++)
                {
                    *buffer = srcBuf[2];
                    buffer[2] = *srcBuf;
                    buffer[1] = srcBuf[1];
                    buffer[3] = srcBuf[3];
                    buffer += 4;
                    srcBuf += 4;
                }
                buffer += dstLinePad;
            }
            fTexture->UnlockRect(0);

            // Preserve image/texture scale for drawing
            fTexScaleX = (CATFloat32)fOverlay->Width() /  (CATFloat32)tw;
            fTexScaleY = (CATFloat32)fOverlay->Height() / (CATFloat32)th;
    
            RECT textureRect;
            textureRect.left = textureRect.top = 0;
            textureRect.right = tw;
            textureRect.bottom = th;

            device->UpdateTexture(fTexture, fBackTexture);

            fOverlayDirty = false;
        }
        else
        {
            ::OutputDebugString(L"Failed to lock texture.");
        }
    }

    // Whew... texture regen'd if needed. Now draw it.
    if (fBackTexture)
    {       
        IDirect3DSurface9* surface = 0;
        //device->GetRenderTarget(0,&surface);
        device->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&surface);
        if (surface)
        {
            IDirect3DSurface9* textSurface = 0;
            fBackTexture->GetSurfaceLevel(0,&textSurface);
            if (textSurface)
            {
                RECT srcRect;
                srcRect.left = srcRect.top = 0;
                srcRect.bottom = fOverlay->Height();
                
                srcRect.right  = fOverlay->Width();

                D3DSURFACE_DESC desc;
                surface->GetDesc(&desc);

                POINT topLeft;
                topLeft.x = topLeft.y = 0;

                RECT destRect;
                fRect.right = desc.Width;
                fRect.bottom = desc.Height;

                CATFloat32 scaleX = (CATFloat32)desc.Width / (CATFloat32)fRefScreenWidth;
                CATFloat32 scaleY = (CATFloat32)desc.Height / (CATFloat32)fRefScreenHeight;
                if (fKeepAspect)
                {
                    scaleY = scaleX = CATMin(scaleX,scaleY);
                }

                destRect.left   = (LONG)(fOverlayRect.left*scaleX);
                destRect.top    = (LONG)(fOverlayRect.top*scaleY);
                destRect.right  = (LONG)(fOverlayRect.right*scaleX);
                destRect.bottom = (LONG)(fOverlayRect.bottom*scaleY);
                
                HRESULT hr = device->StretchRect(textSurface,&srcRect,surface,&destRect,D3DTEXF_NONE);
                if (FAILED(hr))
                {
                    CATString dbg;
                    
                    surface->GetDesc(&desc);
                    dbg.Format(L"Stretch failed: %08x Surface format: %d src %d,%d,%d,%d dst %d,%d,%d,%d",hr,desc.Format,
                        srcRect.left, srcRect.top, srcRect.right, srcRect.bottom, destRect.left, destRect.top,destRect.right,destRect.bottom);
                    ::OutputDebugString(dbg);
                    dbg.Format(L"Target: %dx%d pool %d type %d Multi: %d MQ: %d",
                        desc.Width, desc.Height, desc.Pool,desc.Type,desc.MultiSampleType, desc.MultiSampleQuality);
                    ::OutputDebugString(dbg);
                }
                textSurface->Release();                
            }
            else
            {
                ::OutputDebugString(L"didn't get texture surface..");
            }

            surface->Release();
        }
    }

    fLock.Release();
}

void CATOverlayDirect3D9::Reset(      IDirect3DDevice9*             device,
                                      _D3DPRESENT_PARAMETERS_*    presParams)
{
    fLock.Wait();
    ::OutputDebugString(L"DX9: Reset");
    
    // Flag the texture for recreation.
    fDeviceLost   = true;
    
    if (fTexture)        
        fTexture->Release();
    fTexture = 0;
    if (fBackTexture)
        fBackTexture->Release();
    fBackTexture = 0;

    fLock.Release();
}

void CATOverlayDirect3D9::DeviceLost(     CATHOOK*             hookInst,
                                          IDirect3DDevice9*    device)
{

    CATOverlayDirect3D9* overlay = (CATOverlayDirect3D9*)hookInst->InterceptObj;
    overlay->fLock.Wait();
    ::OutputDebugString(L"DX9: Device Lost");
    
    overlay->fDeviceLost = true;    
    
    if (overlay->fTexture)
        overlay->fTexture->Release();
    overlay->fTexture = 0;
    if (overlay->fBackTexture)
        overlay->fBackTexture->Release();
    overlay->fBackTexture = 0;

    overlay->fLock.Release();
}

void CATOverlayDirect3D9::DeviceOk(     CATHOOK*             hookInst,
                                        IDirect3DDevice9*    device)
{
    

    CATOverlayDirect3D9* overlay = (CATOverlayDirect3D9*)hookInst->InterceptObj;
    overlay->fLock.Wait();

    if (overlay->fDeviceLost)
    {
        ::OutputDebugString(L"DX9: Device OK");
        overlay->fDeviceLost   = false;    
        overlay->fOverlayDirty = true;
    }

    overlay->fLock.Release();
}

CATResult CATOverlayDirect3D9::HookFunctions()
{
    HWND procWnd = ::GetDesktopWindow();

    HRESULT           hr            = 0;
    
    CATResult result = CAT_SUCCESS;

    D3DDISPLAYMODE    displayMode;    
    IDirect3D9*       d3d9          = 0;
    IDirect3DDevice9* device9       = 0;

    // First, try loading values from registry if available.
    result = this->LoadAndHook(L"Direct3DDevice9",kDirect3DDeviceInterceptTable9,this);
    if (CATSUCCEEDED(result))
    {
        return CAT_SUCCESS;
    }
    {
        typedef IDirect3D9* (STDMETHODCALLTYPE *DIRECT3DCREATE9FUNC)(UINT);
        DIRECT3DCREATE9FUNC d3dCreate9 = (DIRECT3DCREATE9FUNC)::GetProcAddress(fD3d9DLL,"Direct3DCreate9");
        if (d3dCreate9 == 0)
        {
            ::OutputDebugStringW(L"Failed to retrieve Direct3dCreate9 func.");
            return CATRESULT(CAT_ERR_INTERCEPT_NO_DSOUND);
        }

        d3d9 = d3dCreate9(D3D_SDK_VERSION);    // 32 is d3d9 version
        if (d3d9 == 0)
        {
            ::OutputDebugStringW(L"Failed to create Direct3d9 object.");
            return CATRESULT(CAT_ERR_INTERCEPT_NO_DSOUND);
        }
               
	    hr = d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode );
        if (FAILED(hr))
        {
            ::OutputDebugStringW(L"Failed retrieving display mode.");
            return CATRESULT(CAT_ERR_INTERCEPT_NO_DSOUND);
        }

        D3DPRESENT_PARAMETERS presentParams; 
        memset(&presentParams, 0, sizeof(presentParams));
        presentParams.Windowed          = true;
        presentParams.SwapEffect        = D3DSWAPEFFECT_DISCARD;
        presentParams.BackBufferFormat  = displayMode.Format;

        
	    hr = d3d9->CreateDevice( D3DADAPTER_DEFAULT, 
                                D3DDEVTYPE_HAL, 
                                procWnd,
		                        D3DCREATE_HARDWARE_VERTEXPROCESSING,//D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		                        &presentParams, 
                                &device9);

        result = this->InterceptCOMObject(device9,kDirect3DDeviceInterceptTable9,this);
        
        // On success, try to save out data so we don't have to do it next time.
        if (CATSUCCEEDED(result))
        {
            SaveInterceptData(L"Direct3DDevice9",device9,kDirect3DDeviceInterceptTable9,this);
        }

        if (device9)
            device9->Release();
        
        if (d3d9)
            d3d9->Release();    
    }
    return result;
}


CATHOOKFUNC void CATOverlayDirect3D9::OnReset9(     CATHOOK*                hookInst,
                                                    IDirect3DDevice9*       device,
                                                    D3DPRESENT_PARAMETERS*  presParams)
{
    CATHOOK_PROLOGUE(2);    
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 2);
    ((CATOverlayDirect3D9*)(hookInst->InterceptObj))->Reset(device,presParams);
    CATHOOK_EPILOGUE_WINAPI(2);
}

CATHOOKFUNC void CATOverlayDirect3D9::OnPresent9(    CATHOOK*             hookInst,
                                                      IDirect3DDevice9*    device,
                                                      const RECT*          sourceRect,
                                                      const RECT*          destRect,
                                                      HWND                 destWindow,
                                                      const RGNDATA*       pDirtyRgn)
{
    CATHOOK_PROLOGUE(5);
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 5);   
    
    _asm 
    {
        mov eax, [ebp-4]
        or  eax, 0x80000000         // Check for lost device
        jnz presentOk
        push device
        push hookInst
        call DeviceLost             // Got one, notify our overlay!
        add  esp,8
        jmp  presentDone
presentOk:
        push device
        push hookInst
        call DeviceOk
        add  esp,8
presentDone:
    }

    CATHOOK_EPILOGUE_WINAPI(5);
}

CATHOOKFUNC void CATOverlayDirect3D9::OnEndScene(    CATHOOK*             hookInst,
                                                     IDirect3DDevice9*    device)
{
    CATHOOK_PROLOGUE(1);    
    ((CATOverlayDirect3D9*)(hookInst->InterceptObj))->DrawToScene(device);
    CATHOOK_CALLORIGINAL_WINAPI(hookInst, 1);       
    CATHOOK_EPILOGUE_WINAPI(1);
}

#endif // CAT_CONFIG_WIN32
