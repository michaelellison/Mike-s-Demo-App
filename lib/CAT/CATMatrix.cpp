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

#include "CATMatrix.h"

// Generate a matrix of width and height
CATMatrix::CATMatrix(CATUInt32 w, CATUInt32 h)
{
	fWidth		= w;
	fHeight		= h;
	fMatrix		= new CATFloat64[w*h];
	this->ZeroMatrix();	
}

// Copy constructor
CATMatrix::CATMatrix(const CATMatrix& matrix)
{
	this->fWidth	= matrix.fWidth;
	this->fHeight	= matrix.fHeight;
	fMatrix = new CATFloat64[this->fWidth * this->fHeight];
	for (CATUInt32 x = 0; x < fWidth; x++)
	{
		for (CATUInt32 y=0; y < fHeight; y++)
		{
			this->Val(x,y) = matrix.cVal(x,y);
		}
	}
}

// Destructor
CATMatrix::~CATMatrix()
{
	if (fMatrix != 0)
	{
		delete [] fMatrix;
		fMatrix = 0;
	}
}

// Retrieve the value by reference
CATFloat64& CATMatrix::Val(CATUInt32 x, CATUInt32 y)
{
	CATASSERT( (y < fHeight) && (y >= 0) && (x >= 0) && (x < fWidth), "Invalid position in matrix!");
	return fMatrix[x+y*fWidth];
}

// Const version of value
CATFloat64 CATMatrix::cVal(CATUInt32 x, CATUInt32 y) const
{
	CATASSERT( (y < fHeight) && (y >= 0) && (x >= 0) && (x < fWidth), "Invalid position in matrix!");
	return fMatrix[x+y*fWidth];
}


// Set the matrix to the identity matrix
void CATMatrix::SetToIdentity()
{
	for (CATUInt32 x=0; x<fWidth; x++)
	{
		for (CATUInt32 y=0; y<fHeight; y++)
		{
			if (x == y)
			{
				this->Val(x,y) = 1;
			}
			else
			{
				this->Val(x,y) = 0.0;
			}
		}
	}	
}

// Zero the matrix
void CATMatrix::ZeroMatrix()
{
	for (CATUInt32 x=0; x<fWidth; x++)
	{
		for (CATUInt32 y=0; y<fHeight; y++)
		{
			this->Val(x,y) = 0.0;
		}
	}
}

// Check for null
bool CATMatrix::IsNullMatrix() const
{
	for (CATUInt32 x=0; x<fWidth; x++)
	{
		for (CATUInt32 y=0; y< fHeight; y++)
		{
			if (this->cVal(x,y) != 0)
			{
				return false;
			}
		}
	}
	return true;
}

// Check for identity
bool CATMatrix::IsIdentityMatrix() const
{
	for (CATUInt32 x=0; x<fWidth; x++)
	{
		for (CATUInt32 y=0; y<fHeight; y++)
		{
			if (x == y)
			{
				if (this->cVal(x,y) != 1)
				{
					return false;
				}
			}
			else
			{
				if (this->cVal(x,y) != 0.0)
				{
					return false;
				}
			}
		}
	}	

	return true;
}

// Compare two matricies for equality
bool CATMatrix::operator==(const CATMatrix& matrix) const
{
	if ((matrix.fWidth  != this->fWidth) || 
		 (matrix.fHeight != this->fHeight))
	{
		return false;
	}

	for (CATUInt32 x=0; x < fWidth; x++)
	{
		for (CATUInt32 y=0; y < fHeight; y++)
		{
			if (this->cVal(x,y) != matrix.cVal(x,y))
			{
				return false;
			}
		}
	}

	return true;
}

// Add two matricies
CATMatrix		CATMatrix::operator+	(const CATMatrix& matrix) const
{	
	if (!this->SameOrder(matrix))
	{
		CATASSERT(false,"Can't add two matricies of differing orders.");
		throw;
	}
	
	CATMatrix sum(fWidth,fHeight);
	sum.ZeroMatrix();

	for (CATUInt32 i = 0; i < this->fWidth; i++)
	{
		for (CATUInt32 j = 0; j < this->fHeight; j++)
		{
			sum.Val(i,j) = this->cVal(i,j) + matrix.cVal(i,j);
		}
	}

	return sum;
}


// Subtract two matricies (this - matrix)
CATMatrix		CATMatrix::operator-	(const CATMatrix& matrix) const
{

	if (!this->SameOrder(matrix))
	{
		CATASSERT(false,"Can't add two matricies of differing orders.");
		throw;
	}
	
	CATMatrix sum(fWidth,fHeight);

	for (CATUInt32 i = 0; i < this->fWidth; i++)
	{
		for (CATUInt32 j = 0; j < this->fHeight; j++)
		{
			sum.Val(i,j) = this->cVal(i,j) - matrix.cVal(i,j);
		}
	}

	return sum;
}

//  Check to see if the two matricies may be multiplied
bool CATMatrix::IsConformable(const CATMatrix& matrix) const
{
	if (this->fWidth != matrix.fHeight)
	{
		return false;
	}

	return true;
}	
	
