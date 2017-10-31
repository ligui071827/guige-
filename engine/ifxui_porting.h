/************************************************************************
*
*               Copyright Mentor Graphics Corporation 2006
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
*        ifxui_porting.h
*
* COMPONENT
*
*        Inflexion UI
*
* DESCRIPTION
*
*        Inflexion UI Engine Porting API
*
* DATA STRUCTURES
*
*        IFXP_TEXT
*        IFXP_RECT
*
* DEPENDENCIES
*        ifxui_defs.h
*        ifxui_cfg.h
*        ifxui_porting_includes.h
*
************************************************************************/
#ifndef _IFXUI_PORTING_H_
#define _IFXUI_PORTING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "inflexionui/engine/ifxui_defs.h"
#include "ifxui_config.h"

#include "ifxui_porting_includes.h"

#if         !defined(IFX_CANVAS_MODE_8888) && \
            !defined(IFX_CANVAS_MODE_888) && \
            !defined(IFX_CANVAS_MODE_565) && \
            !defined(IFX_CANVAS_MODE_1555) && \
            !defined(IFX_CANVAS_MODE_444)
    #error Canvas Mode not defined
#endif

/* Checks to ensure only one canvas mode is selected */
#if         defined(IFX_CANVAS_MODE_8888)
    #if     defined(IFX_CANVAS_MODE_888) || \
            defined(IFX_CANVAS_MODE_565) || \
            defined(IFX_CANVAS_MODE_1555) || \
            defined(IFX_CANVAS_MODE_444)
        #error Only one canvas mode can be configured at a time.
    #endif

    #define IFXP_CANVAS_BPP                 32
    #define IFXP_CANVAS_BITS_RED            8
    #define IFXP_CANVAS_BITS_GREEN          8
    #define IFXP_CANVAS_BITS_BLUE           8
    #define IFXP_CANVAS_BITS_ALPHA          8
#endif

#if         defined(IFX_CANVAS_MODE_888)
    #if     defined(IFX_CANVAS_MODE_8888) || \
            defined(IFX_CANVAS_MODE_565) || \
            defined(IFX_CANVAS_MODE_1555) || \
            defined(IFX_CANVAS_MODE_444)
        #error Only one canvas mode can be configured at a time.
    #endif

    #define IFXP_CANVAS_BPP                 24
    #define IFXP_CANVAS_BITS_RED            8
    #define IFXP_CANVAS_BITS_GREEN          8
    #define IFXP_CANVAS_BITS_BLUE           8
    #define IFXP_CANVAS_BITS_ALPHA          0
#endif

#if         defined(IFX_CANVAS_MODE_565)
    #if     defined(IFX_CANVAS_MODE_8888) || \
            defined(IFX_CANVAS_MODE_888) || \
            defined(IFX_CANVAS_MODE_1555) || \
            defined(IFX_CANVAS_MODE_444)
        #error Only one canvas mode can be configured at a time.
    #endif

    #define IFXP_CANVAS_BPP                 16
    #define IFXP_CANVAS_BITS_RED            5
    #define IFXP_CANVAS_BITS_GREEN          6
    #define IFXP_CANVAS_BITS_BLUE           5
    #define IFXP_CANVAS_BITS_ALPHA          0
#endif

#if         defined(IFX_CANVAS_MODE_1555)
    #if     defined(IFX_CANVAS_MODE_8888) || \
            defined(IFX_CANVAS_MODE_888) || \
            defined(IFX_CANVAS_MODE_565) || \
            defined(IFX_CANVAS_MODE_444)
        #error Only one canvas mode can be configured at a time.
    #endif

    #define IFXP_CANVAS_BPP                 16
    #define IFXP_CANVAS_BITS_RED            5
    #define IFXP_CANVAS_BITS_GREEN          5
    #define IFXP_CANVAS_BITS_BLUE           5
    #define IFXP_CANVAS_BITS_ALPHA          1
#endif

#if         defined(IFX_CANVAS_MODE_444)

#if         defined(IFX_CANVAS_MODE_8888) || \
            defined(IFX_CANVAS_MODE_888) || \
            defined(IFX_CANVAS_MODE_565) || \
            defined(IFX_CANVAS_MODE_1555)
            #error Only one canvas mode can be configured at a time.

