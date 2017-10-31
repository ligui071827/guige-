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

#ifndef IFXUI_DEFS_H
#define IFXUI_DEFS_H

/* Include headers. */
#include "inflexionui/engine/ifxui_rtl.h"

#ifndef IFX_ASSERT
    #define IFX_ASSERT(a)
#endif

#define IFX_UNUSED(p)   (void)p

/* Boolean */
typedef int IFX_BOOL;
#define IFX_FALSE   0
#define IFX_TRUE    1

/* 32-bit types */
#if (ULONG_MAX != 0xFFFFFFFFUL)
#error long is not 32-bits. Please update IFX_INT32 and IFX_UINT32 typedefs in ifxui_defs.h
#endif
typedef unsigned long  IFX_UINT32;
typedef signed long    IFX_INT32;

//number of unsigned longs in hash
#define IFX_HASH_SIZE          5

typedef struct _tagIFX_SIGNATURE
{
    IFX_UINT32 hash[IFX_HASH_SIZE];
} IFX_SIGNATURE;

/*
    IFXI_API Type Definitions.

    These define the types used by the API.
*/

/* File handle type */
typedef void* IFXE_FILE;

/* IFX_RETURN_STATUS    - The following values can be returned as IFX_RETURN_STATUS. */
enum tagIFX_RETURN_STATUS
{
    IFX_SUCCESS = 0,                /* Call completed successfully                  */
    IFX_ERROR,                      /* General catch all error code                 */
    IFX_ERROR_MEMORY,               /* Memory Error                                 */
    IFX_ERROR_TASK_CREATION,        /* Task Creation Error                          */
    IFX_ERROR_EXCLUSIVITY,          /* Exclusivity Error                            */
    IFX_ERROR_RESTART,              /* Event requiring an Engine restart occurred   */
    IFX_ERROR_GRAPHICS,             /* A Graphics Error occurred                    */
    IFX_ERROR_ASYNC_BLOCKING,       /* An asynchronous module call is in progress   */
    IFX_ERROR_NAVIGATION_DISALLOWED,/* Navigation is not allowed                    */
    IFX_ERROR_ENGINE_NOT_READY,     /* Engine is not ready */

    IFX_ERROR_USER_DEFINED = 0x1000 /* Non-standard errors must be >= this value    */
};
typedef IFX_INT32 IFX_RETURN_STATUS;

/* IFX_LINK_MODE    - The following values can be returned as IFX_LINK_MODE.    */
enum tagIFX_LINK_MODE
{
    IFX_MODE_MENU,              /* The link is a menu plugin                                        */
    IFX_MODE_FUNCTION_SYNC,     /* The link is a synchronous task                                   */
    IFX_MODE_FUNCTION_ASYNC,    /* The link is an asynchronous task                                 */
    IFX_MODE_ELEMENT,           /* The link is a graphical element                                  */
    IFX_MODE_EVENT_HANDLER      /* The link is an event handler for processing key trigger on behalf of basic Engine elements */
};
typedef enum tagIFX_LINK_MODE IFX_LINK_MODE;

/* IFX_FIELD_TYPE   - The following values can be returned as IFX_FIELD_TYPE.   */
enum tagIFX_FIELD_TYPE
{
    IFX_FIELD,                  /* Requesting named field data                  */

    IFX_MENU_NAME,              /* Requesting menu name attribute               */
    IFX_MENU_SORT_FIELD,        /* Requesting menu sort field attribute         */
    IFX_MENU_SORT_TYPE,         /* Requesting menu sort type attribute          */
    IFX_MENU_SORT_DIRECTION,    /* Requesting menu sort direction attribute     */
    IFX_MENU_LOAD_ON_DEMAND,    /* Requesting load-on-demand attribute          */
    IFX_MENU_PLACEHOLDER,       /* Requesting placeholder information - deprecated */

    IFX_ITEM_LINK               /* Requesting item link attribute               */
};
typedef enum tagIFX_FIELD_TYPE IFX_FIELD_TYPE;

