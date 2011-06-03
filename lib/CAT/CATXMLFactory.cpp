/// \file CATXMLFactory.cpp
/// \brief XML Factory base
/// \ingroup CAT
///
/// Copyright (c) 2003-2007 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#include "CATXMLFactory.h"

CATXMLFactory::CATXMLFactory()
{}

CATXMLFactory::~CATXMLFactory()
{}

CATXMLObject* CATXMLFactory::CreateObject( const CATWChar* objectType)
{
    CATXMLObject* newObject = new CATXMLObject(objectType);
    return newObject;
}


CATResult CATXMLFactory::Create(  const CATWChar*  objectType,
                                  CATXMLAttribs*   attributes,
                                  CATXMLObject*    parent,
                                  CATXMLObject*&   newObject)
{
    newObject = CreateObject(objectType);
    
    if (newObject == 0)
        return CAT_ERR_XML_CREATE_FAILED;
    
    
    newObject->SetAttributes(attributes);    
    if (parent)
    {
        parent->AddChild(newObject);
    }

    return CAT_SUCCESS;
}
