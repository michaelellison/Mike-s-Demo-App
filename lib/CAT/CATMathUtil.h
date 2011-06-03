//---------------------------------------------------------------------------
/// \file CATMathUtil.h
/// \brief Generic math/audio utility functions
/// 
///
/// Copyright (c) 2004-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
///
//---------------------------------------------------------------------------
#ifndef _CATMathUtil_H_
#define _CATMathUtil_H_

#include "CATInternal.h"
#include <math.h>

/// CATLinearToDBValue() converts a linear value
/// to a dB value from -inf to +6 for volume sliders and
/// the like. 0dB is at about 0.708142, +6 is at 1.0f.
///
/// \param linearVal - input linear value (0 - 1.0f)
/// \return CATFloat32 - dbVal (-inf to +6)
inline CATFloat32 CATLinearToDBValue(CATFloat32 linearVal)
{
	return (CATFloat32)(40 * (log10(linearVal*10))) - 34;
}

/// CATDBValueToLinear() inverts the above CATLinearToDBValue()
inline CATFloat32 CATDBValueToLinear( CATFloat32 dbVal )
{
	CATFloat32 linearVal = (CATFloat32)pow(10,(dbVal + 34.0f)/40.0f) / 10.0f;
	return linearVal;
}

/// CATLinearToDBGain() converts a linear value
/// to a gain multiplier for volume sliders and the like.
/// \param linearVal - input linear value (0 - 1.0f)
/// \return CATFloat32 - gain multiplier (0.0f - 2.0f)
inline CATFloat32 CATLinearToDBGain( CATFloat32 linearVal)
{
	if (linearVal == 0.0f)
		return 0.0f;
	
	CATFloat32 dbVal = (CATLinearToDBValue(linearVal));

	CATFloat32 powVal = (CATFloat32)pow(10, dbVal/20.0f);

	return (CATFloat32)powVal;
}

/// CATSampleToDBF() converts a sample value to
/// dBF value
///
/// \param sample - sample value (-1.0f -> 1.0f)
/// \return CATFloat32 - dBF value (-inf to 0.0)
inline CATFloat32 CATSampleToDBF( CATFloat32 sample )
{
	CATFloat32 dBF = (CATFloat32)(20 * log10(fabs(sample)));
	return dBF;
}

/// CATSampleToDBFMeter converts a sample value to a dBF value,
/// then from -96.0 to 0 dBF on a linear scale for a meter.
///
/// \param sample - sample value (-1.0f -> 1.0f)
/// \return meter value (0 - 1.0f)
inline CATFloat32 CATSampleToDBFMeter( CATFloat32 sample)
{
	CATFloat32 dBF = CATSampleToDBF(sample);
	if (dBF <= -96.0f)
	{
		return 0.0f;
	}

	return 1.0f + (dBF / 96.0f);
}

/// CATSamplesToBeats() converts the number of samples at a given sample rate and tempo into
/// the number of beats
/// \param numSamples - number of samples
/// \param sampleRate - sample rate (e.g. 44100.0f)
/// \param tempo - tempo in bpm
/// \return CATFloat32 - number of beats (fractional)
inline CATFloat32 CATSamplesToBeats( CATUInt32 numSamples, CATFloat32 sampleRate, CATFloat32 tempo)
{
	CATASSERT(sampleRate != 0.0f, "Invalid sample rate!");
	return (numSamples / sampleRate) * (tempo / 60.0f);
}

/// CATBeatsToSamples() converts the number of beats at a given sample rate and tempo to
/// the number of samples (Frames)
/// \param numBeats - number of beats to convert - may be fractional
/// \param sampleRate - sample rate (e.g. 44100.0f)
/// \param tempo - tempo in bpm
/// \return CATFloat32 - number of samples (may be fractional)
inline CATFloat32 CATBeatsToSamples( CATFloat32 numBeats, CATFloat32 sampleRate, CATFloat32 tempo)
{
	return (numBeats * sampleRate) / (tempo / 60.0f);
}

/// CATSamplesToBeats() converts the number of samples at a given sample rate and tempo into
/// the number of beats
/// \param numSamples - number of samples
/// \param sampleRate - sample rate (e.g. 44100.0f)
/// \param tempo - tempo in bpm
/// \return CATFloat32 - number of beats (fractional)
inline CATFloat64 CATSamplesToBeats( CATUInt32 numSamples, CATFloat64 sampleRate, CATFloat64 tempo)
{
	CATASSERT(sampleRate != 0.0f, "Invalid sample rate!");
	return (numSamples / sampleRate) * (tempo / 60.0f);
}