/* IFX_CALLBACK_CODE    - The following values can be returned as IFX_CALLBACK_CODE. */
enum tagIFX_CALLBACK_CODE
{
    IFX_QUERY_ACTIVE_ITEM,          /* Query state of UI menu                                       */
    IFX_REFRESH_MENU,               /* Cause an open menu to refresh                                */
    IFX_REFRESH_FIELD,              /* Notify that system data has changed                          */
    IFX_REFRESH_BUFFERED_ELEMENT,   /* Notify that the contents of a buffered element have changed  */
    IFX_RESOURCE_SEARCH,            /* Find the full path of a resource file                        */
    IFX_FULL_SCREEN_START,          /* Embedded element requests to go into 'full screen' mode      */
    IFX_FULL_SCREEN_STOP,           /* Embedded element requests to end 'full screen' mode          */
    IFX_COMMAND_LINK,               /* Execute the given link URI                                   */
    IFX_TRIGGER_KEY,                /* Simulate a specific key press                                */
    IFX_RESIZE_BUFFERED_ELEMENT_BUFFER, /* Resize a buffered element's buffer                   */
    IFX_CLOSE_MENU,                 /* Close an open menu                                           */
    IFX_SET_ACTVE_ITEM
};
typedef enum tagIFX_CALLBACK_CODE IFX_CALLBACK_CODE;

enum tagIFX_ELEMENT_MODE
{
    IFX_MODE_ELEMENT_INVALID = 0x00,             /* Invalid mode, used when not initialized */
    IFX_MODE_ELEMENT_OPENGL_TEXTURE  = 0x01,     /* An OpenGL Texture is created and managed externally.
                                                    Only supported if the Runtime engine is built in Open GL mode*/
    IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT = 0x02, /* The plugin supports buffered and also direct where
                                                   it is responsible for writing to the display directly.
                                                    Only supported if the Runtime engine is built in internal rendering mode*/
    IFX_MODE_ELEMENT_BUFFERED = 0x04,            /* The buffer for the plugin is owned and managed externally. */
    IFX_MODE_ELEMENT_BUFFERED_NORMAL = 0x08,     /* The buffer for the plugin is owned and managed by the engine (deprecated). */
};
typedef enum tagIFX_ELEMENT_MODE IFX_ELEMENT_MODE;

enum tagIFX_ELEMENT_TRANSLUCENCY
{
    IFX_TRANSLUCENCY_ELEMENT_OPAQUE = 0x00,      /* Element is opaque, this is the default value */
    IFX_TRANSLUCENCY_ELEMENT_NONOPAQUE = 0x01,   /* Element is non-opaque */
};
typedef enum tagIFX_ELEMENT_TRANSLUCENCY IFX_ELEMENT_TRANSLUCENCY;

typedef struct tIFX_BUFFER_SIZE{
    IFX_INT32 x;
    IFX_INT32 y;
} IFX_BUFFER_SIZE;

/* IFX_BUFFER_FORMAT    - The following values indicate buffered plug-in element drawing formats. */
enum tagIFX_BUFFER_FORMAT
{
    IFX_32BPP_RGBA,             /* Buffer is 32 bit-per-pixel, with each DWORD in the buffer
                                   containing the R, G, B, A components of the pixel in that order.
                                   if the Alpha channel is unused, the A component should be set to
                                   0xFF */
    IFX_24BPP_RGB,              /* Buffer is 24 bit-per-pixel, 3 bytes per pixel
                                   containing the R, G, B components of the pixel in that order. */
    IFX_16BPP_RGB565            /* Buffer is 16 bit-per-pixel, 2 bytes per pixel (unsigned short)
                                   containing the R(5), G(6), B(5) components of the pixel in that order. */

};
typedef enum tagIFX_BUFFER_FORMAT IFX_BUFFER_FORMAT;

typedef struct tagIFX_ELEMENT_PROPERTY {
    IFX_ELEMENT_MODE    mode;                   /* [OUT] Specify what mode the plugin supports */
    IFX_ELEMENT_TRANSLUCENCY translucency;      /* [OUT] Specify if the element is opaque or nonopaque */
	IFX_BOOL			hasPreMultipliedAlpha;	/* [OUT] Specify if the element has pre-multiplied alpha */
    IFX_INT32           requiredBufferWidth;    /* [IN]: extent hint in theme, [OUT] Requested plugin width */
    IFX_INT32           requiredBufferHeight;   /* [IN]: extent hint in theme, [OUT] Requested plugin height */
    IFX_BUFFER_FORMAT   requiredBufferFormat;   /* [IN]: Default color format, [OUT] Request a color format for the plugin */
} IFX_ELEMENT_PROPERTY;

