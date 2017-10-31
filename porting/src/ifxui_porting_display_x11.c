/************************************************************************
*
*               Copyright 2006 Mentor Graphics Corporation
*                         All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
*************************************************************************

*************************************************************************
* FILE NAME
*
*        ifxui_porting_display_x11.c
*
* COMPONENT
*
*        Inflexion UI Porting Layer.
*
* DESCRIPTION
*
*       Inflexion UI Engine Porting API Display Implementation for Linux X11
*
* DATA STRUCTURES
*
*       ifxp_display                        Engine porting layer display
*                                           object.
*
* FUNCTIONS
*
*       PL_Display_Initialize               Initializes the display module.
*
*       PL_Display_Shutdown                 Shuts down the display module.
*
*       IFXP_Display_Set_Mode               Sets up the display mode.
*
*       IFXP_Display_Get_Info               Populates the display information
*                                           structure.
*
*       IFXP_Canvas_Create                  Create a canvas for drawing.
*
*       IFXP_Canvas_Destroy                 Destroys a previously created
*                                           canvas.
*
*       IFXP_Canvas_Draw_Rect_Fill          Draws a filled rectangle to the
*                                           canvas.
*
*       IFXP_Canvas_Draw_Rect_Debug         Draws a debug rectangle to the
*                                           canvas.
*
*       IFXP_Render_Start                   Start of the rendering cycle.
*
*       IFXP_Render_Background_Start        Background rendering start step.
*
*       IFXP_Render_Background              Copies background canvas to
*                                           display canvas.
*
*       IFXP_Render_Background_End          Background rendering stop step.
*
*       IFXP_Render_Foreground_Start        Foreground rendering start step.
*
*       IFXP_Render_Foreground_End          Foreground rendering stop step.
*
*       IFXP_Render_Display_Start           Function called before the
*                                           start blit.
*
*       IFXP_Render_Display                 Blits the canvas data.
*
*       IFXP_Render_Display_End             Function called after the
*                                           completion of blit.
*
*       IFXP_Render_End                     End of the rendering cycle.
*
*       IFXP_Egl_Get_Default_Display        Retrieve the default display
*                                           for EGL.
*
*       IFXP_Egl_Get_Default_Window         Retrieve the default window
*                                           for EGL.
*
*       IFXP_Display_Error_Note             Displays error note on canvas.
*
* DEPENDENCIES
*
*       ifxui_porting_linux.h               Main include for Linux
*                                           porting layer.
*
************************************************************************/

/* Inflexion UI Engine includes. */
#include    "inflexionui/porting/inc/ifxui_porting_linux.h"
#include    "inflexionui/framework/inc/framework_linux.h"

/* Linux includes. */
#include    <X11/X.h>
#include    <X11/Xlib.h>
#include    <X11/Xutil.h>

#include    "inflexionui/porting/inc/ifxui_porting_display_x11.h"

#if         !(defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20))

#if (IFXP_CANVAS_BPP != 24)
    #error Sorry, the xlib implementation only supports 24bpp mode - please check rules.mk
#endif

#include    <gdk-pixbuf-xlib/gdk-pixbuf-xlibrgb.h>

#endif      /* !(defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20)) */

/* Global variables */
int x11_width = 0;
int x11_height = 0;

IFX_DISPLAY pl_display = {0};
static PL_X11_DISPLAY x11_local_display = {0};

static int pl_display_info_initialized = 0;

/*************************************************************************
* FUNCTION
*
*       PL_Display_Initialize
*
* DESCRIPTION
*
*       This function initializes the display module.
*
* INPUTS
*
*       None
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   PL_Display_Initialize(void)
{
    x11_local_display.dpy = XOpenDisplay(NULL);

    if (!x11_local_display.dpy)
        return IFX_ERROR;

#if         !(defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20))
    xlib_rgb_init(x11_local_display.dpy, DefaultScreenOfDisplay(x11_local_display.dpy));
#endif      /* !defined (IFX_RENDER_DIRECT_OPENGL) */

    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       PL_Display_Shutdown
