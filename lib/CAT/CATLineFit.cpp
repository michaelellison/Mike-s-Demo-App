/// \file CATLineFit.cpp
/// \brief Regression to fit a line to a set of points.
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATLineFit.h"
#include "CATUtil.h"

CATLineFit::CATLineFit()
{
	fDirty			= true;
	fSlope			= 0.0;
	fIntercept		= 0.0;
	fSumX			= 0.0;
	fSumY			= 0.0;
	fSumXY			= 0.0;
	fSumXSquared	= 0.0;
}

CATLineFit::~CATLineFit()
{
	Clear();
}

bool CATLineFit::AddPoint( CATFloat64 x, CATFloat64 y)
{
	// Push both points onto Lists and set dirty flag
	fDirty = true;
	fPointList.push_back(CATPoint(x,y));
	// Keep vars up to date to minimize calculations later
	fSumX += x;
	fSumY += y;
	fSumXY += x*y;
	fSumXSquared += x*x;

	return true;
}

bool CATLineFit::Clear()
{
	
	// Clear all the Lists...
    fPointList.clear();

	// Set dirty flag
	fDirty = true;

	fSlope			= 0.0;
	fIntercept		= 0.0;
	fSumX				= 0.0;
	fSumY				= 0.0;
	fSumXY			= 0.0;
	fSumXSquared	= 0.0;

	return true;
}


CATUInt32 CATLineFit::GetNumPoints()
{	
	return (CATUInt32)fPointList.size();
}

bool CATLineFit::GetDataPoint( CATUInt32 n, CATFloat64& x, CATFloat64& y)
{	
	if (n >= fPointList.size())
	{
		return false;
	}

	CATPoint point = fPointList[n];

	x = point.x;
	y = point.y;

	return true;
}

bool CATLineFit::GetCurrentErr(CATFloat64 &err)
{
	err = 0.0;
	// Recalc if the coefficient table is dirty
	if (fDirty)
	{
		if (!CalcFit())
		{
			return false;
		}
	}
	
	// Return the last calc'd error
	err = fLastErr;

	return true;
}



bool CATLineFit::CalcYVal( CATFloat64 x, CATFloat64& y)
{
	y = 0.0;
	// Recalc if the coefficient table is dirty
	if (fDirty)
	{
		if (!CalcFit())
		{
			return false;
		}
	}

	// Calculate Y val from X and calculated coefficients....
	y = this->fSlope * x + this->fIntercept;

	return true;
}


bool CATLineFit::CalcFit()
{
	// Return false if we don't have any points to calculate....
	CATUInt32 numPoints = GetNumPoints();
	
	if (numPoints < 2)
	{
		return false;
	}

	fSlope		= 0.0;
	fIntercept	= 0.0;

	fSlope		= ((numPoints * fSumXY) - (fSumX * fSumY))  / ((numPoints * fSumXSquared) - (fSumX * fSumX));
	fIntercept  = (fSumY - (fSlope * fSumX)) / numPoints;

	// Reset dirty flag
	fDirty = false;
	return true;
}

bool CATLineFit::GetMinMax( CATFloat64& minX, CATFloat64& minY, CATFloat64& maxX, CATFloat64& maxY)
{
	CATUInt32 numPoints = GetNumPoints();
	if (numPoints == 0)
	{
		return false;
	}
	

	for (CATUInt32 i=0; i<numPoints; i++)
	{
		CATPoint curPoint = fPointList[i];

		if (i == 0)
		{
			minX = maxX = curPoint.x;
			minY = maxY = curPoint.y;
		}
		else
		{
			minX = CATMin(curPoint.x, minX);
			maxX = CATMax(curPoint.x, maxX);
			minY = CATMin(curPoint.y, minY);
			maxY = CATMax(curPoint.y, maxY);
		}
	}

	return true;
}

bool CATLineFit::Slope( CATFloat64& slope)
{
	if (fDirty)
	{
		if (!CalcFit())
		{
			return false;
		}
	}

	slope = fSlope;

	return true;
}

bool CATLineFit::Intercept(CATFloat64& intercept)
{
	if (fDirty)
	{
		if (!CalcFit())
		{
			return false;
		}
	}
	intercept = fIntercept;
	return true;
}
