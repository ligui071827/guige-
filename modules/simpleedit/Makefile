# Copyright Mentor Graphics Corporation 2009
#
#    All Rights Reserved.
#
# THIS WORK CONTAINS TRADE SECRET
# AND PROPRIETARY INFORMATION WHICH IS THE
# PROPERTY OF MENTOR GRAPHICS
# CORPORATION OR ITS LICENSORS AND IS
# SUBJECT TO LICENSE TERMS. 

OBJS := \
$(OBJDIR)/ifxui_simpleedit.o \
$(OBJDIR)/simpleedit_multitap.o

LIBRARY=$(BUILDDIR)/lib/libInflexionUISimpleEdit.a

default: all

vpath %.cpp .:src
vpath %.c .:src

all: $(LIBRARY)

include $(BUILDDIR)/inflexionui/library.mk