#endif

    #define IFXP_CANVAS_BPP                 16
    #define IFXP_CANVAS_BITS_RED            4
    #define IFXP_CANVAS_BITS_GREEN          4
    #define IFXP_CANVAS_BITS_BLUE           4
    #define IFXP_CANVAS_BITS_ALPHA          4
#endif

#define IFXP_ERROR_STRING_LEN               (256)

#define IFXP_ERROR_NOTE_SHOW_TIME           5000

#define IFXP_MUTEX_INFINITE                 0xFFFFFFFF

#if         defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)

/* The following events are sent to the porting layer to detect the
   specified events occurring within the Engine */
    #define IFXP_BENCHMARKING_FRAME_UPDATE_STARTED    0x0001
    #define IFXP_BENCHMARKING_FRAME_UPDATE_COMPLETE   0x0002
    #define IFXP_BENCHMARKING_TRANSITION_STARTED      0x0004
    #define IFXP_BENCHMARKING_TRANSITION_COMPLETE     0x0008
    #define IFXP_BENCHMARKING_OPEN_PAGE_STARTED       0x0010
    #define IFXP_BENCHMARKING_OPEN_PAGE_COMPLETE      0x0020
    #define IFXP_BENCHMARKING_GPU_MEMORY_CHANGED      0x0040    /* The data parameter contains the delta (+/-)*/
    #define IFXP_BENCHMARKING_GPU_MEMORY_RESET        0x0080    /* The GPU memory has been reset (new context) */
#endif      /* defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE) */

/* Porting layer specific errors */
#define IFXP_MUTEX_TIMEOUT      IFX_ERROR_USER_DEFINED

/* Determines the font face when creating a font canvas */
typedef enum
{
    IFXP_FONT_FACE_NORMAL = 0,
    IFXP_FONT_FACE_BOLD,
    IFXP_FONT_FACE_ITALIC,
    IFXP_FONT_FACE_BOLDITALIC
} IFXP_FONT_FACE;

/* Structures and Types */

typedef struct _ifxp_text
{
    void          *alphaBuffer;             /* Special 8-bit gray scale canvas */
    IFX_UINT32     width;                   /* Width of the buffer (pixels) */
    IFX_UINT32     height;                  /* Height of the buffer (rows) */
    void          *internal;                /* Pointer to implementation */
                                            /* specific data. */
} IFXP_TEXT;

typedef struct _ifxp_rect
{
    IFX_INT32 x1;                           /* Top Left x coordinate */
    IFX_INT32 y1;                           /* Top Left y coordinate */
    IFX_INT32 x2;                           /* Bottom right x coordinate */
    IFX_INT32 y2;                           /* Bottom right y coordinate */
} IFXP_RECT;

typedef enum _ifxp_canvas
{
    IFXP_FOREGROUND = 0,                    /* This is a foreground canvas */
    IFXP_BACKGROUND                         /* This is a background canvas */
} IFXP_FGBG;

typedef void* IFXP_FONT;

/* These typedefs are required for platform files */
#if         defined (IFX_USE_PLATFORM_FILES)

typedef void* IFXP_FILE;

typedef enum _ifx_info
{
    IFXP_INFO_INVALID,
    IFXP_INFO_DIRECTORY,
    IFXP_INFO_FILE
} IFXP_INFO;

typedef struct _ifxp_search
{
    IFXP_INFO info;                         /* The file information */
    char fileName[IFX_MAX_FILE_NAME_LEN];   /* The filename */
#if defined(IFX_WIN_PLAYER)
    char dirPrefix[IFX_MAX_FILE_NAME_LEN];  /* Directory name*/
#endif
    void* internal;                         /* Implementation specific
                                               pointer */
} IFXP_SEARCH;

#endif      /* defined (IFX_USE_PLATFORM_FILES) */

typedef void* IFXP_TIMER;

typedef void* IFXP_MUTEX;

typedef enum _ifxp_mem
{
    IFXP_MEM_ENGINE,
    IFXP_MEM_EXTERNAL
} IFXP_MEM_USE;

/* Command request enum */
typedef enum _ifxp_cmd
{
    IFXP_CMD_EXIT,
    IFXP_CMD_SUSPEND
} IFXP_CMD;


