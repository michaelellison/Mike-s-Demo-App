/// \file CATStringTableCore.cpp
/// \brief Core String table for CAT library
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATStringTableCore.h"
#include "CATXMLFactory.h"
#include "CATXMLParser.h"
#include "CATStreamFile.h"

struct CATStringEntry
{
    CATResult       resultCode;
    const CATWChar* resultString;
};

CATStringTableCore::CATStringTableCore()
: CATStringTable()
{
}

CATStringTableCore::~CATStringTableCore()
{
}

CATString CATStringTableCore::GetString(CATUInt32     stringId)
{
    const std::map<CATUInt32,CATString>::iterator it = fStringMap.find(stringId);
    CATString theString;

    if (it != fStringMap.end())
    {
        theString = it->second;
    }
    else
    {    
        // Check subtables
        theString = CATStringTable::GetString(stringId);
    }
    return theString;
}

CATResult CATStringTableCore::LoadXMLStringTable(const CATString& path, const char* langId)
{
    CATResult     result;
    CATStreamFile xmlFile;
    CATXMLFactory factory;
    CATXMLObject* rootObj = 0;
    

    if (CATFAILED(result = CATXMLParser::Parse(path,&factory, rootObj)))
        return result;

    if (!rootObj)
        return CAT_SUCCESS;

    if (((CATString)rootObj->GetType()).Compare("CATStringTable") != 0)
        return CAT_ERR_INVALID_STRINGTABLE;
    CATUInt32 numStrings = rootObj->GetNumChildren();

    CATString catLang = langId;

    for (CATUInt32 i = 0; i < numStrings; i++)
    {
        CATXMLObject *curObj = rootObj->GetChild(i);
        if (curObj)
        {
            CATString id = curObj->GetAttribute(L"id");
            CATUInt32 value = curObj->GetAttribute(L"value",0);
                        
            CATString theString = curObj->GetAttribute(catLang);
            if (value != 0)
            {
                fStringMap[value] = theString;
            }           
        }
    }

    return result;
}

CATResult CATStringTableCore::GenHeaderForXML(const CATString& path)
{
    CATResult     result;
    CATXMLFactory factory;
    CATXMLObject* rootObj = 0;
    

    if (CATFAILED(result = CATXMLParser::Parse(path,&factory, rootObj)))
        return result;

    if (!rootObj)
        return CAT_SUCCESS;


    if (((CATString)rootObj->GetType()).Compare("CATStringTable") != 0)
        return CAT_ERR_INVALID_STRINGTABLE;

    CATStreamFile headerFile;
    CATString headerName = rootObj->GetAttribute(L"name");
    if (headerName.IsEmpty())
        return CAT_ERR_INVALID_STRINGTABLE;

	 CATString headerDir = path.GetDriveDirectory();
    CATString headerFn;
	 headerFn << headerDir << headerName << ".h";

    result = headerFile.Open(headerFn,CATStream::READ_WRITE_CREATE_TRUNC);
    if (CATFAILED(result))
        return result;
    CATString headerBuf;
    headerBuf << "#ifndef _" << headerName << "_H_\r\n";
    headerBuf << "#define _" << headerName << "_H_\r\n";
    headerFile.Write((const char*)headerBuf,(CATUInt32)strlen(headerBuf));

    CATUInt32 numStrings = rootObj->GetNumChildren();

    for (CATUInt32 i = 0; i < numStrings; i++)
    {
        CATXMLObject *curObj = rootObj->GetChild(i);
        if (curObj)
        {
            CATString id = curObj->GetAttribute(L"id");
            CATUInt32 value = (CATUInt32)curObj->GetAttribute(L"value",0);
            CATString theString = curObj->GetAttribute(L"eng");

            
            if (value != 0)
            {
                headerBuf = "#define     ";
                headerBuf << id;
                while (headerBuf.Length() < 50)
                    headerBuf << " ";
                headerBuf << " ";
                headerBuf.AppendHex(value);
                headerBuf << " // " << theString << "\r\n";
                headerFile.Write((const char*)headerBuf,(CATUInt32)strlen(headerBuf));
            }           
        }
    }

    headerBuf = "#endif  //_";
    headerBuf << headerName << "_H_\r\n\r\n";
    headerFile.Write((const char*)headerBuf,(CATUInt32)strlen(headerBuf));
    headerFile.Close();
    return result;
}

CATResult CATStringTableCore::GenHTML(const CATString& path)
{
    CATResult     result;
    CATXMLFactory factory;
    CATXMLObject* rootObj = 0;
    

    if (CATFAILED(result = CATXMLParser::Parse(path,&factory, rootObj)))
        return result;

    if (!rootObj)
        return CAT_SUCCESS;


    if (((CATString)rootObj->GetType()).Compare("CATStringTable") != 0)
        return CAT_ERR_INVALID_STRINGTABLE;

    CATStreamFile headerFile;
    CATString headerName = rootObj->GetAttribute(L"name");
    if (headerName.IsEmpty())
        return CAT_ERR_INVALID_STRINGTABLE;

    CATString headerFn = headerName;
    headerFn << ".html";
    result = headerFile.Open(headerFn,CATStream::READ_WRITE_CREATE_TRUNC);
    if (CATFAILED(result))
        return result;
    CATString headerBuf;
    headerBuf << "<HTML><BODY><OL>\r\n";

    headerFile.Write((const char*)headerBuf,(CATUInt32)strlen(headerBuf));

    CATUInt32 numStrings = rootObj->GetNumChildren();

    for (CATUInt32 i = 0; i < numStrings; i++)
    {
        CATXMLObject *curObj = rootObj->GetChild(i);
        if (curObj)
        {
            CATString id = curObj->GetAttribute(L"id");
            CATUInt32 value = (CATUInt32)curObj->GetAttribute(L"value",0);
            CATString theString = curObj->GetAttribute(L"eng");

            
            if (value != 0)
            {
                headerBuf = "<LI>";
                headerBuf << "<B>";
                headerBuf.AppendHex(value);
                headerBuf << "</B>: " << theString << "</LI>\r\n";
                headerFile.Write((const char*)headerBuf,(CATUInt32)strlen(headerBuf));
            }           
        }
    }

    headerBuf = "</OL></BODY></HTML>";
    headerFile.Write((const char*)headerBuf,(CATUInt32)strlen(headerBuf));
    headerFile.Close();
    return result;
}
