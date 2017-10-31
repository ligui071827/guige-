/* $Id: winbmp.c 13764 2011-05-04 03:01:23Z humingming $
**
** Low-level Windows bitmap read/save function.
**
** Copyright (C) 2003 ~ 2008 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** All rights reserved by Feynman Software.
**
** Create date: 2000/08/26, derived from original bitmap.c
**
** Current maintainer: Wei Yongming.
*/
#ifndef _WINBMP_H_
#define _WINBMP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define BMP_DEBUG
#ifdef BMP_DEBUG
    #define dprintf(format, ...) printf("[BMP]" format, ##__VA_ARGS__)
#else
    #define dprintf(format, ...)
#endif

//#define HAVE_PALETTE

typedef unsigned char      BYTE;
typedef unsigned char      Uint8;
typedef unsigned int       Uint32;
typedef unsigned short     WORD;
typedef unsigned long      DWORD;

typedef struct _RGB {
    BYTE r;
    BYTE g;
    BYTE b;
    BYTE a;
} RGB;

/**     
 * * \defgroup bmp_struct Bitmap structure     *     
 * * MiniGUI uses a MYBITMAP structure to represent a device-independent      
 * * bitmap, and BITMAP structure to represent a device-dependent bitmap.     
 * *     
 * * @{     
 * */
#define MYBMP_TYPE_NORMAL       0x00000000
#define MYBMP_TYPE_RLE4         0x00000001
#define MYBMP_TYPE_RLE8         0x00000002
#define MYBMP_TYPE_RGB          0x00000003
#define MYBMP_TYPE_BGR          0x00000004
#define MYBMP_TYPE_RGBA         0x00000005
#define MYBMP_TYPE_MASK         0x0000000F
#define MYBMP_FLOW_DOWN         0x00000010
#define MYBMP_FLOW_UP           0x00000020
#define MYBMP_FLOW_MASK         0x000000F0
#define MYBMP_TRANSPARENT       0x00000100
#define MYBMP_ALPHACHANNEL      0x00000200
#define MYBMP_ALPHA             0x00000400
#define MYBMP_RGBSIZE_3         0x00001000
#define MYBMP_RGBSIZE_4         0x00002000
#define MYBMP_LOAD_GRAYSCALE    0x00010000
#define MYBMP_LOAD_ALLOCATE_ONE 0x00020000
#define MYBMP_LOAD_NONE         0x00000000

/** Device-independent bitmap structure. */
typedef struct _MYBITMAP{
    /**     
     * * Flags of the bitmap, can be OR'ed by the following values:     
     * *  - MYBMP_TYPE_NORMAL\n     
     * *    A normal palette bitmap.     
     * *  - MYBMP_TYPE_RGB\n     
     * *    A RGB bitmap.     
     * *  - MYBMP_TYPE_BGR\n     
     * *    A BGR bitmap.     
     * *  - MYBMP_TYPE_RGBA\n     
     * *    A RGBA bitmap.     
     * *  - MYBMP_FLOW_DOWN\n     
     * *    The scanline flows from top to bottom.     
     * *  - MYBMP_FLOW_UP\n     
     * *    The scanline flows from bottom to top.     
     * *  - MYBMP_TRANSPARENT\n     
     * *    Have a trasparent value.     
     * *  - MYBMP_ALPHACHANNEL\n     
     * *    Have a alpha channel.     
     * *  - MYBMP_ALPHA\n     
     * *    Have a per-pixel alpha value.     
     * *  - MYBMP_RGBSIZE_3\n     
     * *    Size of each RGB triple is 3 bytes.     
     * *  - MYBMP_RGBSIZE_4\n     
     * *    Size of each RGB triple is 4 bytes.     
     * *  - MYBMP_LOAD_GRAYSCALE\n     
     * *    Tell bitmap loader to load a grayscale bitmap.     
     * *  - MYBMP_LOAD_ALLOCATE_ONE\n     
     *    Tell bitmap loader to allocate space for only one scanline.     
     *    */
    DWORD flags;    
    /** The number of the frames. */    
    int   frames;    
    /** The pixel depth. */    
    Uint8 depth;    
    /** The alpha channel value. */    
    Uint8 alpha;    
    Uint8 reserved [2];    
    /** The transparent pixel. */    
    Uint32 transparent;    

    /** The width of the bitmap. */    
    Uint32 w;    
    /** The height of the bitmap. */    
    Uint32 h;    
    /** The pitch of the bitmap. */    
    Uint32 pitch;    
    /** The size of the bits of the bitmap. */    
    Uint32 size;    

    /** The pointer to the bits of the bitmap. */    
    BYTE* bits;

    unsigned long biCompression;
    DWORD rmask, gmask, bmask;
} MYBITMAP;

int __mg_init_bmp(FILE* fp, MYBITMAP *bmp, RGB *pal);
int __mg_load_bmp(FILE* fp,MYBITMAP *bmp);
#endif