IFX_RETURN_STATUS   IFXP_Initialize                 (void);
IFX_RETURN_STATUS   IFXP_Shutdown                   (void);
IFX_RETURN_STATUS   IFXP_Runtime_State_Changed      (IFXE_STATE state);
IFX_RETURN_STATUS   IFXP_Display_Set_Mode           (IFX_INT32 left,
                                                     IFX_INT32 top,
                                                     IFX_INT32 width,
                                                     IFX_INT32 height,
                                                     const char *display_mode);
IFX_RETURN_STATUS   IFXP_Display_Get_Info           (IFX_DISPLAY **display);
IFX_RETURN_STATUS   IFXP_Canvas_Create              (IFX_CANVAS **canvas,
                                                     IFX_UINT32 canvas_width,
                                                     IFX_UINT32 canvas_height,
                                                     IFXP_FGBG fgbg);
IFX_RETURN_STATUS   IFXP_Canvas_Destroy             (IFX_CANVAS *canvas);
IFX_RETURN_STATUS   IFXP_Canvas_Draw_Rect_Fill      (IFX_CANVAS *dst_canvas,
                                                     IFXP_RECT *dst_rect,
                                                     IFX_INT32 color);
IFX_RETURN_STATUS   IFXP_Canvas_Draw_Rect_Debug     (IFX_CANVAS *dst_canvas,
                                                     IFXP_RECT *dst_rect,
                                                     IFX_INT32 color);

IFX_RETURN_STATUS   IFXP_Render_Start               (void);
IFX_RETURN_STATUS   IFXP_Render_Background_Start    (IFX_CANVAS *dst_canvas);
IFX_RETURN_STATUS   IFXP_Render_Background          (IFX_CANVAS *dst_canvas,
                                                     IFX_INT32 dstTop,
                                                     IFX_INT32 dstLeft,
                                                     IFX_CANVAS *src_canvas,
                                                     IFXP_RECT *src_rect);
IFX_RETURN_STATUS   IFXP_Render_Background_End      (IFX_CANVAS *dst_canvas);
IFX_RETURN_STATUS   IFXP_Render_Foreground_Start    (IFX_CANVAS *canvas);
IFX_RETURN_STATUS   IFXP_Render_Foreground_End      (IFX_CANVAS *canvas);
IFX_RETURN_STATUS   IFXP_Render_Display_Start       (IFX_CANVAS *canvas);
IFX_RETURN_STATUS   IFXP_Render_Display             (IFX_INT32 dstTop,
                                                     IFX_INT32 dstLeft,
                                                     IFX_CANVAS *src_canvas,
                                                     IFXP_RECT *src_rect);
IFX_RETURN_STATUS   IFXP_Render_Display_End         (IFX_CANVAS *canvas);
IFX_RETURN_STATUS   IFXP_Render_End                 (void);

#if          defined (IFX_USE_NATIVE_FONTS)

IFX_RETURN_STATUS   IFXP_Text_Open_Font             (IFXP_FONT *font_handle,
                                                     IFXP_FONT_FACE face,
                                                     const IFX_WCHAR *font_name);
IFX_RETURN_STATUS   IFXP_Text_Close_Font            (IFXP_FONT font_handle);
IFX_RETURN_STATUS   IFXP_Text_Get_Width             (IFXP_FONT font_handle,
                                                     IFX_WCHAR *text,
                                                     IFX_UINT32 in_length,
                                                     IFX_UINT32 *out_length,
                                                     IFX_INT32 *rtlOut);
IFX_RETURN_STATUS   IFXP_Text_Get_Char_Height       (IFXP_FONT font_handle,
                                                     IFX_UINT32* height);
IFX_RETURN_STATUS   IFXP_Text_Canvas_Create         (IFXP_TEXT **text_canvas,
                                                     IFXP_FONT font_handle,
                                                     IFX_WCHAR *text,
                                                     IFX_INT32 *rtlOut);
IFX_RETURN_STATUS   IFXP_Text_Canvas_Destroy        (IFXP_TEXT *canvas);

#endif      /* defined (IFX_USE_NATIVE_FONTS) */

