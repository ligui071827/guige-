export
#CFLAGS += -Wall

# Profiling on
#CFLAGS += -pg

# Platform options
ifeq ($(IO_SYSTEM), x11)
CFLAGS += -I$(SYSROOT)/usr -I$(SYSROOT)/

else ifeq ($(IO_SYSTEM), fbdev)
CFLAGS += -I$(SYSROOT)/usr -I$(SYSROOT) -I$(SYSROOT)/usr/lib -I$(SYSROOT)/usr/include 
CXXFLAGS += -march=armv7-a -mthumb-interwork -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 -mno-unaligned-access
CFLAGS += -march=armv7-a -mthumb-interwork -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 -mno-unaligned-access
LDFLAGS += -march=armv7-a -mthumb-interwork -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 -mno-unaligned-access

endif

# Inflexion UI for Linux defines and includes
CFLAGS+=-I $(includedir) -DHAVE_OT -DHB_NO_MT
CFLAGS+=-I $(TOPDIR) -DLINUX
CFLAGS+=-I $(BUILDDIR)
CFLAGS+=-I $(TOPDIR)/inflexionui/engine
CFLAGS+=-I $(TOPDIR)/inflexionui/framework/inc
CFLAGS+=-I $(TOPDIR)/inflexionui/porting/inc
CFLAGS+=-I $(TOPDIR)/inflexionui/porting
CFLAGS+=-I $(TOPDIR)/inflexionui/modules/simpleedit/
CFLAGS+=-I $(TOPDIR)/inflexionui/modules/simpleedit/inc
CFLAGS+=-I $(TOPDIR)/inflexionui/modules/simpleedit
CFLAGS+=-I $(TOPDIR)/inflexionui/harfbuzz/src
CFLAGS+=-I $(TOPDIR)/fribidi
CFLAGS+=-I $(TOPDIR)/inflexionui/ucdn

# Required to build a shared library
CFLAGS += -fpic 

# Reduces superfluous code overhead
CXXFLAGS += -fno-exceptions -fno-rtti 
LDFLAGS += -Xlinker --gc-sections

# Do not warn about undefined shared library symbols (useful with OpenGL libraries)
LDFLAGS += -Xlinker --allow-shlib-undefined

# 4-byte to 2-byte wchar workaround
#CFLAGS += -fshort-wchar

LDFLAGS += -lpthread -lrt
LDFLAGS += -L $(libdir)

ifdef DEBUG_BUILD
  CFLAGS += -g -gstabs -DIFX_DEBUG_MODE
else
  CFLAGS += -O3
endif

ifeq ($(ENABLE_FREETYPE), 1)
  CFLAGS += -I $(SYSROOT)/usr/include/freetype2
  LDFLAGS += -lfreetype
  DEFS += -DIFX_USE_NATIVE_FONTS
endif

ifeq ($(ENABLE_PNG_LOADING), 1)
  CFLAGS += -I $(SYSROOT)/usr/include/libpng12
  LDFLAGS += -lpng12 -lz
  DEFS += -DIFXP_PNG_SUPPORT
endif

ifeq ($(ENABLE_JPEG_LOADING), 1)
  CFLAGS += -I $(SYSROOT)/usr/include
  LDFLAGS += -ljpeg
  DEFS += -DIFXP_JPEG_SUPPORT
endif

ifdef ENABLE_OPENGL
  DEFS += -DIFX_RENDER_DIRECT_OPENGL
else ifdef ENABLE_OPENGL_20
  DEFS += -DIFX_RENDER_DIRECT_OPENGL_20
else
  DEFS += -DIFX_RENDER_INTERNAL
endif

# Platform-specific settings.
# IMPORTANT: X11 Desktop uses a Pseudo EGL layer to enable compilation.
ifeq ($(PLATFORM), x11_desktop)
  CROSS_TOOLCHAIN=
  DEFS += -DIFX_CANVAS_MODE_8888
  ifdef ENABLE_OPENGL
	CFLAGS += -I /usr/include -I $(TOPDIR)/inflexionui/porting/opengles
	LDFLAGS += -lGL
  endif
  ifdef ENABLE_OPENGL_20
	CFLAGS += -I /usr/include -I $(TOPDIR)/inflexionui/porting/opengles
	LDFLAGS += -lGL
  endif
else ifeq ($(PLATFORM), x11)
  DEFS += -DIFX_CANVAS_MODE_8888
  ifdef ENABLE_OPENGL
	CFLAGS += -I $(SYSROOT)/usr/include/EGL -I $(SYSROOT)/usr/include/GLES
	LDFLAGS += -lGLESv1_CM -lEGL
  endif
  ifdef ENABLE_OPENGL_20
	CFLAGS += -I $(SYSROOT)/usr/include/EGL -I $(SYSROOT)/usr/include/GLES2
	LDFLAGS += -lGLESv2 -lEGL
  endif
else
  DEFS += -DIFX_CANVAS_MODE_565
  CROSS_TOOLCHAIN ?=
  ifdef ENABLE_OPENGL
	CFLAGS += -I $(SYSROOT)/usr/include/EGL -I $(SYSROOT)/usr/include/GLES --sysroot=$(SYSROOT)
	LDFLAGS += -lGLESv1_CM -lEGL
  endif
  ifdef ENABLE_OPENGL_20
	CFLAGS += -I $(SYSROOT)/usr/include/EGL -I $(SYSROOT)/usr/include/GLES2 --sysroot=$(SYSROOT)
	LDFLAGS += -lGLESv2 -lEGL
  endif
endif

# I/O System-specific settings.
ifeq ($(IO_SYSTEM), x11_desktop)
  DEFS += -DUSE_X11_DESKTOP
  # 32 Bit
  CFLAGS += -m32
  LDFLAGS += -m32
  LDFLAGS += -lX11
else ifeq ($(IO_SYSTEM), x11)
  DEFS += -DUSE_X11
  LDFLAGS += -lX11
else
  DEFS += -DUSE_FBDEV
endif

# Work out the cross compile host type
HOST_TMP = $(subst -foo,,$(notdir $(CROSS_TOOLCHAIN))foo)
HOST = $(subst foo,,$(HOST_TMP))

buzhid