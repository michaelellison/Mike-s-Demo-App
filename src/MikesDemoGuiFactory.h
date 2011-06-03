/// \file MikesDemoGuiFactory.h
/// \brief GUI factory for demo app
/// \ingroup MikesDemo
///
// Copyright (c) 2011 by Michael Ellison.
// See COPYING.txt for license (MIT License).
//

#ifndef _MikesDemoGuiFactory_H_
#define _MikesDemoGuiFactory_H_

#include "CAT.h"
#include "CATGuiFactory.h"
#include "MikesDemoWindow_Main.h"

/// \class MikesDemoGuiFactory
/// \brief GUI factory for demo app
/// \ingroup MikesDemo
class MikesDemoGuiFactory : public CATGuiFactory
{
    public:
        MikesDemoGuiFactory(const CATString& skinRoot, const CATString& skinPath)
            :CATGuiFactory(skinRoot,skinPath)
        {
        }

        virtual ~MikesDemoGuiFactory()
        {
        }

        virtual CATXMLObject* CreateObject( const CATWChar* objectType)
        {
				// Add named custom objects here...
            if (0 == wcscmp(objectType,L"MikesDemoWindow_Main"))          
				{
					return new MikesDemoWindow_Main         (objectType,fSkinRoot);
				}
            return CATGuiFactory::CreateObject(objectType);
        }
};


#endif // _MikesDemoGuiFactory_H_
