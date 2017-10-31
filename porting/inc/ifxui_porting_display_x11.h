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
* FILE NAME                                 VERSION
*
*       ifxui_porting_display_x11.h
*
* COMPONENT
*
*       Inflexion UI Porting Layer.
*
* DESCRIPTION
*
*       Inflexion UI Engine Porting API Display Implementation for Linux X11
*
* DATA STRUCTURES
*
*       PL_X11_DISPLAY                      Engine porting layer
*                                           display structure for X11.
*
* DEPENDENCIES
*
*       None
*
*************************************************************************/
#ifndef     _IFXUI_PORTING_DISPLAY_X11_
#define     _IFXUI_PORTING_DISPLAY_X11_

typedef struct _PL_X11_DISPLAY_STRUCT
{
    Display    *dpy;
    Window      win;
    GC          gc;

} PL_X11_DISPLAY;

extern int x11_width;
extern int x11_height;

#endif      /* _IFXUI_PORTING_DISPLAY_X11_ */
