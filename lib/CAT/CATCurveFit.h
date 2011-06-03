/// \file CATCurveFit.h
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

#ifndef _CATCURVEFIT_H_
#define _CATCURVEFIT_H_

#include "CATInternal.h"
#include "CATPoint.h"
#include "CATMatrix.h"

/// \class CATCurveFit
/// \brief  Fit a curve to a set of points
/// \ingroup CAT
///
/// Fits a curve of the specified degree to a set of data.
/// Based off the algorithm presented in "Introduction to Algorithms" Second Edition,
///    by Cormen, Leiserson, Rivest, and Stein (MIT press)
///
class CATCurveFit
{
	public:		
		/// Pass in the number of coefficients you want to calculate.
		/// defaults to quadratic.  For line, use 2.
		CATCurveFit(CATUInt32 curveDegree = 3);

		virtual ~CATCurveFit();

		/// Add a point to the interpolation data
		bool		AddPoint( CATFloat64 x, CATFloat64 y);

		/// Clear out all current data
		bool		Clear();
		
		/// Get the polynomial degree
		bool		GetDegree(CATUInt32 &degree);

		/// Get a specific coefficient
		bool		GetCoefficient( CATUInt32 deg, CATFloat64& coef);
		
		/// Get the number of data points acquired
		CATUInt32   GetNumPoints();

		/// Get an individual data point
		bool		GetDataPoint( CATUInt32 n, CATFloat64& x, CATFloat64& y);
		
		/// Calculate the Y val from a specified x val
		bool		CalcYVal( CATFloat64 x, CATFloat64& y);

		/// Get error value
		bool		GetCurrentErr( CATFloat64& err);

		bool		GetMinMax( CATFloat64& minX, CATFloat64& minY, CATFloat64& maxX, CATFloat64& maxY);

		bool		LangrangianCalcY(CATFloat64 x, CATFloat64 &y);
	protected:		
		bool		CalcFit();	// Internal calc 	
		
        std::vector <CATPoint> fPointList; ///< list of X points
        std::vector <CATFloat64> fCoef;    ///< list of coefficients (double's)		
		
		CATFloat64	fLastErr;	           ///< last calculated err

		CATUInt32   fDegree;               ///< Degree of coefficient to calculate with fit
				
		bool		fDirty;                ///< Are the coefficients dirty? (i.e. need to call CalcFit?)
};

#endif // _CATCURVEFIT_H_