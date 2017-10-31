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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "common.h"
//#include "gdi.h"
//#include "readbmp.h"
#include "winbmp.h"

//xg add for porting start
#ifdef HAVE_PALETTE
#define MAX_NCOLORS         256
#endif

typedef int                 BOOL;
#define FALSE               0
#define TRUE                1
#if 0
typedef unsigned char       BYTE;
typedef unsigned char       Uint8;
typedef unsigned int        Uint32;
typedef unsigned long       DWORD;
#endif

/**     
 * * \defgroup bmp_struct Bitmap structure     *     
 * * MiniGUI uses a MYBITMAP structure to represent a device-independent      
 * * bitmap, and BITMAP structure to represent a device-dependent bitmap.     
 * *     
 * * @{     
 * */
#if 0
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
#endif

#define ERR_BMP_OK              0
#define ERR_BMP_IMAGE_TYPE      -1
#define ERR_BMP_UNKNOWN_TYPE    -2
#define ERR_BMP_CANT_READ       -3
#define ERR_BMP_CANT_SAVE       -4
#define ERR_BMP_NOT_SUPPORTED   -5   
#define ERR_BMP_MEM             -6
#define ERR_BMP_LOAD            -7
#define ERR_BMP_FILEIO          -8
#define ERR_BMP_OTHER           -9
#define ERR_BMP_ERROR_SOURCE    -10

unsigned char fp_getc(FILE* fp)
{
    unsigned char data = 0;
    fread(&data, 1, 1, fp);
    return data;
}
unsigned short fp_igetw(FILE* fp)
{
    unsigned short data = 0;
    fread(&data, 2, 1, fp);
    return data;
}
unsigned int fp_igetl(FILE* fp)
{
    unsigned int data = 0;
    fread(&data, 4, 1, fp);
    return data;
}
//xg add for porting end

/************************* Bitmap-related structures  ************************/
typedef struct tagRGBTRIPLE
{
    BYTE    rgbtBlue;
    BYTE    rgbtGreen;
    BYTE    rgbtRed;
} RGBTRIPLE;
typedef RGBTRIPLE* PRGBTRIPLE;

typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD* PRGBQUAD;

#define SIZEOF_RGBQUAD      4

#define BI_RGB          0
#define BI_RLE8         1
#define BI_RLE4         2
#define BI_BITFIELDS    3

#define SIZEOF_BMFH     14
#define SIZEOF_BMIH     40

#define OS2INFOHEADERSIZE  12
#define WININFOHEADERSIZE  40

typedef struct BITMAPFILEHEADER
{
   unsigned short bfType;
   unsigned long  bfSize;
   unsigned short bfReserved1;
   unsigned short bfReserved2;
   unsigned long  bfOffBits;
} BITMAPFILEHEADER;


/* Used for both OS/2 and Windows BMP. 
 * Contains only the parameters needed to load the image 
 */
typedef struct BITMAPINFOHEADER
{
   unsigned long  biWidth;
   unsigned long  biHeight;
   unsigned short biBitCount;
   unsigned long  biCompression;
} BITMAPINFOHEADER;


typedef struct WINBMPINFOHEADER  /* size: 40 */
{
   unsigned long  biSize;
   unsigned long  biWidth;
   unsigned long  biHeight;
   unsigned short biPlanes;
   unsigned short biBitCount;
   unsigned long  biCompression;
   unsigned long  biSizeImage;
   unsigned long  biXPelsPerMeter;
   unsigned long  biYPelsPerMeter;
   unsigned long  biClrUsed;
   unsigned long  biClrImportant;
} WINBMPINFOHEADER;


typedef struct OS2BMPINFOHEADER  /* size: 12 */
{
   unsigned long  biSize;
   unsigned short biWidth;
   unsigned short biHeight;
   unsigned short biPlanes;
   unsigned short biBitCount;
} OS2BMPINFOHEADER;

/* read_bmfileheader:
 *  Reads a BMP file header and check that it has the BMP magic number.
 */
