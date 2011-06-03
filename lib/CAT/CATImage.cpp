/// \file    CATImage.cpp
/// \brief   PNG image class
/// \ingroup CAT
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include <memory.h>
#include "png.h"
#include "CATImage.h"
#include "CATStreamFile.h"

const CATInt32 kBytesPerPixel = 4;

//------------------------------------------------------------------------
// CreateImage creates an image.
//
// Use this instead of new to create image objects.
// If width and height are non-zero, creates the appropriate memory for
// the image and adds a reference. Otherwise, does not create memory 
// buffer or add reference.
//
// Call CATImage::ReleaseImage() on the returned image when done.
//
//    image - uninitialized image ptr. Set on return.
//    width - desired width of image in pixels.
//    height - desired height of image in pixels.
//    init - if true, image data is set to 0.
//    transparent - if false, sets all alpha channels 
//                      to 255 (opaque). Otherwise, sets the
//                      alpha channels to 0 (transparent).      
//                      Ignored if init == false.
//------------------------------------------------------------------------
CATResult CATImage::CreateImage(   CATImage*&      image,
                                 CATInt32            width,
                                 CATInt32            height,
                                 bool           init,
                                 bool           transparent)
{
   image = 0;

   try
   {
      image = new CATImage();
   }
   catch(...)
   {
      image = 0;
   }

   if (image == 0)
   {
      return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
   }

   if ((width != 0) && (height != 0))
   {
      return image->Create(width, height, init, transparent);
   }
   
   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// ReleaseImage decrements the reference count of an image and
// will free the image if it hits zero.
// It may also free parent images if the specified
// image holds the last reference to a parent.
//
// image - pointer to image to release. Is set to NULL
//         if last reference was deleted.
//---------------------------------------------------------------------------
CATResult CATImage::ReleaseImage( CATImage*& image)
{
   CATASSERT(image != 0, "Invalid image released!");

   if (image != 0)
   {     
      // DecRef also decrements parent(s)
      if (image->fRefCount <= 1)
      {
         // Delete if reference count hit zero.
         delete image;

         // Null caller's pointer since it's invalid now.
         image = 0;
      }
      else
      {
         // If we have other references, just decrement our count.
         image->DecRef();
      }
      
      return CAT_SUCCESS;
   }
   return CATRESULT(CAT_ERR_IMAGE_NULL);
   
}



//---------------------------------------------------------------------------
// CreateSub creates a sub-image of the specified parent.
//
// dstImage should not be instantiated prior to calling CreateSub.
// Sub image is returned in dstImage.
//
// The sub image references the parent's fData
// member, and the parent image must not be deleted before you
// are done using the sub image.
//
// You may create a sub image of a sub image, ad infinum. It'll
// handle the offsets.  Call CATImage::ReleaseImage() when done.
//
//        orgImg - parent image to derive subimage from.
//        dstImg - uninitialized image ptr. Set on return.
//        xOffset - relative x offset for top-left of sub image.
//        yOffset - relative y offset for top-left of sub image.
//        width - width of sub image.
//        height - height of sub image.
//---------------------------------------------------------------------------
CATResult CATImage::CreateSub      (  const CATImage*    orgImg,
                                 CATImage*&         dstImg,
                                 CATInt32               xOffset,
                                 CATInt32               yOffset,
                                 CATInt32               width,
                                 CATInt32               height)
{
   dstImg = 0;
   
   CATASSERT(xOffset >= 0, "XOffset must be >= 0");
   CATASSERT(yOffset >= 0, "YOffset must be >= 0");
   
   CATASSERT(xOffset + width  <= orgImg->fWidth,  
            "Invalid sub image width");
   
   CATASSERT(yOffset + height <= orgImg->fHeight, 
            "Invalid sub image height");

   CATASSERT(orgImg->fData != 0, "Parent image is invalid.");

   // Bail on invalid parameters (same checks as asserts above)
   if ((xOffset < 0) || (yOffset < 0)        ||
       (xOffset + width  > orgImg->fWidth)      ||
       (yOffset + height > orgImg->fHeight)     ||
       (orgImg->fData == 0))
   {
      return CATRESULT(CAT_ERR_IMAGE_INVALID_SUB_POSITION);
   }

   // Create an uninitialized image object
   CATResult result;
   if (CATFAILED(result = 
         CreateImage(dstImg, 0, 0, false)))
   {
      return result;
   }

   
   // Point it at parent and store options
   dstImg->fOwnData     = false;
   dstImg->fParentImage = (CATImage*)orgImg;
   dstImg->fData        = orgImg->fData;

   // Remember these are just width and height of the sub image. To retrieve
   // the width and height of the fData data buffer, use AbsWidth() and
   // AbsHeight().
   dstImg->fWidth       = width;
   dstImg->fHeight      = height;

   // Remember on the x and y offsets that these are
   // relative to the parent only - to get the absolute x and y offsets
   // within the fData data buffer, use XOffsetAbs() and YOffsetAbs().
   dstImg->fXOffset     = xOffset;
   dstImg->fYOffset     = yOffset;
   
   // Add reference to ourself and our parents.  CreateImage() did not do this
   // previously, since width and height were set to 0.
   dstImg->AddRef();

   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// CopyImage() creates a new image of the same type as srcImg
// and stores it in dstImg.  The data from srcImg is
// copied into a buffer owned by dstImg. Caller must call
// CATImage::ReleaseImage() on the returned image when done.
//
//       srcImg - source image to copy from
//       dstImg - uninitizlied image ptr. Contains copy
//                 of srcImg on return.
//---------------------------------------------------------------------------
CATResult CATImage::CopyImage      (  const CATImage*    srcImg,
                                 CATImage*&         dstImg)
{
   dstImg = 0;
   CATASSERT(srcImg != 0, "Can't copy a null image.");
   if (srcImg == 0)
   {
      return CATRESULT(CAT_ERR_IMAGE_NULL);
   }

   // Simply copy - make full copy of source 
   return CopyImage( srcImg, 
                     dstImg, 
                     srcImg->XOffsetRel(), 
                     srcImg->YOffsetRel(),
                     srcImg->Width(),
                     srcImg->Height());
}
//---------------------------------------------------------------------------
// CopyImage() creates a new image of the same type as srcImg
// and stores it in dstImg.  The data from srcImg is
// copied into a buffer owned by dstImg. Caller must call
// CATImage::ReleaseImage() on the returned image when done.
//
// This version allows you to specify offsets, width, and
// height of the data to copy into a new image.
// 
//        srcImg - source image to copy from
//        dstImg - uninitialized image ptr. Contains copy
//                 of srcImg on return.
//        xOffset - relative x offset for top-left of copy.
//        yOffset - relative y offset for top-left of copy.
//        width - width of destination image.
//        height - height of destination image.
//---------------------------------------------------------------------------
CATResult CATImage::CopyImage      (  const CATImage*    srcImg,
                                 CATImage*&         dstImg,
                                 CATInt32               xOffset,
                                 CATInt32               yOffset,
                                 CATInt32               width,
                                 CATInt32               height)
{
   dstImg = 0;
   CATASSERT(srcImg != 0, "Can't copy a null image.");
   if (srcImg == 0)
   {
      return CATRESULT(CAT_ERR_IMAGE_NULL);
   }
   
   CATASSERT(xOffset >= 0, "XOffset must be >= 0");
   CATASSERT(yOffset >= 0, "YOffset must be >= 0");
   CATASSERT(xOffset + width  <= srcImg->fWidth,  "Invalid offset/width");
   CATASSERT(yOffset + height <= srcImg->fHeight, "Invalid offset/height");
   CATASSERT(srcImg->fData != 0, "Source image is invalid.");

   // Bail on invalid parameters (same checks as asserts above)
   if ((xOffset < 0) || (yOffset < 0)        ||
       (xOffset + width  > srcImg->fWidth)      ||
       (yOffset + height > srcImg->fHeight)     ||
       (srcImg->fData == 0))
   {
      return CATRESULT(CAT_ERR_IMAGE_INVALID_SUB_POSITION);
   }

   // Create an image object and allocate space for memory.
   CATResult result;
   if (CATFAILED(result = CreateImage(  dstImg, 
                                       width, 
                                       height, 
                                       false)))
   {
      return result;
   }

   // Copy image buffer...
   CATInt32 y;
   
   // Offset into line for source buffer in bytes
   CATInt32 srcLineOffset = 
         (srcImg->XOffsetAbs() + xOffset) * kBytesPerPixel;
   
   // current position of start of buffer in source
   unsigned char* srcPtr = 
            srcImg->fData + srcLineOffset + 
            ((srcImg->YOffsetAbs() + yOffset) * srcImg->AbsWidth() * kBytesPerPixel);

   unsigned char* dstPtr = dstImg->fData;

   for (y = 0; y < height; y++)
   {
      // Copy one row at a time into our new image
      memcpy(  dstPtr,
               srcPtr,
               width * kBytesPerPixel );

      // step to next line in source
      srcPtr += srcImg->AbsWidth() * kBytesPerPixel;
      
      // and in destination....
      dstPtr += width * kBytesPerPixel;
   }

   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// Increment reference count
//---------------------------------------------------------------------------
unsigned long  CATImage::AddRef()
{
   // Increment parent reference count
   if (fParentImage != 0)
   {
      fParentImage->AddRef();
   }

   fRefCount++;
   return fRefCount;
}

//---------------------------------------------------------------------------
// Decrement reference count
//---------------------------------------------------------------------------
unsigned long  CATImage::DecRef()
{
   if (fParentImage != 0)
   {
      // If parent is non-null, release our ref count for it.
      // This may delete it.
      ReleaseImage(fParentImage);
      
      // If parent was deleted, then our ref count should be 1    
      if (fParentImage == 0)
      {
         // Make sure that if we deleted parent, we're going to delete here
         CATASSERT(fRefCount == 1, "Deleted parent, but we're not done here!");
      }
   }

   CATASSERT(fRefCount != 0, "Decrementing reference count too far!");
   if (fRefCount == 0)
      return fRefCount;

   fRefCount--;
   return fRefCount;
}

//---------------------------------------------------------------------------
// Protected constructor
//---------------------------------------------------------------------------
CATImage::CATImage()
{
   fOwnData       = false;
   fData          = 0;
   fWidth         = 0;
   fHeight        = 0;   
   fXOffset       = 0;
   fYOffset       = 0;
   fParentImage   = 0;
   fRefCount      = 0;
}

//---------------------------------------------------------------------------
// Protected destructor
//---------------------------------------------------------------------------
CATImage::~CATImage()
{
   // Dec and check our reference count - make sure we're not in here when
   // we shouldn't be.
   if (fData)
      DecRef();

   CATASSERT(fRefCount == 0, "Destructor called while active!");

   if (fOwnData)
   {
      if (fData != 0)
      {
         delete [] fData;
         fData = 0;
      }
   }
}


//---------------------------------------------------------------------------
// Create
//    Creates the buffer for the image. If init is true,
//    the data is initialized to 0. Otherwise, it's whatever
//    happened to be there.
//---------------------------------------------------------------------------
CATResult CATImage::Create(CATInt32 width, CATInt32 height, bool init, bool transparent)
{
   if ( (width == 0) || (height == 0) )
   {
      return CATRESULT(CAT_ERR_IMAGE_INVALID_SIZE);
   }

	if (fData != 0)
	{
		delete [] fData;
	}
   
   fData = new unsigned char[width * height * kBytesPerPixel];
   if (fData == 0)
   {
      return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
   }
   fOwnData = true;

   fWidth   = width;
   fHeight  = height;

   if (init)
      this->Clear(transparent);

   this->AddRef();
   
   return CAT_SUCCESS;
}

//---------------------------------------------------------------------------
// Clear
//    Clears the image data to 0
//---------------------------------------------------------------------------
CATResult CATImage::Clear(bool transparent)
{
   CATASSERT(fData != 0, "Can't clear an image unless it's been created.");
   if (fData == 0)
   {
      return CATRESULT(CAT_ERR_IMAGE_MUST_INITIALIZE);
   }

   // Offset into line for source buffer in bytes
   CATInt32 dstOffX = this->XOffsetAbs() * kBytesPerPixel;   
   // Absolute length of line in fData in bytes ( >= fWidth * 3)
   CATInt32 dstLineLength = this->AbsWidth()   * kBytesPerPixel;   
   // Distance from end of one line to beginning of the next in bytes.
   CATInt32 dstStep   = dstLineLength - (fWidth * kBytesPerPixel);

   // current position of start of buffer in source
   unsigned char* dstPtr = fData + (YOffsetAbs() * dstLineLength);
 
   unsigned char alpha = (transparent?0:255);

   // Endian-neutral way to set it up
   *(CATUInt32 *)dstPtr = 0;
   dstPtr[3] = alpha;

   // Get the val we'll copy in for clear into a reg/var
   CATUInt32 val = *(CATUInt32*)dstPtr;

   for (CATInt32 y = 0; y < fHeight; y++)
   {      
      for (CATInt32 x = 0; x < fWidth; x++)
      {
         *(CATUInt32*)dstPtr = val;
         dstPtr += kBytesPerPixel;         
      }

      dstPtr += dstStep;   
   }

   return CAT_SUCCESS;
}


//---------------------------------------------------------------------------
// FillRect() fills an area to a specific color.  If the alpha channel of
// the color is != 255, then fills w/alpha (e.g. blends color w/bg)
//
// \param rect  - rectangle to fill
// \param color - color to fill to
// \return CATResult - CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult CATImage::FillRect(const CATRect& rect, const CATColor& color)
{
   CATResult result = CAT_SUCCESS;
   CATRect   imageRect(0,0,fWidth,fHeight);

   CATASSERT(color.a != 0, "Uhm... filling with 0 alpha doesn't do much.");

   if (!imageRect.Inside(rect))
   {
      CATASSERT(false,"Rect must be contained by the image.");
      return CAT_ERR_IMAGE_FILL_OUT_OF_BOUNDS;
   }

   CATASSERT(fData != 0, "Image must be created first!");
   if (fData == 0)
   {
      return CATRESULT(CAT_ERR_IMAGE_MUST_INITIALIZE);
   }

   // Now fill the image in on all four channels.
   CATInt32 x,y;

   // Offset into line for source buffer in bytes
   CATInt32 dstOffX = (this->XOffsetAbs() + rect.left) * kBytesPerPixel;   
   
   // Absolute length of line in fData in bytes ( >= fWidth * 3)
   CATInt32 dstLineLength = this->AbsWidth() * kBytesPerPixel;   
   
   // Distance from end of one line to beginning of the next in bytes.
   CATInt32 dstStep   = dstLineLength - (rect.Width() * kBytesPerPixel);
   
   // current position of start of buffer in source
   unsigned char* dstPtr = fData + 
                           dstOffX + 
                           ((YOffsetAbs() + rect.top) * dstLineLength);


   CATASSERT(kBytesPerPixel == 4, "Woops.");

   // If we're not totally opaque, then alpha blend fill color
   if (color.a != 255)
   {
      // Loop through image performing fill
      for (y = 0; y < rect.Height(); y++)
      {      
         for (x = 0; x < rect.Width(); x++)
         {            
            // Blend the r,g,b channels. Alpha remains the same for destination image.         
            dstPtr[0] =  (CATUInt8)(((CATInt32)(color.r   * color.a) +
                                  (CATInt32)(dstPtr[0] * (255 - color.a))) / 255);

            dstPtr[1] =  (CATUInt8)(((CATInt32)(color.g   * color.a) +
                                  (CATInt32)(dstPtr[1] * (255 - color.a))) / 255);

            dstPtr[2] =  (CATUInt8)(((CATInt32)(color.b   * color.a) +
                                  (CATInt32)(dstPtr[2] * (255 - color.a))) / 255);

            dstPtr+=4;
         }         
         dstPtr += dstStep;
      }

   }
   else
   {
      // Totally opaque - just overwrite.

      // Loop through image performing merge
      for (y = 0; y < rect.Height(); y++)
      {      
         for (x = 0; x < rect.Width(); x++)
         {         
            dstPtr[0] = color.r;
            dstPtr[1] = color.g;
            dstPtr[2] = color.b;
            
            dstPtr += 4;
         }

         dstPtr += dstStep;
      }
   }
   return result;
}


//---------------------------------------------------------------------------
// XOffsetRel
//    Returns relative x offset within parent image.
//---------------------------------------------------------------------------
CATInt32 CATImage::XOffsetRel () const
{
   return fXOffset;
}

//---------------------------------------------------------------------------
// YOffsetRel
//    Returns relative y offset within parent image.
//---------------------------------------------------------------------------
CATInt32 CATImage::YOffsetRel () const
{
   return fYOffset;
}

//---------------------------------------------------------------------------
// XOffsetAbs
//    Returns absolute X offset within fData
//---------------------------------------------------------------------------
CATInt32 CATImage::XOffsetAbs () const
{
   CATInt32 absOffset = fXOffset;
   
   // Recursively add all parent offsets into ours to find real offset
   // within fData.
   if (fParentImage != 0)
   {
      absOffset += fParentImage->XOffsetAbs();
   }
   
   return absOffset;
}

//---------------------------------------------------------------------------
// YOffsetAbs
//    Returns absolute Y offset within fData
//---------------------------------------------------------------------------
CATInt32 CATImage::YOffsetAbs () const
{
   CATInt32 absOffset = fYOffset;

   // Recursively add all parent offsets into ours to find real offset
   // within fData.
   if (fParentImage != 0)
   {
      absOffset += fParentImage->YOffsetAbs();
   }
   
   return absOffset;
}
//---------------------------------------------------------------------------
// AbsWidth
//    Returns the absolute width of the fData image buffer
//---------------------------------------------------------------------------
CATInt32 CATImage::AbsWidth   () const
{
   CATInt32 absWidth = fWidth;
   if (fParentImage != 0)
   {
      absWidth = fParentImage->AbsWidth();
   }

   return absWidth;
}

//---------------------------------------------------------------------------
// AbsHeight
//    Returns the absolute height of the fData image buffer
//---------------------------------------------------------------------------
CATInt32 CATImage::AbsHeight  () const
{
   CATInt32 absHeight = fHeight;
   if (fParentImage != 0)
   {
      absHeight = fParentImage->AbsHeight();
   }

   return absHeight;
}

//---------------------------------------------------------------------------
// Width()
//    Retrieve the width of the image in pixels
//---------------------------------------------------------------------------
CATInt32 CATImage::Width() const
{
   return fWidth;
}

//---------------------------------------------------------------------------
// Height()
//    Retrieve the height of the image in pixels
//---------------------------------------------------------------------------
CATInt32 CATImage::Height() const
{
   return fHeight;
}

//---------------------------------------------------------------------------
// Size()
//    Retrieve the size of the image in bytes
//---------------------------------------------------------------------------
CATInt32 CATImage::Size() const
{
   return fWidth * fHeight * kBytesPerPixel;
}

//---------------------------------------------------------------------------
// AbsSize()
//    Retrieve the absolute size of the root image in bytes
//---------------------------------------------------------------------------
CATInt32 CATImage::AbsSize() const
{
   return AbsWidth() * AbsHeight() * kBytesPerPixel;
}

//---------------------------------------------------------------------------
// IsImageRoot()
//    Checks if an image is the root image. Returns true if so.
//    Otherwise, it is a sub image of another image.
//---------------------------------------------------------------------------
bool CATImage::IsImageRoot() const
{
   bool isRoot = fOwnData;
   
   CATASSERT(isRoot == (fParentImage == 0), 
      "Root image should not have a parent image, sub images must.");

   return isRoot;
}

//---------------------------------------------------------------------------
// GetRawDataPtr()
//    Retrieve the image data ptr
//
//    Remember that this may be a sub image, in which case the data ptr 
//    returned is to the raw buffer of a parent image.
//
//    So, any time you access the raw data you should use the
//    XOffsetAbs, YOffsetAbs, AbsWidth, and AbsHeight functions
//    when calculating your pointers into the data.
//
//    Also remember that if you modify a sub image, you'll be modifying
//    the parent image and any other overlapping sub images.  If you
//    want to muck with an image without messing up any others, use
//    CATImage::CopyImage() first to get a base image that owns its own
//    data.
//
//    Note that the xOffset and yOffset are relative to parent. They will
//    be calculated at runtime if you call this function.      
//---------------------------------------------------------------------------
unsigned char* CATImage::GetRawDataPtr() const
{
   return fData;
}


//---------------------------------------------------------------------------
// Because of the factory style setup, we aren't going to support
// copy operators. So it's protected, and just asserts.
// Use CreateCopy() or CreateSub() instead depending on whether you
// want a deep or shallow copy of the object.
//---------------------------------------------------------------------------
CATImage& CATImage::operator=(const CATImage& img)
{
   CATASSERT(false,"operator= not supported for CATImage.");
   return *this;
}

//---------------------------------------------------------------------------
// SetSubPosition
//    Move the ROI of the image within its parent.
//    Will return CAT_ERR_IMAGE_OPERATION_INVALID_ON_ROOT if you try
//    to use this on a root image instead of a sub image.
//---------------------------------------------------------------------------
CATResult CATImage::SetSubPosition (  CATInt32      newXOffset,
                                 CATInt32      newYOffset,
                                 CATInt32      newWidth,
                                 CATInt32      newHeight )
{
   
   // Cannot set offsets or modify width/height on the root image.
   CATASSERT(IsImageRoot() == false, 
      "Cannot SetSubPosition on a root image.");   
   
   if (IsImageRoot())
   {     
      return CATRESULT(CAT_ERR_IMAGE_OPERATION_INVALID_ON_ROOT);
   }

   // Perform sanity checks on values.
   CATASSERT(newXOffset >= 0, "XOffset must be >= 0");
   CATASSERT(newYOffset >= 0, "YOffset must be >= 0");
   
   CATASSERT(newXOffset + newWidth  <= fParentImage->fWidth,  
            "Invalid sub image width");
   
   CATASSERT(newYOffset + newHeight <= fParentImage->fHeight, 
            "Invalid sub image height");

   CATASSERT(fParentImage->fData != 0, "Parent image is invalid.");

   // Bail on invalid parameters (same checks as asserts above)
   if ((newXOffset < 0) || (newYOffset < 0)        ||
       (newXOffset + newWidth  > fParentImage->fWidth)      ||
       (newYOffset + newHeight > fParentImage->fHeight)     ||
       (fParentImage->fData == 0))
   {
      return CATRESULT(CAT_ERR_IMAGE_INVALID_SUB_POSITION);
   }

   // Okay, position is valid. Make the change.
   fXOffset = newXOffset;
   fYOffset = newYOffset;
   fWidth   = newWidth;
   fHeight  = newHeight;

   return   CAT_SUCCESS;
}


//---------------------------------------------------------------------------
// GetPixel() retrieves the red, green, blue, and alpha values for a 
// specified pixel.
//
// This is for convenience and prototyping - for high-speed image
// processing you'll need to work more directly with the image
// buffer.
//
//
// \param x - x position within the image of the pixel
// \param y - y position within the image of the pixel
// \param r - receives the red value of the pixel
// \param g - receives the green value of the pixel
// \param b - receives the blue value of the pixel
// \param a - receives the alpha value of the pixel
//
// \return CATResult result code.  CAT_SUCCESS on success.
// \sa SetPixel()
//---------------------------------------------------------------------------

CATResult CATImage::GetPixel(   CATInt32              x,
                              CATInt32              y,
                              CATColor&         color) const
{
   return GetPixel(x, y, color.r, color.g, color.b, color.a);
}

CATResult CATImage::GetPixel(   CATInt32              x,
                              CATInt32              y,
                              unsigned char&   r,
                              unsigned char&   g,
                              unsigned char&   b,
                              unsigned char&   a) const
{
   CATResult result = CAT_SUCCESS;
   
   CATASSERT(fData != 0, "Image must be created first!");
   if (fData == 0)
   {
      return CATRESULT(CAT_ERR_IMAGE_EMPTY);
   }

   // Bounds check coordinates
   CATASSERT(( (x >= 0) || (x < fWidth)), "X position is out of bounds!");
   CATASSERT(( (y >= 0) || (y < fHeight)), "Y position is out of bounds!");

   if ( (x < 0) || (x >= fWidth))
   {
      return CATRESULT(CAT_ERR_IMAGE_OUT_OF_RANGE);
   }

   if ( (y < 0) || (y >= fHeight))
   {
      return CATRESULT(CAT_ERR_IMAGE_OUT_OF_RANGE);
   }
      
   
   // Offset of pixel on x axis in image data
   CATInt32 lineOffset = (this->XOffsetAbs() + x) * kBytesPerPixel;

   // Absolute length of line in fData in bytes ( >= fWidth * GetBytesPerPixel())
   CATInt32 lineLength = this->AbsWidth() * kBytesPerPixel;
   
   // current position of start of buffer in source
   unsigned char* curPtr = fData + lineOffset + 
                           ((YOffsetAbs() + y) * lineLength);

   // pull out pixels
   r = curPtr[0];
   g = curPtr[1];
   b = curPtr[2];
   a = curPtr[3];
         
   return result;
}
//---------------------------------------------------------------------------
// SetPixel() sets the red, green, blue, and alpha pixel values
// for a pixel
//
// This is for convenience and prototyping - for high-speed image
// processing you'll need to work more directly with the image
// buffer.
//
// \param x - x position within the image of the pixel
// \param y - y position within the image of the pixel
// \param r - red value of the pixel
// \param g - green value of the pixel
// \param b - blue value of the pixel
// \param a - alpha value of the pixel
//
// \return CATResult result code.  CAT_SUCCESS on success.
// \sa GetPixel()
//---------------------------------------------------------------------------
CATResult CATImage::SetPixel(   CATInt32              x,
                              CATInt32              y,
                              const CATColor&   color)
{
   return SetPixel(x, y, color.r, color.g, color.b, color.a);
}

CATResult CATImage::SetPixel (        CATInt32               x,
                                    CATInt32               y,
                                    unsigned char     r,
                                    unsigned char     g,
                                    unsigned char     b,
                                    unsigned char     a)
{
   CATResult result = CAT_SUCCESS;
   
   CATASSERT(fData != 0, "Image must be created first!");
   if (fData == 0)
   {
      return CATRESULT(CAT_ERR_IMAGE_EMPTY);
   }

   // Bounds check coordinates
   CATASSERT(( (x >= 0) || (x < fWidth)), "X position is out of bounds!");
   CATASSERT(( (y >= 0) || (y < fHeight)), "Y position is out of bounds!");

   if ( (x < 0) || (x >= fWidth))
   {
      return CATRESULT(CAT_ERR_IMAGE_OUT_OF_RANGE);
   }

   if ( (y < 0) || (y >= fHeight))
   {
      return CATRESULT(CAT_ERR_IMAGE_OUT_OF_RANGE);
   }
   
   // Offset of pixel on x axis in image data
   CATInt32 lineOffset = (this->XOffsetAbs() + x) * kBytesPerPixel;

   // Absolute length of line in fData in bytes ( >= fWidth * GetBytesPerPixel())
   CATInt32 lineLength = this->AbsWidth() * kBytesPerPixel;
   
   // current position of start of buffer in source
   unsigned char* curPtr = fData + lineOffset + 
                           ((YOffsetAbs() + y) * lineLength);
   
   // Store
   curPtr[0] = r;
   curPtr[1] = g;
   curPtr[2] = b;
   curPtr[3] = a;
         
   return result;
}

//---------------------------------------------------------------------------
// CopyOver() copies from another image over the current image
// at the specified offsets for the specified width and height.
//
// If no width/height parameters are given (or are 0), the entire 
// width and/or height of the source image is used.
//
// Clipping is not performed. However, if the copy would go out
// of bounds, an error will be returned and the copy will not
// be performed.
//
// \param srcImage - source image to copy from
// \param dstOffsetX - x offset in dest image to copy to
// \param dstOffsetY - y offset in dest image to copy to
// \param srcOffsetX - x offset in src image to copy from
// \param srcOffsetY - y offset in src image to copy from
// \param width    - width to copy in
// \param height   - height to copy in
// \return CATResult - CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult  CATImage::CopyOver (    const CATImage*    srcImg,
                                 CATInt32               dstOffsetX,
                                 CATInt32               dstOffsetY,
                                 CATInt32               srcOffsetX,
                                 CATInt32               srcOffsetY,
                                 CATInt32               width,
                                 CATInt32               height)
{
   CATResult result = CAT_SUCCESS;
   CATASSERT(srcImg != 0, "Null source image.");
   
   // Bail if image is null
   if (srcImg == 0)
   {
      return CATRESULT(CAT_ERR_IMAGE_NULL);
   }

   // If the width/height are 0, set them to source size
   if (width == 0)
   {
      width = srcImg->Width();
   }  
   
   if (height == 0)
   {
      height = srcImg->Height();
   } 

   CATASSERT(width != 0, "Width still null in image copy");
   CATASSERT(height != 0, "Height still null in image copy");
   if ((width == 0) || (height == 0))
   {
      return CATRESULT(CAT_ERR_IMAGE_NULL);
   }

   CATASSERT(width + dstOffsetX <= fWidth, "Out of bounds in image copy");
   CATASSERT(height + dstOffsetY <= fHeight, "Out of bounds in image copy");
   if ( (width + dstOffsetX > fWidth) ||
         (height + dstOffsetY > fHeight))
   {
      return CATRESULT(CAT_ERR_IMAGE_OVERLAY_OUT_OF_BOUNDS);
   }

   // Now copy the image in on all four channels.
   CATInt32 y;
	// CATInt32 x
   CATASSERT(fData != 0, "Image must be created first!");
   if (fData == 0)
   {
      return CATRESULT(CAT_ERR_IMAGE_MUST_INITIALIZE);
   }


   // Offset into line for source buffer in bytes
   CATInt32 dstOffX = (this->XOffsetAbs() + dstOffsetX) * kBytesPerPixel;   
   
   // Absolute length of line in fData in bytes ( >= fWidth * 3)
   CATInt32 dstLineLength = this->AbsWidth()   * kBytesPerPixel;   
   
   // Distance from end of one line to beginning of the next in bytes.
   CATInt32 dstStep   = dstLineLength - (width * kBytesPerPixel);
   
   // current position of start of buffer in source
   unsigned char* dstPtr = fData + 
                           dstOffX + 
                           ((YOffsetAbs() + dstOffsetY) * dstLineLength);


   // as above for source
   CATInt32 srcOffX           = (srcImg->XOffsetAbs() + srcOffsetX) * kBytesPerPixel;   
   CATInt32 srcLineLength     = srcImg->AbsWidth()   * kBytesPerPixel;   
   CATInt32 srcStep           = srcLineLength - (width * kBytesPerPixel);
   unsigned char* srcPtr = srcImg->fData + 
                           srcOffX + 
                           ((srcImg->YOffsetAbs() + srcOffsetY) * srcLineLength);

   bool alphaUnused = true;

   CATASSERT(kBytesPerPixel == 4, "Woops.");

   // Loop through image performing merge
   
	CATUInt32 lineWidth = width*4;

	for (y = 0; y < height; y++)
   {     
		memcpy(dstPtr, srcPtr,lineWidth);
      dstPtr += dstStep + lineWidth;
      srcPtr += srcStep + lineWidth;
   }
   return result;
}

//---------------------------------------------------------------------------
// Overlay() merges from another image over the current image
// at the specified offsets for the specified width and height.
//
// If no width/height parameters are given (or are 0), the entire 
// width and/or height of the source image is used.
//
// Clipping is not performed. However, if the copy would go out
// of bounds, an error will be returned and the copy will not
// be performed.
//
// The alpha channel is used for the merge operation from the
// source image only.  The destination's alpha remains 
// unaffected - the alpha is used for merging the colors only.
//
// \param srcImage - source image to copy from
// \param xOffset  - offset in dest image to copy to
// \param yOffset  - offset in dest image to copy to
// \param width    - width to copy in
// \param height   - height to copy in
// \return CATResult - CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult  CATImage::Overlay(   const CATImage*    srcImg,
                              CATInt32               dstOffsetX,
                              CATInt32               dstOffsetY,
                              CATInt32               srcOffsetX,
                              CATInt32               srcOffsetY,
                              CATInt32               width,
                              CATInt32               height)
{
   CATResult result = CAT_SUCCESS;
   CATASSERT(srcImg != 0, "Null source image.");
   
   // Bail if image is null
   if (srcImg == 0)
   {
      return CATRESULT(CAT_ERR_IMAGE_NULL);
   }

   // If the width/height are 0, set them to source size
   if (width == 0)
   {
      width = srcImg->Width();
   }
   
   if (height == 0)
   {
      height = srcImg->Height();
   } 

   CATASSERT(width != 0, "Width still null in image copy");
   CATASSERT(height != 0, "Height still null in image copy");
   if ((width == 0) || (height == 0))
   {
      return CATRESULT(CAT_ERR_IMAGE_NULL);
   }

   CATASSERT(width + dstOffsetX <= fWidth, "Out of bounds in image copy");
   CATASSERT(height + dstOffsetY <= fHeight, "Out of bounds in image copy");
   if ( (width + dstOffsetX > fWidth) ||
         (height + dstOffsetY > fHeight))
   {
      return CATRESULT(CAT_ERR_IMAGE_OVERLAY_OUT_OF_BOUNDS);
   }

   if ( (width + dstOffsetX > fWidth) ||
         (height + dstOffsetY > fHeight))
   {
      return CATRESULT(CAT_ERR_IMAGE_OVERLAY_OUT_OF_BOUNDS);
   }

   CATASSERT(width + srcOffsetX <= srcImg->Width(), "Out of bounds in image copy");
   CATASSERT(height + srcOffsetY <= srcImg->Height(), "Out of bounds in image copy");
   if (width + srcOffsetX > srcImg->Width())
   {
      width = srcImg->Width() - srcOffsetX;
   }
   if (height + srcOffsetY > srcImg->Height())
   {
      height = srcImg->Height() - srcOffsetY;
   }


   // Now copy the image in on all four channels.
   CATInt32 x,y;
   CATASSERT(fData != 0, "Image must be created first!");
   if (fData == 0)
   {
      return CATRESULT(CAT_ERR_IMAGE_MUST_INITIALIZE);
   }


   // Offset into line for source buffer in bytes
   CATInt32 dstOffX = (this->XOffsetAbs() + dstOffsetX) * kBytesPerPixel;   
   
   // Absolute length of line in fData in bytes ( >= fWidth * 3)
   CATInt32 dstLineLength = this->AbsWidth()   * kBytesPerPixel;   
   
   // Distance from end of one line to beginning of the next in bytes.
   CATInt32 dstStep   = dstLineLength - (width * kBytesPerPixel);
   
   // current position of start of buffer in source
   unsigned char* dstPtr = fData + 
                           dstOffX + 
                           ((YOffsetAbs() + dstOffsetY) * dstLineLength);


   // as above for source
   CATInt32 srcOffX           = (srcImg->XOffsetAbs() + srcOffsetX) * 
                           kBytesPerPixel;   

   CATInt32 srcLineLength     = srcImg->AbsWidth()   * kBytesPerPixel;   
   CATInt32 srcStep           = srcLineLength - (width * kBytesPerPixel);
   unsigned char* srcPtr = srcImg->fData + 
                           srcOffX + 
                           ((srcImg->YOffsetAbs() + 
                              srcOffsetY) * srcLineLength);

   // Loop through image performing merge.
   // If this turns out to be a bottleneck, 
   // convert to SIMD on win32 and equiv on Mac.
   for (y = 0; y < height; y++)
   {      
      for (x = 0; x < width; x++)
      {       
         CATInt32 i;  
         unsigned char alpha = srcPtr[3];

         // skip multiplication steps for totally transparent or opaque alpha.
         if (alpha == 0)
         {
            srcPtr +=4;
            dstPtr +=4;
            continue;
         }
         else if (alpha == 255)
         {
            for (i=0; i<3; i++)
            {
               *dstPtr = *srcPtr;
               dstPtr++;
               srcPtr++;
            }
            dstPtr ++;
            srcPtr ++;
            continue;
         }
         
         // Blend the r,g,b channels. Alpha remains the same for destination image.         
         for (i=0; i < 3; i++)
         {
				*dstPtr = (CATUInt8)((( alpha * (CATInt32)( *srcPtr - *dstPtr) ) + ((CATInt32)(*dstPtr)<<8) ) >> 8);
/*
            *dstPtr =  (CATUInt8)((( (CATInt32)(*srcPtr) * alpha)  + 
                        ( (CATInt32)(*dstPtr) * (255 - alpha) )) / 255);
*/
            dstPtr++;
            srcPtr++;
         }
         
         srcPtr++;
         dstPtr++;
      }

      dstPtr += dstStep;
      srcPtr += srcStep;
   }

	


   return result;
}

//---------------------------------------------------------------------------
/// Load() loads an image from a file.
///
/// Currently, only .PNG is supported.
///
/// Call CATImage::ReleaseImage() when done with the returned image.
///
/// \param stream - an opened stream to load the image from.
///                 The stream should be positioned at the 
///                 start of the image.
///
/// \param image - image ref set to image from stream on success.
/// \return CATResult - CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult CATImage::Load           (  CATStream*         stream,
                                    CATImage*&         image)
{   
   image = 0;

   CATResult result = CAT_SUCCESS;

   CATASSERT(stream != 0, "Stream must be created and opened first!");
   CATASSERT(stream->IsOpen(), "Stream must be created and opened first!");
   if ((stream == 0) || (stream->IsOpen() == false))
   {
      return CATRESULT(CAT_ERR_INVALID_PARAM);
   }

   image = new CATImage();


   png_structp png_ptr;
   png_infop info_ptr;
   png_uint_32 width, height;  

   try
   {
      // Create the base PNG pointer and set the error handlers.
      png_ptr = png_create_read_struct(   PNG_LIBPNG_VER_STRING,
                                          image, 
                                          PNGError, 
                                          PNGWarning);

      if (png_ptr == NULL)
      {
         delete image;
         image = 0;
         return CATRESULT(CAT_ERR_PNG_ERROR_CREATING_READ);
      }

      // Create the info struct   
      info_ptr = png_create_info_struct(png_ptr);
   

      if (info_ptr == NULL)
      {
         png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);       
         delete image;
         image = 0;
         return CATRESULT(CAT_ERR_PNG_ERROR_CREATING_READ);
      }

      // Set read callback and pass stream* as user_io_ptr
      png_set_read_fn(png_ptr, (void *)stream, PNGRead);

      // Convert png to 32-bit RGBA regardless of what it is
      png_set_expand(png_ptr);
      png_set_palette_to_rgb(png_ptr);
      png_set_tRNS_to_alpha(png_ptr);
      png_set_gray_to_rgb(png_ptr);

      // Read in the .png file
      png_read_png(png_ptr, info_ptr,  PNG_TRANSFORM_SHIFT, png_voidp_NULL);


      // Get pointer to row data
      unsigned char** rows = png_get_rows(png_ptr, info_ptr);

      // Get width / height
      width = png_get_image_width(png_ptr, info_ptr);
      height = png_get_image_height(png_ptr, info_ptr);

      CATUInt32 y,x;

   
      // Since we're creating a new image here, no worries about offsets and padding -
      // the image data is contiguous. Just copy row by row into our image
      image->Create(width, height, true, false);
      unsigned char* rawData = image->GetRawDataPtr();

      CATInt32 channels = png_get_channels(png_ptr,info_ptr);

      if (channels == 4)
      {
         for (y=0; y < height; y++)
         {
            memcpy(  rawData + (y * width * kBytesPerPixel),
                     rows[y],
                     width * kBytesPerPixel);
         }
      }
      else if (channels == 3)
      {
         for (y=0; y < height; y++)
         {
            unsigned char* linePtr = rawData + (y * width * kBytesPerPixel);            
            unsigned char* srcPtr = rows[y];

            for (x = 0; x < width; x++)
            {
               *linePtr = *srcPtr; linePtr++; srcPtr++;
               *linePtr = *srcPtr; linePtr++; srcPtr++;
               *linePtr = *srcPtr; linePtr++; srcPtr++;
               *linePtr = 255;
               linePtr++;
            }
         }
      }
      else
      {
         CATASSERT(false,"Unsupported number of channels in .PNG!");
         result = CATRESULTFILE(CAT_ERR_PNG_UNSUPPORTED_FORMAT,stream->GetName());
      }
   
      // Clean up
      png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);        

      return result;
   }
   // Catch errors that were thrown from PNGError()
   catch (CATResult& thrownRes)
   {
      result = thrownRes;
      if (image != 0)
      {
         delete image;
         image = 0;
      }
      png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

      // Tack on filename to result if we're using result classes.      
      return CATRESULTFILE(thrownRes, stream->GetName());
   }

}

