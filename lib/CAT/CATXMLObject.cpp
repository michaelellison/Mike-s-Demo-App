/// \file    CATXMLObject.cpp
/// \brief   XML Object Base
/// \ingroup CAT
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATXMLObject.h"
#include "CATStream.h"

CATXMLObject::CATXMLObject(const CATWChar* type)
{
    this->fParent = 0;

    // Have to set a variable and redirect for vs6.0
    CATXMLKEYCOMP keyComp = CATXMLObject::CATXMLKeyComp;

    this->fAttribs = new CATXMLAttribs(keyComp);
    this->fType = new CATWChar[wcslen(type) + 1];
    wcscpy(fType,type);
}

// On destruction, clean up and delete children
CATXMLObject::~CATXMLObject()
{
    std::vector<CATXMLObject*>::iterator iter = fChildren.begin();
    while (iter != fChildren.end())
    {
        delete (*iter);
        iter = fChildren.erase(iter);
    }

    if (fAttribs)
    {
        while (fAttribs->size())
        {
            CATXMLAttribsIter iter = fAttribs->begin();
            delete [] (CATWChar*)iter->first;
            delete [] (CATWChar*)iter->second;
            fAttribs->erase(iter);
        }
        delete fAttribs;
        fAttribs = 0;
    }
    delete [] fType;
    fType = 0;
}

/// Insert a child object into the theme
void CATXMLObject::AddChild(CATXMLObject* child)
{
    child->SetParent(this);
    this->fChildren.push_back(child);
}

/// Retrieve number of child theme objects
CATUInt32 CATXMLObject::GetNumChildren()
{
    return (CATUInt32)this->fChildren.size();
}

// Retrieve a ptr to a specific child (do NOT delete - owned by parent object)
// \param  index               index in child vector (0 <= index < GetNumChildren)
// \return CATXMLObject*       Requested xml object, or 0 if not found
CATXMLObject* CATXMLObject::GetChild(CATUInt32 index) const
{
    return this->fChildren[index];
}

// Set our owner object
void CATXMLObject::SetParent(CATXMLObject* parent)
{
    this->fParent = parent;
}

// Add an attribute to the object
CATResult CATXMLObject::AddAttribute(const CATWChar* key, const CATWChar* value)
{    
    if ((key == 0) || (value == 0))
        return CAT_ERR_XML_INVALID_ATTRIBUTE;

    CATWChar* newKey   = new CATWChar[wcslen(key)+1];
    CATWChar* newValue = new CATWChar[wcslen(value)+1];
    if ((newKey == 0) || (newValue == 0))
        return CAT_ERR_XML_INVALID_ATTRIBUTE;

    wcscpy(newKey,key);
    wcscpy(newValue,value);

    // Clear out attrib if exists already
    CATXMLAttribsIter iter = this->fAttribs->find(key);
    if (iter != fAttribs->end())
    {
        delete [] (CATWChar*)iter->first;
        delete [] (CATWChar*)iter->second;
        this->fAttribs->erase(iter);
    }

    // set new attrib
    this->fAttribs->insert( std::make_pair(newKey, newValue) );

    return CAT_SUCCESS;
}

// Retrieve an attribute value
CATString CATXMLObject::GetAttribute(const CATWChar* key)
{    
    if (key == 0)
        return L"";

    CATXMLAttribs::iterator iter = fAttribs->find(key);

    if (iter != fAttribs->end())
    {
        return CATString(iter->second);
    }
    return L"";
}

// Sets the attributes for the object, takes ownership of the CATXMLAttribs passed in.
void CATXMLObject::SetAttributes(CATXMLAttribs* attribs)
{
    if (fAttribs)
    {
        while (fAttribs->size())
        {
            CATXMLAttribsIter iter = fAttribs->begin();
            delete [] (CATWChar*)iter->first;
            delete [] (CATWChar*)iter->second;
            fAttribs->erase(iter);
        }
        delete fAttribs;
        fAttribs = 0;
    }

    fAttribs = attribs;

    if (fAttribs == 0)
    {
        CATXMLKEYCOMP keyComp = CATXMLObject::CATXMLKeyComp;
        fAttribs = new CATXMLAttribs(keyComp);
    }
}


