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
#ifndef NdhsCCfgRulesH
#define NdhsCCfgRulesH

#include "ifxui_config.h"

/* Conditional defines */

/* Verify that we have at least one file access method defined. */
#if !defined (IFX_USE_ROM_FILES) && !defined (IFX_USE_PLATFORM_FILES)

    #error You must define at least one of IFX_USE_ROM_FILES or IFX_USE_PLATFORM_FILES

#endif

#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) && defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)

    #warning "Frame rate and heap usage benchmarking modes are both enabled.  Please disable one."

#endif

/* Uniform frame interval for autotesting */
#if defined(IFX_GENERATE_SCRIPTS) || defined(IFX_USE_SCRIPTS) || \
    defined(IFX_FAST_TEST_MODE)
    
	#undef IFX_FRAME_INTERVAL
	#define IFX_FRAME_INTERVAL 30
	
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

    /* Wait time in seconds before restarting script execution
       after external events end.
    */
    #define EXTERNAL_EVENTS_TIMEOUT 30

#endif

/* Fast mode script execution needs script executor. */
#ifdef IFX_FAST_TEST_MODE

    #ifndef IFX_USE_SCRIPTS
        #define IFX_USE_SCRIPTS
    #endif

#endif

/* Script generation and execution do not work at one time. */
#if defined(IFX_USE_SCRIPTS) && defined(IFX_GENERATE_SCRIPTS)

    #error "Script generation and execution both enabled. Please disable one."

#endif

/* Platform file support should be enabled to generate or execute script. */
#ifndef IFX_USE_PLATFORM_FILES

    #if defined(IFX_GENERATE_SCRIPTS) || defined(IFX_USE_SCRIPTS)

        #error "Script generation or execution needs PLATFORM file support.\
 Enable IFX_USE_PLATFORM_FILES to generate or execute scripts."

    #endif

    #if defined(IFX_GENERATE_SCRIPTS)
        #undef IFX_GENERATE_SCRIPTS
    #endif

    #if defined(IFX_USE_SCRIPTS)
        #undef IFX_USE_SCRIPTS
    #endif

#endif

#if defined(IFX_GENERATE_SCRIPTS) || defined(IFX_USE_SCRIPTS)

    #ifdef IFX_USE_ROM_FILES
        #error "ROM files support should be disabled in script modes."
    #endif

#endif

/* Set up debug mode */
#ifdef IFX_DEBUG_MODE

    #define IFX_USE_CLEANUP_STACK
    #undef IFX_LOG_MAIN
    #define IFX_LOG_VERBOSE

#endif 

/* Uniform frame interval for auto testing */
#if defined(IFX_GENERATE_SCRIPTS) || defined(IFX_USE_SCRIPTS) || defined(IFX_FAST_TEST_MODE)

	#undef IFX_FRAME_INTERVAL
	#define IFX_FRAME_INTERVAL 30

#endif

#endif /* NdhsCCfgRulesH */
