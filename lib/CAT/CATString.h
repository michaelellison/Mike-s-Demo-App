/// \file    CATString.h
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

#ifndef _CATString_H_
#define _CATString_H_

#include "CATInternal.h"

const CATWChar kCRLF[3] = { 0x0d, 0x0a, 0 };

/// \class CATString 
/// \brief String class that supports both char* and unicode/CATWChar types
/// \ingroup CAT
///
/// \todo UGH. Ok, ASCII funcs are renamed - it's just UTF-8 vs UTF-16. NOW,
/// the double-buffer needs to be killed. Badly. Rewrite.
///
class CATString
{
public:
    /// CATString() - Default constructor
    CATString();

    /// Copy constructor from existing string.
    /// \param str - String to copy into new CATString object
    CATString(const CATString& str);		

    /// Constructor using a normal ASCIIZ string
    /// \param str - ASCIIZ string to copy into new CATString object
    CATString(const char* str);

    /// Constructor using wide character null-terminated string
    /// \param str - Zero-terminated wide-char string to copy in
    CATString(const CATWChar* str);

    /// Constructor conversion from unsigned long value
    /// \param val - unsigned long value to convert to a string
    /// \sa AppendHex()
    CATString(CATUInt32 val);

    /// Constructor conversion from long value
    /// \param val - signed long value to convert to a string
    /// \sa AppendHex()
    CATString(CATInt32 val);      

    /// Constructor conversion from 32-bit float value
    /// \param val - floating point value to convert to a string
    CATString(CATFloat32 val);

    /// Constructor conversion from 64-bit float value
    /// \param val = floating point value to convert to a string
    CATString(CATFloat64 val);
#ifdef CAT_CONFIG_WIN32
    CATString(const GUID& guid);
    CATString(const GUID* guid);
#endif
    /// Constructor conversion from char
    /// \param val - char value to convert directly into a string
    CATString(char    val);

    /// Constructor conversion from a wide char
    /// \param val - wide char value to convert directly into a string
    CATString(CATWChar val);

    /// Constructor conversion from a bool
    /// \param val - boolean to convert to a "True" or "False" string
    CATString(bool val);

    /// Destructor
    ~CATString();		

    //--------------------------------------------------------------------
    // Operator overrides		
    //--------------------------------------------------------------------

    /// = override to copy in a CATString
    /// \param str - source CATString
    CATString& operator=(const CATString& str);

    /// = override to copy in an ASCIIZ string
    /// \param asciistr - ASCIIZ string source
    CATString& operator=(const char* asciistr);

    /// = override to copy in a zero-terminated wide char string
    /// \param unistr - Unicode string source
    CATString& operator=(const CATWChar* unistr);

    /// = override to copy in a character as a string
    /// \param val - char value
    CATString& operator=(char    val);

    /// = override to copy in a wide char as a string
    /// \param val - char value
    CATString& operator=(CATWChar val);

    /// = override to copy in a float value as a string
    /// \param val - floating point value
    CATString& operator=(CATFloat32 val);

    /// = override to copy in a large float value as a string
    /// \param val - floating point value
    CATString& operator=(CATFloat64 val);
#ifdef CAT_CONFIG_WIN32
    CATString& operator=(const GUID& guid);
    CATString& operator=(const GUID* guid);
#endif
    /// = override to copy in an unsigned integer value
    /// \param val - unsigned integer to convert to string
    CATString& operator=(CATUInt32 val);

    /// = override to copy in an integer value
    /// \param val - integer to convert to string
    CATString& operator=(CATInt32 val);

    /// = override to set string to True or False from a bool
    /// \param val - boolean value. String becomes "True" if true, "False"
    CATString& operator=(bool val);

    /// += appending override - appends a string to current
    /// \param str - string to append
    CATString& operator+=(const CATString& str);

    //--------------------------------------------------------------------
    // Add 'em as you need 'em, but don't make it 
    // dependant on other classes. Instead, provide 
    // an override to a char* from your class.

    /// << override - append a string to the current one
    /// \param str - string to append (CATString)
    CATString& operator<<(const CATString& str);

    /// << override - append a string to the current one
    /// \param str - string to append (ASCIIZ)
    CATString& operator<<(const char* str);	

    /// << override - append a string to the current one
    /// \param str - string to append (Unicode)
    CATString& operator<<(const CATWChar* str);

    /// << override - append a char to the current string
    /// \param strChar - char to append
    CATString& operator<<(char strChar);

    /// << override - append a wide char to the current string
    /// \param strChar - char to append
    ///
    /// WARNING: short's will be interpreted as wchar's in some
    ///          compilers!
    CATString& operator<<(const CATWChar strChar);  