//---------------------------------------------------------------------------
/// Save() saves an image to a file.
///
/// Currently, only .PNG format is supported.
///
/// \param stream - an opened stream to save the image to.
///                 The stream should be positioned to the location
///                 to save the image to, and it must be writeable.
///
/// \param image - ptr to image to save.
/// \return CATResult - CAT_SUCCESS on success.
//---------------------------------------------------------------------------


CATResult CATImage::Save(  const CATWChar*	filename,
                           CATImage*        image)
{
	CATResult result = CAT_SUCCESS;
	CATStreamFile outFile;
	if (CATFAILED(result = outFile.Open(filename,CATStream::READ_WRITE_CREATE_TRUNC)))
	{
		return result;
	}

	result = Save(&outFile, image);

	outFile.Close();

	return result;
}

CATResult CATImage::Save             (  CATStream*         stream,
                                        CATImage*          image,
                                        CATIMAGEFORMAT     imageFormat)
{
   CATResult result = CAT_SUCCESS;
   unsigned char** row_pointers = 0;

   CATASSERT(imageFormat == CATIMAGE_PNG_RGBA32, "Only 32-bit RGBA .PNG's are supported.");
   if (imageFormat != CATIMAGE_PNG_RGBA32)
   {
      return CATRESULT(CAT_ERR_IMAGE_UNKNOWN_FORMAT);
   }

   CATASSERT(stream != 0, "Stream must be created and opened first!");
   CATASSERT(stream->IsOpen(), "Stream must be created and opened first!");
   if ((stream == 0) || (stream->IsOpen() == false))
   {
      return CATRESULT(CAT_ERR_INVALID_PARAM);
   }

   CATASSERT(image != 0, "Image must be valid for save.");   
   if ((image == 0) || (image->fData == 0))
   {
      return CATRESULT(CAT_ERR_INVALID_PARAM);
   }

   png_structp png_ptr;
   png_infop info_ptr;

   try
   {
      png_ptr = png_create_write_struct(  PNG_LIBPNG_VER_STRING,
                                          image, 
                                          PNGError,
                                          PNGWarning);

      if (png_ptr == NULL)
      {
         return CATRESULT(CAT_ERR_PNG_ERROR_CREATING_WRITE);
      }

      // Create infostruct
      info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr == NULL)
      {      
         png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
         return CATRESULT(CAT_ERR_PNG_ERROR_CREATING_WRITE);
      }

      // Set write callbacks
      png_set_write_fn( png_ptr, (void*)stream, 
                        PNGWrite,
                        PNGFlush);      

      
      try
      {
         row_pointers = new unsigned char*[image->fHeight];
      }
      catch (...)
      {
         row_pointers = 0;
         // Throw CATResult to function catch
         throw CATRESULT(CAT_ERR_OUT_OF_MEMORY);
      }

      for (CATInt32 y = 0; y < image->fHeight; y++)
      {
         row_pointers[y] = image->fData + (image->XOffsetAbs() * kBytesPerPixel) +
                           (( (y + image->YOffsetAbs()) * image->AbsWidth()) 
                              * kBytesPerPixel);
      }

      png_set_IHDR(  png_ptr,
                     info_ptr,
                     image->fWidth,
                     image->fHeight,
                     8,
                     PNG_COLOR_TYPE_RGB_ALPHA,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE,
                     PNG_FILTER_TYPE_BASE);

      png_set_rows(png_ptr,info_ptr,row_pointers);

      png_write_png(png_ptr, info_ptr, 0, png_voidp_NULL);

      delete [] row_pointers;
   }
   catch (CATResult& thrownRes)
   {            
      if (row_pointers)
      {
         delete [] row_pointers;
      }

      result = thrownRes;
      // Tack on filename to result if we're using result classes.
      result = CATRESULTFILE(result,stream->GetName());      
   }

   // Clean up
   png_destroy_write_struct(&png_ptr, &info_ptr);      

   return result;
}

