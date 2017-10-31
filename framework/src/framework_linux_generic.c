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
*       framework_linux_generic.c
*
* COMPONENT
*
*       Application Framework
*
* DESCRIPTION
*
*       This is a generic implementation for the Linux fbdev and event
*       system.
*
*       This module provides the main entry point for an external
*       application.
*
*       Any hooks required between the application framework and
*       porting layer should be implemented in this module.
*
*       Environment Variables:
*
*       FRAMEBUFFER                         Frame buffer device to open
*                                           Default: /dev/fb0
*
*       KEYPAD                              Keypad input device to open
*                                           Default: /dev/input/event0
*
*       TOUCHPANEL                          Touch panel input device to open
*                                           Default: /dev/input/event1
*
*       TPCAL                               Touch panel calibration values
*                                           (left, right, top, bottom)
*                                           Default: 80, 1935, 1830, 80
*
*       TPSWAP                              Touch panel has X and Y values
*                                           swapped
*                                           Default: 0 (false)
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
*************************************************************************/

/* Linux Framework and Engine includes. */

#include    "inflexionui/engine/ifxui_engine.h"
#include    "ifxui_config.h"
#include    "inflexionui/framework/inc/framework_linux.h"

//xg add on 20150806
#include    "inflexionui/framework/inc/mxcfb.h"

/* Linux fbdev and system includes */

#include    <linux/fb.h>
#include    <linux/input.h>
#include    <termios.h>
#include    <linux/vt.h>
#include    <linux/kd.h>
#include    <signal.h>
#include    <fcntl.h>
#include    <sys/ipc.h>
#include    <sys/msg.h>
#include    <sys/errno.h>
#include    <sys/stat.h>
#include    <dirent.h>
#include    <sys/types.h>
#include    <stdint.h>
#include 	"diag.h"//michael add 20151030

#define     ALARM_OFF                       ((IFX_UINT32)-1)
#define     ALARM_ON                        0

/* Framework event queue */
#define     IFXF_EVENT_MESSAGE_QUEUE_KEY    0x1235
#define     IFXF_QUEUE_ELEMENT_SIZE         3

/* Maximum number of events to process before continuing */
#define     PL_MAX_EVENTS                   25

/* Touch panel default calibration */

#define     TP_LEFT                        80
#define     TP_RIGHT                       1935
#define     TP_TOP                         1830
#define     TP_BOTTOM                      80

#define     NO_PRESSURE_VAL                 0

/* Input device default configuration */
#define     EVENT_INTERFACE_PATH            "/dev/input/"

#define     MAX_NAME_LENGTH               256
#define     MAX_INFO_LENGTH               4096

typedef struct _TS_COORD_STRUCT
{
    int x;
    int y;
} COORD;

//xg add start
//#define DEBUG
#ifdef DEBUG
    //#define dprintf(format, ...) printf("[TOUCH][%s:%d]" format, __func__, __LINE__, ##__VA_ARGS__)
    #define dprintf(format, ...) printf("[TOUCH]" format, ##__VA_ARGS__)
#else
    #define dprintf(format, ...)
#endif

#define MAX_SLOT_NUM		1

typedef struct _TS_SLOT 
{
	int tracking_id; //available value starting from 0
	int x;
	int y;
} SLOT;

void (*touch_handle_func)(int, int, int, int, int);
void (*gesture_handle_func)(int, int, int);
//xg add end

typedef struct _INPUT_DEVICE
{
    char    name[MAX_NAME_LENGTH];
    char    path[IFX_MAX_FILE_NAME_LEN];
    int     fd;
    uint8_t type;

    struct input_absinfo abs[2];

    IFX_BOOL tp_swap_x_y;

    /* Device state with respect to Inflexion */
    int active;
    int x;
    int y;
    COORD coord;
    int dragging;
    int pen_down;
	
	//xg add start
	SLOT slots[2];
	int cur_slot;
	int touch_state;
	//xg add end 

    struct _INPUT_DEVICE* next;

} INPUT_DEVICE;

/* Mouse cursor */
IFX_BOOL PL_cursor_active = IFX_FALSE;
IFX_BOOL PL_mouse_moved = IFX_FALSE;
int  PL_touch_x = 0;
int  PL_touch_y = 0;
int  PL_dragging = 0;
int  PL_pen_down = 0;

/* Message queue. */
int         IFXF_Message_Queue_ID = -1;
#if 1 /*ligui added here*/
static      IFX_UINT32 Previous_Page;
#endif

static      struct timeval timer_alarm_val;
static      IFX_UINT32 timer_alarm;
static      IFXE_CONFIGURATION engine_config;

/* Global variable declarations. */

/* Input device structure */
static INPUT_DEVICE* input_devices = 0;
/* Touch panel calibration values */
static int tp_left   = TP_LEFT;
static int tp_right  = TP_RIGHT;
static int tp_top    = TP_TOP;
static int tp_bottom = TP_BOTTOM;
static IFX_BOOL tp_defined = IFX_FALSE;
#ifdef TP_INVERT_X_Y
static IFX_BOOL tp_swap_x_y = IFX_TRUE;
#else
static IFX_BOOL tp_swap_x_y = IFX_FALSE;
#endif


static int g_done = 0;

extern  IFX_DISPLAY pl_display;

/* Local functions. */

static IFX_RETURN_STATUS   PL_Initialize_Input(void);
static int      PL_Process_Input_Events(void);
static void     PL_Convert_Touch(INPUT_DEVICE *dev);

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

