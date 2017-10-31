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
* FILE NAME
*
*       framework_linux_x11.c
*
* COMPONENT
*
*       Application Framework
*
* DESCRIPTION
*
*       This module provides the main entry point for an external
*       application.
*
*       Any hooks required between the application framework and
*       porting layer should be implemented in this module.
*
* DATA STRUCTURES
*
*       None
*
* FUNCTIONS
*
*       IFXF_Initialize                     Main hook into the framework.
*                                           Called by the application, initializes
*                                           the framework and UI engine.
*
*       IFXF_Shutdown                       Shutdown the engine and framework.
*
*       IFXF_Schedule_Timer                 Hook into the framework from the porting
*                                           layer to inform it when to schedule the
*                                           next event.
*
*       IFXF_Suspend                        Puts the engine into a low memory state.
*
*       IFXF_Exit                           Causes the framework to exit and shutdown
*                                           the engine.
*
* DEPENDENCIES
*
*       ifxui_engine.h                      UI Engine API.
*
*       framework_linux.h                   Linux framework include.
*
*       ifxui_porting_linux.h               Linux porting include.
*
*************************************************************************/

/* Linux Framework and Engine includes. */
#include    "inflexionui/framework/inc/framework_linux.h"
#include    "inflexionui/porting/inc/ifxui_porting_linux.h"
#include    "inflexionui/engine/ifxui_engine.h"
#include    "ifxui_config.h"

/* Maximum number of events to process before continuing */
#define     PL_MAX_EVENTS                   25

/* Message queue. */
int         IFXF_Message_Queue_ID = -1;

/* Global variable declarations. */
static      struct timeval timer_alarm_val;
static      IFX_UINT32 timer_alarm;
static      IFXE_CONFIGURATION engine_config;

static int g_done = 0;

/* External functions. */
extern int PL_Process_Input_Events(void);

void sig_handler (int signal)
{
    g_done = 1;
}

/* Compare time1 to time2. */
int compare_time(struct timeval time1, struct timeval time2)
{
    if (time1.tv_sec < time2.tv_sec)
        return -1;
    else if (time1.tv_sec > time2.tv_sec)
        return 1;
    else if (time1.tv_usec < time2.tv_usec)
        return -1;
    else if (time1.tv_usec > time2.tv_usec)
        return 1;
    else
        return 0;
}

/*************************************************************************
*   FUNCTION
*
*       IFXF_Initialize
*
*   DESCRIPTION
*
*       This function initializes the framework and UI Engine.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXF_Initialize (void)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    /* Create Message Queue. */
    IFXF_Message_Queue_ID = msgget(IFXF_EVENT_MESSAGE_QUEUE_KEY,
                                   IPC_CREAT | S_IRUSR | S_IWUSR);
    if (IFXF_Message_Queue_ID < 0)
    {
        printf("\n Event Queue Creation Failed (%d).\n", errno);
        return IFX_ERROR;
    }

    /* Initialize the timer to off. */
    timer_alarm = ALARM_OFF;

    if (status == IFX_SUCCESS)
    {
        /* Set the default configuration */

        memset(&engine_config, 0, sizeof(IFXE_CONFIGURATION));
        lc_strcpy(engine_config.language, "en");
        engine_config.left = 0;
        engine_config.top  = 0;

        /* Set the engine configuration based on the screen dimensions */

#if         defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || \
            defined (IFX_ENABLE_BENCHMARKING_HEAP_USAGE)

        /* Configure the engine for benchmarking */
        lc_strncpy(engine_config.display_mode, IFXF_BENCHMARK_SCREEN_MODE, IFX_MAX_CONFIG_STRING_LENGTH);
        engine_config.left = IFXF_BENCHMARK_SCREEN_POS_X;
        engine_config.top  = IFXF_BENCHMARK_SCREEN_POS_Y;

#else       /* defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) ||
               defined (IFX_ENABLE_BENCHMARKING_HEAP_USAGE) */

        /* The engine will automatically choose the default display mode
           Uncomment and modify the following line to force a mode */

       /* lc_strcpy(engine_config.display_mode, "VGA_P"); */



#endif      /* defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) ||
               defined (IFX_ENABLE_BENCHMARKING_HEAP_USAGE) */
    }

    /* Initialize the Inflexion UI for Linux Engine */

    if (status == IFX_SUCCESS)
    {
        status = IFXE_Initialize(&engine_config);
    }

    return status;
}


