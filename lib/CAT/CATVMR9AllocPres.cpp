/// \file  CATVMR9AllocPres.cpp
/// \brief Allocator/Presenter for VMR9
/// \ingroup CAT
///
/// Copyright (c) 2011 by Michael Ellison.
/// See COPYING.txt for the License (MIT License).
///
///
#include "CATVMR9AllocPres.h"
#include <d3dx9tex.h>

CATVMR9AllocPres::CATVMR9AllocPres()
{
	fD3d			  = 0;
	fD3dDev		  = 0;
	fNotify		  = 0;
	fSurfaces	  = 0;
	fSurfaceCount = 0;
	fMonitor		  = 0;
	fRefCount	  = 1;
	fImage		  = 0;
	fCallback	  = 0;
	fContext		  = 0;
}

CATVMR9AllocPres::~CATVMR9AllocPres()
{
	CATASSERT(fRefCount == 0, "WARNING: CATVMR9AllocPres Refcount != 0!");
}

CATResult CATVMR9AllocPres::Init(HWND hwnd, CATInt32 w, CATInt32 h,CATCaptureCB cb, void* context)
{
	HRESULT hr = E_FAIL;
	D3DPRESENT_PARAMETERS presParam;
	memset(&presParam,0,sizeof(presParam));
	
	fCallback = cb;
	fContext = context;
	
	fD3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (fD3d)
	{
		presParam.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		presParam.BackBufferWidth     = w;
		presParam.BackBufferHeight    = h;
		presParam.BackBufferCount		= 1;
		presParam.BackBufferFormat		= D3DFMT_X8R8G8B8;
		presParam.MultiSampleType		= D3DMULTISAMPLE_NONE;
		presParam.SwapEffect				= D3DSWAPEFFECT_DISCARD;
		presParam.Windowed				= true;
		presParam.hDeviceWindow			= hwnd;
		presParam.EnableAutoDepthStencil = 0;
		presParam.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		hr = fD3d->CreateDevice(D3DADAPTER_DEFAULT, 
									   D3DDEVTYPE_HAL,
										hwnd, 
										D3DCREATE_MIXED_VERTEXPROCESSING,
										&presParam,
										&fD3dDev);

		fMonitor = this->fD3d->GetAdapterMonitor(D3DADAPTER_DEFAULT);
	}
	if (SUCCEEDED(hr))
	{
		CATImage::CreateImage(fImage, presParam.BackBufferWidth, presParam.BackBufferHeight,true,false);
		return CAT_SUCCESS;
	}
	
	if (fD3d)
	{
		fD3d->Release();
		fD3d = 0;
	}

	return hr;
}

CATResult CATVMR9AllocPres::Uninit()
{
	CleanSurfaces();
	if (fD3d)
	{
		if (fD3dDev)
		{
			fD3dDev->Release();
			fD3dDev = 0;
		}
		fD3d->Release();
		fD3d = 0;
	}	
	
	if (fImage)
	{
		CATImage::ReleaseImage(fImage);
		fImage = 0;
	}
	
	return CAT_SUCCESS;
}

void CATVMR9AllocPres::CleanSurfaces()
{
	if (fSurfaces)
	{
		for (CATUInt32 i = 0; i < fSurfaceCount; i++)
		{
			if (fSurfaces[i])
				fSurfaces[i]->Release();
			fSurfaces[i] = 0;
		}
		delete [] fSurfaces;
	}
	fSurfaces = 0;
	fSurfaceCount = 0;
}

//---------------------------------------------------------------------------
// IVMRSurfaceAllocator9
HRESULT STDMETHODCALLTYPE CATVMR9AllocPres::InitializeDevice( DWORD_PTR dwUserID, VMR9AllocationInfo *lpAllocInfo, DWORD *lpNumBuffers)
{
	HRESULT hr;
	if ((!lpNumBuffers) || (!lpAllocInfo))
		return E_POINTER;

	 fMutex.Wait();
	// Nuke existing surfaces
	CleanSurfaces();

	// create surfaces.
	fSurfaces = new IDirect3DSurface9*[*lpNumBuffers];
	if (FAILED(hr = fNotify->AllocateSurfaceHelper(lpAllocInfo,lpNumBuffers,fSurfaces)))
	{
		CleanSurfaces();
		*lpNumBuffers = 0;
		fMutex.Release();
		return hr;
	}
	
	fSurfaceCount = *lpNumBuffers;

	fMutex.Release();
	return S_OK;
}


HRESULT STDMETHODCALLTYPE CATVMR9AllocPres::TerminateDevice ( DWORD_PTR dwID)
{
	fMutex.Wait();

	CleanSurfaces();


	fMutex.Release();
	return S_OK;
}    


