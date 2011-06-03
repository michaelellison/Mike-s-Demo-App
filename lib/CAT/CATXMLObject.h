/// \file    CATXMLObject.h
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

#ifndef _CATXMLOBJECT_H_
#define _CATXMLOBJECT_H_

#include "CATInternal.h"
#include "CATString.h"
#include <map>
#include <string>

class CATStream;
/// CATXMLAttribs are currently just a string map of attribute name/value
typedef bool (*CATXMLKEYCOMP)( const CATWChar*  g1, const CATWChar* g2);
typedef std::map<const CATWChar*, CATWChar*, CATXMLKEYCOMP> CATXMLAttribs ;
typedef CATXMLAttribs::iterator CATXMLAttribsIter;

/// \class   CATXMLObject
/// \brief   XML Object Base
/// \ingroup CAT
///
class CATXMLObject
{
public:
    /// Object constructor
    ///
    /// \param type Name of object type, this corresponds to the XML tag
    CATXMLObject(const CATWChar* type);

    /// Destructor
    virtual ~CATXMLObject();

    /// Insert a child object into the XML
    /// \param child Child to add to object (parent becomes set)
    void            AddChild(CATXMLObject* child);

    /// Retrieve number of child xml objects
    CATUInt32       GetNumChildren();

    /// Retrieve a ptr to a specific child (do NOT delete - owned by object)
    /// \param  index               index in child vector (0 <= index < GetNumChildren)
    /// \return CATXMLObject*       Requested xml object, or 0 if not found
    CATXMLObject* GetChild(CATUInt32 index) const;

    /// Set the parent object (generally only called by parent/factory)
    /// \param parent   Parent object
    void          SetParent(CATXMLObject* parent);

    /// Retrieves the parent object 
    /// \return CATXMLObject*  Parent object, or 0 if none.
    CATXMLObject* GetParent();

    /// Add an attribute to the object
    /// \param  key   String key value
    /// \param  value String value for the key
    /// \return CATResult 0 on success
    CATResult       AddAttribute(const CATWChar* key, const CATWChar* value);

    int GetNumAttributes();

    CATString GetAttributeKeyByIndex(int index);

    /// Retrieve the value for a specified attribute key
    /// \param  key               Name of key to retrieve value of
    /// \return const CATWChar*   Value, or NULL if not found (may be empty if defined but clear)
    CATString GetAttribute(const CATWChar* key);        

    /// Templated attribute conversion with default val.
    /// Returns default value if attribute is not found or is empty.
    /// otherwise, converts the CATString to the proper type.
    template<class T>
    T GetAttribute(const CATWChar* key, T defaultVal)
    {
        if (key == 0)
            return defaultVal;
        CATString attrib = GetAttribute(key);

        if (attrib.IsEmpty())
            return defaultVal;

        return (T)attrib;
    }

    /// Sets the attributes to an existing CATXMLAttribs object. 
    ///
    /// The object then takes control of the XMLAttribs object - DO NOT DELETE.
    /// The CATXMLObject becomes responsible for deletion.
    ///
    /// \param attribs Attributes to set for object - ownership is taken by XMLObject
    ///
    void          SetAttributes(CATXMLAttribs* attribs);

    /// Child classes should override this to parse out the
    /// attribute values.        
    virtual CATResult ParseAttributes();

    /// Key comparator function
    static bool CATXMLKeyComp( const CATWChar*  g1, const CATWChar* g2);    

    /// Retrieve the type (the tag name) for the object.
    const CATWChar* GetType();

    /// Append to the character data found between start and end tags of the object.
    void	AppendData(const CATWChar* data, CATInt32 len);

    /// Set the data directly.
    void	SetData(const CATWChar* data);

    /// Retrieve the data 
    const	CATWChar* GetData();

    /// WriteToStream() is a recursive function that writes the object
    /// and its children to a stream as XML.
    ///
    /// You must open the stream and write the header out first, then
    /// call this on the top level nodes to create a valid XML file.
    ///
    /// \param stream - the opened stream to write to.
    /// \return CATResult - CAT_SUCCESS on success.
    CATResult WriteToStream(CATStream* stream);

protected:
    CATWChar*                     fType;      ///< Text type
    CATXMLAttribs*                fAttribs;   ///< Attributes of the object
    CATXMLObject*                 fParent;    ///< Parent xml object
    std::vector<CATXMLObject*>    fChildren;  ///< Child objects in xml
    std::wstring				  fData;	  ///< Data from xml for object
};

#endif //_CATXMLOBJECT_H_
