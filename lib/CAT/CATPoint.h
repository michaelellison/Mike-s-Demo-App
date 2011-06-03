/// \file CATPoint.h
/// \brief Point class, mainly for small sets for clarity of code.
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
#ifndef CATPOINT_H
#define CATPOINT_H

#include "CATTypes.h"
#include "CATUtil.h"
#include "CATColor.h"
#include <math.h>

/// \class CATPoint
/// \brief 2d point class w/size, mainly for small sets for clarity of code.
/// \ingroup CAT
class CATPoint
{
	public:
		CATPoint( const CATPoint& point)
		{
			x=point.x;
			y=point.y;
			size=point.size;
		}

		CATPoint( CATFloat64 xp, CATFloat64 yp, CATFloat64 sizep=0.0)
		{
			x = xp;
			y = yp;
			size = sizep;
		}

		CATPoint()
		{
			Clear();
		}

		void Clear()
		{
			x = 0.0;
			y = 0.0;
			size = 0.0;
		}

		virtual ~CATPoint() 
		{
		
		}

		
		CATPoint operator+(const CATPoint& npoint)
		{
			CATPoint point(*this);
			point.x += npoint.x;
			point.y += npoint.y;
			// size doesn't change.
			return point;
		}

		CATPoint operator-(const CATPoint& npoint)
		{
			CATPoint point(*this);
			point.x -= npoint.x;
			point.y -= npoint.y;
			// size doesn't change.
			return point;
		}

		CATPoint& operator=(const CATPoint& npoint)
		{
			x = npoint.x;
			y = npoint.y;
			size = npoint.size;
			return *this;
		}

		CATPoint operator*(CATFloat64 scaler)
		{
			CATPoint point(*this);
			point.x *= scaler;
			point.y *= scaler;
			// size doesn't change.
			return point;
		}

		CATPoint operator/(CATFloat64 scaler)
		{
			CATPoint point(*this);
			point.x /= scaler;
			point.y /= scaler;
			// size doesn't change.
			return point;
		}

		CATPoint operator*(const CATPoint& npoint)
		{
			CATPoint point(*this);
			point.x *= npoint.x;
			point.y *= npoint.y;
			// size doesn't change.
			return point;
		}
		
	public:
		CATFloat64 x;
		CATFloat64 y;
		// Size has a variety of uses - right now, using for size of lines in line checks
		CATFloat64 size;		
};

/// \class CATScanPoint
/// \brief Cartesian 3D point, used in 3d scanner and the like. Has color
/// \ingroup CAT
class CATScanPoint
{
	public:
		CATScanPoint( const CATScanPoint& point)
		{
			y			=	point.y;
			z			=	point.z;
			rotation =	point.rotation;
			color		=	point.color;
		}

		CATScanPoint( CATFloat64 yp, CATFloat64 zp, CATFloat64 rot_deg = 0, CATUInt32 colorp = 0)
		{			
			y			= yp;
			z			= zp;
			rotation = rot_deg;
			color		= colorp;			
		}

		CATScanPoint()
		{			
			y			= 0.0;
			z			= 0.0;
			color		= 0;
			rotation = 0.0;			
		}

		virtual ~CATScanPoint() 
		{
		
		}
	
		CATScanPoint& operator=(const CATScanPoint& npoint)
		{			
			y			= npoint.y;
			z			= npoint.z;
			rotation = npoint.rotation;
			color		= npoint.color;			
			return *this;
		}

	public:
		CATFloat64 y;
		CATFloat64 z;
		CATFloat64 rotation;
		CATUInt32 color;		
};


/// \class CAT3DPoint
/// \brief 3D point, with color and size
/// \ingroup CAT
class CATC3DPoint
{
	public:
		CATC3DPoint( const CATC3DPoint& point)
		{
			x		=	point.x;
			y		=	point.y;
			z		=	point.z;
			color =	point.color;
			size  =  point.size;			
		}

		CATC3DPoint( CATFloat64 xp, CATFloat64 yp, CATFloat64 zp, CATFloat64 sizep = 0, CATUInt32 colorp = 0)
		{
			x = xp;
			y = yp;
			z = zp;
			color.rgba = colorp;
			size = sizep;			
		}

		CATC3DPoint()
		{
			x		= 0.0;
			y		= 0.0;
			z		= 0.0;
			color.rgba = 0;
			size  = 0.0;			
		}

		virtual ~CATC3DPoint() 
		{
		
		}
	
		CATC3DPoint& FromScannedPolar( CATFloat64 yp, CATFloat64 zp, CATFloat64 theta_degrees, CATUInt32 colorp)
		{	
			// color remains unchanged
			color.rgba = colorp;

			// our y axis will remain unchanged (y does not change as the stage rotates)
			y = yp;
			
			// x is 0 for each scan - the theta changes
			//CATFloat64 x1 = 0.0;

			// Our data is all set with 6.0 as the center... should probably fix that elsewhere, but for now...			
			// z1 is our calulated position on the stage
			CATFloat64 z1 = zp - 6.0;	

			// convert degrees to rads 
			CATFloat64 thetaRads = theta_degrees*kCATDEGTORAD;

			// Now rotate around y axis
			x = z1*sin(thetaRads);
			z = z1*cos(thetaRads);
			// size doesn't change
			return *this;
		}
		
		// Right now, arithmetics don't affect color.
		CATC3DPoint operator+(const CATC3DPoint& npoint)
		{
			CATC3DPoint point(*this);
			point.x += npoint.x;
			point.y += npoint.y;
			point.z += npoint.z;
			// size doesn't change
			return point;
		}

		CATC3DPoint operator-(const CATC3DPoint& npoint)
		{
			CATC3DPoint point(*this);
			point.x -= npoint.x;
			point.y -= npoint.y;
			point.z -= npoint.z;
			// size doesn't change
			return point;
		}

		CATC3DPoint& operator=(const CATC3DPoint& npoint)
		{
			x = npoint.x;
			y = npoint.y;
			z = npoint.z;
			color = npoint.color;
			size = npoint.size;
			return *this;
		}

		CATC3DPoint operator*(CATFloat64 scaler)
		{
			CATC3DPoint point(*this);
			point.x *= scaler;
			point.y *= scaler;
			point.z *= scaler;
			// size doesn't change
			return point;
		}

		CATC3DPoint operator/(CATFloat64 scaler)
		{
			CATC3DPoint point(*this);
			point.x /= scaler;
			point.y /= scaler;
			point.z /= scaler;
			// size doesn't change
			return point;
		}

		CATC3DPoint operator*(const CATC3DPoint& npoint)
		{
			CATC3DPoint point(*this);
			point.x *= npoint.x;
			point.y *= npoint.y;
			point.z *= npoint.z;
			// size doesn't change
			return point;
		}
		
	public:
		CATFloat64 x;
		CATFloat64 y;
		CATFloat64 z;
		CATFloat64 size;
		CATColor   color;		
};
#endif // CATPOINT_H