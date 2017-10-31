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
*        ifxui_porting_display_fbdev_imx6.c
*
* COMPONENT
*
*        Inflexion UI Porting Layer.
*
* DESCRIPTION
*
*       Inflexion UI Engine Porting API Display Implementation for Linux fbdev
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

#if         (defined (IFX_RENDER_DIRECT_OPENGL) || \
            defined (IFX_RENDER_BUFFERED_OPENGL) ||\
            defined (IFX_RENDER_DIRECT_OPENGL_20)) && \
            defined (IFX_USE_EGL)

#include    "egl.h"
#define PL_USE_EGL

#endif


/* Linux fbdev and system includes */

#include    <linux/fb.h>
#include    <unistd.h>
#include    <sys/mman.h>
#include    <sys/ioctl.h>
#include    <sys/stat.h>
#include    <sys/errno.h>
#include    <fcntl.h>
#include    <linux/input.h>
#include    <termios.h>
#include    <linux/vt.h>
#include    <linux/kd.h>


/* Uncomment the following define to allow the porting layer to setup the
   fbdev memory as the foreground canvas.
   This will increase performance, however if the frame update and
   memory write are not in sync artifacts will be visible. */

/* #define     DIRECT_FB */

/* Uncomment the following define to force the porting layer to initialize
   the fbdev to the desired width and height. By default, the display is
   left as it was found and we try to best fit into it. */

/* #define FORCE_FBDEV_SETUP */

/* #define CLEAR_GLOBAL_ALPHA */

/* Work out the color channel offsets for when setting up the frame buffer */
#ifdef IFX_CANVAS_ORDER_BGR

#define ALPHA_OFFSET    (0)
#define RED_OFFSET      (ALPHA_OFFSET + IFXP_CANVAS_BITS_ALPHA)
#define GREEN_OFFSET    (RED_OFFSET   + IFXP_CANVAS_BITS_RED)
#define BLUE_OFFSET     (GREEN_OFFSET + IFXP_CANVAS_BITS_GREEN)

#else

#define BLUE_OFFSET     (0)
#define GREEN_OFFSET    (BLUE_OFFSET  + IFXP_CANVAS_BITS_BLUE)
#define RED_OFFSET      (GREEN_OFFSET + IFXP_CANVAS_BITS_GREEN)
#define ALPHA_OFFSET    (RED_OFFSET   + IFXP_CANVAS_BITS_RED)

#endif

typedef struct _PL_FBDEV_DISPLAY_STRUCT
{
    int                         fbdev;
    struct fb_var_screeninfo    varInfo;
    struct fb_fix_screeninfo    fixInfo;
#ifndef PL_USE_EGL
    unsigned char               *frameBuffer;
    int                         frameBufferLen;
#else
    EGLNativeDisplayType        native_display;
    EGLNativePixmapType         native_pixmap;
    EGLNativeWindowType         native_window;
#endif
} PL_FBDEV_DISPLAY;

/* Global variables */

IFX_DISPLAY     pl_display = {0};
static PL_FBDEV_DISPLAY pl_local_display = {0};
static struct fb_var_screeninfo pl_varInfo;

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
    IFX_RETURN_STATUS   ifxp_status = IFX_SUCCESS;

    int fbdev;
    int ret;
    char *fbEnv;
#ifdef CLEAR_GLOBAL_ALPHA
	struct mxcfb_gbl_alpha galpha;
	int ioRet;
#endif

    fbEnv = getenv("FRAMEBUFFER");

    if (fbEnv)
    {
        fbdev = open(fbEnv, O_RDWR);
    }
    else
    {
        fbdev = open("/dev/fb0", O_RDWR);
    }

    if (fbdev <= 0)
    {
        printf("Failed to open %s\n", fbEnv?fbEnv:"/dev/fb0");
        ifxp_status =  IFX_ERROR;
    }

#ifdef CLEAR_GLOBAL_ALPHA
	/* Clear the global alpha to fix brightness issues on the imx6 boards. */
	galpha.enable = 0;
	galpha.alpha = 255;
	ioRet = ioctl(fbdev, MXCFB_SET_GBL_ALPHA, &galpha);
	if (ioRet)
	{
		printf("Failed to clear alpha on %s\n", fbEnv?fbEnv:"/dev/fb0");
	}