void send_message(IFX_UINT32  *engine_msg)
{
    int         send_status = 0;

    /* Send the event in message queue. */
    send_status = msgsnd(IFXF_Message_Queue_ID, engine_msg,
                         IFXF_QUEUE_ELEMENT_SIZE * sizeof(IFX_UINT32),0);
    if (send_status < 0)
    {
        printf("Failed to send message to engine (%d)\n", errno);
    }
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

    /* Set the default configuration */
    memset(&engine_config, 0, sizeof(IFXE_CONFIGURATION));
    lc_strcpy(engine_config.language, "cn");
    //lc_strcpy(engine_config.display_mode, "1920p");
    engine_config.left = IFX_CENTER_DISPLAY;
    engine_config.top  = IFX_CENTER_DISPLAY;
//    lc_strcpy(engine_config.entrypoint_id, "rootPage");

    if (status == IFX_SUCCESS)
    {
#if         defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || \
            defined (IFX_ENABLE_BENCHMARKING_HEAP_USAGE)

        /* Configure the engine for benchmarking */
        lc_strncpy(engine_config.display_mode, IFXF_BENCHMARK_SCREEN_MODE, IFX_MAX_CONFIG_STRING_LENGTH);
        engine_config.left = IFXF_BENCHMARK_SCREEN_POS_X;
        engine_config.top  = IFXF_BENCHMARK_SCREEN_POS_Y;

#else       /* defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) ||
               defined (IFX_ENABLE_BENCHMARKING_HEAP_USAGE) */

        int fbdev;
        int ret;
        char *fbEnv;
        struct fb_var_screeninfo    varInfo;

        fbEnv = getenv("FRAMEBUFFER");

        if (fbEnv)
            fbdev = open(fbEnv, O_RDWR);
        else
            fbdev = open("/dev/fb0", O_RDWR);

        if (fbdev <= 0)
        {
            printf("Failed to open %s\n", fbEnv?fbEnv:"/dev/fb0");
            status =  IFX_ERROR;
        }

        if (status == IFX_SUCCESS)
        {
            ret = ioctl(fbdev, FBIOGET_VSCREENINFO, &varInfo);

            if (ret < 0)
            {
                printf("Failed to configure the fb device (error = %d)\n", ret);
                status = IFX_ERROR;
            }
        }

        printf("Screen resolution = %dx%d\r\n", varInfo.xres, varInfo.yres);

        /* Set the engine configuration based on the screen dimensions */
        if (status == IFX_SUCCESS)
        {
            engine_config.width  = varInfo.xres;
            engine_config.height = varInfo.yres;
        }

        close(fbdev);

#endif      /* defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) ||
               defined (IFX_ENABLE_BENCHMARKING_HEAP_USAGE) */
    }

    if (status == IFX_SUCCESS)
    {
        char *env_var = getenv("TPCAL");
        if (env_var)
        {
            char *tok;

            tok = strtok(env_var, ",");
            if (tok)
            {
                tp_left = atoi(tok);
                tok = strtok(NULL, ",");
            }

            if (tok)
            {
                tp_right = atoi(tok);
                tok = strtok(NULL, ",");
            }

            if (tok)
            {
                tp_top = atoi(tok);
                tok = strtok(NULL, ",");
            }

            if (tok)
            {
                tp_bottom = atoi(tok);
                tp_defined = IFX_TRUE;
            }
            else
            {
                printf("Error parsing touch panel calibration values.\n"
                       "Please use: left, right, top, bottom\n");
            }
        }
        else
        {
            printf("Using default touch panel calibration values (use TPCAL=\"left, right, top, bottom\" to override)\n",
                   tp_left, tp_right, tp_top, tp_bottom);
        }

        env_var = getenv("TPSWAP");
        if (env_var)
        {
            if (strstr(env_var, "0")
                || strstr(env_var, "false"))
            {
                tp_swap_x_y = IFX_FALSE;
            }
            else
            {
                tp_swap_x_y = IFX_TRUE;
            }
        }
    }

    /* Initialize the Inflexion UI for Linux Engine */
    if (status == IFX_SUCCESS)
    {
        status = IFXE_Initialize(&engine_config);
    }

    if (status == IFX_SUCCESS)
    {
        printf("Inflexion UI Engine initialized with:\n");
        printf("\tLanguage          = %s\n", engine_config.language);
        printf("\tDisplay mode      = %s\n", engine_config.display_mode);
        printf("\tTheme offset      = %dx%d\r\n", (int)engine_config.left, (int)engine_config.top);
        printf("\tTheme dimensions  = %dx%d\r\n", (int)engine_config.width, (int)engine_config.height);
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
    INPUT_DEVICE *dev = input_devices;
    INPUT_DEVICE *del;

    /* Shutdown the system. */

    printf("Shutting down the framework...\n");

    /* Close all input devices. */
    for (dev = input_devices; dev; )
    {
        close(dev->fd);
        del = dev;
        dev = dev->next;
        free(del);
    }

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
    printf("IFXF_Suspend not supported.\n");

    return IFX_ERROR;
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
    IFX_UINT32 event_count;
    IFX_UINT32 mat_cnt = 0, loop_count = 0;
    IFX_UINT32 current_time_upper;
    IFX_UINT32 current_time_lower;
    struct timeval local_time;
    IFX_UINT32 msg_buf[IFXF_QUEUE_ELEMENT_SIZE];
    int        recv_status;

#if defined (IFX_USE_SCRIPTS)
    IFX_UINT32 sec_sleep_time;
    IFX_UINT32 us_sleep_time;
#endif

    /* Initialize input devices. */
    status = PL_Initialize_Input();
    if (status != IFX_SUCCESS)
    {
        printf("\n Error in initializing input devices. \n");
        return status;
    }

    /* Deal with Signals */
    signal(SIGUSR1, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    /* Main event-reading loop */
    g_done = 0;
    while (g_done == 0)
    {

        /* Process incoming event. */
        event_count = PL_Process_Input_Events();

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
                    case IFXFKeyDown:
                        IFXE_Key_Down(msg_buf[1]);
                    break;

                    case IFXFKeyUp:
                        IFXE_Key_Up(msg_buf[1]);
                    break;

                    case IFXFTouchDown:
                        IFXE_Touch_Down(0, msg_buf[1], msg_buf[2]);

						if(touch_handle_func != NULL) //xg ++
							touch_handle_func(IFXFTouchDown, msg_buf[1], msg_buf[2], 0, 0);
			Previous_Page = getCurrenWndId();
                    break;

                    //xg add start
                    case IFXFTouchDClickDown:
						printf("\033[31m IFXFTouchDClickDown(%d, %d) \033[0m\n", msg_buf[1], msg_buf[2]);
//						if(gesture_handle_func != NULL) //xg ++
//							gesture_handle_func(IFXFTouchDClickDown, msg_buf[1], msg_buf[2]);
				if(msg_buf[1]>=640 && msg_buf[2]>=9 && msg_buf[2] <= 480)
                        		IFXE_Key_Down(87);
                        	else if(msg_buf[2]>=0 && msg_buf[2] <= 590)
                        		IFXE_Key_Down(88);
                        		
                    break;
                    //xg add end

                    case IFXFTouchMove:
                        IFXE_Touch_Move(0, msg_buf[1], msg_buf[2]);
#if 1/*ligui added*/

	if(getCurrenWndId()/*g_pMusicModule&&g_pMusicModule->paraRoot*/ == 17)
	{
#if 0 /*ligui added here*/
//	printf("%s %d",__func__,__LINE__);
		if(msg_buf[1]>=TOUCHREGION_X_BEGIN&&msg_buf[1]<=TOUCHREGION_X_END&&
		msg_buf[2]>=TOUCHREGION_Y_BEGIN&&msg_buf[2]<=TOUCHREGION_Y_END)
		{
//	printf("%s %d",__func__,__LINE__);
			float item = (msg_buf[1]-TOUCHREGION_X_BEGIN)*1.0  / TOUCHREGION_X_AVERAGE;
			refresh_rootTouchItem((int)(item>=0.5?item-0.5:0));
//	printf("%s %d item:%d",__func__,__LINE__,item);
		}
#else
		refresh_rootTouchItem(msg_buf[1],msg_buf[2]);
#endif
	}
	else
	{
#endif

						if(touch_handle_func != NULL) //xg ++
							touch_handle_func(IFXFTouchMove, msg_buf[1], msg_buf[2], 0, 0);

#if 1 /*ligui added here*/
	}
#endif
                    break;

                    case IFXFTouchUp:
                    	if(Previous_Page==0||Previous_Page == getCurrenWndId())
                    	{/*ignore when page has changed !!!*/
                       	IFXE_Touch_Up(0, msg_buf[1], msg_buf[2]);
                       }
#if 1/*ligui added*/
       #define BIT_SEEKBAR_USBMUSIC 	0x1 
       #define BIT_SEEKBAR_USBVIDEO 	0x2  
       #define BIT_SEEKBAR_IPODMUSIC 0x4           	
	extern int global_seekstatus;
	switch(global_seekstatus&0x7)
	{
		case BIT_SEEKBAR_USBMUSIC:
		{
			IFXM_music_SetFieldBoolData_usbmusicSeekFlag(NULL,IFX_FALSE);
			break;
		}
		case BIT_SEEKBAR_USBVIDEO:
		{
			IFXM_music_SetFieldBoolData_usbvideoSeekFlag(NULL,IFX_FALSE);
			break;
		}
		case BIT_SEEKBAR_IPODMUSIC:
		{
			IFXM_iPOD_SetFieldBoolData_iPODmusicSeekFlag(NULL,IFX_FALSE);
			break;
		}
		default:
		{
#endif

						if(touch_handle_func != NULL) //xg ++
							touch_handle_func(IFXFTouchUp, msg_buf[1], msg_buf[2], 0, 0);
#if 1/*ligui added*/
		}
	}
	global_seekstatus = 0;/*clear seek flag*/
#endif
	refresh_focusOpacity(IFX_TRUE);
                    break;

					// xg add start
                    case IFXFTouchDDown:
                    case IFXFTouchDMove:
                    case IFXFTouchDUp:
						if(touch_handle_func != NULL)
							touch_handle_func(msg_buf[0], msg_buf[1]>>16, msg_buf[1]&0xffff, msg_buf[2]>>16, msg_buf[2]&0xffff);
					break;
					// xg add end

                    case IFXFShutdown:
                        g_done = 1;
                    break;

                    default:
                        printf("Unhandled event type: %d\n", (int)msg_buf[0]);
                    break;
                } /* switch */
            } /* Receive event */
        } /* while task running */

        /* If the mouse has moved, then we need to refresh the screen */
        if (PL_mouse_moved == IFX_TRUE)
        {
            PL_mouse_moved = IFX_FALSE;
            IFXE_Refresh_Display();
        }

        /* Check our timer status. */
        IFXP_Timer_Get_Current_Time(&current_time_upper, &current_time_lower);
		Diag_HardKeyEnterDiag2( current_time_lower, 0x00 );//michael add 20151030
		Diag_MicPhoneRecordorPlayTime( current_time_lower );//michael add 20151030
		Diag_HvacKey_SetTime( current_time_lower );//michael add 20151110

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
        if ((event_count == 0) && (timer_alarm == ALARM_OFF || (compare_time(local_time, timer_alarm_val) < 0)))
        {
            usleep(1000);
        }
#endif
    }

    return status;
}

