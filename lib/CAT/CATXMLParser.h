/// \file    CATXMLParser.h
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


#ifndef _CATXMLPARSER_H_
#define _CATXMLPARSER_H_

#include "CATInternal.h"
#include "CATXMLFactory.h"
#include "CATStream.h"

#include <stack>
#include "expat.h"

/// Size to use for buffered reading from files/streams
const int kMaxXMLBufferSize = 1024;

/// \class CATXMLParser
/// \brief XML PArser
/// \ingroup CAT
class CATXMLParser
{
    public:
        /// Parses a file with the given factory.  The root
        /// node will be placed into root.
        ///
        /// \param path     Path to file
        /// \param factory  Factory to use for object creation. 
        ///                 Must be derived from CATXMLFactory.
        /// \param root     Ptr to receive root object of tree on success.
        /// \return CATRESULT CATRESULT_SUCCESS on success.
        static CATResult Parse( const CATWChar*  path,
                                CATXMLFactory*  factory,
                                CATXMLObject*&  root);

        /// Parses a memory buffer with the given factory.  The root
        /// node will be placed into root.
        ///
        /// \param memoryBuf    Pointer to memory buffer containing
        ///                     full XML file.
        /// \param bufLenBytes  size of buffer in bytes (NOT characters) 
        /// \param factory      Factory to use for object creation. 
        ///                     Must be derived from CATXMLFactory.
        /// \param root         Ptr to receive root object of tree on success.
        /// \return CATRESULT   CATRESULT_SUCCESS on success.
		static CATResult ParseMemory(  const void*     memoryBuf,
                                       CATInt32            bufLenBytes,
								       CATXMLFactory*   factory,
								       CATXMLObject*&   root);

        /// Parses a stream with the given factory. Stream location should
        /// be at the start of the XML.  Root node is placed in root on
        /// success;
        ///
        /// \param stream       Stream to read XML data from
        /// \param factory      Factory to use for object creation.
        /// \param root         Ptr to receive root object of tree.
        /// \return CATRESULT   CATRESULT_SUCCESS on success.
        static CATResult ParseStream(CATStream*         stream,
                                     CATXMLFactory*     factory,
                                     CATXMLObject*&     root);        

        static CATResult Write(      const CATString&   filename, 
                                     CATXMLObject*      rootNode);

    protected:
        CATXMLParser();        
        virtual ~CATXMLParser();

        // expat XML callbacks used during a Parse
        static void XMLCALL StartElement( void *userData,
                                          const XML_Char *name,
                                          const XML_Char **atts);

        static void XMLCALL EndElement( void *userData,
                                        const XML_Char *name);

        static void XMLCALL CharacterHandler( void *userData,
                                              const XML_Char *s,
                                              int len);
        
    private:
        std::stack<CATXMLObject*>   fObjectStack;       ///< Current object stack during a parse
        CATXMLObject*               fCurParent;         ///< Last object pushed onto the stack (parent of current)
        CATXMLFactory*              fFactory;           ///< Factory to create objects with
        CATXMLObject**              fRootObjPtr;        ///< Root object ptr
};

#endif //_CATXMLPARSER_H_