#endif

    if (ifxp_status == IFX_SUCCESS)
    {
        ret = ioctl(fbdev, FBIOGET_VSCREENINFO, &pl_local_display.varInfo);
        if (ret >= 0)
        {
            ret = ioctl(fbdev, FBIOGET_FSCREENINFO, &pl_local_display.fixInfo);
        }

        if (ret < 0)
        {
            printf("Failed to configure the fb device (error = %d)\n", ret);
            ifxp_status = IFX_ERROR;
        }
    }

    if (ifxp_status == IFX_SUCCESS)
    {
        /* Make a copy of the framebuffer setup */
        memcpy(&pl_varInfo, &pl_local_display.varInfo, sizeof(pl_varInfo));

        pl_local_display.fbdev = fbdev;
    }

    return ifxp_status;
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
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;

#ifdef PL_USE_EGL
	if (pl_local_display.native_pixmap)
	{
	    fbDestroyPixmap(pl_local_display.native_pixmap);
		pl_local_display.native_pixmap = 0;
	}

	if (pl_local_display.native_window)
	{
	    fbDestroyWindow(pl_local_display.native_window);
		pl_local_display.native_window = 0;
	}
#endif

    if (pl_local_display.fbdev)
    {
        int ret;
        ret = ioctl(pl_local_display.fbdev, FBIOPUT_VSCREENINFO, &pl_varInfo);

        if (ret < 0)
        {
            printf("Failed to restore fb configuration (error = %d)\n", ret);
            ifxp_status = IFX_ERROR;
        }
    }

#ifndef PL_USE_EGL
    if (pl_display.frameBuffer)
        memset(pl_display.frameBuffer, 0, pl_display.stride * pl_display.height);
#endif

    if (pl_local_display.fbdev)
    {
#ifndef PL_USE_EGL
        if (pl_local_display.frameBuffer && pl_local_display.frameBufferLen)
            munmap(pl_local_display.frameBuffer,pl_local_display.frameBufferLen);
#endif

        close(pl_local_display.fbdev);
        pl_local_display.fbdev = 0;
    }

    return ifxp_status;
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
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;
    int ret = 0;

    if (display_mode == 0)
    {
        return IFX_ERROR;
    }

    if (width <= 0 || height <= 0)
    {
        /* The requested mode is not supported */
        printf("The requested display mode is not supported\r\n");
        ifxp_status = IFX_ERROR;
    }

#ifndef PL_USE_EGL
    if (pl_local_display.fbdev <= 0)
    {
        printf("No frame buffer available!\n");
        ifxp_status = IFX_ERROR;
    }
#endif

    /* Request the size from the frame buffer */
    if ((IFX_SUCCESS == ifxp_status) && (pl_local_display.fbdev > 0))
    {

#ifdef FORCE_FBDEV_SETUP
        printf("Configuring display for \"%s\" (%dx%d)\n", display_mode, width, height);

        pl_local_display.varInfo.xres            = width;
        pl_local_display.varInfo.yres            = height;

        pl_local_display.varInfo.xres_virtual    = pl_local_display.varInfo.xres;
        pl_local_display.varInfo.yres_virtual    = pl_local_display.varInfo.yres * 3;
        pl_local_display.varInfo.bits_per_pixel  = IFXP_CANVAS_BPP;


        pl_local_display.varInfo.transp.length   = IFXP_CANVAS_BITS_ALPHA;
        pl_local_display.varInfo.transp.offset   = ALPHA_OFFSET;
        pl_local_display.varInfo.red.length      = IFXP_CANVAS_BITS_RED;
        pl_local_display.varInfo.red.offset      = RED_OFFSET;
        pl_local_display.varInfo.green.length    = IFXP_CANVAS_BITS_GREEN;
        pl_local_display.varInfo.green.offset    = GREEN_OFFSET;
        pl_local_display.varInfo.blue.length     = IFXP_CANVAS_BITS_BLUE;
        pl_local_display.varInfo.blue.offset     = BLUE_OFFSET;

        ret = ioctl(pl_local_display.fbdev, FBIOPUT_VSCREENINFO, &pl_local_display.varInfo);
#endif

        if (ret < 0)
        {
            printf("Couldn't set screen configuration\n");
            ifxp_status = IFX_ERROR;
        }
        else
        {
            ioctl(pl_local_display.fbdev, FBIOGET_VSCREENINFO, &pl_local_display.varInfo);

#ifdef FORCE_FBDEV_SETUP
            printf("Screen set to:\n\txres=%d\n\txres_virtual=%d\n\tyres=%d\n\tyres_virtual=%d\n\tbpp=%d\n",
                pl_local_display.varInfo.xres,
                pl_local_display.varInfo.xres_virtual,
                pl_local_display.varInfo.yres,
                pl_local_display.varInfo.yres_virtual,
                pl_local_display.varInfo.bits_per_pixel);
#endif
        }
    }

    /* Use the true values from the display if possible */
    width = pl_local_display.varInfo.xres;
    height = pl_local_display.varInfo.yres;

