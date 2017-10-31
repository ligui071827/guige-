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
#ifndef IFXUI_ENGINE_H
#define IFXUI_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "inflexionui/engine/ifxui_defs.h"

/* Engine Configuration Structure */
typedef struct tagIFXE_CONFIGURATION
{
    char      language[IFX_MAX_CONFIG_STRING_LENGTH];
    IFX_INT32 left;
    IFX_INT32 top;
    IFX_INT32 width;
    IFX_INT32 height;
    char      display_mode[IFX_MAX_CONFIG_STRING_LENGTH];
    char      package_name[IFX_MAX_CONFIG_STRING_LENGTH];
	char      entrypoint_id[IFX_MAX_CONFIG_STRING_LENGTH];
	char	  restoreState;
} IFXE_CONFIGURATION;

/* Initialize library and pass in queue to receive notifications */
IFX_RETURN_STATUS    IFXE_Initialize               (IFXE_CONFIGURATION *config);

/* Terminate the Inflexion UI Engine */
IFX_RETURN_STATUS    IFXE_Shutdown                 (void);


/* Change the current display mode, theme package name and/or language to
   be used by the Engine.

   Any combination of display_mode, package_name or language may
   be specified, using NULL where a feature is not required to be modified.
*/
IFX_RETURN_STATUS    IFXE_Set_Configuration        (IFXE_CONFIGURATION *config);
/* Retrieve the current configuration */
IFX_RETURN_STATUS    IFXE_Get_Configuration        (IFXE_CONFIGURATION *config);

/* Call when key events occur */
IFX_RETURN_STATUS    IFXE_Key_Down                 (IFX_INT32 code);
IFX_RETURN_STATUS    IFXE_Key_Up                   (IFX_INT32 code);
IFX_RETURN_STATUS    IFXE_Save_State               (int *abortSave);

/* Call when mouse/stylus events occur */

IFX_RETURN_STATUS    IFXE_Touch_Up                 (IFX_UINT32 touch_id, IFX_INT32 x, IFX_INT32 y);
IFX_RETURN_STATUS    IFXE_Touch_Down               (IFX_UINT32 touch_id, IFX_INT32 x, IFX_INT32 y);
IFX_RETURN_STATUS    IFXE_Touch_Move               (IFX_UINT32 touch_id, IFX_INT32 x, IFX_INT32 y);

/* Call to drive the engine. All scheduled events up to time will be processed */
IFX_RETURN_STATUS    IFXE_Process_Timers           (IFX_UINT32 time_upper, IFX_UINT32 time_lower);

/* Instruct the engine to go stop all running animations and go to a static state */
IFX_RETURN_STATUS    IFXE_Set_Idle                 (void);

/* Instruct the engine to repaint the whole display. This shall not refresh field data. */
IFX_RETURN_STATUS    IFXE_Refresh_Display          (void);

/* Instruct the engine to repaint the whole display. This shall refresh all field data. */
IFX_RETURN_STATUS    IFXE_Refresh_All              (void);

/* Inform the engine that an asynchronous link has completed. This shall repaint the whole screen. */
IFX_RETURN_STATUS    IFXE_Async_Link_Complete      (void);

/* This shall stop any animations and free the foreground and background canvasses. */
IFX_RETURN_STATUS    IFXE_Suspend                  (void);

/* This shall restore the foreground and background canvasses and repaint the screen. */
IFX_RETURN_STATUS    IFXE_Resume                   (void);

/* Instruct the engine to resume static animation */
IFX_RETURN_STATUS    IFXE_Resume_Static_Animations (void);

/* File access API */
IFX_RETURN_STATUS    IFXE_File_Open                (IFXE_FILE *handle, const char *file_name);
IFX_RETURN_STATUS    IFXE_File_Read                (IFXE_FILE handle, void *addr, IFX_UINT32 size, IFX_UINT32 *bytes_read);
IFX_RETURN_STATUS    IFXE_File_Seek                (IFXE_FILE handle, IFX_INT32 offset, IFX_SEEK seek_mode);
IFX_RETURN_STATUS    IFXE_File_Size                (IFXE_FILE handle, IFX_UINT32 *file_size);
IFX_RETURN_STATUS    IFXE_File_Close               (IFXE_FILE handle);

#ifdef __cplusplus
}
#endif
#endif /* IFXUI_ENGINE_H */