//---------------------------------------------------------------------------
// Debugging func - saves image to disk as a .png
//---------------------------------------------------------------------------
CATResult CATImage::DbgSave( const CATWChar* filename)
{
   // Create file    
   CATResult result = CAT_SUCCESS;
   CATStreamFile file;
   if (CATFAILED(result = file.Open(filename, CATStream::READ_WRITE_CREATE_TRUNC)))
   {
      return result;
   }

   result = this->Save(&file,this);

   file.Close();
   
   return result;
}

//------------------------------------------------------------------------
// PNG utility functions
// These are mainly callbacks required by the PNG library.
//
// Functions which may call into these need to catch CATResult&'s.
//------------------------------------------------------------------------


//------------------------------------------------------------------------
// PNGRead() reads from the current stream into the buffer
// provided by libpng.
//
// You shouldn't need to call this directly - these are called
// by libpng only.
//
// \param png_ptr - structure for the .png library. The CATStream*
//                  for the current stream is in png_ptr->io_ptr.
// \param data    - target buffer
// \param length  - number of bytes to read.
// \return none. Throws a CATRESULT on error.
// \sa Load()
//---------------------------------------------------------------------------
void CATImage::PNGRead(     png_structp       png_ptr,
                           png_bytep         data, 
                           png_size_t        length)
{
   CATStream* userStream = (CATStream*)png_ptr->io_ptr;   
   CATUInt32 amountRead = (CATUInt32)length;
   CATResult result = userStream->Read(data,amountRead);
   if (CATFAILED(result))
   {
      throw(result);
   }
}

