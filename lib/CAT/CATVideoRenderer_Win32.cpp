/// \file  CATVideoRenderer.cpp
/// \brief Video renderer class. Works with CATVideoSource
/// \ingroup CAT
///
/// Copyright (c) 2011 by Michael Ellison.
/// See COPYING.txt for the License (MIT License).
///
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATVideoRenderer.h"
#include "CATVideoSource.h"

#include <Dshow.h>
#include <D3d9.h>
#include <Vmr9.h>

// Internal platform specific struct to keep
// windows defs out of framework
struct CATVideoRenderer::INTERNAL
{
	INTERNAL()
	{
		fControl				= 0;
		fGraph				= 0;
		fRenderer			= 0;
		fVMR9Windowless	= 0;
		fVMR9Config			= 0;
	}
	IMediaControl*				fControl;
	IGraphBuilder*  			fGraph;
	IBaseFilter*				fRenderer;
	IVMRWindowlessControl9* fVMR9Windowless;
	IVMRFilterConfig9*		fVMR9Config;
};

CATVideoRenderer::CATVideoRenderer() 
{
	fInternal = new INTERNAL;
	fTarget   = 0;
	fStarted  = false;
	fShown	 = false;
}

CATVideoRenderer::~CATVideoRenderer() 
{
	Cleanup();
	delete fInternal;
}

CATResult CATVideoRenderer::Init(CATWND target, CATRect displayRect)
{
	Cleanup();

	fTarget		= target;
	fTargetRect = displayRect;

	CATResult res = CreateGraph();
	return res;
}

CATResult CATVideoRenderer::Start(const CATVideoSource& source)
{
	if ((!fTarget) || (!fInternal->fGraph))
		return CAT_ERR_NOT_INITIALIZED;
	
	CATResult result = CAT_ERR_CAPTURE_CREATE;
	if (source.IsSourceFile())
	{
		CATString sourceFile = source.GetSourceFile();
		if (S_OK == fInternal->fGraph->RenderFile((const wchar_t*)sourceFile,0))
			result = CAT_SUCCESS;			
	}
	else
	{

	}

	if (CATSUCCEEDED(result))
	{
		if (FAILED(fInternal->fControl->Run()))
			result = CAT_ERR_CAPTURE_CREATE;
		else
			fStarted = true;
	}

	return result;
}

CATResult CATVideoRenderer::Stop()
{
	if (fInternal && fInternal->fControl)
	{
		fInternal->fControl->Stop();
	}
	fStarted = false;
	return CAT_SUCCESS;
}

bool CATVideoRenderer::IsRunning()
{
	return fStarted;
}


void CATVideoRenderer::Cleanup()
{
		if (fInternal)
		{
			if (fInternal->fControl)
			{
				fInternal->fControl->Stop();
				fInternal->fControl->Release();
				fInternal->fControl = 0;
			}

			if (fInternal->fVMR9Config)
			{
				fInternal->fVMR9Config->Release();
				fInternal->fVMR9Config = 0;
			}
			if (fInternal->fVMR9Windowless)
			{
				fInternal->fVMR9Windowless->Release();
				fInternal->fVMR9Windowless = 0;
			}

			if (fInternal->fVMR9Config)
			{
				fInternal->fVMR9Config->Release();
				fInternal->fVMR9Config = 0;
			}

			if (fInternal->fRenderer)
			{
				fInternal->fRenderer->Release();
				fInternal->fRenderer = 0;
			}
			if (fInternal->fGraph)
			{
				fInternal->fGraph->Release();
				fInternal->fGraph = 0;
			}		
		}
}


CATResult CATVideoRenderer::CreateGraph()
{
	// Create graphbuilder and filtergraph
	CATResult res = CAT_ERR_CAPTURE_CREATE;

	// goto or throw works too.  just bail on any error and cleanup anything we created.
	for (;;)
	{
		HRESULT hr;
		
		if (FAILED(hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&fInternal->fGraph)))
			break;

		// Get control interface (start/stop/etc)
		if (FAILED(hr = fInternal->fGraph->QueryInterface(IID_IMediaControl,
															(void**)&fInternal->fControl)))
			break;

		// Create a windowless renderer
		if (FAILED(hr =CoCreateInstance(CLSID_VideoMixingRenderer9, 0, 
 												  CLSCTX_INPROC_SERVER, 
												  IID_IBaseFilter, 
												  (void**)&fInternal->fRenderer)))
			break;

		if (FAILED(hr = fInternal->fGraph->AddFilter(fInternal->fRenderer,L"Video Mixing Renderer 9")))
			break;

		if (FAILED(hr = fInternal->fRenderer->QueryInterface(IID_IVMRFilterConfig9, (void**)&fInternal->fVMR9Config)))
			break;

		if (FAILED(hr = fInternal->fVMR9Config->SetRenderingMode(VMR9Mode_Windowless)))
			break;
		

		if (FAILED(hr = fInternal->fRenderer->QueryInterface(IID_IVMRWindowlessControl9,(void**)&fInternal->fVMR9Windowless)))
			break;

		// Don't die if this fails, but it's preferred.
		fInternal->fVMR9Windowless->SetAspectRatioMode( VMR9ARMode_LetterBox );

		if (FAILED(hr = fInternal->fVMR9Windowless->SetVideoClippingWindow(fTarget)))
			break;

	
		this->Move(fTargetRect);

		res = CAT_SUCCESS;
		break;
	}
	
	if (CATFAILED(res))
		Cleanup();

	return res;
}

void CATVideoRenderer::Refresh()
{
	HDC dc = ::GetDC(fTarget);
	if ((fInternal->fVMR9Windowless) && (fStarted))
	{
		fInternal->fVMR9Windowless->RepaintVideo(fTarget,dc);
	}
	else
	{
		RECT rect;
		::SetRect(&rect,fTargetRect.left,fTargetRect.top, fTargetRect.right, fTargetRect.bottom);
		::FillRect(dc,&rect,(HBRUSH)::GetStockObject(BLACK_BRUSH));
	}
	::ReleaseDC(fTarget,dc);
}

CATResult CATVideoRenderer::Move(CATRect displayRect)
{	
	if (fInternal->fVMR9Windowless)
	{
		RECT destRect;
		destRect.left	 = displayRect.left;  destRect.top	  = displayRect.top;
		destRect.bottom = displayRect.bottom; destRect.right = displayRect.right;
		fInternal->fVMR9Windowless->SetVideoPosition(0,&destRect);	
	}
	return CAT_SUCCESS;
}

void CATVideoRenderer::Show(bool show)
{
	if (show != fShown)
	{
		fShown = show;
	}
}