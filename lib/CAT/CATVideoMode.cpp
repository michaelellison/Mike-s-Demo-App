/// \file CATVideoMode.cpp
/// \brief Generic Video Mode handling by Michael Ellison
/// \ingroup CAT
///
///
// The MIT License
//
// Copyright (c) 2008 by Michael Ellison
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include <algorithm>
#include "CATVideoMode.h"

CATVideoMode::CATVideoMode()
{
    fInitializedAndClean = false;
    fMonitorModesOnly    = true;
}

CATVideoMode::~CATVideoMode()
{
    if (fInitializedAndClean)
        Uninitialize();
}


CVM_RESULT CATVideoMode::Initialize(bool monitorModesOnly)
{
    Uninitialize();

    fMonitorModesOnly = monitorModesOnly;

    GetAllDisplays(this->fDisplays,fMonitorModesOnly);

    for (CVMUInt32 i = 0; i < fDisplays.size(); i++)
    {
        GetAllModes(fDisplays[i],fModes,fMonitorModesOnly);
    }

    fInitializedAndClean = true;

    return CVM_SUCCESS;
}

void CATVideoMode::Uninitialize()
{
    fDisplays.clear();
    fModes.clear();    
    fInitializedAndClean = false;
}

CVMUInt32 CATVideoMode::GetNumberOfDisplays()
{
    CVM_RESULT result = CVM_SUCCESS;

    if (!fInitializedAndClean)
    {
        result = Initialize(fMonitorModesOnly);
        if (CVM_FAILED(result))
            return 0;
    }
        
    return (CVMUInt32)fDisplays.size();
}

CVM_RESULT CATVideoMode::GetCurrentDisplayMode( CVMInt32 display,
                                                CVM_INFO_ADAPTER_CURRENT* info)
{
    CVM_RESULT result = CVM_SUCCESS;

    if (info == 0)
        return CVM_ERR_INVALID_PARAM;

    if (!fInitializedAndClean)
    {
        result = Initialize(fMonitorModesOnly);
        if (CVM_FAILED(result))
            return result;
    }

    if ((display < 0) || (display >= (CVMInt32)this->fDisplays.size()))
    {
        return CVM_ERR_INVALID_PARAM;
    }


    *info = fDisplays[display];
    
    return result;
}

// Retrieves the current display mode for the display at the 
// specified point.
CVM_RESULT CATVideoMode::GetDisplayFromPoint  ( CVMInt32 x, 
                                                CVMInt32 y,
                                                CVM_FIND_OPTION  findOption,
                                                CVM_INFO_ADAPTER_CURRENT* info)
{
    CVMUInt32  i;
    CVM_RESULT result = CVM_SUCCESS;

    if (!fInitializedAndClean)
    {
        result = Initialize(fMonitorModesOnly);
        if (CVM_FAILED(result))
            return result;
    }

    if (info == 0)
        return CVM_ERR_INVALID_PARAM;

    for (i = 0; i < this->fDisplays.size(); i++)
    {
        if (PointInDisplay(x,y,&fDisplays[i]))
        {
            *info = fDisplays[i];
            return CVM_SUCCESS;
        }
    }

    if (findOption == CVM_DEFAULT_NULL)
        return CVM_ERR_NO_MATCHING_DISPLAY;

    if (findOption == CVM_DEFAULT_PRIMARY)
    {
        for (i = 0; i < this->fDisplays.size(); i++)
        {
            if (fDisplays[i].Primary)
            {
                *info = fDisplays[i];
                return CVM_SUCCESS;
            }
        }

        return CVM_ERR_NO_MATCHING_DISPLAY;
    }

    if (findOption == CVM_DEFAULT_NEAREST)
    {
        // Find nearest monitor
        for (i = 0; i < this->fDisplays.size(); i++)
        {
            CVM_ERR_NO_MATCHING_DISPLAY;
        }
    }

    return CVM_ERR_NO_MATCHING_DISPLAY;
}

/// Intersection info used for sorting lists of display
/// intersections.
struct IntersectInfo
{
    CVM_INFO_ADAPTER_CURRENT    DisplayInfo;
    CVMRECT                     IntersectRect;
};

/// Sorting callback 
bool IntersectCompare(const IntersectInfo& d1, const IntersectInfo& d2)
{
    CVMInt32 d1Area = (d1.IntersectRect.right  - d1.IntersectRect.left) * 
                      (d1.IntersectRect.bottom - d1.IntersectRect.top);

    CVMInt32 d2Area = (d2.IntersectRect.right  - d2.IntersectRect.left) * 
                      (d2.IntersectRect.bottom - d2.IntersectRect.top);

	return d2Area < d1Area;
}