/*************************************************************************
*   FUNCTION
*
*       IFXF_Shutdown
*
*   DESCRIPTION
*
*       This function shuts down UI Engine and framework.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXF_Shutdown (void)
{
    /* Shutdown the system. */

    printf("Shutting down the framework...\n");
    IFXE_Shutdown();

    /* Close the message queue */
    msgctl(IFXF_Message_Queue_ID, IPC_RMID, NULL);
    IFXF_Message_Queue_ID = -1;

    /* system("/sbin/poweroff"); */
    return  IFX_SUCCESS;
}

/*************************************************************************
*   FUNCTION
*
*       IFXF_Suspend
*
*   DESCRIPTION
*
*       This function puts the engine into a low memory mode.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXF_Suspend (void)
{
    return IFX_SUCCESS;
}

/*************************************************************************
*   FUNCTION
*
*       IFXF_Exit
*
*   DESCRIPTION
*
*       This function shuts down the engine.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXF_Exit (void)
{
    IFX_UINT32    engine_msg[IFXF_QUEUE_ELEMENT_SIZE] = {0};
    int           send_status = 0;

    engine_msg[0] = IFXFShutdown;

    /* Send the event in message queue. */
    send_status = msgsnd(IFXF_Message_Queue_ID, engine_msg,
                         IFXF_QUEUE_ELEMENT_SIZE * sizeof(IFX_UINT32),0);
    if (send_status < 0)
    {
        printf("Failed to send message to engine (%d)\n", errno);
    }

    return IFX_SUCCESS;
}