/*************************************************************************
*   FUNCTION
*
*       PL_Convert_Touch
*
*   DESCRIPTION
*
*       Converts the input touch events.
*
*   INPUTS
*
*       touch_x                             Input x-coordinate.
*
*       touch_y                             Input y-coordinate.
*
*       out                                 Output coord structure.
*
*   OUTPUTS
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
#if 0
static void     PL_Convert_Touch(INPUT_DEVICE *dev)
{
    int x, y;
    int touch_x, touch_y;

	printf("dev->type == %d\n", EV_REL);
    if (dev->type == EV_REL)
    {
		printf("dev->type == EV_REL\n");
        dev->coord.x = dev->x;
        dev->coord.y = dev->y;
        return;
    }

    touch_x = dev->slots[0].x;
    touch_y = dev->y;

	if(dev->slots[0].tracking_id >= 0)
	{
		if (dev->abs[ABS_X].maximum > dev->abs[ABS_X].minimum)
		{
			if (touch_x > dev->abs[ABS_X].maximum)
				touch_x = dev->abs[ABS_X].maximum;
			if (touch_x < dev->abs[ABS_X].minimum)
				touch_x = dev->abs[ABS_X].minimum;
			touch_x -= dev->abs[ABS_X].minimum;
			x = (pl_display.width * touch_x) / (dev->abs[ABS_X].maximum - dev->abs[ABS_X].minimum);
		}
		else
		{
			if (touch_x > dev->abs[ABS_X].minimum)
				touch_x = dev->abs[ABS_X].minimum;
			if (touch_x < dev->abs[ABS_X].maximum)
				touch_x = dev->abs[ABS_X].maximum;
			touch_x -= dev->abs[ABS_X].maximum;
			x = (pl_display.width * touch_x) / (dev->abs[ABS_X].minimum - dev->abs[ABS_X].maximum);
			x = pl_display.width - x;
		}
	}

    if (dev->abs[ABS_Y].minimum > dev->abs[ABS_Y].maximum)
    {
        if (touch_y > dev->abs[ABS_Y].minimum)
            touch_y = dev->abs[ABS_Y].minimum;
        if (touch_y < dev->abs[ABS_Y].maximum)
            touch_y = dev->abs[ABS_Y].maximum;
        touch_y -= dev->abs[ABS_Y].maximum;
        y = (pl_display.height * touch_y) / (dev->abs[ABS_Y].minimum - dev->abs[ABS_Y].maximum);
        y = pl_display.height - y;
    }
    else
    {
        if (touch_y > dev->abs[ABS_Y].maximum)
            touch_y = dev->abs[ABS_Y].maximum;
        if (touch_y < dev->abs[ABS_Y].minimum)
            touch_y = dev->abs[ABS_Y].minimum;
        touch_y -= dev->abs[ABS_Y].minimum;
        y = (pl_display.height * touch_y) / (dev->abs[ABS_Y].maximum - dev->abs[ABS_Y].minimum);
    }

    dev->coord.x = x;
    dev->coord.y = y;

    return;
}
#else
static void     PL_Convert_Touch(INPUT_DEVICE *dev)
{
    int x, y;
    int touch_x, touch_y;

    if (dev->type == EV_REL)
    {
        dev->coord.x = dev->x;
        dev->coord.y = dev->y;
        return;
    }

    touch_x = dev->x;
    touch_y = dev->y;

    if (dev->abs[ABS_X].maximum > dev->abs[ABS_X].minimum)
    {
        if (touch_x > dev->abs[ABS_X].maximum)
            touch_x = dev->abs[ABS_X].maximum;
        if (touch_x < dev->abs[ABS_X].minimum)
            touch_x = dev->abs[ABS_X].minimum;
        touch_x -= dev->abs[ABS_X].minimum;
        x = (pl_display.width * touch_x) / (dev->abs[ABS_X].maximum - dev->abs[ABS_X].minimum);
    }
    else
    {
        if (touch_x > dev->abs[ABS_X].minimum)
            touch_x = dev->abs[ABS_X].minimum;
        if (touch_x < dev->abs[ABS_X].maximum)
            touch_x = dev->abs[ABS_X].maximum;
        touch_x -= dev->abs[ABS_X].maximum;
        x = (pl_display.width * touch_x) / (dev->abs[ABS_X].minimum - dev->abs[ABS_X].maximum);
        x = pl_display.width - x;
    }

    if (dev->abs[ABS_Y].minimum > dev->abs[ABS_Y].maximum)
    {
        if (touch_y > dev->abs[ABS_Y].minimum)
            touch_y = dev->abs[ABS_Y].minimum;
        if (touch_y < dev->abs[ABS_Y].maximum)
            touch_y = dev->abs[ABS_Y].maximum;
        touch_y -= dev->abs[ABS_Y].maximum;
        y = (pl_display.height * touch_y) / (dev->abs[ABS_Y].minimum - dev->abs[ABS_Y].maximum);
        y = pl_display.height - y;
    }
    else
    {
        if (touch_y > dev->abs[ABS_Y].maximum)
            touch_y = dev->abs[ABS_Y].maximum;
        if (touch_y < dev->abs[ABS_Y].minimum)
            touch_y = dev->abs[ABS_Y].minimum;
        touch_y -= dev->abs[ABS_Y].minimum;
        y = (pl_display.height * touch_y) / (dev->abs[ABS_Y].maximum - dev->abs[ABS_Y].minimum);
    }

    dev->coord.x = x;
    dev->coord.y = y;

    return;
}
#endif

/*************************************************************************

*   FUNCTION
*
*       PL_Add_Input_Device
*
*   DESCRIPTION
*
*       Adds an input device to the list.
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
static int is_set(int n, void *addr)
{
    uint8_t *value = (uint8_t *)addr;
    return ((1 << (n%8)) & value[n>>3]) != 0;
}

#define test_bit(bit, value)    is_set(bit, value)

static IFX_RETURN_STATUS   PL_Add_Input_Device(char *path)
{
    INPUT_DEVICE *dev = input_devices;
    INPUT_DEVICE *new_dev = 0;
    uint8_t evtype[EV_MAX/8 + 1];
    int fd;
    char *env_var;

    fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd <= 0)
    {
        return IFX_ERROR;
    }

    if (ioctl(fd, EVIOCGBIT(0, sizeof(evtype)), evtype) < 0)
    {
        close(fd);
        return IFX_ERROR;
    }

    if (test_bit(EV_KEY, evtype)
        || test_bit(EV_REL, evtype)
        || (test_bit(EV_ABS, evtype)
            && test_bit(ABS_X, evtype)
            && test_bit(ABS_Y, evtype)
            && test_bit(ABS_PRESSURE, evtype)))
    {
        new_dev = malloc(sizeof(INPUT_DEVICE));
    }

    if (new_dev == NULL)
    {
        close(fd);
        return IFX_ERROR;
    }

    memset(new_dev, 0, sizeof(INPUT_DEVICE));

    /* Find the end of the list */
    if (dev)
    {
        while (dev->next)
        {
            dev = dev->next;
        }
        dev->next = new_dev;
        dev = dev->next;
    }
    else
    {
        dev = new_dev;
        input_devices = dev;
    }

    /* Populate the device structure */
    strncpy(dev->path, path, IFX_MAX_FILE_NAME_LEN);
    dev->fd = fd;

    /* Set the type - Note that if a device is both ABS and REL, we
       opt to treat it as a 'REL' device, if ABS and KEY (as touchpanels
       often are) treat it as ABS */
    if (test_bit(EV_KEY, evtype)) dev->type = EV_KEY;
    if (test_bit(EV_ABS, evtype)) dev->type = EV_ABS;
    if (test_bit(EV_REL, evtype)) dev->type = EV_REL;

    /* Get the device name */
    ioctl(fd, EVIOCGNAME(MAX_NAME_LENGTH), dev->name);

    /* Get the touch panel calibration */
    if (dev->type == EV_ABS)
    {
        int i;

        ioctl(fd, EVIOCGABS(ABS_X), &dev->abs[ABS_X]);
        ioctl(fd, EVIOCGABS(ABS_Y), &dev->abs[ABS_Y]);

        if (tp_defined==IFX_TRUE || dev->abs[ABS_X].maximum <= 0)
        {
            dev->abs[ABS_X].minimum = tp_left;
            dev->abs[ABS_X].maximum = tp_right;
            dev->abs[ABS_Y].minimum = tp_top;
            dev->abs[ABS_Y].maximum = tp_bottom;
        }

        dev->tp_swap_x_y = tp_swap_x_y;

		//xg add start
		dev->slots[0].tracking_id = -1;
		dev->slots[1].tracking_id = -1;
		//xg add end
    }

    printf("Added new input device: \"%s\" at \"%s\"\n", dev->name, dev->path);

    return IFX_SUCCESS;
}

