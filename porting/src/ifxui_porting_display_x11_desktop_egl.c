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
*        ifxui_porting_display_x11_egl.c
*
* COMPONENT
*
*        Inflexion UI Porting Layer.
*
* DESCRIPTION
*
*       EGL Implementation for Linux X11
*
* DATA STRUCTURES
*
* FUNCTIONS
*
* DEPENDENCIES
*
************************************************************************/
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <GL/glx.h>
#include    "egl.h"
#include    "inflexionui/porting/inc/ifxui_porting_display_x11.h"

typedef struct _x11_config
{
    EGLint bufferSize;
    EGLint alphaSize;
    EGLint blueSize;
    EGLint greenSize;
    EGLint redSize;
    EGLint depthSize;
    EGLint stencilSize;
    EGLint level;
    EGLint surfaceType;
    EGLint transparentType;
    EGLint transparentBlueValue;
    EGLint transparentGreenValue;
    EGLint transparentRedValue;
    EGLint renderableType;
    XVisualInfo *visinfo;
} X11_CONFIG;

typedef struct _x11_surface
{
    Window win;
    XSetWindowAttributes attr;
    EGLint surfaceType;
    X11_CONFIG *x11_config;
} X11_SURFACE;

static EGLint lastError = EGL_SUCCESS;
static EGLDisplay currentDisplay = EGL_NO_DISPLAY;
static EGLSurface currentReadSurface = EGL_NO_SURFACE;
static EGLSurface currentDrawSurface = EGL_NO_SURFACE;
static EGLContext currentContext = EGL_NO_CONTEXT;
static EGLBoolean initialized = EGL_FALSE;

#define NUM_CONFIGS 16

static X11_CONFIG x11_configs[NUM_CONFIGS] = { \

                  {0, 0, 8, 8, 8, 24, 0, 0, EGL_WINDOW_BIT,  0, 0, 0, 0, EGL_OPENGL_ES_BIT },
                  {0, 0, 5, 6, 5, 24, 0, 0, EGL_WINDOW_BIT,  0, 0, 0, 0, EGL_OPENGL_ES_BIT },
                  {0, 0, 8, 8, 8, 24, 0, 0, EGL_PBUFFER_BIT, 0, 0, 0, 0, EGL_OPENGL_ES_BIT },
                  {0, 0, 5, 6, 5, 24, 0, 0, EGL_PBUFFER_BIT, 0, 0, 0, 0, EGL_OPENGL_ES_BIT },
                  {0, 0, 8, 8, 8, 24, 0, 0, EGL_WINDOW_BIT,  0, 0, 0, 0, EGL_OPENGL_ES2_BIT},
                  {0, 0, 5, 6, 5, 24, 0, 0, EGL_WINDOW_BIT,  0, 0, 0, 0, EGL_OPENGL_ES2_BIT},
                  {0, 0, 8, 8, 8, 24, 0, 0, EGL_PBUFFER_BIT, 0, 0, 0, 0, EGL_OPENGL_ES2_BIT},
                  {0, 0, 5, 6, 5, 24, 0, 0, EGL_PBUFFER_BIT, 0, 0, 0, 0, EGL_OPENGL_ES2_BIT},

                  {0, 0, 8, 8, 8, 16, 0, 0, EGL_WINDOW_BIT,  0, 0, 0, 0, EGL_OPENGL_ES_BIT },
                  {0, 0, 5, 6, 5, 16, 0, 0, EGL_WINDOW_BIT,  0, 0, 0, 0, EGL_OPENGL_ES_BIT },
                  {0, 0, 8, 8, 8, 16, 0, 0, EGL_PBUFFER_BIT, 0, 0, 0, 0, EGL_OPENGL_ES_BIT },
                  {0, 0, 5, 6, 5, 16, 0, 0, EGL_PBUFFER_BIT, 0, 0, 0, 0, EGL_OPENGL_ES_BIT },
                  {0, 0, 8, 8, 8, 16, 0, 0, EGL_WINDOW_BIT,  0, 0, 0, 0, EGL_OPENGL_ES2_BIT},
                  {0, 0, 5, 6, 5, 16, 0, 0, EGL_WINDOW_BIT,  0, 0, 0, 0, EGL_OPENGL_ES2_BIT},
                  {0, 0, 8, 8, 8, 16, 0, 0, EGL_PBUFFER_BIT, 0, 0, 0, 0, EGL_OPENGL_ES2_BIT},
                  {0, 0, 5, 6, 5, 16, 0, 0, EGL_PBUFFER_BIT, 0, 0, 0, 0, EGL_OPENGL_ES2_BIT}
                  };

