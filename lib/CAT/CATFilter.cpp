/// \file CATFilter.cpp
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

#include "CATFilter.h"

//---------------------------------------------------------------------------
CATFilter::CATFilter()
{
	fLastFiltered = 0;
}
//---------------------------------------------------------------------------
CATFilter::~CATFilter()
{
}

//---------------------------------------------------------------------------
// Reset() resets the filter and filter count.
//---------------------------------------------------------------------------
void CATFilter::Reset()
{
	fLastFiltered = 0;
}

//---------------------------------------------------------------------------
// GetLastFiltered() retrieves the last output value from the filter.
//
// \return CATFloat32 - last output from Filter() call.
// \sa Filter(), GetLastRaw()
//---------------------------------------------------------------------------
CATFloat32 CATFilter::GetLastFiltered()
{
	return fLastFiltered;
}

