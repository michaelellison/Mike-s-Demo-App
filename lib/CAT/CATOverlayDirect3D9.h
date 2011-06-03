/// \file  CATOverlayDirect3D9.h
/// \brief Direct3D function interception
/// \ingroup CAT
///
/// Copyright (c) 2007-2008 by Michael Ellison.
/// See COPYING.txt for the \ref gaslicense License (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $

#ifndef _CATOverlayDirect3D_H_
#define _CATOverlayDirect3D_H_

#include "CATOverlay.h"
#ifdef CAT_CONFIG_WIN32

struct IDirect3DDevice9;
struct _D3DPRESENT_PARAMETERS_;
struct IDirect3DTexture9;
struct ID3DXSprite;

/// \class CATOverlayDirect3D9
/// \brief Direct3D function interception
/// \ingroup CAT
class CATOverlayDirect3D9 : public CATOverlay
{
    public:
        CATOverlayDirect3D9();
        virtual ~CATOverlayDirect3D9();

        CATResult HookFunctions();

    protected:        
        static void DeviceLost( CATHOOK* hookInst, IDirect3DDevice9* device);
        static void DeviceOk  ( CATHOOK* hookInst, IDirect3DDevice9* device);

        void DrawToScene(    IDirect3DDevice9*           device);
        void Reset(          IDirect3DDevice9*           device,
                             _D3DPRESENT_PARAMETERS_*    presParams);

        static CATINTERCEPT_COM_TABLE_ENTRY kDirect3DDeviceInterceptTable9[];    ///< COM functions for IDirect3DDevice9 
                                                                                 ///< to hook.

        static void     OnReset9(       CATHOOK*                    hookInst,
                                        IDirect3DDevice9*           device,
                                        _D3DPRESENT_PARAMETERS_*    presParams);

        static void     OnPresent9  (   CATHOOK*                    hookInst,
                                        IDirect3DDevice9*           device,
                                        const RECT*                 sourceRect,
                                        const RECT*                 destRect,
                                        HWND                        destWindow,
                                        const RGNDATA*              pDirtyRgn);

        static void     OnEndScene  (   CATHOOK*                    hookInst,
                                        IDirect3DDevice9*           device);

    protected:
        bool               fDeviceLost;
        CATFloat32         fTexScaleX;
        CATFloat32         fTexScaleY;
        IDirect3DTexture9* fBackTexture;
        IDirect3DTexture9* fTexture;
        HMODULE            fD3d9DLL;
};

#endif // CAT_CONFIG_WIN32
#endif // _CATOverlayDirect3D_H_