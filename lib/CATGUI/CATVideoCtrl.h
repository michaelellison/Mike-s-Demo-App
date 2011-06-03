// ------------------------------------------------------------------
/// \file CATVideoCtrl.h
/// \brief Simple control for video display
// Copyright (c) 2011 Mike Ellison. See Copying.txt.
// ------------------------------------------------------------------
#ifndef CATVideoCtrl_H
#define CATVideoCtrl_H
#include "CATPoint.h"
#include "CATVideoCapture.h"
#include "CATControlWnd.h"
#include "CATCritSec.h"

class CATVideoCtrl : public CATControl
{
public:
	CATVideoCtrl(	const CATString&               element, 
						const CATString&               rootDir);

	virtual	~CATVideoCtrl();
	
	virtual void		Draw(CATImage* image, const CATRect& dirtyRect);
	virtual void		OnParentCreate();
	virtual void		OnParentDestroy();

	virtual CATResult	Start();
	virtual CATResult Stop();

protected:
	static void OnFrame(CATImage* frame, void* context);
	CATCritSec			fCritSec;
	CATImage*			fCurImage;
	CATVideoCapture	fCapture;
};

#endif //CATVideoCtrl_H
