/// \file CATFilterEWMA.h
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
#ifndef _CATFilterEWMA_H_
#define _CATFilterEWMA_H_

#include "CATFilter.h"

/// \class CATFilterEWMA
/// \brief Exponentially weighted moving average filter (EWMA / Low pass)
/// \ingroup CAT
///
/// f(x) = coef * f(x-1) + (1-coef) * x
///
class CATFilterEWMA : public CATFilter
{
	public:
        /// CATFilterEWMA() - constructor for EWMA filter
        ///
        /// \param coefficient - filter coefficient. High values = slower movement.
        CATFilterEWMA(CATFloat32 coefficient);

        virtual ~CATFilterEWMA();

        /// SetCoefficient() sets the filter coefficient.
        ///
        /// \param coef - filter coefficient     
        void	  SetCoefficient( CATFloat32 coef);

        CATFloat32 GetCoefficient();

        /// Filter() - add a value and get the next one
        inline CATFloat32 Filter( CATFloat32 rawVal )
        {			
            fLastFiltered = CATTruncDenormals((fCoef * fLastFiltered) + (1.0f - fCoef) * rawVal);
            return fLastFiltered;
        }

	protected:
		CATFloat32	fCoef;
};


#endif //_CATFilterEWMA_H_
