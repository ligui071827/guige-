#
#               Copyright 2007 Mentor Graphics Corporation
#                         All Rights Reserved.
#
# THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS THE
# PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS SUBJECT
# TO LICENSE TERMS.
#

OBJS := $(OBJDIR)/ifxui_porting_general.o \
		$(OBJDIR)/ifxui_porting_file.o \
		$(OBJDIR)/ifxui_porting_font.o \
		$(OBJDIR)/ifxui_porting_memory.o \
		$(OBJDIR)/ifxui_porting_mutex.o \
		$(OBJDIR)/ifxui_porting_benchmarking.o

OBJS +=	$(OBJDIR)/ifxui_porting_image.o
OBJS +=	$(OBJDIR)/ifxui_porting_image_dds.o
OBJS +=	$(OBJDIR)/ifxui_porting_image_png.o
OBJS +=	$(OBJDIR)/ifxui_porting_image_jpeg.o
OBJS +=	$(OBJDIR)/ifxui_porting_image_bmp.o
OBJS +=	$(OBJDIR)/winbmp.o

# Set the porting layer display module based on the I/O System.
OBJS += $(OBJDIR)/ifxui_porting_display_$(IO_SYSTEM).o

ifeq ($(IO_SYSTEM),x11_desktop)
	OBJS += $(OBJDIR)/ifxui_porting_display_$(IO_SYSTEM)_egl.o
endif

LIBRARY=$(BUILDDIR)/lib/libInflexionUIPorting.a

default: all

include $(BUILDDIR)/inflexionui/library.mk