static int read_bmfileheader(FILE *f, BITMAPFILEHEADER *fileheader)
{
   fileheader->bfType = fp_igetw(f);
   fileheader->bfSize= fp_igetl(f);
   fileheader->bfReserved1= fp_igetw(f);
   fileheader->bfReserved2= fp_igetw(f);
   fileheader->bfOffBits= fp_igetl(f);

   dprintf("BITMAPFILEHEADER\n\tbfType:0x%04x\n\tbfSize:0x%08x\n\tbfReserved1:0x%04x\n\tbfReserved2:0x%04x\n\tbfOffBits:0x%08x\n", 
        fileheader->bfType, fileheader->bfSize, fileheader->bfReserved1, fileheader->bfReserved2, fileheader->bfOffBits);
   if (fileheader->bfType != 19778)
      return -1;

   return 0;
}


/* read_win_bminfoheader:
 *  Reads information from a BMP file header.
 */
static int read_win_bminfoheader(FILE *f, BITMAPINFOHEADER *infoheader)
{
   WINBMPINFOHEADER win_infoheader;

   win_infoheader.biWidth = fp_igetl(f);
   win_infoheader.biHeight = fp_igetl(f);
   win_infoheader.biPlanes = fp_igetw(f);
   win_infoheader.biBitCount = fp_igetw(f);
   win_infoheader.biCompression = fp_igetl(f);
   win_infoheader.biSizeImage = fp_igetl(f);
   win_infoheader.biXPelsPerMeter = fp_igetl(f);
   win_infoheader.biYPelsPerMeter = fp_igetl(f);
   win_infoheader.biClrUsed = fp_igetl(f);
   win_infoheader.biClrImportant = fp_igetl(f);

   dprintf("WINBMPINFOHEADER\n\tbiWidth:0x%08x\n\tbiHeight:0x%08x\n\tbiPlanes:0x%04x\n\tbiBitCount:0x%04x\n\tbiCompression:0x%08x\n\t"
        "biSizeImage:0x%08x\n\tbiXPelsPerMeter:0x%08x\n\tbiYPelsPerMeter:0x%08x\n\tbiClrUsed:0x%08x\n\tbiClrImportant:0x%08x\n",
        win_infoheader.biWidth, win_infoheader.biHeight, win_infoheader.biPlanes, win_infoheader.biBitCount, win_infoheader.biCompression,
        win_infoheader.biSizeImage, win_infoheader.biXPelsPerMeter, win_infoheader.biYPelsPerMeter, win_infoheader.biClrUsed, win_infoheader.biClrImportant);

   infoheader->biWidth = win_infoheader.biWidth;
   infoheader->biHeight = win_infoheader.biHeight;
   infoheader->biBitCount = win_infoheader.biBitCount;
   infoheader->biCompression = win_infoheader.biCompression;

   return 0;
}


/* read_os2_bminfoheader:
 *  Reads information from an OS/2 format BMP file header.
 */
static int read_os2_bminfoheader(FILE *f, BITMAPINFOHEADER *infoheader)
{
   OS2BMPINFOHEADER os2_infoheader;

   os2_infoheader.biWidth = fp_igetw(f);
   os2_infoheader.biHeight = fp_igetw(f);
   os2_infoheader.biPlanes = fp_igetw(f);
   os2_infoheader.biBitCount = fp_igetw(f);

   infoheader->biWidth = os2_infoheader.biWidth;
   infoheader->biHeight = os2_infoheader.biHeight;
   infoheader->biBitCount = os2_infoheader.biBitCount;
   infoheader->biCompression = 0;

   return 0;
}


/* read_bmicolors:
 *  Loads the color palette for 1,4,8 bit formats.
 */
static void read_bmicolors (int ncols, RGB *pal, FILE *f, int win_flag)
{
   int i;

   for (i=0; i<ncols; i++) {
      pal[i].b = fp_getc(f);
      pal[i].g = fp_getc(f);
      pal[i].r = fp_getc(f);
      if (win_flag)
	    fp_getc(f);
   }
}

#define BMP_ERR     0
#define BMP_LINE    1
#define BMP_END     2
/* read_RLE8_compressed_image:
 *  For reading the 8 bit RLE compressed BMP image format.
 */
