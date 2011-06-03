/// \file MikesDemo.h
/// \brief App shell for demo program
/// \ingroup MikesDemo
/// Demo app, Copyright (c) 2011 by Mike Ellison
#ifndef _MikesDemo_H_
#define _MikesDemo_H_

#include "CATApp.h"
#include "CATGUI.h"
#include "MikesDemoGuiFactory.h"

/// \class MikesDemo
/// \brief App shell for demo program
/// \ingroup MikesDemo
class MikesDemo : public CATApp
{
   public:
        /// App construct - requires you to specify a runmode in the 
        /// constructor.
        ///
        /// \param runMode - current runmode. See CATRunMode
        ///
                  MikesDemo(CATINSTANCE appInstance, CATRunMode runMode, const CATString& appName);
        virtual    ~MikesDemo();

        virtual CATResult     OnStart();
        virtual CATResult     OnEnd(const CATResult& result);
        virtual CATResult     OnEvent(const CATEvent& event, CATInt32& retVal);
        virtual CATResult     MainLoop();

        virtual void                 OnHelp();

        virtual void         OnCommand(  CATCommand& command, 
                                         CATControl* ctrl, 
                                         CATWindow* wnd, 
                                         CATSkin* skin);

    protected:
        CATWindow*          fWindow;
};

#endif // _MikesDemo_H_