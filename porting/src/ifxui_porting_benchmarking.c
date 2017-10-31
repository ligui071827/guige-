/*************************************************************************
*
*            Copyright 2006 Mentor Graphics Corporation
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
*       ifxui_porting_benchmarking.c
*
*   COMPONENT
*
*       IFXP
*
*   DESCRIPTION
*
*       Inflexion UI Engine Porting Layer Benchmarking Implementation
*
*       The Engine library must be built with either
*       IFX_ENABLE_BENCHMARKING_FRAME_RATE (for looking at frame rate or
*       page opening times) or IFX_ENABLE_BENCHMARKING_HEAP_USAGE (to
*       look at heap and stack usage).
*
*   DATA STRUCTURES
*
*       None
*
*   FUNCTIONS
*
*       PL_Benchmarking_Initialize          Initializes benchmarking.
*
*       PL_Benchmarking_Shutdown            Shuts down benchmarking.
*
*       PL_Benchmarking_Signal              Receives a benchmarking signal
*                                           from the Engine.
*
* DEPENDENCIES
*
*       ifxui_porting_linux.h               Main include for Linux
*                                           porting layer.
*
*************************************************************************/

/* Linux Porting layer includes. */
#include    "inflexionui/porting/inc/ifxui_porting_linux.h"

#if         defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || \
            defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
#include    <pthread.h>

/* Heap tracking variables used by benchmarking. */

#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE

/* Period between benchmark data refreshes to screen in s. */
#define     BENCHMARING_REFRESH_RATE        1

unsigned int pl_engine_heap_usage       = 0;
unsigned int pl_engine_heap_usage_max   = 0;
unsigned int pl_generic_heap_usage      = 0;
unsigned int pl_generic_heap_usage_max  = 0;
unsigned int pl_gpu_memory_usage        = 0;
unsigned int pl_gpu_memory_usage_max    = 0;

/* Benchmarking Output Task */
#define BM_EXIT 0
#define BM_RUN 1
static int      Benchmarking_Task_Status = BM_EXIT;
static pthread_t Benchmarking_Thread_Id;

#endif      /* IFX_ENABLE_BENCHMARKING_HEAP_USAGE */

/* Frame Rate tracking variables used by benchmarking. */
#ifdef      IFX_ENABLE_BENCHMARKING_FRAME_RATE

/* Frame rate in Frames Per Second. */
static unsigned int Pl_Benchmarking_Frame_Rate = 0;

/* Page opening time in ms */
static unsigned int Pl_Benchmarking_Page_Open_Time = 0;

#endif      /* IFX_ENABLE_BENCHMARKING_FRAME_RATE */


/* Function Prototypes. */

#ifdef      IFX_ENABLE_BENCHMARKING_FRAME_RATE
static void BM_Frame_Rate_Output(unsigned int system_time,unsigned int frameRate,unsigned int pageOpenTime);
#endif      /* IFX_ENABLE_BENCHMARKING_FRAME_RATE */

#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE
static  void*    Benchmarking_Task_Entry(void*a);
#endif      /* IFX_ENABLE_BENCHMARKING_HEAP_USAGE */

static void BM_Initialize_Output(void);
static unsigned int BM_Ticks(unsigned int new_time);

static int  Benchmarking_Message_Queue_ID = -1;

/*************************************************************************
*   FUNCTION
*
*       PL_Benchmarking_Initialize
*
*   DESCRIPTION
*
*       This function initializes any benchmarking tasks / data.
*
*   INPUTS
*
*      none.
*
*   OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS PL_Benchmarking_Initialize(void)
{

    BM_Initialize_Output();

#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE

    Benchmarking_Task_Status = BM_RUN;
    printf("[benchmarking]---------------------------------------------------\n");
    if (pthread_create(&Benchmarking_Thread_Id, NULL, Benchmarking_Task_Entry, NULL) != 0)
    {
        printf("Benchmarking Thread not created\n");

        return (IFX_ERROR);
    }

#endif      /* IFX_ENABLE_BENCHMARKING_FRAME_RATE */

    /* Create benchmarking Message Queue. */
    Benchmarking_Message_Queue_ID = msgget(PL_BENCHMARKING_MESSAGE_QUEUE_KEY,
                                           IPC_CREAT | S_IRUSR | S_IWUSR);
    if (Benchmarking_Message_Queue_ID < 0)
    {
        printf("\n Benchmarking Queue Creation Failed.\n");
        return IFX_ERROR;
    }

    return (IFX_SUCCESS);

}

