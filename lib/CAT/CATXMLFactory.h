/// \file CATXMLFactory.h
/// \brief XML Factory base
/// \ingroup CAT
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $


#ifndef _CATXMLFACTORY_H_
#define _CATXMLFACTORY_H_

#include "CATInternal.h"
#include "CATXMLObject.h"

/// \class   CATXMLFactory
/// \brief   XML Factory base
/// \ingroup CAT
class CATXMLFactory
{
    public:
        CATXMLFactory();
        virtual ~CATXMLFactory();

        /// Overridable function to create an object.
        /// \param   objectType     Type of object (XML Tag)
        /// \return  CATXMLObject*  Object created (must be derived from CATXMLObject)
        virtual CATXMLObject* CreateObject( const CATWChar* objectType);

        /// Called by parser to create an object.
        ///
        /// \param  objectType   Type of object (XML Tag)
        /// \param  attributes   Attributes vector
        /// \param  parent       Parent object
        /// \param  newObject    Set to new object on success.
        /// \return CATResult    CAT_SUCCESS on success.
        CATResult Create( const CATWChar*  objectType,
                          CATXMLAttribs*   attributes,
                          CATXMLObject*    parent,
                          CATXMLObject*&   newObject);        
};

#endif //_CATXMLFACTORY_H_