/// CATBeatsToSamples() converts the number of beats at a given sample rate and tempo to
/// the number of samples (Frames)
/// \param numBeats - number of beats to convert - may be fractional
/// \param sampleRate - sample rate (e.g. 44100.0f)
/// \param tempo - tempo in bpm
/// \return CATFloat32 - number of samples (may be fractional)
inline CATFloat64 CATBeatsToSamples( CATFloat64 numBeats, CATFloat64 sampleRate, CATFloat64 tempo)
{
	return (numBeats * sampleRate) / (tempo / 60.0f);
}


/// CATInterpolateCubic() returns the cubic interpolation of a sample in a buffer of samples.
///
/// Note: if you attempt to use on the final sample in a buffer, result is weighted towards
///       the base sample.
///
/// \param buffer - ptr to buffer to interpolate from
/// \param position - floating point index into buffer to interpolate
/// \param bufSize - number of samples in buffer
/// \return CATFloat32 - interpolated sample value
inline CATFloat32 CATInterpolateCubic( CATFloat32* buffer, CATFloat32 position, CATUInt32 bufSize)
{
	// Cubic interpolation of sound
	CATUInt32 basePos	= (CATUInt32)position;
	CATFloat32 diff		= position - basePos;

	CATFloat32 xm1 = (basePos > 0)?buffer[basePos - 1] : buffer[basePos];
	CATFloat32 x0  = buffer[basePos];
	CATFloat32 x1  = (basePos < bufSize - 1)?buffer[basePos + 1] : buffer[basePos];
	CATFloat32 x2  = (basePos < bufSize - 2)?buffer[basePos + 2] : buffer[basePos];

	CATFloat32 a = (3 * (x0-x1) - xm1 + x2) / 2.0f;
	CATFloat32 b = 2*x1 + xm1 - (5*x0 + x2) / 2.0f;
	CATFloat32 c = (x1 - xm1) / 2.0f;
	return ((((a * diff) + b) * diff + c) * diff + x0);	
}

// Hermite interpolation
inline CATFloat32 CATInterpolateHermite( CATFloat32* buffer, CATFloat32 position, CATUInt32 bufSize)
{
	CATUInt32 basePos = (CATUInt32)position;
	CATFloat32 diff = position - basePos;

	CATFloat32 xm1 = (basePos > 0)?buffer[basePos - 1] : buffer[basePos];
	CATFloat32 x0  = buffer[basePos];
	CATFloat32 x1  = (basePos < bufSize - 1)?buffer[basePos + 1] : buffer[basePos];
	CATFloat32 x2  = (basePos < bufSize - 2)?buffer[basePos + 2] : buffer[basePos];

	CATFloat32 c = (x1 - xm1) * 0.5f;
	CATFloat32 v = x0 - x1;
	CATFloat32 w = c + v;
	CATFloat32 a = w + v + (x2 - x0) * 0.5f;
	CATFloat32 b = w + a;
	return ((((a * diff) - b) * diff + c) * diff + x0);
}

/// CATTruncDenormals() truncates denormal floats to 0.
inline CATFloat32 CATTruncDenormals(CATFloat32 value)
{
	if (( (*(unsigned int*)&value) & 0x7f800000) == 0) 
		value = 0.0f;
	
	return value;
}

/// CATIsDenormal() detects denormal floats
inline bool CATIsDenormal(CATFloat32 value)
{
	return (( (*(unsigned int*)&value) & 0x7f800000) == 0);
}

/// CATUndenormal() adds a tiny value to a float to avoid denormals
inline CATFloat32 CATUndenormal(CATFloat32 value)
{
	return (value += 0.0000000000000000000000001f);
}

/// CATInterpolateLinear() returns the linear interpolation of a sample in a buffer of samples.
///
/// Note: if you attempt to use on the final sample in a buffer, result is weighted towards
///       the base sample.
///
/// \param buffer - ptr to buffer to interpolate from
/// \param position - floating point index into buffer to interpolate
/// \param bufSize - number of samples in buffer
/// \return CATFloat32 - interpolated sample value
inline CATFloat32 CATInterpolateLinear( CATFloat32* buffer, CATFloat32 position, CATUInt32 bufSize)
{
	CATUInt32 basePos = (CATUInt32)position;
	CATFloat32 diff	 = position - basePos;
	CATFloat32 x0		 = buffer[basePos];
	CATFloat32 x1		 = (basePos < bufSize-1)?buffer[basePos] : buffer[basePos];
	return (x0 * (1.0f - diff) + (x1 * diff));
}



#endif // _CATMathUtil_H_