/// \file CBMagInfo.cpp
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
#include "CBMagInfo.h"
#include <string.h>
#include <memory.h>
#include <math.h>
//---------------------------------------------------
// CBMagInfo()
CBMagInfo::CBMagInfo()
{
    fRedLUT        = fGreenLUT    = fBlueLUT       = 0;
    fIntLUTRed     = fIntLUTGreen = fIntLUTBlue    = 0;
    fSatLUTRed     = fSatLUTGreen = fSatLUTBlue    = 0;
    fIntLUTYellow  = fIntLUTCyan  = fIntLUTMagenta = 0;
    fSatLUTYellow  = fSatLUTCyan  = fSatLUTMagenta = 0;
    fHueLUT        = 0;

    // Static LUTs
    fMergeRedLUT = fMergeGreenLUT = fMergeBlueLUT  = 0;
    fSevLUT      = 0;
    fGreyRedLUT  = fGreyGreenLUT  = fGreyBlueLUT   = 0;

    SetupDefaults(&this->fInfo);    
    fFileDirty   = true;
    InitLUTs();
}

//---------------------------------------------------
// ~CBMagInfo()
CBMagInfo::~CBMagInfo()
{
    FreeLUTs();
}

//---------------------------------------------------
// CBMagInfo()
//        Copy constructors
CBMagInfo::CBMagInfo( const CBMAGINFOSTRUCT& copyStruct)
{
    *this = copyStruct;    
    InitLUTs();
}

//---------------------------------------------------
// CBMagInfo()
//        Copy constructors
CBMagInfo::CBMagInfo( const CBMagInfo&  copyInfo  )
{
    *this = copyInfo;    
    InitLUTs();
}

//---------------------------------------------------
// ProcessImage
//        Main processing function
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
//
//
void CBMagInfo::ProcessImage    (    unsigned char*        rgbBuffer,
                                     int                   imgWidth,
                                     int                   imgHeight,
                                     int                   xOff,
                                     int                   yOff,
                                     int                   procWidth,
                                     int                   procHeight,
												 bool						  skipAlpha)
{
    int x,y;

    // Rebuild our lookup tables if something has changed.
    if (this->fStructDirty)
    {
        BuildLookupTables();
    }
    
    // per pixel for now - optimize into SIMD-type stuff
    // later as much as possible per platform.
    unsigned char tmp = 0;

    unsigned char* srcPtr;
    unsigned char* dstPtr;

	 int pixWidth = 3;
	 if (skipAlpha)
		 pixWidth++;

    // offset from end of one line to beginning of next
    int step = (imgWidth*pixWidth) - (procWidth*pixWidth);

    // Starting points in buffer
    srcPtr = rgbBuffer + ((yOff)*imgWidth*pixWidth) + xOff*pixWidth;
    dstPtr = srcPtr;

    // convert to DWORD widths.    

    for (y = yOff; y < yOff + procHeight; y++)
    {

        for (x = 0; x < procWidth; x++)
        {
            // Get pixel            
            unsigned char r,g,b;
            short bg;
            // get two in one... it'd be better to do 4, but ah well...
            bg = *(short*)srcPtr;            
            b = (unsigned char)(bg);
            g = (unsigned char)(bg >> 8);
            
            ++srcPtr;
            ++srcPtr;
            
            r = *srcPtr;
            
				++srcPtr;
            
				if (skipAlpha)
					++srcPtr;

            // Negate image
            if (fInfo.fNegative)
            {
                b = 255 - b;
                g = 255 - g;
                r = 255 -r ;
            }

            // Swap colors first
            switch (fInfo.fSwapType)
            {
                case SWAP_GREEN_BLUE: tmp = g; g = b; b = tmp; break;
                case SWAP_RED_BLUE:   tmp = r; r = b; b = tmp; break;
                case SWAP_RED_GREEN:  tmp = r; r = g; g = tmp; break;
            }                

            // Integer versions are much mo' faster, 
            // big thanks to Jace/TBL for posting that info.
            RGBtoHSI(r,g,b);

            // -------- BIG NOTE - r,g,b are now hue, saturation, and intensity!!!!
            // They are converted back by HSItoRGB.
            
            r = fHueLUT[r];

            // Grey requested colors
            if ((r < 22) || (r >= 234)) // red
            {                
                b = fIntLUTRed[b + g*256];
                g = fSatLUTRed[g];
            }            
            else if (r < 64) // yellow
            {
                b = fIntLUTYellow[b + g*256];
                g = fSatLUTYellow[g];
            }
            else if (r < 107) // green
            {
                b = fIntLUTGreen[b + g*256];
                g = fSatLUTGreen[g];
            }
            else if (r < 150) // cyan
            {
                b = fIntLUTCyan[b + g*256];
                g = fSatLUTCyan[g];
            }
            else if (r < 192) // blue
            {
                b = fIntLUTBlue[b + g*256];
                g = fSatLUTBlue[g];
            }
            else // magenta
            {
                b = fIntLUTMagenta[b + g*256];
                g = fSatLUTMagenta[g];
            }

            HSItoRGB(r,g,b);

            // Process pixel - try to use LUTs as much as possible here
            // instead of doing the calculations real-time.
            // 
            r =   fRedLUT[r];
            g =   fGreenLUT[g];
            b =   fBlueLUT[b];


            // Merge colors if any of the modes are on.
            switch (fInfo.fMergeType)
            {                
                case MERGE_Red:     
                    r =  (unsigned char)((int)fSevLUT[r] + fMergeRedLUT[g + b*256]);
                    break;                                                  
                
                case MERGE_Green:                         
                    g =  (unsigned char)((int)fSevLUT[g] + fMergeGreenLUT[r + b*256]);                     
                    break;
                
                case MERGE_Blue:  
                    b = (unsigned char)((int)fSevLUT[b] + fMergeBlueLUT[r + g*256]); 
                    break;
                
                // Can't do a single LUT in easy space, so use LUTs for intensities at least
                case MERGE_ALL: 
                    {
                        unsigned char grey = (unsigned char)(((unsigned int)fGreyRedLUT[r] + fGreyGreenLUT[g] + fGreyBlueLUT[b]) * fInfo.fSeverity);
                        r = (unsigned char)(fSevLUT[r]   + grey);
                        g = (unsigned char)(fSevLUT[g] + grey);
                        b = (unsigned char)(fSevLUT[b]  + grey);
                    }
                    break;
            }

            // get two in one... it'd be better to do 4, but ah well...
            *(short*)dstPtr = ((short)b | (g << 8));
            ++dstPtr;
            ++dstPtr;            
            *dstPtr = r;
            ++dstPtr;

				if (skipAlpha)
					++dstPtr;
        }

        // Inc to next line        
        srcPtr += step;
        dstPtr += step;
    }
}


