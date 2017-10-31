#
#               Copyright 2007 Mentor Graphics Corporation
#                         All Rights Reserved.
#
# THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS THE
# PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS SUBJECT
# TO LICENSE TERMS.
#

SHARED_LIBRARY_OBJS = \
engine/$(OBJDIR)/ifxui_engine.o \
engine/$(OBJDIR)/ifxui_rtl.o \
engine/$(OBJDIR)/ifxui_uriparser.o \
engine/$(OBJDIR)/NdhsIntegrationInternals.o 

SHARED_LIBRARY_OBJS += porting/$(OBJDIR)/ifxui_porting_display_$(IO_SYSTEM).o
SHARED_LIBRARY_OBJS += porting/$(OBJDIR)/ifxui_porting_general.o
ifeq ($(IO_SYSTEM),x11_desktop)
	SHARED_LIBRARY_OBJS += porting/$(OBJDIR)/ifxui_porting_display_$(IO_SYSTEM)_egl.o
endif

default: all

include rules.mk

LIBRARY = $(BUILDDIR)/lib/$(SHARED_LIBRARY)
LIBRARY_FBIDI = $(BUILDDIR)/lib/$(SHARED_LIBRARY_FRIBIDI)
ifeq ($(ENABLE_FREETYPE), 1)
    LINK_LIBRARIES = libInflexionUIEngine,libInflexionUIPorting,libHarfbuzz,libUCDN
else
    LINK_LIBRARIES = libInflexionUIEngine,libInflexionUIPorting
endif

.PHONY: all
all:
	@$(MAKE) -C engine
#	@$(MAKE) -C modules
	@$(MAKE) -C porting
ifeq ($(ENABLE_FREETYPE), 1)
	@$(MAKE) -C harfbuzz
	@$(MAKE) -C ucdn
endif
	@$(RM) -f $(OBJS) $(LIBRARY)* $(LIBRARY_FBIDI)*
	@$(CROSS_TOOLCHAIN)$(IFX_CC) -Wl,-s \
	 $(LDFLAGS) \
	 -Xlinker --exclude-libs -Xlinker $(LINK_LIBRARIES) \
	 -shared -Wl,-soname,$(SHARED_LIBRARY) -o $(LIBRARY)$(SHARED_LIBRARY_VERSION)$(SHARED_LIBRARY_SUFFIX) $(SHARED_LIBRARY_OBJS) $(BUILDDIR)/lib/*.a $(BUILDDIR)/lib/*.a
	@ln -s $(SHARED_LIBRARY)$(SHARED_LIBRARY_VERSION)$(SHARED_LIBRARY_SUFFIX) $(LIBRARY)$(SHARED_LIBRARY_VERSION)
	@ln -s $(SHARED_LIBRARY)$(SHARED_LIBRARY_VERSION) $(LIBRARY)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(LIBRARY) $(LIBRARY_FBIDI)*
	$(RM) $(LIBRARY)$(SHARED_LIBRARY_SUFFIX)
	$(MAKE) clean -C engine
	$(MAKE) clean -C modules
	$(MAKE) clean -C porting
ifeq ($(ENABLE_FREETYPE), 1)
	$(MAKE) clean -C harfbuzz
	$(MAKE) clean -C ucdn
endif
	$(RM) $(LIBRARY)$(SHARED_LIBRARY_VERSION)
	$(RM) $(LIBRARY)$(SHARED_LIBRARY_VERSION)
	$(RM) $(LIBRARY)$(SHARED_LIBRARY_VERSION)$(SHARED_LIBRARY_SUFFIX)

.PHONY: install
install:
	$(MAKE) install -C engine
	$(MAKE) install -C modules
	$(MAKE) install -C porting
ifeq ($(ENABLE_FREETYPE), 1)
	$(MAKE) install -C harfbuzz
	$(MAKE) install -C ucdn
endif
	$(INSTALL) -s $(LIBRARY)$(SHARED_LIBRARY_VERSION)$(SHARED_LIBRARY_SUFFIX) -D $(DESTDIR)/$(libdir)/$(SHARED_LIBRARY)$(SHARED_LIBRARY_VERSION)$(SHARED_LIBRARY_SUFFIX)
	$(INSTALL) -s $(LIBRARY)$(SHARED_LIBRARY_VERSION) -D $(DESTDIR)/$(libdir)/$(SHARED_LIBRARY)$(SHARED_LIBRARY_VERSION)
	$(INSTALL) -s $(LIBRARY) -D $(DESTDIR)/$(libdir)/$(SHARED_LIBRARY)