*
* DESCRIPTION
*
*       This function shuts down the display module.
*
* INPUTS
*
*       None
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   PL_Display_Shutdown(void)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (x11_local_display.dpy)
    {
#if         !(defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20))
        if (x11_local_display.gc)
        {
            XFreeGC (x11_local_display.dpy, x11_local_display.gc);
        }
#endif      /* !defined (IFX_RENDER_DIRECT_OPENGL) */
        if (x11_local_display.win)
        {
            XDestroyWindow(x11_local_display.dpy, x11_local_display.win);
        }
        XCloseDisplay(x11_local_display.dpy);
    }
#if         !(defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20))
    xlib_rgb_init(0, 0);
#endif      /* !defined (IFX_RENDER_DIRECT_OPENGL) */
    memset(&x11_local_display, 0, sizeof(x11_local_display));

    return status;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_Display_Set_Mode
*
*   DESCRIPTION
*
*       This function sets up the display mode so that the IFXP knows
*       which mode we are blitting in (e.g. to allow rotation when
*       blitting a landscape canvas onto a portrait display).
*
*   INPUTS
*
*       left                - The x offset to output to screen at.
*
*       top                 - The y offset to output to screen at.
*
*       width               - Width in pixels to match the display_mode.
*
*       height              - Height in pixels to match the display_mode.
*
*       display_mode        - Pointer to variable containing the display mode string.
*
*   OUTPUTS
*
*       IFX_SUCCESS         - On success.
*       IFX_ERROR           - If a parameter is invalid.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Display_Set_Mode(IFX_INT32 left, IFX_INT32 top,
                                          IFX_INT32 width, IFX_INT32 height,
                                          const char *display_mode)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (display_mode == 0)
    {
        return IFX_ERROR;
    }

    if (width <= 0 || height <= 0)
    {
        /* The requested mode is not supported */
        status = IFX_ERROR;
    }

    if (IFX_SUCCESS == status)
    {
        if(x11_local_display.win)
        {
            XDestroyWindow(x11_local_display.dpy,x11_local_display.win);
        }

        XRectangle winrect;
        Window root = XDefaultRootWindow(x11_local_display.dpy);
        IFX_INT32 blackColour = BlackPixel(x11_local_display.dpy, DefaultScreen(x11_local_display.dpy));

        winrect.x = 0;
        winrect.y = 0;
        winrect.width = width;
        winrect.height = height;

        x11_local_display.win = XCreateSimpleWindow( x11_local_display.dpy, root, winrect.x, winrect.y, winrect.width, winrect.height,
                                   0, blackColour, blackColour);

        if (x11_local_display.win)
        {

#if         !(defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20))
            x11_local_display.gc = XCreateGC (x11_local_display.dpy, x11_local_display.win, 0, 0);

            if (!x11_local_display.gc)
            {
            	status = IFX_ERROR;
            }
#endif      /* !defined (IFX_RENDER_DIRECT_OPENGL) */

            if(status==IFX_SUCCESS)
            {
                /* input event selection */
                XSelectInput (x11_local_display.dpy, x11_local_display.win, ButtonPressMask | ButtonReleaseMask | KeyPressMask | ExposureMask | ButtonMotionMask | PointerMotionMask | StructureNotifyMask);

                /* window mapping */
                XMapRaised (x11_local_display.dpy, x11_local_display.win);
                XFlush(x11_local_display.dpy);
            }
            else
            {
                status = IFX_ERROR;
            }
        }
        else
        {
            status = IFX_ERROR;
        }
    }

    if (IFX_SUCCESS == status)
    {
        /* Set up the global display parameters */
        pl_display.offsetX = left;
        pl_display.offsetY = top;
        pl_display.width   = width;
        pl_display.height  = height;
        pl_display.stride  = 3*pl_display.width;
        pl_display.frameBuffer = 0;
        pl_display.internal = (void*)&x11_local_display;
        pl_display.bpp     = IFXP_CANVAS_BPP;
        pl_display.msaa_samples = 4;
/*      pl_display.depth_buffer_size = 24;     /* Depth size*/
/*      pl_display.egl_swap_interval = 0;      /* Egl Swap interval*/

        /* Store the dimensions locally (to X11 module) */
        x11_width = width;
        x11_height = height;

        /*
           We do not support rendering the full frame
           on IFXP_Render_Display_Start
        */
        pl_display.renderFull = 0;

        /* Default is no rotation */
        pl_display.rotation = IFX_ROTATE_0;

        /* Mark us as initialized */
        pl_display_info_initialized = 1;
    }

    return status;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Display_Get_Info