// Build gamma related lookups
void CBMagInfo::BuildGamma(bool red, bool green, bool blue)
{
    int i;
    float gamDiv = 1.0f;
    float rLevel, gLevel, bLevel;


    for (i = 0; i < 256; i++)
    {        
        float level = (float)i;
                
        if (red)
        {
            rLevel = i + (fInfo.fBright_Red - 0.5f)*512;
            if (rLevel < 0)   rLevel = 0;
            if (rLevel > 255) rLevel = 255;
        }
        
        if (green)
        {
            gLevel = i + (fInfo.fBright_Green - 0.5f)*512;
            if (gLevel < 0)   gLevel = 0;
            if (gLevel > 255) gLevel = 255;
        }
        
        if (blue)
        {
            bLevel = i + (fInfo.fBright_Blue - 0.5f)*512;        
            if (bLevel < 0)   bLevel = 0;
            if (bLevel > 255) bLevel = 255;
        }

        float gamDiv = 1.0f;

        if (fInfo.fGamma > 0.5f)
        {            
            gamDiv = 1.0f / (1.0f - (fInfo.fGamma - 0.5f));
            
            if (red)
            {
                level = (float) pow(rLevel, gamDiv);
                if (level > 255)
                    level = 255;
                fRedLUT[i] = (unsigned char)level;
            }

            if (green)
            {
                level = (float) pow(gLevel, gamDiv);
                if (level > 255)
                    level = 255;
                fGreenLUT[i] = (unsigned char)level;
            }

            if (blue)
            {
                level = (float) pow(bLevel, gamDiv);
                if (level > 255)
                    level = 255;
                fBlueLUT[i] = (unsigned char)level;
            }

        }
        else if (fInfo.fGamma < 0.5f)
        {
            gamDiv = 1.0f/(1.0f + (0.5f - fInfo.fGamma)*2);

            if (red)
            {
                level =(float) pow(rLevel, gamDiv);
                if (level > 255)
                    level = 255;
                fRedLUT[i] = (unsigned char)level;
            }

            if (green)
            {
                level =(float) pow(gLevel, gamDiv);
                if (level > 255)
                    level = 255;
                fGreenLUT[i] = (unsigned char)level;
            }

            if (blue)
            {
                level =(float) pow(bLevel, gamDiv);
                if (level > 255)
                    level = 255;
                fBlueLUT[i] = (unsigned char)level;
            }
        }
        else
        {
            if (red)
            {
                fRedLUT[i]   = (unsigned char)rLevel;
            }
            if (green)
            {
                fGreenLUT[i] = (unsigned char)gLevel;
            }
            if (blue)
            {
                fBlueLUT[i]  = (unsigned char)bLevel;
            }
        }
    }
}


// Build grey-related lookups
void CBMagInfo::BuildGreys(bool buildRed, bool buildYellow, bool buildGreen, bool buildCyan, bool buildBlue, bool buildMag)
{
    int i,j;

    if (buildRed)
    {
        for (i = 0; i < 256; i++)
        {
            for (j = 0; j < 256; j++)
            {
                fIntLUTRed[i+j*256] = (unsigned char)((i * (255 - j*fInfo.fGreyRed * 0.3f)) / 255);
            }
            fSatLUTRed[i] = (unsigned char)(i * (1.0f - fInfo.fGreyRed));
        }
    }

    if (buildYellow)
    {
        for (i = 0; i < 256; i++)
        {
            for (j = 0; j < 256; j++)
            {
                fIntLUTYellow[i+j*256] = (unsigned char)((i * (255 - j*fInfo.fGreyYellow *0.45f)) / 255);                
            }
            fSatLUTYellow[i] = (unsigned char)(i * (1.0f - fInfo.fGreyYellow));
        }
    }

    if (buildGreen)
    {
        for (i = 0; i < 256; i++)
        {
            for (j = 0; j < 256; j++)
            {
                fIntLUTGreen[i+j*256] = (unsigned char)((i * (255 - j*fInfo.fGreyGreen * 0.59f)) / 255);
            }
            fSatLUTGreen[i] = (unsigned char)(i * (1.0f - fInfo.fGreyGreen));
        }
    }

    if (buildCyan)
    {
        for (i = 0; i < 256; i++)
        {
            for (j = 0; j < 256; j++)
            {
                fIntLUTCyan[i+j*256] = (unsigned char)((i * (255 - j*fInfo.fGreyCyan * 0.3f)) / 255);                
            }

            fSatLUTCyan[i] = (unsigned char)(i * (1.0f - fInfo.fGreyCyan));
        }
    }


    if (buildBlue)
    {
        for (i = 0; i < 256; i++)
        {
            for (j = 0; j < 256; j++)
            {
                fIntLUTBlue[i+j*256] = (unsigned char)((i * (255 - j*fInfo.fGreyBlue * 0.11f)) /255);                
            }

            fSatLUTBlue[i] = (unsigned char)(i * (1.0f - fInfo.fGreyBlue));
        }
    }

    if (buildMag)
    {
        for (i = 0; i < 256; i++)
        {
            for (j = 0; j < 256; j++)
            {
                fIntLUTMagenta[i+j*256] = (unsigned char)((i * (255 - j*fInfo.fGreyMagenta*0.21f)) /255);
            }
            
            fSatLUTMagenta[i] = (unsigned char)(i * (1.0f - fInfo.fGreyMagenta));
        }        
    }

}

