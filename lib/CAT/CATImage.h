/// \file    CATImage.h
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
//
#ifndef _CATImage_H_
#define _CATImage_H_

#include "CATInternal.h"
#include "CATStream.h"
#include "CATColor.h"
#include "CATRect.h"
#include "png.h"

/// \class CATImage
/// \brief Base image class
/// \ingroup CAT
///
/// All images are 32-bit R,G,B,A. This interface provides the basics
/// needed for working with the images. 
///
/// Note: Alpha channels are stored where 255 means opaque, and
///       0 is totally translucent.  Make sure to set this when
///       loading / building images.
///
class CATImage
{
   public:                             
      /// Eventually, this may specify file format.
      ///
      /// Currently, we only support PNG with RGB/Alpha channels.
      enum CATIMAGEFORMAT
      {
         CATIMAGE_PNG_RGBA32
      };

      /// Load() loads an image from a file.
      ///
      /// Currently, only .PNG is supported. Images are converted
      /// into 32-bit RGBA (8-bits per channel)
      ///
      /// Call CATImage::ReleaseImage() when done with the returned image.
      ///
      /// \param stream - an opened stream to load the image from.
      ///                 The stream should be positioned at the 
      ///                 start of the image.
      ///
      /// \param image - image ref set to image from stream on success.
      /// \return CATResult - CAT_SUCCESS on success.
      static CATResult Load             (  CATStream*         stream,
                                           CATImage*&         image);
      
      /// Save() saves an image to a .png file.
      ///
      /// Currently, only .PNG format is supported. Images are
      /// saved as 32-bit RGBA (8-bits per channel).
      ///
      /// \param stream - an opened stream to save the image to.
      ///                 The stream should be positioned to the location
      ///                 to save the image to, and it must be writeable.
      ///
      /// \param image - ptr to image to save.
      /// \param imageFormat - right now, doesn't do anything. Should 
      ///                      always be CATIMAGE_PNG_RGBA32.
      ///
      /// \return CATResult - CAT_SUCCESS on success.      
      static CATResult Save(  CATStream*         stream,
                              CATImage*          image,
                              CATIMAGEFORMAT     imageFormat = CATIMAGE_PNG_RGBA32);

      static CATResult Save(  const CATWChar*    filename,
                              CATImage*			 image);

      /// CreateImage() creates an image.
      ///
      /// Use this instead of new to create image objects.
      /// If width and height are non-zero, creates the appropriate memory for
      /// the image and adds a reference. Otherwise, does not create memory 
      /// buffer or add reference.
      ///
      /// Call CATImage::ReleaseImage() on the returned image when done.
      ///
      /// \param image - uninitialized image ptr. Set on return.
      /// \param width - desired width of image in pixels.
      /// \param height - desired height of image in pixels.
      /// \param init - if true, image data is set to 0.
      /// \param transparent - if false, sets all alpha channels 
      ///                      to 255 (opaque). Otherwise, sets the
      ///                      alpha channels to 0 (transparent).  
      ///                      Ignored if init == false.    
      ///
      /// \return CATResult result code 
      /// \sa CATImage::ReleaseImage()
      static CATResult CreateImage      (  CATImage*&      image,
                                           CATInt32        width,
                                           CATInt32        height,
                                           bool            init = true,
                                           bool            transparent = true);
#ifdef CAT_CONFIG_WIN32
      /// Create an image from a DIB section (Win32 only)
      static CATResult CreateImageFromDIB(	CATImage*&		image,
                                            HBITMAP         dibSection);

      /// Create an image from an Icon (win32?)  
      static CATResult CreateImageFromIcon(	CATImage*&		image,
                                            CATInt32        width,
                                            CATInt32        height,
                                            CATColor        bgColor,
                                            CATICON         iconHandle);
#endif

      /// ReleaseImage decrements the reference count of an image and
      /// will free the image if it hits zero.
      /// It may also free parent images if the specified
      /// image holds the last reference to a parent.
      ///
      /// \param image - pointer to image to release. Is set to NULL
      ///                if last reference was deleted.
      /// 
      /// \return CATResult result code 
      /// \sa CATImage::CreateImage()
      static CATResult ReleaseImage     (  CATImage*&         image );


