/*************************************************************************
*
*            Copyright 2006 Mentor Graphics Corporation
*                         All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
**************************************************************************

**************************************************************************
*   FILE NAME
*
*       ifxui_porting_includes.h
*
* COMPONENT
*
*       Inflexion UI Porting Layer for Linux OS.
*
* DESCRIPTION
*
*       Platform specific includes for Inflexion UI Engine Porting Layer
*       This file should include the OpenGL/ES or OpenGL headers
*       for your platform.
*
* DATA STRUCTURES
*
*       None
*
* DEPENDENCIES
*
*       None
*
*************************************************************************/
#ifndef     _IFXUI_PORTING_INCLUDES_
#define     _IFXUI_PORTING_INCLUDES_

#include    "ifxui_config_rules.h"

#if         defined(IFX_RENDER_DIRECT_OPENGL)   || \
            defined(IFX_RENDER_BUFFERED_OPENGL) || \
            defined(IFX_RENDER_DIRECT_OPENGL_20)

#if         defined(IFX_USE_EGL)

#include    "egl.h"

#endif      /* defined(IFX_USE_EGL) */

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

#include    "gl2.h"
#include    "gl2ext.h"

#else    /* IFX_RENDER_DIRECT_OPENGL */

#include    "gl.h"
#include    "glext.h"

#endif      /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

#endif

#endif      /* _IFXUI_PORTING_INCLUDES_ */