static int read_RLE8_compressed_image(FILE * f, BYTE * bits, int pitch, int width)
{
    unsigned char count, val, val0;
    int j, pos = 0;
    int flag = BMP_ERR;

    while (pos <= width && flag == BMP_ERR) {
	    count = fp_getc(f);
	    val = fp_getc(f);

        if (count > 0) {
	        for (j = 0;j < count;j++) {
	            bits[pos] = val;
	            pos++;
	        }
	    }
	    else {
            switch (val) {

            case 0:                       /* end of line flag */
	            flag = BMP_LINE;
	        break;

	        case 1:                       /* end of picture flag */
	            flag = BMP_END;
	        break;

	        case 2:                       /* displace picture */
                count = fp_getc(f);
	            val = fp_getc(f);
	            pos += count;
	        break;

            default:                      /* read in absolute mode */
	            for (j = 0; j < val; j++) {
	                val0 = fp_getc(f);
                    bits[pos] = val0;
	                pos++;
	            }
	            if (j % 2 == 1)
	                val0 = fp_getc(f);    /* align on word boundary */
	        break;
	        }
	    }
    }

    return flag;
}

/* read_RLE4_compressed_image:
 *  For reading the 4 bit RLE compressed BMP image format.
 */

static int read_RLE4_compressed_image (FILE *f, BYTE *bits, int pitch, int width)
{
    unsigned char b[8];
    unsigned char count;
    unsigned short val0, val;
    int j, k, pos = 0, flag = BMP_ERR;

    while (pos <= width && flag == BMP_ERR) {
        count = fp_getc(f);
        val = fp_getc(f);

        if (count > 0) {                    /* repeat pixels count times */
            b[1] = val & 15;
            b[0] = (val >> 4) & 15;
            for (j = 0; j < count; j++) {
                if (pos % 2 == 0)
                    bits[pos / 2] = b[0] << 4;
                else
                    bits[pos / 2] = bits[pos / 2] | b[1];
                if (pos >= width) return flag;
                pos++;
            }
        }
        else {
            switch (val) {
            case 0:                       /* end of line */
                flag = BMP_LINE;
            break;

            case 1:                       /* end of picture */
                flag = BMP_END;
            break;

            case 2:                       /* displace image */
                count = fp_getc(f);
                val = fp_getc(f);
                pos += count;
            break;

            default:                      /* read in absolute mode */
                for (j=0; j<val; j++) {
                    if ((j%4) == 0) {
                        val0 = fp_igetw(f);
                        for (k=0; k<2; k++) {
                            b[2*k+1] = val0 & 15;
                            val0 = val0 >> 4;
                            b[2*k] = val0 & 15;
                            val0 = val0 >> 4;
                        }
                    }

                    if (pos % 2 == 0)
                        bits [pos/2] = b[j%4] << 4;
                    else
                        bits [pos/2] = bits [pos/2] | b[j%4];
                    pos++;
                }
            break;

            }
        }
    }

    return flag;
}

/* read_16bit_image:
 *  For reading the 16-bit BMP image format.
 * This only support bit masks specific to Windows 95.
 */
static void read_16bit_image (FILE *f, BYTE *bits, int pitch, int width, DWORD gmask)
{
    int i;
    WORD pixel;
    BYTE *line;

    line = bits;
    for (i = 0; i < width; i++) {
        pixel = fp_igetw (f);
        if (gmask == 0x03e0)    /* 5-5-5 */
        {
            line [2] = ((pixel >> 10) & 0x1f) << 3;
            line [1] = ((pixel >> 5) & 0x1f) << 3;
            line [0] = (pixel & 0x1f) << 3;
        }
        else                    /* 5-6-5 */
        {
            line [2] = ((pixel >> 11) & 0x1f) << 3;
            line [1] = ((pixel >> 5) & 0x3f) << 2;
            line [0] = (pixel & 0x1f) << 3;
        }

        line += 3;
    }

    if (width & 0x01)
        pixel = fp_igetw (f);   /* read the gap */

}

#define PIX2BYTES(n)    (((n)+7)/8)
/* * compute image line size and bytes per pixel 
 * * from bits per pixel and width 
 * */