/*************************************************************************
* FUNCTION
*
*       IFXP_Benchmarking_Shutdown
*
* DESCRIPTION
*
*       This function closes down any benchmarking tasks / data.
*
* INPUTS
*
*      none.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS PL_Benchmarking_Shutdown(void)
{

#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE

    Benchmarking_Task_Status = BM_EXIT;
    pthread_join(Benchmarking_Thread_Id, NULL);

#endif      /* IFX_ENABLE_BENCHMARKING_HEAP_USAGE */

    msgctl(Benchmarking_Message_Queue_ID, IPC_RMID, NULL);
    Benchmarking_Message_Queue_ID = -1;

    return (IFX_SUCCESS);
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Benchmarking_Signal
*
* DESCRIPTION
*
*       This function is provided for benchmarking the Engine.
*       The events are defined in ifxui_control.h and represent particular
*       events during the Engine's operation.
*       Use of the information provided is optional.
*
* INPUTS
*
*      signal                               Type of event that has occurred.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS IFXP_Benchmarking_Signal(IFX_INT32 signal, IFX_INT32 data)
{

#ifdef      IFX_ENABLE_BENCHMARKING_FRAME_RATE

    static unsigned int frame_count = 0;
    static unsigned int frame_count_time = 0;
    static unsigned int page_open_start = 0;
    unsigned int ticks;

    IFX_UINT32 system_time_upper;
    IFX_UINT32 system_time_lower;
    IFXP_Timer_Get_Current_Time(&system_time_upper, &system_time_lower);
    ticks = BM_Ticks(system_time_lower);

    if ((signal&IFXP_BENCHMARKING_FRAME_UPDATE_COMPLETE) == IFXP_BENCHMARKING_FRAME_UPDATE_COMPLETE)
    {
        frame_count++;

        if (system_time_lower >= (frame_count_time + IFXP_BENCHMARK_FRAME_COUNT_TIME))
        {
            /* Calculate and store the frame rate. */
            Pl_Benchmarking_Frame_Rate = (1000 * frame_count) / (system_time_lower - frame_count_time);
            frame_count_time = system_time_lower;
            frame_count = 0;
            BM_Frame_Rate_Output(ticks, Pl_Benchmarking_Frame_Rate, Pl_Benchmarking_Page_Open_Time);
        }
    }

    if ((signal&IFXP_BENCHMARKING_OPEN_PAGE_STARTED) == IFXP_BENCHMARKING_OPEN_PAGE_STARTED)
    {
        page_open_start = system_time_lower;
    }

    if ((signal&IFXP_BENCHMARKING_OPEN_PAGE_COMPLETE) == IFXP_BENCHMARKING_OPEN_PAGE_COMPLETE)
    {
        Pl_Benchmarking_Page_Open_Time = system_time_lower - page_open_start;
        BM_Frame_Rate_Output(ticks, Pl_Benchmarking_Frame_Rate,Pl_Benchmarking_Page_Open_Time);
    }

#endif      /* IFX_ENABLE_BENCHMARKING_FRAME_RATE */

#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE
    if ((signal&IFXP_BENCHMARKING_GPU_MEMORY_RESET) == IFXP_BENCHMARKING_GPU_MEMORY_RESET)
    {
        pl_gpu_memory_usage = 0;
    }

    if ((signal&IFXP_BENCHMARKING_GPU_MEMORY_CHANGED) == IFXP_BENCHMARKING_GPU_MEMORY_CHANGED)
    {
        pl_gpu_memory_usage += (data / 1024);
        if (pl_gpu_memory_usage > pl_gpu_memory_usage_max)
        {
            pl_gpu_memory_usage_max = pl_gpu_memory_usage;
        }
    }
#endif      /* IFX_ENABLE_BENCHMARKING_HEAP_USAGE */

    /* Send the events so the host application can process them */
    if (Benchmarking_Message_Queue_ID >= 0)
    {
        static int first_sent = 0;
        struct msqid_ds queue_info;
        msgctl(Benchmarking_Message_Queue_ID, IPC_STAT, &queue_info);

        /* Only send the message if some one is listening
           (causes a lockup after ~ 2000 frames otherwise) */
        if (queue_info.msg_rtime > 0 || first_sent == 0)
        {
            if (msgsnd(Benchmarking_Message_Queue_ID, &signal, sizeof(IFX_INT32), 0) != 0)
            {
                printf("Failed to send benchmarking signal\n");
                return IFX_ERROR;
            }
            first_sent = 1;
        }
    }

    return (IFX_SUCCESS);
}

/*************************************************************************
*   FUNCTION
*
*       BM_Initialize_Output
*
*   DESCRIPTION
*
*       This function initializes the GrafixRS grafports for rendering.
*
*   INPUTS
*
*      None.
*
*************************************************************************/
static void BM_Initialize_Output(void)
{

#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE

    printf("      Tick  Cur.Engine  Max.Engine  Cur.Generic  Max.Generic  Cur.GPU    Max.GPU\n");
    printf("             Heap(KB)    Heap(KB)     Heap(KB)    Heap(KB)    Heap(KB)   Heap(KB)\n");
#else

    printf("      Tick,  Frame Rate,  Page Open Time (ms)\n");

#endif      /* IFX_ENABLE_BENCHMARKING_HEAP_USAGE */
}