int event_filter (const struct dirent *dirent)
{
    if (strstr(dirent->d_name, "event"))
        return 1;

    return 0;
}

/*************************************************************************
*   FUNCTION
*
*       PL_Initialize_Input
*
*   DESCRIPTION
*
*       Initializes the input devices.
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
static IFX_RETURN_STATUS   PL_Initialize_Input(void)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;
    char *env_var;
    struct dirent **namelist;
    int n;
    int auto_scan = 1;

    /* Read the environment variables (if any) */
    env_var = getenv("KEYPAD");
    if (env_var)
    {
        PL_Add_Input_Device(env_var);
        auto_scan = 0;
    }

    env_var = getenv("TOUCHPANEL");
    if (env_var)
    {
        PL_Add_Input_Device(env_var);
        auto_scan = 0;
    }

    if (auto_scan != 0)
    {
        /* Search through the /dev/input folder for event interfaces */
        n = scandir(EVENT_INTERFACE_PATH, &namelist, event_filter, alphasort);
        if (n > 0)
        {
            char fullPath[IFX_MAX_FILE_NAME_LEN];

            while(n--)
            {
                sprintf(fullPath, "%s%s", EVENT_INTERFACE_PATH, namelist[n]->d_name);
                PL_Add_Input_Device(fullPath);
                free(namelist[n]);
            }
            free(namelist);
        }
    }

    return  status;
}