*
* DESCRIPTION
*
*       Populates the display information structure.
*
* INPUTS
*
*       display                             Pointer to display structure
*                                           pointer to be populated.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Display_Get_Info(IFX_DISPLAY** display)
{
    if (!pl_display_info_initialized)
    {
        return IFX_ERROR;
    }

    *display = &pl_display;

    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Canvas_Create
*
* DESCRIPTION
*
*       Create a canvas matching the bit depth defined by
*       IFX_CANVAS_MODE_xxxx in ifxui_config.h.
*
* INPUTS
*
*       canvas                              Pointer to canvas pointer to be
*                                           populated with an allocated
*                                           IFX_CANVAS structure.
*
*       width                               Width of the canvas in pixels.
*
*       height                              Height of the canvas in rows
*                                           (pixels).
*
*       fgbg                                Is this a foreground or
*                                           background canvas
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Canvas_Create(IFX_CANVAS** canvas,
                                       IFX_UINT32 width,
                                       IFX_UINT32 height,
                                       IFXP_FGBG fgbg)
{
    IFX_RETURN_STATUS ifxp_status = IFX_ERROR;
    IFX_CANVAS *local_canvas;

    ifxp_status = IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL, sizeof(IFX_CANVAS), (void**)&local_canvas);

    if (ifxp_status == IFX_SUCCESS)
    {
        ifxp_status = IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL, (IFXP_CANVAS_BPP/8)*width*height, (void**)&local_canvas->colorBuffer);
    }

    if (ifxp_status == IFX_SUCCESS)
    {

        local_canvas->width = width;
        local_canvas->stride = (IFXP_CANVAS_BPP/8)*width;
        local_canvas->height = height;

        *canvas = local_canvas;
    }

    return ifxp_status;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Canvas_Destroy
*
* DESCRIPTION
*
*       Destroys a previously created canvas
*
* INPUTS
*
*       canvas                              Pointer to an IFX_CANVAS
*                                           structure.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
************************************************************************/
IFX_RETURN_STATUS   IFXP_Canvas_Destroy(IFX_CANVAS *canvas)
{
    if (!canvas)
    {
        return IFX_ERROR;
    }

    if (canvas->colorBuffer)
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, canvas->colorBuffer);

    IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, canvas);

    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Canvas_Draw_Rect_Fill