// Find the product of two matricies (this*matrix)
CATMatrix		CATMatrix::operator*	(const CATMatrix& matrix) const
{
	CATMatrix product(matrix.fWidth, this->fHeight);
	product.ZeroMatrix();

	CATASSERT( this->fWidth == matrix.fHeight, "Can only multiply matricies where 1st matrix's width == 2nd matrix's height");
	if (fWidth != matrix.fHeight)
	{
		return product;
	}

	for (CATUInt32 x = 0; x < matrix.fWidth; x++)
	{
		for (CATUInt32 y=0; y < this->fHeight; y++)
		{
			for (CATUInt32 i = 0; i < this->fWidth; i++)
			{
				product.Val(x,y) = product.cVal(x,y) + this->cVal(i,y) * matrix.cVal(x,i);
			}
		}
	}

	return product;
}

// Find the product of this matrix and a scalar
CATMatrix		CATMatrix::operator*	(const CATFloat64 scalar) const
{

	CATMatrix product(fWidth,fHeight);

	for (CATUInt32 i = 0; i < this->fWidth; i++)
	{
		for (CATUInt32 j = 0; j < this->fHeight; j++)
		{
			product.Val(i,j) = this->cVal(i,j) * scalar;
		}
	}

	return product;
}

// Check if the two matricies are of the same order
bool CATMatrix::SameOrder(const CATMatrix& matrix) const
{
	if ((this->fWidth != matrix.fWidth) || (this->fHeight != matrix.fHeight))
	{
		return false;
	}

	return true;
}

CATFloat64 CATMatrix::GetDeterminant() const
{

	CATASSERT(fWidth == fHeight, "Determinants may only be gotten from square matricies...");
	CATASSERT(fWidth > 1, "Determinants may only be gotten from square matricies of at least 2x2");
	
	// Bail on invalid width/height
	if (fWidth != fHeight)
	{
		return 0.0;
	}
	if (fWidth < 2)
	{
		return 0.0;
	}

	CATUInt32 n = fWidth;
		

	// If we've got a 2x2 matrix, we can calculate it directly.
	if (n == 2)
	{
		return (this->cVal(0,0) * this->cVal(1,1)) - 
				 (this->cVal(1,0) * this->cVal(0,1));
	}

	// Otherwise, go across the columns and calc sub matricies recursively
	CATFloat64 determ = 0;
	
	for ( CATUInt32 j1 = 0; j1 < n; j1++) 
	{
		CATMatrix m(n-1,n-1);

		for (CATUInt32 i=1; i < n; i++) 
		{
			CATUInt32 j2 = 0;
			
			for (CATUInt32 j=0;j<n;j++) 
			{
				if (j != j1)
				{				
					m.Val(i - 1, j2) = this->cVal(i,j);				
					j2++;
				}
			}
		}
		// Calc each sub matricies' determinant 
		determ += pow(-1.0,(CATFloat64)j1) * this->cVal(0,j1) * m.GetDeterminant();
	}

   return(determ);
}


CATMatrix CATMatrix::GetInverted() const
{
	CATMatrix inversion(fWidth,fHeight);
	
	// Find the determinant of the matrix
	CATFloat64 determ = this->GetDeterminant();
	CATUInt32 i,j;

	// calc the inversion matrix
	for (i = 0; i < fWidth; i++)
	{
		for (j = 0; j < fHeight; j++)
		{
			CATUInt32 i1, j1, i2, j2;

			// Build matrix to find minor
			CATMatrix m(fWidth-1,fHeight-1);			
			i2 = 0;
			for (i1 = 0; i1 < fWidth; i1++)
			{
				if (i1 != i)
				{
					j2 = 0;
					for (j1 = 0; j1 < fHeight; j1++)
					{
						if (j1 != j)
						{		
							m.Val(i2,j2) = this->cVal(i1,j1);
							j2++;
						}
					}
					i2++;
				}
			}
			
			// Calc inversion
			inversion.Val(j,i) = pow(-1.0,(CATFloat64)i+j) *  (m.GetDeterminant() / determ);
		}
	}

	return inversion;
}


// Dump the matrix to debugger
void CATMatrix::DebugDump() const
{
#ifdef CAT_CONFIG_WIN32
	CATWChar matrixTmp[100];
	wsprintf(matrixTmp,L"\nMatrix: (%d,%d):\n",fWidth,fHeight);
	::OutputDebugString(matrixTmp);
	for (CATUInt32 y=0; y<fHeight; y++)
	{
		for (CATUInt32 x=0; x<fWidth; x++)
		{
			wsprintf(matrixTmp,L"%f  ",this->cVal(x,y));
			::OutputDebugString(matrixTmp);
		}
		::OutputDebugString(L"\n");
	}
#endif
}


// Create a transposed copy of the matrix
//
// e.g. the following matrix:
//
// 1 2 3
// 4 5 6
//        becomes
//                   1 4
//                   2 5
//                   3 6
//
CATMatrix CATMatrix::GetTransposed() const
{
	CATMatrix newMatrix(this->fHeight, this->fWidth);

	for (CATUInt32 x=0; x < this->fWidth; x++)
	{
		for (CATUInt32 y=0; y<this->fHeight; y++)
		{
			newMatrix.Val(y,x) = this->cVal(x,y);
		}
	}

	return newMatrix;
}

// Pseudo inverse of a matrix is  ((At * A)^-1) * At
// Or, the product of the transposed matrix by the inverted product of the transposed matrix and the original matrix
CATMatrix CATMatrix::GetPseudoInverse() const
{
	// Find At
	CATMatrix transposed	= this->GetTransposed();

	// (At*A)
	CATMatrix product		= transposed * (*this);

	// (At*A)^-1
	CATMatrix inverted		= product.GetInverted();

	// (At*A)^-1 * At
	CATMatrix result		= inverted * transposed;

	return result;
}