// Child objects should parse out attributes here, but we don't need.
CATResult CATXMLObject::ParseAttributes()
{
    for (CATUInt32 i = 0; i < this->GetNumChildren(); i++)
    {
        this->GetChild(i)->ParseAttributes();
    }

    return CAT_SUCCESS;
}

bool CATXMLObject::CATXMLKeyComp( const CATWChar*  g1, const CATWChar* g2)
{
    if (wcscmp(g1,g2) < 0)
        return true;

    return false;
}

const CATWChar* CATXMLObject::GetType()
{
    return fType;
}

void CATXMLObject::AppendData(const CATWChar* data, CATInt32 len)
{	
    fData.append(data,len);
}

const CATWChar* CATXMLObject::GetData()
{
    return fData.c_str();
}

void CATXMLObject::SetData(const CATWChar* data)
{
    if (data == 0)
    {
        fData = L"";
        return;
    }
    fData = data;
}


int CATXMLObject::GetNumAttributes()
{
    return (int)fAttribs->size();
}

CATString CATXMLObject::GetAttributeKeyByIndex(int index)
{
    CATXMLAttribsIter iter = fAttribs->begin();
    int i = 0;

    while (iter != fAttribs->end())
    {
        if (i == index)
            return CATString(iter->first);
        i++;
        iter++;
    }

    return L"";		
}


CATXMLObject* CATXMLObject::GetParent()
{
    return fParent;
}

CATResult CATXMLObject::WriteToStream(CATStream* stream)
{
    CATResult result = CAT_SUCCESS;
    if ((stream == 0) || (stream->IsOpen() == false))
    {
        return CATRESULT(CAT_ERR_STREAM_INVALID);
    }

    CATString tmpTxt = L"<";
    tmpTxt << ((CATString)fType).Escape();

    if (fAttribs->size() == 0)
    {
        tmpTxt << L">";     
        if (CATFAILED(result = stream->Write((const char*)tmpTxt, tmpTxt.Length())))
        {
            // Bail if we get a write error
            return result;
        }
    }
    else
    {                    
        if (CATFAILED(result = stream->Write((const char*)tmpTxt, tmpTxt.Length())))
        {
            // Bail if we get a write error
            return result;
        }

        CATXMLAttribsIter iter = fAttribs->begin();
        while (iter != fAttribs->end())
        {
            CATString key = (*iter).first;
            CATString val = (*iter).second;

            tmpTxt = " ";
            tmpTxt << key.Escape() << L"=\"" << val.Escape() << "\"";

            if (CATFAILED(result = stream->Write((const char*)tmpTxt, tmpTxt.Length())))
            {
                // Bail if we get a write error
                return result;
            }

            ++iter;
        }

        // Write out closing text for start tag
        tmpTxt = L">";
        if (CATFAILED(result = stream->Write((const char*)tmpTxt, tmpTxt.Length())))
        {
            // Bail if we get a write error
            return result;
        }
    }

    // Enumerate through the children and let them write themselves out.
    CATUInt32 numChildren = this->GetNumChildren();
    CATXMLObject* childObj = 0;         
    for (CATUInt32 childIndex = 0; childIndex < numChildren; childIndex++)
    {
        childObj = GetChild(childIndex);
        result = childObj->WriteToStream(stream);
        
        // break on error
        if (CATFAILED(result))
            break;
    }

    // Write ending tag
    tmpTxt = L"</";
    tmpTxt << fType << L">";
    if (CATFAILED(result = stream->Write((const char*)tmpTxt, tmpTxt.Length())))
        return result;

    return result;         
}
