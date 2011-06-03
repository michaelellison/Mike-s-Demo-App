//---------------------------------------------------------------------------
/// \file CATBrowser.cpp
/// \brief Browser GUI control
/// 
/// Copyright (c) 2003 by Michael Ellison
/// All Rights Reserved.
///
/// $Author: mike $
/// $Revision: 3 $
/// $RCSFile: $
/// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
//
//---------------------------------------------------------------------------
#include "CATBrowser.h"
#include "CATEventDefs.h"
#include "CATApp.h"

// Event handler
CATResult CATBrowser::OnEvent(const CATEvent& event, CATInt32& retVal)
{
   CATResult result = CAT_SUCCESS;

   // handle incoming browser events....
   // All of our events are targetted by fStringParam2 of the event.
   // IF it is empty or matches our control name, then we should respond.
   if ((event.fStringParam2.IsEmpty()) || (event.fStringParam2.Compare(this->fName) == 0))
   {
      switch (event.fEventCode)
      {
         case CATEVENT_BROWSER_REFRESH:
             result = this->Refresh();
             retVal++;
             return result;
    
         case CATEVENT_BROWSER_GO:
             result = this->Go(event.fStringParam1);
             retVal++;
             return result;
    
         case CATEVENT_BROWSER_STOP:
            result = this->Stop();
            retVal++;
            return result;    

         case CATEVENT_BROWSER_BACK:
            result = this->Back();
            retVal++;
            return result;
    
         case CATEVENT_BROWSER_FORWARD:
            result = this->Forward();
            retVal++;
            return result;
    
         case CATEVENT_BROWSER_ZOOM:
            {
               CATInt32 maxZ = this->GetZoomRange();
               CATInt32 minZ = 1;
               CATInt32 newZoom = (CATUInt32)(((maxZ - minZ) * event.fFloatParam1) + 0.5f) + minZ;

               if (newZoom != this->GetZoom())
               {
                  result = this->Zoom(newZoom);
               }
               retVal++;
               return result;
            }
            break;
         case CATEVENT_TAB_SHOW:
            {
               this->Show(true);
               this->MarkDirty();
               return result;
            }
            break;
   
         case CATEVENT_TAB_HIDE:
            {
               this->Show(false);
               this->MarkDirty();
               return result;
            }
            break;
      }
   }

   return CATControlWnd::OnEvent(event,retVal);
}

/// TranslateAttribs() parses the known attributes for an object.
CATResult CATBrowser::ParseAttributes()
{
	CATResult result = CATControlWnd::ParseAttributes();

	fDefaultURL = GetAttribute(L"URL");
	CATString attribVal;
	attribVal = GetAttribute(L"FileURL");
	if (!attribVal.IsEmpty())
		fDefaultURL.Format(L"file:///%s%s",(const char*)gApp->GetBaseDir(),(const char*)attribVal);

	return result;
}

void CATBrowser::PostDraw(CATDRAWCONTEXT drawContext, const CATRect& dirtyRect)
{
   
}

CATString CATBrowser::EscapeString( const CATString& inputStr)
{
   CATString escaped;
   CATUInt32 strLen = inputStr.LengthCalc();
   for (CATUInt32 i = 0; i < strLen; i++)
   {
      wchar_t curChar = inputStr.GetWChar(i);

      if (  ((curChar >= 'A') && (curChar <= 'Z')) ||
            ((curChar >= 'a') && (curChar <= 'z')) ||
            ((curChar >= '0') && (curChar <= '9')) ||
            (curChar == '-') ||
            (curChar == '.') ||
            (curChar == ',') ||
            (curChar == '*') ||
            (curChar == '_'))
      {
         escaped << curChar;
         continue;
      }
      
      if (curChar == ' ')
      {
         escaped << "+";
         continue;
      }

      escaped << L"%";
      escaped.AppendHexByte((unsigned char)curChar);
   }
   return escaped;
}