/// \file CBMagInfo.h
/// \brief Color Modification Information for CodeBlind.
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-06-02 02:47:03 -0500 (Thu, 02 Jun 2011) $
// $Revision: 7 $
// $NoKeywords: $

//----------------------------------------------------------------------
// Magnification and Color Modification Information for CodeBlind.
//----------------------------------------------------------------------
//
// This is the storage and processing class for the Magnification
// and Color Modification data for CodeBlind.  Ideally, you should
// be able to take this file, plop it into a project on just about
// any 32-bit platform, and be able to load CodeBlind's save files
// and process images in the same way.
//
//----------------------------------------------------------------------
//                                Technical Notes:
//----------------------------------------------------------------------
//
//   Magnification is not implemented in the processing.  In CodeBlind,
// it's a lot easier to use the magnification level as a 
// region of interest (ROI) selector to calculate the offsets and 
// procWidth/procHeight.  Then, I'm simply copying from that ROI after 
// processing it (via StretchBlt on win32).
//
// Parameter validation is performed on file loads / struct copies.  
// This may seem slow, but at least in the uses I expect the variables 
// don't change much - only the ProcessImage() function is worth 
// optimizing.  The robustness gains seem worthwhile to me.
//
// This file is tested under Win32 with Microsoft's Visual C++ 6.0.  
// I've tried to make it operable for other platforms, but those 
// functions have not been fully tested.
//----------------------------------------------------------------------
//
#ifndef _CBMAGINFO_H_
#define _CBMAGINFO_H_

#include <stdio.h>
#include "CATTypes.h"
//----------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------
#define   CBRGBTOGREY(r,g,b)   ((CATUInt8)( 0.3f*(r) + 0.59f*(g) + 0.11f*(b)))
// Buffer should be kCBMagMaxNameLength + 1 for null
const int kCBMagMaxNameLength  = 127;
const int kCBMagNameBufferSize = kCBMagMaxNameLength + 1;

// Current version of the structure
const int kCBMagVersion           = 1;
// CBMagInfo error / status codes
enum CBMAGRESULT
{
    // Success code
    CBMAG_SUCCESS = 0,

    // Status codes
    CBMAG_STAT_NEW_VERSION,                   // Newer version of struct detected.
    CBMAG_STAT_ENDIAN_FLIPPED,                // Endian was flipped on struct
    
    // Error codes
    CBMAG_ERROR = 0x80000000,                 // Unknown error
    CBMAG_ERR_INVALID_PARAMETER,              // Invalid parameter in function
    CBMAG_ERR_PARAMETER_OUT_OF_RANGE,         // Parameter is out of range and was adjusted
    CBMAG_ERR_FILE_CREATE,                    // Error creating output file
    CBMAG_ERR_FILE_OPEN,                      // Error opening input file
    CBMAG_ERR_FILE_READ_FAIL,                 // Error reading from input file
    CBMAG_ERR_FILE_WRITE_FAIL,                // Error writing to output file
    CBMAG_ERR_FILE_CORRUPT,                   // File is corrupt or not a CBMG file
    CBMAG_ERR_OUT_OF_MEMORY,                  // Out of memory - can be thrown by constructors too
    CBMAG_ERR_BUF_TOO_SMALL,                  // Buffer is too small for all of the data
};


//----------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------

// Quick macros for success / failure on the above results.
#define CBMAGSUCCESS(x)  ( ( (x) & 0x80000000 ) == 0 )
#define CBMAGFAILED(x)   ( ( (x) & 0x80000000 ) != 0 )

// Ye old min/max macros
#ifndef CBMAX
    #define CBMAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef CBMIN
    #define CBMIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif


//----------------------------------------------------------------------
// Structs
//----------------------------------------------------------------------

// Pack the struct to 1-bytes for file saving.
#pragma pack(push)
#pragma pack(1)

struct CBMAGINFOSTRUCT
{    
    union    // 'CBMG' for big-endian, 'GMBC' for little endian.
    {
        char    fCBMagSig[4];
        int     fCBMagSigInt;
    };
    
    int         fVersion;                            // Version of file
    char        fInfoName[kCBMagNameBufferSize];     // User-defined name        

    // Magnifier options - most people won't really use these, but
    // it allows people to create fully compatible magnifiers if
    // they want.
    int         fAllowFeedback;                    // Allow feedback 
                                                   // loops in magnifier?
    int         fRefresh;                          // milliseconds between refresh
    int         fOnTop;                            // Is wnd on top?    
    float       fMagnificationLevel;               // 1.0x - 10.0x multiplier
    float       fFilterMouse;                      // Low pass filter on mouse movements
                                                   // Smooths moves quite a bit.    
    int         fNegative;                         // Negate image first?