/* IFX_ROTATE   - Controls the angle of rotation when blitting from canvas to screen */
typedef enum
{
    IFX_ROTATE_0 = 0,
    IFX_ROTATE_90,
    IFX_ROTATE_180,
    IFX_ROTATE_270
} IFX_ROTATE;

/* IFX_DISPLAY  - Struct passed when referring to a display. */
typedef struct tagIFX_DISPLAY
{
    void          *frameBuffer;             /* Pointer to the start of the */
                                            /* frame buffer */
    unsigned int width;                     /* Width of the buffer (pixels) */
    unsigned int height;                    /* Height of the buffer (rows) */
    unsigned int stride;                    /* Width of the buffer (bytes) */
    IFX_ROTATE  rotation;                   /* Our current rotation value */

    unsigned int bpp;                       /* Display color depth (bits per pixel) */
    unsigned int msaa_samples;              /* Number of samples for MSAA */
                                            /* 0 or 1 disables MSAA */

    unsigned int depth_buffer_size;         /* Depth size */
    unsigned int egl_swap_interval;         /* Egl Swap interval */

    int offsetX;                            /* Offset to the canvas X start */
    int offsetY;                            /* Offset to the canvas Y start */

    int renderFull;                         /* If this is <> 0 then the porting
                                               layer will transfer the full frame
                                               to display when
                                               IFXP_Render_Display_Start is
                                               called.
                                               If this is 0, then normal
                                               rendering will occur */

    void *internal;                         /* Pointer to implementation */
                                            /* specific data. */
} IFX_DISPLAY;

/* IFX_CANVAS   - Struct passed when referring to a canvas (scene buffer). */
typedef struct tagIFX_CANVAS
{
    void        *colorBuffer;               /* Pointer to the start of the */
                                            /* color buffer (Canvas) */
    unsigned int width;                     /* Width of the buffer (pixels) */
    unsigned int stride;                    /* Width of the buffer (bytes) */
    unsigned int height;                    /* Height of the buffer (rows) */

    void        *internal;                  /* Pointer to implementation */
                                            /* specific data. */
} IFX_CANVAS;

/* IFX_OPENGL_RENDER_CONTEXT  - Struct passed when creating an opengl texture plug-in element. */
struct tagIFX_OPENGL_RENDER_CONTEXT
{
    IFX_UINT32          texture;            /* [OUT] OpenGL textures ID for use by the plugin */

    IFX_INT32           tex_width;          /* [IN / OUT] The texture dimensions (used to calculate the */
    IFX_INT32           tex_height;         /* texture co-ordinates to map onto the plugin rectangle (defaults to whole texture) */

    IFX_INT32           top;                /* [IN / OUT] The location and size of the plugin bitmap data. */
    IFX_INT32           left;               /* Note that these can be used to manipulate which part of the data is drawn, */
    IFX_INT32           height;             /* for example to perform cropping / stretching of video. */
    IFX_INT32           width;              /* The values are used to calculate the U,V texture coordinates. */

                                            /* The EGL variables are only valid in the same thread context as the engine */
    void                *eglDisplay;        /* The current EGL Display */
    void                *eglSurface;        /* The current EGL Surface */
    void                *eglContext;        /* The current EGL Context */
    IFX_INT32           textureTarget;      /* [OUT] GLenum member indicating the target to which the texture will be bound.  If not 
                                               changed by the module during the call to changeElementMode, this will be GL_TEXTURE_2D */
    IFX_UINT32          secondaryTextures[3];/* [OUT] Array of Secondary OpenGL textures for use by custom effects (e.g. YUV conversion),
                                                Maximum of 3, unused textures should be set to 0. */
};
typedef struct tagIFX_OPENGL_RENDER_CONTEXT IFX_OPENGL_RENDER_CONTEXT;

/* IFX_BUFFERED_RENDER_CONTEXT  - Struct passed when creating a buffered plug-in element. */
struct tagIFX_BUFFERED_RENDER_CONTEXT
{
    void*               pBuffer;            /* [OUT] Pointer to the buffer data */
    IFX_INT32           height;             /* [IN] Width and height of the buffer */
    IFX_INT32           width;
    IFX_BUFFER_FORMAT   format;             /* [IN] Color format of the buffer */
};
typedef struct tagIFX_BUFFERED_RENDER_CONTEXT IFX_BUFFERED_RENDER_CONTEXT;