      /// CreateSub creates a sub-image of the specified parent.
      ///
      /// dstImage should not be instantiated prior to calling CreateSub.
      /// Sub image is returned in dstImage.
      ///
      /// The sub image references the parent's fData
      /// member, and the parent image must not be deleted before you
      /// are done using the sub image.
      ///
      /// You may create a sub image of a sub image, ad infinum. It'll
      /// handle the offsets.  Call CATImage::ReleaseImage() when done.
      ///
      /// \param orgImg - parent image to derive subimage from.
      /// \param dstImg - uninitialized image ptr. Set on return.
      /// \param xOffset - relative x offset for top-left of sub image.
      /// \param yOffset - relative y offset for top-left of sub image.
      /// \param width - width of sub image.
      /// \param height - height of sub image.
      ///
      /// \return CATResult result code 
      /// \sa CATImage::ReleaseImage()
      /// \sa CATImage::SetSubPosition()
      static CATResult CreateSub       (  const CATImage*    orgImg,
                                          CATImage*&         dstImg,
                                          CATInt32               xOffset,
                                          CATInt32               yOffset,
                                          CATInt32               width,
                                          CATInt32               height);

      /// CopyImage() creates a new image and stores it in dstImg.  
      /// The data from srcImg is copied into a buffer owned by 
      /// dstImg. 
      ///
      /// Caller must call CATImage::ReleaseImage() on the returned 
      /// image when done.
      ///
      /// \param srcImg - source image to copy from
      /// \param dstImg - uninitizlied image ptr. Contains copy
      ///                 of srcImg on return.
      ///
      /// \return CATResult result code 
      /// \sa CATImage::ReleaseImage()
      static CATResult CopyImage        (  const CATImage*    srcImg,
                                           CATImage*&         dstImg);
      
      /// CopyImage() creates a new image of the same type as srcImg
      /// and stores it in dstImg.  The data from srcImg is
      /// copied into a buffer owned by dstImg. 
      ///
      /// Caller must call CATImage::ReleaseImage() on the returned 
      /// image when done.
      ///
      /// This version allows you to specify offsets, width, and
      /// height of the data to copy into a new image.
      /// 
      /// \param srcImg - source image to copy from
      /// \param dstImg - uninitialized image ptr. Contains copy
      ///                 of srcImg on return.
      /// \param xOffset - relative x offset for top-left of copy.
      /// \param yOffset - relative y offset for top-left of copy.
      /// \param width - width of destination image.
      /// \param height - height of destination image.
      ///
      /// \return CATResult result code 
      /// \sa CATImage::ReleaseImage()
      static CATResult CopyImage       (  const CATImage*    srcImg,
                                          CATImage*&         dstImg,
                                          CATInt32               xOffset,
                                          CATInt32               yOffset,
                                          CATInt32               width,
                                          CATInt32               height);

      /// CopyOver() copies from another image over the current image
      /// at the specified offsets for the specified width and height.
      ///
      /// If no width/height parameters are given, the entire width
      /// and/or height are used.
      ///
      /// Clipping is not performed. However, if the copy would go out
      /// of bounds, an error will be returned and the copy will not
      /// be performed.
      ///
      /// \param srcImg - source image to copy from
      /// \param dstOffsetX - x offset in dest image to copy to
      /// \param dstOffsetY - y offset in dest image to copy to
      /// \param srcOffsetX - x offset in src image to copy from
      /// \param srcOffsetY - y offset in src image to copy from
      /// \param width    - width to copy in
      /// \param height   - height to copy in
      /// \return CATResult - CAT_SUCCESS on success.
      CATResult  CopyOver (   const CATImage*    srcImg,
                              CATInt32               dstOffsetX,
                              CATInt32               dstOffsetY,
                              CATInt32               srcOffsetX,
                              CATInt32               srcOffsetY,
                              CATInt32               width  = 0,
                              CATInt32               height = 0);

      /// CopyOutBGR() copies the RGB data from a rectangle
      /// into a buffer and flips the Red and Blue channels.  
      /// The buffer must be width*height*3
      /// bytes in size.
      ///
      /// \param rgbBuf - raw rgb buffer of width/height size
      /// \param offsetX - x offset to copy from
      /// \param offsetY - y offset to copy from
      /// \param width    - width to copy 
      /// \param height   - height to copy 
      /// \param widthBytes - width of line in bytes (w/pad)
      /// \return CATResult - CAT_SUCCESS on success.
      CATResult  CopyOutBGR ( CATUInt8*              rgbBuf,
                              CATInt32               offsetX,
                              CATInt32               offsetY,
                              CATInt32               width,
                              CATInt32               height,
                              CATInt32               widthBytes);