int bmp_ComputePitch(int bpp, Uint32 width, Uint32 *pitch, BOOL does_round)
{    
    int linesize;    
    int bytespp = 1;    
    
    if(bpp == 1)        
        linesize = PIX2BYTES (width);    
    else if(bpp <= 4)        
        linesize = PIX2BYTES (width << 2);    
    else if (bpp <= 8)        
        linesize = width;    
    else if(bpp <= 16) {        
        linesize = width * 2;        
        bytespp = 2;    
    } else if(bpp <= 24) {        
        linesize = width * 3;        
        bytespp = 3;    
    } else {        
        linesize = width * 4;        
        bytespp = 4;    
    }    
    
    /* rows are DWORD right aligned */    
    if (does_round)        
        *pitch = (linesize + 3) & -4;    
    else        
        *pitch = linesize;    
        
    return bytespp;
}

/* __mg_load_bmp:
 *  Loads a Windows BMP file, returning in the my_bitmap structure and storing
 *  the palette data in the specified palette (this should be an array of
 *  at least 256 RGB structures).
 *
 *  Thanks to Seymour Shlien for contributing this function.
 */

int __mg_init_bmp (FILE* fp, MYBITMAP * bmp, RGB * pal)
{
    dprintf("%s;%d\n", __func__, __LINE__);
    int effect_depth, biSize;
#ifdef HAVE_PALETTE
    int ncol;
#endif
    BITMAPFILEHEADER fileheader;
    BITMAPINFOHEADER infoheader;

    if (read_bmfileheader (fp, &fileheader) != 0)
        return -1;

    biSize = fp_igetl(fp);
    dprintf("biSize:0x%08x\n", biSize);
    if (biSize >= WININFOHEADERSIZE) {
        if (read_win_bminfoheader (fp, &infoheader) != 0)
            return -1;

        fseek (fp, biSize - WININFOHEADERSIZE, SEEK_CUR);//seek to color table

#ifdef HAVE_PALETTE
        ncol = (fileheader.bfOffBits - biSize - 14) / 4;
        if(ncol > MAX_NCOLORS)
            ncol = MAX_NCOLORS;
        /* there only 1,4,8 bit read color panel data */
        if (infoheader.biBitCount <= 8)
            read_bmicolors(ncol, pal, fp, 1);
#else
        if (infoheader.biBitCount <= 8)
            return -1; 
#endif
    }
    else if (biSize == OS2INFOHEADERSIZE) {
        if (read_os2_bminfoheader (fp, &infoheader) != 0)
            return -1;

#ifdef HAVE_PALETTE
        ncol = (fileheader.bfOffBits - 26) / 3;
        if(ncol > MAX_NCOLORS)
            ncol = MAX_NCOLORS;
        if (infoheader.biBitCount <= 8)
            read_bmicolors (ncol, pal, fp, 0);
#else
        if (infoheader.biBitCount <= 8)
            return -1; 
#endif
    }
    else
        return -1;

#if 0
    if (infoheader.biBitCount == 16)
        effect_depth = 24;
    else
        effect_depth = infoheader.biBitCount;
#endif
    //bmp_ComputePitch(effect_depth, infoheader.biWidth, (Uint32 *)(&bmp->pitch), TRUE);
    bmp_ComputePitch(infoheader.biBitCount, infoheader.biWidth, (Uint32 *)(&bmp->pitch), TRUE);

    bmp->rmask = 0x001f;
    bmp->gmask = 0x03e0;
    bmp->bmask = 0x7c00;
    //bmp->flags |= MYBMP_TYPE_BGR | MYBMP_FLOW_DOWN;
    bmp->depth = infoheader.biBitCount;
    bmp->w     = infoheader.biWidth;
    bmp->h     = infoheader.biHeight;
    if (infoheader.biBitCount == 16)
        bmp->size = (bmp->w*bmp->h*24+7)/8;
    else
        bmp->size = (bmp->w*bmp->h*bmp->depth+7)/8;
    bmp->biCompression = infoheader.biCompression;

    return 0;
}

void __mg_cleanup_bmp (void* init_info)
{
    if (init_info)
        free(init_info);

    init_info = NULL;
}