// Retrieves a sorted list of display(s) under the specified
// rectangle. Displays containing the largest portion of the
// rectangle will be sorted to the beginning of the list.
CVM_RESULT CATVideoMode::GetDisplaysFromRect( 
                 CVMRECT                                 rect,
                 std::vector<CVM_INFO_ADAPTER_CURRENT>&  displayList)
{
    CVM_RESULT result = CVM_SUCCESS;
    if (!fInitializedAndClean)
    {
        result = Initialize(fMonitorModesOnly);
        if (CVM_FAILED(result))
            return result;
    }
    
    displayList.clear();

    std::vector<IntersectInfo> intersectList;


    // Find display(s)
    CVMUInt32 i;
    for (i = 0; i < this->fDisplays.size(); i++)
    {
        IntersectInfo curIntersect;
        curIntersect.DisplayInfo = fDisplays[i];
        
        if (IntersectRectDisplay(rect,
                &curIntersect.IntersectRect, 
                &curIntersect.DisplayInfo))
        {            
            intersectList.push_back(curIntersect); 
        }
    }

    // Now sort in order of intersection size.
    std::sort(intersectList.begin(), intersectList.end(), IntersectCompare); 

    // Copy sorted into display list
    for (i = 0; i < intersectList.size(); i++)
    {
        displayList.push_back(intersectList[i].DisplayInfo);
    }

    intersectList.clear();
    
    if (displayList.size())
        return CVM_SUCCESS;

    return CVM_ERR_NO_MATCHING_DISPLAY;
}    

CVM_RESULT CATVideoMode::GetModes(CVMInt32                        display,
                                  std::vector<CVM_INFO_ADAPTER>&  modeList,
                                  CVMInt32                        width,
                                  CVMInt32                        height,
                                  CVMInt32                        refreshRate,
                                  CVMInt32                        bitsPerPixel,
                                  CVM_ORIENT                      orient)
{
    for (CVMUInt32 i = 0; i < fModes.size(); i++)
    {
        if ((display != -1) && (fModes[i].DisplayNumber != display))
            continue;

        if ((width        != 0) && (fModes[i].WidthPixels   != width))
            continue;

        if ((height       != 0) && (fModes[i].HeightPixels  != height))
            continue;

        if ((refreshRate  != 0) && (fModes[i].RefreshRateHz != refreshRate))
            continue;

        if ((bitsPerPixel != 0) && (fModes[i].BitsPerPixel  != bitsPerPixel))
            continue;

        if ((orient != CVM_ROT_ANY) && (fModes[i].Orientation != orient))
            continue;

        modeList.push_back(fModes[i]);
    }

    return CVM_SUCCESS;
}

// Checks if a point is within a specific display.
bool CATVideoMode::PointInDisplay(CVMInt32 x, 
                                  CVMInt32 y, 
                                  CVM_INFO_ADAPTER_CURRENT* displayInfo)
{
    if ( (( x >= displayInfo->Left ) && 
          ( x < displayInfo->Left + displayInfo->WidthPixels)) &&
         (( y >= displayInfo->Top  ) && 
          ( y < displayInfo->Top  + displayInfo->HeightPixels)))
    {
        return true;
    }

    return false;
}

// Intersects the srcRect with the display rect.
bool CATVideoMode::IntersectRectDisplay( CVMRECT   srcRect,
                                         CVMRECT*  intersect,
                                         CVM_INFO_ADAPTER_CURRENT* displayInfo)
{

    CVMRECT disRect;
    disRect.left   = displayInfo->Left;
    disRect.top    = displayInfo->Top;
    disRect.right  = displayInfo->Left + displayInfo->WidthPixels;
    disRect.bottom = displayInfo->Top + displayInfo->HeightPixels;

    // Skip if totally out 
    // Remember bottom/right are exclusive, hence >= / <= 
    if ((disRect.right     <= srcRect.left) || 
        (disRect.left      >= srcRect.right) ||
        (disRect.bottom    <= srcRect.top) ||
        (disRect.top       >= srcRect.bottom) ||
        (displayInfo->WidthPixels   == 0) ||
        (displayInfo->HeightPixels  == 0) ||
        (srcRect.right  - srcRect.left == 0) ||
        (srcRect.bottom - srcRect.top  == 0) )
    {
        // Zero rect if present
        if (intersect != 0)
        {
            intersect->left = intersect->right  = 0;
            intersect->top  = intersect->bottom = 0;
        }

        return false;
    }

    // If we don't need to know intersect, then bail here         
    if (intersect == 0)
    {
        return true;
    }


    // Calc intersection between the two
    intersect->left   = CVMMax(disRect.left,    srcRect.left);
    intersect->top    = CVMMax(disRect.top,     srcRect.top);
    intersect->right  = CVMMin(disRect.right,   srcRect.right);
    intersect->bottom = CVMMin(disRect.bottom,  srcRect.bottom);
    return true;
}