    /// << override - append an unsigned long to the string
    /// \param val - value to convert to string and append
    CATString& operator<<(CATUInt32 val);

    /// << override - append a long to the string
    /// \param val - value to convert to string and append
    CATString& operator<<(CATInt32 val);

    /// << override - append a long to the string
    /// \param val - value to convert to string and append
    CATString& operator<<(CATInt64 val);

    /// << override - append a float to the string
    /// \param val - value to convert to string and append
    CATString& operator<<(CATFloat32 val);				

    /// << override - append a double to the string
    /// \param val - value to convert to string and append
    CATString& operator<<(CATFloat64 val);

    /// << override - append a hex guid (no {}'s)
    /// \param guid - guid to covert to string
#ifdef CAT_CONFIG_WIN32
    CATString& operator<<(const GUID& guid);
    CATString& operator<<(const GUID* guid);
#endif
    /// Const char* override - you may not modify the
    /// string this way, but you can use it for const 
    /// functions.  
    ///
    /// Be careful to specifically specify
    /// this overload for multi-argument functions like printf,
    /// otherwise you'll end up with garbage.
    ///
    /// \return const char* - pointer to string buffer in ASCIIZ format.
    /// \sa GetAsciiBuffer(), ReleaseBuffer()
    operator const char *() const;

    /// Const CATWChar* override - you may not modify the
    /// string this way, but you can use it for const 
    /// functions.  
    ///
    /// Be careful to specifically specify
    /// this overload for multi-argument functions like wprintf,
    /// otherwise you'll end up with garbage.
    ///
    /// \return const CATWChar* - pointer to string buffer in wide-char
    /// (Unicode) format.
    /// \sa GetUnicodeBuffer(), ReleaseBuffer()
    operator const CATWChar *() const;

    /// Convert the string to a long, if possible.
    operator CATInt32  () const;

    operator CATInt64 () const;

    /// Convert the string to a float, if possible.
    operator CATFloat32 () const;

    /// Convert the string to a float, if possible.
    operator CATFloat64 () const;

    /// Convert the string to an unsigned long, if possible.
    operator CATUInt32 () const;

    /// Convert the string to a bool, if possible.
    operator bool    () const;

    //--------------------------------------------------------------------
    // Standard string functions
    //--------------------------------------------------------------------


    /// GetWChar() retrieves the wide character at the specified offset 
    /// in the string.
    ///
    /// GetWChar is preferred to GetChar().
    ///
    /// \param offset - 0-based offset of character.
    /// \return CATWChar - character at offset or 0 if the offset is
    /// out of bounds (will also assert in debug in this case)
    /// \sa SetWChar()
    CATWChar	GetWChar(CATUInt32 offset) const;


    /// SetWChar() sets the character at the specified offset in the
    /// string.
    ///
    /// SetWChar() is preferred over SetChar().
    /// You may *not* set a character past the current length of the
    /// string or buffer size.
    ///
    /// \param offset - 0-based offset of character.
    /// \param theChar - the wide character to place at the offset
    /// \return bool - true on success.      
    /// \sa GetWChar()
    bool		SetWChar(CATUInt32 offset, CATWChar theChar);

    /// Length() retrieves the length of active string.
    ///
    /// \return CATUInt32 - length of string.
    /// \sa LengthCalc()
    CATUInt32 Length();

    /// LengthCalc() calculates the length w/o using or updating dirty flag
    ///
    /// \return CATUInt32 - length of string
    /// \sa Length()
    CATUInt32 LengthCalc() const;

    /// Compare() checks the equality of two strings - 
    /// Case sensitive.
    ///
    /// Result is the same as strcmp (0 == same).
    ///
    /// \param str    - CATString to compare against
    /// \param cmpLen - length to compare (0 = all)
    /// \param offset - offset to begin comparison
    /// 
    /// \return CATInt32 - 0 if equal, < 0 if less than, > 0 if greater.
    ///
    /// \sa CompareNoCase()
    CATInt32 Compare (const CATString&     str, 
        CATUInt32             cmpLen = 0, 
        CATUInt32             offset = 0) const;

    /// CompareNoCase() checks the equality of two strings -
    /// case insensitive.
    /// Result is the same as strcmp (0 == same).
    ///
    /// \param str    - CATString to compare against
    /// \param cmpLen - length to compare (0 = all)
    /// \param offset - offset to begin comparison
    /// 
    /// \return CATInt32 - 0 if equal, < 0 if less than, > 0 if greater.
    ///
    /// \sa Compare()
    CATInt32 CompareNoCase (  const CATString&   str, 
        CATUInt32           cmpLen = 0, 
        CATUInt32           offset = 0) const;

