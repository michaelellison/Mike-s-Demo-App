/// \file CATDates.h
/// \brief Date/time wrappers
/// \ingroup CAT
///
/// Copyright (c) 2010 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
#ifndef __CATDates_H__
#define __CATDates_H__

#include <time.h>
#include "CATInternal.h"
#include "CATString.h"

enum CATDaysOfWeek
{
    kCAT_Sunday      = 0,
    kCAT_Monday      = 1,
    kCAT_Tuesday     = 2,
    kCAT_Wednesday   = 3,
    kCAT_Thursday    = 4,
    kCAT_Friday      = 5,
    kCAT_Saturday    = 6
};

enum CATMonthsOfYear
{
    kCAT_January     = 0,
    kCAT_February    = 1,
    kCAT_March       = 2,
    kCAT_April       = 3,
    kCAT_May         = 4,
    kCAT_June        = 5,
    kCAT_July        = 6,
    kCAT_August      = 7,
    kCAT_September   = 8,
    kCAT_October     = 9,
    kCAT_November    = 0x0a,
    kCAT_December    = 0x0b,
};

enum CATRepeatDate
{
    kCAT_Rep_None             = 0,

    // Days
    kCAT_Rep_Sundays          = 0x00000001,
    kCAT_Rep_Mondays          = 0x00000002, // 1 << kCD_Monday
    kCAT_Rep_Tuesday          = 0x00000004, // 1 << kCD_Tuesday
    kCAT_Rep_Wednesday        = 0x00000008, // 1 << kCD_Wednesday
    kCAT_Rep_Thursday         = 0x00000010,
    kCAT_Rep_Friday           = 0x00000020,
    kCAT_Rep_Saturday         = 0x00000040,
    
    // Day combos
    kCAT_Rep_EveryDay         = 0x0000007f,
    kCAT_Rep_Weekdays         = 0x0000003e,
    kCAT_Rep_MonWedFri        = 0x0000002c,
    kCAT_Rep_TueThur          = 0x00000014,
    kCAT_Rep_SatSun           = 0x00000041,

    kCAT_Rep_Weekly_Day       = 0x00001000, //  1yxx, where y is freq and xx is days.
                                            //        e.g. 1314 = every three weeks on tues/thurs.

    kCAT_Rep_Monthly_MDay     = 0x00002000, //  2ydd, where y is freq, and where dd is the 1-based day of month.
                                            //        e.g. 1020b = Every 2 months on the 11th day.
    
    kCAT_Rep_Monthly_WDay     = 0x00010000, // 1zyxx, where z is index, y is freq, and xx is weekday from kCAT_Rep_.
                                            //        e.g. 12304 = every three months on the third tuesday.
                                            //        (third = 2, as is zero based)

    kCAT_Rep_Yearly_MDay      = 0x00020000, //  4mydd, where m is month #, y is freq, and dd is the 1-based day of month.
                                            //        e.g. 20201 = every 2 years on January 1st.
};


#endif // __CATDates_H__