//------------------------------------------------------------------------
// PNGWrite() writes to the current stream from the buffer
// provided by libpng.
//
// You shouldn't need to call this directly - these are called
// by libpng only.
//
// \param png_ptr - structure for the .png library. The CATStream*
//                  for the current stream is in png_ptr->io_ptr.
// \param data    - source buffer
// \param length  - number of bytes to write.
// \return none. Throws a CATRESULT on error.
// \sa Save()
//---------------------------------------------------------------------------
void CATImage::PNGWrite(    png_structp       png_ptr,
                           png_bytep         data, 
                           png_size_t        length)
{
   CATStream* userStream = (CATStream*)png_ptr->io_ptr;   
   CATResult result = userStream->Write(data,(CATUInt32)length);
   if (CATFAILED(result))
   {
      throw(result);
   }
}

//------------------------------------------------------------------------
// PNGFlush() flushes the current stream for libpng.
//
// You shouldn't need to call this directly - these are called
// by libpng only.
//
// \param png_ptr - structure for the .png library. The CATStream*
//                  for the current stream is in png_ptr->io_ptr.
// \return none. Throws a CATRESULT on error.
// \sa Load(), Save()
//---------------------------------------------------------------------------
void CATImage::PNGFlush(    png_structp       png_ptr)
{
   CATStream* userStream = (CATStream*)png_ptr->io_ptr;      
}