HRESULT STDMETHODCALLTYPE CATVMR9AllocPres::GetSurface( DWORD_PTR dwUserID, DWORD SurfaceIndex, DWORD SurfaceFlags, IDirect3DSurface9 **lplpSurface)
{
	if (!lplpSurface)
		return E_POINTER;
	
	*lplpSurface = 0;

	fMutex.Wait();	
	if ((fSurfaces) && (SurfaceIndex < fSurfaceCount))
	{
		fSurfaces[SurfaceIndex]->AddRef();
		*lplpSurface = fSurfaces[SurfaceIndex];
	}	
	fMutex.Release();

	if (!(*lplpSurface))
		return E_FAIL;

	return S_OK;
}    

HRESULT STDMETHODCALLTYPE CATVMR9AllocPres::AdviseNotify( IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify)
{
	if (!lpIVMRSurfAllocNotify)
		return E_POINTER;

	fMutex.Wait();

	fNotify = lpIVMRSurfAllocNotify;

	fMutex.Release();

	return S_OK;
}

//---------------------------------------------------------------------------
// IVMRImagePresenter9
HRESULT STDMETHODCALLTYPE CATVMR9AllocPres::StartPresenting( DWORD_PTR dwUserID)
{
	// Notify start
	return S_OK;
}    
HRESULT STDMETHODCALLTYPE CATVMR9AllocPres::StopPresenting( DWORD_PTR dwUserID)
{
	// Notify stop
	return S_OK;
}    

HRESULT STDMETHODCALLTYPE CATVMR9AllocPres::PresentImage( DWORD_PTR dwUserID, VMR9PresentationInfo *lpPresInfo)
{	
	fMutex.Wait();

	// Copy data and/or display - lpPresInfo->lpSurf
	HRESULT hr;
	int w = fImage->Width();
	int h = fImage->Height();

	RECT copyrect;
	copyrect.left = copyrect.top = 0;
	copyrect.right = w;
	copyrect.bottom = h;

	IDirect3DSurface9* bb = 0;
	fD3dDev->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&bb);
	if (SUCCEEDED(hr = fD3dDev->StretchRect(lpPresInfo->lpSurf,0,bb,&copyrect, D3DTEXF_NONE)))
	{		
		D3DLOCKED_RECT rect;
		if (SUCCEEDED(hr = bb->LockRect(&rect,0,D3DLOCK_READONLY)))
		{	
			CATUInt32 color;
			CATUInt32* src, *dst;
			src = (CATUInt32*)rect.pBits;
			dst = (CATUInt32*)fImage->GetRawDataPtr();

			for (int y = h-1; y != 0; --y)
			{
				src = (CATUInt32*)((unsigned char*)rect.pBits+rect.Pitch*y);
				for (int x = 0; x < w; x++)
				{
					color = *src++;
					*dst++ = ((color & 0xff) << 16) |
								((color & 0xff0000) >> 16) |
								 (color & 0x00ff00) | 
								 0xff000000;
				}
			}

			if (fCallback)
				fCallback(fImage,fContext);

		}	
		bb->UnlockRect();
		bb->Release();			
	}
	fMutex.Release();

	return S_OK;
}    


//---------------------------------------------------------------------------
// IUnknown
HRESULT STDMETHODCALLTYPE CATVMR9AllocPres::QueryInterface( REFIID riid, void** ppvObject)
{
    if (!ppvObject)
        return E_POINTER;

	 if (__uuidof(IUnknown) == riid)
		 *ppvObject = (IUnknown*)(IVMRSurfaceAllocator9*)this;
	 else if (__uuidof(IVMRSurfaceAllocator9) == riid)
		 *ppvObject = (IVMRSurfaceAllocator9*)this;
	 else if (__uuidof(IVMRImagePresenter9) == riid)
		 *ppvObject = (IVMRImagePresenter9*)this;
	 else
	 {
		 *ppvObject = 0;
		 return E_NOINTERFACE;
	 }
    
	 AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE   CATVMR9AllocPres::AddRef()
{
	return InterlockedIncrement(&fRefCount);
}
ULONG STDMETHODCALLTYPE	  CATVMR9AllocPres::Release()
{
    LONG refCount = InterlockedDecrement(&fRefCount);
	 
	 CATASSERT(refCount >= 0,"Released too many times!");
    
	 if (!refCount)
        delete this;
	 
	 return refCount;
}
	
IDirect3DDevice9* CATVMR9AllocPres::GetDevice()
{
	return this->fD3dDev;
}

HMONITOR CATVMR9AllocPres::GetMonitor()
{
	return this->fMonitor;
}
