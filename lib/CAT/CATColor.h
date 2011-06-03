/// \file CATColor.h
/// \brief Color struct/functions for GUI
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------
#ifndef CATCOLOR_H_
#define CATCOLOR_H_

#include "CATTypes.h"
/// \brief   Color structure. Alpha channel - 255 opaque, 0 transparent.
/// \ingroup CAT
struct CATCOLOR
{
   union
   {      
      CATUInt32  rgba;    ///< rgba color - stored in *big endian* format.
      struct
      {
         CATUInt8 r;      ///< red   channel
         CATUInt8 g;      ///< green channel
         CATUInt8 b;      ///< blue  channel
         CATUInt8 a;      ///< alpha channel
      };
   };
};

/// \class CATColor
/// \brief Color wrapper class
/// \ingroup CAT
///
/// Alpha channel - 255 is opaque, 0 is transparent.
class CATColor : public CATCOLOR
{
   public:
      /// Basic constructor initializes to opaque black.
      CATColor()
      {
         // Default to opaque black.
         r = g = b = 0;
         a = 255;
      }
      
      /// Construct to build a color by providing red, green, blue, and optionally alpha.
      ///
      /// All values are 0-255.  255 alpha is opaque, 0 is transparent.
      CATColor( CATUInt8 red, 
                CATUInt8 green, 
                CATUInt8 blue,
                CATUInt8 alpha = 255)
      {
         r = red;
         g = green;
         b = blue;
         a = alpha;
      }

      /// Color destructor
      ~CATColor()  
      {}

      /// Intensity() returns the grey level of the color
      ///
      /// \return CATUInt8 - intensity / greylevel of color
      inline CATUInt8 Intensity() const
      {
         return ((CATUInt8)( 0.3f*(r) + 0.59f*(g) + 0.11f*(b)));
      }   
};


#endif // CATCOLOR_H_


