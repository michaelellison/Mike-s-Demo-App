/// \file    CATString.cpp
/// \brief   String class
/// \ingroup CAT
///
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-06-02 02:47:03 -0500 (Thu, 02 Jun 2011) $
// $Revision: 7 $
// $NoKeywords: $


#include "CATString.h"
#include <stdlib.h>
    
//---------------------------------------------------------------------------
CATString::CATString()
{
    Init();
}
//---------------------------------------------------------------------------
CATString::CATString(const char* str)
{	   
    Init();
    *this = str;
}
//---------------------------------------------------------------------------
CATString::CATString(const CATString& str)
{
    if (&str == this)
        return;
    Init();   
    *this = str;
}
//---------------------------------------------------------------------------
CATString::CATString(const CATWChar* str)
{
    Init();
    *this = str;
}
//---------------------------------------------------------------------------
// Destructor
CATString::~CATString()
{
    delete [] fBuffer;
    delete [] fUnicodeBuffer;
}
//---------------------------------------------------------------------------
// Creates an empty string of specified length 
bool CATString::Create(CATUInt32 length)
{
    Destroy();

    if (!AllocBuffer(length))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
CATUInt32 CATString::Length()
{
    // If it isn't dirty, no need to recalc
    if (!fLenDirty)
    {
        return fStrLen;
    }

    fStrLen = this->LengthCalc();
    fLenDirty = false;

    return fStrLen;
}
//---------------------------------------------------------------------------
CATUInt32 CATString::LengthCalc() const
{
    CATASSERT( (fAsciiLocked == false) || (fUnicodeLocked == false), "Either ascii or unicode should be unlocked...");

    if (this->fAsciiLocked)
    {
        if (fBuffer == 0)
            return 0;

        return (CATUInt32)strlen(fBuffer);
    }

    if (fUnicodeBuffer == 0)
        return 0;

    return (CATUInt32)wcslen(fUnicodeBuffer);
}

//---------------------------------------------------------------------------
char* CATString::GetAsciiBuffer(CATUInt32 minlength)
{
    CATASSERT(fUnicodeLocked == false, "Can't get an ascii buffer while the unicode buffer is locked");

    CATUInt32 bufSize = CATMax(minlength, this->Length());

    if (bufSize == 0)
        bufSize = 1;

    if (bufSize > fBufferLength)
    {
        this->ExpandBuffer(bufSize+1);
    }

    // Create ascii buffer
    this->AsciiFromUnicode();

    this->fBufferSizeLocked = true;
    this->fAsciiLocked = true;

    return fBuffer;
}
//---------------------------------------------------------------------------
CATWChar* CATString::GetUnicodeBuffer(CATUInt32 minlength)
{
    CATASSERT(fAsciiLocked == false, "Can't get a unicode buffer while the ascii buffer is locked");

    CATUInt32 bufSize = CATMax(minlength, this->Length());

    if (bufSize > fBufferLength)
    {
        this->ExpandBuffer(bufSize+1);
    }

    this->fBufferSizeLocked = true;
    this->fUnicodeLocked = true;

    return fUnicodeBuffer;
}

//---------------------------------------------------------------------------
void CATString::ReleaseBuffer()
{
    // Should shrink the buffer here, but we don't yet.	
    this->fBufferSizeLocked = false;		
    this->fUnicodeLocked = false;	

    if (fAsciiLocked)
    {
        this->fAsciiLocked = false;
        this->UnicodeFromAscii();
    }

    fLenDirty = true;
}
//---------------------------------------------------------------------------
CATInt32 CATString::Compare(const CATString& str, CATUInt32 cmpLen, CATUInt32 offset) const
{    
    // Check for self.
    if (&str == this)
        return 0;

    CATInt32 retVal = 0;

    CATASSERT((fAsciiLocked == false) && (str.fAsciiLocked == false),				
        "Not supporting locked ascii strings for compares currently");

    CATASSERT((offset == 0) || (offset < this->LengthCalc()),"offset beyond length of string." );	

    // Bail if either string is 0....
    // not a good way to indicate failure... hmmm...
    if (fUnicodeBuffer == 0)
    {
        return -1;
    }

    if (str.fUnicodeBuffer == 0)
    {
        return 1;
    }

    // Return equal if both are empty...
    if (this->IsEmpty() && str.IsEmpty())
    {
        return 0;
    }

    // Bail if offset is passed length or greater (this also bails on 0 strings)
    if (offset >= this->LengthCalc())
    {
        return -1;
    }

    CATUInt32 i = 0;

    // Scan strings for differences. Quit when difference is found or when string
    // terminator (0) is found.
    while ((fUnicodeBuffer[i+offset] != 0) && (str.fUnicodeBuffer[i] != 0) && (retVal == 0) && ((cmpLen == 0) || (i < cmpLen)))
    {
        if (fUnicodeBuffer[i+offset] == str.fUnicodeBuffer[i])
        {
            i++;
        }
        else
        {
            if (fUnicodeBuffer[i+offset] < str.fUnicodeBuffer[i])
            {
                retVal = -1;
            }
            else
            {
                retVal = 1;
            }				
        }
    }

    // Check for longer strings - shorter string is <
    if ((retVal == 0) && ((cmpLen == 0) || (i < cmpLen)))
    {
        if (fUnicodeBuffer[i+offset] != 0)
        {
            retVal = 1;
        }
        else if (str.fUnicodeBuffer[i] != 0)
        {
            retVal = -1;
        }
    }

    return retVal;
}
//---------------------------------------------------------------------------
// Ignores case - 
// WARNING: currently only supports english char sets (even though it supports unicode)
//          so lowercase/capital non-english chars are still mismatched. 
//          example: an 'a' with an accent will not match an 'A' with an accent...
//          This needs to be fixed.
//
CATInt32 CATString::CompareNoCase(const CATString& str,CATUInt32 cmpLen, CATUInt32 offset) const
{
    // Check for self.
    if (&str == this)
        return 0;

    CATInt32 retVal = 0;

    CATASSERT((fAsciiLocked == false) && (str.fAsciiLocked == false),				
        "Not supporting locked or dirty ascii strings for compares currently");

    CATASSERT((offset == 0) || (offset < this->LengthCalc()),"offset beyond length of string." );	

    // Bail if either is locked... or either string is 0....
    // not a good way to indicate failure... hmmm...
    if (fUnicodeBuffer == 0)
    {
        return -1;
    }

    if (str.fUnicodeBuffer == 0)
    {
        return 1;
    }

    // Bail if offset is passed length
    if (offset >= this->LengthCalc())
    {
        return -1;
    }

    CATUInt32 i = 0;

    // Scan strings for differences. Quit when difference is found or when string
    // terminator (0) is found.
    while ((fUnicodeBuffer[i+offset] != 0) && (str.fUnicodeBuffer[i] != 0) && (retVal == 0) && ((cmpLen == 0) || (i < cmpLen)))
    {
        CATWChar c1, c2;
        c1 = fUnicodeBuffer[i+offset];
        c2 = str.fUnicodeBuffer[i];

        // Convert both to lower case
        if ((c1 >= (CATWChar)'A') && (c1 <= (CATWChar)'Z'))
        {
            c1 ^= 0x0020;
        }

        if ((c2 >= (CATWChar)'A') && (c2 <= (CATWChar)'Z'))
        {
            c2 ^= 0x0020;
        }

        if (c1 == c2)
        {
            i++;
        }
        else
        {
            if (c1 < c2)
            {
                retVal = -1;
            }
            else
            {
                retVal = 1;
            }				
        }
    }

    // Check for longer strings - shorter string is <
    if ((retVal == 0) && ((cmpLen == 0) || (i < cmpLen)))
    {
        if (fUnicodeBuffer[i+offset] != 0)
        {
            retVal = 1;
        }
        else if (str.fUnicodeBuffer[i] != 0)
        {
            retVal = -1;
        }
    }

    return retVal;
}
//---------------------------------------------------------------------------
// Finds a substring within the string
// Starts looking at the offset passed in. Returns offset found (if found) in offset as well.
// returns true if found, or false otherwise.
// NOTE: brute force right now.  should be migrated to a more efficient algorithm
// if used for large strings.
bool CATString::Find(const CATString& str, CATUInt32& offset) const
{
    CATASSERT((fAsciiLocked == false) && (str.fAsciiLocked == false),				
        "Not supporting locked or dirty strings for searches currently");

    bool found = false;

    CATUInt32 i = 0;
    CATUInt32 j = 0;

    CATUInt32 len;
    CATUInt32 patlen;

    if (this->fLenDirty)
    {
        len = this->LengthCalc();
    }
    else
    {
        len = this->fStrLen;
    }

    if (str.fLenDirty)
    {
        patlen = str.LengthCalc();
    }
    else
    {
        patlen = str.fStrLen;
    }


    len -= offset;

    // Precalc as much of the pointers as possible
    CATWChar* data = this->fUnicodeBuffer + offset;
    CATWChar* pattern = str.fUnicodeBuffer;

    if (len > patlen)
    {
        while (i <= len - patlen)
        {
            if (data[i+j] == pattern[j])
            {
                j++;
            }
            else
            {
                j = 0;
                i++;
            }

            if (j == patlen)
            {
                found = true;
                break;
            }
        }
    }

    if (found)
    {
        offset += i;
    }

    return found;
}

//---------------------------------------------------------------------------
bool CATString::Find(CATWChar theChar, CATUInt32& offset) const
{
    CATASSERT((fAsciiLocked == false),
        "Not supporting locked or dirty strings for searches currently");

    bool found = false;

    CATUInt32 i = 0;	
    CATUInt32 len = this->LengthCalc();	
    len -= offset;

    while ( (i < len) && (!found))
    {
        if (this->fUnicodeBuffer[i+offset] == theChar)
        {
            found = true;
        }
        else
        {
            i++;
        }
    }

    if (found)
    {
        offset += i;
    }

    return found;
}


//---------------------------------------------------------------------------
bool CATString::ReverseFind(const CATString& str, CATUInt32& offset) const
{
    CATASSERT((fAsciiLocked == false) && (str.fAsciiLocked == false),				
        "Not supporting locked or dirty strings for searches currently");

    bool found = false;

    CATUInt32 i = 0;
    CATUInt32 j = 0;

    CATUInt32 len;
    CATUInt32 patlen;
    CATUInt32 locoffset = offset;

    if (this->fLenDirty)
    {
        len = this->LengthCalc();
    }
    else
    {
        len = this->fStrLen;
    }

    if (str.fLenDirty)
    {
        patlen = str.LengthCalc();
    }
    else
    {
        patlen = str.fStrLen;
    }

    if (locoffset == -1)
    {
        locoffset = len - patlen;
    }

    // i is the pointer to where we are in the string
    i = locoffset;
    if (i + patlen > len)
    {
        i = len - patlen;
    }	

    // Precalc as much of the pointers as possible
    CATWChar* data = this->fUnicodeBuffer;
    CATWChar* pattern = str.fUnicodeBuffer;

    if (len > patlen)
    {
        for ( ; ; )
        {
            if (data[i+j] == pattern[j])
            {
                j++;
            }
            else
            {
                j = 0;
                // Bail if we've gotten to the start of the string w/o finding a match
                if (i == 0)
                    break;

                i--;
            }

            if (j == patlen)
            {
                found = true;
                break;
            }
        }
    }

    if (found)
    {
        offset = i;
    }

    return found;
}

//---------------------------------------------------------------------------
bool CATString::ReverseFind(CATWChar theChar, CATUInt32& offset) const
{
    CATASSERT((fAsciiLocked == false),
        "Not supporting locked or dirty strings for searches currently");

    bool found = false;

    CATUInt32 i = 0;	
    CATUInt32 len = this->GetLength(fUnicodeBuffer);	


    CATUInt32 locoffset = offset;
    if (locoffset == -1)
    {
        locoffset = len - 1;
    }

    i = locoffset;

    while (!found)
    {
        if (this->fUnicodeBuffer[i] == theChar)
        {
            found = true;
        }
        else
        {
            if (i == 0)
            {
                // bail if we hit the beginning
                break;
            }
            i--;
        }
    }

    if (found)
    {
        offset = i;
    }

    return found;
}

//---------------------------------------------------------------------------
// Creates a string up to the specified character
CATString CATString::Left(CATUInt32 maxlength) const
{
    CATString newString;

    CATUInt32 newLen = CATMin(maxlength,this->LengthCalc());
    newString.Create( newLen + 1);
    this->CopyBuffer(newString.fUnicodeBuffer, this->fUnicodeBuffer, newLen);

    newString.fLenDirty = true;
    return newString;
}
//---------------------------------------------------------------------------
// Creates a string starting at the specified location
// If [start] is passed the end of the current string, the resulting
// string will be empty.
CATString CATString::Right(CATUInt32 start) const
{	
    CATString newString;

    if (start >= this->LengthCalc())
    {
        return newString;
    }

    CATUInt32 length = this->LengthCalc() - start;
    newString.Create(length+1);
    this->CopyBuffer(newString.fUnicodeBuffer, this->fUnicodeBuffer + start, length);	

    newString.fLenDirty = true;
    return newString;
}
//---------------------------------------------------------------------------
CATString CATString::FromRight(CATUInt32 length) const
{
    return Right(LengthCalc() - length);
}
//---------------------------------------------------------------------------
// Creates a substring of [length] size starting at [start]
CATString CATString::Sub(CATUInt32 start, CATUInt32 length) const
{
    CATString newString;

    CATUInt32 actualLength;

    if (start >= this->LengthCalc())
    {
        return newString;
    }

    actualLength = CATMin(length, this->LengthCalc() - start);
    newString.Create(length);
    CopyBuffer(newString.fUnicodeBuffer, this->fUnicodeBuffer + start, length);

    newString.fLenDirty = true;

    return newString;
}
//--------------------------------------------------------------------------
// Numeric conversions
//--------------------------------------------------------------------------
CATString::CATString(CATUInt32 val)
{
    Init();
    *this = val;
}
//---------------------------------------------------------------------------
CATString::CATString(CATInt32 val)
{
    Init();
    *this = val;
}
//---------------------------------------------------------------------------
CATString::CATString(CATFloat32 val)
{
    Init();
    *this = val;
}

//---------------------------------------------------------------------------
CATString::CATString(CATFloat64 val)
{
    Init();
    *this = val;
}

//---------------------------------------------------------------------------
CATString::CATString(bool val)
{
    Init();
    *this = val;
}

//---------------------------------------------------------------------------
CATString::CATString(char val)
{
    Init();
    *this = val;
}

//---------------------------------------------------------------------------
CATString::CATString(CATWChar val)
{
    Init();
    *this = val;
}

#ifdef CAT_CONFIG_WIN32
CATString::CATString(const GUID& guid)
{
    Init();
    *this = guid;
}

CATString::CATString(const GUID* guid)
{
    Init();
    *this = guid;
}
#endif

//---------------------------------------------------------------------------
CATString& CATString::operator=(CATFloat32 val)
{
    CATWChar tmpBuf[256];
    xplat_swprintf(tmpBuf,256,L"%.5f",val);
    

    int lastByte = (int)(wcslen(tmpBuf) - 1);
    while ((lastByte > 0) && (tmpBuf[lastByte] == '0') && (tmpBuf[lastByte - 1] != '.'))
    {
        tmpBuf[lastByte] = 0;
        lastByte--;
    }

    *this = tmpBuf;	
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator=(CATFloat64 val)
{
    CATWChar tmpBuf[512];
    xplat_swprintf(tmpBuf,512,L"%.5f",val);

    int lastByte = (int)(wcslen(tmpBuf) - 1);
    while ((lastByte > 0) && (tmpBuf[lastByte] == '0') && (tmpBuf[lastByte - 1] != '.'))
    {
        tmpBuf[lastByte] = 0;
        lastByte--;
    }

    *this = tmpBuf;	
    return *this;
}

#ifdef CAT_CONFIG_WIN32
CATString& CATString::operator=(const GUID& guid)
{
    CATWChar tmpBuf[512];   
    xplat_swprintf(tmpBuf,512,L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        guid.Data1, guid.Data2,guid.Data3,
        guid.Data4[0], 
        guid.Data4[1], 
        guid.Data4[2], 
        guid.Data4[3], 
        guid.Data4[4], 
        guid.Data4[5], 
        guid.Data4[6], 
        guid.Data4[7]
        );
    *this = tmpBuf;
    return *this;
}

CATString& CATString::operator=(const GUID* guid)
{
    *this = (*guid);
    return *this;
}
#endif 
//---------------------------------------------------------------------------
CATString& CATString::operator=(CATUInt32 val)
{
    CATWChar tmpBuf[32];
    xplat_swprintf(tmpBuf,32,L"%u",val);
    *this = tmpBuf;	
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator=(bool val)
{

    if (val)
    {
        *this = (const CATWChar*)L"True";
    }
    else
    {
        *this = (const CATWChar*)L"False";
    }

    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator=(char val)
{	
    char tmpBuf[2];
    tmpBuf[0] = val;
    tmpBuf[1] = 0;	
    *this = tmpBuf;	
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator=(CATWChar val)
{	
    CATWChar tmpBuf[2];
    tmpBuf[0] = val;
    tmpBuf[1] = 0;	
    *this = tmpBuf;	
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator=(CATInt32 val)
{		
    this->ExpandBuffer(33);
    xplat_swprintf(fUnicodeBuffer,33,L"%d",val);
    this->fLenDirty = true;
    return *this;
}

//---------------------------------------------------------------------------
CATString::operator CATInt32() const
{
    if (fUnicodeBuffer == 0)
        return 0;

    if ((fUnicodeBuffer[0] == '0') && ((fUnicodeBuffer[1] == 'x')|| (fUnicodeBuffer[1] == 'X')))
    {
        return (CATInt32)this->FromHex();
    }

    CATWChar* endPtr = 0;
	return wcstol(fUnicodeBuffer,&endPtr,10);
}


CATString::operator CATInt64() const
{
    CATWChar* endPtr = 0;

    if (fUnicodeBuffer == 0)
        return 0;

    if ((fUnicodeBuffer[0] == '0') && ((fUnicodeBuffer[1] == 'x')|| (fUnicodeBuffer[1] == 'X')))
    {
        return (CATInt64)xplat_wcstoull(fUnicodeBuffer,&endPtr,16);
    }
    
	return (CATInt64)xplat_wcstoull(fUnicodeBuffer,&endPtr,10);    
}
//---------------------------------------------------------------------------
CATString::operator bool() const
{
    if (fUnicodeBuffer == 0)
        return false;

    CATWChar firstChar = this->GetWChar(0);

    // Scan for true/false and yes/no
    switch (firstChar)
    {
        // [T]rue / [Y]es
    case 'T':
    case 't':
    case 'y':
    case 'Y':      
        return true;
    case 'F':
    case 'f':
    case 'n':
    case 'N':      
        return false;
    }

    // true numerics
    if ((CATInt32)*this)
    {
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------
CATString::operator CATFloat32() const
{
    if (fUnicodeBuffer == 0)
        return 0;

    return  (CATFloat32)xplat_wtof(fUnicodeBuffer);	
}

//---------------------------------------------------------------------------
CATString::operator CATFloat64() const
{
    if (fUnicodeBuffer == 0)
        return 0;
    
	return xplat_wtof(fUnicodeBuffer);    
}

//---------------------------------------------------------------------------
CATString::operator CATUInt32() const
{
    if (fUnicodeBuffer == 0)
        return 0;

    if ((fUnicodeBuffer[0] == '0') && ((fUnicodeBuffer[1] == 'x')|| (fUnicodeBuffer[1] == 'X')))
    {
        return this->FromHex();
    }
    
    CATWChar* endPtr = 0;
	return wcstoul(fUnicodeBuffer,&endPtr,10);        
}
//--------------------------------------------------------------------------
// Operators
//--------------------------------------------------------------------------
CATString& CATString::operator=(const CATString& str)
{
    if (&str == this)
        return *this;

    if (str.IsEmpty())
    {
        *this = "";
        return *this;
    }

    CATASSERT((str.fUnicodeLocked != true) && (str.fAsciiLocked != true), "Can't copy locked strings right now");
    Destroy();

    CATUInt32 newlen = str.LengthCalc();

    this->Create(newlen+1);
    this->CopyBuffer(this->fUnicodeBuffer,str.fUnicodeBuffer,newlen);		

    fLenDirty = true;

    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator=(const CATWChar* unistr)
{
    Destroy();
    if (unistr == 0)
        return *this;

    CATUInt32 newlen = (CATUInt32)(wcslen(unistr));
    this->AllocBuffer(newlen+1);

    if (fUnicodeBuffer == 0)
    {
        CATASSERT(fUnicodeBuffer != 0,"Make sure we could allocate it");	
        throw;
    }

    this->CopyBuffer(this->fUnicodeBuffer,unistr,newlen);

    fLenDirty = true;
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator=(const char* asciistr)
{
    Destroy();	
    this->ImportAscii(asciistr);	
    return *this;
}

//---------------------------------------------------------------------------
CATString::operator const char *() const
{
    CATASSERT(fUnicodeLocked != true,"Getting pointer to ascii while unicode is locked - dangerous.");	
    this->AsciiFromUnicode();
    return fBuffer;
}

//---------------------------------------------------------------------------
CATString::operator const CATWChar*() const
{
    CATASSERT(fAsciiLocked != true,"Getting pointer to unicode while ascii is locked - dangerous.");
    return fUnicodeBuffer;
}


//---------------------------------------------------------------------------
CATString& CATString::operator+=(const CATString& str)
{	
    CATASSERT(this->fBufferSizeLocked != true, "Performing operations that modify the string length after locking buffer with GetBuffer() is dangerous. Use ReleaseBuffer first!");
    // Check for self
    if (this == &str)
    {
        CATString temp = str + str;
        *this = temp;
        return *this;
    }


    CATUInt32 newLength = Length();

    // If we can avoid recalc'ing w/o using a non-const function of the string,
    // then do it to save time...
    if (str.fLenDirty)
    {
        newLength += str.LengthCalc() + 1;
    }
    else
    {
        newLength += str.fStrLen + 1;
    }

    if (fUnicodeBuffer == 0)
    {
        this->AllocBuffer(newLength);
    }
    else
    {
        this->ExpandBuffer(newLength);	
    }

    CopyBuffer(this->fUnicodeBuffer + this->Length(),str.fUnicodeBuffer,newLength);

    fStrLen	 = newLength-1;
    fLenDirty = false;	
    return *this;
}


CATUInt32 GetUtf8CharLen(const char* utf8)
{
    if (!utf8)
        return 0;

    CATUInt8 utf8Char = (CATUInt8)*utf8;
    if (!(utf8Char & 0x80))         return 1;
    if ((utf8Char & 0xe0) == 0xc0)  return 2;
    if ((utf8Char & 0xf0) == 0xe0)  return 3;
    if ((utf8Char & 0xf8) == 0xf0)  return 4;
    if ((utf8Char & 0xfc) == 0xf8)  return 5;
    if ((utf8Char & 0xfe) == 0xfc)  return 6;

    return 0;
}    

CATWChar Utf8ToUcs2Char(const char* utf8, CATUInt32& retLen)
{
    if (!utf8)
        return 0;
    if (!(retLen = GetUtf8CharLen(utf8)))
        return 0;
        
    CATUInt32 ucs2Char = 0;
    switch (retLen)
    {
        case 1: 
            ucs2Char = (CATUInt32)*utf8;
            break;
        case 2: 
            ucs2Char = ((CATUInt32)(*(utf8+1)) & 0x7f) 
                       + ((((CATUInt32)*utf8) & 0x1f) << 6);
            break;
        case 3: 
            ucs2Char = ((CATUInt32)(*(utf8+2)) & 0x7f)
                       + (((CATUInt32)(*(utf8+1)) & 0x7f) << 6) 
                       + ((((CATUInt32)*utf8) & 0x0f) << 12);
            break;
    }
   return (CATWChar)ucs2Char;
}

void Utf8ToUcs2(const char* utf8s, CATWChar* ucs2, CATUInt32 bufChars)
{
	if (!ucs2)
		return;

	if (!utf8s)
	{
        *ucs2 = 0;
		return;
	}

   CATUInt32 len = (CATUInt32)-1;
   CATWChar* endBuf = ucs2 + (bufChars-1);
   while ((*utf8s != 0) && (ucs2 < endBuf) && (len != 0))
   {
      *ucs2 = Utf8ToUcs2Char(utf8s,len);
      utf8s += len;
      ucs2++;
   }
   *ucs2 = 0;
}

char* Ucs2ToUtf8Char(CATWChar ucs2Char, char* dest)
{
	if (!dest)
		return 0;

   char* curPos = dest;

   if (ucs2Char <= 0x7f)
   {
      *curPos = (char)ucs2Char;
      curPos++;
      return curPos;
   }

   if (ucs2Char <= 0x7ff)
   {
      *(curPos+1) = (char)(ucs2Char & 0x3f) | 0x80;
      *(curPos)   = (char)((ucs2Char >> 6)  & 0x1f) | 0xC0;
      return curPos+2;
   }

   if (ucs2Char <= 0xffff)
   {
      *(curPos+2) = (char)(ucs2Char & 0x3f) | 0x80;
      *(curPos+1) = (char)((ucs2Char >> 6 ) & 0x3f) | 0x80;
      *(curPos)   = (char)((ucs2Char >> 12) & 0x0f) | 0xE0;
      return curPos+3;
   }

   *(curPos = 0);
   return 0;
}


void Ucs2ToUtf8(const CATWChar* ucs2, char* utf8s, CATUInt32 bufLen)
{
    if (!utf8s)
        return;
    if (!ucs2)
    {
        *utf8s = 0;
        return;
    }

   char* endBuf = utf8s + bufLen; 
   while ((*ucs2 != 0) && (utf8s < endBuf) && (utf8s != 0))
   {
      utf8s = Ucs2ToUtf8Char(*ucs2,utf8s);
      ucs2++;
   };
   *utf8s = 0;
}


//--------------------------------------------------------------------------
// Protected functions
//--------------------------------------------------------------------------
bool CATString::AsciiFromUnicode() const
{
    if (fBuffer)
    {
        delete [] fBuffer;
        fBuffer = 0;
    }
    // should measure this first...
    CATUInt32 maxLen = (CATUInt32)wcslen(fUnicodeBuffer)*3;
    fBuffer = new char[CATMax(maxLen,this->fBufferLength*3)+1];
    Ucs2ToUtf8(fUnicodeBuffer,fBuffer,maxLen);
    
    return true;
}

bool CATString::UnicodeFromAscii()
{
    if (fBuffer == 0)
    {
        if (fUnicodeBuffer)
        {
            delete [] fUnicodeBuffer;
            fUnicodeBuffer = 0;
        }
        fBufferLength = 0;
        fStrLen = 0;
        fLenDirty = 0;
        return true;
    }

    // should measure this first...
    CATUInt32 maxLen = (CATUInt32)strlen(fBuffer)+1;
    fUnicodeBuffer = new CATWChar[maxLen+1];
    Utf8ToUcs2(fBuffer,fUnicodeBuffer,maxLen);
    
    this->fLenDirty = true;	
    return true;
}

bool CATString::ImportAscii(const char* ascii)
{
    if (ascii == 0)
    {
        if (fUnicodeBuffer)
            delete [] fUnicodeBuffer;
        fBufferLength = 0;
        fStrLen = 0;
        fLenDirty = 0;
        return true;
    }

    if (fBuffer)
    {
        delete [] fBuffer;
        fBuffer = 0;
    }
    if (fUnicodeBuffer)
    {
        delete [] fUnicodeBuffer;
        fUnicodeBuffer = 0;
    }


    CATUInt32 maxLen = (CATUInt32)strlen(ascii)+1;
	 AllocBuffer(maxLen);
	 Utf8ToUcs2(ascii,fUnicodeBuffer,maxLen);
    AsciiFromUnicode();
    
    this->fLenDirty = true;	
    return true;
}



//---------------------------------------------------------------------------
bool CATString::AllocBuffer(CATUInt32 minLength)
{
    // This should only be called when fBuffer is 0 and the string is
    // not locked.
    CATASSERT(!fBufferSizeLocked,"Buffer size shouldn't be locked here...");	

    if ((fBufferSizeLocked == true) || (fUnicodeBuffer != 0) )
    {
        return false;
    }

    if (fBuffer)
    {
        delete [] fBuffer;
        fBuffer = 0;
    }

    CATUInt32 realLength = 0;

    // Allocate a buffer with some space to spare if the string is < 1k
    // Always make sure there's enough room for a 0.
    if (minLength < 32)
    {
        realLength = 32;
    }
    else if (minLength < 64)
    {
        realLength = 64;
    }
    else if (minLength < 128)
    {
        realLength = 128;
    }
    else if (minLength < 256)
    {
        realLength = 256;
    }
    else if (minLength < 1024)
    {
        realLength = 1024;
    }
    else
    {
        // On first allocation, if the allocation is > 1k, allocate only what we need.
        realLength = minLength + 1;
    }

    fUnicodeBuffer = new CATWChar[realLength+1];
    memset(fUnicodeBuffer,0,realLength*sizeof(CATWChar));
    CATASSERT(fUnicodeBuffer != 0, "Got a null string buffer when we tried to allocate it.");
    if (fUnicodeBuffer == 0)
    {
        fBufferLength = 0;
        return false;
    }

    fBufferLength = realLength;	

    return true;
}

//---------------------------------------------------------------------------
bool CATString::ExpandBuffer(CATUInt32 minLength)
{
    // This should only be called when fUnicodeBuffer exists and the string
    // is not locked.

    CATASSERT(this->fBufferSizeLocked != true,"Always use ReleaseBuffer() before performing operations that could change the string size!!!");

    if (fBufferSizeLocked)
    {
        return false;
    }

    if (this->fUnicodeBuffer == 0)
    {
        return this->AllocBuffer(minLength);
    }

    // Decide on our minimum size - either minLength + 1, or our current size
    CATUInt32 realLength = this->Length() + 1;

    if (minLength >= realLength)
    {
        realLength = minLength + 1;
    }

    // If we've already got the amount requested or more, just exit 
    // with success
    if (realLength <= fBufferLength)
    {
        return true;
    }

    // Otherwise, allocate to nearest reasonable block size, or 
    // the current amount + 1k if > 1k.
    if (realLength < 16)
    {
        realLength = 16;
    }
    else if (realLength < 32)
    {
        realLength = 32;
    }
    else if (realLength < 64)
    {
        realLength = 64;
    }
    else if (realLength < 128)
    {
        realLength = 128;
    }
    else if (realLength < 256)
    {
        realLength = 256;
    }
    else if (realLength < 1024)
    {
        realLength = 1024;
    }
    else if (realLength < 1024*10)
    {	
        // If the requested length is > 1k, always allocate 1k extra so we're not constantly
        // reallocating the string...
        realLength = realLength + 1024;
    }
    else
    {
        realLength = realLength + 1024*10;
    }

    CATWChar *tmpBuf = fUnicodeBuffer;

    // Nuke ascii string - it's unreliable for conversions
    if (fBuffer != 0)
    {
        delete [] fBuffer;
        fBuffer = 0;
    }

    // Allocate new string
    fUnicodeBuffer = new CATWChar[realLength];
    CATASSERT(fUnicodeBuffer != 0, "Got a null buffer when reallocating a string");
    if (!fUnicodeBuffer)
    {
        return false;
    }
    memset(fUnicodeBuffer,0,realLength*sizeof(CATWChar));
    fBufferLength = realLength;

    // Copy in our new string
    CopyIn(tmpBuf);

    // Delete old string
    delete [] tmpBuf;

    fStrLen = (CATUInt32)(wcslen(fUnicodeBuffer));
    fLenDirty = false;	
    return true;
}

//---------------------------------------------------------------------------
void CATString::CopyIn(const CATWChar* str)
{	
    if (str == 0)
    {
        if (fUnicodeBuffer)
        {
            fUnicodeBuffer[0] = 0;
        }
        return;
    }

    CATUInt32 i = 0;

    while ((str[i] != 0) && (i < fBufferLength - 1))
    {
        fUnicodeBuffer[i] = str[i];
        i++;
    }

    fUnicodeBuffer[i] = 0;		


    fLenDirty = true;
}


//---------------------------------------------------------------------------
bool CATString::IsEmpty() const
{
    // If we're in ascii mode, then use ascii. otherwise, default to unicode
    if (this->fAsciiLocked)
    {
        if ((this->fBuffer == 0) || (this->fBuffer[0] == 0))
        {
            return true;
        }

        return false;
    }

    if ((this->fUnicodeBuffer == 0) || (fUnicodeBuffer[0] == 0))
    {
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------
CATString& CATString::AppendHex(CATUInt32 hexValue, bool addX)
{
    // 0x12345678
    if (addX)
        *this << L"0x";

    for (CATUInt32 i = 0; i < 8; i++)
    {	
        CATUInt32 shift = ((7 - i)*4);
        char nibble = (char)( ((hexValue & (0xf << shift)) >> shift) );
        if (nibble >= 0x0a)
        {
            nibble += 'A' - 0x0a;
        }
        else
        {
            nibble += '0';
        }

        *this << nibble;
    }
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::AppendHexByte(CATUInt8 hexValue, bool addX)
{	
    if (addX)
    {
        *this << L"0x";
    }
    char nibble = (char)((hexValue & 0xf0) >> 4);
    if (nibble >= 0x0a)
    {
        nibble += 'A' - 0x0a;
    }
    else
    {
        nibble += '0';
    }

    *this << nibble;

    nibble = (char)(hexValue & 0xf);
    if (nibble >= 0x0a)
    {
        nibble += 'A' - 0x0a;
    }
    else
    {
        nibble += '0';
    }

    *this << nibble;

    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator<<(const CATString& str)
{
    // str == this is okay, since += takes care of it.
    *this += str;
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator<<(const char* str)
{
    *this += str;
    return *this;
}


//---------------------------------------------------------------------------
CATString& CATString::operator<<(const CATWChar* str)
{
    *this += str;
    return *this;
}


//---------------------------------------------------------------------------
CATString& CATString::operator<<(char strChar)
{
    char tmpBuf[2];
    tmpBuf[1] = 0;
    tmpBuf[0] = strChar;
    *this += tmpBuf;
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator<<(CATWChar strChar)
{
    CATWChar tmpBuf[2];
    tmpBuf[1] = 0;
    tmpBuf[0] = strChar;
    *this += tmpBuf;
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator<<(CATUInt32 val)
{
    CATWChar tmpBuf[32];
    xplat_swprintf(tmpBuf,32,L"%u",val);
    *this += tmpBuf;
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator<<(CATInt32 val)
{
    CATWChar tmpBuf[32];
    xplat_swprintf(tmpBuf,32,L"%d",val);
    *this += tmpBuf;
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator<<(CATInt64 val)
{
    CATWChar tmpBuf[32];
#ifdef _WIN32
    xplat_swprintf(tmpBuf,32,L"%I64d",val);
#else
    xplat_swprintf(tmpBuf,32,L"%lld",val);
#endif
    *this += tmpBuf;
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator<<(CATFloat32 val)
{
    CATWChar tmpBuf[256];
    xplat_swprintf(tmpBuf,256,L"%.5f",val);

    CATInt32 lastByte = (CATInt32)(wcslen(tmpBuf) - 1);
    while ((lastByte > 0) && (tmpBuf[lastByte] == '0') && (tmpBuf[lastByte - 1] != '.'))
    {
        tmpBuf[lastByte] = 0;
        lastByte--;
    }

    *this += tmpBuf;
    return *this;
}

//---------------------------------------------------------------------------
CATString& CATString::operator<<(CATFloat64 val)
{
    CATWChar tmpBuf[512];
    xplat_swprintf(tmpBuf,512,L"%.5f",val);

    CATInt32 lastByte = (CATInt32)(wcslen(tmpBuf) - 1);
    while ((lastByte > 0) && (tmpBuf[lastByte] == '0') && (tmpBuf[lastByte - 1] != '.'))
    {
        tmpBuf[lastByte] = 0;
        lastByte--;
    }

    *this += tmpBuf;
    return *this;
}
#ifdef CAT_CONFIG_WIN32
CATString& CATString::operator<<(const GUID& guid)
{
    CATWChar tmpBuf[512];   
    xplat_swprintf(tmpBuf,512,L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        guid.Data1, guid.Data2,guid.Data3,
        guid.Data4[0], 
        guid.Data4[1], 
        guid.Data4[2], 
        guid.Data4[3], 
        guid.Data4[4], 
        guid.Data4[5], 
        guid.Data4[6], 
        guid.Data4[7]
        );
    *this += tmpBuf;
    return *this;
}

CATString& CATString::operator<<(const GUID* guid)
{
    *this << *guid;
    return *this;
}
#endif
//---------------------------------------------------------------------------
CATWChar CATString::GetWChar(CATUInt32 offset) const
{
    CATASSERT( (!fUnicodeLocked) && (!fAsciiLocked), "Unlock the string before playing with it.");
	 CATASSERT( offset < fBufferLength, "Offset is invalid.");
    // If offset is past the end of the buffer, then bail
    if (offset >= fBufferLength)
    {
        return 0;
    }
    // if offset is past the end of the string, bail
    if (offset > GetLength(fUnicodeBuffer))
    {
        return 0;
    }

    // return the wchar at the offset
    return fUnicodeBuffer[offset];
}

//---------------------------------------------------------------------------
bool CATString::SetWChar(CATUInt32 offset, CATWChar theChar)
{
    CATASSERT( (!fUnicodeLocked) && (!fAsciiLocked), "Unlock the string before playing with it.");
    if (fUnicodeLocked || fAsciiLocked)
    {
        return false;
    }

    CATASSERT( offset < fBufferLength, "Offset is invalid.");		
    if (offset >= fBufferLength)
    {
        return false;
    }

    fUnicodeBuffer[offset] = theChar;

    return true;
}

//---------------------------------------------------------------------------
// Pull Next Token...
//
// Finds the first instance of any of the characters in 'splitTokens'.
// Then, sets "token" to be everything to the left of that char.
// The string object becomes everything to the right of the char.
// If no more tokenizers remain in the string, then the command returns
// false and any remaining string is placed into 'token', and the string
// is left empty.
//
// example:
// CATString test = "Beer:Whiskey!Sex";
// CATString token;
//
// while (!test.IsEmpty())
// {
//		 if (!test.PullNextToken(token,":!"))
//         printf("(Last token....)\n");
//     printf("%s\n",(const char*)token);
// }
//
//
// Output:
//    Beer
//    Whiskey
//    (Last token....)
//    Sex
//
bool CATString::PullNextToken(CATString& token, const CATString& splitTokens)
{
    CATASSERT((fAsciiLocked == false),
        "Not supporting locked or dirty strings for searches currently");
    this->Trim();

    for (CATUInt32 i=0; i<this->fStrLen; i++)
    {
        for (CATUInt32 tIndex = 0; tIndex < splitTokens.LengthCalc(); tIndex++)
        {
            if (fUnicodeBuffer[i] == splitTokens.GetWChar(tIndex))
            {
                // Found a tokenizer.
                token = this->Left(i);
                *this = this->Right(i+1);
                token.Trim();
                this->Trim();
                return true;
            }
        }
    }

    // Didn't find a tokenizer, just return the whole string as token and clear this
    token = *this;
    token.Trim();
    *this = L"";	
    return false;
}


void CATString::Pad(CATUInt32 length, CATWChar theChar)
{
    CATUInt32 curLen = this->Length();

    if (curLen >= length)
        return;

    CATWChar* buffer = this->GetUnicodeBuffer(length+1);
    for (CATUInt32 i = curLen; i < length; i++)
    {
        buffer[i] = theChar;
    }
    buffer[length] = 0;
    this->ReleaseBuffer();
}
//---------------------------------------------------------------------------
// Trims whitespace chars off both ends of the string
void CATString::Trim()
{
    CATASSERT( (!fUnicodeLocked) && (!fAsciiLocked), "Unlock the string before playing with it.");	
    CATUInt32 start = 0;
    CATUInt32 end   = this->Length();

    // find starting point
    bool done = false;
    while ( (!done) && (start <= end))
    {
        switch (fUnicodeBuffer[start])
        {
        case 0x0a:
        case 0x0d:
        case 0x20:
        case 0x00:
        case 0x09:
            start++;
            break;
        default:
            done = true;
        }
    }

    done = false;
    while ( (!done) && (end >= start))
    {
        switch (fUnicodeBuffer[end])
        {
        case 0x0a:
        case 0x0d:
        case 0x20:
        case 0x00:
        case 0x09:
            end--;
            break;
        default:				
            done = true;
        }
    }

    // check for empty
    if ( end < start)
    {
        *this = L"";
        return;
    }

    // Trim the string
    *this = this->Sub(start, (end - start) + 1);
}

//------------------------------------------------------------------------------
// FromHex() converts from text to a CATUInt32
CATUInt32 CATString::FromHex() const
{
    CATUInt32 resultVal = 0;
    CATUInt32 start = 0;

    CATUInt32 length = this->LengthCalc();
    if (  (length > 2) &&
        (this->GetWChar(0) == '0') && 
        ((this->GetWChar(1) | (char)0x20) == 'x'))
    {
        start = 2;
    }


    for (CATUInt32 i = start; i < length; i++)
    {
        resultVal = resultVal << 4;
        CATUInt8 curChar = (CATUInt8)this->GetWChar(i);

        if ((curChar >= 'A') && (curChar <= 'F'))
        {
            resultVal |= ((curChar - 'A') + (CATUInt8)0x0a);
        }
        else if ((curChar >= 'a') && (curChar <= 'f'))
        {
            resultVal |= ((curChar - 'a') + (CATUInt8)0x0a);
        }
        else if ((curChar >= '0') && (curChar <= '9'))
        {
            resultVal |= (curChar - '0');
        }
        else
        {
            CATASSERT(false,"Invalid hex number.");
            return 0;
        }      
    }

    return resultVal;
}

//------------------------------------------------------------------------------
// ToUpper() converts the string to upper case
void CATString::ToUpper()
{
    CATUInt32 length = this->Length();
    CATWChar* buffer = this->GetUnicodeBuffer();
    for (CATUInt32 i = 0; i < length; i++)
    {
        if ((buffer[i] >= 'a') && (buffer[i] <= 'z'))      
        {
            buffer[i] &= ~((CATWChar)0x20);
        }
    }
    ReleaseBuffer();
}

//------------------------------------------------------------------------------
// ToLower() converts the string to upper case
void CATString::ToLower()
{
    CATUInt32 length = this->Length();
    CATWChar* buffer = this->GetUnicodeBuffer();
    for (CATUInt32 i = 0; i < length; i++)
    {
        if ((buffer[i] >= 'A') && (buffer[i] <= 'Z'))
        {
            buffer[i] |= (CATWChar)0x20;
        }
    }
    ReleaseBuffer();
}


const CATString& CATString::Format(const CATWChar* formatSpecs, ...)
{
    va_list fmtArgs;
    va_start(fmtArgs, formatSpecs);

    CATString testString;
    int testLen = 1024;
    CATWChar* testBuf = testString.GetUnicodeBuffer(testLen);

    // Win32 only, but hopefully *very* similar on other platforms!
    // Otherwise, will have to predetermine length with something like using the file handle version
    // to a null file (e.g. fsprintf -> /dev/null).
    int length = xplat_vsnwprintf(testBuf,testLen - 1, formatSpecs,fmtArgs);

    testString.ReleaseBuffer();

    while (length == -1)
    {
        testLen *= 2;
        testBuf = testString.GetUnicodeBuffer(testLen);
        length = xplat_vsnwprintf(testBuf,testLen - 1,formatSpecs,fmtArgs);
        testString.ReleaseBuffer();
    }

    va_end(fmtArgs);

    *this = testString;
    return *this;
}

const CATString& CATString::FormatArgs(const CATWChar* formatSpecs, va_list fmtArgs)
{
    CATString testString;
    int testLen = 1024;
    CATWChar* testBuf = testString.GetUnicodeBuffer(testLen);

    // Win32 only, but hopefully *very* similar on other platforms!
    // Otherwise, will have to predetermine length with something like using the file handle version
    // to a null file (e.g. fsprintf -> /dev/null).
    
    int length = xplat_vsnwprintf(testBuf,testLen - 1, formatSpecs,fmtArgs);
    testString.ReleaseBuffer();

    while (length == -1)
    {
        testLen *= 2;
        testBuf = testString.GetUnicodeBuffer(testLen);
        length = xplat_vsnwprintf(testBuf,testLen - 1,formatSpecs,fmtArgs);
        testString.ReleaseBuffer();
    }

    *this = testString;
    return *this;
}


CATUInt32 CATString::GetLength(const char* asciistr) const
{
    return (CATUInt32)strlen(asciistr);
}

CATUInt32 CATString::GetLength(const CATWChar* unistr) const
{
    return (CATUInt32)wcslen(unistr);
}

void CATString::CopyBuffer(char* str1, const char* str2, CATUInt32 length)
{
    if (str1 == 0 || str2 == 0)
    {
        return;
    }

    CATUInt32 i = 0;
    while ((str2[i] != 0) && (i < length))
    {
        str1[i] = str2[i];
        i++;
    }

    str1[i] = 0;
}


//---------------------------------------------------------------------------
void CATString::CopyBuffer(CATWChar* str1, const CATWChar* str2, CATUInt32 length)
{
    if (str1 == 0 || str2 == 0)
    {
        return;
    }

    CATUInt32 i = 0;
    while ((str2[i] != 0) && (i < length))
    {
        str1[i] = str2[i];
        i++;
    }

    str1[i] = 0;
}

CATString CATString::Escape() const
{
	CATString retString;
	CATUInt32 length = LengthCalc();
	CATUInt32 i;

	for (i = 0; i < length; i++)
	{
		CATWChar curChar = GetWChar(i);
		switch (curChar)
		{
            case 0x0a:
                retString << "&#x0a;";
                break;
            case 0x0d:
                retString << "&#x0d;";
                break;
            case 0x09:
                retString << "&#x09;";
                break;
            case '-':
                if (i < length - 1)
                {
                    if (GetWChar(i+1) == '-')
                    {
                        retString << "&#x2D;";
                    }
                    else
                    {
                        retString << curChar;
                    }
                }
                break;
			case '&':
				retString << "&amp;";
				break;
			case '<':
				retString << "&lt;";
				break;
			case '>':
				retString << "&gt;";
				break;
			case '\"':
				retString << "&quot;";
				break;
			case '\'':
				retString << "&apos;";
				break;
			default:
				retString << curChar;
				break;
		}
	}

	return retString;
}
#ifdef CAT_CONFIG_WIN32
bool CATString::GetGUID(GUID& guid) const
{
    CATASSERT( (fAsciiLocked == false) || (fUnicodeLocked == false), "Either ascii or unicode should be unlocked...");
    
    int numEntries = 0;
    GUID tmpGuid;

    if (this->LengthCalc() < 35)
        return false;

    int data0, data1,data2,data3,data4,data5,data6,data7;

    if (this->fAsciiLocked)
    {
        if (fBuffer == 0)
            return 0;
		
        numEntries = sscanf(fBuffer, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            &tmpGuid.Data1, 
            &tmpGuid.Data2, 
            &tmpGuid.Data3, 
            &data0,
            &data1,
            &data2,
            &data3,
            &data4,
            &data5,
            &data6,
            &data7);

        if (numEntries != 11)
        {
            numEntries = sscanf(fBuffer, "%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
                &tmpGuid.Data1, 
                &tmpGuid.Data2, 
                &tmpGuid.Data3, 
                &data0,
                &data1,
                &data2,
                &data3,
                &data4,
                &data5,
                &data6,
                &data7);
        }

    }
    else
    {
        if (fUnicodeBuffer == 0)
            return 0;
     
        numEntries = swscanf(fUnicodeBuffer, L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            &tmpGuid.Data1, 
            &tmpGuid.Data2, 
            &tmpGuid.Data3, 
            &data0,
            &data1,
            &data2,
            &data3,
            &data4,
            &data5,
            &data6,
            &data7);

        if (numEntries != 11)
        {
            numEntries = swscanf(fUnicodeBuffer, L"%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
                &tmpGuid.Data1, 
                &tmpGuid.Data2, 
                &tmpGuid.Data3, 
                &data0,
                &data1,
                &data2,
                &data3,
                &data4,
                &data5,
                &data6,
                &data7);
        }
    }

    if (numEntries == 11)
    {
        tmpGuid.Data4[0] = (BYTE)data0;
        tmpGuid.Data4[1] = (BYTE)data1;
        tmpGuid.Data4[2] = (BYTE)data2;
        tmpGuid.Data4[3] = (BYTE)data3;
        tmpGuid.Data4[4] = (BYTE)data4;
        tmpGuid.Data4[5] = (BYTE)data5;
        tmpGuid.Data4[6] = (BYTE)data6;
        tmpGuid.Data4[7] = (BYTE)data7;

        guid = tmpGuid;
        return true;
    }
    
    return false;
}
#endif

bool CATString::SplitPath(CATString* drive, CATString* path, CATString* filename, CATString* ext) const
{
    if (drive)      *drive    = L"";
    if (path)       *path     = L"";
    if (filename)   *filename = L"";   
    if (ext)        *ext      = L"";
    
    CATUInt32 curPos  = 0;
    CATUInt32 filePos = (CATUInt32)-1;
    CATUInt32 length = LengthCalc();
    
    if (length == 0)
        return false;

    if (length >= 2)
    {
        if (GetWChar(1) == CAT_DRIVESEPERATOR)
        {
            if (drive)
                (*drive) << GetWChar(0) << GetWChar(1);

            curPos = 2;
        }        
    }    

    CATUInt32 lastPath = (CATUInt32)-1;
    ReverseFind(CAT_PATHSEPERATOR,lastPath);    
    CATUInt32 lastPath2 = (CATUInt32)-1;
    ReverseFind(CAT_OPTPATHSEPERATOR,lastPath2);

    if (lastPath != (CATUInt32)-1)
        filePos = lastPath;
    if (lastPath2 != (CATUInt32)-1)
    {
        if (lastPath == -1)
            filePos = lastPath2;
        else if (lastPath2 > lastPath)
            filePos = lastPath2;
    }    
    
    if (filePos != -1)
    {
        if (path)
            (*path) = Sub(curPos,(filePos-curPos)+1);

        curPos = filePos + 1;
    }

    while (curPos < length)
    {
        if (GetWChar(curPos) != CAT_EXTSEPERATOR)
        {
            if (filename)
                (*filename) << GetWChar(curPos);
        }
        else
        {
            break;
        }
        curPos++;
    }

    if ((curPos < length) && (GetWChar(curPos) == CAT_EXTSEPERATOR))
    {
        while (curPos < length)
        {
            if (ext)
                (*ext) << GetWChar(curPos);
            curPos++;
        }
    }

    // Change all path seperators to the 'proper' one.
    if (path)
    {
        CATUInt32 pathLen = path->LengthCalc();
        for (CATUInt32 i = 0; i < pathLen; i++)
        {
            if ( path->GetWChar(i) == CAT_OPTPATHSEPERATOR)
            {
                path->SetWChar(i,CAT_PATHSEPERATOR);
            }
        }
    }


    return true;
}

CATString CATString::GetDriveDirectory() const
{
    CATString drive,directory;
    CATString path;
    if (this->SplitPath(&drive,&directory,0,0))
    {
        path << drive << directory;
    }
    return path;
}

CATString CATString::GetFilenameExt() const
{
    CATString filename,ext;
    CATString fullfile;
    if (this->SplitPath(0,0,&filename,&ext))
    {
        fullfile << filename << ext;
    }
    return fullfile;
}

CATString CATString::GetFilenameNoExt() const
{
    CATString filename;
    this->SplitPath(0,0,&filename,0);
    return filename;
}

// Escape a string for use in a URL.
CATString CATString::EncodeURL() const
{
	CATString retString;
	CATUInt32 length = LengthCalc();
	CATUInt32 i;

	for (i = 0; i < length; i++)
	{
		CATWChar curChar = GetWChar(i);
        if ((curChar >= 0x7f) || (curChar <= 0x20))
        {
            // Escape the char - out of range
            retString << L"%";
            retString.AppendHexByte((CATUInt8)curChar,false);
        }
        else
        {
		    switch (curChar)
		    {                                
                case 0x22: case 0x23: // " #
                case 0x24: case 0x25: // $ %
                case 0x26: case 0x2B: // & +
                case 0x2C: case 0x2F: // , /
                case 0x3A: case 0x3B: // : ;
                case 0x3C: case 0x3D: // < =
                case 0x3E: case 0x3F: // > ?
                case 0x40: case 0x5B: // @ [
                case 0x5C: case 0x5D: // \ ]
                case 0x5E: case 0x60: // ^ `
                case 0x7B: case 0x7C: // { |
                case 0x7D: case 0x7E: // } ~
                    retString << L"%";
                    retString.AppendHexByte((CATUInt8)curChar,false);
                    break;
                default:
				    retString << curChar;
				    break;
		    }
        }
	}
	return retString;
}

// Unescape a string from URL encoding.
CATString CATString::DecodeURL() const
{
	CATString retString;
	CATUInt32 length = LengthCalc();
	CATUInt32 i;

	for (i = 0; i < length; i++)
	{
		CATWChar curChar = GetWChar(i);        
        if (curChar == '%')
        {
            if (i < (length-2))
            {
                CATString hexByte;
                hexByte   << GetWChar(i+1) << GetWChar(i+2);
                retString << (char)hexByte.FromHex();
            }
            i+=2;
        }
        else
        {
            retString << curChar;
        }                              
    }
    return retString;
}