#ifndef PL_USE_EGL
    /* Map the frame buffer into user space */
    if (IFX_SUCCESS == ifxp_status)
    {
        int size;

        if (pl_local_display.frameBuffer && pl_local_display.frameBufferLen)
            munmap(pl_local_display.frameBuffer, pl_local_display.frameBufferLen);

        size = pl_local_display.fixInfo.line_length * height;
        if (size > pl_local_display.fixInfo.smem_len)
        {
            size = width * height * (pl_local_display.varInfo.bits_per_pixel / 8);
        }

        if ((pl_local_display.fixInfo.smem_len != 0) && (size > pl_local_display.fixInfo.smem_len))
        {
            printf("Insufficient memory for frame buffer\n");
            ifxp_status = IFX_ERROR;
        }
        else
        {
            pl_local_display.frameBuffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, pl_local_display.fbdev, 0);
            if (pl_local_display.frameBuffer == (void*)MAP_FAILED)
            {
                printf("Couldn't map frame buffer memory into user space (%d)\n", errno);
                ifxp_status = IFX_ERROR;
            }
        }
    }
#endif

    if (IFX_SUCCESS == ifxp_status)
    {
        char *msaa_samples = getenv("IFX_ENV_MSAA_SAMPLES");

        /* Set up the global display parameters */
        pl_display.offsetX = left;
        pl_display.offsetY = top;
        pl_display.width   = width;
        pl_display.height  = height;
        pl_display.bpp     = pl_local_display.varInfo.bits_per_pixel;
        pl_display.stride  = width * (pl_display.bpp / 8);
        pl_display.depth_buffer_size = 24;
        pl_display.egl_swap_interval = 1;


        pl_display.internal = (void*)&pl_local_display;
#ifndef PL_USE_EGL
        pl_display.frameBuffer = pl_local_display.frameBuffer;
#endif

        /* Set the MSAA configuration */
        if (msaa_samples)
        {
            pl_display.msaa_samples = atoi(msaa_samples);
        }

        /*
           We do not support rendering the full frame
           on IFXP_Render_Display_Start
        */
        pl_display.renderFull = 0;

        /* Default is no rotation */
        pl_display.rotation = IFX_ROTATE_0;

#ifdef PL_USE_EGL
        /* (Re)Create the native display's */
		if (pl_local_display.native_pixmap)
		{
			fbDestroyPixmap(pl_local_display.native_pixmap);
			pl_local_display.native_pixmap = 0;
		}

		if (pl_local_display.native_window)
		{
			fbDestroyWindow(pl_local_display.native_window);
			pl_local_display.native_window = 0;
		}

        EGLNativeDisplayType native_display = fbGetDisplay();
        //pl_local_display.native_pixmap  = fbCreatePixmap(native_display, width, height, 32);
        pl_local_display.native_window  = fbCreateWindow(native_display, 0, 0, width, height);
#endif

        /* Mark us as initialized */
        pl_display_info_initialized = 1;
    }
    else
    {
        ifxp_status = IFX_ERROR;
    }

#ifndef PL_USE_EGL
    if (IFX_SUCCESS == ifxp_status)
        memset(pl_display.frameBuffer, 0, pl_display.stride * pl_display.height);
