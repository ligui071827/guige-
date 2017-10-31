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
#ifndef IFXUI_ENGINE_PORTING_H
#define IFXUI_ENGINE_PORTING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "inflexionui/engine/ifxui_defs.h"

/*--------------------------------Memory robustness---------------------------------*/

/* #define IFX_MEMORYTEST_STARTUP   // Perform exhaustive memory failure checks during startup */
/* #define IFX_MEMORYTEST_DYNAMIC   // Perform randomized memory failure checks after startup  */

#if defined(IFX_MEMORYTEST_DYNAMIC) && defined(IFX_MEMORYTEST_STARTUP)
    #error "Both IFX_MEMORYTEST_DYNAMIC and IFX_MEMORYTEST_STARTUP are defined - choose at most one"
#endif /* IFX_MEMORYTEST_DYNAMIC */

#ifdef IFX_MEMORYTEST_STARTUP
/*-------------------------------------------------------------------------*//**
  Startup memory test harness config
*/

    #define IFX_MEMORYTEST_LOGGING   /* Platform-specific test logging */

    extern int  g_currentTestProcessTimer;
    extern int  g_startupTest_allocsUntilFail;
    extern int  g_startupTest_testComplete;
    extern unsigned int g_testCount;
	
	IFX_RETURN_STATUS   IFXP_Get_MemoryAvailable(IFX_UINT32* memoryAvailable); 
#endif /* IFX_MEMORYTEST_STARTUP */

#ifdef IFX_MEMORYTEST_DYNAMIC
/*-------------------------------------------------------------------------*//**
  Dynamic memory test harness config
*/

    #define IFX_MEMORYTEST_LOGGING   /* Platform-specific test logging */
    #define IFX_MEMORYTEST_SCREENSHOT_PATH     "C:/temp2/screen/"

    typedef enum
    {
         EIfxMemoryTestStateUnknown = 0,
         EIfxMemoryTestStateStartup,
         EIfxMemoryTestStateNormal,
         EIfxMemoryTestStateBeforeSnapshot,
         EIfxMemoryTestStateWaitForCanvasRepaint,
         EIfxMemoryTestStateAfterSnapshot,
         EIfxMemoryTestStateComplete
     } EIfxMemoryTestState;

     extern int                 g_dynamicTest_chanceOfAllocFail;
     extern unsigned int        g_dynamicTest_currentAllocId;
     extern EIfxMemoryTestState g_dynamicTest_state;
     extern int                 g_dynamicTest_numberOfFailuresToTest;
     extern unsigned int        g_dynamicTest_snapshotId;
     extern int                 g_dynamicTest_startSeed;
#endif /* IFX_MEMORYTEST_DYNAMIC */

#ifdef IFX_MEMORYTEST_LOGGING
/*-------------------------------------------------------------------------*//**
  Memory test harness logging config
*/
  /* Choose one output method: */
     #ifdef NU_SIMULATION
          #define IFX_MEMORYTEST_LOGGING_PRINTF  /* Output via printf */
     #else
           #define IFX_MEMORYTEST_LOGGING_SERIAL  /* Output via serial */
     #endif
#endif /* IFX_MEMORYTEST_LOGGING */

#ifndef IFX_MEMORYTEST_LOGGING
     #define IFX_MEMORYTEST_LOG(m)
     #define IFX_MEMORYTEST_LOG_N(m,n)
#endif /* !IFX_MEMORYTEST_LOGGING */

#ifdef IFX_MEMORYTEST_LOGGING
     #ifdef IFX_MEMORYTEST_LOGGING_PRINTF
          #define IFX_MEMORYTEST_LOG(m)    lc_printf(m)
          #define IFX_MEMORYTEST_LOG_N(m,n)  lc_printf(m,n)
     #endif /* IFX_MEMORYTEST_LOGGING_PRINTF */

     #ifdef IFX_MEMORYTEST_LOGGING_SERIAL
          extern  void serialPrintString(char* s); /* LC_IMPORT */
          extern  void serialPrintStringAndNumber(char* s, int n); /* LC_IMPORT */

          #define IFX_MEMORYTEST_LOG(m)    serialPrintString(m)
          #define IFX_MEMORYTEST_LOG_N(m,n)  serialPrintStringAndNumber(m,n)
     #endif /* IFX_MEMORYTEST_LOGGING_SERIAL */
#endif /* IFX_MEMORYTEST_LOGGING */

/*--------------------------------Memory robustness---------------------------------*/

#if defined(IFX_GENERATE_SCRIPTS) || defined (IFX_USE_SCRIPTS)
    typedef enum
    {
        TouchEventDown = 0,
        TouchEventMove,
        TouchEventUp,
        TouchEventInvalid
    } TouchEventType;

    typedef enum
    {
        KeyEventDown = 0,
        KeyEventUp,
        KeyEventInvalid
    } KeyEventType;
    typedef enum
    {
        KeyEvent = 0,
        TouchEvent,
        CaptureScreenEvent,
        InvalidEvent
    } TestScriptEventType;
#endif /*IFX_GENERATE_SCRIPTS */

#if defined (IFX_USE_SCRIPTS) || defined(IFX_GENERATE_SCRIPTS)
/* Functions used to set and get Testing Framework Time. */
IFX_RETURN_STATUS    IFXE_Set_Test_Time       (IFX_UINT32 timeUpper,  IFX_UINT32 timeLower);
IFX_RETURN_STATUS    IFXE_Get_Test_Time       (IFX_UINT32* timeUpper, IFX_UINT32* timeLower);
#endif

#if defined (IFX_GENERATE_SCRIPTS)
IFX_RETURN_STATUS    IFXE_Update_Test_Time    (IFX_UINT32 timeUpper, IFX_UINT32 timeLower);
IFX_RETURN_STATUS    IFXE_Increment_Test_Time (IFX_UINT32 timeUpper, IFX_UINT32 timeLower);
#endif

#if defined (IFX_USE_SCRIPTS)
void                 AT_Set_Engine_Restart    (void);
#endif

#ifdef __cplusplus
}
#endif
#endif /* IFXUI_ENGINE_PORTING_H */
