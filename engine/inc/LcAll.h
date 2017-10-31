/***************************************************************************
*
*               Copyright 2006 Mentor Graphics Corporation
*                         All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
****************************************************************************/
#ifndef LcAllH
#define LcAllH

// This is the public definitions file that also defines the platform defines
// on some platforms.

// Configuration file
#include "ifxui_config.h"

/* 'generic' rules header */
#include "inflexionui/engine/inc/NdhsCCfgRules.h"


// External APIs
#include "inflexionui/engine/ifxui_defs.h"

// Porting Layer Support
extern "C"
{
	#include "inflexionui/engine/ifxui_porting.h"
}

// Definitions that may affect other inclusions
// This modifies platform symbols and pulls in project/profile defs
#include "inflexionui/engine/inc/LcDefs.h"

/*-------------------------------------------------------------------------*//**
	Core NDE headers.  Include this file, and ONLY this file, at the head of
	each source file in an NDE project, typically followed by a #pragma hdrstop.

	Core classes are defined as those which are interdependent and require
	each other to link.  Other components of NDE may be selectively included
	in an NDE build, and these are configured in LcProfileHdrs.h.  Also,
	additional project headers may be included in LcProjectHeaders.h.  Both of
	these are pulled in by this file, which ensures that precompiled headers
	can work consistently even when a project build contains statically-linked
	NDE classes.

	Note that core headers are listed in the order as documented in the file
	inventory in practice they will include each other in dependency order.
*/

// Core types, containers, math and cleanup support
#include "inflexionui/engine/inc/LcTAlloc.h"
#include "inflexionui/engine/inc/LcTAuto.h"
#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcCCleanup.h"
#include "inflexionui/engine/inc/LcTColor.h"
#include "inflexionui/engine/inc/LcTOwner.h"
#include "inflexionui/engine/inc/LcTPixelDim.h"
#include "inflexionui/engine/inc/LcTPixelPoint.h"
#include "inflexionui/engine/inc/LcTPixelRect.h"
#include "inflexionui/engine/inc/LcTPlaneRect.h"
#include "inflexionui/engine/inc/LcTQuaternion.h"
#include "inflexionui/engine/inc/LcTScalarRect.h"
#include "inflexionui/engine/inc/LcTScalarQuad.h"
#include "inflexionui/engine/inc/LcStl.h"
#include "inflexionui/engine/inc/LcTString.h"
#include "inflexionui/engine/inc/LcCSerializeMaster.h"
#include "inflexionui/engine/inc/LcTTime.h"
#include "inflexionui/engine/inc/LcTTransform.h"
#include "inflexionui/engine/inc/LcTTransform4.h"
#include "inflexionui/engine/inc/LcTVector.h"




// Core NDE framework
#include "inflexionui/engine/inc/LcCAggregate.h"
#include "inflexionui/engine/inc/LcCAnimator.h"
#include "inflexionui/engine/inc/LcCApplet.h"
#include "inflexionui/engine/inc/LcCBitmap.h"
#include "inflexionui/engine/inc/LcCEnvironment.h"
#include "inflexionui/engine/inc/LcCFont.h"
#include "inflexionui/engine/inc/LcIImage.h"
#include "inflexionui/engine/inc/LcTMessage.h"
#include "inflexionui/engine/inc/LcTPlacement.h"
#include "inflexionui/engine/inc/LcCReadOnlyFile.h"
#include "inflexionui/engine/inc/LcIResourceManager.h"
#include "inflexionui/engine/inc/LcCSpace.h"
#include "inflexionui/engine/inc/LcCText.h"

#include "inflexionui/engine/inc/LcITimer.h"
#include "inflexionui/engine/inc/LcCWidget.h"
#include "inflexionui/engine/inc/LcTWidgetEvent.h"
#include "inflexionui/engine/inc/LcCFileSearch.h"
#include "inflexionui/engine/inc/LcCMutex.h"
#include "inflexionui/engine/inc/LcCNdiBitmapFile.h"
#include "inflexionui/engine/inc/LcCCustomBitmapFile.h"
#include "inflexionui/engine/inc/LcCNdiGraphics.h"

// Optional core feature sets
#ifdef IFX_USE_ROM_FILES
	#include "inflexionui/engine/inc/LcCRomFileSearch.h"
#endif

#ifdef IFX_USE_PLATFORM_FILES
	#include "inflexionui/engine/inc/LcCPlatformFileSearch.h"
#endif

#ifdef LC_USE_XML
	#include "inflexionui/engine/inc/LcCXml.h"
#endif

