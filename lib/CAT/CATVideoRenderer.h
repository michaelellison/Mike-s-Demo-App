/// \file  CATVideoCapture.h
/// \brief Video renderer class. Works with CATVideoSource
/// \ingroup CAT
///
/// Copyright (c) 2011 by Michael Ellison.
/// See COPYING.txt for the License (MIT License).
///
///
// $Author: mike $
// $Date: 2011-06-01 07:28:30 -0500 (Wed, 01 Jun 2011) $
// $Revision: 4 $
// $NoKeywords: $
#ifndef _CATVideoRenderer_H_
#define _CATVideoRenderer_H_

#include "CATInternal.h"
#include "CATString.h"
class CATVideoSource;

class CATVideoRenderer
{
	public:
		CATVideoRenderer();
		virtual ~CATVideoRenderer();

		virtual CATResult Init(CATWND target, CATRect displayRect);
		virtual CATResult Move(CATRect displayRect);
		virtual CATResult Start(const CATVideoSource& source);
		virtual CATResult Stop();

		virtual void		Refresh();
		virtual bool		IsRunning();
		virtual void		Show(bool show);

protected:		
		bool				fShown;
		void				Cleanup();												///< Clean up any referenced interfaces.
		CATResult		CreateGraph();	///< Set up rendering
		
		struct			INTERNAL;
		INTERNAL*		fInternal;				///< Internal defs - don't export dshow to clients

		bool				fStarted;
		CATWND			fTarget;
		CATRect			fTargetRect;
};

#endif _CATVideoRenderer_H_