      /// Overlay() merges from another image over the current image
      /// at the specified offsets for the specified width and height.
      ///
      /// If no width/height parameters are given, the entire width
      /// and/or height are used.
      ///
      /// Clipping is not performed. However, if the copy would go out
      /// of bounds, an error will be returned and the copy will not
      /// be performed.
      ///
      /// The alpha channel is used for the merge operation from the
      /// source image only.  The destination's alpha remains 
      /// unaffected - the alpha is used for merging the colors only.
      ///
      /// \param srcImg - source image to copy from
      /// \param dstOffsetX  - offset in dest image to copy to
      /// \param dstOffsetY  - offset in dest image to copy to
      /// \param srcOffsetX  - offset in src image to copy from
      /// \param srcOffsetY  - offset in src image to copy from
      /// \param width    - width to copy in
      /// \param height   - height to copy in
      /// \return CATResult - CAT_SUCCESS on success.
      CATResult Overlay(   const CATImage*    srcImg,
                           CATInt32               dstOffsetX,
                           CATInt32               dstOffsetY,
                           CATInt32               srcOffsetX,
                           CATInt32               srcOffsetY,
                           CATInt32               width  = 0,
                           CATInt32               height = 0);
      

      /// GetPixel() retrieves the red, green, blue, and alpha values for a 
      /// specified pixel.
      ///
      /// This is for convenience and prototyping - for high-speed image
      /// processing you'll need to work more directly with the image
      /// buffer.
      ///
      /// \param x - x position within the image of the pixel
      /// \param y - y position within the image of the pixel
      /// \param r - receives the red value of the pixel
      /// \param g - receives the green value of the pixel
      /// \param b - receives the blue value of the pixel
      /// \param a - receives the alpha values of the pixel
      ///
      /// \return CATResult result code.  CAT_SUCCESS on success.
      /// \sa SetPixel()
      CATResult GetPixel(CATInt32              x,
                         CATInt32              y,
                         unsigned char&   r,
                         unsigned char&   g,
                         unsigned char&   b,
                         unsigned char&   a)  const;

      /// GetPixel() retrieves the red, green, blue, and alpha values for a 
      /// specified pixel.
      ///
      /// This is for convenience and prototyping - for high-speed image
      /// processing you'll need to work more directly with the image
      /// buffer.
      ///
      /// \param x - x position within the image of the pixel
      /// \param y - y position within the image of the pixel
      /// \param color - receives the color of the pixel
      ///
      /// \return CATResult result code.  CAT_SUCCESS on success.
      /// \sa SetPixel()
      CATResult GetPixel( CATInt32              x,
                          CATInt32              y,
                          CATColor&         color)  const;
 
      /// SetPixel() sets the red, green, blue, and alpha values for a 
      /// specified pixel.
      ///
      /// This is for convenience and prototyping - for high-speed image
      /// processing you'll need to work more directly with the image
      /// buffer.
      ///
      /// \param x - x position within the image of the pixel
      /// \param y - y position within the image of the pixel
      /// \param r - red value of the pixel
      /// \param g - green value of the pixel
      /// \param b - blue value of the pixel
      /// \param a - alpha value of the pixel (255 = opaque, 0 = transparent)
      ///
      /// \return CATResult result code.  CAT_SUCCESS on success.
      /// \sa GetPixel()
      CATResult SetPixel       (  CATInt32            x,
                                  CATInt32            y,
                                  unsigned char  r,
                                  unsigned char  g,
                                  unsigned char  b,
                                  unsigned char  a = 255);
                                 
      /// SetPixel() sets the red, green, blue, and alpha values for a 
      /// specified pixel.
      ///
      /// This is for convenience and prototyping - for high-speed image
      /// processing you'll need to work more directly with the image
      /// buffer.
      ///
      /// \param x - x position within the image of the pixel
      /// \param y - y position within the image of the pixel
      /// \param color - color to set pixel to
      ///
      /// \return CATResult result code.  CAT_SUCCESS on success.
      /// \sa GetPixel()
      CATResult SetPixel       (   CATInt32            x,
                                   CATInt32            y,
                                   const CATColor& color);

      /// Clear() sets all the pixels in the image to 0.
      ///
      /// \param transparent - if false, sets all alpha channels 
      ///                      to 255 (opaque). Otherwise, sets the
      ///                      alpha channels to 0 (transparent).      
      ///
      /// \return CATResult result code.
      CATResult Clear(bool transparent = true);