/*************************************************************************
*   FUNCTION
*
*       PL_Process_Input_Events
*
*   DESCRIPTION
*
*       Processes input events.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       int                                 Flag to indicate status of inputs.
*
*************************************************************************/
#if 1 //xg modify
void register_touch_handle(void (*func_pointer)(int, int, int, int, int))
{
	if(func_pointer != NULL)
		touch_handle_func = func_pointer;
}
void unregister_touch_handle(void)
{
	touch_handle_func = NULL;
}

void register_gesture_handle(void (*func_pointer)(int, int, int))
{
	if(func_pointer != NULL)
		gesture_handle_func = func_pointer;
}

struct timeval touchDown_time1 = {0, 0}, touchDown_time2 = {0, 0};
int is_dclick = 0;
int down_x = 0, down_y = 0;
static int     PL_Process_Input_Events(void)
{
    IFX_UINT32  current_time;
    int         eventCount = 0;
    struct input_event event;
    INPUT_DEVICE *dev;
    IFX_UINT32  engine_msg[IFXF_QUEUE_ELEMENT_SIZE] = {0};

    eventCount = 0;

    /* Check for events from all input devices. */
    for (dev = input_devices; dev; dev = dev->next)
    {
        while ((read(dev->fd, &event, sizeof(struct input_event)) > 0)
            && (eventCount++ < PL_MAX_EVENTS) )
        {
            engine_msg[0] = 0;

            dprintf("event(%d, %d, %d)\n", event.type, event.code, event.value);
            if ((event.type == EV_KEY) && (event.value == 0x01))
            {
                switch (event.code)
                {
                    /* Keypad / Keyboard keys. */

                    case KEY_UP:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_UP;
                    break;

                    case KEY_DOWN:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_DOWN;
                    break;

                    case KEY_LEFT:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_LEFT;
                    break;

                    case KEY_RIGHT:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_RIGHT;
                    break;

                    case KEY_SEND:
                    case KEY_ENTER:
                    case KEY_SELECT:
                    case KEY_OK:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_SELECT;
                    break;

                    case KEY_HOME:
                    case KEY_END:
                    case KEY_TAB:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_BACK;
                    break;

                    case KEY_0:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '0';
                    break;

                    case KEY_1:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '1';
                    break;

                    case KEY_2:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '2';
                    break;

                    case KEY_3:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '3';
                    break;

                    case KEY_4:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '4';
                    break;

                    case KEY_5:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '5';
                    break;

                    case KEY_6:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '6';
                    break;

                    case KEY_7:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '7';
                    break;

                    case KEY_8:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '8';
                    break;

                    case KEY_9:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '9';
                    break;

                    default:
                        dprintf("EV_KEY:default\n");
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = event.code;
                    break;
                    break;
                }
            }

            if (event.type == EV_ABS)
            {
                switch (event.code)
                {
					case ABS_MT_TRACKING_ID:
						if(dev->cur_slot > MAX_SLOT_NUM)
							break;

						dev->slots[dev->cur_slot].tracking_id = event.value;
                        dprintf("EV_ABS:ABS_MT_TRACKING_ID(%d, %d)\n", dev->slots[0].tracking_id, dev->slots[1].tracking_id);
						if(event.value >= 0)//create a touch point
						{
							if(dev->touch_state == IFXFTouchDown || dev->touch_state == IFXFTouchMove)
								dev->touch_state = IFXFTouchDDown;
							else if(dev->touch_state == 0){
								dev->touch_state = IFXFTouchDown;
                            }
						}
						else //destroy a touch point
						{
							if(dev->touch_state == IFXFTouchDMove)
								dev->touch_state = IFXFTouchDUp;
							else if(dev->touch_state == IFXFTouchMove)
								dev->touch_state = IFXFTouchUp;
						}
                    break;

                    case ABS_MT_POSITION_X:
                        dprintf("EV_ABS:ABS_MT_POSITION_X\n");
						if(dev->cur_slot > MAX_SLOT_NUM)
							break;
						dev->slots[dev->cur_slot].x = event.value;
                    break;

                    case ABS_MT_POSITION_Y:
						dprintf("EV_ABS:ABS_MT_POSITION_Y\n");
						if(dev->cur_slot > MAX_SLOT_NUM)
							break;
						dev->slots[dev->cur_slot].y = event.value;
					break;

					case ABS_MT_PRESSURE:
						dprintf("EV_ABS:ABS_MT_PRESSURE\n");
					break;

					case ABS_MT_TOUCH_MAJOR:
					dprintf("EV_ABS:ABS_MT_TOUCH_MAJOR\n");
					break;

					case ABS_MT_SLOT:
						dprintf("EV_ABS:ABS_MT_SLOT\n");
						dev->cur_slot = event.value;
					break;

                    default:
                    break;
                }
            }

            else if (event.type == EV_REL)
            {
                /* We have seen a mouse input, enable the cursor */
                dev->active = IFX_TRUE;

                switch (event.code)
                {
                    case REL_X:
                        if (dev->tp_swap_x_y)
                        {
                            dev->y += (int)event.value;
                            if (dev->y < 0)
                                dev->y = 0;
                            if (dev->y > pl_display.height)
                                dev->y = pl_display.height;
                        }
                        else
                        {
                            dev->x += (int)event.value;
                            if (dev->x < 0)
                                dev->x = 0;
                            if (dev->x > pl_display.width)
                                dev->x = pl_display.width;
                        }
                    break;

                    case REL_Y:
                        if (dev->tp_swap_x_y)
                        {
                            dev->x += (int)event.value;
                            if (dev->x < 0)
                                dev->x = 0;
                            if (dev->x > pl_display.width)
                                dev->x = pl_display.width;
                        }
                        else
                        {
                            dev->y += (int)event.value;
                            if (dev->y < 0)
                                dev->y = 0;
                            if (dev->y > pl_display.height)
                                dev->y = pl_display.height;
                        }
                    break;

                    default:
                    break;
                }
			}

			else if(event.type == EV_SYN)
			{
				switch(event.code)
				{
					case SYN_REPORT:
#if 0
						if(dev->touch_state == IFXFTouchDOWN || dev->touch_state == IFXFTouchMOVE || dev->touch_state == IFXFTouchUP)
						{
							PL_Convert_Touch(dev);

							if (dev->coord.x < pl_display.offsetX)
								dev->coord.x = pl_display.offsetX;
							if (dev->coord.x > engine_config.width + pl_display.offsetX)
								dev->coord.x = engine_config.width + pl_display.offsetX;
							if (dev->coord.y < pl_display.offsetY)
								dev->coord.y = pl_display.offsetY;
							if (dev->coord.y > engine_config.height + pl_display.offsetY)
								dev->coord.y = engine_config.height + pl_display.offsetY;

							engine_msg[1] = dev->coord.x;
							engine_msg[2] = dev->coord.y;
						}
#endif
						if(dev->touch_state == IFXFTouchDown){
                            gettimeofday(&touchDown_time2, NULL);

                            if(touchDown_time1.tv_sec != 0 && touchDown_time1.tv_usec != 0){
                                if( (touchDown_time2.tv_sec - touchDown_time1.tv_sec)*1000000 + touchDown_time2.tv_usec - touchDown_time1.tv_usec <= 300*1000
                                        && is_dclick == 0
                                        && (dev->slots[0].x-down_x)*(dev->slots[0].x-down_x)+(dev->slots[0].y-down_y)*(dev->slots[0].y-down_y)<=(40*40) ){
                                    is_dclick = 1;
                                    dev->touch_state = IFXFTouchDClickDown;
                                }
                            }

                            if(dev->touch_state != IFXFTouchDClickDown){
                                touchDown_time1 = touchDown_time2;
                                is_dclick = 0;
                            }
                            down_x = dev->slots[0].x;
                            down_y = dev->slots[0].y;
                        }
						else if(dev->touch_state == IFXFTouchMove){
                            if(is_dclick == 0){
                                if( (dev->slots[0].x-down_x)*(dev->slots[0].x-down_x)+(dev->slots[0].y-down_y)*(dev->slots[0].y-down_y) > 40*40 )
                                    is_dclick = -1;
                            }
                        }

                        //this line is very import and don't comment it.
						printf("\033[31mSYN_REPORT(%d, %d, %d, %d, %d, %d, %d)\033[0m\n", dev->touch_state, dev->slots[0].tracking_id, dev->slots[0].x, dev->slots[0].y,
							dev->slots[1].tracking_id, dev->slots[1].x, dev->slots[1].y);

						if(dev->touch_state == IFXFTouchDown || dev->touch_state == IFXFTouchDClickDown 
                                || dev->touch_state == IFXFTouchMove || dev->touch_state == IFXFTouchUp)
						{
							engine_msg[0] = dev->touch_state;
							engine_msg[1] = dev->slots[dev->cur_slot].x;
							engine_msg[2] = dev->slots[dev->cur_slot].y;
						}
						else if(dev->touch_state == IFXFTouchDDown || dev->touch_state == IFXFTouchDMove || dev->touch_state == IFXFTouchDUp)
						{
							engine_msg[0] = dev->touch_state;
							engine_msg[1] = dev->slots[0].x<<16 | dev->slots[0].y;
							engine_msg[2] = dev->slots[1].x<<16 | dev->slots[1].y;
						}

						if(dev->touch_state == IFXFTouchDown || dev->touch_state == IFXFTouchDClickDown){
							dev->touch_state = IFXFTouchMove;
                        }
						else if(dev->touch_state == IFXFTouchDDown)
							dev->touch_state = IFXFTouchDMove;
						else if(dev->touch_state == IFXFTouchUp){
							dev->touch_state = 0;
                        }
						else if(dev->touch_state == IFXFTouchDUp)
						{
							if(dev->slots[0].tracking_id < 0 && dev->slots[1].tracking_id < 0)
								dev->touch_state = 0;
							else
								dev->touch_state = IFXFTouchMove;
						}
						break;
				}
			}

            if (engine_msg[0] > 0)
            {
                send_message(engine_msg);
            }
        }//while end
    }

    return eventCount;
}
#else
static int     PL_Process_Input_Events(void)
{
    IFX_UINT32  current_time;
    int         eventCount = 0;
    struct input_event event;
    INPUT_DEVICE *dev;
    IFX_UINT32  engine_msg[IFXF_QUEUE_ELEMENT_SIZE] = {0};

    eventCount = 0;

    /* Check for events from all input devices. */
    for (dev = input_devices; dev; dev = dev->next)
    {
        IFX_BOOL    input_down = IFX_FALSE;
        IFX_BOOL    input_move = IFX_FALSE;
        IFX_BOOL    input_up   = IFX_FALSE;

        while ((read(dev->fd, &event, sizeof(struct input_event)) > 0)
            && (eventCount++ < PL_MAX_EVENTS) )
        {
            engine_msg[0] = 0;

            dprintf("event(%d, %d, %d)\n", event.type, event.code, event.value);
            if ((event.type == EV_KEY) && (event.value == 0x01))
            {
                switch (event.code)
                {
                    /* Keypad / Keyboard keys. */

                    case KEY_UP:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_UP;
                    break;

                    case KEY_DOWN:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_DOWN;
                    break;

                    case KEY_LEFT:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_LEFT;
                    break;

                    case KEY_RIGHT:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_RIGHT;
                    break;

                    case KEY_SEND:
                    case KEY_ENTER:
                    case KEY_SELECT:
                    case KEY_OK:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_SELECT;
                    break;

                    case KEY_HOME:
                    case KEY_END:
                    case KEY_TAB:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = IFX_KEY_CODE_BACK;
                    break;

                    case KEY_0:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '0';
                    break;

                    case KEY_1:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '1';
                    break;

                    case KEY_2:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '2';
                    break;

                    case KEY_3:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '3';
                    break;

                    case KEY_4:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '4';
                    break;

                    case KEY_5:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '5';
                    break;

                    case KEY_6:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '6';
                    break;

                    case KEY_7:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '7';
                    break;

                    case KEY_8:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '8';
                    break;

                    case KEY_9:
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = '9';
                    break;

                    default:
                        dprintf("EV_KEY:default\n");
                        engine_msg[0] = IFXFKeyDown;
                        engine_msg[1] = event.code;
                    break;
                    break;
                }
            }

            if (event.type == EV_KEY)
            {
                switch (event.code)
                {
                    /* Mouse / Touch related keys */

                    case ABS_PRESSURE:
                        if (event.value <= NO_PRESSURE_VAL)
                        {
                            input_up = IFX_TRUE;
                            dev->dragging = 0;
                            dev->pen_down = 0;
                            input_move = IFX_FALSE;
                        }
                        else if (event.value > NO_PRESSURE_VAL)
                        {
                            dev->pen_down = 1;
                            //dprintf("ABS_PRESSURE input_down=TRUE\n");
			    input_down = IFX_TRUE;
                        }
                    break;

                    case BTN_TOUCH:
			dprintf("EV_KEY:BTN_TOUCH\n");
                    case BTN_MOUSE:
                        if (event.value == 0)
                        {
			    //dprintf("input_up=TRUE\n");
                            input_up = IFX_TRUE;
                            dev->dragging = 0;
                            dev->pen_down = 0;
                        }
                        else if (event.value > 0)
                        {
                            if (dev->active == IFX_TRUE)
                            {
				//dprintf("input_down=TRUE\n");
                                input_down = IFX_TRUE;
                            }
                            dev->pen_down = 1;
                        }
                    break;

                    default:
                    break;

                } // switch code
            }

            else if (event.type == EV_ABS)
            {
                switch (event.code)
                {
                    case ABS_X:
                        dprintf("EV_ABS:ABS_X\n");
                    case 0x02:
                        if (event.value > 0)
                        {
                            if (dev->tp_swap_x_y)
                                dev->y = event.value;
                            else
                                dev->x = event.value;

			    //dprintf("input_move=TRUE\n");
                            input_move = IFX_TRUE;
                        }
                    break;

                    case ABS_Y:
                        dprintf("EV_ABS:ABS_Y\n");
                    case 0x03:
                        if (event.value > 0)
                        {
                            if (dev->tp_swap_x_y)
                                dev->x = event.value;
                            else
                                dev->y = event.value;

			    //dprintf("input_move=TRUE\n");
                            input_move = IFX_TRUE;
                        }
                    break;

                    case ABS_PRESSURE:
			dprintf("EV_ABS:ABS_PRESSURE\n");
                        if (event.value <= NO_PRESSURE_VAL)
                        {
			    //dprintf("input_up=TRUE\n");
                            input_up = IFX_TRUE;
                            dev->dragging = 0;
                            dev->pen_down = 0;
                            input_move = IFX_FALSE;
                        }
                        else if (event.value > NO_PRESSURE_VAL)
                        {
                            /* We have seen a mouse input, enable the cursor */
                            dev->active = IFX_TRUE;

                            dev->pen_down = 1;
                        }
                    break;

                    case ABS_MT_TRACKING_ID:
                        dprintf("EV_ABS:ABS_MT_TRACKING_ID\n");
                    break;

                    case ABS_MT_POSITION_X:
                        dprintf("EV_ABS:ABS_MT_POSITION_X\n");
                    break;

                    case ABS_MT_POSITION_Y:
			dprintf("EV_ABS:ABS_MT_POSITION_Y\n");
		    break;
	
		    case ABS_MT_PRESSURE:
			dprintf("EV_ABS:ABS_MT_PRESSURE\n");
		    break;

		    case ABS_MT_TOUCH_MAJOR:
			dprintf("EV_ABS:ABS_MT_TOUCH_MAJOR\n");
		    break;

		    case ABS_MT_SLOT:
			dprintf("EV_ABS:ABS_MT_SLOT\n");
		    break;

                    default:
                    break;
                }
            }

            else if (event.type == EV_REL)
            {
                /* We have seen a mouse input, enable the cursor */
                dev->active = IFX_TRUE;

                switch (event.code)
                {
                    case REL_X:
                        if (dev->tp_swap_x_y)
                        {
                            dev->y += (int)event.value;
                            if (dev->y < 0)
                                dev->y = 0;
                            if (dev->y > pl_display.height)
                                dev->y = pl_display.height;
                        }
                        else
                        {
                            dev->x += (int)event.value;
                            if (dev->x < 0)
                                dev->x = 0;
                            if (dev->x > pl_display.width)
                                dev->x = pl_display.width;
                        }
                        input_move = IFX_TRUE;
                    break;

                    case REL_Y:
                        if (dev->tp_swap_x_y)
                        {
                            dev->x += (int)event.value;
                            if (dev->x < 0)
                                dev->x = 0;
                            if (dev->x > pl_display.width)
                                dev->x = pl_display.width;
                        }
                        else
                        {
                            dev->y += (int)event.value;
                            if (dev->y < 0)
                                dev->y = 0;
                            if (dev->y > pl_display.height)
                                dev->y = pl_display.height;
                        }
                        input_move = IFX_TRUE;
                    break;

                    default:
                    break;
                }
            }

	    else if(event.type == EV_SYN)
	    {
		switch(event.code)
		{
		    case SYN_REPORT:
			dprintf("EV_SYN:SYN_REPORT\n");
		    break;
		}
	    }

            if (engine_msg[0] > 0)
            {
                send_message(engine_msg);
            }
        }//while end

        /* Check to see if the touch coordinates need updating and send the appropriate message(s) */
        if (input_up || input_move || input_down)
        {
			dprintf("\033[31mbefore convert(%d, %d)\033[0m\n", dev->x, dev->y);
            PL_Convert_Touch(dev);

            if (dev->coord.x < pl_display.offsetX)
                dev->coord.x = pl_display.offsetX;
            if (dev->coord.x > engine_config.width + pl_display.offsetX)
                dev->coord.x = engine_config.width + pl_display.offsetX;
            if (dev->coord.y < pl_display.offsetY)
                dev->coord.y = pl_display.offsetY;
            if (dev->coord.y > engine_config.height + pl_display.offsetY)
                dev->coord.y = engine_config.height + pl_display.offsetY;

			dprintf("\033[31mafter convert(%d, %d)\033[0m\n", dev->coord.x, dev->coord.y);
            engine_msg[1] = dev->coord.x;
            engine_msg[2] = dev->coord.y;
        }

        if (input_down)
        {
            engine_msg[0] = IFXFTouchDown;
            //dprintf("\033[31meventCount:%d, send input_down(%d,%d)\033[0m\n", eventCount, engine_msg[1], engine_msg[2]);
            send_message(engine_msg);
        }

        if (input_move == IFX_TRUE)
        {
            if (dev->pen_down)
            {
                engine_msg[0] = IFXFTouchMove;
                if (0 == dev->dragging)
                {
                    /* If we are not dragging, and the pen is down, set dragging on */
                    dev->dragging = 1;
                }
            }
            else
            {
                /* Issue mouse move events */
                engine_msg[0] = IFXFTouchMove;
            }

            if (engine_msg[0] > 0)
            {
                //dprintf("\033[31meventCount:%d, send input_move(%d,%d)\033[0m\n", eventCount, engine_msg[1], engine_msg[2]);
                send_message(engine_msg);
            }
        }

        if (input_up)
        {
            engine_msg[0] = IFXFTouchUp;
            //dprintf("\033[31meventCount:%d, send input_up(%d,%d)\033[0m\n", eventCount, engine_msg[1], engine_msg[2]);
            send_message(engine_msg);
        }
    }

    return eventCount;
}
#endif

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
*       timeUpper           Upper part of time
*       timeLower           Lower part of time
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

