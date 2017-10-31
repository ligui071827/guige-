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

#ifndef SimpleEditPlatformH
#define SimpleEditPlatformH

#include    <sys/time.h>

/* Allocate Memory */
#define SimpleEdit_AllocateMemoryUnsafe(size, ptr)                       \
                               *ptr = malloc(size);                      \

/* Free memory */
#define SimpleEdit_FreeMemoryUnsafe(ptr)                                 \
                               if (ptr != NULL)                          \
                               {                                         \
                                  free(ptr);                             \
                               }                                         \

/* Get current time */
#define SimpleEdit_Get_Current_Time(time)                                \
                                struct timeval tp;                       \
                                gettimeofday(&tp, NULL);                 \
                                *time = tp.tv_sec * 1000;                \
                                *time += tp.tv_usec / 1000;              \

#endif /* SimpleEditPlatformH */
