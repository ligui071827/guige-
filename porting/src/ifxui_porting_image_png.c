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
*       ifxui_porting_image_png.c
*
*   COMPONENT
*
*       Inflexion UI Porting Layer.
*
*   DESCRIPTION
*
*       Inflexion UI Engine Porting Layer PNG Image API Implementation.
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

#ifdef IFXP_PNG_SUPPORT

#include    "inflexionui/porting/inc/ifxui_porting_image_png.h"
#include    <png.h>

#define     PL_MAX_FILE_LENGTH              256
#define     PL_IMAGE_MAX_EXTENSION_LENGTH   10

typedef struct _tagIFX_PNG_INFO
{
    png_structp         png_ptr;
    png_infop           info_ptr;
    png_infop           end_info;
    IFXE_FILE           fileHandle;
    IFX_RETURN_STATUS   status;
} IFX_PNG_INFO;




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

IFX_RETURN_STATUS IFXP_Image_PNG_Open(IFX_IMAGE_INFO *image_info,
                                     void **image_descriptor,
                                     char const* filename)
{
    IFX_RETURN_STATUS status=IFX_ERROR;
    IFXE_FILE fileHandle;
    IFX_PNG_INFO *png_info;

    if(IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,sizeof(IFX_PNG_INFO),(void**)&png_info)==IFX_ERROR)
        return status;

    if(IFXE_File_Open(&fileHandle,filename)==IFX_ERROR)
            return status;

    png_structp png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING,NULL,
                                                    png_error_fn,
                                                    png_warning_fn,
                                                    NULL,memory_malloc_fn,
                                                    memory_free_fn);
    if (!png_ptr)
        return status;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr,
           (png_infopp)NULL, (png_infopp)NULL);
        return status;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr,
          (png_infopp)NULL);
        return IFX_ERROR;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr,
           (png_infopp)NULL);
        return (status);
    }

    png_set_read_fn(png_ptr,fileHandle,ifx_png_read_data);

    png_read_info(png_ptr, info_ptr);

    // compress 16-bit channels to 8-bit
    if(info_ptr->bit_depth > 8)
        png_set_strip_16(png_ptr);

    // expand palette channels to 8-bit
    if (info_ptr->bit_depth < 8 &&
            info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
        png_set_expand(png_ptr);

    /* expand transperancy to alpha channels */
    if (info_ptr->valid & PNG_INFO_tRNS)
        png_set_expand(png_ptr);

    // update info with results of any transformations if applied.
    png_read_update_info(png_ptr, info_ptr);

     image_info->width=png_get_image_width(png_ptr,info_ptr);
     image_info->height=png_get_image_height(png_ptr,info_ptr);
     image_info->length=png_get_rowbytes(png_ptr,info_ptr)*(image_info->height);
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

    if(status==IFX_ERROR)
    {
        IFXP_Image_Close(image_info);
    }
    else
    {
        png_info->info_ptr=info_ptr;
        png_info->end_info=end_info;
        png_info->png_ptr=png_ptr;
        png_info->status=IFX_SUCCESS;
        png_info->fileHandle=fileHandle;
        *image_descriptor=png_info;
    }
     return status;
}

IFX_RETURN_STATUS IFXP_Image_PNG_Get_Data(IFX_IMAGE_INFO *image_info,
                                            void *image_descriptor,
                                            IFX_UINT32 *length,
                                            void **image_data)
{

    IFXE_FILE fileHandle;
    IFX_PNG_INFO *png_info;
    png_structp png_ptr;
    png_infop info_ptr;

    if(image_descriptor==NULL
            || image_info==NULL
            || length==NULL
            || image_data==NULL)
        return IFX_ERROR;

    png_info=(IFX_PNG_INFO*)image_descriptor;
    fileHandle=png_info->fileHandle;
    png_ptr=png_info->png_ptr;
    info_ptr=png_info->info_ptr;

    if(fileHandle==NULL
            || png_ptr==NULL
            || info_ptr==NULL)
        return IFX_ERROR;

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
    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXP_Image_PNG_Release_Data(IFX_IMAGE_INFO *image_info,
                                            void *image_descriptor,
                                              void *image_data)
{
    if(image_data!=NULL)
    {
        if(IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL,image_data)==IFX_ERROR)
            return IFX_ERROR;

        return IFX_SUCCESS;
    }
    return IFX_ERROR;
}

IFX_RETURN_STATUS IFXP_Image_PNG_Close(IFX_IMAGE_INFO *image_info,
                                            void *image_descriptor)
{
    IFXE_FILE fileHandle;
    IFX_PNG_INFO *png_info;
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;

    if(image_descriptor==NULL
            || image_info==NULL)
        return IFX_ERROR;

    png_info=(IFX_PNG_INFO*)image_descriptor;
    fileHandle=png_info->fileHandle;
    png_ptr=png_info->png_ptr;
    info_ptr=png_info->info_ptr;
    end_info=png_info->end_info;
    if(fileHandle==NULL
            || png_ptr==NULL
            || info_ptr==NULL
            || end_info==NULL)
        return IFX_ERROR;

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    if(IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL,png_info)==IFX_ERROR)
        return IFX_ERROR;

    if(IFXE_File_Close(fileHandle)==IFX_ERROR)
        return IFX_ERROR;

    return IFX_SUCCESS;
}

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

#endif // IFXP_PNG_SUPPORT