IFX_RETURN_STATUS   IFXP_Display_Error_Note         (const IFX_WCHAR *error_text);

#if         defined (IFX_RENDER_DIRECT_OPENGL) || \
            defined (IFX_RENDER_DIRECT_OPENGL_20) || \
            defined (IFX_RENDER_BUFFERED_OPENGL)

#if         defined(IFX_USE_EGL)

NativeDisplayType   IFXP_Egl_Get_Default_Display    (void);
NativeWindowType    IFXP_Egl_Get_Default_Window     (void);

#endif      /* defined(IFX_USE_EGL) */

#endif /* defined (IFX_RENDER_DIRECT_OPENGL) ||
            defined (IFX_RENDER_BUFFERED_OPENGL) */

#if          defined (IFX_SERIALIZATION)

IFX_RETURN_STATUS   IFXP_StateRead                  (void *addr,IFX_UINT32 size, IFX_UINT32 *bytes_read);
IFX_RETURN_STATUS   IFXP_StateWrite                 (char *buffer,IFX_UINT32 size, int *bytes_wrote);
IFX_RETURN_STATUS   IFXP_StateOpen                  (IFX_UINT32 *numberOfBlocks,IFX_UINT32 *offsetMapSize);
IFX_RETURN_STATUS   IFXP_StateCreate                ();
IFX_RETURN_STATUS   IFXP_StateClose                 ();

#endif      /* defined (IFX_SERIALIZATION) */

#if defined(IFX_WIN_PLAYER)
    // Player Message  identifier = Base + Msg (32768 + 1000)
    #define         IFX_SPLASH_OFF                  0x000083E8
    #define         IFX_DO_INIT                     0x000083E9
    #define         IFX_SPLASH_ON                   0x000083EA
    void            IFXP_IfxPlayer_Signal(unsigned int signal);
#endif

#if         defined (IFX_USE_PLATFORM_FILES)

IFX_RETURN_STATUS   IFXP_File_Create                (IFXP_FILE *handle, const char *file_name);
IFX_RETURN_STATUS   IFXP_File_Open                  (IFXP_FILE *handle, const char *file_name);
IFX_RETURN_STATUS   IFXP_File_Read                  (IFXP_FILE handle, void *addr, IFX_UINT32 size, IFX_UINT32 *bytes_read);
IFX_RETURN_STATUS   IFXP_File_Write                 (IFXP_FILE handle, void *addr, IFX_UINT32 size, IFX_UINT32 *bytes_written);
IFX_RETURN_STATUS   IFXP_File_Seek                  (IFXP_FILE handle, IFX_INT32 offset, IFX_SEEK seek_mode);
IFX_RETURN_STATUS   IFXP_File_Size                  (IFXP_FILE handle, IFX_UINT32 *file_size);
IFX_RETURN_STATUS   IFXP_File_Close                 (IFXP_FILE handle);
IFX_RETURN_STATUS   IFXP_File_Find_First            (IFXP_FILE *handle, IFXP_SEARCH *info_ptr, char *search_text);
IFX_RETURN_STATUS   IFXP_File_Find_Next             (IFXP_FILE handle, IFXP_SEARCH *info_ptr);
IFX_RETURN_STATUS   IFXP_File_Find_Close            (IFXP_FILE handle, IFXP_SEARCH *info_ptr);

IFX_RETURN_STATUS   IFXP_Dir_Create                 (const char *new_dir);

#if          defined (IFX_GENERATE_SCRIPTS) || defined(IFX_USE_SCRIPTS)

IFX_RETURN_STATUS   IFXP_Get_Relative_Cwd           (char *dir_path);
IFX_RETURN_STATUS   IFXP_Set_Relative_Cwd           (const char *dir_path);

#endif      /* defined (IFX_GENERATE_SCRIPTS) || defined(IFX_USE_SCRIPTS) */
#endif

IFX_RETURN_STATUS   IFXP_Timer_Schedule             (IFX_UINT32 time_upper, IFX_UINT32 time_lower);
IFX_RETURN_STATUS   IFXP_Timer_Get_Current_Time     (IFX_UINT32* time_upper, IFX_UINT32* time_lower);
IFX_RETURN_STATUS   IFXP_Mem_Allocate               (IFXP_MEM_USE usage, IFX_UINT32 size, void **ptr);
IFX_RETURN_STATUS   IFXP_Mem_Deallocate             (IFXP_MEM_USE usage, void *ptr);