    // The values here are directly from the knobs.
    // They are between 0.0f and 1.0f, and are linear.
    // 0.5f should be none.  We'll map them to a proper
    // curve when building the lookup tables.
        
    float       fGamma;

    float       fBright_Red;
    float       fBright_Green;
    float       fBright_Blue;        

    // convert colors to grey
    float       fGreyRed;
    float       fGreyGreen;
    float       fGreyBlue;
    float       fGreyYellow;
    float       fGreyCyan;
    float       fGreyMagenta;
    
    float       fHue;                                 // Hue rotation
    float       fHueCompress;                         // Hue space compression

    int         fSwapType;                            // Swap value (see SWAPTYPE)    
    int         fMergeType;                           // Merge type - see MERGETYPE
    
    float       fSeverity;                            // Severity of simulation
    int         fReserved [31];                       // Pad for future official use.    
    int         fThirdParty[32];                      // Third party use. Have fun.
};

#pragma pack(pop)

//--------------------------------------------------------------------
// MagInfo class.
//--------------------------------------------------------------------
class CBMagInfo
{    
    //-----------------------------------------------------------------
    public:
        
        enum SWAPTYPE
        {
            SWAP_NONE,
            SWAP_GREEN_BLUE,
            SWAP_RED_BLUE,
            SWAP_RED_GREEN,
            SWAP_LAST_TYPE = SWAP_RED_GREEN,
        };

        enum MERGETYPE
        {
            MERGE_NONE,
            MERGE_Red,
            MERGE_Blue,
            MERGE_ALL,
            MERGE_Green,
            MERGE_LAST_TYPE = MERGE_Green
        };    
    
    //-----------------------------------------------------------------
    public:        

        // Constructors
        // Note: All of these may throw a CBMAGRESUlT on error.
        //       Currently the only one that may be thrown is
        //       the CBMAG_ERR_OUT_OF_MEMORY. However, that one may
        //       be thrown instead of your usual memory exception,
        //       so please check this in your code.
        CBMagInfo();        
        CBMagInfo( const CBMAGINFOSTRUCT& copyStruct);
        CBMagInfo( const CBMagInfo&       copyInfo  );
        
        // Destructor
        virtual ~CBMagInfo();        
        
        // Copy operators
        CBMagInfo& operator= (const CBMAGINFOSTRUCT& copyStruct);
        CBMagInfo& operator= (const CBMagInfo&            copyInfo);

        // Load a MagInfo file
        CBMAGRESULT LoadFromFile   (const char* filename);
        CBMAGRESULT LoadFromHandle (FILE*       fp);
        
        // Save our config to a MagInfo file
        CBMAGRESULT SaveToFile     (const char* filename);
        CBMAGRESULT SaveToHandle   (FILE*       fp);

        void Reset();

        //--------------------------------------------------------------
        // Main processing function
        //
        // RGB buffer is an array for 24-bit RGB pixels.
        // Right now, assumes it's in B,G,R order for wintel.
        // Buffer is top down (y = 0 is at the top)
        //            
        // imgWidth and imgHeight are the total image buffer
        // sizes. They are assumed not to be width-padded.
        // i.e. imgWidth*3 should be the total width of a line.
        //            
        // xOff and yOff are the offsets to the top-left of
        // the region of interest within the image array to
        // be processed.
        //
        // procWidth and procHeight are the width and height
        // of the region of interest to be processed.
        void            ProcessImage    (   unsigned char*    rgbBuffer,
                                            int               imgWidth,
                                            int               imgHeight,
                                            int               xOff,
                                            int               yOff,
                                            int               procWidth,
                                            int               procHeight,
														  bool				  skipAlpha = false);

        //--------------------------------------------------------------
        // Accessors w/validation

