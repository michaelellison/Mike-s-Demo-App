/// \file  CATVideoCapture.cpp
/// \brief Video capture class. 
/// \ingroup CAT
///
/// Copyright (c) 2011 by Michael Ellison.
/// See COPYING.txt for the License (MIT License).
///
///
// $Author: mike $
// $Date: 2011-06-02 02:47:03 -0500 (Thu, 02 Jun 2011) $
// $Revision: 7 $
// $NoKeywords: $


#include "CATVideoCapture.h"
#include "CATVMR9AllocPres.h"

#include <Dshow.h>
#include <D3d9.h>
#include <Vmr9.h>

// Internal platform specific struct to keep
// windows defs out of framework
struct CATVideoCapture::INTERNAL
{
	INTERNAL()
	{		
		fCapGraph			= 0;
		fControl				= 0;
		fGraph				= 0;
		fRenderer			= 0;
		fVMR9Config			= 0;
		fVMR9Notify			= 0;
		fAllocPres        = 0;
		fCaptureFilter		= 0;
	}
	
	CATVMR9AllocPres*					fAllocPres;	
	IMediaControl*						fControl;
	IGraphBuilder*  					fGraph;
	ICaptureGraphBuilder2*			fCapGraph;
	IBaseFilter*						fRenderer;
	IVMRFilterConfig9*				fVMR9Config;
	IVMRSurfaceAllocatorNotify9*	fVMR9Notify;	
	IBaseFilter*						fCaptureFilter;
};

CATVideoCapture::CATVideoCapture() 
{
	fCallback = 0;
	fContext = 0;
	fInternal = new INTERNAL;
}

CATVideoCapture::~CATVideoCapture() 
{
	Cleanup();
	delete fInternal;
}

CATResult CATVideoCapture::Init(CATWND wnd, const CATRect& rect,CATCaptureCB callback, void* context)
{
	Cleanup();
	fTargetRect = rect;
	fWnd = wnd;
	fCallback = callback;
	fContext  = context;
	CATResult res = CreateGraph();
	return res;
}

CATResult CATVideoCapture::Start()
{
	if (!fInternal->fGraph)
		return CAT_ERR_NOT_INITIALIZED;
	
	CATResult result = CAT_ERR_CAPTURE_CREATE;

/* Source file test
	{
		CATString sourceFile = 
		if (S_OK == fInternal->fGraph->RenderFile((const wchar_t*)sourceFile,0))
			result = CAT_SUCCESS;			
	}
*/
	if (CATFAILED(result = InitCapture()))
		return result;

	if (FAILED(fInternal->fControl->Run()))
		result = CAT_ERR_CAPTURE_CREATE;

	return result;
}

CATResult CATVideoCapture::Stop()
{
	if (fInternal && fInternal->fControl)
	{
		fInternal->fControl->Stop();
	}
	return CAT_SUCCESS;
}