void CBMagInfo::BuildHue()
{
    int i;
    // Build the lookup tables
    for (i = 0; i < 256; i++)
    {                
        // Compress hue space        
        fHueLUT[i] = (unsigned char)(i * (1.0f - fInfo.fHueCompress));
        
        // Wrap around is automatic and fine - it's a circle.        
        fHueLUT[i] += (unsigned char)(fInfo.fHue*255.0f - 128.0f);
    }    
}

void CBMagInfo::BuildSeverity()
{
    // Build severity luts
    for (int i=0; i < 256; i++)
    {
        // Severity luts for merging
        fSevLUT[i]   =  (unsigned char)((1.0f -fInfo.fSeverity)*i);

        // Setup colorblind conversion LUTs
        for (int j = 0; j < 256; j++)
        {
            // These are the assignments in processing
            fMergeRedLUT[i + j*256]   =  (unsigned char)(fInfo.fSeverity*((0.59f*i) + (0.11f*j)) * 1.42f);
            fMergeGreenLUT[i + j*256] =  (unsigned char)(fInfo.fSeverity*((0.3f*i)  + (0.11f*j)) * 2.4f);
            fMergeBlueLUT[i + j*256]  =  (unsigned char)(fInfo.fSeverity*((0.3f*i)  + (0.59f*j)) * 1.12f);
            // no lut for fMergeRGB, is 3-dimensional. Maybe LUT the grey conversion though...
        }
    }
}

//---------------------------------------------------------------
// BuildLookupTables()
//
//        Lookup table builder - this rebuilds the LUTs for the
//        ProcessImage() function from the CBMAGINFOSTRUCT.  It's called
//        automatically if the fStructDirty flag is set and ProcessImage
//        gets called, but if for some reason this ends up being a long
//        function you can call it ahead of time to avoid the speed hit
//        on the first process func.
//
void CBMagInfo::BuildLookupTables()
{

    BuildGamma(true,true,true);
    BuildGreys(true,true,true,true,true,true);
    BuildHue();
    BuildSeverity();

    // Mark structs as clean since we've rebuild from them.
    fStructDirty = false;
}

//---------------------------------------------------
// operator=
CBMagInfo& CBMagInfo::operator= (const CBMAGINFOSTRUCT& copyStruct)
{
    // Perform validation on copy
    this->SetBrightBlue    (    copyStruct.fBright_Blue          );
    this->SetBrightGreen   (    copyStruct.fBright_Green         );
    this->SetBrightRed     (    copyStruct.fBright_Red           );
    this->SetGamma         (    copyStruct.fGamma                );
    this->SetHue           (    copyStruct.fHue                  );
    this->SetSeverity      (    copyStruct.fSeverity             );
    this->SetCompress      (    copyStruct.fHueCompress          );
    this->SetGreyRed       (    copyStruct.fGreyRed              );
    this->SetGreyGreen     (    copyStruct.fGreyGreen            );
    this->SetGreyBlue      (    copyStruct.fGreyBlue             );
    this->SetGreyYellow    (    copyStruct.fGreyYellow           );
    this->SetGreyCyan      (    copyStruct.fGreyCyan             );
    this->SetGreyMagenta   (    copyStruct.fGreyMagenta          );
    this->SetMagLevel      (    copyStruct.fMagnificationLevel   );
    this->SetFilterMouse   (    copyStruct.fFilterMouse          );
    this->SetMergeType     (    (MERGETYPE)copyStruct.fMergeType );
    this->SetName          (    copyStruct.fInfoName             );
    this->SetRefresh       (    copyStruct.fRefresh              );
    this->SetFeedback      (    copyStruct.fAllowFeedback > 0    );
    this->SetOnTop         (    copyStruct.fOnTop > 0            );
    this->SetNegative      (    copyStruct.fNegative > 0         );
    this->SetSwapType      (    (SWAPTYPE)copyStruct.fSwapType   );    
        
    fFileDirty        = true;    
    return *this;
}