        CBMAGRESULT SetName         (const char* name        );
        CBMAGRESULT SetFeedback     (bool        allow       );
        CBMAGRESULT SetRefresh      (int         refresh     );
        CBMAGRESULT SetOnTop        (bool        onTop       );
        CBMAGRESULT SetMagLevel     (float       magLvl      );
        CBMAGRESULT SetFilterMouse  (float       coefficient );
        CBMAGRESULT SetNegative     (bool        negative    );
        CBMAGRESULT SetGamma        (float       gamma       );
        CBMAGRESULT SetBrightRed    (float       brightRed   );
        CBMAGRESULT SetBrightGreen  (float       brightGreen );
        CBMAGRESULT SetBrightBlue   (float       brightBlue  );
        CBMAGRESULT SetHue          (float       hue         );
        CBMAGRESULT SetSeverity     (float       severity    );
        CBMAGRESULT SetGreyRed      (float       greyRed     );
        CBMAGRESULT SetGreyGreen    (float       greyGreen   );
        CBMAGRESULT SetGreyBlue     (float       greyBlue    );
        CBMAGRESULT SetGreyYellow   (float       greyYellow  );
        CBMAGRESULT SetGreyCyan     (float       greyCyan    );
        CBMAGRESULT SetGreyMagenta  (float       greyMagenta );
        CBMAGRESULT SetCompress     (float       compress    );
        CBMAGRESULT SetSwapType     (SWAPTYPE    swapType    );
        CBMAGRESULT SetMergeType    (MERGETYPE   mergeType   );
        

        //--------------------------------------------------------------
        // Retrieval Accessors
        CBMAGRESULT GetName         (char*       nameBuffer, 
                                     int&        bufLen) const;

        int         GetRefresh      () const;        
        bool        GetFeedback     () const;
        bool        GetOnTop        () const;
        bool        GetNegative     () const;
        float       GetMagLevel     () const;
        float       GetFilterMouse  () const;
        float       GetGamma        () const;
        float       GetBrightRed    () const;
        float       GetBrightGreen  () const;
        float       GetBrightBlue   () const;
        float       GetHue          () const;
        float       GetSeverity     () const;
        float       GetGreyRed      () const;
        float       GetGreyGreen    () const;
        float       GetGreyBlue     () const;
        float       GetGreyYellow   () const;
        float       GetGreyCyan     () const;
        float       GetGreyMagenta  () const;
        float       GetCompress     () const;
        SWAPTYPE    GetSwapType     () const;
        MERGETYPE   GetMergeType    () const;
        

    //---------------------------------------------------------------
    // Lookup table builder - this rebuilds the LUTs for the
    // ProcessImage() function from the CBMAGINFOSTRUCT.  It's called
    // automatically if the fStructDirty flag is set and ProcessImage
    // gets called, but if for some reason this ends up being a long
    // function you can call it ahead of time to avoid the speed hit
    // on the first process func.
    //
    void BuildLookupTables();
    
    void BuildGamma(bool buildRed, 
                    bool buildGreen, 
                    bool buildBlue);
    
    void BuildGreys(bool buildRed, 
                    bool buildYellow, 
                    bool buildGreen, 
                    bool buildCyan, 
                    bool buildBlue, 
                    bool buildMag);

    void BuildHue();
    void BuildSeverity();
        
    //---------------------------------------------------------------
    // Static utility functions

    // Setup a mag info struct with the defaults (no process)
    static CBMAGRESULT SetupDefaults (  CBMAGINFOSTRUCT*    magInfo );

    // Endian flippers for xplat
    static void FlipEndian       (      int&                value);
    static void FlipEndian       (      float&              value);        
    static void FlipEndian       (      CBMAGINFOSTRUCT&    infoStruct);

    // Red, green, blue to Hue, saturation, intensity
    static xplat_inline void RGBtoHSI( unsigned char& rh, unsigned char& gs, unsigned char& bi);
    static xplat_inline void HSItoRGB( unsigned char& rh, unsigned char& gs, unsigned char& bi);

    // This will flip it to native.
    static CBMAGRESULT CorrectEndian    (    CBMAGINFOSTRUCT&    infoStruct);
    //-------------------------------------------------------------------
    protected:
        void InitLUTs();
        void FreeLUTs();

    //-------------------------------------------------------------------
    protected:        
        CBMAGINFOSTRUCT                   fInfo;            // All our parameters.
        bool                              fFileDirty;       // Have we modified from the file?
        bool                              fStructDirty;     // Do we need to rebuild lookups?
        
        // Lookup Tables
        unsigned char*                    fRedLUT;          // gamma/brightness lookups
        unsigned char*                    fGreenLUT;        
        unsigned char*                    fBlueLUT;
        unsigned char*                    fIntLUTRed;       // Intensity lookups
        unsigned char*                    fIntLUTGreen;
        unsigned char*                    fIntLUTBlue;
        unsigned char*                    fIntLUTYellow;    // Intensity lookups
        unsigned char*                    fIntLUTCyan;
        unsigned char*                    fIntLUTMagenta;
        unsigned char*                    fSatLUTRed;       // Saturation lookups
        unsigned char*                    fSatLUTGreen;
        unsigned char*                    fSatLUTBlue;
        unsigned char*                    fSatLUTYellow;    // Saturation lookups
        unsigned char*                    fSatLUTCyan;
        unsigned char*                    fSatLUTMagenta;
        unsigned char*                    fHueLUT;          // Hue lookups
        unsigned char*                    fMergeGreenLUT;   // Merge lut's for Greens
        unsigned char*                    fMergeRedLUT;     // Merge lut for Reds
        unsigned char*                    fMergeBlueLUT;    // Merge lut's
        unsigned char*                    fSevLUT;          // Severity LUT