void CATVideoCapture::Cleanup()
{
		if (fInternal)
		{
			if (fInternal->fControl)
			{
				fInternal->fControl->Stop();
				fInternal->fControl->Release();
				fInternal->fControl = 0;
			}

			if (fInternal->fVMR9Notify)
			{
				fInternal->fVMR9Notify->Release();
				fInternal->fVMR9Notify = 0;
			}

			if (fInternal->fAllocPres)
			{
				fInternal->fAllocPres->Uninit();
				fInternal->fAllocPres->Release();
				fInternal->fAllocPres = 0;
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
			if (fInternal->fCapGraph)
			{
				fInternal->fCapGraph->Release();
				fInternal->fCapGraph = 0;
			}	
			if (fInternal->fGraph)
			{
				fInternal->fGraph->Release();
				fInternal->fGraph = 0;
			}	

		}
}


CATResult CATVideoCapture::CreateGraph()
{
	// Init our allocator/presenter
	fInternal->fAllocPres = new CATVMR9AllocPres();

	CATResult res = fInternal->fAllocPres->Init( fWnd, fTargetRect.Width(),fTargetRect.Width(),fCallback, fContext);
	if (CATFAILED(res))
		return res;
	
	// Create graphbuilder and filtergraph
	// goto or throw works too.  just bail on any error and cleanup anything we created.
	for (;;)
	{
		HRESULT hr;
		
		if (FAILED(hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&fInternal->fGraph)))
			break;

		if (FAILED(hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, 0, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&fInternal->fCapGraph)))
			break;

		fInternal->fCapGraph->SetFiltergraph(fInternal->fGraph);

		// Get control interface (start/stop/etc)
		if (FAILED(hr = fInternal->fGraph->QueryInterface(IID_IMediaControl,
															(void**)&fInternal->fControl)))
			break;

		// Create a renderless VMR9
		if (FAILED(hr =CoCreateInstance(CLSID_VideoMixingRenderer9, 0, 
 												  CLSCTX_INPROC_SERVER, 
												  IID_IBaseFilter, 
												  (void**)&fInternal->fRenderer)))
			break;

		if (FAILED(hr = fInternal->fRenderer->QueryInterface(IID_IVMRFilterConfig9, (void**)&fInternal->fVMR9Config)))
			break;

		fInternal->fVMR9Config->SetNumberOfStreams(1);

		if (FAILED(hr = fInternal->fGraph->AddFilter(fInternal->fRenderer,L"VMR9")))
			break;

		if (FAILED(hr = fInternal->fVMR9Config->SetRenderingMode(VMR9Mode_Renderless)))
			break;


		// Hook in our custom allocator/presenter
		if (FAILED(hr = fInternal->fRenderer->QueryInterface(IID_IVMRSurfaceAllocatorNotify9, (void**)&fInternal->fVMR9Notify)))
			break;

		hr = fInternal->fVMR9Notify->SetD3DDevice(fInternal->fAllocPres->GetDevice(), fInternal->fAllocPres->GetMonitor());	
		hr = fInternal->fVMR9Notify->AdviseSurfaceAllocator( 0, fInternal->fAllocPres);
		hr = fInternal->fAllocPres->AdviseNotify(fInternal->fVMR9Notify);
		
		res = CAT_SUCCESS;
		break;
	}
	
	if (CATFAILED(res))
		Cleanup();

	return res;
}



CATResult CATVideoCapture::InitCapture()
{
	HRESULT hr;
	ICreateDevEnum* devEnum	  = 0;
	IEnumMoniker*	 classEnum = 0;
	IMoniker*		 moniker   = 0;		
	IBaseFilter*	 pSrc = NULL;

	// Create an enumerator for video input
	if (FAILED(hr = CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **) &devEnum)))
		return CAT_ERR_CAPTURE_CREATE;

	if (FAILED(hr = devEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &classEnum, 0))
		|| (classEnum == 0))

	{
		// Failed or no devices.
		devEnum->Release();
		return CAT_ERR_CAPTURE_CREATE;
	}


	// Got a device (at least one)
	while (S_OK ==  (hr = classEnum->Next(1,&moniker,0)))
	{
		IPropertyBag* propBag;
		VARIANT       devName;

		// Get device properties
		if (FAILED(moniker->BindToStorage( 0, 0, IID_IPropertyBag, (void**)(&propBag))))
		{
			moniker->Release();
			continue;
		} 

		// Get the device name from the property bag
		VariantInit(&devName);           
		if (FAILED(hr = propBag->Read(L"Description", &devName, 0)))
		{
			hr = propBag->Read(L"FriendlyName", &devName, 0);
		}

		if (SUCCEEDED(hr))
		{
			// Convert string to ASCII
			int strLength = WideCharToMultiByte(   CP_ACP, 0, devName.bstrVal, -1, 0, 0, 0, 0);
			WideCharToMultiByte( CP_ACP, 0, devName.bstrVal, -1, fDevName.GetAsciiBuffer(128),strLength,0,0);
			fDevName.ReleaseBuffer();
			CATTRACE(fDevName);
		

			//OK, got a device.  Let's just take the first one, it's late.
         hr = moniker->BindToObject(0,0,IID_IBaseFilter, (void**)&fInternal->fCaptureFilter);
			if (SUCCEEDED(hr))
			{
				hr = this->fInternal->fGraph->AddFilter(fInternal->fCaptureFilter,L"Capture");
				if (SUCCEEDED(hr))
				{
					hr = fInternal->fCapGraph->RenderStream (&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,fInternal->fCaptureFilter, NULL, fInternal->fRenderer);				
				}
			}		
		}
		moniker->Release();
		propBag->Release();      
		VariantClear(&devName); 
	}
	classEnum->Release();
	devEnum->Release();

	return CAT_SUCCESS;
}

CATResult CATVideoCapture::LockImage(CATUInt32 wait)
{
	if (!fInternal->fAllocPres)
		return CAT_ERROR;

	return fInternal->fAllocPres->LockImage(wait);
}

void CATVideoCapture::ReleaseImage()
{
	if (fInternal->fAllocPres)
		fInternal->fAllocPres->ReleaseImage();
}
