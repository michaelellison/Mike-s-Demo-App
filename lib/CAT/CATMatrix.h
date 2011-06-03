/// \file CATMatrix.h
/// \brief Simple class for matrix operations
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CRMATRIX_H
#define _CRMATRIX_H

#include "CATInternal.h"
#include <math.h>

/// \class CATMatrix
/// \brief Simple class for matrix operations
/// \ingroup CAT
class CATMatrix
{
	public:
		// Generate a matrix of width and height
		CATMatrix(CATUInt32 w, CATUInt32 h);
		CATMatrix(const CATMatrix& matrix);
		virtual ~CATMatrix();

		CATFloat64& Val(CATUInt32 x, CATUInt32 y);
		
		CATFloat64  cVal(CATUInt32 x, CATUInt32 y) const;
		
		CATUInt32	Width() const		{return fWidth;}
		CATUInt32	Height() const		{return fHeight;}

		bool			operator==	(const CATMatrix& matrix)	const;
		CATMatrix		operator*	(const CATMatrix& matrix)	const;
		CATMatrix		operator*	(const CATFloat64 scalar)		const;
		CATMatrix		operator+	(const CATMatrix& matrix)	const;		
		CATMatrix		operator-	(const CATMatrix& matrix)	const;		

		/// Get the determinant of a matrix
		CATFloat64		GetDeterminant() const;

		/// Get the inversion of the matrix
		CATMatrix		GetInverted() const;

		/// Get the pseudo-inverse of a matrix
		CATMatrix		GetPseudoInverse() const;

		/// Get the transposed matrix
		CATMatrix		GetTransposed() const;

		/// Set to special matricies
		void			SetToIdentity();
		void			ZeroMatrix();

		/// Check for special matricies
		bool			IsNullMatrix()			const;
		bool			IsIdentityMatrix()	const;

		/// Can the matricies be multiplied?
		bool			IsConformable(const CATMatrix& matrix) const;
		

		/// Check for same order between to matricies
		bool			SameOrder(const CATMatrix& matrix) const;

		/// Debug
		void			DebugDump() const;
	protected:
		CATFloat64 *fMatrix;
		CATUInt32	fWidth;
		CATUInt32	fHeight;
};
#endif _CRMATRIX_H