//---------------------------------------------------
// operator=
CBMagInfo& CBMagInfo::operator= (const CBMagInfo& copyInfo)
{
    // Do item by item so we can refresh LUTs only if needed.
    
    this->SetBrightBlue   (    copyInfo.fInfo.fBright_Blue          );
    this->SetBrightGreen  (    copyInfo.fInfo.fBright_Green         );
    this->SetBrightRed    (    copyInfo.fInfo.fBright_Red           );
    this->SetGamma        (    copyInfo.fInfo.fGamma                );
    this->SetHue          (    copyInfo.fInfo.fHue                  );
    this->SetSeverity     (    copyInfo.fInfo.fSeverity             );
    this->SetCompress     (    copyInfo.fInfo.fHueCompress          );
    this->SetGreyRed      (    copyInfo.fInfo.fGreyRed              );
    this->SetGreyGreen    (    copyInfo.fInfo.fGreyGreen            );
    this->SetGreyBlue     (    copyInfo.fInfo.fGreyBlue             );
    this->SetGreyYellow   (    copyInfo.fInfo.fGreyYellow           );
    this->SetGreyCyan     (    copyInfo.fInfo.fGreyCyan             );
    this->SetGreyMagenta  (    copyInfo.fInfo.fGreyMagenta          );
    this->SetMagLevel     (    copyInfo.fInfo.fMagnificationLevel   );
    this->SetFilterMouse  (    copyInfo.fInfo.fFilterMouse          );
    this->SetMergeType    (    (MERGETYPE)copyInfo.fInfo.fMergeType );
    this->SetName         (    copyInfo.fInfo.fInfoName             );
    this->SetRefresh      (    copyInfo.fInfo.fRefresh              );
    this->SetFeedback     (    copyInfo.fInfo.fAllowFeedback > 0    );
    this->SetOnTop        (    copyInfo.fInfo.fOnTop > 0            );
    this->SetNegative     (    copyInfo.fInfo.fNegative > 0         );
    this->SetSwapType     (    (SWAPTYPE)copyInfo.fInfo.fSwapType   );    

    fFileDirty = copyInfo.fFileDirty;
    return *this;    
}

//---------------------------------------------------
// LoadFromFile
//        Load a MagInfo file
CBMAGRESULT CBMagInfo::LoadFromFile(const char* filename)
{
    FILE *fp = fopen(filename,"rb");
    
    if (fp == 0)
    {
        return CBMAG_ERR_FILE_OPEN;
    }
    
    CBMAGRESULT result = LoadFromHandle(fp);        
    fclose(fp);

    return result;
}

//---------------------------------------------------
// LoadFromHandle
CBMAGRESULT CBMagInfo::LoadFromHandle(FILE* fp)
{
    CBMAGRESULT res = CBMAG_SUCCESS;

    CBMAGINFOSTRUCT tmpStruct;

    // Verify file read
    if (sizeof(fInfo) != fread(&tmpStruct,1,sizeof(CBMAGINFOSTRUCT),fp))
    {
        return CBMAG_ERR_FILE_READ_FAIL;
    }    

    // Correct endian-ness of struct if needed.
    if (CBMAGFAILED(res = CorrectEndian(tmpStruct)))
    {
        return res;
    }

    // Copy it
    *this = tmpStruct;

    // Version check
    if (tmpStruct.fVersion > kCBMagVersion)
    {
        res = CBMAG_STAT_NEW_VERSION;
    }

    return res;
}

//---------------------------------------------------
// SavetoFile
//        Save our config to a MagInfo file
CBMAGRESULT CBMagInfo::SaveToFile(const char* filename)
{
    FILE *fp = fopen(filename,"wb");
    
    if (fp == 0)
    {
        return CBMAG_ERR_FILE_CREATE;
    }
    
    CBMAGRESULT result = SaveToHandle(fp);        
    fclose(fp);

    return result;
}


//---------------------------------------------------
// SavetoHandle
CBMAGRESULT CBMagInfo::SaveToHandle   (FILE*       fp)
{
    if (sizeof(fInfo) != fwrite(&fInfo,1,sizeof(fInfo),fp))
    {
        return CBMAG_ERR_FILE_WRITE_FAIL;
    }

    fFileDirty = false;
    return CBMAG_SUCCESS;
}

//----------------------------------------------------
// Accessors w/validation
//----------------------------------------------------


//---------------------------------------------------
// SetName
CBMAGRESULT CBMagInfo::SetName(const char* name)
{
    if (name == 0)
    {
        strcpy(fInfo.fInfoName, "Untitled");
        return CBMAG_SUCCESS;
    }

    strncpy(fInfo.fInfoName,name,kCBMagMaxNameLength);
    fInfo.fInfoName[kCBMagMaxNameLength] = 0;
    
    fFileDirty = true;        

    return CBMAG_SUCCESS;
}

//---------------------------------------------------
// SetOnTop
CBMAGRESULT CBMagInfo::SetOnTop(bool onTop)
{    
    fInfo.fOnTop = onTop;
    fFileDirty   = true;    
    
    return CBMAG_SUCCESS;
}

//---------------------------------------------------
// SetNegative
CBMAGRESULT CBMagInfo::SetNegative(bool negative)
{    
    fInfo.fNegative   = negative;
    fFileDirty        = true;    
    return CBMAG_SUCCESS;
}
//---------------------------------------------------
// SetFeedback
CBMAGRESULT CBMagInfo::SetFeedback(bool allow)
{    
    fInfo.fAllowFeedback  = allow?1:0;
    fFileDirty            = true;    
    
    return CBMAG_SUCCESS;
}
//---------------------------------------------------
// SetRefresh
//    time is in milliseconds between refreshes
CBMAGRESULT CBMagInfo::SetRefresh(int refresh)
{    
    if (fInfo.fRefresh == refresh)
        return CBMAG_SUCCESS;

    fInfo.fRefresh         = refresh;
    fFileDirty             = true;    
    return CBMAG_SUCCESS;
}