//------------------------------------------------------------------------
// PNGError() is called by libpng when a fatal error occurs.
//
// This just throws a CATResult when called.
//
// \param png_ptr - structure for .png lib.
// \param error_msg - ASCIIZ message reporting the error.
//---------------------------------------------------------------------------
void CATImage::PNGError(    png_structp       png_ptr,
                  png_const_charp   error_msg)
{
   throw(CATRESULTDESC(CAT_ERR_PNG_CORRUPT,error_msg));
}


//------------------------------------------------------------------------
// PNGWarning() is called by libpng when a recoverable error occurs.
//
// This just throws a CATResult when called.
//
// \param png_ptr - structure for .png lib.
// \param warning_msg - ASCIIZ message reporting the warning.
//---------------------------------------------------------------------------
void CATImage::PNGWarning(  png_structp       png_ptr,
                  png_const_charp   warning_msg)
{
   throw(CATRESULTDESC(CAT_ERR_PNG_WARNING,warning_msg));
}

//---------------------------------------------------------------------------
// CopyOutBGR() copies the RGB data from a rectangle
// into a buffer and flips the Red and Blue channels.  
// The buffer must be width*height*3
// bytes in size.
//
// \param rgbBuf - raw rgb buffer of width/height size
// \param offsetX - x offset to copy from
// \param offsetY - y offset to copy from
// \param width    - width to copy 
// \param height   - height to copy 
// \param paddedWidth - padded width of buffer
// \return CATResult - CAT_SUCCESS on success.
//---------------------------------------------------------------------------
CATResult  CATImage::CopyOutBGR (  CATUInt8*              rgbBuf,
                                 CATInt32               offsetX,
                                 CATInt32               offsetY,
                                 CATInt32               width,
                                 CATInt32               height,
                                 CATInt32               widthBytes)
{
   CATASSERT(width != 0, "Width null");
   CATASSERT(height != 0, "Height null");
   CATASSERT(rgbBuf != 0, "Null buffer passed in.");
   if ((width == 0) || (height == 0) || (rgbBuf == 0))
   {
      return CATRESULT(CAT_ERR_IMAGE_OVERLAY_OUT_OF_BOUNDS);
   }

   CATASSERT(width + offsetX <= fWidth, "Out of bounds in image copy");
   CATASSERT(height + offsetY <= fHeight, "Out of bounds in image copy");
   if ( (width + offsetX > fWidth) ||
        (height + offsetY > fHeight))
   {
      return CATRESULT(CAT_ERR_IMAGE_OVERLAY_OUT_OF_BOUNDS);
   }

   // Now copy the image to the buffer
   CATInt32 x,y;
   CATASSERT(fData != 0, "Image must be created first!");
   if (fData == 0)
   {
      return CATRESULT(CAT_ERR_IMAGE_MUST_INITIALIZE);
   }


   // Offset into line for source buffer in bytes
   CATInt32 srcOffX = (this->XOffsetAbs() + offsetX) * kBytesPerPixel;   
   
   // Absolute length of line in fData in bytes ( >= fWidth * 3)
   CATInt32 srcLineLength = this->AbsWidth()   * kBytesPerPixel;   
   
   // Distance from end of one line to beginning of the next in bytes.
   CATInt32 srcStep   = srcLineLength - (width * kBytesPerPixel);
   
   // current position of start of buffer in source
   unsigned char* srcPtr = fData + 
                           srcOffX + 
                           ((YOffsetAbs() + offsetY) * srcLineLength);


   unsigned char* dstPtr = rgbBuf;
   CATInt32 dstStep = widthBytes - width*3;

	CATASSERT(kBytesPerPixel == 4, "Making assumptions on pixel size right now...");

   // Loop through image performing merge
   for (y = 0; y < height; y++)
   {      
		CATUInt32* srcDwPtr = (CATUInt32*)srcPtr;

      for (x = 0; x < width; x++)
      {
			CATUInt32 curPixel = *srcDwPtr;
			srcDwPtr++;
			*dstPtr = (CATUInt8)(curPixel >> 16);
			dstPtr++;
			*dstPtr = (CATUInt8)(curPixel >> 8);
			dstPtr++;
			*dstPtr = (CATUInt8)(curPixel);
			dstPtr++;
			
			
			/*
         dstPtr[0] = srcPtr[2];
         dstPtr[1] = srcPtr[1];
         dstPtr[2] = srcPtr[0];
         
         srcPtr += kBytesPerPixel;
         dstPtr += 3;
			*/


			
      }

      srcPtr += srcStep + width*4;
      dstPtr += dstStep;
   }
   
   return CAT_SUCCESS;
}