/* IFX_DIRECT_RENDER_CONTEXT    - Struct passed when creating a direct plug-in element. */
struct tagIFX_DIRECT_RENDER_CONTEXT
{
    IFX_DISPLAY         *pDisplay;
    IFX_INT32           iClipHeight;
    IFX_INT32           iClipWidth;
};
typedef struct tagIFX_DIRECT_RENDER_CONTEXT IFX_DIRECT_RENDER_CONTEXT;

/* IFX_API handle and size definitions */
typedef void*                           IFX_HUI;
typedef void*                           IFX_HIL;
typedef void*                           IFX_HELEMENT;
typedef void*                           IFX_HMENU;
typedef void*                           IFX_HMODULEID;
typedef void*                           IFX_HBUFFERID;
typedef void*                           IFX_HMENUID;
typedef void*                           IFX_HMODULE;
typedef void*                           IFX_HELEMENTID;
typedef void*                           IFX_HREQUEST;
typedef void*                           IFX_HBUFFER;
typedef void*                           IFX_HUTILITY;
typedef void*                           IFX_HEXCLUSIVITY;
#define IFX_MAX_LINK_TYPE_NAME_LENGTH   64
#define IFX_MAX_PATH_LENGTH             256
#define IFX_MAX_FIELD_NAME_LENGTH       256
#define IFX_MAX_CONFIG_STRING_LENGTH    256
#define IFX_MAX_FILE_NAME_LEN           256

/* IFX_POSITION - positional data */
typedef struct tIFX_Position
{
    IFX_INT32 left;
    IFX_INT32 top;
    IFX_INT32 width;
    IFX_INT32 height;
    IFX_INT32 opacity;
    IFX_INT32 color;
}IFX_POSITION;

/* IFX_TIME - time type */
typedef time_t IFX_TIME;

/* IFX_SORT_TYPE - menu sort type */
typedef enum {
    IFX_SORT_CASE_SENSITIVE,
    IFX_SORT_CASE_INSENSITIVE
} IFX_SORT_TYPE;

/* IFX_SORT_DIRECTION - menu sort direction */
typedef enum {
    IFX_SORT_ASCENDING,
    IFX_SORT_DESCENDING
} IFX_SORT_DIRECTION;

/* IFX_DIALOG_RETURN_STATUS - return values from a dialog*/
typedef enum {
    IFX_DIALOG_CANCEL,
    IFX_DIALOG_OK
} IFX_DIALOG_RETURN_STATUS;

/* IFX_KEY_CODE - predefined keycodes */
typedef enum {
    IFX_KEY_CODE_HASH = 0x0000FFFF,
    IFX_KEY_CODE_ASTERISK,
    IFX_KEY_CODE_UP,
    IFX_KEY_CODE_DOWN,
    IFX_KEY_CODE_LEFT,
    IFX_KEY_CODE_RIGHT,
    IFX_KEY_CODE_SELECT,
    IFX_KEY_CODE_BACK
} IFX_KEY_CODE;

/* Exclusivity status */
typedef enum
{
    IFX_EXCLUSIVITY_STATUS_GRANTED,
    IFX_EXCLUSIVITY_STATUS_DENIED,
    IFX_EXCLUSIVITY_STATUS_SUSPENDED,
    IFX_EXCLUSIVITY_STATUS_RESUMED,
    IFX_EXCLUSIVITY_STATUS_ERROR
} IFX_EXCLUSIVITY_STATUS;

/* Define used in IFXE_CONFIGURATION when specifying left and top. */
#define IFX_CENTER_DISPLAY ((IFX_INT32)0xFFFFFFFFUL)

/* File access types */
typedef enum _ifx_seek
{
    IFX_SEEK_SET,
    IFX_SEEK_CUR,
    IFX_SEEK_END
} IFX_SEEK;

/* Drive letter used to access packaged theme rom files */
#define IFX_ROM_FILE_DRIVE_LETTER   'Z'

/* Runtime states 
   If these are changed or updated, please
   check the 'shadow enum' in IfxFramework.java is kept
   synchronized */
typedef enum _ifxe_state 
{ 
	IFXE_STATE_UNINITIALIZED, 
	IFXE_STATE_INITIALIZED, 
	IFXE_STATE_ACTIVE, 
	IFXE_STATE_INACTIVE_SUSPENDED, 
	IFXE_STATE_INACTIVE_ASYNC_LINK, 
	IFXE_STATE_INACTIVE_RESTARTING
} IFXE_STATE;

#endif /* !def IFXUI_DEFS_H */

