/// \file  CATVideoCapture.h
/// \brief Video capture class (quick and dirty)
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
#ifndef _CATVideoCapture_H_
#define _CATVideoCapture_H_

#include "CATInternal.h"
#include "CATString.h"
#include "CATImage.h"

/// \class CATVideoCapture
/// \brief Quick and dirty video capture class using VMR9
/// \ingroup CAT
///
/// Uses a renderless VMR9 with CATVMR9AllocPres as the allocator/presenter.
/// Calls a callback each frame with a CATImage* (the same one, updated)
class CATVideoCapture
{
	public:
		CATVideoCapture();
		virtual ~CATVideoCapture();

		/// Pass it a window (base is fine), rect size, and a callback.
		/// Callback will be called each frame after starting it.
		virtual CATResult Init(CATWND wnd, const CATRect& rect, CATCaptureCB callback, void* context);

		/// Begin capturing.
		virtual CATResult Start();

		/// Stop capturing and clean up.
		virtual CATResult Stop();

	protected:				
		CATResult		InitCapture();
		void				Cleanup();			///< Clean up any referenced interfaces.
		CATResult		CreateGraph();		///< Set up rendering
		

		struct			INTERNAL;
		INTERNAL*		fInternal;			///< Internal defs - don't export dshow to clients
		CATWND			fWnd;
		CATCaptureCB	fCallback;
		void*				fContext;
		CATRect			fTargetRect;
		CATRect			fSourceRect;
		CATString		fDevName;
};

#endif _CATVideoCapture_H_
