//---------------------------------------------------------------------------
/// \file CATFilterEWMA.cpp
/// \brief Exponentially weighted moving average filter (EWMA / Low pass)
/// \ingroup CAT
///
/// f(x) = coef * f(x-1) + (1-coef) * x
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------

#include "CATFilterEWMA.h"

//---------------------------------------------------------------------------
// CATFilterEWMA() - constructor for EWMA filter
//
// \param coefficient - filter coefficient. High values = slower movement.
//---------------------------------------------------------------------------
CATFilterEWMA::CATFilterEWMA(CATFloat32 coefficient) 
: CATFilter()
{
	if (coefficient > .999f)
	{
		coefficient = .999f;
	}

	fCoef = CATTruncDenormals(coefficient);
}

CATFilterEWMA::~CATFilterEWMA()
{
	
}
		
//---------------------------------------------------------------------------
// SetCoefficient() sets the filter coefficient.
//
// \param coef - filter coefficient     
//---------------------------------------------------------------------------
void CATFilterEWMA::SetCoefficient(CATFloat32 coef)
{
	if (coef > .999f)
	{
		coef = .999f;
	}
	fCoef = CATTruncDenormals(coef);
}

CATFloat32 CATFilterEWMA::GetCoefficient()
{
	return fCoef;
}