unsigned long CATImage::GetRefCount()
{
   return this->fRefCount;
}

#ifdef CAT_CONFIG_WIN32
CATResult CATImage::CreateImageFromDIB(	CATImage*&		image,
										HBITMAP			dibSection)
{
	CATResult result = CAT_SUCCESS;
   image = 0;

   try
   {
      image = new CATImage();
   }
   catch(...)
   {
      image = 0;
   }

   if (image == 0)
   {
      return CATRESULT(CAT_ERR_OUT_OF_MEMORY);
   }

	if ((dibSection == 0) || (dibSection == INVALID_HANDLE_VALUE))
	{
		return CATRESULT(CAT_ERR_INVALID_PARAM);
	}

	BITMAP bmpInfo;
	memset(&bmpInfo,0,sizeof(bmpInfo));
	::GetObject( dibSection, sizeof(BITMAP), &bmpInfo);

	CATInt32 width,height;
	width	= bmpInfo.bmWidth;
	height	= bmpInfo.bmHeight;

	bool upsideDown = (height > 0);
	if (!upsideDown)
	{
		height = -height;
	}

   if ((width != 0) && (height != 0))
   {
      result = image->Create(width, height, false, false);
   }

	if (CATFAILED(result))
	{
		return result;
	}

	unsigned char* rawPtr = image->GetRawDataPtr();

	CATASSERT((bmpInfo.bmBitsPixel == 24) ||(bmpInfo.bmBitsPixel == 32), 
            "Only 24-bit and 32-bit images are currently supported.");
	
	if ((bmpInfo.bmBits != 0) && (bmpInfo.bmBitsPixel == 24))
	{			
		unsigned char* dstPtr = 0;
		unsigned char* srcPtr = 0;

		for (CATInt32 y = 0; y < height; y++)
		{
			dstPtr = rawPtr + (y*image->AbsWidth()*4);
			srcPtr = (unsigned char*)bmpInfo.bmBits;
			
			if (upsideDown)
				srcPtr += ((height-1) - y) * bmpInfo.bmWidthBytes;
			else
				srcPtr += (y*bmpInfo.bmWidthBytes);


			for (CATInt32 x = 0; x < width; x++)
			{
				// BGR to RGB
				*dstPtr = srcPtr[2]; dstPtr++;				
				*dstPtr = srcPtr[1]; dstPtr++;				
				*dstPtr = srcPtr[0];	dstPtr++; 
								
				// alpha
				*dstPtr = 0xff;
				dstPtr++;	
				
				srcPtr += 3;
			}
		}
	}
	else if ((bmpInfo.bmBits != 0) && (bmpInfo.bmBitsPixel == 32))
	{			
		unsigned char* dstPtr = 0;
		unsigned char* srcPtr = 0;

		for (CATInt32 y = 0; y < height; y++)
		{
			dstPtr = rawPtr + (y*image->AbsWidth()*4);
			srcPtr = (unsigned char*)bmpInfo.bmBits;
			
			if (upsideDown)
				srcPtr += ((height-1) - y) * bmpInfo.bmWidthBytes;
			else
				srcPtr += (y*bmpInfo.bmWidthBytes);


			for (CATInt32 x = 0; x < width; x++)
			{
				// BGR to RGB
				*dstPtr = srcPtr[2]; dstPtr++;				
				*dstPtr = srcPtr[1]; dstPtr++;				
				*dstPtr = srcPtr[0]; dstPtr++; 
                
				// alpha
				*dstPtr = srcPtr[3];
				dstPtr++;	
				
				srcPtr += 4;
			}
		}
	}


	return result;
}

