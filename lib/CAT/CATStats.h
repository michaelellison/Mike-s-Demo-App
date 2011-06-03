/// \file CATStats.h
/// \brief Basic statistics class
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
#ifndef _CATStats_H
#define _CATStats_H

#include "CATInternal.h"

/// \class CATStats
/// \brief Basic statistics class
/// \ingroup CAT
class CATStats
{
	public:
		CATStats();		
		virtual ~CATStats();		
		
        bool AddValue(CATFloat64 value);
		void Clear();		
		
		
		CATFloat64	Mean();						// Average
		CATFloat64	Median();					// Median
		CATFloat64	Min();						// Minimum
		CATFloat64	Max();						// Maximum
		CATFloat64	Variance();					// Variance
		CATFloat64	StdDev();					// Standard Deviation
		CATFloat64	AvgDev();					// Average Deviation
		CATFloat64	Variance(CATFloat64 sample); // Variance of a sample
		CATFloat64  StdDev  (CATFloat64 sample); // StdDev of a sample
        CATFloat64  GetValue(CATUInt32 index);
        CATFloat64  Mode(CATUInt32 steps);
		CATUInt32  Size() {return fNumEntries;}
        
        

	protected:
		static bool FloatSortCallback(CATFloat64 elem1, CATFloat64 elem2);

		CATFloat64		fMin;
		CATFloat64		fMax;
		CATFloat64		fSum;
		CATFloat64		fSumSquares;
		CATUInt32		fNumEntries;
        std::vector<CATFloat64> fEntryList;
};
#endif _CATStats_H