*
* DESCRIPTION
*
*       Draws a filled rectangle to the canvas
*
* INPUTS
*
*       dst_canvas                          The canvas to draw to.
*
*       dst_rect                            The rectangle to draw.
*
*       dst_color                           Color of the rectangle.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Canvas_Draw_Rect_Fill(IFX_CANVAS *dst_canvas,
                                               IFXP_RECT *dst_rect,
                                               IFX_INT32 dst_color)
{
    IFX_INT32 width = 0;
    IFX_INT32 height = 0;

    if ( (!dst_canvas) || (!dst_rect) )
    {
        return IFX_ERROR;
    }

    width = (dst_rect->x2 - dst_rect->x1);
    height = (dst_rect->y2 - dst_rect->y1);

    if (width == 0 || height == 0)
        return IFX_ERROR;

    {
        unsigned char* dst_ptr = dst_canvas->colorBuffer + ((dst_rect->y1 * dst_canvas->stride) + (dst_rect->x1 * (IFXP_CANVAS_BPP / 8)));
        IFX_UINT32 dst_inc = dst_canvas->stride;
        IFX_INT32 y;

        for (y=0; y<height; y++)
        {
            memset(dst_ptr, dst_color, width * (IFXP_CANVAS_BPP / 8));
            dst_ptr += dst_inc;
        }
    }

    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Canvas_Draw_Rect_Debug
*
* DESCRIPTION
*
*       Draws a debug rectangle to the canvas
*
* INPUTS
*
*       dst_canvas                          The canvas to draw to.
*
*       dst_rect                            The rectangle to draw.
*
*       dst_color                           Color of the rectangle.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
************************************************************************/
IFX_RETURN_STATUS   IFXP_Canvas_Draw_Rect_Debug(IFX_CANVAS *dst_canvas,
                                                IFXP_RECT *dst_rect,
                                                IFX_INT32 dst_color)
{
    if ( (!dst_canvas) || (!dst_rect) )
    {
        return IFX_ERROR;
    }
    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Render_Start
*
* DESCRIPTION
*
*       This function is called at the start of the render cycle.
*       It is provided to allow synchronization with an external system
*       and is called in all render modes.
*
* INPUTS
*
*       None.
*
* OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*************************************************************************/
IFX_RETURN_STATUS IFXP_Render_Start(void)
{
    return (IFX_SUCCESS);
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Render_Background_Start
*
* DESCRIPTION
*
*       This function is called before the background rendering step.
*       For an asynchronous system, this function shall block until any
*       active operations on the specified canvas have completed.
*       (e.g. an asynchronous blit to display started on the previous cycle).
*       For synchronous implementations, this shall always return success
*       immediately.
*
* INPUTS
*
*       dst_canvas                          The canvas in use for this
*                                           render cycle.
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
************************************************************************/
IFX_RETURN_STATUS   IFXP_Render_Background_Start(IFX_CANVAS *dst_canvas)
{
    return IFX_SUCCESS;
}

/************************************************************************
* FUNCTION
*
*       IFXP_Render_Background
*
* DESCRIPTION
*
*       This function is called to copy portions of src_canvas
*       (background) bounded by src_rect to dst_canvas (the canvas)
*       at point dstPt. It will be called 1 or more times depending on
*       the number of rectangles that require copying.
*       For an asynchronous system, this function may queue multiple
*       blits, or where a queue is not available, this function shall
*       ensure that appropriate action is taken to ensure the correct
*       execution of previous operations before starting a new operation.
*       For synchronous implementations, this function shall perform
*       the requested blit and return.
*
* INPUTS
*
*       dst_canvas                          The canvas to blit to.
*
*       left                                The destination left corner.
*
*       top                                 The destination top corner.
*
*       src_canvas                          The canvas to blit from.
*
*       src_rect                            The source area.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
************************************************************************/
IFX_RETURN_STATUS   IFXP_Render_Background(IFX_CANVAS *dst_canvas,
                                           IFX_INT32 left,
                                           IFX_INT32 top,
                                           IFX_CANVAS *src_canvas,
                                           IFXP_RECT *src_rect)
{
    IFX_INT32 width = 0;
    IFX_INT32 height = 0;

    if ((!dst_canvas) || (!src_canvas) || (!src_rect))
    {
        return IFX_ERROR;
    }

    width = (src_rect->x2 - src_rect->x1);
    height = (src_rect->y2 - src_rect->y1);

    if (width == 0 || height == 0)
        return IFX_ERROR;

    {
        unsigned char* src_ptr = src_canvas->colorBuffer + ((src_rect->y1 * src_canvas->stride) + (src_rect->x1 * (IFXP_CANVAS_BPP / 8)));
        unsigned char* dst_ptr = dst_canvas->colorBuffer + ((top * dst_canvas->stride) + (left * (IFXP_CANVAS_BPP / 8)));
        IFX_UINT32 src_inc = src_canvas->stride;
        IFX_UINT32 dst_inc = dst_canvas->stride;
        IFX_INT32 y;

        for (y=0; y<height; y++)
        {
            memcpy(dst_ptr, src_ptr, width * (IFXP_CANVAS_BPP / 8));
            dst_ptr += dst_inc;
            src_ptr += src_inc;
        }
    }
    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Render_Background_End
*
* DESCRIPTION
*
*       This function is called after the background rendering step.
*
* INPUTS
*
*       dst_canvas                          The canvas in use for this
*                                           render cycle.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Render_Background_End(IFX_CANVAS *dst_canvas)
{
    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Render_Foreground_Start
*
* DESCRIPTION
*
*       This function is called before the internal foreground rendering step.
*       For an asynchronous system, this function shall block until any
*       active operations on the specified canvas have completed.
*       (e.g. an asynchronous background blit).
*       It shall also lock the canvas if required so that UI Engine can
*       write directly to the canvas.
*
* INPUTS
*
*       canvas                              The canvas in use for this
*                                           render cycle.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
************************************************************************/
IFX_RETURN_STATUS   IFXP_Render_Foreground_Start(IFX_CANVAS *canvas)
{
    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Render_Foreground_End
*
* DESCRIPTION
*
*       This function is called after the internal foreground rendering step.
*       For an asynchronous system, this function shall unlock the canvas
*       if required.
*
* INPUTS
*
*       canvas                              The canvas in use for this
*                                           render cycle.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
************************************************************************/
IFX_RETURN_STATUS IFXP_Render_Foreground_End (IFX_CANVAS *canvas)
{
    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Render_Display_Start
*
* DESCRIPTION
*
*       This function is called before the start of the display blit(s).
*       There will be no outstanding operations at this point due to the
*       requirements above, so this function shall never block.
*       Where the return is a non-zero value, the Engine will assume
*       that the whole canvas has been transferred to the display by this
*       call. An example of this would be where double buffering is used
*       and the frame buffer pointer is switched to this canvas.
*       There will be no following calls to IFXP_Render_Display, and the
*       canvas is guaranteed not to be modified before the next call to
*       IFXP_Render_Background_Start with this canvas as the parameter.
*
* INPUTS
*
*       canvas                              The canvas in use for this
*                                           render cycle.
*
* OUTPUTS
*
*        IFX_SUCCESS                        Succeeded, full frame was not
*                                           blitted.
*
*        IFX_ERROR                          An error occurred
*
************************************************************************/
IFX_RETURN_STATUS   IFXP_Render_Display_Start(IFX_CANVAS *canvas)
{
    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Render_Display
*
* DESCRIPTION
*
*       Blits the canvas data bounded by src_rect to the display
*       starting at dstPt.
*       Where the display depth and canvas depth do not match, color
*       conversion must be performed by this function.
*       For an asynchronous system, this function may be called several
*       times to queue multiple blits. Where a queue is not available,
*       this function shall ensure the correct execution of previous
*       operations before starting a new operation.
*       For synchronous implementations, this function shall perform
*       the requested blit and return.
*
* INPUTS
*
*       left                                The destination left corner.
*
*       top                                 The destination top corner.
*
*       src_canvas                          The canvas to blit from.
*
*       src_rect                            The source area.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Render_Display(IFX_INT32 left, IFX_INT32 top,
                                        IFX_CANVAS *src_canvas,
                                        IFXP_RECT *src_rect)
{

#if         !(defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20))

    IFX_INT32 width = 0;
    IFX_INT32 height = 0;
    unsigned char* ptr;

    if ((!src_canvas) || (!src_rect) )
    {
        return IFX_ERROR;
    }

    width = (src_rect->x2 - src_rect->x1);
    height = (src_rect->y2 - src_rect->y1);

    if (width == 0 || height == 0)
        return IFX_ERROR;

    ptr = src_canvas->colorBuffer + ((src_rect->y1 * src_canvas->stride) + (src_rect->x1 * (IFXP_CANVAS_BPP / 8)));

    if (x11_local_display.gc)
    {
        xlib_draw_rgb_image (x11_local_display.win, x11_local_display.gc,
                             left + pl_display.offsetX,
                             top  + pl_display.offsetY,
                             width, height,
                             XLIB_RGB_DITHER_NORMAL,
                             ptr,
                             src_canvas->stride);
    }

#endif         /* !defined (IFX_RENDER_DIRECT_OPENGL) */

    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Render_Display_End
*
* DESCRIPTION
*
*       This function is called at the end of a render cycle.
*       For an asynchronous system, this should start any queued
*       asynchronous operations. Where a queue is not available this
*       function shall return success immediately.
*       On a synchronous system, this function shall return success
*       immediately.
*
* INPUTS
*
*       canvas                              The canvas in use for this
*                                           render cycle.
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
************************************************************************/
IFX_RETURN_STATUS   IFXP_Render_Display_End(IFX_CANVAS *canvas)
{
#if         !(defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20))

    XFlush (x11_local_display.dpy);

#endif      /* !defined (IFX_RENDER_DIRECT_OPENGL) */

    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Render_End
*
* DESCRIPTION
*
*       This function is called at the end of the render cycle.
*       It is provided to allow synchronization with an external system
*       and is called in all render modes.
*
* INPUTS
*
*       None.
*
* OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*************************************************************************/
IFX_RETURN_STATUS IFXP_Render_End(void)
{
    return (IFX_SUCCESS);
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Display_Error_Note
*
* DESCRIPTION
*
*       This function is called when a critical error has occurred within
*       the Engine and must be relayed to the user. An example of this is
*       when there is insufficient memory to perform an operation.
*       The implementation must have pre-allocated any resource required to
*       display the note, as it is likely that there is no free memory
*       remaining on the system when called.
*       We leave the precise behavior of this function to the implementer.
*       This function should return on completing the display process,
*       the Engine will then repaint the UI and continue if possible.
*
* INPUTS
*
*       error_text                          The text to be displayed in the
*                                           error note. The maximum length
*                                           of this string is 256 chars.
*
* OUTPUTS
*
*       IFX_SUCCESS                         If the error note displayed.
*
*       IFX_ERROR                           Any error.
*
************************************************************************/
IFX_RETURN_STATUS   IFXP_Display_Error_Note(const IFX_WCHAR *error_text)
{
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;
    IFX_WCHAR *wchar_ptr = (IFX_WCHAR*)error_text;

    printf("ERROR: ");
    while (*wchar_ptr != 0)
    {
        printf("%c", (char)*wchar_ptr);
        wchar_ptr++;
    }
    printf("\r\n");

    return ifxp_status;
}

#if         (defined (IFX_RENDER_DIRECT_OPENGL) || \
            defined (IFX_RENDER_DIRECT_OPENGL_20) || \
            defined (IFX_RENDER_BUFFERED_OPENGL)) && \
            defined (IFX_USE_EGL)

/************************************************************************
* FUNCTION
*
*       IFXP_Egl_Get_Default_Display
*
* DESCRIPTION
*
*       Retrieve the default display for EGL
*
* INPUTS
*
*       None
*
* OUTPUTS
*
*       EGL NativeDisplayType
*
************************************************************************/
NativeDisplayType   IFXP_Egl_Get_Default_Display(void)
{
    return (NativeDisplayType)x11_local_display.dpy;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Egl_Get_Default_Window
*
* DESCRIPTION
*
*       Retrieve the default window for EGL
*
* INPUTS
*
*       None
*
* OUTPUTS
*
*       EGL NativeWindowType
*
*************************************************************************/
NativeWindowType IFXP_Egl_Get_Default_Window(void)
{
    return (NativeWindowType)x11_local_display.win;
}

#endif      /* (defined (IFX_RENDER_DIRECT_OPENGL) ||
               defined (IFX_RENDER_BUFFERED_OPENGL)) &&
               defined (IFX_USE_EGL)*/

int PL_Process_Input_Events(void)
{
    XEvent myevent;
    int done = 0;
    int send_status;
    IFX_UINT32 engine_msg[IFXF_QUEUE_ELEMENT_SIZE] = {0};

#if !(defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20))
    Display *dpy = x11_local_display.dpy;
#else
    Display *dpy = (Display *)IFXP_Egl_Get_Default_Display();
#endif

    if (dpy)
    {
        /* read the next event(s) */
        while (XCheckMaskEvent(dpy, ButtonPressMask | ButtonReleaseMask | KeyPressMask | ExposureMask | ButtonMotionMask | PointerMotionMask | StructureNotifyMask, &myevent))
        {
            engine_msg[0] = 0;

            switch (myevent.type)
            {
                /* repaint window on expose events */
                case Expose:
                    //IFXE_Suspend();
                    engine_msg[0] = IFXFRefreshDisplay;
                break;

                /* process mouse-button presses */
                case ButtonPress:
                    {
                        XButtonEvent *event = (XButtonEvent*)&myevent;
                        engine_msg[0] = IFXFTouchDown;
                        engine_msg[1] = event->x;
                        engine_msg[2] = event->y;
                    }
                break;

                case ButtonRelease:
                    {
                        XButtonEvent *event = (XButtonEvent*)&myevent;
                        engine_msg[0] = IFXFTouchUp;
                        engine_msg[1] = event->x;
                        engine_msg[2] = event->y;
                    }
                break;

                case MotionNotify:
                    {
                        XMotionEvent *event = (XMotionEvent*)&myevent;
                        engine_msg[0] = IFXFTouchMove;
                        engine_msg[1] = event->x;
                        engine_msg[2] = event->y;
                    }
                break;

                /* process keyboard input */
                case KeyPress:
                    {
                        XKeyEvent *event = (XKeyEvent*)&myevent;
                        KeySym keySym;

                        keySym = XLookupKeysym(event, 0);

                        switch (keySym)
                        {
                            case 0xFF52:    // Cursor Up
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = IFX_KEY_CODE_UP;
                            break;

                            case 0xFF54:    // Cursor Down
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = IFX_KEY_CODE_DOWN;
                            break;

                            case 0xFF51:    // Cursor Left
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = IFX_KEY_CODE_LEFT;

                            break;

                            case 0xFF53:    // Cursor Right
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = IFX_KEY_CODE_RIGHT;
                            break;

                            case 0xFF0D:    // Enter
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = IFX_KEY_CODE_SELECT;
                            break;

                            case 0xFF08:    // Backspace
                            case 0xFF1B:    // Escape
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = IFX_KEY_CODE_BACK;

                            break;

                            case '*':
                            case 0xFFE2:
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = IFX_KEY_CODE_ASTERISK;

                            break;

                            case '#':
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = IFX_KEY_CODE_HASH;

                            break;

                            case '0':
                            case 0xFF9E:
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = '0';

                            break;

                            case '1':
                            case 0xFF9C:
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = '1';

                            break;

                            case '2':
                            case 0xFF99:
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = '2';

                            break;

                            case '3':
                            case 0xFF9B:
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = '3';

                            break;

                            case '4':
                            case 0xFF96:
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = '4';

                            break;

                            case '5':
                            case 0xFF9D:
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = '5';

                            break;

                            case '6':
                            case 0xFF98:
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = '6';

                            break;

                            case '7':
                            case 0xFF95:
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = '7';

                            break;

                            case '8':
                            case 0xFF97:
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = '8';

                            break;

                            case '9':
                            case 0xFF9A:
                                engine_msg[0] = IFXFKeyDown;
                                engine_msg[1] = '9';

                            break;

                            default:
                            break;

                        }
                    }
                break;

                default:
                break;
            } /* switch (myevent.type) */

            /* Send the event in message queue. */
            if (engine_msg[0] > 0)
            {
                send_status = msgsnd(IFXF_Message_Queue_ID, engine_msg,
                                     IFXF_QUEUE_ELEMENT_SIZE * sizeof(IFX_UINT32),0);
                if (send_status < 0)
                {
                    printf("Failed to send message (%d)\n", errno);
                }
            }

        }
    }

    return  done;
}
