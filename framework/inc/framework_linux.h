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
*       framework_linux.h
*
*   COMPONENT
*
*       Application Framework
*
*   DESCRIPTION
*
*       Framework to interface UI Engine and Porting layer with the system.
*
*   DATA STRUCTURES
*
*       None.
*
*   DEPENDENCIES
*
*       ifxui_engine.h                      UI Engine API.
*
*************************************************************************/
#ifndef     _FRAMEWORK_LINUX_
#define     _FRAMEWORK_LINUX_

#ifdef __cplusplus
extern "C" {
#endif

/* UI Engine includes */
#include    "inflexionui/engine/ifxui_engine.h"
#include    "inflexionui/engine/ifxui_engine_porting.h"

/* Force input spoofing to be enabled (for demos etc) */
/*#define     FWRK_SPOOF_INPUT_EVENTS */

/* To test for a particular screen size, adjust the following values.
   Typically, frame rates will be better the smaller the output screen is
   for a particular device. */

#define     IFXF_BENCHMARK_SCREEN_MODE      "QVGA_P"
#define     IFXF_BENCHMARK_SCREEN_POS_X     0
#define     IFXF_BENCHMARK_SCREEN_POS_Y     0

#define     ALARM_OFF                       ((IFX_UINT32)-1)
#define     ALARM_ON                        0

/* Framework event queue */
#define     IFXF_EVENT_MESSAGE_QUEUE_KEY    0x1235
#define     IFXF_QUEUE_ELEMENT_SIZE         3

extern int IFXF_Message_Queue_ID;

enum
{
    IFXFKeyDown = 1,
    IFXFKeyUp,
    IFXFTouchDown,
    IFXFTouchUp,
    IFXFTouchMove,
	//xg add start
    IFXFTouchDDown,
    IFXFTouchDUp,
    IFXFTouchDMove,
    IFXFTouchDClickDown,
	//xg add end 
    IFXFTimerExpire,
    IFXFSuspend,
    IFXFShutdown,
    IFXFRefreshDisplay,
    IFXFPowerOff
} IFXF_Enum_Key_Types;

//xg add start
void register_touch_handle(void (*func_pointer)(int touch_state, int x0, int y0, int x1, int y1));
void unregister_touch_handle(void);

void register_gesture_handle(void (*func_pointer)(int touch_state, int x0, int y0));

#if 1
int SetUIGlobalAlphaAndColorKey(const char *pALFBDevice, int iGlobalAlphaEnable, int iGloblAlpha, int iColorKeyEnable, int iColorKey);
#endif
//xg add end

IFX_RETURN_STATUS   IFXF_Initialize     (void);
IFX_RETURN_STATUS   IFXF_Shutdown       (void);
IFX_RETURN_STATUS   IFXF_Schedule_Timer (IFX_UINT32 time_upper, IFX_UINT32 time_lower);
IFX_RETURN_STATUS   IFXF_Suspend        (void);
IFX_RETURN_STATUS   IFXF_Exit           (void);
IFX_RETURN_STATUS   IFXF_Event_Handler  (void);

#if defined (IFX_USE_SCRIPTS) || defined(IFX_GENERATE_SCRIPTS)
/* Functions used to set and get Testing Framework Time. */
IFX_RETURN_STATUS   IFXF_Set_Test_Time  (IFX_UINT32 timeUpper,  IFX_UINT32 timeLower);
IFX_RETURN_STATUS   IFXF_Get_Test_Time  (IFX_UINT32* timeUpper, IFX_UINT32* timeLower);
#endif

#ifdef __cplusplus
}
#endif

#endif      /* _FRAMEWORK_LINUX_ */