    /// Left() returns the left portion of the string, up to
    /// maxlength characters in length.
    ///
    /// \param maxlength - the maximum length from the left
    ///
    /// \return CATString - the resulting string.
    /// \sa Right(), FromRight(), Sub()
    CATString Left(CATUInt32 maxlength) const;

    /// Right() returns the right-hand of the string, starting
    /// at the position specified by start.
    ///
    /// \param start - index of first character of new string.
    ///
    /// \return CATString - the resulting string.
    /// \sa Left(), FromRight(), Sub()		
    CATString Right(CATUInt32 start) const;

    /// FromRight() returns the right-hand of the string, starting
    /// length characters from the end (e.g. the string will be
    /// [length] characters long).
    ///
    /// \param length - length from end of string to return
    ///
    /// \return CATString - the resulting string.
    /// \sa Right(), Left(), Sub()				
    CATString FromRight(CATUInt32 length) const;

    /// Sub() returns a substring of the current string.
    ///
    /// \param start - starting 0-based offset in string
    /// \param length - number of characters, starting at [start],
    ///        to return.
    ///
    /// \return CATString - the resulting string.
    /// \sa Left(), Right(), FromRight()
    CATString Sub(CATUInt32 start, CATUInt32 length) const;

    /// PullNextToken() retrieves the next token from a string.
    ///
    /// Finds the first instance of any of the characters in 'splitTokens'.
    /// Then, sets "token" to be everything to the left of that char.
    /// The string object becomes everything to the right of the char.
    /// If no more tokenizers remain in the string, then the command returns
    /// false and any remaining string is placed into 'token', and the string
    /// is left empty.
    ///
    /// example:
    /// CATString test = "Beer:Whiskey!Sex";
    /// CATString token;
    ///
    /// while (!test.IsEmpty())
    /// {
    ///		 if (!test.PullNextToken(token,":!"))
    ///         printf("(Last token....)\n");
    ///     printf("%s\n",(const char*)token);
    /// }
    ///
    ///
    /// Output:
    ///    Beer
    ///    Whiskey
    ///    (Last token....)
    ///    Sex
    bool PullNextToken(CATString& token, const CATString& splitTokens);

    /// Find() finds a substring within the string, starting at offset.  
    ///
    /// \param str - substring to find
    /// \param offset - reference used to return position if found.  
    ///
    /// \return true if found, false otherwise      
    /// \sa ReverseFind()
    bool Find(const CATString& str, CATUInt32& offset) const;

    /// Find() finds a char within the string, starting at offset.  
    ///
    /// \param theChar - character to find
    /// \param offset - reference used to return position if found.  
    ///
    /// \return true if found, false otherwise      
    /// \sa ReverseFind()
    bool Find(CATWChar theChar, CATUInt32& offset) const;

    /// ReverseFind() finds a substring in a string starting at the end.
    ///
    /// \param str - substring to find
    /// \param offset - offset to start search from
    ///       -1 is a special value indicating "from the end of the string".
    ///        Otherwise, the offset value returned will be the position 
    ///        within the string as in the Find functions above. 
    ///        e.g. offset == -1 has the same effect as offset==mystring.Length();
    ///
    /// \sa Find()
    bool ReverseFind(const CATString& str, CATUInt32& offset) const;

    /// ReverseFind() finds a character in a string starting at the end.
    ///
    /// \param theChar - character to find
    /// \param offset - offset to start search from
    ///       -1 is a special value indicating "from the end of the string".
    ///        Otherwise, the offset value returned will be the position 
    ///        within the string as in the Find functions above. 
    ///        e.g. offset == -1 has the same effect as offset==mystring.Length();
    ///
    /// \sa Find()
    bool ReverseFind(CATWChar theChar, CATUInt32& offset) const;


    /// Returns true if string is empty
    bool IsEmpty() const;

    /// Ensures string is contiguous and makes the buffer the greater
    /// of the current string length (+1 for null), or the minLength specified.
    ///
    /// Call 
    char*	 GetAsciiBuffer(CATUInt32 minLength = 0);
    CATWChar* GetUnicodeBuffer(CATUInt32 minlength = 0);

    /// Releases the previous get buffer
    void ReleaseBuffer();

    /// Trims whitespace chars off both ends of the string
    void Trim();

    /// Pad() pads the string with the specified char to be a certain length.
    ///
    /// If the string is already longer, then it leaves it alone.
    ///
    /// \param length - desired minimum length to pad to.
    /// \param theChar - pad char, space by default.
    void Pad (CATUInt32 length, CATWChar theChar = (CATWChar)0x20);