// Determines if the inside rect is fully contained within the display.
bool CATVideoMode::InsideDisplay(CVMRECT                   insideRect, 
                                 CVM_INFO_ADAPTER_CURRENT* displayInfo)
{
    CVMRECT disRect;
    disRect.left   = displayInfo->Left;
    disRect.top    = displayInfo->Top;
    disRect.right  = displayInfo->Left + displayInfo->WidthPixels;
    disRect.bottom = displayInfo->Top  + displayInfo->HeightPixels;

    if (( disRect.left   <= insideRect.left   ) &&
        ( disRect.right  >= insideRect.right  ) &&
        ( disRect.top    <= insideRect.top    ) &&
        ( disRect.bottom >= insideRect.bottom ) )
    {
        return true;
    }
    return false;
}

CVMRECT CATVideoMode::GetBoundingDisplayRect()
{
    if (!fInitializedAndClean)
        Initialize(fMonitorModesOnly);

    CVMRECT bounds;
    memset(&bounds,0,sizeof(CVMRECT));

    if (fDisplays.size() == 0)
        return bounds;

    bounds.left   = fDisplays[0].Left;
    bounds.top    = fDisplays[0].Top;
    bounds.right  = bounds.left + fDisplays[0].WidthPixels;
    bounds.bottom = bounds.top  + fDisplays[0].HeightPixels;

    for (CVMUInt32 i = 1; i < fDisplays.size(); i++)
    {
        bounds.left   = CVMMin(bounds.left,      fDisplays[i].Left);
        bounds.top    = CVMMin(bounds.top,       fDisplays[i].Top);
        
        bounds.right  = CVMMax(bounds.right,     
                               fDisplays[i].Left + fDisplays[i].WidthPixels);
        
        bounds.bottom = CVMMax(bounds.bottom,
                               fDisplays[i].Top + fDisplays[i].HeightPixels);
    }

    return bounds;
}

CVM_RESULT CATVideoMode::SetDisplayMode(    CVMInt32   display,
                                            CVMInt32   width,
                                            CVMInt32   height,
                                            CVMInt32   bitsPerPixel,
                                            CVMInt32   refreshRate,
                                            CVM_ORIENT orient,
                                            bool       queueSet)                                          
{
    CVM_INFO info;
    memset(&info,0,sizeof(CVM_INFO));
    info.DisplayNumber = display;
    info.WidthPixels   = width;
    info.HeightPixels  = height;
    info.RefreshRateHz = refreshRate;
    info.Orientation   = orient;
    info.BitsPerPixel  = bitsPerPixel;
    
    return SetDisplayMode(display,&info,queueSet);
}


CVMCARDTYPE CATVideoMode::CheckVendorId(const CVMWChar* devIdString)
{
    CVMCARDTYPE cardType = CVMCARD_UNKNOWN;

    CVMWChar* start = (CVMWChar*)devIdString;
    CVMWChar* end   = 0;

    CVMUInt32 vendor = (CVMUInt32)-1;

    do
    {
        while ( (*start != 0) && (*start != '\\') && (*start != '&'))
            start++;

        if (*start == 0)
            return cardType;
        
        if ((start[1] == 'V') &&
            (start[2] == 'E') &&
            (start[3] == 'N') &&
            (start[4] == '_'))
        {
            // found
            start += 5;
            vendor = wcstoul(start,&end,16);
            break;
        }        
    } while (*start != 0);

    switch (vendor)
    {
        case 0x10de:    cardType = CVMCARD_NVIDIA;  break;
        case 0x1002:    cardType = CVMCARD_ATI;     break;
        case 0x102b:    cardType = CVMCARD_MATROX;  break;
        case 0x8086:    cardType = CVMCARD_INTEL;   break;
        case 0x1106:    cardType = CVMCARD_VIA;     break;
        case 0x5333:    cardType = CVMCARD_S3;      break;        
    }

    return cardType;
}

const CVMWChar* CATVideoMode::CardBrandToString(CVMCARDTYPE cardType)
{
    switch (cardType)
    {
        case CVMCARD_GENERIC: return L"GENERIC";
        case CVMCARD_NVIDIA:  return L"NVIDIA";
        case CVMCARD_ATI:     return L"ATI";
        case CVMCARD_MATROX:  return L"MATROX";
        case CVMCARD_INTEL:   return L"INTEL"; 
        
        default:
        case CVMCARD_UNKNOWN:
            return L"UNKNOWN";
    }    
}