#ifdef LC_USE_MESHES
	#include "inflexionui/engine/inc/LcCMesh.h"
#endif

#ifdef IFX_USE_NATIVE_FONTS
	#include "inflexionui/engine/inc/LcCNativeFont.h"
	#include "inflexionui/engine/inc/LcCNativeText.h"
#endif

#ifdef IFX_USE_BITMAPPED_FONTS
	#include "inflexionui/engine/inc/LcCBitmappedFont.h"
	#include "inflexionui/engine/inc/LcCBitmappedText.h"
	#include "inflexionui/engine/inc/LcCNdiFont.h"
	#include "inflexionui/engine/inc/LcCNdiText.h"
#endif

#ifdef LC_USE_LIGHTS
	#include "inflexionui/engine/inc/LcCLight.h"
#endif

// Core and optional feature set platform classes
#ifdef LC_PLAT_OGL
	#include "inflexionui/engine/inc/LcOglCGlobalState.h"
	#include "inflexionui/engine/inc/LcOglCBitmap.h"
	#include "inflexionui/engine/inc/LcOglCContext.h"
	#include "inflexionui/engine/inc/LcOglCNdiBitmap.h"
	#include "inflexionui/engine/inc/LcOglCCustomBitmap.h"
	#include "inflexionui/engine/inc/LcOglCTexture.h"
	#include "inflexionui/engine/inc/LcOglCGPUCaps.h"

	#ifdef LC_PLAT_OGL_20
		#include "inflexionui/engine/inc/LcOglCSLType.h"
		#include "inflexionui/engine/inc/LcOglCSLTypeScalar.h"
		#include "inflexionui/engine/inc/LcOglCSLTypeVector.h"
		#include "inflexionui/engine/inc/LcOglCSLTypeMatrix.h"
		#include "inflexionui/engine/inc/LcOglCEffect.h"
		#include "inflexionui/engine/inc/LcOglCCubeTexture.h"
	#endif

	#ifdef IFX_USE_BITMAPPED_FONTS
		#include "inflexionui/engine/inc/LcOglCFont.h"
		#include "inflexionui/engine/inc/LcOglCNdiFont.h"
		#include "inflexionui/engine/inc/LcOglCText.h"
	#endif

	// OGL Native fonts.
	#ifdef IFX_USE_NATIVE_FONTS
		#include "inflexionui/engine/inc/LcOglCNativeFont.h"
		#include "inflexionui/engine/inc/LcOglCNativeText.h"
	#endif

	#ifdef LC_USE_MESHES
		#include "inflexionui/engine/inc/LcOglCMesh.h"
	#endif

	#ifdef LC_USE_LIGHTS
		#include "inflexionui/engine/inc/LcOglCLight.h"
	#endif
#else
	#include "inflexionui/engine/inc/LcCNdiBitmap.h"
	#include "inflexionui/engine/inc/LcCCustomBitmap.h"
	// NDI Native fonts.
	#ifdef IFX_USE_NATIVE_FONTS
		#include "inflexionui/engine/inc/LcCNdiNativeFont.h"
		#include "inflexionui/engine/inc/LcCNdiNativeText.h"
	#endif
#endif

/*-------------------------------------------------------------------------*//**
	Additional non-core components in the NDE build (e.g. widgets).
	If NDE core is linked statically, additional components should be
	included via project headers instead.
*/
#ifndef LC_USE_STATIC_NDE
	#include "inflexionui/engine/inc/LcProfileHdrs.h"
#endif

/*-------------------------------------------------------------------------*//**
	Additional components not in the NDE build - included here for the
	purpose of keeping precompiled headers consistent
*/
#ifndef LC_BUILD_NDE_DLL

	// Disable import directives so that we can statically link NDE
	// modules if we wish to by including them in project headers.
	#undef	LC_IMPORT
	#define LC_IMPORT
	#undef	LC_VIRTUAL
	#define LC_VIRTUAL virtual
	#undef	LC_EXPORT
	#define LC_EXPORT
	#undef	LC_EXPORT_VIRTUAL
	#define LC_EXPORT_VIRTUAL

	// Additional headers here ... this enables them to be included in
	// precompiled headers even when statically linking NDE.  Note that
	// application includes must be searched before NDE include
	#include "LcProjectHdrs.h"

#endif //LC_BUILD_NDE_DLL

// Inflexion Player specific defines.
#if defined(IFX_WIN_PLAYER)
	#include <Windows.h>
	#include <gdiplus.h>
	#pragma comment(lib, "gdiplus.lib")
#endif

#endif //LcAllH