    /// Converts an unsigned long value to hex and appends it
    /// to the string.
    /// 
    /// \param hexValue - value to convert and append
    /// \return - reference to the string
    /// \sa AppendHexByte()
    CATString& AppendHex(CATUInt32 hexValue, bool addX = true);

    /// Converts a byte to hex and appends it to the string
    ///
    /// \param hexValue - unsigned char to convert and append
    /// \param addX - if true, '0x' is appended to the string.
    /// \return - reference to the string
    /// \sa AppendHex()
    CATString& AppendHexByte(CATUInt8 hexValue, bool addX = false);

    /// FromHex() converts a string from hex to a CATUInt32.
    CATUInt32 FromHex() const;
#ifdef CAT_CONFIG_WIN32
    /// Retrieve a GUID from the string. returns true if 
    /// it's a GUID (and sets guid), false otherwise.
    bool GetGUID(GUID& guid) const;
#endif
    /// ToUpper() converts the string to all upper-case.
    /// It will ignore anything that's not in standard a-z range.
    /// \sa ToLower()
    void ToUpper();

    /// ToLower() converts the string to all lower-case.
    ///
    /// It will ignore anything that's not in standard A-Z range.
    /// \sa ToUpper()
    void ToLower();

    /// Similar to (and uses) swprintf, variable arg formatting
    const CATString& Format(const CATWChar* formatSpecs, ...);
    
    /// Formats a string with arguments given a va_list (wide char version)
    const CATString& FormatArgs(const CATWChar* formatSpecs, va_list args);
    
    /// Splits a path into its components.  All parameters are optional and may be null if you don't
    /// need the portion. behaves similarly to wsplitpath.
    ///
    /// \param drive    ptr to string to receive drive, or null
    /// \param path     ptr to string to receive path (directory), or null
    /// \param filename ptr to string to receive filename, or null
    /// \param ext      ptr to receive file extension, or null.
    /// \return bool    true on success
    bool SplitPath(CATString* drive, CATString* path, CATString* filename, CATString* ext) const;

    /// Retrieve just the drive and directory from a full path
    CATString GetDriveDirectory() const;

    /// Retrieve just the filename and extension from a full path
    CATString GetFilenameExt()    const;

    /// Retrieve the filename, but not the extension from a full path
    CATString GetFilenameNoExt()  const;

    /// Escape() a string for XML
    /// \return CATString   Escaped string ready for output
    CATString Escape() const;

    /// Unescape() a string from XML
    /// \return CATString   Escape string ready for output
    CATString Unescape() const;

    /// Escape a string for use in a URL.
    ///
    /// Escapes characters in string to be valid within a URL. 
    /// Do NOT use on a full URL - it will escape the control characters.
    ///
    /// \return CATString   Escaped string
    CATString EncodeURL() const;

    /// Unescape a string from URL encoding.
    /// \return CATString   Unescaped string
    CATString DecodeURL() const;

protected:
    /// Creates a buffer for a string of the specified length
    bool Create(CATUInt32 length);

    /// Should only be called by constructors (or very carefully from Destroy) to init members
    void Init()
    {
        fUnicodeBuffer		= 0;
        fBufferSizeLocked	= false;
        fBuffer				= 0;	
        fBufferLength		= 0;
        fStrLen				= 0;
        fLenDirty			= true;			
        fAsciiLocked		= false;
        fUnicodeLocked		= false;
    }

    void Destroy()
    {
        delete [] fBuffer;
        fBuffer = 0;
        delete [] fUnicodeBuffer;
        fUnicodeBuffer = 0;
        Init();
    }

    // These retrieve the length of ascii or unicode strings (0 terminated ones)		
    CATUInt32 GetLength(const char* asciistr) const;
    CATUInt32 GetLength(const CATWChar* unistr) const;

    /// Copies the unicode string into the fUnicodeStr
    void CopyIn(const CATWChar* unistr);

    /// Copies a buffer of chars
    static void CopyBuffer(char* str1, const char* str2, CATUInt32 length);

    /// Copies a buffer of wide chars
    static void CopyBuffer(CATWChar* str1, const CATWChar* str2, CATUInt32 length);

    // Allocation / expansion of string buffer
    bool AllocBuffer(CATUInt32 minLength = 0);
    bool ExpandBuffer(CATUInt32 minLength = 0);

