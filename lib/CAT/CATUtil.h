//---------------------------------------------------------------------------
/// \file CATUtil.h
/// \brief Defines basic utility functions
/// \ingroup CAT
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------
#ifndef CATUtil_H_
#define CATUtil_H_

// limits.h allows us to assert on floats that are too large to round
// into integer types
#include <limits.h>
#include <math.h>

#include "CATDebug.h"

enum CATVALUE_TYPE
{
	CATVALUE_LINEAR,
	CATVALUE_DB
};

const CATFloat32 kCATPI		  = (CATFloat32)(3.1415926535897932384626433832795);
const CATFloat32 kCATPI_2	  = kCATPI/2;
const CATFloat32 kCAT2_PI	  = kCATPI*2;
const CATFloat32 kCATDEGTORAD = (CATFloat32)(kCATPI / 180.0);
const CATFloat32 kCATRADTODEG = (CATFloat32)(180.0 / kCATPI);
const CATFloat32 kCATINV2PI   = (CATFloat32)(1.0 / ( 8 * 0.78539816339744830961566084581988));

/// CATMin returns the minimum of two values. Types must be the same.
template<class T>
inline T const& CATMin(T const& a, T const& b)
{
   // return minimum
   return a < b ? a : b;
}

/// CATMax returns the maximum of two values. Types must be the same.
template<class T>
inline T const& CATMax(T const& a, T const& b)
{
   // return maximum
   return a > b ? a : b;
}

/// CATSwap swaps to values of the same type
template<class T>
inline void CATSwap(T& a, T& b)
{
   T c = a;
   a = b;
   b = c;
}

/// CATAbs finds the absolute value
template<class T>
inline T CATAbs(T const& a)
{
   if (a < 0)
      return -a;
   return a;
}

/// CATAbsDiff returns the absolute difference between values
template<class T>
inline T CATAbsDiff(T const& a, T const& b)
{
   T c = a - b;
   if (c < 0)
      return -c;
   return c;
}

/// Round a floating point to the nearest integer
/// Assumption: float must be within valid integer range.
/// \param floatVal - floating point value to round. 
///        Must be within integer range.
/// \return CATInt32 - floatVal rounded to nearest integer
inline CATInt32 CATRound(CATFloat32 floatVal)
{
   CATASSERT( (floatVal >= INT_MIN) && (floatVal <= INT_MAX),
             "Value is too large to round to an integer.");

   if (floatVal < 0) 
   {
      return (CATInt32)(floatVal - 0.5f);
   }
   else
   {
      return (CATInt32)(floatVal + 0.5f);
   }   
}  

/// Round a double-precision floating point to the nearest integer
/// Assumption: float must be within valid integer range.
/// \param dblVal - double-precision floating point value to round. 
///        Must be within integer range.
/// \return CATInt32 - dblVal rounded to nearest integer
inline CATInt32 CATRound(CATFloat64 dblVal)
{
   CATASSERT( (dblVal >= INT_MIN) && (dblVal <= INT_MAX),
             "CATRound returns an integer, so the passed in value " \
             "to round must be within range for an int. It isn't.");

   if (dblVal < 0) 
   {
      return (CATInt32)(dblVal - 0.5);
   }
   else
   {
      return (CATInt32)(dblVal + 0.5);
   }   
}  

inline CATFloat32 CATConstrainAngle( CATFloat32 angle)
{
   int k = (int)(angle * kCATINV2PI);
   return angle - ((CATFloat32)k * kCAT2_PI);   
}

inline CATFloat64 CATConstrainAngle( CATFloat64 angle)
{
   int k = (int)(angle * kCATINV2PI);
   return angle - ((CATFloat32)k * kCAT2_PI);
}

/// CATModFloat() performs 'retVal = modFloat % modBase', keeping the floating point 
/// fraction in retVal.  e.g.  CATModFloat( 7.5, 3 )  would return 1.5.
inline CATFloat32 CATModFloat (CATFloat32 modFloat, CATUInt32 modBase)
{
	CATUInt32 tmpVal = (CATUInt32)modFloat;
	CATFloat32 diff   = modFloat - tmpVal;
	tmpVal = tmpVal % modBase;
	CATFloat32 result = (CATFloat32)tmpVal + diff;
	CATASSERT(result < modBase, "Huh... that shouldn't happen...");
	return result;
}

inline CATFloat64 CATModFloat (CATFloat64 modFloat, CATUInt32 modBase)
{
	CATUInt32 tmpVal = (CATUInt32)modFloat;
	CATFloat64 diff   = modFloat - tmpVal;
	tmpVal = tmpVal % modBase;
	
	CATFloat64 result = (CATFloat64)tmpVal + diff;
	CATASSERT(result < modBase, "Huh... that shouldn't happen...");
	return result;
}

// Find the greatest common denominator with Euclid's method
inline CATUInt32 CATGreatestCommonDenominator( CATUInt32 a, CATUInt32 b)
{
	if (b == 0)
	{
		CATASSERT(false,"Greatest common denominator doesn't work with 0...");
		return 0;
	}

	while (a > 0)
	{
		if (a < b) CATSwap(a,b);
		a = a - b;
	}

	return b;
}

inline CATInt32 CATGreatestCommonDenominator( CATInt32 a, CATInt32 b)
{
	if (b == 0)
	{
		CATASSERT(false,"Greatest common denominator doesn't work with 0...");
		return 0;
	}

	while (a > 0)
	{
		if (a < b) CATSwap(a,b);
		a = a - b;
	}

	return b;
}

// Find the lowest common multiple
inline CATUInt32 CATLowestCommonMultiple( CATUInt32 a, CATUInt32 b)
{
	if (b == 0)
	{
		CATASSERT(false,"Lowest common multiple doesn't like 0.");
		return 0;
	}

	CATUInt32 gcd = CATGreatestCommonDenominator(a,b);	
	return (a*b) / gcd;
}

inline CATInt32 CATLowestCommonMultiple( CATInt32 a, CATInt32 b)
{
	if (b == 0)
	{
		CATASSERT(false,"Lowest common multiple doesn't like 0.");
		return 0;
	}

	CATInt32 gcd = CATGreatestCommonDenominator(a,b);
	return (a*b) / gcd;
}

/// Swap endian of value if and only if we're on a little-endian machine
inline CATInt32 CATSwapLittleEndian( CATInt32 a )
{
#ifdef CAT_LITTLE_ENDIAN
	   a =    ( ((a & 0xFF) << 24)    | 
				((a & 0xff00) << 8)	 | 
				((a & 0xff0000) >> 8) |
				((a & 0xff000000) >> 24));
#endif

	return a;
}

inline CATUInt32 CATSwapLittleEndian( CATUInt32 a )
{
#ifdef CAT_LITTLE_ENDIAN
	   a =    ( ((a & 0xFF) << 24)    | 
				((a & 0xff00) << 8)	 | 
				((a & 0xff0000) >> 8) |
				((a & 0xff000000) >> 24));
#endif

	return a;
}


inline CATUInt16 CATSwapLittleEndian( CATUInt16 a )
{
#ifdef CAT_LITTLE_ENDIAN
	a =    ( ((a & 0xFF) << 8)    | 
				((a & 0xff00) >> 8));
#endif

	return a;
}

#endif // CATUtil_H_