#else
    /* In the EGL case, close the framebuffer device now - required on some platforms
       to avoid conflicts in the EGL layer */
    if (pl_local_display.fbdev)
        close(pl_local_display.fbdev);
    pl_local_display.fbdev = 0;
#endif

    return ifxp_status;
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
#if defined(DIRECT_FB) && !defined(PL_USE_EGL)
        if (fgbg == IFXP_FOREGROUND)
        {
            local_canvas->colorBuffer = pl_local_display.frameBuffer;
        }
        else
#endif
        {
            ifxp_status = IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL, (IFXP_CANVAS_BPP/8)*width*height, (void**)&local_canvas->colorBuffer);
        }
    }

    if (ifxp_status == IFX_SUCCESS)
    {

        local_canvas->width = width;
        local_canvas->stride = (IFXP_CANVAS_BPP/8)*width;
        local_canvas->height = height;
        memset(local_canvas->colorBuffer, 0, local_canvas->stride * local_canvas->height);

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
    if (!canvas || !pl_display_info_initialized)
    {
        return IFX_ERROR;
    }

#if defined(DIRECT_FB) && !defined(PL_USE_EGL)
    if (canvas->colorBuffer != pl_display.frameBuffer)
#endif
    if (canvas->colorBuffer)
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, canvas->colorBuffer);

    /* Free the allocated memory. */
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

#ifndef PL_USE_EGL

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
    IFX_INT32 width = 0;
    IFX_INT32 height = 0;
    unsigned char* ptr;

    if ((!pl_local_display.frameBuffer) ||
          (!src_canvas) || (!src_rect)
          || (!pl_display_info_initialized))
    {
        return IFX_ERROR;
    }

#ifndef DIRECT_FB

    width = (src_rect->x2 - src_rect->x1);
    height = (src_rect->y2 - src_rect->y1);

    /* Apply the offset */
    left += pl_display.offsetX;
    top  += pl_display.offsetY;

    if (width == 0 || height == 0)
     return IFX_ERROR;


    /* Ensure that we crop to the display size */
    if (pl_local_display.varInfo.xres_virtual < (src_rect->x1 + pl_display.offsetX))
    {
        return IFX_ERROR;
    }

    if (pl_local_display.varInfo.yres_virtual < (src_rect->y1 + pl_display.offsetY))
    {
        return IFX_ERROR;
    }

    if (pl_local_display.varInfo.xres_virtual < (src_rect->x2 + pl_display.offsetX))
        width -= ((src_rect->x2 + pl_display.offsetX) - pl_local_display.varInfo.xres_virtual);
    if (pl_local_display.varInfo.yres_virtual < (src_rect->y2 + pl_display.offsetY))
        height -= ((src_rect->y2 + pl_display.offsetY) - pl_local_display.varInfo.yres_virtual);

    {
        unsigned char* srcPtr = src_canvas->colorBuffer + ((src_rect->y1 * src_canvas->stride) + (src_rect->x1 * (IFXP_CANVAS_BPP/8)));
        unsigned char* dstPtr = pl_local_display.frameBuffer + ((top * pl_display.stride) + (left * (pl_local_display.varInfo.bits_per_pixel/8)));
        unsigned int srcInc = src_canvas->stride;
        unsigned int dstInc = pl_display.stride;
        int y;
        int copyLen;

        copyLen = width*(IFXP_CANVAS_BPP/8);

        for (y=0; y<height; y++)
        {
            memcpy(dstPtr, srcPtr, copyLen);
            dstPtr += dstInc;
            srcPtr += srcInc;
        }
    }

#endif


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
printf("iiii%s %d\n",__func__,__LINE__);
    return IFX_SUCCESS;
}
#endif /* PL_USE_EGL */

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
	static int i;
	if(i==0)
	{
		i=1;
		Engine_Render_End();
	}
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
	EGLNativeDisplayType native_display = fbGetDisplay();
    return native_display;
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
    return pl_local_display.native_window;
}

#endif      /* (defined (IFX_RENDER_DIRECT_OPENGL) || \
                defined (IFX_RENDER_DIRECT_OPENGL_20) || \
                defined (IFX_RENDER_BUFFERED_OPENGL)) && \
                defined (IFX_USE_EGL) */