        unsigned char*                    fGreyRedLUT;      // Convert red->intensity
        unsigned char*                    fGreyGreenLUT;    // Convert green->intensity
        unsigned char*                    fGreyBlueLUT;     // convert blue->intensity
};

//---------------------------------------------------
//  RGBtoHSI
//        RGB to HSI conversion (integer)
//        All values are 0-255.
//        Based on code by Jace/TBL posted to comp.graphics.algorithms on 6/18/1999
void xplat_inline CBMagInfo::RGBtoHSI (unsigned char &rh, unsigned char &gs, unsigned char &bi)
{
    int maxVal,midVal,minVal;
    // Find min/max/middle colors for intensity and
    // to determine the hue quadrant

    if (rh>=gs)
    { // rh > gs
        if (rh>=bi)
        { // rh > gs,bi
            if (gs>=bi)
            { // rh > gs > bi
                maxVal = rh;
                midVal = gs;
                minVal = bi;
                rh = 0;
            }
            else
            { // rh > bi > gs
                maxVal = rh;
                midVal = bi;
                minVal = gs;
                rh = 5;
            }
        }
        else
        { // bi > rh > gs
            maxVal = bi;
            midVal = rh;
            minVal = gs;
            rh = 4;
        }
    } 
    else
    { // gs > rh
        if (gs>=bi)
        { // gs > rh,bi
            if (rh>=bi)
            { // gs > rh > bi
                maxVal = gs;
                midVal = rh;
                minVal = bi;
                rh = 1;
            }
            else
            { // gs > bi > rh
                maxVal = gs;
                midVal = bi;
                minVal = rh;
                rh = 2;
            }
        }
        else
        { // bi > gs > rh
            maxVal = bi;
            midVal = gs;
            minVal = rh;
            rh = 3;
        }
    }
    // colors are split into maxVal,midVal,minVal and base h.
    
    
    // if (min == max), it's grey
    if (minVal == maxVal) 
    { 
        rh=0; 
        gs=0; 
        return; 
    } 
    
    // Saturation is the amount of spread between min/max
    gs = 255 - ((minVal<<8)/maxVal);

    // intensity is the brightness
    bi = maxVal;
    
    midVal = maxVal - (maxVal * (maxVal-midVal))/(maxVal-minVal);
    minVal = ( midVal << 8) / maxVal;
    if (rh & 1) 
        minVal = 256 - minVal;
    
    rh = (( ( rh << 8 ) + minVal) / 6) & 255;
}


//---------------------------------------------------
//  HSItoRGB
//        HSI to RGB conversion (integer)
//        All values are 0-255.
//        Based on code by Jace/TBL posted to comp.graphics.algorithms on 6/18/1999
xplat_inline void CBMagInfo::HSItoRGB (unsigned char &hr, unsigned char &sg, unsigned char &ib)
{
    // hue (hr) becomes red, saturation (sg) becomes green, intensity (ib) becomes blue
    int maxVal,midVal,minVal,hue;
    
    hue = 6*hr;
    maxVal = ib;
    
    minVal = (maxVal * (256-sg)) >> 8;
    
    midVal = ( ( (hue & 255 ) + 1 ) * maxVal) >> 8;
        
    if ( hue & 256) 
    {
        midVal = maxVal-midVal;
    }

    midVal = maxVal-(( (maxVal - midVal) * (sg + 1)) >>8);
    
    hue >>= 8;

    switch(hue)
    {
        case 0: hr=maxVal; sg=midVal; ib=minVal; break;
        case 1: hr=midVal; sg=maxVal; ib=minVal; break;
        case 2: hr=minVal; sg=maxVal; ib=midVal; break;
        case 3: hr=minVal; sg=midVal; ib=maxVal; break;
        case 4: hr=midVal; sg=minVal; ib=maxVal; break;
        case 5: hr=maxVal; sg=minVal; ib=midVal; break;
    }
}

//-------------------------------------------------------------------
#endif // _CBMAGINFO_H_