IFX_RETURN_STATUS   IFXP_Mutex_Create               (IFXP_MUTEX *mutex, const IFX_WCHAR *name);
IFX_RETURN_STATUS   IFXP_Mutex_Destroy              (IFXP_MUTEX mutex);
IFX_RETURN_STATUS   IFXP_Mutex_Lock                 (IFXP_MUTEX mutex, IFX_UINT32 timeout);
IFX_RETURN_STATUS   IFXP_Mutex_Unlock               (IFXP_MUTEX mutex);

#if         defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)

IFX_RETURN_STATUS   IFXP_Benchmarking_Signal        (IFX_INT32 signal, IFX_INT32 data);

#endif      /* defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE) */

IFX_RETURN_STATUS   IFXP_Error_Print                (const IFX_WCHAR *error_string);

IFX_RETURN_STATUS   IFXP_Command                    (IFXP_CMD command);

#if defined(IFX_RENDER_DIRECT_OPENGL_20) || defined(IFX_RENDER_DIRECT_OPENGL)

IFX_RETURN_STATUS   IFXP_Check_GL_Extension         (const char *ext_name);

#endif

/* Image Unpacking Porting Layer Interface for Inflexion Engine */

/* Type of image data whether it is uncompressed or compressed */
typedef enum
{
    IFX_IMAGE_TYPE_RGBA_DATA = 0,               /* uncompressed RGBA data */
    IFX_IMAGE_TYPE_RGB_DATA,                    /* uncompressed RGB data  */
    IFX_IMAGE_TYPE_OPENGL_COMPRESSED_TEXTURE    /* OGL compressed texture in vendor format */

} IFX_IMAGE_TYPE;

/* Type of image translucency whether it is opaque or nonopaque */
typedef enum
{
    IFX_TRANSLUCENCY_IMAGE_OPAQUE = 0,              /* image is opaque */
    IFX_TRANSLUCENCY_IMAGE_NONOPAQUE                /* image is non-opaque */
} IFX_IMAGE_TRANSLUCENCY;

/* Information about the custom image */
typedef struct _ifxp_image_info
{
    IFX_IMAGE_TYPE  type;               /* Inflexion type of packed image */
    IFX_IMAGE_TRANSLUCENCY translucency; /* Image translucency information */
    IFX_UINT32      width;              /* Width of the image */
    IFX_UINT32      height;             /* Height of the image */

    IFX_UINT32      length;             /* Length in bytes of the image data */

    void*           internal;           /* Pointer for implementation specific session data */

    /* These only apply to OGL compressed textures */

    IFX_UINT32      texture_format;     /* OpenGL Enum for compressed texture */
    IFX_UINT32      first_mipmap_level; /* First available mip-map level */
    IFX_UINT32      last_mipmap_level;  /* Last available mip-map level */

} IFX_IMAGE_INFO;

/* The un-packer populates the info structure so that the engine may set up any required
internal objects in preparation to receive the data. */
IFX_RETURN_STATUS IFXP_Image_Open (IFX_IMAGE_INFO *info,const char *image_file);

/* The engine will call this function for each mipmap level (once for jpgs / pngs) */
IFX_RETURN_STATUS IFXP_Image_Get_Data (IFX_IMAGE_INFO *image_info,
                                       IFX_UINT32 level,
                                       IFX_UINT32 *length,
                                       void **image_data);

/* The engine will call this function after it has finished with the data required in
the call to IFXP_Image_Get_Data */
IFX_RETURN_STATUS IFXP_Image_Release_Data (IFX_IMAGE_INFO *info, void *data);

/* The engine invokes this API all data has been passed onto OpenGL pipeline so that data is freed */
IFX_RETURN_STATUS IFXP_Image_Close (IFX_IMAGE_INFO *info);

/* Engine invoke this api to verify whether extension is supported or not */
IFX_RETURN_STATUS IFXP_Image_Check_Extension (const char *image_file);

#ifdef __cplusplus
}
#endif

#endif /* _IFXUI_PORTING_H_ */