int __mg_load_bmp(FILE* fp, MYBITMAP *bmp)
{
    dprintf("%s;%d\n", __func__, __LINE__);
    int i, flag;
    BYTE * bits;
    int bits_linesize;

    switch (bmp->biCompression) {
        case BI_BITFIELDS:
            bmp->rmask = fp_igetl(fp);
            bmp->gmask = fp_igetl(fp);
            bmp->bmask = fp_igetl(fp);
        break;

        case BI_RGB:
#if 0
            if (bmp->depth == 16)
                bmp->flags |= MYBMP_RGBSIZE_3;
            else if (bmp->depth == 32)
                bmp->flags |= MYBMP_RGBSIZE_4;
            else
                bmp->flags |= MYBMP_RGBSIZE_3;
#endif
        break;

        case BI_RLE8:
        case BI_RLE4:
        break;

        default:
            goto err;
    }

    flag = BMP_LINE;
    if(bmp->depth == 16)
        bits_linesize = bmp->w * 3;
    else if(bmp->depth == 32)
        bits_linesize = bmp->w * 4;
    else
        bits_linesize = bmp->w * 3;
    bits = bmp->bits + (bmp->h - 1) * bits_linesize;
    dprintf("bmp->bits:%p, bits:%p, w:%d, h:%d, size:%d\n", bmp->bits, bits, bmp->w, bmp->h, bmp->size);
    for (i = bmp->h - 1; i >= 0; i--, bits -= bits_linesize ) {
        switch (bmp->biCompression) {
        case BI_BITFIELDS:
        case BI_RGB:
            if (bmp->depth == 16){
                read_16bit_image(fp, bits, bmp->pitch, bmp->w, bmp->gmask);

                char tmp = 0;
                int j;
                for(j = 0; j < bmp->w; j++){
                    tmp = *(bits+j*3); 
                    *(bits+j*3) = *(bits+j*3+2);
                    *(bits+j*3+2) = tmp;
                } 
            }
            else if (bmp->depth == 32){
                fread(bits, bmp->pitch, 1, fp);

                char tmp = 0;
                int j;
                for(j = 0; j < bmp->w; j++){
                    tmp = *(bits+j*4); 
                    *(bits+j*4) = *(bits+j*4+2);
                    *(bits+j*4+2) = tmp;
                    //*(bits+j*4+3) = 0xff;
                } 
            }
            else{
                fread(bits, bmp->w*bmp->depth/8, 1, fp);

                char tmp = 0;
                int j;
                for(j = 0; j < bmp->w; j++){
                    tmp = *(bits+j*3); 
                    *(bits+j*3) = *(bits+j*3+2);
                    *(bits+j*3+2) = tmp;
                } 

                fseek(fp, bmp->pitch-bmp->w*bmp->depth/8, SEEK_CUR);
            }
        break;

        case BI_RLE8:
            flag = read_RLE8_compressed_image(fp, bits, bmp->pitch, bmp->w);
            if (flag == BMP_ERR)
                goto err;
        break;

        case BI_RLE4:
            flag = read_RLE4_compressed_image(fp, bits, bmp->pitch, bmp->w);
            if (flag == BMP_ERR)
                goto err;
        break;

        }

        if (flag == BMP_END)
            goto ret;
    }

ret:
    return ERR_BMP_OK;

err:
    return ERR_BMP_OTHER;
}

#if 0
BOOL __mg_check_bmp (FILE* fp)
{
   WORD bfType = fp_igetw (fp);

   if (bfType != 19778)
      return FALSE;

   return TRUE;
}
#endif

#if 0
#ifdef _MGMISC_SAVEBITMAP
static void bmpGet16CScanline(BYTE* bits, BYTE* scanline, 
                        int pixels)
{
    int i;

    for (i = 0; i < pixels; i++) {
        if (i % 2 == 0)
            *scanline = (bits [i] << 4) & 0xF0;
        else {
            *scanline |= bits [i] & 0x0F;
            scanline ++;
        }
    }
}

static inline void bmpGet256CScanline (BYTE* bits, BYTE* scanline, 
                        int pixels)
{
    memcpy (scanline, bits, pixels);
}

