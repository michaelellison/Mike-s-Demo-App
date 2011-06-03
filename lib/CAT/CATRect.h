//---------------------------------------------------------------------------
/// \file    CATRect.h
/// \brief   Rectangles, points, size, etc. for GUI
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef CATRect_H_
#define CATRect_H_

#pragma pack(push)
#pragma pack(1)
/// CATRECT defines the basic rectangle structure.
///
/// It should be analogous to other rect structs (e.g. RECT from win32).
struct CATRECT
{
   CATInt32   left;
   CATInt32   top;
   CATInt32   right;
   CATInt32   bottom;
};

/// CATPOINT is a simple 2D integer point
struct CATPOINT
{
   CATInt32   x;
   CATInt32   y;
};

/// CATSIZE is a 2D integer size (width = cx, height = cy)
struct CATSIZE
{
   CATInt32   cx;
   CATInt32   cy;
};
#pragma pack(pop)


/// \class CATRect
/// \brief Rectangles for GUI.
/// \ingroup CAT
///
/// Rectangles include their top,left position, but not their bottom,right
/// position.  (top,left) is inclusive, (bottom,right) is exclusive.
///
/// In other words, a rectangle of (0,0,1,1) has a width of 1, 
/// and a height of 1.  The point (0,0) is inside the rectangle, but the
/// point (1,1) is just outside the rectangle.
class CATRect : public CATRECT
{
   public:
      /// Default constructor - sets to all 0's.
      CATRect()
      {
         left = top = right = bottom = 0;
      }

      /// Copy constructor
      CATRect(const CATRECT& rect)
      {
         this->left     = rect.left;
         this->top      = rect.top;
         this->right    = rect.right;
         this->bottom   = rect.bottom;
      }

      /// Initializing constructor
      /// \param l - left position (inclusive)
      /// \param t - top position (inclusive)
      /// \param r - right position (exclusive)
      /// \param b - bottom position (exclusive)
      CATRect(CATInt32 l, CATInt32 t, CATInt32 r, CATInt32 b)
      {
         CATASSERT(l <= r, "Left should be <= right.");
         CATASSERT(t <= b, "Top should be <= bottom.");

         this->left     = l;
         this->top      = t;
         this->right    = r;
         this->bottom   = b;
      }
         
      /// Destructor
      ~CATRect()
      {
         CATASSERT(left <= right, "Left should be <= right.");
         CATASSERT(top <= bottom, "Top should be <= bottom.");
      }

      /// Operator= override for CATRECT structs
      CATRect& operator=(const CATRECT& rect)
      {
         this->left     = rect.left;
         this->top      = rect.top;
         this->right    = rect.right;
         this->bottom   = rect.bottom;
         return *this;
      }

      /// Operator== override for CATRECT structs
      bool operator==(const CATRECT& rect)
      {
         if ((left == rect.left) &&
             (top == rect.top) &&
             (right == rect.right) &&
             (bottom == rect.bottom))
         {
            return true;
         }
         return false;
      }

      /// Operator!= override for CATRECT structs
      bool operator!=(const CATRECT& rect)
      {
         if ((left == rect.left) &&
             (top == rect.top) &&
             (right == rect.right) &&
             (bottom == rect.bottom))
         {
            return false;
         }
         return true;
      }

      /// Width() returns the width of the rect
      /// \return CATInt32 - width (right - left).
      inline CATInt32 Width() const
      {
         return right - left;
      }

      /// Height() returns the height of the rect
      /// \return CATInt32 - heigt (bottom - top)
      inline CATInt32 Height() const
      {
         return bottom - top;
      }
   
      /// Size() returns a CATSIZE struct with the
      /// width and height.
      /// \return CATSIZE - width and height of rectangle      
      inline CATSIZE Size() const
      {
         CATSIZE rsize = {right - left, bottom - top};
         return rsize;
      }

      /// Origin() returns the current top,left of the
      /// rectangle.
      /// \return CATPOINT - top left of rectangle
      inline CATPOINT Origin() const
      {         
         CATPOINT point = {left,top};
         return point;         
      }

      /// CenterX() returns the center X position.
      /// If the width is odd, returns the leftmost edge of center 
      /// (e.g. rounds down)
      inline CATInt32 CenterX() const
      {
         return (left + right)/2;
      }
      
      /// CenterY() returns the center Y position.
      /// If the height is odd, returns the upper edge of center 
      /// (e.g. rounds down)
      inline CATInt32 CenterY() const
      {
         return (top + bottom)/2;
      }

      /// Center() returns the point in the center of the rectangle.
      /// Values are rounded down.
      inline CATPOINT Center() const
      {
         CATPOINT point = {CenterX(), CenterY() };
         return point;
      }

      /// ZeroOrigin() moves the top-left corner to the
      /// 0,0 origin.
      void ZeroOrigin()
      {
         right    -= left;
         bottom   -= top;
         left     = 0;
         top      = 0;         
      }

      /// SetOrigin() moves the top-left corner to the specified
      /// location
      inline void SetOrigin(CATPOINT& point)
      {
         SetOrigin(point.x, point.y);
      }

      void SetOrigin(CATInt32 x, CATInt32 y)
      {
         ZeroOrigin();
         left   += x;
         right  += x;
         top    += y;
         bottom += y;
      }


      /// Offset() offsets the rect by the x,y values
      /// in the point.
      ///
      /// \param point - contains the x,y values to be added to the rectangle
      void Offset(const CATPOINT& point)
      {
         Offset(point.x, point.y);
      }

      /// Offset() offsets the rect by the specified x,y.
      /// \param x - x offset to be added
      /// \param y - y offset to be added
      void Offset(CATInt32 x, CATInt32 y)
      {
         left     += x;
         right    += x;
         top      += y;
         bottom   += y;
      }

