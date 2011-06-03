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

#include "CATStats.h"
#include "CATUtil.h"

#include <algorithm>
#include <math.h>
#include <float.h>

CATStats::CATStats()
{
    fNumEntries = 0;
    fSum		= 0;
    fSumSquares = 0;
    fMin		= 0;
    fMax		= 0;
}

CATStats::~CATStats()
{
    this->Clear();
}

void CATStats::Clear()
{
    fEntryList.clear();
    fNumEntries     = 0;
    fSum			= 0;
    fSumSquares     = 0;
    fMin			= 0;
    fMax			= 0;
}

bool CATStats::AddValue(CATFloat64 value)
{	    
    // Add it to our data list
    fEntryList.push_back(value);

    // It's added, so now add to our running calculations
    fSum			+= value;
    fSumSquares     += value*value;

    // Set min/max values if value goes out of current bounds or is first value
    (fNumEntries == 0)?fMin = value:fMin = CATMin(fMin,value);
    (fNumEntries == 0)?fMax = value:fMax = CATMax(fMax,value);

    fNumEntries	+= 1;
    return true;
}


CATFloat64 CATStats::Mean()
{
    return fSum / (CATFloat64)fNumEntries;	
}

CATFloat64 CATStats::Median()
{
    std::sort(fEntryList.begin(), fEntryList.end(), FloatSortCallback);	
    CATFloat64 median = fEntryList[fEntryList.size()/2];
    return median;
}

CATFloat64 CATStats::Min()
{
    return fMin;
}

CATFloat64 CATStats::Max()
{
    return fMax;
}

CATFloat64	CATStats::Variance()
{
    if (fNumEntries == 0)
    {
        return 0;
    }

    CATFloat64 meanSquared = this->Mean();
    meanSquared *= meanSquared;

    CATFloat64 variance = (fSumSquares/(CATFloat64)fNumEntries) - meanSquared;
    return variance;
}

CATFloat64 CATStats::Variance(CATFloat64 sample)
{
    if (fNumEntries == 0)
    {
        return 0;
    }
    CATFloat64 mean = this->Mean();	
    return  ((sample - mean)*(sample - mean) / (CATFloat64)fNumEntries);	
}

CATFloat64 CATStats::StdDev(CATFloat64 sample)
{
    if (fNumEntries == 0)
    {
        return 0;
    }

    return sqrt(Variance(sample));
}

CATFloat64 CATStats::AvgDev()
{
    if (fNumEntries == 0)
    {
        return 0;
    }

    // Avg deviation requires recalc with current mean afaik
    CATFloat64 avgDev = 0;
    CATFloat64 mean = this->Mean();

    for (CATUInt32 i = 0; i < fNumEntries; i++)
    {
        CATFloat64 curVal = fEntryList[i];
        avgDev += fabs((curVal) - mean);
    }
    avgDev /= (CATFloat64)fNumEntries;
    return avgDev;
}

CATFloat64 CATStats::StdDev()
{
    if (fNumEntries == 0)
    {
        return 0;
    }

    return sqrt(Variance());
}

bool CATStats::FloatSortCallback(CATFloat64 p1,CATFloat64 p2)
{
    return p1 < p2;
}

CATFloat64 CATStats::GetValue(CATUInt32 index)
{
    if (index < fNumEntries)
    {
        return fEntryList[index];
    }

    return 0.0;
}

CATFloat64 CATStats::Mode(CATUInt32 steps)
{
    CATFloat64 mode = fMax;
    CATFloat64 stepSize = (fMax - fMin);
    if ((stepSize > DBL_EPSILON*steps) || (stepSize < -DBL_EPSILON*steps))
    {
        CATUInt32 *histogram = new CATUInt32[steps];
        memset(histogram,0,sizeof(CATUInt32)*steps);

        CATUInt32 maxIndex = (CATUInt32)-1;
        CATUInt32 maxVal   = 0;        

        for (CATUInt32 i=0; i < fNumEntries; i++)
        {
            CATUInt32 stepPos = (CATUInt32)((fEntryList[i] - fMin) / stepSize);
            histogram[stepPos]++;
            if (histogram[stepPos] >= maxVal)
            {
                maxIndex = stepPos;
                maxVal = histogram[stepPos];
                mode = (stepPos * stepSize) + fMin;
            }
        }

        delete [] histogram;

        return mode;
    }
    else
    {
        return mode;
    }
}