inline void pixel2rgb (gal_pixel pixel, GAL_Color* color, int depth)
{
    switch (depth) {
    case 24:
    case 32:
        color->r = (gal_uint8) ((pixel >> 16) & 0xFF);
        color->g = (gal_uint8) ((pixel >> 8) & 0xFF);
        color->b = (gal_uint8) (pixel & 0xFF);
        break;

    case 15:
        color->r = (gal_uint8)((pixel & 0x7C00) >> 7) | 0x07;
        color->g = (gal_uint8)((pixel & 0x03E0) >> 2) | 0x07;
        color->b = (gal_uint8)((pixel & 0x001F) << 3) | 0x07;
        break;

    case 16:
        color->r = (gal_uint8)((pixel & 0xF800) >> 8) | 0x07;
        color->g = (gal_uint8)((pixel & 0x07E0) >> 3) | 0x03;
        color->b = (gal_uint8)((pixel & 0x001F) << 3) | 0x07;
        break;
    }
}

static void bmpGetHighCScanline (BYTE* bits, BYTE* scanline, 
                        int pixels, int bpp, int depth)
{
    int i;
    gal_pixel c;
    GAL_Color color;
    memset (&color, 0, sizeof(GAL_Color));

    for (i = 0; i < pixels; i++) {
        c = *((gal_pixel*)bits);

        pixel2rgb (c, &color, depth);
        *(scanline)     = color.b;
        *(scanline + 1) = color.g;
        *(scanline + 2) = color.r;

        bits += bpp;
        scanline += 3;
    }
}

inline static int depth2bpp (int depth)
{
    switch (depth) {
    case 4:
    case 8:
        return 1;
    case 15:
    case 16:
        return 2;
    case 24:
        return 3;
    case 32:
        return 4;
    }
    
    return 1;
}