      /// Resize() resizes the rectangle to the specified size.
      /// Only the bottom right corner is moved.
      /// \param size - width and height of new rectangle
      void Resize(const CATSIZE& size)
      {
         right = left + size.cx;
         bottom = top + size.cy;
      }

      /// Resize() resizes the rectangle to the specified size.
      ///
      /// \param width - new width
      /// \param height - new height
      void Resize(CATInt32 width, CATInt32 height)
      {
         right = left + width;
         bottom = top + height;
      }

      /// Fix() fixes the rectangle if it's broken.
      /// Right now, just flips left/right and top/bottom
      /// if they are swapped.
      void Fix()
      {
         if (left > right)
            CATSwap(left, right);

         if (top > bottom)
            CATSwap(top, bottom);
      }

      /// Set() sets the rectangle to the specified coordinates
      /// \param l - new left position (inclusive)
      /// \param t - new top position (inclusive)
      /// \param r - new right position (exclusive)
      /// \param b - new bottom position (exclusive)
      void Set(CATInt32 l, CATInt32 t, CATInt32 r, CATInt32 b)
      {
         this->left = l;
         this->top = t;
         this->right = r;
         this->bottom = b;
      }

      /// Set() sets the rectangle to the specified coordinates
      /// \param topLeft - top,left position
      /// \param widthHeight -  width/height of rect
      void Set(const CATPOINT& topLeft, const CATSIZE& widthHeight)
      {
         this->left = topLeft.x;
         this->top  = topLeft.y;
         this->right  = topLeft.x + widthHeight.cx;
         this->bottom = topLeft.y + widthHeight.cy;
      }


      /// InRect() returns true if the point is inside the rectangle,
      /// and false if it is outside.
      ///
      /// A point is outside the rectangle if it is on the bottom right
      /// corner. e.g. (top,left) is inclusive, (bottom,right) is exclusive.
      /// 
      bool InRect(const CATPOINT& point) const
      {
         return InRect(point.x, point.y);
      }

      /// InRect() returns true if the point is inside the rectangle,
      /// and false if it is outside.
      ///
      /// A point is outside the rectangle if it is on the bottom right
      /// corner. e.g. (top,left) is inclusive, (bottom,right) is exclusive.
      /// 
      /// \param x - x position of point to check
      /// \param y - y position of point to check
      /// \return bool - true if point is inside rect, false otherwise
      bool InRect(CATInt32 x, CATInt32 y) const
      {
         if ( ((x >= left) && (x < right)) &&
              ((y >= top)  && (y < bottom)) )
         {
            return true;
         }
         return false;
      }

      /// Intersect() determines if two rectangles intersect, then 
      /// calculates the rectangle of intersection between two rectangles
      /// if any.
      ///
      /// \param srcRect - rectangle to check for intersection against
      ///        this one.
      /// \param intersectRect - ptr to rectangle to receive the
      ///        intersecting rectangle between the two.
      /// \return bool - true if they intersect, false otherwise.
      bool Intersect( const CATRect& srcRect, CATRect* intersectRect = 0) const
      {           
         // Skip if totally out 
         // Remember bottom/right are exclusive, hence >= / <= 
         if (  (this->right   <= srcRect.left) || 
               (this->left    >= srcRect.right) ||
               (this->bottom  <= srcRect.top) ||
               (this->top     >= srcRect.bottom) ||
               (this->Width() == 0) ||
               (this->Height() == 0) ||
               (srcRect.Width() == 0) ||
               (srcRect.Height() == 0) )
         {
            // Zero rect if present
            if (intersectRect != 0)
            {
               intersectRect->left = intersectRect->right  = 0;
               intersectRect->top  = intersectRect->bottom = 0;
            }

            return false;
         }

         // If we don't need to know intersect, then bail here         
         if (intersectRect == 0)
         {
            return true;
         }
         
         // Calc intersection between the two
         intersectRect->left   = CATMax(left,    srcRect.left);
         intersectRect->top    = CATMax(top,     srcRect.top);
         intersectRect->right  = CATMin(right,   srcRect.right);
         intersectRect->bottom = CATMin(bottom,  srcRect.bottom);
         return true;
      }
      
      /// Inside() determines if the inside rect is fully contained
      /// by *this rect.
      ///
      /// \param insideRect - Rect to check
      /// \return bool - true if insideRect is fully contained by *this.      
      bool Inside(const CATRect& insideRect) const
      {
         if (  ( this->left   <= insideRect.left   ) &&
               ( this->right  >= insideRect.right  ) &&
               ( this->top    <= insideRect.top    ) &&
               ( this->bottom >= insideRect.bottom ) )
         {
            return true;
         }
         return false;
      }

      /// Stretch() stretches the rectangle by the specified amount, 
      /// optionally keeping it in bounds of the bounds rectangle.
      void Stretch(CATInt32 stretchSize = 1, const CATRect* bounds = 0)
      {
            left   -= stretchSize;
            top    -= stretchSize;
            right  += stretchSize;
            bottom += stretchSize;

            if (bounds)
            {
                if ( left < bounds->left)
                    left = bounds->left;
                if ( top  < bounds->top)
                    top = bounds->top;
                if (right > bounds->right)
                    right = bounds->right;
                if (bottom > bounds->bottom)
                    right = bounds->bottom;
            }            
      }

      void Shrink(CATInt32 stretchSize = 1)
      {
            left   += stretchSize;
            top    += stretchSize;
            right  -= stretchSize;
            bottom -= stretchSize;
      }

};

#endif // CATRect_H_