CATResult CATImage::CreateImageFromIcon( CATImage*&		image,
                                         CATInt32       width,
                                         CATInt32       height,
                                         CATColor       bgColor,
    									 CATICON		iconHandle)
{
   CATResult result = CAT_SUCCESS;
	if ((iconHandle == 0) || (iconHandle == INVALID_HANDLE_VALUE))
	{
		return CATRESULT(CAT_ERR_INVALID_PARAM);
	}
    
    int bytesPerPix = 4;
    HDC memDC = ::CreateCompatibleDC(NULL);
    BITMAPINFO bmpInfo;
    bmpInfo.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth           = width;
    bmpInfo.bmiHeader.biHeight          = height;
    bmpInfo.bmiHeader.biPlanes          = 1;
    bmpInfo.bmiHeader.biBitCount        = bytesPerPix*8;
    bmpInfo.bmiHeader.biCompression     = BI_RGB;
    bmpInfo.bmiHeader.biSizeImage       = 0;
    bmpInfo.bmiHeader.biXPelsPerMeter   = 0;
    bmpInfo.bmiHeader.biYPelsPerMeter   = 0;
    bmpInfo.bmiHeader.biClrUsed         = 0;
    bmpInfo.bmiHeader.biClrImportant    = 0;

    CATUInt8 *dataPtr = 0;
    HBITMAP dib   = ::CreateDIBSection(memDC,&bmpInfo,DIB_RGB_COLORS,(void**)&dataPtr,0,0);
    HGDIOBJ oldObj = ::SelectObject(memDC,dib);
    RECT rect;
    rect.left       = rect.top = 0;
    rect.right      = width;
    rect.bottom     = height;

    for (int i = 0; i < width*height; i++)
    {
        *(DWORD*)&dataPtr[i*4] = bgColor.rgba;
    }
    
    ::DrawIconEx(memDC,0,0,iconHandle,width,height,0,0,DI_NORMAL);
    ::SelectObject(memDC,oldObj);
    ::DeleteDC(memDC);

    result = CreateImage(image,width,height,true,true);
    if (!CATFAILED(result))
    {
		unsigned char* dstPtr = 0;
		unsigned char* srcPtr = 0;

		for (CATInt32 y = 0; y < height; y++)
		{
			dstPtr = image->GetRawDataPtr() + (y*image->AbsWidth()*4);
			srcPtr = dataPtr + ((height-1)*width*4) - (y*width*4);

			for (CATInt32 x = 0; x < width; x++)
			{
				// BGR to RGB
				*dstPtr = srcPtr[2]; dstPtr++;				
				*dstPtr = srcPtr[1]; dstPtr++;				
				*dstPtr = srcPtr[0]; dstPtr++; 
                
				// alpha
				*dstPtr = 0xff; //srcPtr[3];
				dstPtr++;	
				
				srcPtr += 4;
			}
		}
    }
    ::DeleteObject(dib);
	return result;
}
#endif

CATResult CATImage::MakeDisabled()
{
    CATInt32 width  = Width();
    CATInt32 height = Height();
    
    unsigned char* rawPtr  = GetRawDataPtr();
    unsigned char* destPtr = 0;

    for (CATInt32 y = 0; y < height; y++)
    {
	    destPtr = rawPtr + (y*AbsWidth()*4);
		
	    for (CATInt32 x = 0; x < width; x++)
	    {
		    // Greyscale
            CATFloat32 colorVal = destPtr[0]*0.3f + destPtr[1]*0.59f + destPtr[2]*0.11f;

            // lower contrast and brighten
            colorVal = colorVal/8 + 192;

            destPtr[0] = destPtr[1] = destPtr[2] = (CATUInt8)colorVal;

            destPtr += 4;
	    }
    }
    return CAT_SUCCESS;
}