/***************************************************************************
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
*       ifxui_config.h
*
*   COMPONENT
*
*       Inflexion UI
*
*   DESCRIPTION
*
*       Inflexion UI Configurations.
*
*   DATA STRUCTURES
*
*       None.
*
*   DEPENDENCIES
*
*       None.
*
*************************************************************************/

#ifndef IFXUI_CONFIG_H
#define IFXUI_CONFIG_H

/*
    Define the display mode.

    NOTE: Select only one display mode at a time.

    IFX_RENDER_DIRECT_OPENGL
    IFX_RENDER_DIRECT_OPENGL_20
    IFX_RENDER_INTERNAL
    IFX_RENDER_INTERNAL_COMPRESSED
*/
/* This define is configured in rules.mk on a per build target basis */
/* #define IFX_RENDER_INTERNAL */

/*
    Enable debug mode. This will produce verbose logging,
    and enable additional memory management.
*/
/* #define     IFX_DEBUG_MODE */
/*
    Use these defines to force the bit depth that the canvas shall use

    Note that IFX_CANVAS_MODE_888 is the most efficient within the Engine
    however the overall performance depends on how efficient the output
    to display will be within the porting layer if the canvas and
    display color modes do not match.
    By default the display depth is used to work out the best canvas mode.

    NOTE: Select only one canvas mode at a time.
          The canvas mode is automatically determined from the display
          BPP at the end of this file.
          Use a define here to over-ride it.

    IFX_CANVAS_MODE_8888
    IFX_CANVAS_MODE_888
    IFX_CANVAS_MODE_565
    IFX_CANVAS_MODE_1555
    IFX_CANVAS_MODE_444
*/
/* This define is configured in rules.mk on a per build target basis */
/* #define IFX_CANVAS_MODE_888 */

/*
    Define the canvas color order.

    IFX_CANVAS_ORDER_RGB
    IFX_CANVAS_ORDER_BGR
*/
#define IFX_CANVAS_ORDER_RGB

/* Include support for compiled-in files */
//#define IFX_USE_ROM_FILES

/* Include support for files located on some platform-specific file system */
#define IFX_USE_PLATFORM_FILES
#define IFX_PLAT_DIR_SEP            "/"
#define IFX_PLAT_DIR_SEP_CHAR       '/'

/*
    Define the drive search order for packages.
    The platform path for each drive is mapped in the porting layer.
    When IFX_USE_ROM_FILES is defined, Z is reserved for ROM files.
*/
#define IFX_PACKAGE_DRIVES          "C,B,Z"

/* Include if benchmarking frame rate / page opening times. */
/* #define IFX_ENABLE_BENCHMARKING_FRAME_RATE */

/* Include if benchmarking heap and stack usage - note that
    this will significantly affect frame rate.
*/
/* #define IFX_ENABLE_BENCHMARKING_HEAP_USAGE */

/* Include if generating script for demo mode. */
/* #define IFX_GENERATE_DEMOMODE_SCRIPT */

/* Include if executing script in demo mode. */
/* #define IFX_EXECUTE_DEMOMODE_SCRIPT */

/* Include if generating script for autotesting mode. */
/* #define IFX_GENERATE_SCRIPTS */

/* Include if executing autotesting mode. */
/* #define IFX_USE_SCRIPTS */

/* Include to run autotesting in fast mode. */
/* #define IFX_FAST_TEST_MODE */

/* Enable support for bitmapped fonts. */
#define IFX_USE_BITMAPPED_FONTS

/* Enable support for native fonts. */
/* #define IFX_USE_NATIVE_FONTS */

/* Enable support for plugin elements. */
#define IFX_USE_PLUGIN_ELEMENTS

/* Enable project validation against its signature. */
/* Note that this will cause delays on startup and page
   navigation if enabled */
/* #define IFX_VALIDATE_PROJECTS */

/* Specify the frame interval of the space in milliseconds. */
/* 1000/IFX_FRAME_INTERVAL is the target frames-per-second */
#define IFX_FRAME_INTERVAL 16

/* Enable support for a big endian platform. */
/* #define IFX_USE_BIG_ENDIAN */

/* Enable Stylus support. */
#define IFX_USE_STYLUS

/* Enable Mouseover displacement support */
#define IFX_USE_MOUSEOVER

/*
    Define the log level.

    NOTE: Select only one log level at a time.

    IFX_LOG_MAIN
    IFX_LOG_VERBOSE
*/
/* #define     IFX_LOG_MAIN */

#endif /* !def IFXUI_CONFIG_H */

