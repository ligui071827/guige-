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
*       ifxui_porting_general.c
*
* COMPONENT
*
*       Inflexion UI Porting Layer.
*
* DESCRIPTION
*
*       This module implements the remaining function of the porting layer.
*       This include timer schedule and get current time and error print.
*
* DATA STRUCTURES
*
*       None
*
* FUNCTIONS
*
*       IFXP_Initialize                     Initializes all modules within the
*                                           porting layer.
*
*       IFXP_Shutdown                       Shuts down all modules within the
*                                           porting layer.
*
*       IFXP_Runtime_State_Changed          Called when runtime status changes
*
*       IFXP_Timer_Get_Current_Time         Retrieves the current system time in ms.
*
*       IFXP_Timer_Schedule                 Schedules a timer to expire.
*                                           On expiry, IFXE_Process_Timers is called.
*
* DEPENDENCIES
*
*       ifxui_porting_linux.h               Main include for Linux
*                                           porting layer.
*
*       framework_linux.h                   Linux Framework API.
*
*************************************************************************/
#include    <sys/time.h>
#include    <time.h>

/* Inflexion UI Porting layer include. */
#include    "inflexionui/porting/inc/ifxui_porting_linux.h"

/* Inflexion UI Framework include */
#include    "inflexionui/framework/inc/framework_linux.h"
#if defined(IFX_RENDER_DIRECT_OPENGL)
    #include    "gl.h"
#elif defined(IFX_RENDER_DIRECT_OPENGL_20)
    #include    "gl2.h"
#endif  /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

/*************************************************************************
* FUNCTION
*
*       IFXP_Initialize
*
* DESCRIPTION
*
*       This function initializes the porting layer and all sub modules.
*
* INPUTS
*
*       none
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Initialize(void)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    /* Initialize memory pools. */
    status = PL_Memory_Initialize();

    if (status == IFX_SUCCESS)
    {
        /* Initialize display related functionality. */
        status = PL_Display_Initialize();
#ifdef IFX_USE_NATIVE_FONTS
        if (status == IFX_SUCCESS)
        {
            status = PL_Font_Initialize();
        }
#endif
    }

#ifdef      IFX_USE_PLATFORM_FILES

    if (status == IFX_SUCCESS)
    {
        /* Initialize file related functionality. */
        status = PL_File_Initialize();
    }

#endif      /* IFX_USE_PLATFORM_FILES */

#if         defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || \
            defined (IFX_ENABLE_BENCHMARKING_HEAP_USAGE)

    if (status == IFX_SUCCESS)
    {
        status = PL_Benchmarking_Initialize();
    }

#endif      /* defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) ||
               defined (IFX_ENABLE_BENCHMARKING_HEAP_USAGE) */

    return status;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Shutdown
*
* DESCRIPTION
*
*       This function closes down the porting layer and event handler.
*
* INPUTS
*
*       none
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Shutdown(void)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

#if         defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || \
            defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)

    if (status == IFX_SUCCESS)
    {
        status = PL_Benchmarking_Shutdown();
    }

#endif      /* defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) ||
               defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE) */

#ifdef      IFX_USE_PLATFORM_FILES

    if (status == IFX_SUCCESS)
    {
        /* Close down the file related services. */
        status = PL_File_Shutdown();
    }

#endif      /* IFX_USE_PLATFORM_FILES */

    if (status == IFX_SUCCESS)
    {
        /* Close down the display. */
        status = PL_Display_Shutdown();

#ifdef IFX_USE_NATIVE_FONTS
		if (status == IFX_SUCCESS)
        {
            status = PL_Font_Shutdown();
        }
#endif
    }

    if (status == IFX_SUCCESS)
    {
        /* Close down the memory system. */
        status = PL_Memory_Shutdown();
    }

    return status;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Runtime_State_Changed
*
* DESCRIPTION
*
*       This function is called each time the runtime status is changed.
*
* INPUTS
*
*       state                               New runtime state
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Runtime_State_Changed      (IFXE_STATE state)
{
	return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Timer_Get_Current_Time
*
* DESCRIPTION
*
*       This function returns the current system time in ms.
*
* INPUTS
*
*       time_upper                         Pointer to variable to receive
*                                          the upper DWORD of the current
*                                          system time in ms.
*       time_lower                         Pointer to variable to receive
*                                          the lower DWORD of the current
*                                          system time in ms.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Timer_Get_Current_Time(IFX_UINT32 *time_upper,
                                                IFX_UINT32 *time_lower)
{
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    *time_upper = (tp.tv_sec / (UINT_MAX/1000));
    *time_lower = (tp.tv_sec % (UINT_MAX/1000)) * 1000;
    *time_lower += tp.tv_nsec / 1000000;

    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Timer_Schedule
*
* DESCRIPTION
*
*       This function shall schedule the specified timer to expire at
*       system clock "time" in ms at the latest.
*       If the timer is already scheduled to expire before this time,
*       then the timer shall not be modified.
*
* INPUTS
*
*       time_upper                          upper DWORD of required timeout
*                                           time in milliseconds
*       time_lower                          lower DWORD of required timeout
*                                           time in milliseconds
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Timer_Schedule(IFX_UINT32 time_upper,
                                        IFX_UINT32 time_lower)
{
    return IFXF_Schedule_Timer(time_upper, time_lower);
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Error_Print
*
* DESCRIPTION
*
*       This function is called when a non-fatal error occurs.
*       Processing the error message is optional, but shall always return
*       IFX_SUCCESS.
*
* INPUTS
*
*       error_string                        Error description in engineering
*                                           English.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Error_Print(const IFX_WCHAR *error_string)
{
    printf("%S\r\n", error_string);

    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Command
*
* DESCRIPTION
*
*       This function is called to execute a command that has originated
*       from a theme.
*
* INPUTS
*
*       command                             The command to execute.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Command(IFXP_CMD command)
{
    IFX_RETURN_STATUS status = IFX_ERROR;

    switch (command)
    {
        case IFXP_CMD_SUSPEND:
            status = IFXF_Suspend();
        break;

        case IFXP_CMD_EXIT:
            status = IFXF_Exit();
        break;

        default:
        break;
    }

    return status;
}

#if defined(IFX_RENDER_DIRECT_OPENGL_20) || defined(IFX_RENDER_DIRECT_OPENGL)
/*************************************************************************
*   FUNCTION
*
*       IFXP_Check_GL_Extension
*
*   DESCRIPTION
*
*       This function is called to check a particular GL extension
*
*
*   INPUTS
*
*       ext_name         - The extension name to search.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS IFXP_Check_GL_Extension (const char *ext_name)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    char *end = NULL;
    int extNameLen = -1;
    char *p = NULL;

    if (ext_name == NULL)
        return status;

    // Get GL extension string
    p = (char *) glGetString(GL_EXTENSIONS);
    if (p == NULL || lc_strlen(p) == 0)
        return status;

    extNameLen = lc_strlen(ext_name);
    end = p + lc_strlen(p);

    while (p < end)
    {
        // As per specification, extension are separated by space
        int n = lc_strcspn(p, " ");
        if ((extNameLen == n) && (lc_strnicmp(ext_name, p, n) == 0))
        {
            return IFX_SUCCESS;
        }
        p += (n + 1);
    }
    return (status);
}
#endif //defined(IFX_RENDER_DIRECT_OPENGL_20)) || defined(IFX_RENDER_DIRECT_OPENGL)
