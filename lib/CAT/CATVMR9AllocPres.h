/// \file  CATVMR9AllocPres.h
/// \brief Quick and dirty Allocator/Presenter for VMR9
/// \ingroup CAT
///
/// Copyright (c) 2011 by Michael Ellison.
/// See COPYING.txt for the License (MIT License).
///
/// 
#ifndef _CATVMR9AllocPres_H_
#define _CATVMR9AllocPres_H_

#include "CATInternal.h"
#include "CATCritSec.h"
#include "CATMutex.h"
#include "CATImage.h"
#include <Dshow.h>
#include <D3d9.h>
#include <Vmr9.h>

/// \class CATVMR9AllocPres
/// \brief Quick and dirty Allocator/Presenter for VMR9
/// \ingroup CAT
///
/// This class does the DirectX surface allocation for CATVideoCapture,
/// and converts frames into CATImage*'s and calls the callback.
///
/// No frills, and not enough error handling or features, but it works.
/// \todo Add device configuration, USB notifications, surface/device lost handling,
///       and all sorts of stuff.
class CATVMR9AllocPres : public  IVMRSurfaceAllocator9, IVMRImagePresenter9
{
	public:
		CATVMR9AllocPres();
		virtual ~CATVMR9AllocPres();

		virtual CATResult Init(HWND wnd, CATInt32 w, CATInt32 h,CATCaptureCB cb, void* context);
		virtual CATResult Uninit();

		CATResult LockImage(CATUInt32 msWait = 0xFFFFFFFF);
		void ReleaseImage();
		IDirect3DDevice9* GetDevice();
		HMONITOR				GetMonitor();

		// IVMRSurfaceAllocator9
		virtual HRESULT STDMETHODCALLTYPE InitializeDevice( DWORD_PTR dwUserID, VMR9AllocationInfo *lpAllocInfo, DWORD *lpNumBuffers);            
		virtual HRESULT STDMETHODCALLTYPE TerminateDevice ( DWORD_PTR dwID);    
		virtual HRESULT STDMETHODCALLTYPE GetSurface( DWORD_PTR dwUserID, DWORD SurfaceIndex, DWORD SurfaceFlags, IDirect3DSurface9 **lplpSurface);    
		virtual HRESULT STDMETHODCALLTYPE AdviseNotify( IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify);
    
		// IVMRImagePresenter9
		virtual HRESULT STDMETHODCALLTYPE StartPresenting( DWORD_PTR dwUserID);    
		virtual HRESULT STDMETHODCALLTYPE StopPresenting( DWORD_PTR dwUserID);    
		virtual HRESULT STDMETHODCALLTYPE PresentImage( DWORD_PTR dwUserID, VMR9PresentationInfo *lpPresInfo);    
    
		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void** ppvObject);
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

	protected:
		void CleanSurfaces();
	
	protected:
		CATImage*							fImage;
		CATMutex								fImageLock;

		IDirect3D9*							fD3d;
		IDirect3DDevice9*					fD3dDev;
		HMONITOR								fMonitor;
		IVMRSurfaceAllocatorNotify9*	fNotify;

		IDirect3DSurface9**				fSurfaces;
		DWORD									fSurfaceCount;

		CATCritSec							fCritSec;
		volatile LONG						fRefCount;		
		void*									fContext;
		CATCaptureCB						fCallback;
	
};

#endif // _CATVMR9AllocPres_H_