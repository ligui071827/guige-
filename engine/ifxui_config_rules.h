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
#ifndef _IFXUI_CONFIG_RULES_
#define _IFXUI_CONFIG_RULES_

#include "ifxui_config.h"

/* Platfrom specific conditional defines */

/* The defines configure the default behavior of UI Engine.*/
#if defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20)

	/* Use dithering for , this will be enabled only when required. */
	#define IFX_USE_DITHERING

	/* By default enable EGL when OpenGL is in use */
	#define IFX_USE_EGL

#endif /* defined(IFX_RENDER_DIRECT_OPENGL) */


#ifndef IFX_BASE_PACKAGE_DRIVES

    /* Define the drive search order for the application (base package). */
    /* By default this is the same as the package drives, but may be
       modified where the base package search should be different.
    */
    #define IFX_BASE_PACKAGE_DRIVES     IFX_PACKAGE_DRIVES

#endif

/* Demo mode script generation needs script generator. */
#ifdef IFX_GENERATE_DEMOMODE_SCRIPT

    #ifndef IFX_GENERATE_SCRIPTS
        #define IFX_GENERATE_SCRIPTS
    #endif

#endif

/* Demo mode script execution needs script executor. */
#ifdef IFX_EXECUTE_DEMOMODE_SCRIPT

    #ifndef IFX_USE_SCRIPTS
        #define IFX_USE_SCRIPTS
    #endif
#endif

/* Fast mode script execution needs script executor. */
#ifdef IFX_FAST_TEST_MODE

    #ifndef IFX_USE_SCRIPTS
        #define IFX_USE_SCRIPTS
    #endif

#endif

/* Enable this for storing captured images under ATF in png format. */
/* #define ATF_SAVE_PNG */

#endif /* _IFXUI_CONFIG_RULES_ */