#if 1
//xg added on 20150806
int SetUIGlobalAlphaAndColorKey(const char *pALFBDevice, int iGlobalAlphaEnable, int iGloblAlpha,
                                int iColorKeyEnable, int iColorKey)
{
    struct mxcfb_gbl_alpha gbl_alpha;
    struct mxcfb_color_key key;
    struct mxcfb_loc_alpha loc_alpha = {0};
    int fd_fb;

    if(NULL == pALFBDevice)
    {
        printf("Invalid Frame buffer device!\n");
        return -1;
    }

    if((0 == iGlobalAlphaEnable) || (1 == iGlobalAlphaEnable) || (0 == iColorKeyEnable) || (1 == iColorKeyEnable))
    {
        if ((fd_fb = open(pALFBDevice, O_RDWR, 0)) < 0)
        {
                printf("Unable to open %s\n", pALFBDevice);
                return -2;
        }
        //DEBUG("Open FB device ok!\n");
    }

    if((0 == iGlobalAlphaEnable) || (1 == iGlobalAlphaEnable))
    {
        gbl_alpha.enable = iGlobalAlphaEnable;
        if((iGloblAlpha <= 255) && (iGloblAlpha >= 0))
        {
            gbl_alpha.alpha = iGloblAlpha;
        }
        else
        {
            gbl_alpha.alpha = 128;
        }
        ioctl(fd_fb, MXCFB_SET_GBL_ALPHA, &gbl_alpha);
        //DEBUG("Set global alpha enable, alpha: %d, %d\n", gbl_alpha.enable, gbl_alpha.alpha);
    }
    else
    {
        //DEBUG("we will not set alpha, iGlobalAlphaEnable: %d!\n", iGlobalAlphaEnable);
    }

    if((0 == iColorKeyEnable) || (1 == iColorKeyEnable))
    {
        key.enable = iColorKeyEnable;
        key.color_key = iColorKey; // black
        ioctl(fd_fb, MXCFB_SET_CLR_KEY, &key);
        //DEBUG("Set color key enable, colorkey: %d, 0x%X\n", key.enable, key.color_key);
    }
    else
    {
        //DEBUG("we will not set color key, iColorKeyEnable: %d!\n", iColorKeyEnable);
    }

    close(fd_fb);

    return 0;
}
#endif