EGLint eglGetError()
{
    EGLint err = lastError;
    lastError = EGL_SUCCESS;
    return err;
}

EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id)
{
    Display *dpy;

    /* Use the opened display */
    dpy = (Display *)display_id;
    currentDisplay = (EGLDisplay)dpy;

    /* Check that this succeeded */
    if (dpy == NULL)
        lastError = EGL_BAD_DISPLAY;

    return currentDisplay;
}

EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
    if (initialized == EGL_FALSE)
    {
        /* Set any globals that need initializing */
        lastError = EGL_SUCCESS;
        currentDisplay = EGL_NO_DISPLAY;
        currentReadSurface = EGL_NO_SURFACE;
        currentDrawSurface = EGL_NO_SURFACE;
        currentContext = EGL_NO_CONTEXT;

        initialized = EGL_TRUE;
    }

    if (major)
        *major = 1;
    if (minor)
        *minor = 2;

    return EGL_TRUE;
}

EGLBoolean eglTerminate(EGLDisplay dpy)
{
    EGLBoolean status = EGL_FALSE;

    if (initialized == EGL_TRUE)
    {
        /* Delete any resources */
        if (currentDisplay)
        {
            currentDisplay = EGL_NO_DISPLAY;
        }

        initialized = EGL_FALSE;
        status = EGL_TRUE;
    }
    else
    {
        lastError = EGL_NOT_INITIALIZED;
    }

    return status;
}

const char *eglQueryString(EGLDisplay dpy, EGLint name)
{
    lastError = EGL_BAD_CONFIG;
    return NULL;
}

EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs,
             EGLint config_size, EGLint *num_config)
{
    int i;

    if (configs == 0)
    {
        /* Return only the number of configurations available */
        if (num_config)
        {
            *num_config = NUM_CONFIGS;
        }
        return EGL_TRUE;
    }

    /* Only return at most NUM_CONFIGS */
    if (config_size > NUM_CONFIGS)
    {
        config_size = NUM_CONFIGS;
    }

    /* Populate the caller's array with our configs */
    for (i=0; i<config_size; i++)
    {
        configs[i] = &x11_configs[i];
    }

    /* Let the caller know how many configs there are */
    if (num_config)
    {
        *num_config = config_size;
    }

    return EGL_TRUE;
}

EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list,
               EGLConfig *configs, EGLint config_size,
               EGLint *num_config)
{
    /* Iterate through the attribute list, to find all matching configs */
    EGLint *attrib_ptr;
    EGLint attrib;
    EGLint value;
    X11_CONFIG *x11_config;
    int i;
    EGLBoolean config_found = EGL_FALSE;

    if ( (attrib_list == NULL) || (configs == NULL) || (num_config == NULL) )
    {
        lastError = EGL_BAD_PARAMETER;
        return EGL_FALSE;
    }

    /* Default to no configs found */
    *num_config = 0;

    /* Iterate through the available configs */
    for (i=0; (i<config_size) && (i< NUM_CONFIGS); i++)
    {
        EGLint error = EGL_FALSE;
        attrib_ptr = (EGLint*)attrib_list;

        x11_config = &x11_configs[i];

        while ( (error == EGL_FALSE) && (*attrib_ptr != EGL_NONE) )
        {
            attrib = *attrib_ptr++;
            value  = *attrib_ptr++;

            if (value == EGL_DONT_CARE)
                continue;

            switch (attrib)
            {
                case EGL_BUFFER_SIZE:
                    if (x11_config->bufferSize < value)
                        error = EGL_TRUE;
                break;
                case EGL_ALPHA_SIZE:
                    if (x11_config->alphaSize < value)
                        error = EGL_TRUE;
                break;
                case EGL_BLUE_SIZE:
                    if (x11_config->blueSize < value)
                        error = EGL_TRUE;
                break;
                case EGL_GREEN_SIZE:
                    if (x11_config->greenSize < value)
                        error = EGL_TRUE;
                break;
                case EGL_RED_SIZE:
                    if (x11_config->redSize < value)
                        error = EGL_TRUE;
                break;
                case EGL_DEPTH_SIZE:
                    if (x11_config->depthSize < value)
                        error = EGL_TRUE;
                break;
                case EGL_STENCIL_SIZE:
                    if (x11_config->stencilSize < value)
                        error = EGL_TRUE;
                break;
                case EGL_SURFACE_TYPE:
                    if (x11_config->surfaceType != value)
                        error = EGL_TRUE;
                break;
                case EGL_RENDERABLE_TYPE:
                    if (x11_config->renderableType != value)
                        error = EGL_TRUE;
                break;
                default:
                break;
            }
        } /* Attrib while loop */

        if (error == EGL_FALSE)
        {
            /* Matching config, add it to the list. */
            (*num_config)++;

            if ((config_size >= *num_config) && configs)
            {
                /* Add the config to the list */
                *configs++ = (EGLConfig)x11_config;
                config_found = EGL_TRUE;
            }
        }
    } /* Config loop */

    return config_found;
}

EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config,
                  EGLint attribute, EGLint *value)
{
    int glxVal = 0;
    X11_CONFIG *x11_config = (X11_CONFIG*)config;

    switch (attribute)
    {
        case EGL_BUFFER_SIZE:
            if (x11_config->visinfo)
                glXGetConfig(dpy, x11_config->visinfo, GLX_BUFFER_SIZE, &glxVal);
            else
                glxVal = x11_config->bufferSize;
        break;

        case EGL_RED_SIZE:
            if (x11_config->visinfo)
                glXGetConfig(dpy, x11_config->visinfo, GLX_RED_SIZE, &glxVal);
            else
                glxVal = x11_config->redSize;
        break;

        case EGL_GREEN_SIZE:
            if (x11_config->visinfo)
                glXGetConfig(dpy, x11_config->visinfo, GLX_GREEN_SIZE, &glxVal);
            else
                glxVal = x11_config->greenSize;
        break;

        case EGL_BLUE_SIZE:
            if (x11_config->visinfo)
                glXGetConfig(dpy, x11_config->visinfo, GLX_BLUE_SIZE, &glxVal);
            else
                glxVal = x11_config->blueSize;
        break;

        case EGL_ALPHA_SIZE:
            if (x11_config->visinfo)
                glXGetConfig(dpy, x11_config->visinfo, GLX_ALPHA_SIZE, &glxVal);
            else
                glxVal = x11_config->alphaSize;
        break;

        case EGL_CONFIG_CAVEAT:
        break;

        case EGL_CONFIG_ID:
        break;

        case EGL_DEPTH_SIZE:
            if (x11_config->visinfo)
                glXGetConfig(dpy, x11_config->visinfo, GLX_DEPTH_SIZE, &glxVal);
            else
                glxVal = x11_config->depthSize;
        break;

        case EGL_SAMPLES:
            if (x11_config->visinfo)
                glXGetConfig(dpy, x11_config->visinfo, GLX_SAMPLES, &glxVal);
            else
                glxVal = 0;
        break;

        case EGL_LEVEL:
            glxVal = x11_config->level;
        break;

        case EGL_STENCIL_SIZE:
            if (x11_config->visinfo)
                glXGetConfig(dpy, x11_config->visinfo, GLX_STENCIL_SIZE, &glxVal);
            else
                glxVal = x11_config->stencilSize;
        break;

        case EGL_SURFACE_TYPE:
            glxVal = x11_config->surfaceType;
        break;

        case EGL_TRANSPARENT_TYPE:
            glxVal = x11_config->transparentType;
        break;

        case EGL_TRANSPARENT_RED_VALUE:
            glxVal = x11_config->transparentRedValue;
        break;

        case EGL_TRANSPARENT_GREEN_VALUE:
            glxVal = x11_config->transparentGreenValue;
        break;

        case EGL_TRANSPARENT_BLUE_VALUE:
            glxVal = x11_config->transparentBlueValue;
        break;

        case EGL_MAX_PBUFFER_WIDTH:
        case EGL_MAX_PBUFFER_HEIGHT:
        case EGL_MAX_PBUFFER_PIXELS:
        case EGL_NATIVE_RENDERABLE:
        case EGL_NATIVE_VISUAL_ID:
        case EGL_NATIVE_VISUAL_TYPE:
        case EGL_PRESERVED_RESOURCES:
        case EGL_SAMPLE_BUFFERS:
        default:
        break;
    }

    *value = glxVal;

    return EGL_TRUE;
}

EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,
                  NativeWindowType win,
                  const EGLint *attrib_list)
{

    int visAttributes[] = {
        GLX_RGBA,
        GLX_RED_SIZE, 1,
        GLX_GREEN_SIZE, 1,
        GLX_BLUE_SIZE, 1,
        GLX_DEPTH_SIZE, 1,
        GLX_SAMPLE_BUFFERS, 0,
        GLX_SAMPLES, 0,
        GLX_DOUBLEBUFFER,
        None
    };
    unsigned long attrMask;
    Window root;
    X11_SURFACE *x11_surface;
    X11_CONFIG *x11_config = (X11_CONFIG*)config;

    if (x11_config == NULL)
    {
        lastError = EGL_BAD_PARAMETER;
        return EGL_NO_SURFACE;
    }

    visAttributes[2] = x11_config->redSize;
    visAttributes[4] = x11_config->greenSize;
    visAttributes[6] = x11_config->blueSize;
    visAttributes[8] = x11_config->depthSize;

    x11_surface = malloc(sizeof(X11_SURFACE));
    if (x11_surface == NULL)
    {
        lastError = EGL_BAD_ALLOC;
        return EGL_NO_SURFACE;
    }

    /* This is a window */
    x11_surface->surfaceType = EGL_WINDOW_BIT;

    /* the root window */
    root = (Window) win;

    /* Choose GLX visual / pixel format */

    /* Attempt to get a multi-sampled surface */
    visAttributes[10] = 1;
    visAttributes[12] = 4;
    x11_config->visinfo = glXChooseVisual((Display*)dpy, 0, visAttributes);

    if (!x11_config->visinfo)
    {
        /* Fall back to non-multisampled */
        visAttributes[10] = 0;
        visAttributes[12] = 0;
        printf("Unabled to create a multisampled EGL surface, dropping back to no MSAA.\n");
        x11_config->visinfo = glXChooseVisual((Display*)dpy, 0, visAttributes);
    }

    if (!x11_config->visinfo)
    {
        printf("Error: couldn't get an RGB, Double-buffered visual\n");
        lastError = EGL_BAD_ALLOC;
        return EGL_NO_SURFACE;
    }

    /* Use the created window */
    x11_surface->attr.background_pixel = 0;
    x11_surface->attr.border_pixel = 0;
    x11_surface->attr.colormap = XCreateColormap((Display*)dpy, root, x11_config->visinfo->visual, AllocNone);
    attrMask = CWBackPixel | CWBorderPixel | CWColormap;
    x11_surface->win = (Window) win;
    x11_surface->x11_config = x11_config;
    return (EGLSurface)x11_surface;
}

EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config,
                   const EGLint *attrib_list)
{
    int visAttributes[] = {
        GLX_RGBA,
        GLX_RED_SIZE, 1,
        GLX_GREEN_SIZE, 1,
        GLX_BLUE_SIZE, 1,
        GLX_DEPTH_SIZE, 1,
        GLX_WIDTH, 32,
        GLX_HEIGHT, 32,
        GLX_PBUFFER_BIT,
        None
    };
    Window root;
    X11_SURFACE *x11_surface;
    X11_CONFIG *x11_config = (X11_CONFIG*)config;
    EGLint *attrib_ptr;

    if (x11_config == NULL)
    {
        lastError = EGL_BAD_PARAMETER;
        return EGL_NO_SURFACE;
    }

    visAttributes[2] = x11_config->redSize;
    visAttributes[4] = x11_config->greenSize;
    visAttributes[6] = x11_config->blueSize;
    visAttributes[8] = x11_config->depthSize;

    if (attrib_list)
    {
        /* Loop through the list setting up the visAttributes */
        while (*attrib_ptr != EGL_NONE)
        {
            EGLint attrib = *attrib_ptr++;
            EGLint value  = *attrib_ptr++;

            switch (attrib)
            {
                case EGL_WIDTH:
                    visAttributes[10] = value;
                break;
                case EGL_HEIGHT:
                    visAttributes[12] = value;
                break;
                default:
                break;
            }
        }
    }

    x11_surface = malloc(sizeof(X11_SURFACE));
    if (x11_surface == NULL)
    {
        lastError = EGL_BAD_ALLOC;
        return EGL_NO_SURFACE;
    }

    /* This is a window */
    x11_surface->surfaceType = EGL_PBUFFER_BIT;

    /* Get the root window */
    root = RootWindow((Display*)dpy, 0);

    /* Choose GLX visual / pixel format */
    x11_config->visinfo = glXChooseVisual((Display*)dpy, 0, visAttributes);
    if (!x11_config->visinfo)
    {
        printf("Error: couldn't create a pBuffer visual\n");
        lastError = EGL_BAD_ALLOC;
        return EGL_NO_SURFACE;
    }
    x11_surface->x11_config = x11_config;
    return (EGLSurface)x11_surface;
}

EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config,
                  NativePixmapType pixmap,
                  const EGLint *attrib_list)
{
    /* This is not supported. */
    return EGL_NO_SURFACE;
}


EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
    // Delete the surface. (Note, need to know what type of surface it was!)

    X11_SURFACE *x11_surface = (X11_SURFACE*)surface;

    if (x11_surface == NULL)
    {
        lastError = EGL_BAD_PARAMETER;
        return EGL_FALSE;
    }

    if (x11_surface->surfaceType == EGL_WINDOW_BIT)
    {
        XFreeColormap((Display*)dpy, x11_surface->attr.colormap);
    }

    if (x11_surface->x11_config && x11_surface->x11_config->visinfo)
    {
       XFree(x11_surface->x11_config->visinfo);
    }
    //Deallocating the surface
    free(x11_surface);

    // Free resources
    glXMakeCurrent((Display*)dpy, None, NULL);

    return EGL_TRUE;
}

EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface,
               EGLint attribute, EGLint *value)
{
    lastError = EGL_BAD_CONFIG;
    return EGL_FALSE;
}

EGLBoolean eglBindAPI(EGLenum api)
{
    if (api == EGL_OPENGL_ES_API)
        return EGL_TRUE;

    return EGL_FALSE;
}

EGLenum eglQueryAPI(void)
{
    return EGL_OPENGL_ES_API;
}

EGLBoolean eglWaitClient(void)
{
    return EGL_TRUE;
}

EGLBoolean eglReleaseThread(void)
{
    return EGL_TRUE;
}

EGLSurface eglCreatePbufferFromClientBuffer(
          EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
          EGLConfig config, const EGLint *attrib_list)
{
    lastError = EGL_BAD_CONFIG;
    return EGL_NO_SURFACE;
}

EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface,
                EGLint attribute, EGLint value)
{
    lastError = EGL_BAD_SURFACE;
    return EGL_FALSE;
}

EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    lastError = EGL_BAD_SURFACE;
    return EGL_FALSE;
}

EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    lastError = EGL_BAD_SURFACE;
    return EGL_FALSE;
}


EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    /* We do not support this, but NG calls it */
    return EGL_TRUE;
}


EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config,
                EGLContext share_context,
                const EGLint *attrib_list)
{
    GLXContext ctx;
    X11_CONFIG *x11_config = (X11_CONFIG*)config;

    if (x11_config == NULL)
    {
        lastError = EGL_BAD_PARAMETER;
        return EGL_NO_SURFACE;
    }

    /* Create GLX rendering context */
    ctx = glXCreateContext((Display*)dpy, x11_config->visinfo, NULL, True);

    if (!ctx)
    {
        printf("Error: glXCreateContext failed\n");
        lastError = EGL_BAD_ALLOC;
        return EGL_NO_CONTEXT;
    }

    return (EGLContext)ctx;
}

EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
    GLXContext gl_context = (GLXContext)ctx;

    if (!gl_context)
    {
        lastError = EGL_BAD_PARAMETER;
        return EGL_FALSE;
    }

    glXDestroyContext((Display*)dpy, gl_context);

    return EGL_TRUE;
}

EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw,
              EGLSurface read, EGLContext ctx)
{
    X11_SURFACE *x11_surface = (X11_SURFACE*)draw;

    if (x11_surface != NULL)
    {
        if (x11_surface->surfaceType == EGL_WINDOW_BIT)
        {
            /* Bind the rendering context and window */
            glXMakeCurrent((Display*)dpy, x11_surface->win, (GLXContext)ctx);
        }
        else
        {
            /* Bind the rendering context */
            glXMakeCurrent((Display*)dpy, 0, (GLXContext)ctx);
        }
    }

    /* Store the current state */
    currentDisplay = dpy;
    currentDrawSurface = draw;
    currentReadSurface = read;
    currentContext = ctx;

    return EGL_TRUE;
}


EGLContext eglGetCurrentContext(void)
{
    return currentContext;
}

EGLSurface eglGetCurrentSurface(EGLint readdraw)
{
    if (readdraw)
        return currentReadSurface;
    return currentDrawSurface;
}

EGLDisplay eglGetCurrentDisplay(void)
{
    return currentDisplay;
}

EGLBoolean eglQueryContext(EGLDisplay dpy, EGLContext ctx,
               EGLint attribute, EGLint *value)
{
    return EGL_FALSE;
}

EGLBoolean eglWaitGL(void)
{
    /* Wait for any outstanding GL operations to complete. */
    glXWaitGL();
    return EGL_TRUE;
}

EGLBoolean eglWaitNative(EGLint engine)
{
    /* Wait for any outstanding X operations to complete. */
    glXWaitX();
    return EGL_TRUE;
}

EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    X11_SURFACE *x11_surface = (X11_SURFACE*)surface;

    if (x11_surface == NULL)
    {
        lastError = EGL_BAD_PARAMETER;
        return EGL_FALSE;
    }

    /* Call GLX swap buffers */
    glXSwapBuffers((Display*)dpy, x11_surface->win);

    return EGL_TRUE;
}

EGLBoolean eglCopyBuffers(EGLDisplay dpy, EGLSurface surface,
              NativePixmapType target)
{
    /* This is not supported. */
    return EGL_FALSE;
}

__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *procname)
{
    /* This is provided here as it is just to fulfill the linkage
       requirements only. */
    return NULL;
}

/************************************************************************
* FUNCTION
*
*       glGenerateMipmap
*
* DESCRIPTION
*
*       Generate Mip-maps for the specified texture target. This
*       function has been provided here to fulfill linking
*       requirements for X11 OpenGL ES 2.0. This is a standard gl2
*       API function but is not available on X11 causing linking
*       issues.
*
* INPUTS
*
*       Texture target (2D / Cube texture)
*
* OUTPUTS
*
*       None
*
************************************************************************/
void glGenerateMipmap (GLenum target)
{
    // Do nothing for now as this is not directly available on Desktop  ...
}

void glShaderBinary (GLsizei n,
                     const GLuint* shaders,
                     GLenum binaryformat,
                     const void* binary,
                     GLsizei length)
{
    // Do nothing as Binary shaders are not supported on Desktop / emulation ...
}

void glClearDepthf (GLclampf depth)
{
    glClearDepth (depth);
}

void glDepthRangef (GLclampf near, GLclampf far)
{
    glDepthRange (near, far);
}
