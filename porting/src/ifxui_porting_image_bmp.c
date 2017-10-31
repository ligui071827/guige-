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
*       ifxui_porting_image_bmp.c
*
*   COMPONENT
*
*       Inflexion UI Porting Layer.
*
*   DESCRIPTION
*
*       Inflexion UI Engine Porting Layer BMP Image API Implementation.
*
*   DATA STRUCTURES
*

*   FUNCTIONS
*

*   DEPENDENCIES
*
*       ifxui_porting.h
*       ifxui_engine.h
*
*************************************************************************/

/* Porting Layer configuration */
#include    "ifxui_porting_cfg.h"

//#ifdef IFXP_PNG_SUPPORT

#include    "inflexionui/porting/inc/ifxui_porting_image_bmp.h"
//#include    <png.h>
#include "winbmp.h"

#define     PL_MAX_FILE_LENGTH              256
#define     PL_IMAGE_MAX_EXTENSION_LENGTH   10


typedef struct _tagIFX_BMP_INFO
{
    MYBITMAP*           pmybmp;
    FILE*               fileHandle;
    IFX_RETURN_STATUS   status;
} IFX_BMP_INFO;

#if 0
// Memory functions for libpng
static png_voidp memory_malloc_fn(png_structp png_ptr,png_size_t size);
static void memory_free_fn(png_structp png_ptr, png_voidp ptr);

// I/O functions for libpng
static void ifx_png_read_data(png_structp png_ptr,png_bytep data, png_size_t length);
static void ifx_png_write_data(png_structp png_ptr,png_bytep data, png_size_t length);
static void ifx_png_flush_data(png_structp png_ptr);

// Error handling functions for libpng
static void png_error_fn(png_structp png_ptr,png_const_charp error_msg);
void png_warning_fn(png_structp png_ptr,png_const_charp warning_msg);
#endif

IFX_RETURN_STATUS IFXP_Image_BMP_Open(IFX_IMAGE_INFO *image_info,
                                     void **image_descriptor,
                                     char const* filename)
{
    dprintf("\033[31m%s:%d\033[0m\n", __func__, __LINE__);
    IFX_RETURN_STATUS status=IFX_ERROR;
    FILE* fileHandle;
    IFX_BMP_INFO *bmp_info;
    MYBITMAP *bmp;
#ifdef HAVE_PALETTE
    RGB pal[256];
#endif

    if(IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,sizeof(IFX_BMP_INFO),(void**)&bmp_info)==IFX_ERROR)
        return status;

    bmp = (MYBITMAP*)malloc(sizeof(MYBITMAP));
    if(bmp == NULL){
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL,bmp_info);
        return status;
    }

    fileHandle = fopen(filename, "rb");
    if(fileHandle==NULL){
        free(bmp);
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL,bmp_info);
        return status;
    }

#ifdef HAVE_PALETTE
    if(-1 == __mg_init_bmp(fileHandle, bmp, pal) ){
#else
    if(-1 == __mg_init_bmp(fileHandle, bmp, NULL) ){
#endif
        fclose(fileHandle);
        free(bmp);
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL,bmp_info);
        return status;
    }

    image_info->width = bmp->w;
    image_info->height= bmp->h;
    image_info->length = bmp->size;
    dprintf("\033[31m%s:%d, w:%d, h:%d, size:%d\033[0m\n", __func__, __LINE__, bmp->w, bmp->h, image_info->length);

    if(bmp->depth == 32){
        image_info->type = IFX_IMAGE_TYPE_RGBA_DATA;
        image_info->translucency = IFX_TRANSLUCENCY_IMAGE_NONOPAQUE;
        dprintf("depth:32, set IFX_IMAGE_TYPE_RGBA_DATA IFX_TRANSLUCENCY_IMAGE_NONOPAQUE\n");
    }
    else {
        image_info->type = IFX_IMAGE_TYPE_RGB_DATA;
    }
#if 0
     int color_type=png_get_color_type(png_ptr,info_ptr);
     status=IFX_SUCCESS;
     switch (color_type)
     {
        case PNG_COLOR_TYPE_RGB:
            image_info->type=IFX_IMAGE_TYPE_RGB_DATA;
            break;
        case PNG_COLOR_TYPE_RGBA:
            image_info->type=IFX_IMAGE_TYPE_RGBA_DATA;
            image_info->translucency = IFX_TRANSLUCENCY_IMAGE_NONOPAQUE;
            break;
        default:
            status=IFX_ERROR;
            break;
    }
