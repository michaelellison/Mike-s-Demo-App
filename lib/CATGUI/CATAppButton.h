//---------------------------------------------------------------------------
/// \file CATAppButton.h
/// \brief Specialized pushbutton for launching apps
/// \ingroup CATGUI
/// 
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------
#ifndef CATAppButton_H_
#define CATAppButton_H_

#include "CATButton.h"

/// \class CATAppButton CATAppButton.h
/// \brief Specialized pushbutton for launching apps
/// \ingroup CATGUI
///
/// AppButtons automatically pick up the icon of the executable they are
/// set to, or disable themselves if they cannot find it.
///
/// A few ways to configure AppButtons for the executable.
/// Currently supported attributes:
///     -# "AppPath" attribute may contain full absolute path to the .exe
///     -# "UninstallId" may be used to get the uninstall path.  This is the
///        subkey under HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall
///        for the game's uninstaller.  InstallLocation should contain the base
///        directory.
///        Use "AppFile" for the relative path from the install location of the
///        executable to use.
///
class CATAppButton : public CATButton
{
    public:
        CATAppButton(   const CATString&             element, 
                        const CATString&             rootDir);

        virtual ~CATAppButton();          

        /// ParseAttributes() parses the known attributes for an object.
        virtual CATResult ParseAttributes();

        /// Draw() draws the control into the parent's image
        /// \param image - parent image to draw into
        /// \param dirtyRect - portion of control (in window coordinates)
        ///        that requires redrawing.
        virtual void   Draw(CATImage* image, const CATRect& dirtyRect);

        virtual CATResult Load(CATPROGRESSCB progressCB     = 0, 
                               void*         progressParam  = 0, 
                               CATFloat32    progMin        = 0.0f, 
                               CATFloat32    progMax        = 0.0f);

        virtual void      Reset();
    protected:
        CATString   fAppPath;
        CATString   fAppName;
        CATICON     fIcon;
        CATInt32    fBorder;
        CATImage*   fIconImage;
        CATImage*   fIconDisabled;
};

#endif // CATAppButton_H_


