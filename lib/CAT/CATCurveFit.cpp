/// \file CATCurveFit.cpp
/// \brief Fit a curve to a set of points
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATCurveFit.h"
#include "CATUtil.h"

CATCurveFit::CATCurveFit(CATUInt32 degree)
{
	fDirty	= true;
	if (degree < 3)
	{
		CATASSERT(degree >= 3,"Degree must be 3 or greater for curve fitting. Use CATLineFit for lines.");
		degree = 3;
	}
	fDegree	= degree;
}

CATCurveFit::~CATCurveFit()
{
	Clear();
}

bool CATCurveFit::AddPoint( CATFloat64 x, CATFloat64 y)
{	
	fPointList.push_back(CATPoint(x,y));
	fDirty = true;
	return true;
}

bool CATCurveFit::Clear()
{	
	// Clear all the Lists...
    fCoef.clear();
    fPointList.clear();	

	// Set dirty flag
	fDirty = true;
	return true;
}


bool CATCurveFit::GetDegree(CATUInt32 &degree)
{
	degree = 0;

	// Recalc if table is dirty
	if (fDirty)
	{
		if (!CalcFit())
		{
			return false;
		}
	}

	// return number of coefficients
	degree = (CATUInt32)fCoef.size();
	return degree > 0;
}

bool CATCurveFit::GetCoefficient( CATUInt32 deg, CATFloat64& coef)
{
	// Recalc if the coefficient table is dirty
	if (fDirty)
	{
		if (!CalcFit())
		{
			return false;
		}
	}

	// Bail if degree is invalid
	if (deg >= fCoef.size())
	{
		return false;
	}

	// Get the coefficient and return
	coef = fCoef[deg];
	return true;
}


CATUInt32 CATCurveFit::GetNumPoints()
{	
	return (CATUInt32)fPointList.size();	
}

bool CATCurveFit::GetDataPoint( CATUInt32 n, CATFloat64& x, CATFloat64& y)
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

bool CATCurveFit::GetCurrentErr(CATFloat64 &err)
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



bool CATCurveFit::CalcYVal( CATFloat64 x, CATFloat64& y)
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
	CATUInt32 degree = (CATUInt32)fCoef.size();
	
	for (CATUInt32 i=0; i < degree; i++)
	{
        CATFloat64 coef = fCoef[i];
		y += (coef) * pow(x,(CATFloat64)i);
	}
	
	return true;
}


bool CATCurveFit::CalcFit()
{
	// Return false if we don't have any points to calculate....
	CATUInt32 numPoints = GetNumPoints();
	
	if (numPoints < fDegree)
	{
		CATTRACE("Too few points for calc fit.\n");
		return false;
	}

	// Clear coefficients

    fCoef.clear();

    this->fLastErr = 0.0;


	// Setup for whatever type they instantiated.
	// Eventually we should try different degrees to find the best fit

	CATMatrix primary(fDegree, (CATUInt32)fPointList.size() ); 
	CATMatrix ymatrix(1,       (CATUInt32)fPointList.size() );
	
	// Setup initial matrix for least-squares approximation.
	// primary matrix is setup as:
	//
	// x1^0  x1^1 ... x1^degree
	// x2^0  x2^1 ... x2^degree
	// ........................
	// xn^0  xn^1 ... xn^degree
	//
	// where n = number of points
	//
	// ymatrix is setup as:
	//
	// y1
	// y2
	// ..
	// yn
	//
	CATUInt32 i;
	for (i=0; i < fPointList.size(); i++)
	{
		CATPoint curPoint = fPointList[i];	
		ymatrix.Val(0,i) = curPoint.y;

		for (CATUInt32 curDegree = 0; curDegree < fDegree; curDegree++)
		{
			primary.Val(curDegree,i) = pow(curPoint.x,(CATFloat64)curDegree);
		}
	}

	// Find the pseudoInverse of the primary matrix
	CATMatrix pseudoInverse = primary.GetPseudoInverse();
	
	// Multiply it by the ymatrix
	CATMatrix coefs = pseudoInverse * ymatrix;

	// We now have the coefficients
	for (i = 0; i < coefs.Height(); i++)
	{		
		fCoef.push_back(coefs.cVal(0,i));		
	}

	// Reset dirty flag
	fDirty = false;
	return true;
}

bool CATCurveFit::GetMinMax( CATFloat64& minX, CATFloat64& minY, CATFloat64& maxX, CATFloat64& maxY)
{
	CATUInt32 numPoints = GetNumPoints();
    if (numPoints == 0)
        return false;	

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



//--------------------------------------------------
// Brute force Langrangian interpolation - not really useful 
// for our needs (too slow and unpredictable with large samples,
// and it doesn't like experimental error much).
// But good for testing the class and display....
bool CATCurveFit::LangrangianCalcY( CATFloat64 x, CATFloat64& y)
{
	y = 0.0;
	// Calculate Y val from X and calculated coefficients....
	CATFloat64 sum = 0;
	CATUInt32 degree = (CATUInt32)fPointList.size();	
	
    for (CATUInt32 i = 0; i < degree; i++)
	{
		CATPoint pi = fPointList[i];		

		CATFloat64 p = 1;
		for (CATUInt32 j=0; j < degree; j++)
		{
			if (j != i)
			{				
				CATPoint pj = fPointList[j];
				p = p * (x - pj.x) / (pi.x - pj.x);
			}
		}
				
		sum += p * pi.y;
	}
	y = sum;
	return true;
}