//---------------------------------------------------
// SetMagLevel
CBMAGRESULT CBMagInfo::SetMagLevel(float magLvl)
{
    if (fInfo.fMagnificationLevel == magLvl)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;

    if (magLvl < 1.0f)
    {
        magLvl = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }


    if (magLvl > 16.0f)
    {
        magLvl = 16.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    fInfo.fMagnificationLevel = magLvl;
    
    fFileDirty        = true;
//    fStructDirty    = true;
    
    return res;
}

//---------------------------------------------------
// SetFilterMouse
CBMAGRESULT CBMagInfo::SetFilterMouse(float coef)
{
    if (fInfo.fFilterMouse = coef)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;

    if (coef < 0.0f)
    {
        coef = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }


    if (coef > 0.9f)
    {
        coef = 0.9f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    fInfo.fFilterMouse = coef;
    
    fFileDirty        = true;
//    fStructDirty    = true;
    
    return res;
}


//---------------------------------------------------
// SetGammaBlue
CBMAGRESULT CBMagInfo::SetGamma(float gamma)
{
    if (fInfo.fGamma == gamma)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;

    if (gamma < 0.0f)
    {
        gamma = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    
    if (gamma > 1.0f)
    {
        gamma = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }


    fInfo.fGamma        = gamma;
    fFileDirty          = true;
    
    BuildGamma(true,true,true);

    return res;
}

//---------------------------------------------------
// SetBrightRed
CBMAGRESULT CBMagInfo::SetBrightRed(float brightRed)
{
    if (fInfo.fBright_Red == brightRed)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;

    if (brightRed < 0.0f)
    {
        brightRed = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    
    if (brightRed > 1.0f)
    {
        brightRed = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }


    fInfo.fBright_Red = brightRed;
    fFileDirty        = true;
    
    BuildGamma(true,false,false);

    return res;
}

//---------------------------------------------------
// SetBrightGreen
CBMAGRESULT CBMagInfo::SetBrightGreen(float brightGreen)
{
    if (fInfo.fBright_Green == brightGreen)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;

    if (brightGreen < 0.0f)
    {
        brightGreen = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    
    if (brightGreen > 1.0f)
    {
        brightGreen = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    fInfo.fBright_Green    = brightGreen;
    fFileDirty             = true;
    
    BuildGamma(false,true,false);
    return res;
}

//---------------------------------------------------
// SetBrightBlue
CBMAGRESULT CBMagInfo::SetBrightBlue(float brightBlue)
{
    CBMAGRESULT res = CBMAG_SUCCESS;
    if (this->fInfo.fBright_Blue == brightBlue)
        return res;

    if (brightBlue < 0.0f)
    {
        brightBlue = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    
    if (brightBlue > 1.0f)
    {
        brightBlue = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    fInfo.fBright_Blue    = brightBlue;
    fFileDirty            = true;

    BuildGamma(false,false,true);
    return res;
}

//---------------------------------------------------
// SetHue
CBMAGRESULT CBMagInfo::SetHue(float hue)
{
    CBMAGRESULT res = CBMAG_SUCCESS;
    if (this->fInfo.fHue == hue)
        return res;
    
    if (hue < 0.0f)
    {    
        hue = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    if (hue > 1.0f)
    {
        hue = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }


    fInfo.fHue        = hue;
    fFileDirty        = true;
    
    BuildHue();

    return res;
}

//---------------------------------------------------
// SetSeverity
CBMAGRESULT CBMagInfo::SetSeverity(float severity)
{
    CBMAGRESULT res = CBMAG_SUCCESS;
    if (this->fInfo.fSeverity == severity)
        return res;
    
    if (severity < 0.0f)
    {    
        severity = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    if (severity > 1.0f)
    {
        severity = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }


    fInfo.fSeverity = severity;
    fFileDirty      = true;
    
    BuildSeverity();
    
    return res;
}

//---------------------------------------------------
// SetGreyRed
CBMAGRESULT CBMagInfo::SetGreyRed(float addbg2r)
{
    if (fInfo.fGreyRed == addbg2r)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;
    
    if (addbg2r < 0.0f)
    {    
        addbg2r = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    if (addbg2r > 1.0f)
    {
        addbg2r = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    fInfo.fGreyRed    = addbg2r;    
    fFileDirty        = true;
    
    BuildGreys(true,false,false,false,false,false);
    return res;
}

//---------------------------------------------------
// SetGreyGreen
CBMAGRESULT CBMagInfo::SetGreyGreen(float addbr2g)
{
    if (fInfo.fGreyGreen == addbr2g)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;
    
    if (addbr2g < 0.0f)
    {    
        addbr2g = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    if (addbr2g > 1.0f)
    {
        addbr2g = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    fInfo.fGreyGreen    = addbr2g;    
    fFileDirty          = true;
    
    BuildGreys(false,false,true,false,false,false);

    return res;
}

//---------------------------------------------------
// SetGreyBlue
CBMAGRESULT CBMagInfo::SetGreyBlue(float addrg2b)
{
    if (fInfo.fGreyBlue == addrg2b)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;
    
    if (addrg2b < 0.0f)
    {    
        addrg2b = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    if (addrg2b > 1.0f)
    {
        addrg2b = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    fInfo.fGreyBlue        = addrg2b;    
    fFileDirty             = true;

    BuildGreys(false,false,false,false,true,false);

    return res;
}
//---------------------------------------------------
// SetGreyYellow
CBMAGRESULT CBMagInfo::SetGreyYellow(float addbg2r)
{
    if (fInfo.fGreyYellow == addbg2r)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;
    
    if (addbg2r < 0.0f)
    {    
        addbg2r = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    if (addbg2r > 1.0f)
    {
        addbg2r = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    fInfo.fGreyYellow    = addbg2r;    
    fFileDirty           = true;
    
    BuildGreys(false,true,false,false,false,false);

    return res;
}
//---------------------------------------------------
// SetGreyCyan
CBMAGRESULT CBMagInfo::SetGreyCyan(float addbg2r)
{
    if (fInfo.fGreyCyan == addbg2r)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;
    
    if (addbg2r < 0.0f)
    {    
        addbg2r = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    if (addbg2r > 1.0f)
    {
        addbg2r = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    fInfo.fGreyCyan    = addbg2r;    
    fFileDirty         = true;
    
    BuildGreys(false,false,false,true,false,false);
    
    return res;
}
//---------------------------------------------------
// SetGreyMagenta
CBMAGRESULT CBMagInfo::SetGreyMagenta(float addbg2r)
{
    if (fInfo.fGreyMagenta == addbg2r)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;
    
    if (addbg2r < 0.0f)
    {    
        addbg2r = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    if (addbg2r > 1.0f)
    {
        addbg2r = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    fInfo.fGreyMagenta    = addbg2r;    
    fFileDirty            = true;
    
    BuildGreys(false,false,false,false,false,true);
    
    return res;
}

//---------------------------------------------------
// SetSwapType
CBMAGRESULT CBMagInfo::SetSwapType(SWAPTYPE swapType)
{
    if (fInfo.fSwapType == swapType)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;

    if (swapType > SWAP_LAST_TYPE)
    {
        swapType = SWAP_NONE;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    
    if (swapType < SWAP_NONE)
    {
        swapType = SWAP_NONE;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }


    fInfo.fSwapType    = swapType;
    fFileDirty         = true;    

    return res;
}

//---------------------------------------------------
// SetMergeType
CBMAGRESULT CBMagInfo::SetMergeType(MERGETYPE mergeType)
{
    if (fInfo.fMergeType == mergeType)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;

    if (mergeType > MERGE_LAST_TYPE)
    {
        mergeType = MERGE_NONE;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    if (mergeType < MERGE_NONE)
    {
        mergeType = MERGE_NONE;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }
        
    fInfo.fMergeType = mergeType;
    fFileDirty = true;    

    return res;
}


//---------------------------------------------------
// SetCompress
CBMAGRESULT CBMagInfo::SetCompress(float hueCompress)
{
    if (fInfo.fHueCompress == hueCompress)
        return CBMAG_SUCCESS;

    CBMAGRESULT res = CBMAG_SUCCESS;
    
    if (hueCompress < 0.0f)
    {    
        hueCompress = 0.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }

    if (hueCompress > 1.0f)
    {
        hueCompress = 1.0f;
        res = CBMAG_ERR_PARAMETER_OUT_OF_RANGE;
    }


    fInfo.fHueCompress    = hueCompress;
    fFileDirty            = true;
    
    BuildHue();

    return res;
}


//---------------------------------------------
// Retrieval Accessors
//---------------------------------------------



//---------------------------------------------------
// GetName
CBMAGRESULT CBMagInfo::GetName( char*        nameBuffer, 
                                int&         bufLen) const
{
    CBMAGRESULT res = CBMAG_SUCCESS;

    if (nameBuffer == 0)
    {
        return CBMAG_ERR_INVALID_PARAMETER;
    }

    strncpy(nameBuffer, fInfo.fInfoName, bufLen);
    
    if (bufLen <= (int)strlen(fInfo.fInfoName))
    {        
        nameBuffer[bufLen - 1] = 0;        
        res = CBMAG_ERR_BUF_TOO_SMALL;
    }

    bufLen = (int)strlen(fInfo.fInfoName);
    
    return res;
}

//---------------------------------------------------
// GetOnTop
bool CBMagInfo::GetOnTop() const
{
    return (fInfo.fOnTop > 0);
}

//---------------------------------------------------
// GetNegative
bool CBMagInfo::GetNegative() const
{
    return (fInfo.fNegative > 0);
}

//---------------------------------------------------
// GetFeedback
bool CBMagInfo::GetFeedback() const
{    
    return fInfo.fAllowFeedback > 0;
}
//---------------------------------------------------
// GetRefresh
//    time is in milliseconds between refreshes
int CBMagInfo::GetRefresh() const
{    
    return fInfo.fRefresh;
}

//---------------------------------------------------
// GetMagLevel
float CBMagInfo::GetMagLevel() const
{
    return fInfo.fMagnificationLevel;
}

//---------------------------------------------------
// GetFilterMouse
float CBMagInfo::GetFilterMouse() const
{
    return fInfo.fFilterMouse;
}

//---------------------------------------------------
// GetGamma
float CBMagInfo::GetGamma() const
{
    return fInfo.fGamma;
}

//---------------------------------------------------
// GetBrightRed
float CBMagInfo::GetBrightRed() const
{
    return fInfo.fBright_Red;
}

//---------------------------------------------------
// GetBrightGreen
float CBMagInfo::GetBrightGreen() const
{
    return fInfo.fBright_Green;
}

//---------------------------------------------------
// GetBrightBlue
float CBMagInfo::GetBrightBlue() const
{
    return fInfo.fBright_Red;
}

//---------------------------------------------------
// GetHue
float CBMagInfo::GetHue() const
{
    return fInfo.fHue;
}

//---------------------------------------------------
// GetSeverity
float CBMagInfo::GetSeverity() const
{
    return fInfo.fSeverity;
}


//---------------------------------------------------
// 
float CBMagInfo::GetGreyRed() const
{
    return fInfo.fGreyRed;
}
float CBMagInfo::GetGreyGreen() const
{
    return fInfo.fGreyGreen;
}
float CBMagInfo::GetGreyBlue() const
{
    return fInfo.fGreyBlue;
}
float CBMagInfo::GetGreyYellow() const
{
    return fInfo.fGreyYellow;
}
float CBMagInfo::GetGreyCyan() const
{
    return fInfo.fGreyCyan;
}
float CBMagInfo::GetGreyMagenta() const
{
    return fInfo.fGreyMagenta;
}

//---------------------------------------------------
// GetSwapType
CBMagInfo::SWAPTYPE CBMagInfo::GetSwapType() const
{
    return (CBMagInfo::SWAPTYPE)fInfo.fSwapType;
}

//---------------------------------------------------
// GetMergeType
CBMagInfo::MERGETYPE CBMagInfo::GetMergeType() const
{
    return (CBMagInfo::MERGETYPE)fInfo.fMergeType;
}


//---------------------------------------------------
// GetCompress
float CBMagInfo::GetCompress() const
{
    return fInfo.fHueCompress;
}


//---------------------------------------------------
// SetupDefaults
//        Setup a mag info struct with the defaults (no process)
CBMAGRESULT CBMagInfo::SetupDefaults ( CBMAGINFOSTRUCT* magInfo )
{
    if (magInfo == 0)    
        return CBMAG_ERR_INVALID_PARAMETER;
    
    memset(magInfo->fInfoName,0,kCBMagNameBufferSize);
    strcpy(magInfo->fInfoName,"Untitled");
    
    magInfo->fCBMagSigInt          = 0x43424D47;
    magInfo->fVersion              = kCBMagVersion;
    magInfo->fGamma                = 0.5f;
    magInfo->fBright_Red           = 0.5f;
    magInfo->fBright_Green         = 0.5f;
    magInfo->fBright_Blue          = 0.5f;
    magInfo->fHue                  = 0.5f;
    magInfo->fSeverity             = 1.0f;
    magInfo->fHueCompress          = 0.0f;
    
    magInfo->fGreyRed              = 0.0f;
    magInfo->fGreyGreen            = 0.0f;
    magInfo->fGreyBlue             = 0.0f;
    magInfo->fGreyYellow           = 0.0f;
    magInfo->fGreyCyan             = 0.0f;
    magInfo->fGreyMagenta          = 0.0f;
    
    magInfo->fSwapType             = SWAP_NONE;
    magInfo->fMergeType            = MERGE_NONE;
    magInfo->fOnTop                = true;    
    magInfo->fNegative             = false;    
    magInfo->fMagnificationLevel   = 1.0f;
    magInfo->fFilterMouse          = 0.0f;
    magInfo->fAllowFeedback        = false;
    magInfo->fRefresh              = 100;    // 10Hz refresh speed default (for slower PC's)
        
    memset(magInfo->fReserved,0,sizeof(magInfo->fReserved));
    memset(magInfo->fThirdParty,0,sizeof(magInfo->fThirdParty));    
    return CBMAG_SUCCESS;
}

//---------------------------------------------------
// FlipEndian (float)
//     Swap the endian-ness of the value
//     For using mac files on pc, vise-versa, etc.
void CBMagInfo::FlipEndian(float& value)
{
    int* tmpSpot = (int *)&value;
    
    FlipEndian(*tmpSpot);

    value = *(float *)tmpSpot;
}

//---------------------------------------------------
// FlipEndian (int)
void CBMagInfo::FlipEndian(int& value)
{
    value = ((value & 0x000000ff) << 24) |
            ((value & 0x0000ff00) << 8)  |
            ((value & 0x00ff0000) >> 8)  |
            ((value & 0xff000000) >> 24);
}


//---------------------------------------------------
// FlipEndian (entire structure)
void CBMagInfo::FlipEndian(CBMAGINFOSTRUCT& infoStruct)
{
    // It's in reversed endian from us. flip everything.
    FlipEndian(infoStruct.fCBMagSigInt);
    FlipEndian(infoStruct.fVersion);
    FlipEndian(infoStruct.fAllowFeedback);
    FlipEndian(infoStruct.fRefresh);
    FlipEndian(infoStruct.fOnTop);
    FlipEndian(infoStruct.fNegative);
    FlipEndian(infoStruct.fMagnificationLevel);
    FlipEndian(infoStruct.fFilterMouse);
    FlipEndian(infoStruct.fGamma);
    FlipEndian(infoStruct.fBright_Red);
    FlipEndian(infoStruct.fBright_Green);
    FlipEndian(infoStruct.fBright_Blue);
    FlipEndian(infoStruct.fGreyRed);
    FlipEndian(infoStruct.fGreyGreen);
    FlipEndian(infoStruct.fGreyBlue);
    FlipEndian(infoStruct.fGreyYellow);
    FlipEndian(infoStruct.fGreyCyan);
    FlipEndian(infoStruct.fGreyMagenta);
    FlipEndian(infoStruct.fHue);
    FlipEndian(infoStruct.fSeverity);
    FlipEndian(infoStruct.fHueCompress);
    FlipEndian(infoStruct.fSwapType);
    FlipEndian(infoStruct.fMergeType);
}

//---------------------------------------------------
// CorrectEndian (corrects to our machine)
CBMAGRESULT CBMagInfo::CorrectEndian(CBMAGINFOSTRUCT&    infoStruct)
{
    // Check sanity on the struct...
    if (0x43424D47 == infoStruct.fCBMagSigInt)
    {
        // It's good and in our format.
        return CBMAG_SUCCESS;
    }
    else if (0x474D4243 == infoStruct.fCBMagSigInt)
    {
        FlipEndian(infoStruct);
        return CBMAG_STAT_ENDIAN_FLIPPED;
    }
    else
    {
        return CBMAG_ERR_FILE_CORRUPT;
    }
}

//---------------------------------------------------
void CBMagInfo::InitLUTs()
{
    // Some platforms, new throws on, some it doesn't.
    // If a new fails, throw. We catch at the end, clean up,
    // and rethrow, so CBMagInfo() can only throw a
    // CBMAG_ERR_OUT_OF_MEMORY error.
    FreeLUTs();

    try
    {
        if (0 == (fRedLUT   = new unsigned char[256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }
        
        if (0 == (fGreenLUT = new unsigned char[256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fBlueLUT  = new unsigned char[256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fGreyRedLUT   = new unsigned char[256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }
        
        if (0 == (fGreyGreenLUT = new unsigned char[256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fGreyBlueLUT  = new unsigned char[256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }


        if (0 == (fIntLUTRed = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }
        
        if (0 == (fIntLUTGreen = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fIntLUTBlue  = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fIntLUTYellow = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }
        
        if (0 == (fIntLUTCyan = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fIntLUTMagenta  = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fSatLUTRed = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }
        
        if (0 == (fSatLUTGreen = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fSatLUTBlue  = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fSatLUTYellow = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }
        
        if (0 == (fSatLUTCyan = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fSatLUTMagenta  = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fHueLUT  = new unsigned char[256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }


        if (0 == (fMergeRedLUT  = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fMergeGreenLUT  = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fMergeBlueLUT  = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        if (0 == (fSevLUT  = new unsigned char[256*256]))
        {
            throw CBMAG_ERR_OUT_OF_MEMORY;
        }

        memset(fRedLUT,0,256);
        memset(fGreenLUT,0,256);
        memset(fBlueLUT,0,256);        
        memset(fGreyRedLUT,0,256);
        memset(fGreyGreenLUT,0,256);
        memset(fGreyBlueLUT,0,256);        
        memset(fIntLUTRed,0,256*256);
        memset(fIntLUTGreen,0,256*256);
        memset(fIntLUTBlue,0,256*256);        
        memset(fIntLUTYellow,0,256*256);
        memset(fIntLUTCyan,0,256*256);
        memset(fIntLUTMagenta,0,256*256);        
        memset(fSatLUTRed,0,256);
        memset(fSatLUTGreen,0,256);
        memset(fSatLUTBlue,0,256);        
        memset(fSatLUTYellow,0,256);
        memset(fSatLUTCyan,0,256);
        memset(fSatLUTMagenta,0,256);        
        memset(fHueLUT,0,256);        
        memset(fMergeRedLUT,0,256*256);        
        memset(fMergeGreenLUT,0,256*256);        
        memset(fMergeBlueLUT,0,256*256);        
        memset(fSevLUT,0,256*256);        
    }        
    catch (...)
    {
        FreeLUTs();
        throw CBMAG_ERR_OUT_OF_MEMORY;
    }

    // Setup static LUTs
    int i;
    for (i=0; i<256; i++)
    {        
        // Greyscale intensity LUTs for colors. Based on standard NTSC values
        fGreyRedLUT[i]     = (unsigned char)(i*0.3f);
        fGreyGreenLUT[i]   = (unsigned char)(i*0.59f);
        fGreyBlueLUT[i]    = (unsigned char)(i*0.11f);
    }

    fStructDirty = true;
}

//---------------------------------------------------
// FreeLUTs
//    Private function to free the lookup tables
//    Don't call externally.
//
void CBMagInfo::FreeLUTs()
{
    if (fRedLUT)        { delete fRedLUT;        fRedLUT        = 0;  }
    if (fGreenLUT)      { delete fGreenLUT;      fGreenLUT      = 0;  }
    if (fBlueLUT)       { delete fBlueLUT;       fBlueLUT       = 0;  }
    if (fGreyRedLUT)    { delete fGreyRedLUT;    fGreyRedLUT    = 0;  }
    if (fGreyGreenLUT)  { delete fGreyGreenLUT;  fGreyGreenLUT  = 0;  }
    if (fGreyBlueLUT)   { delete fGreyBlueLUT;   fGreyBlueLUT   = 0;  }
    if (fIntLUTRed)     { delete fIntLUTRed;     fIntLUTRed     = 0;  }
    if (fIntLUTGreen)   { delete fIntLUTGreen;   fIntLUTGreen   = 0;  }
    if (fIntLUTBlue)    { delete fIntLUTBlue;    fIntLUTBlue    = 0;  }
    if (fIntLUTYellow)  { delete fIntLUTYellow;  fIntLUTYellow  = 0;  }
    if (fIntLUTCyan)    { delete fIntLUTCyan;    fIntLUTCyan    = 0;  }
    if (fIntLUTMagenta) { delete fIntLUTMagenta; fIntLUTMagenta = 0;  }
    if (fSatLUTRed)     { delete fSatLUTRed;     fSatLUTRed     = 0;  }
    if (fSatLUTGreen)   { delete fSatLUTGreen;   fSatLUTGreen   = 0;  }
    if (fSatLUTBlue)    { delete fSatLUTBlue;    fSatLUTBlue    = 0;  }
    if (fSatLUTYellow)  { delete fSatLUTYellow;  fSatLUTYellow  = 0;  }
    if (fSatLUTCyan)    { delete fSatLUTCyan;    fSatLUTCyan    = 0;  }
    if (fSatLUTMagenta) { delete fSatLUTMagenta; fSatLUTMagenta = 0;  }
    if (fHueLUT)        { delete fHueLUT;        fHueLUT        = 0;  }
    if (fMergeRedLUT)   { delete fMergeRedLUT;   fMergeRedLUT   = 0;  }
    if (fMergeGreenLUT) { delete fMergeGreenLUT; fMergeGreenLUT = 0;  }
    if (fMergeBlueLUT)  { delete fMergeBlueLUT;  fMergeBlueLUT  = 0;  }
    if (fSevLUT)        { delete fSevLUT;        fSevLUT        = 0;  }
}


void CBMagInfo::Reset()
{
    SetupDefaults(&fInfo);
    fStructDirty = true;
    BuildLookupTables();
}