      /// FillRect() fills an area to a specific color.
      ///
      /// \param rect  - rectangle to fill
      /// \param color - color to fill to
      /// \return CATResult - CAT_SUCCESS on success.      
      CATResult FillRect(const CATRect& rect, const CATColor& color);

      /// MakeDisabled() converts the image to a low-contrast greyscale.
      ///
      CATResult MakeDisabled();

      /// SetSubPosition() moves the ROI of the image within its parent.
      /// Will return CAT_ERR_IMAGE_OPERATION_INVALID_ON_ROOT if you try
      /// to use this on a root image instead of a sub image.
      ///
      /// \param newXOffset - relative x offset of new pos within parent.
      /// \param newYOffset - relative y offset of new pos within parent.
      /// \param newWidth - new width of sub image.
      /// \param newHeight - new height of sub image.
      ///
      /// \return CATResult result code.      
      /// \sa CATImage::CreateSub()
      CATResult SetSubPosition (  CATInt32      newXOffset,
                                 CATInt32      newYOffset,
                                 CATInt32      newWidth,
                                 CATInt32      newHeight );

      /// GetRawDataPtr() retrieves the image data ptr.
      ///
      /// Remember that this may be a sub image, in which case the data ptr 
      /// returned is to the raw buffer of a parent image. 
      ///
      /// So, any time you access the raw data you should use the
      /// XOffsetAbs, YOffsetAbs, AbsWidth, and AbsHeight functions
      /// when calculating your pointers into the data.
      ///
      /// Also remember that if you modify a sub image, you'll be modifying
      /// the parent image and any other overlapping sub images.  If you
      /// want to muck with an image without messing up any others, use
      /// CATImage::CopyImage() first to get a base image that owns its own
      /// data.
      ///
      /// Note that the xOffset and yOffset are relative to parent. They will
      /// be calculated at runtime if you call this function.      
      ///
      /// \return unsigned char* to image data
      unsigned char* GetRawDataPtr() const;
      
      /// Returns relative offset within parent image.
      /// \return CATInt32 - X Offset relative to parent.
      CATInt32   XOffsetRel  () const;

      /// Returns relative offset within parent image.
      /// \return CATInt32 - Y Offset relative to parent.
      CATInt32   YOffsetRel  () const;
   
      /// Returns absolute X offset within fData.
      /// \return CATInt32 - Absolute x offset of current image within root
      CATInt32   XOffsetAbs  () const;

      /// Returns absolute Y offset within fData.
      /// \return CATInt32 - Absolute y offset of current image within root
      CATInt32   YOffsetAbs  () const;

      /// Returns the absolute width of the fData image buffer.
      /// \return CATInt32 - Absolute width of root image
      CATInt32   AbsWidth    () const;

      /// Returns the absolute height of the fData image buffer.
      /// \return CATInt32 - Absolute height of root image
      CATInt32   AbsHeight   () const;

      /// Returns width of image in pixels.
      /// \return CATInt32 - Width of image
      CATInt32   Width       () const;

      /// Returns height of image in pixels.
      /// \return CATInt32 - Height of image
      CATInt32   Height      () const;

      /// Returns size of image buffer in bytes.
      /// \return CATInt32 - Size of image in bytes
      CATInt32   Size        () const;

      /// Returns absolute size of parent image.
      /// \return CATInt32 - Absolute size of root image in bytes.
      CATInt32   AbsSize     () const;

      /// IsImageRoot() returns true if the image is a root (parent)
      /// image that owns its own data.  If the image is a sub image,
      /// it returns false.
      ///
      /// \return bool - true if the image is a root (without a parent),
      ///                or fase otherwise.
      bool  IsImageRoot () const;

      /// AddRef increments our reference count and calls
      /// AddRef on our parent if one exists.
      /// \return long - current reference count
      unsigned long  AddRef();

      /// DecRef decrements our reference count and calls
      /// ReleaseImage() on our parent if one exists. 
      /// \return long - current reference count
      unsigned long  DecRef();      
      
      /// GetRefCount() retrieves the current reference
      /// count
      /// return unsigned long - current reference count
      /// \sa AddRef(), DecRef()
      unsigned long GetRefCount();

      // Debugging func for saving image to disk as a .png
      CATResult DbgSave     (  const CATWChar*  filename );

   protected:
      /// Constructor for CATImage - use CATImage::CreateImage() or similar 
      /// static functions instead!
      ///
      CATImage  ();
      
