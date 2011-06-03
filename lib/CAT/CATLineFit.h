/// \file CATLineFit.h
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
#ifndef _CATLineFit_H_
#define _CATLineFit_H_

#include "CATInternal.h"
#include "CATPoint.h"
#include "CATMatrix.h"


/// \class CATLineFit
/// \brief Regression to fit a line to a set of points.
/// \ingroup CAT
class CATLineFit
{
	public:		
		CATLineFit();
		virtual ~CATLineFit();

		/// Add a point to the interpolation data
		bool		AddPoint( CATFloat64 x, CATFloat64 y);

		/// Clear out all current data
		bool		Clear();
		
		/// Get the number of data points acquired
		CATUInt32   GetNumPoints();

		/// Get an individual data point
		bool		GetDataPoint( CATUInt32 n, CATFloat64& x, CATFloat64& y);
		
		/// Calculate the Y val from a specified x val
		bool		CalcYVal( CATFloat64 x, CATFloat64& y);

		/// Get error value
		bool		GetCurrentErr( CATFloat64& err);

        /// Get min/max values for x and y
		bool		GetMinMax( CATFloat64& minX, CATFloat64& minY, CATFloat64& maxX, CATFloat64& maxY);

		/// Get the slope of the line 
		bool		Slope(CATFloat64& slope);

		/// Get the intercept of the line 
		bool 		Intercept(CATFloat64& intercept);
	protected:		
        /// Internal calculation
		bool		CalcFit();	
		
        std::vector<CATPoint> fPointList;	///< list of X points		
		CATFloat64	          fLastErr;	    ///< last calculated err

		CATUInt32   fDegree;		///< Degree of coefficient to calculate with fit		
		bool		fDirty;			///< Are the coefficients dirty? (i.e. need to call CalcFit?)
		CATFloat64	fSlope;			///< If not dirty, this is the slope of the line
		CATFloat64  fIntercept;		///< If not dirty, this is the intercept of the line
		CATFloat64	fSumX;			///< The sum of all X values
		CATFloat64	fSumY;          ///< The sum of all Y values
		CATFloat64	fSumXY;			///< The sum of all X values times their Y values
		CATFloat64	fSumXSquared;	///< The sum of all X^2  (e.g. X1^2 + X2^2 .... + Xn^2)
};

#endif // _CATLineFit_H_