/*************************************************************************
*   FUNCTION
*
*       BM_Ticks
*
*   DESCRIPTION
*
*       Return the Time ticks.
*
*   INPUTS
*
*       system_time    - Time.
*
* OUTPUTS
*
*       unsigned int  ticks.
*
*************************************************************************/
static unsigned int BM_Ticks(unsigned int new_time)
{
    static unsigned int start_time=0;

    if(start_time == 0)
    {
        start_time = new_time;
    }

    return (new_time - start_time);
}

#ifdef      IFX_ENABLE_BENCHMARKING_FRAME_RATE
/*************************************************************************
*   FUNCTION
*
*       BM_Frame_Rate_Output
*
*   DESCRIPTION
*
*       This function output the frame rate.
*
*   INPUTS
*
*       system_time    - Time.
*
*       frameRate      - Frame rate.
*
*       pageOpenTime   - Page open time.
*
*************************************************************************/
static void BM_Frame_Rate_Output(unsigned int ticks,
                                 unsigned int frameRate,
                                 unsigned int pageOpenTime)
{

        if (frameRate > 99)
        {
            frameRate = 99;
        }

        if (pageOpenTime > 9999)
        {
            pageOpenTime = 9999;
        }

        printf("%10d       %02d            %04d\n", ticks, frameRate,pageOpenTime);

}
#endif      /* IFX_ENABLE_BENCHMARKING_FRAME_RATE */

/*************************************************************************
*   FUNCTION
*
*       BM_Heap_Usage_Output
*
*   DESCRIPTION
*
*       This function output the heap usage.
*
*   INPUTS
*
*       system_time     - Time.
*
*       curEngineHeap   - Current heap used by engine.
*
*       maxEngineHeap   - Maximum heap used by engine.
*
*       curGenericHeap  - Current heap used from system.
*
*       maxGenericHeap  - Maximum heap used from system.
*
*************************************************************************/
static void BM_Heap_Usage_Output(unsigned int ticks,
                                 unsigned int curEngineHeap,
                                 unsigned int maxEngineHeap,
                                 unsigned int curGenericHeap,
                                 unsigned int maxGenericHeap,
                                 unsigned int curGpuHeap,
                                 unsigned int maxGpuHeap)
{
#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE

        /* Work in KB for heap values. */
        curEngineHeap = (curEngineHeap>>10);
        maxEngineHeap = (maxEngineHeap>>10);
        curGenericHeap = (curGenericHeap>>10);
        maxGenericHeap = (maxGenericHeap>>10);


        /* Limit to 99,999. */

        if (curEngineHeap > 99999)
            curEngineHeap = 99999;
        if (maxEngineHeap > 99999)
            maxEngineHeap = 99999;
        if (curGenericHeap > 99999)
            curGenericHeap = 99999;
        if (maxGenericHeap > 99999)
            maxGenericHeap = 99999;
        if (curGpuHeap > 99999)
            curGpuHeap = 99999;
        if (maxGpuHeap > 99999)
            maxGpuHeap = 99999;

        printf("%10d    %05d       %05d        %05d       %05d       %05d     %05d\n",
             ticks, curEngineHeap, maxEngineHeap, curGenericHeap, maxGenericHeap, curGpuHeap, maxGpuHeap);

#endif      /* IFX_ENABLE_BENCHMARKING_HEAP_USAGE */
}

/*************************************************************************
*   FUNCTION
*
*       Benchmarking_Task_Entry
*
*   DESCRIPTION
*
*       Benchmark Task to format and output the benchmarking information.
*
*   INPUTS
*
*      a      - Unused parameters.
*
*************************************************************************/
static void* Benchmarking_Task_Entry(void* a)
{
#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE

    unsigned int ticks;
    IFX_UINT32 system_time_upper;
    IFX_UINT32 system_time_lower;

    while ( Benchmarking_Task_Status == BM_RUN )
    {

        IFXP_Timer_Get_Current_Time(&system_time_upper, &system_time_lower);
        ticks = BM_Ticks(system_time_lower);

        BM_Heap_Usage_Output(ticks, pl_engine_heap_usage, pl_engine_heap_usage_max,
                             pl_generic_heap_usage, pl_generic_heap_usage_max,
                             pl_gpu_memory_usage, pl_gpu_memory_usage_max);

        /* Only output the data at the rate specified by our refresh rate. */
         sleep(BENCHMARING_REFRESH_RATE);

    }
#endif      /* IFX_ENABLE_BENCHMARKING_HEAP_USAGE */

    return NULL;
}

#endif      /* defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) ||
               defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE) */