#endif

     bmp_info->pmybmp = bmp;
     bmp_info->fileHandle=fileHandle;
     *image_descriptor=bmp_info;

     return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXP_Image_BMP_Get_Data(IFX_IMAGE_INFO *image_info,
                                            void *image_descriptor,
                                            IFX_UINT32 *length,
                                            void **image_data)
{
    dprintf("\033[31m%s\033[0m\n", __func__);
    FILE* fileHandle;
    IFX_BMP_INFO *bmp_info;

    if(image_descriptor==NULL
            || image_info==NULL
            || length==NULL
            || image_data==NULL)
        return IFX_ERROR;

    bmp_info=(IFX_BMP_INFO*)image_descriptor;
    fileHandle=bmp_info->fileHandle;
    if(fileHandle==NULL)
        return IFX_ERROR;

    *length = image_info->length;
    dprintf("%s image_info->length:%d\n", __func__, image_info->length);

    if (IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,*length,image_data)==IFX_ERROR)
        return IFX_ERROR;
    bmp_info->pmybmp->bits = *image_data;
    __mg_load_bmp(fileHandle, bmp_info->pmybmp);

#if 0
    int row_bytes=png_get_rowbytes(png_ptr,info_ptr);
    int height=png_get_image_height(png_ptr,info_ptr);
    png_bytep *row_pointers;

    *length=row_bytes*height;

    if (IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,*length,image_data)==IFX_ERROR)
        return IFX_ERROR;

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep)*height);
    void *ptr=*image_data;
    int i = 0;
    for(;i<height;++i)
    {
        row_pointers[i] = ptr;
        ptr=((char*)ptr)+ row_bytes;
    }
    png_read_image(png_ptr, row_pointers);
    free(row_pointers);
#endif
    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXP_Image_BMP_Release_Data(IFX_IMAGE_INFO *image_info,
                                            void *image_descriptor,
                                              void *image_data)
{
    dprintf("\033[31m%s\033[0m\n", __func__);
    if(image_data!=NULL)
    {
        if(IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL,image_data)==IFX_ERROR)
            return IFX_ERROR;

        return IFX_SUCCESS;
    }
    return IFX_ERROR;
}

IFX_RETURN_STATUS IFXP_Image_BMP_Close(IFX_IMAGE_INFO *image_info,
                                            void *image_descriptor)
{
    dprintf("\033[31m%s\033[0m\n", __func__);
    FILE* fileHandle;
    IFX_BMP_INFO *bmp_info;

    if(image_descriptor==NULL
            || image_info==NULL)
        return IFX_ERROR;

    bmp_info=(IFX_BMP_INFO*)image_descriptor;
    fileHandle=bmp_info->fileHandle;
    if(fileHandle==NULL)
        return IFX_ERROR;

    if(bmp_info->pmybmp != NULL)
        free(bmp_info->pmybmp);
    if(IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL,bmp_info)==IFX_ERROR)
        return IFX_ERROR;

    fclose(fileHandle);

    return IFX_SUCCESS;
}

#if 0
// Memory functions for libpng
static png_voidp memory_malloc_fn(png_structp png_ptr,png_size_t size)
{
    png_voidp ptr=NULL;

    if(IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,size,&ptr)==IFX_ERROR)
        ptr=NULL;
    return ptr;
}
static void memory_free_fn(png_structp png_ptr, png_voidp ptr)
{
    IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL,ptr);
}

// I/O functions for libpng
static void ifx_png_read_data(png_structp png_ptr,png_bytep data, png_size_t length)
{
    IFXE_FILE fileHandle = png_get_io_ptr(png_ptr);
    IFX_UINT32 bytes_read=0;
    if(fileHandle==NULL)
    {
        length=0;
        return;
    }
    IFXE_File_Read(fileHandle,data,length,&bytes_read);
}
static void ifx_png_write_data(png_structp png_ptr,png_bytep data, png_size_t length)
{
    // not supported
}
static void ifx_png_flush_data(png_structp png_ptr)
{
    // not supported
}

static void png_error_fn(png_structp png_ptr,png_const_charp error_msg)
{

}
void png_warning_fn(png_structp png_ptr,png_const_charp warning_msg)
{

}
#endif

//#endif // IFXP_BMP_SUPPORT