/*************************************************************************
*   FUNCTION
*
*       IFXF_Schedule_Timer
*
*   DESCRIPTION
*
*       This function sets the time for the next Engine event.
*       It is used as a hook from the porting layer into the framework.
*
*   INPUTS
*
*       time_upper                          System time in ms to schedule
*                                           the next event (upper DWORD).
*       time_lower                          System time in ms to schedule
*                                           the next event (lower DWORD).
*
*   OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXF_Schedule_Timer(IFX_UINT32 time_upper,
                                        IFX_UINT32 time_lower)
{
    struct timeval local_time;
    local_time.tv_sec = (time_upper *(UINT_MAX/1000));
    local_time.tv_sec += time_lower / 1000;
    local_time.tv_usec = (time_lower % 1000) * 1000;

    if ( (timer_alarm == ALARM_OFF)
        || (compare_time(local_time, timer_alarm_val) < 0 ) )
    {
        timer_alarm_val = local_time;
        timer_alarm = ALARM_ON;
    }

    return IFX_SUCCESS;
}

/*************************************************************************
*   FUNCTION
*
*       Event_Handler_Task
*
*   DESCRIPTION
*
*       This is the UI Engine function.
*       It retrieves input events from the event subsystem and forwards
*       them to the engine.
*       The Engine is driven by calling IFXE_Process_Timers at regular
*       intervals, or as scheduled by IFXF_Schedule_Timer.
*
*   INPUTS
*
*      None.
*
*   OUTPUTS
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXF_Event_Handler(void)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;
    IFX_UINT32 current_time_upper;
    IFX_UINT32 current_time_lower;
    struct timeval local_time;
    IFX_UINT32 msg_buf[IFXF_QUEUE_ELEMENT_SIZE];
    int        recv_status;

#if defined (IFX_USE_SCRIPTS)
    IFX_UINT32 sec_sleep_time;
    IFX_UINT32 us_sleep_time;
#endif

    /* Deal with Signals */
    signal(SIGUSR1, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    /* Main event-reading loop */

    g_done = 0;
    while (g_done == 0)
    {

        /* Process incoming event. */
        g_done = PL_Process_Input_Events();

        recv_status = 1;
        while (recv_status > 0)
        {
            /* Retrieve a message from the queue. */
            recv_status = msgrcv(IFXF_Message_Queue_ID, msg_buf,
                                 IFXF_QUEUE_ELEMENT_SIZE * sizeof(IFX_UINT32),
                                 0, IPC_NOWAIT);
            if (recv_status > 0)
            {

                switch (msg_buf[0])
                {
                    case IFXFTimerExpire:
                        IFXP_Timer_Get_Current_Time(&current_time_upper, &current_time_lower);
                        IFXE_Process_Timers(current_time_upper, current_time_lower);
                        current_time_upper = 0;
                        current_time_lower = 0;
                    break;

                    case IFXFSuspend:
                        IFXE_Suspend();
                    break;

                    case IFXFRefreshDisplay:
                        IFXE_Refresh_Display();
                    break;

                    case IFXFKeyDown:
                        IFXE_Key_Down(msg_buf[1]);
                    break;

                    case IFXFKeyUp:
                        IFXE_Key_Up(msg_buf[1]);
                    break;

                    case IFXFTouchDown:
                        IFXE_Touch_Down(0, msg_buf[1], msg_buf[2]);
                    break;

                    case IFXFTouchMove:
                        IFXE_Touch_Move(0, msg_buf[1], msg_buf[2]);
                    break;

                    case IFXFTouchUp:
                        IFXE_Touch_Up(0, msg_buf[1], msg_buf[2]);
                    break;

                    case IFXFShutdown:
                        g_done = 1;
                    break;

                    default:
                    break;
                } /* switch */
            } /* Receive event */
        } /* while task running */

        /* Check our timer status. */
        IFXP_Timer_Get_Current_Time(&current_time_upper, &current_time_lower);

#ifdef IFX_GENERATE_SCRIPTS
    #ifndef IFX_GENERATE_DEMOMODE_SCRIPT
        IFXE_Update_Test_Time(current_time_upper, current_time_lower);
        IFXE_Get_Test_Time(&current_time_upper, &current_time_lower);
    #endif
#endif

#ifdef IFX_FAST_TEST_MODE
        /* Forced Timer Expiry */
        current_time_upper = (timer_alarm_val.tv_sec / (UINT_MAX/1000));
        current_time_lower = (timer_alarm_val.tv_sec % (UINT_MAX/1000)) * 1000;
        current_time_lower += timer_alarm_val.tv_usec / 1000;
#endif

        local_time.tv_sec = (current_time_upper *(UINT_MAX/1000));
        local_time.tv_sec += current_time_lower / 1000;
        local_time.tv_usec = (current_time_lower % 1000) * 1000;

        if ((ALARM_ON == timer_alarm) && (compare_time(local_time, timer_alarm_val) >= 0))
        {
            timer_alarm = ALARM_OFF;
            IFXE_Process_Timers(current_time_upper, current_time_lower);
        }

#ifndef IFX_FAST_TEST_MODE
        /* Do not sleep in case of FAST TEST MODE */

        /* Note that the timer alarm may have changed
           during the process timers call */
        if (compare_time(local_time, timer_alarm_val) < 0 || timer_alarm == ALARM_OFF)
        {
            usleep(1000);
        }
#endif
    }

    return status;
}

#if defined (IFX_USE_SCRIPTS) || defined(IFX_GENERATE_SCRIPTS)
/*************************************************************************
*   FUNCTION
*
*       IFXF_Set_Test_Time
*
*   DESCRIPTION
*
*       Set the time of testing framework.
*
*   INPUTS
*
*       timeUpper          Upper part of time
*       timeLower          Lower part of time
*
*   OUTPUTS
*
*       IFX_RETURN_STATUS
*
*************************************************************************/
IFX_RETURN_STATUS IFXF_Set_Test_Time(IFX_UINT32 timeUpper, IFX_UINT32 timeLower)
{
    return(IFXE_Set_Test_Time(timeUpper, timeLower));

}

/*************************************************************************
*   FUNCTION
*
*       IFXF_Get_Test_Time
*
*   DESCRIPTION
*
*       Get the time of testing framework.
*
*   INPUTS
*
*       timeUpper         Pointer to variable that will hold upper part of time
*       timeLower         Pointer to variable that will hold lower part of time
*
*   OUTPUTS
*
*       IFX_RETURN_STATUS
*
*************************************************************************/
IFX_RETURN_STATUS IFXF_Get_Test_Time(IFX_UINT32* timeUpper, IFX_UINT32* timeLower)
{
    return(IFXE_Get_Test_Time(timeUpper, timeLower));

}
#endif
