/// \file CATFilter.h
/// \brief Parent filtering class for one-dimensional filters.
/// \ingroup CAT
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATFilter_H_
#define _CATFilter_H_

#include "CATInternal.h"
#include "CATMathUtil.h"

/// \class CATFilter
/// \brief Parent filtering class for one-dimensional filters.
/// \ingroup CAT
///
/// The base class performs no filtering. Filters should be derived
/// from it to present a common interface.
class CATFilter
{
	public:
		CATFilter();
		virtual ~CATFilter();

		/// Filter() is the primary command for all derived classes,
		/// but it's now inlined for speed instead of being virtual.
        /// CATFloat32 Filter();

        /// GetLastFiltered() retrieves the last output value from the filter.
        ///
        /// \return CATFloat32 - last output from Filter() call.
        /// \sa Filter(), GetLastRaw()
		CATFloat32	GetLastFiltered();

        /// GetCount() retrieves the number of items filtered since the last
        /// reset (or since instantiation).
        ///
        /// \return CATUInt32 - number of filtered inputs
        /// \sa Reset()
		CATUInt32	GetCount();

        /// Reset() resets the filter and filter count.
		virtual void Reset();

	protected:
		CATFloat32 fLastFiltered;
};


#endif // _CATFilter_H_