    /// These replace the old Update() function.
    bool ImportAscii(const char* ascii);
    bool UnicodeFromAscii();
    bool AsciiFromUnicode() const;

private:
    bool             fBufferSizeLocked;	    ///< True when locked by get buffer
    CATUInt32	     fStrLen;				///< String length if known and not dirty
    bool		     fLenDirty;				///< String modified since last size check
    CATUInt32	     fBufferLength;			///< Length of current buffer - include 0.

    mutable char*    fBuffer;			    ///< ASCII buffer
    CATWChar*        fUnicodeBuffer;		///< Unicode buffer

    bool		     fAsciiLocked;			///< Ascii is locked - can't use unicode functions
    bool		     fUnicodeLocked;		///< Unicode is locked - can't use ascii functions
};

//-----------------------------------------------------------------------------
// operator overloads
//-----------------------------------------------------------------------------

inline CATString operator+(const CATString& str1, const CATString& str2)
{
    CATString newStr = str1;
    newStr += str2;
    return newStr;
}

//---------------------------------------------------------------
// Comparators... 
//---------------------------------------------------------------
inline bool operator==(const CATString& str1, const CATString& str2)
{
    return (str1.Compare(str2) == 0);
}

inline bool operator==(const CATString& str1, const char *unistr2)
{	
    CATString str2 = unistr2;
    return (str1.Compare(str2) == 0);
}

inline bool operator==(const CATString& str1, const CATWChar* unistr2)
{	
    CATString str2 = unistr2;
    return (str1.Compare(str2) == 0);
}

inline bool operator==(const char *unistr2,const CATString& str1)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) == 0);
}

inline bool operator==(const CATWChar *unistr2,const CATString& str1)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) == 0);
}

//---------------------------------------------------------------
// Not equals... 
//---------------------------------------------------------------
inline bool operator!=(const CATString& str1, const CATString& str2)
{
    return (str1.Compare(str2) != 0);
}

inline bool operator!=(const CATString& str1, const char *unistr2)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) != 0);
}

inline bool operator!=(const CATString& str1, const CATWChar* unistr2)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) != 0);
}

inline bool operator!=(const char *unistr2,const CATString& str1)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) != 0);
}

inline bool operator!=(const CATWChar *unistr2,const CATString& str1)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) != 0);
}

//---------------------------------------------------------------
// Less Than
//---------------------------------------------------------------

inline bool operator<(const CATString& str1, const CATString& str2)
{
    return (str1.Compare(str2) < 0);
}

inline bool operator<(const CATString& str1, const char *unistr2)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) < 0);
}

inline bool operator<(const CATString& str1, const CATWChar *unistr2)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) < 0);
}

inline bool operator<(const char *unistr2,const CATString& str1)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) > 0);
}

inline bool operator<(const CATWChar *unistr2,const CATString& str1)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) > 0);
}

//---------------------------------------------------------------
// Greater than
//---------------------------------------------------------------

inline bool operator>(const CATString& str1, const CATString& str2)
{
    return (str1.Compare(str2) > 0);
}

inline bool operator>(const CATString& str1, const char *asciistr2)
{
    CATString str2 = asciistr2;
    return (str1.Compare(str2) > 0);
}

inline bool operator>(const CATString& str1, const CATWChar *unistr2)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) > 0);
}

inline bool operator>(const char *asciistr2,const CATString& str1)
{
    CATString str2 = asciistr2;
    return (str1.Compare(str2) < 0);
}

inline bool operator>(const CATWChar *unistr2,const CATString& str1)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) < 0);
}

//---------------------------------------------------------------
// Greater than or equals
//---------------------------------------------------------------

inline bool operator>=(const CATString& str1, const CATString& str2)
{
    return (str1.Compare(str2) >= 0);
}

inline bool operator>=(const CATString& str1, const char *unistr2)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) >= 0);
}

inline bool operator>=(const CATString& str1, const CATWChar *unistr2)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) >= 0);
}

inline bool operator>=(const char *unistr2,const CATString& str1)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) <= 0);
}

inline bool operator>=(const CATWChar *unistr2,const CATString& str1)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) <= 0);
}

//---------------------------------------------------------------
// Less Than or Equals
//---------------------------------------------------------------

inline bool operator<=(const CATString& str1, const CATString& str2)
{
    return (str1.Compare(str2) <= 0);
}

inline bool operator<=(const CATString& str1, const char *unistr2)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) <= 0);
}

inline bool operator<=(const CATString& str1, const CATWChar *unistr2)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) <= 0);
}

inline bool operator<=(const char *unistr2,const CATString& str1)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) >= 0);
}

inline bool operator<=(const CATWChar *unistr2,const CATString& str1)
{
    CATString str2 = unistr2;
    return (str1.Compare(str2) >= 0);
}




#endif // _CATString_H_
