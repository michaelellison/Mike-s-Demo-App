/// \file    CATXMLParser.cpp
/// \brief   XML Parser
/// \ingroup CAT
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATXMLParser.h"
#include "CATXMLObject.h"
#include "CATStreamFile.h"
CATXMLParser::CATXMLParser()
{
    fCurParent    = 0;
}


CATXMLParser::~CATXMLParser()
{
}

CATResult CATXMLParser::Parse(const CATWChar*  path,
                            CATXMLFactory* factory,
                            CATXMLObject*& root)
{
    if (!path)
    {
        return CAT_ERR_XML_PARSER_INVALID_PATH;
    }

    CATStreamFile stream;
    CATResult result;
    if (CATFAILED(result = stream.Open(path,CATStream::READ_ONLY)))
    {
        return result;
    }
    result = CATXMLParser::ParseStream(&stream,factory,root);
    stream.Close();
    return result;
}


CATResult CATXMLParser::ParseStream(CATStream* stream,
                            CATXMLFactory* factory,
                            CATXMLObject*& root)
{
    CATResult result;
    if (!stream)
    {
        return CATRESULT(CAT_ERR_XML_PARSER_INVALID_PATH);
    }

    stream->SeekAbsolute(0);
    CATInt64 filesize = 0;
    stream->Size(filesize);

    CATUInt32 fsize = (CATUInt32)filesize;
    if ((CATInt64)fsize != filesize)
    {        
        return CATRESULT(CAT_ERR_XML_PARSER_OUT_OF_MEMORY);
    }

    CATUInt8* buffer = new CATUInt8[fsize+1];
    if (!buffer)
    {        
        return CATRESULT(CAT_ERR_XML_PARSER_OUT_OF_MEMORY);
    }

    memset(buffer,0,fsize+1);
    if (CATFAILED(result = stream->Read(buffer,fsize)))
    {
        delete [] buffer;
        return result;
    }

    result = ParseMemory(buffer,fsize,factory,root);

    delete [] buffer;
    return result;
}

CATResult CATXMLParser::ParseMemory(const void*  memoryBuf,
                                  CATInt32        bufLen,
								  CATXMLFactory* factory,
								  CATXMLObject*& root)
{
    CATXMLParser* parser = new CATXMLParser();    
    root = 0;    
    parser->fRootObjPtr = &root;
    parser->fFactory    = factory;
    
    XML_Parser expatParser = XML_ParserCreate(0);
    XML_SetUserData(expatParser, parser);    
    XML_SetElementHandler(expatParser,&CATXMLParser::StartElement,&CATXMLParser::EndElement);    
    XML_SetCharacterDataHandler(expatParser,&CATXMLParser::CharacterHandler);    

    if (XML_Parse(expatParser,(char*)memoryBuf,bufLen,1) == XML_STATUS_ERROR)
    {
         // On error, bail with a string....
         XML_ParserFree(expatParser);
         delete parser;
         return CATRESULT(CAT_ERR_XML_INVALID_XML);
    }
    
    if (root)
    {
       root->ParseAttributes();
    }

    XML_ParserFree(expatParser);
    
    delete parser;    
    return CAT_SUCCESS;
}


void XMLCALL CATXMLParser::StartElement(void *userData,
                                        const XML_Char *name,
                                        const XML_Char **atts)
{    
    CATXMLParser* parser = (CATXMLParser*)userData;
    CATXMLAttribs* newAttribs = new CATXMLAttribs(CATXMLObject::CATXMLKeyComp);
    for (CATUInt32 i = 0; atts[i]; i += 2) 
    {
        CATWChar* newKey = new CATWChar[wcslen((const CATWChar*)atts[i])+1];
        CATWChar* newVal = new CATWChar[wcslen((const CATWChar*)atts[i+1])+1];
        wcscpy(newKey,atts[i]);
        wcscpy(newVal,atts[i+1]);

        newAttribs->insert(std::make_pair(newKey, newVal));
    }

    CATXMLObject* newObject = 0;    
    // Attributes object becomes owned by the new object.
    CATResult hr = parser->fFactory->Create( name, newAttribs, parser->fCurParent, newObject);
    if (CATFAILED(hr) || (newObject == 0))
    {
        while (newAttribs->size() != 0)
        {
            CATXMLAttribsIter iter = newAttribs->begin();                        
            delete [] (CATWChar*)iter->first;
            delete [] (CATWChar*)iter->second;
            newAttribs->erase(iter);
        }
        delete newAttribs;
		return;
    }

    if (*parser->fRootObjPtr == 0)
    {
        *parser->fRootObjPtr = newObject;
    }

    parser->fCurParent = newObject;
    parser->fObjectStack.push(newObject);    
}


void XMLCALL CATXMLParser::CharacterHandler(void *userData,
                                            const XML_Char *s,
                                            int len)
{
    CATXMLParser* parser = (CATXMLParser*)userData;
	if (parser->fCurParent)
	{
		parser->fCurParent->AppendData(s,len);
	}
}

        
       
void XMLCALL CATXMLParser::EndElement(void *userData,
                                      const XML_Char* /*name*/)
{
    CATXMLParser* parser = (CATXMLParser*)userData;
    parser->fObjectStack.pop();
    if (parser->fObjectStack.size())
    {
        parser->fCurParent = parser->fObjectStack.top();
    }
    else
    {
        parser->fCurParent = 0;
    }
}

CATResult CATXMLParser::Write(const CATString& filename, CATXMLObject* rootNode)
{
   CATResult result = CAT_SUCCESS;
   CATString header = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
   CATStreamFile xmlFile;
   header << kCRLF;

   if (CATFAILED(result = xmlFile.Open(filename,CATStream::READ_WRITE_CREATE_TRUNC)))
   {
      return result;
   }      

   result = xmlFile.Write((const char*)header, header.Length());
   if (CATFAILED(result))
   {
      (void)xmlFile.Close();
      return result;
   }

   //CATXMLObject* curNode = 0;
   result = rootNode->WriteToStream(&xmlFile);
   (void)xmlFile.Close();

   return result;
}
