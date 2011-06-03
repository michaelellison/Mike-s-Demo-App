/// \file MikesDemoWindow_Main.h
/// \brief Main Window for MikesDemo
/// \ingroup MikesDemo
///
// Copyright (c) 2011 by Michael Ellison.
// See COPYING.txt for license (MIT License).
//
#ifndef MikesDemoWindow_Main_H_
#define MikesDemoWindow_Main_H_

#include "CATWindow.h"
#include "CATPoint.h"

class CAT3DVideo;
class CATTab;
class CATTreeCtrl;
class CATPictureMulti;

/// \class MikesDemoWindow_Main
/// \brief Main window class for Mikes Demo
/// \ingroup MikesDemo
class MikesDemoWindow_Main : public CATWindow
{
    public:
        MikesDemoWindow_Main(				 const CATString&    element, 
													 const CATString&		rootDir);

        virtual ~MikesDemoWindow_Main();

        virtual void OnCommand(         CATCommand&         command, 
                                        CATControl*         ctrl);

        virtual CATResult  OnEvent  (   const CATEvent&     eventStruct, 
                                        CATInt32&           retVal);

        virtual void OnCreate();

        virtual void OnDestroy();

    protected:
		void OnShow();
		
		// Controls (don't delete, and may be null)
		CAT3DVideo*      fView3d;
		CATTab* 	        fTabCtrl;
		CATTreeCtrl*     fProTree;
		CATPictureMulti* fProLogo;
		CATPictureMulti* fProPic;
		CATLabel*		  fProText;

		// Tree helper
		void OnProTreeChange(CATUInt32 index);
		
		// 3D helper functions
		CATResult LoadRawScan(const CATString& fname);
		bool GetPointsFromRawScan(CATInt32 numScans, CATInt32 height, CATScanPoint* pointArray);
		void Clear3dPoints();

		// 3D data
		CATInt32			fNum3dPoints;		
		CATC3DPoint*	f3dPoints;
		CATScanPoint*	fPointScanArray;
		CATInt32			fPointScanHeight; // height of point scan (resolution height)
		CATInt32			fPointScanScans;	// number of point scans (width)
};
#endif // MikesDemoWindow_Main_H_