int __mg_save_bmp (FILE* fp, MYBITMAP* bmp, RGB* pal)
{
    BYTE* scanline = NULL;
    int i, bpp;
    int scanlinebytes;

    BITMAPFILEHEADER bmfh;
    WINBMPINFOHEADER bmih;

    memset (&bmfh, 0, sizeof (BITMAPFILEHEADER));
    bmfh.bfType         = MAKEWORD ('B', 'M');
    bmfh.bfReserved1    = 0;
    bmfh.bfReserved2    = 0;

    memset (&bmih, 0, sizeof (WINBMPINFOHEADER));
    bmih.biSize         = (DWORD)(WININFOHEADERSIZE);
    bmih.biWidth        = (DWORD)(bmp->w);
    bmih.biHeight       = (DWORD)(bmp->h);
    bmih.biPlanes       = 1;
    bmih.biCompression  = BI_RGB;

    bpp = depth2bpp (bmp->depth);
    switch (bmp->depth) {
        case 4:
            scanlinebytes       = (bmih.biWidth + 1)>>1;
            scanlinebytes       = ((scanlinebytes + 3)>>2)<<2;

#ifdef HAVE_ALLOCA
            if (!(scanline = alloca (scanlinebytes))) return ERR_BMP_MEM;
#else
            if (!(scanline = malloc (scanlinebytes))) return ERR_BMP_MEM;
#endif
            memset (scanline, 0, scanlinebytes);

            bmih.biSizeImage    = (DWORD)(bmih.biHeight*scanlinebytes);
            bmfh.bfOffBits      = SIZEOF_BMFH + SIZEOF_BMIH
                                    + (SIZEOF_RGBQUAD<<4);
            bmfh.bfSize         = (DWORD)(bmfh.bfOffBits + bmih.biSizeImage);
            bmih.biBitCount     = 4;
            bmih.biClrUsed      = 16L;
            bmih.biClrImportant = 16L;

            MGUI_RWwrite (fp, &bmfh.bfType, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfSize, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfReserved1, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfReserved2, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfOffBits, sizeof (DWORD), 1);
            
            MGUI_RWwrite (fp, &bmih.biSize, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biWidth, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biHeight, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biPlanes, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmih.biBitCount, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmih.biCompression, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biSizeImage, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biXPelsPerMeter, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biYPelsPerMeter, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biClrUsed, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biClrImportant, sizeof (DWORD), 1);

            for (i = 0; i < 16; i++) {
                RGBQUAD rgbquad;
                rgbquad.rgbRed = pal [i].r;
                rgbquad.rgbBlue = pal [i].b;
                rgbquad.rgbGreen = pal [i].g;
                MGUI_RWwrite (fp, &rgbquad, sizeof (char), sizeof (RGBQUAD));
            }
            
            for (i = bmp->h  - 1; i >= 0; i--) {
                bmpGet16CScanline (bmp->bits + i * bmp->pitch, scanline, bmp->w);
                MGUI_RWwrite (fp, scanline, sizeof (char), scanlinebytes);
            }
        break;

        case 8:
            scanlinebytes       = bmih.biWidth;
            scanlinebytes       = ((scanlinebytes + 3)>>2)<<2;

#ifdef HAVE_ALLOCA
            if (!(scanline = alloca (scanlinebytes))) return ERR_BMP_MEM;
#else
            if (!(scanline = malloc (scanlinebytes))) return ERR_BMP_MEM;
#endif
            memset (scanline, 0, scanlinebytes);

            bmih.biSizeImage    = bmih.biHeight*scanlinebytes;
            bmfh.bfOffBits      = SIZEOF_BMFH + SIZEOF_BMIH
                                    + (SIZEOF_RGBQUAD<<8);
            bmfh.bfSize         = bmfh.bfOffBits + bmih.biSizeImage;
            bmih.biBitCount     = 8;
            bmih.biClrUsed      = 256;
            bmih.biClrImportant = 256;

            MGUI_RWwrite (fp, &bmfh.bfType, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfSize, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfReserved1, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfReserved2, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfOffBits, sizeof (DWORD), 1);

            MGUI_RWwrite (fp, &bmih.biSize, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biWidth, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biHeight, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biPlanes, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmih.biBitCount, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmih.biCompression, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biSizeImage, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biXPelsPerMeter, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biYPelsPerMeter, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biClrUsed, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biClrImportant, sizeof (DWORD), 1);
            
            for (i = 0; i < 256; i++) {
                RGBQUAD rgbquad;
                rgbquad.rgbRed = pal [i].r;
                rgbquad.rgbBlue = pal [i].b;
                rgbquad.rgbGreen = pal [i].g;
                MGUI_RWwrite (fp, &rgbquad, sizeof (char), sizeof (RGBQUAD));
            }
            
            for (i = bmp->h - 1; i >= 0; i--) {
                bmpGet256CScanline (bmp->bits + bmp->pitch * i, scanline, bmp->w);
                MGUI_RWwrite (fp, scanline, sizeof (char), scanlinebytes);
            }
        break;

        default:
            scanlinebytes       = bmih.biWidth*3;
            scanlinebytes       = ((scanlinebytes + 3)>>2)<<2;

#ifdef HAVE_ALLOCA
            if (!(scanline = alloca (scanlinebytes))) return ERR_BMP_MEM;
#else
            if (!(scanline = malloc (scanlinebytes))) return ERR_BMP_MEM;
#endif
            memset (scanline, 0, scanlinebytes);

            bmih.biSizeImage    = bmih.biHeight*scanlinebytes;
            bmfh.bfOffBits      = SIZEOF_BMFH + SIZEOF_BMIH;
            bmfh.bfSize         = bmfh.bfOffBits + bmih.biSizeImage;
            bmih.biBitCount     = 24;

            MGUI_RWwrite (fp, &bmfh.bfType, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfSize, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfReserved1, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfReserved2, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmfh.bfOffBits, sizeof (DWORD), 1);
            
            MGUI_RWwrite (fp, &bmih.biSize, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biWidth, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biHeight, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biPlanes, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmih.biBitCount, sizeof (WORD), 1);
            MGUI_RWwrite (fp, &bmih.biCompression, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biSizeImage, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biXPelsPerMeter, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biYPelsPerMeter, sizeof (LONG), 1);
            MGUI_RWwrite (fp, &bmih.biClrUsed, sizeof (DWORD), 1);
            MGUI_RWwrite (fp, &bmih.biClrImportant, sizeof (DWORD), 1);

            for (i = bmp->h - 1; i >= 0; i--) {
                bmpGetHighCScanline (bmp->bits + i * bmp->pitch, scanline, 
                                bmp->w, bpp, bmp->depth);
                MGUI_RWwrite (fp, scanline, sizeof (char), scanlinebytes);
            }
        break;
    }

#ifndef HAVE_ALLOCA
    free (scanline);
#endif

    return ERR_BMP_OK;
}

#endif
#endif

