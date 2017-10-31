/***************************************************************************
*
*			   Copyright 2006 Mentor Graphics Corporation
*						 All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
****************************************************************************/

// NB: don't prevent from including more than once!

/*-------------------------------------------------------------------------*//**
	Feature-set dependencies - DO NOT edit these when selecting a build
	profile; these constraints are pulled in by LcProfileDefs.h or
	LcProjectDefs.h to ensure that the features selected are compatible
*/

// Things that require XML reading
#if defined(LC_USE_XML_SAVE)
	#define LC_USE_XML
#endif

// OpenGL setup.
#if defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_BUFFERED_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20)
	#define LC_PLAT_OGL				// Enable OpenGL ES support.

	#define LC_OGL_DIRECT

	#if defined(IFX_RENDER_DIRECT_OPENGL_20)
		#define LC_PLAT_OGL_20			// Enable OpenGL ES 2.0 support.
	#endif

	#if defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20)
		#define LC_PAINT_FULL			// Disable partial updates.
	#endif

	#define LC_USE_MESHES				// Include support for colored 3D meshes

	#if defined(LC_USE_MESHES)
		#define LC_USE_XML							// Include support for XML needed for .nd3 XML files
		#define LC_USE_XML_TEXT_NODES		// Include support for XML text nodes needed for parsing .nd3 XML files
	#endif

	#if !defined(NU_SIMULATION) && !defined(WIN32) && !defined(USE_X11_DESKTOP) && !defined(IFX_WIN_PLAYER)
		#if defined(GL_VERSION_ES_CM_1_1) && (GL_VERSION_ES_CM_1_1 == 1)
			#define LC_USE_OGL_VBOS
		#endif

		#if defined(GL_ES_VERSION_2_0)
			#define LC_USE_OGL_VBOS
		#endif
	#else
		// The Win32 and Linux X11 platforms require the OpenGLES 2.0 shader source
		// converting into a form compatible with desktop OpenGL. Also, the binary shader
		// support is not available on the Desktop OpenGL
		#define LC_USE_DESKTOP_OGL
	#endif // !defined(NU_SIMULATION) && !defined(WIN32) && !defined(USE_X11_DESKTOP) && !defined(IFX_WIN_PLAYER)

	#if defined(IFX_USE_EGL)
		#define LC_USE_EGL
	#endif

	#if defined(IFX_USE_UNIFIED_BINARY)
		#define LC_USE_UNIFIED_BINARY
	#endif

	#define LC_USE_LIGHTS				// Include support for secondary lights

	#if defined(LC_PLAT_OGL_20)
		// Include support of texture npot (non power of two) and define its threshold
		#define LC_TEXTURE_NPOT_THRESHOLD	10	// Percentage saving in storage
	#endif

#endif

#if defined(IFX_RENDER_INTERNAL_COMPRESSED)
	// Omit support for rotating layers in 2D mode.
	#define LC_USE_NO_ROTATION
#endif

#if !defined(IFX_USE_NATIVE_FONTS) && defined(LC_PLAT_SERIES60_2_FP3)
	#define IFX_USE_NATIVE_FONTS
#endif

// At least one font type is mandatory (default is bitmapped)
#if !defined(IFX_USE_NATIVE_FONTS)
	#define IFX_USE_BITMAPPED_FONTS
#endif
