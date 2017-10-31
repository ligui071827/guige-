/*************************************************************************
*
*            Copyright 2006 Mentor Graphics Corporation
*                         All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
*************************************************************************/

/*************************************************************************
*   FILE NAME
*
*       ifxui_porting_linux.h
*
*   COMPONENT
*
*       Inflexion UI Porting Layer.
*
*   DESCRIPTION
*
*       Inflexion UI Engine Porting API Internals for Linux.
*
*   DATA STRUCTURES
*
*       None
*
*   DEPENDENCIES
*
*       ifxui_engine.h
*
*************************************************************************/
#ifndef     _IFXUI_PORTING_LINUX_H_
#define     _IFXUI_PORTING_LINUX_H_

#ifdef      __cplusplus
extern "C" {
#endif      /* __cplusplus */

/* System includes */

#include    <stdlib.h>
#include    <stdio.h>
#include    <string.h>
#include    <signal.h>
#include    <unistd.h>
#include    <fcntl.h>
#include    <sys/errno.h>
#include    <sys/types.h>
#include    <sys/ipc.h>
#include    <sys/msg.h>
#include    <sys/stat.h>
#include    <sys/mman.h>
#include    <sys/ioctl.h>

/* Inflexion UI Engine includes. */
#include    "inflexionui/engine/ifxui_porting.h"

/* Number of ms to count the number of frames over when calculating
   frame rate */
#define     IFXP_BENCHMARK_FRAME_COUNT_TIME     5000

#if         defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || \
            defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)

/* Benchmarking event queue */
#define     PL_BENCHMARKING_MESSAGE_QUEUE_KEY   0x2345

#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE

extern unsigned int  pl_untracked_heap_usage;
extern unsigned int  pl_engine_heap_usage;
extern unsigned int  pl_engine_heap_usage_max;
extern unsigned int  pl_generic_heap_usage;
extern unsigned int  pl_generic_heap_usage_max;
extern unsigned int  pl_gpu_memory_usage;
extern unsigned int  pl_gpu_memory_usage_max;

#endif      /* IFX_ENABLE_BENCHMARKING_HEAP_USAGE */

#endif      /* defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) ||
               defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE) */

/* Function prototypes. */

IFX_RETURN_STATUS   PL_Initialize(void);
IFX_RETURN_STATUS   PL_Shutdown(void);
IFX_RETURN_STATUS   PL_Memory_Initialize(void);
IFX_RETURN_STATUS   PL_Memory_Shutdown(void);
IFX_RETURN_STATUS   PL_Display_Initialize(void);
IFX_RETURN_STATUS   PL_Display_Shutdown(void);
IFX_RETURN_STATUS   PL_File_Initialize(void);
IFX_RETURN_STATUS   PL_File_Shutdown(void);
IFX_RETURN_STATUS   PL_File_Change_Context(void);
IFX_RETURN_STATUS   PL_Font_Initialize(void);
IFX_RETURN_STATUS   PL_Font_Shutdown(void);

char               *PL_File_Convert_Drive_Letter(const char *engine_path);

#if         defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || \
            defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)

IFX_RETURN_STATUS   PL_Benchmarking_Initialize(void);
IFX_RETURN_STATUS   PL_Benchmarking_Shutdown(void);

#endif      /* defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) ||
               defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE) */

#ifdef      __cplusplus
}
#endif      /* __cplusplus */

#endif      /* _IFXUI_PORTING_LINUX_H_ */