      /// Destructor for CATImage - use CATImage::ReleaseImage()
      virtual ~CATImage();

      /// operator= is overridden to generate an error if someone
      /// attempts to use it - Don't use it!
      /// Use CreateCopy() or CreateSub() instead depending on whether you
      /// want a deep or shallow copy of the object.            
      CATImage& operator=(const CATImage& img);

      /// Create - internal image creation function.
      /// Creates the buffer and sets it up, then calls AddRef().
      ///
      /// \param width - width of image in pixels.
      /// \param height - height of image in pixels.
      /// \param init - if true, initializes image pixels to 0.
      /// \param transparent - if false, sets all alpha channels 
      ///                      to 255 (opaque). Otherwise, sets the
      ///                      alpha channels to 0 (transparent).      
      ///                      Ignored if init == false.
      ///
      /// \return CATResult result code.
      /// \sa ::CATResult_CORE_ENUM, ::CATResult_IMAGE_ENUM
      CATResult    Create   (  CATInt32            width, 
                              CATInt32            height, 
                              bool           init,
                              bool           transparent);

      //---------------------------------------------------------------------
      // PNG callbacks

      /// PNGRead() reads from the current stream into the buffer
      /// provided by libpng.
      ///
      /// You shouldn't need to call this directly - these are called
      /// by libpng only.
      ///
      /// \param png_ptr - structure for the .png library. The CATStream*
      ///                  for the current stream is in png_ptr->io_ptr.
      /// \param data    - target buffer
      /// \param length  - number of bytes to read.
      /// \return none. Throws a CATRESULT on error.
      /// \sa Load()
      static void PNGRead(    png_structp       png_ptr,
                              png_bytep         data, 
                              png_size_t        length);


      /// PNGWrite() writes to the current stream from the buffer
      /// provided by libpng.
      ///
      /// You shouldn't need to call this directly - these are called
      /// by libpng only.
      ///
      /// \param png_ptr - structure for the .png library. The CATStream*
      ///                  for the current stream is in png_ptr->io_ptr.
      /// \param data    - source buffer
      /// \param length  - number of bytes to write.
      /// \return none. Throws a CATRESULT on error.
      /// \sa Save()
      static void PNGWrite(   png_structp       png_ptr,
                              png_bytep         data, 
                              png_size_t        length);


      /// PNGFlush() flushes the current stream for libpng.
      ///
      /// You shouldn't need to call this directly - these are called
      /// by libpng only.
      ///
      /// \param png_ptr - structure for the .png library. The CATStream*
      ///                  for the current stream is in png_ptr->io_ptr.
      /// \return none. Throws a CATRESULT on error.
      /// \sa Load(), Save()
      static void PNGFlush(   png_structp       png_ptr);
      
      /// PNGError() is called by libpng when a fatal error occurs.
      ///
      /// This just throws a CATResult when called.
      ///
      /// \param png_ptr - structure for .png lib.
      /// \param error_msg - ASCIIZ message reporting the error.
      static void PNGError(   png_structp       png_ptr,
                              png_const_charp   error_msg);

      /// PNGWarning() is called by libpng when a recoverable error occurs.
      ///
      /// This just throws a CATResult when called.
      ///
      /// \param png_ptr - structure for .png lib.
      /// \param warning_msg - ASCIIZ message reporting the warning.
      static void PNGWarning( png_structp       png_ptr,
                              png_const_charp   warning_msg);




   //---------------------------------------------------------------------
   private:
      
      // Basic image information
      CATInt32         fWidth;          ///< Width in pixels.      
      
      CATInt32         fHeight;         ///< Height in pixels.      
      
      CATUInt8*        fData;           ///< Image data.
                                        ///< This may be a pointer to the parent
                                        ///< image's buffer. Remember to use
                                        ///< offsets. 
      // SubImage/ROI information
      CATInt32         fXOffset;        ///< X Offset within parent image.
                                        ///< To find offset within fData, 
                                        ///< call XOffsetAbs().      

      CATInt32         fYOffset;        ///< Y Offset within parent image.
                                        ///< To find offset within fData,
                                        ///< call YOffsetAbs().      

      bool             fOwnData;        ///< Set to true if we own image data.
                                        ///< (i.e. are we the parent?)      
   
      CATUInt32        fRefCount;       ///< Ref count - inc for each sub image.      

      CATImage*        fParentImage;    ///< Parent image - so we can decrement
                                        ///<       ref count on destruction.      
};

#endif // _CATImage_H_
