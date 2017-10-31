/*************************************************************************
*
*               Copyright 2006 Mentor Graphics Corporation
*                         All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
*************************************************************************/

/*************************************************************************
*   FILE NAME
*
*       LcProjectHdrs.h
*
*   COMPONENT
*
*       Inflexion UI
*
*   DESCRIPTION
*
*       These are the headers of non-core NDE files included in a given
*       NDE build or deployment.
*
*       Do NOT change these if you are using a DLL build of NDE, as the
*       headers will no longer match the compiled version.
*
*       This file is intended to be modified to control different build
*       profiles of the same version of NDE, although note that the NDE
*       DLL must be rebuilt.  Parts of this file may be conditionally
*       included based on platform symbols, and for default configurations
*       of NDE, this is recommended above using different edited copies of
*       files for different platforms. The default version of this file
*       checked in to source control will define default configurations
*       for each platform.
*
*   DATA STRUCTURES
*
*       None.
*
*   DEPENDENCIES
*
*       Includes common header files.
*
*************************************************************************/

#ifndef LcProjectHdrsH

#define LcProjectHdrsH


#include "inflexionui/engine/inc/LcCBitmapFrame.h"

#ifdef LC_USE_MESHES

#include "inflexionui/engine/inc/LcCMesh.h"
#include "inflexionui/engine/inc/LcwCMesh.h"

#endif


#ifdef LC_USE_LIGHTS

#include "inflexionui/engine/inc/LcwCLight.h"

#endif /* LC_USE_LIGHTS */

#include "inflexionui/engine/inc/LcwCImage.h"
#include "inflexionui/engine/inc/LcwCLabel.h"

#ifdef IFX_USE_PLUGIN_ELEMENTS

/* Plugin element support. */
#include "inflexionui/engine/inc/LcwCPlugin.h"

#endif

#include "inflexionui/engine/inc/NdhsDefs.h"

/* Decoration Support. */
#include "inflexionui/engine/inc/NdhsCPage.h"
#include "inflexionui/engine/inc/NdhsCPath.h"
#include "inflexionui/engine/inc/LcCLinearAnimator.h"
#include "inflexionui/engine/inc/LcCMarqueeRenderer.h"
#include "inflexionui/engine/inc/NdhsIPath.h"
#include "inflexionui/engine/inc/NdhsCKeyFrameList.h"
#include "inflexionui/engine/inc/NdhsCMenuCon.h"
#include "inflexionui/engine/inc/NdhsCMenu.h"
#include "inflexionui/engine/inc/NdhsCAppUi.h"
#include "inflexionui/engine/inc/NdhsCMenuItem.h"
#include "inflexionui/engine/inc/NdhsCMenuWidgetFactory.h"
#include "inflexionui/engine/inc/NdhsCElement.h"
#include "inflexionui/engine/inc/NdhsCGraphicElement.h"
#include "inflexionui/engine/inc/NdhsCTextElement.h"

#ifdef LC_USE_STYLUS

#include "inflexionui/engine/inc/NdhsCDragRegionElement.h"

#endif /* LC_USE_STYLUS */

#ifdef LC_USE_LIGHTS

#include "inflexionui/engine/inc/NdhsCLightElement.h"

#endif /* LC_USE_LIGHTS */

#include "inflexionui/engine/inc/NdhsIFieldContext.h"
#include "inflexionui/engine/inc/NdhsIExpressionObserver.h"

#include "inflexionui/engine/inc/NdhsCExpression.h"
#include "inflexionui/engine/inc/NdhsCExpressionFunction.h"
#include "inflexionui/engine/inc/NdhsCExpressionField.h"

#include "inflexionui/engine/inc/NdhsCLaundry.h"
#include "inflexionui/engine/inc/NdhsCElementGroup.h"
#include "inflexionui/engine/inc/NdhsCPageManager.h"
#include "inflexionui/engine/inc/NdhsCPageModel.h"
#include "inflexionui/engine/inc/NdhsCComponent.h"
#include "inflexionui/engine/inc/NdhsCMenuComponent.h"
#include "inflexionui/engine/inc/NdhsCMenuComponentTemplate.h"
#include "inflexionui/engine/inc/NdhsCPageTemplate.h"
#include "inflexionui/engine/inc/NdhsCTransitionAgent.h"
#include "inflexionui/engine/inc/NdhsCProjectValidator.h"
#include "inflexionui/engine/inc/NdhsCTokenStack.h"
#include "inflexionui/engine/inc/NdhsCManifest.h"
#include "inflexionui/engine/inc/NdhsCManifestStack.h"

/* Testing framework support */

#ifdef IFX_GENERATE_SCRIPTS

#include "inflexionui/engine/inc/NdhsCScriptGenerator.h"

#endif /* IFX_GENERATE_SCRIPTS */

#ifdef IFX_USE_SCRIPTS

#include "inflexionui/engine/inc/NdhsCScriptExecutor.h"

#endif /* IFX_USE_SCRIPTS */

#include "inflexionui/engine/inc/NdhsCStateManager.h"
#include "inflexionui/engine/inc/NdhsCEntryPointMapStack.h"

#include "inflexionui/engine/inc/NdhsCFieldCache.h"

/* Plugin Support. */

#include "inflexionui/engine/inc/NdhsCPlugin.h"
#include "inflexionui/engine/inc/ifxui_integration.h"

#include "inflexionui/engine/inc/NdhsCField.h"
#include "inflexionui/engine/inc/NdhsCFieldInt.h"
#include "inflexionui/engine/inc/NdhsCFieldTime.h"
#include "inflexionui/engine/inc/NdhsCFieldFloat.h"
#include "inflexionui/engine/inc/NdhsCFieldBool.h"
#include "inflexionui/engine/inc/NdhsCFieldTime.h"
#include "inflexionui/engine/inc/NdhsCScrollPosField.h"

#ifdef IFX_USE_PLUGIN_ELEMENTS

/* Plugin element support - dependent on IFX_USE_PLUGIN_ELEMENTS! */
#include "inflexionui/engine/inc/NdhsCPluginElement.h"
#include "inflexionui/engine/inc/NdhsCPluginElementView.h"
#include "inflexionui/engine/inc/NdhsCPluginElementDirectView.h"
#include "inflexionui/engine/inc/NdhsCPluginElementBufferedView.h"
#ifdef LC_PLAT_OGL
#include "inflexionui/engine/inc/NdhsCPluginElementOGLTextureView.h"
#endif /* LC_PLAT_OGL */
#include "inflexionui/engine/inc/NdhsCMemoryImage.h"

#endif /* IFX_USE_PLUGIN_ELEMENTS */

#include "inflexionui/engine/inc/LcCTokenReplacer.h"
#include "inflexionui/engine/inc/LcCKeyValueList.h"

#endif /* LcProjectHdrsH */
