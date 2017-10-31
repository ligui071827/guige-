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
*       ifxui_porting_image_jpeg.c
*
*   COMPONENT
*
*       Inflexion UI Porting Layer.
*
*   DESCRIPTION
*
*       Inflexion UI Engine Porting Layer JPEG Image API Implementation.
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

#ifdef IFXP_JPEG_SUPPORT

#include    "inflexionui/porting/inc/ifxui_porting_image_jpeg.h"
#include    <jpeglib.h>

//#define JPG_DEBUG
#ifdef JPG_DEBUG
    #define dprintf(format, ...) printf("[JPG]" format, ##__VA_ARGS__)
#else
    #define dprintf(format, ...)
#endif

#define     PL_MAX_FILE_LENGTH              1024
#define     PL_IMAGE_MAX_EXTENSION_LENGTH   10
#define     INPUT_BUFFER_SIZE   4096


typedef struct _tagIFX_JPEG_SOURCE_MGR
{
    struct jpeg_source_mgr base;
    IFXE_FILE fileHandle;
    unsigned char buffer[4096];
} IFX_JPEG_SOURCE_MGR;

typedef struct _tagIFX_JPEG_INFO
{
    struct jpeg_decompress_struct decode_info;
    struct jpeg_error_mgr   err_mgr;
    IFX_JPEG_SOURCE_MGR     src_mgr;
    IFX_RETURN_STATUS       status;
    IFX_BOOL                completed;
} IFX_JPEG_INFO;

static void init_source(j_decompress_ptr decode_info)
{
    IFX_JPEG_SOURCE_MGR *src_mgr=(IFX_JPEG_SOURCE_MGR*)decode_info->src;

    src_mgr->base.bytes_in_buffer=0;
    src_mgr->base.next_input_byte=NULL;
}
static boolean fill_input_buffer(j_decompress_ptr decode_info)
{
    IFX_JPEG_SOURCE_MGR *src_mgr = (IFX_JPEG_SOURCE_MGR *)decode_info->src;
    IFX_UINT32 bytes_read=0;
    if(src_mgr->fileHandle==NULL)
    {
        return FALSE;
    }
    IFXE_File_Read(src_mgr->fileHandle,src_mgr->buffer,INPUT_BUFFER_SIZE,&bytes_read);
    src_mgr->base.bytes_in_buffer=bytes_read;
    src_mgr->base.next_input_byte=src_mgr->buffer;

    return (bytes_read > 0);
}
static void skip_input_data(j_decompress_ptr decode_info, long offset)
{
    IFX_JPEG_SOURCE_MGR *src_mgr = (IFX_JPEG_SOURCE_MGR *)decode_info->src;
    if(offset>src_mgr->base.bytes_in_buffer)
    {
        offset-=src_mgr->base.bytes_in_buffer;
        IFXE_File_Seek(src_mgr->fileHandle,offset,IFX_SEEK_CUR);
        fill_input_buffer(decode_info);
    }
    else
    {
        src_mgr->base.bytes_in_buffer-=offset;
        src_mgr->base.next_input_byte+=offset;
    }
}
static boolean resync_to_restart(j_decompress_ptr decode_info, int offset)
{

}
static void term_source(j_decompress_ptr decode_info)
{
}

typedef struct
{
    IFXP_FILE           platformFile;
}IFXE_INTERNAL_FILE;

int check_jpg(IFXE_INTERNAL_FILE *handle,char const* filename)
{
	FILE* fd = handle->platformFile;
	unsigned char magic[3];
	int ret;

	ret = fread(magic, 2, 1, fd);
	if(ret != 1)
	{
		return -1;
	}
	if (magic[0] != 0xFF || magic[1] != 0xD8)
	{
		char *defaultFile = getenv("IFX_JPG_DEFAULT");
		if(!getenv)
			return -1;
		fclose(fd);	/*close original file handle*/
		fd = fopen(defaultFile, "rb");
		if(!fd)
			return -1;
		handle->platformFile = fd;
	}
	else
	{
		fseek(fd, 0, SEEK_SET);
	}
	return 0;
}

IFX_RETURN_STATUS IFXP_Image_JPEG_Open(IFX_IMAGE_INFO *image_info,
                                     void **image_descriptor,
                                     char const* filename)
{
 dprintf("%s     %d\n",__func__,__LINE__);
    IFX_RETURN_STATUS status=IFX_ERROR;
    IFXE_FILE fileHandle;

    char temp_fileName[PL_MAX_FILE_LENGTH];

    if(lc_strlen(filename)==0)
        return IFX_ERROR;

    char *sourceFileName=temp_fileName;

    lc_strcpy(sourceFileName,filename);

    if(IFXE_File_Open(&fileHandle,sourceFileName)==IFX_ERROR)
        return status;
    if(check_jpg(fileHandle,filename) != 0)
        return status;

    IFX_JPEG_INFO *jpeg_info;

    if(IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,sizeof(IFX_JPEG_INFO),(void**)&jpeg_info)==IFX_ERROR)
        return status;

 dprintf("%s     %d\n",__func__,__LINE__);
TRY_DEFAULT_PIC:
    jpeg_info->src_mgr.fileHandle=fileHandle;
    jpeg_info->src_mgr.base.init_source=init_source;
    jpeg_info->src_mgr.base.resync_to_restart=resync_to_restart;
    jpeg_info->src_mgr.base.skip_input_data=skip_input_data;
    jpeg_info->src_mgr.base.term_source=term_source;
    jpeg_info->src_mgr.base.fill_input_buffer=fill_input_buffer;
    jpeg_info->completed=IFX_FALSE;

    jpeg_info->decode_info.err = jpeg_std_error(&(jpeg_info->err_mgr));

 dprintf("%s     %d\n",__func__,__LINE__);
    jpeg_create_decompress(&jpeg_info->decode_info);

 dprintf("%s     %d\n",__func__,__LINE__);
    jpeg_info->decode_info.src=(struct jpeg_source_mgr *)&(jpeg_info->src_mgr);

 dprintf("%s     %d\n",__func__,__LINE__);
    jpeg_read_header(&jpeg_info->decode_info, TRUE);
    jpeg_start_decompress(&jpeg_info->decode_info);

 dprintf("%s     %d\n",__func__,__LINE__);
    image_info->width=jpeg_info->decode_info.output_width;
    image_info->height=jpeg_info->decode_info.output_height;
    image_info->length=image_info->width*jpeg_info->decode_info.output_components*(image_info->height);
    if(jpeg_info->decode_info.out_color_space == JCS_GRAYSCALE)
	image_info->length=image_info->width*3*(image_info->height);
    int color_type=jpeg_info->decode_info.out_color_space;
dprintf("w:%d, h:%d, bpp:%d, color_type:%d\n", image_info->width, image_info->height, jpeg_info->decode_info.output_components, jpeg_info->decode_info.out_color_space);
    status=IFX_SUCCESS;
    switch (color_type)
    {
        case JCS_RGB:
            image_info->type=IFX_IMAGE_TYPE_RGB_DATA;
        break;
        case JCS_GRAYSCALE:
            image_info->type=IFX_IMAGE_TYPE_RGB_DATA;
        break;
        case JCS_CMYK:
            sourceFileName = getenv("IFX_JPG_DEFAULT");
            if(sourceFileName == NULL || strlen(sourceFileName) == 0){
                IFXP_Image_Close(image_info);
                return IFX_ERROR;
            }
            IFXE_File_Close(fileHandle);
            if(IFXE_File_Open(&fileHandle,sourceFileName)==IFX_ERROR){
                IFXP_Image_Close(image_info);
                return IFX_ERROR;
            }
            goto TRY_DEFAULT_PIC;
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
        jpeg_info->status=IFX_SUCCESS;
        *image_descriptor=jpeg_info;
    }
 dprintf("%s     %d\n",__func__,__LINE__);
     return status;
}

IFX_RETURN_STATUS IFXP_Image_JPEG_Get_Data(IFX_IMAGE_INFO *image_info,
                                            void *image_descriptor,
                                            IFX_UINT32 *length,
                                            void **image_data)
{
 dprintf("%s     %d\n",__func__,__LINE__);
    IFX_JPEG_INFO *jpeg_info=NULL;
    IFXE_FILE fileHandle;
    int row_size=0;

    if(image_descriptor==NULL
            || image_info==NULL
            || length==NULL
            || image_data==NULL)
        return IFX_ERROR;

    jpeg_info=(IFX_JPEG_INFO*)image_descriptor;
    fileHandle=jpeg_info->src_mgr.fileHandle;

    if(fileHandle==NULL)
        return IFX_ERROR;

    row_size = image_info->length/image_info->height;

 dprintf("%s     %d\n",__func__,__LINE__);
    if(IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,image_info->length,image_data)==IFX_ERROR)
        return IFX_ERROR;

 dprintf("%s     %d\n",__func__,__LINE__);
    unsigned char *ptr=*image_data;
    if(jpeg_info->decode_info.out_color_space == JCS_GRAYSCALE){
 dprintf("%s     %d\n",__func__,__LINE__);
	    unsigned char *tbuf = malloc(row_size);
	    int i;
	    while (jpeg_info->decode_info.output_scanline < jpeg_info->decode_info.output_height) {
		    if(jpeg_read_scanlines(&jpeg_info->decode_info, &tbuf, 1)>0)
		    {
			    for(i = 0; i < image_info->width; i++){
#if 0
				    *(ptr + i*3 + 0) = *(tbuf+i)&0xe0;	
				    *(ptr + i*3 + 1) = *(tbuf+i)>>3&0x3<<6;	
				    *(ptr + i*3 + 2) = *(tbuf+i)&0x7<<5;	
#else
				    *(ptr + i*3 + 2) = *(tbuf+i);	
				    *(ptr + i*3 + 1) = *(tbuf+i);	
				    *(ptr + i*3 + 0) = *(tbuf+i);	
#endif
			    }
			    ptr+=row_size;
		    }
	    }
 dprintf("%s     %d\n",__func__,__LINE__);
	    free(tbuf);
    }
    else {
 dprintf("%s     %d\n",__func__,__LINE__);
	    while (jpeg_info->decode_info.output_scanline < jpeg_info->decode_info.output_height) {
 dprintf("%s     %d  %d %d\n",__func__,__LINE__,jpeg_info->decode_info.output_scanline , jpeg_info->decode_info.output_height);
		    if(jpeg_read_scanlines(&jpeg_info->decode_info, &ptr, 1)>0)
		    {
			    ptr+=row_size;
		    }
		    else
		    {
 				printf("%s     %d\n",__func__,__LINE__);
 				jpeg_info->decode_info.output_scanline += 1;
 				
		    }
	    }
 dprintf("%s     %d\n",__func__,__LINE__);
    }
    *length=image_info->length;
    jpeg_info->completed=IFX_TRUE;
 dprintf("%s     %d\n",__func__,__LINE__);
    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXP_Image_JPEG_Release_Data(IFX_IMAGE_INFO *image_info,
                                            void *image_descriptor,
                                              void *image_data)
{
 dprintf("%s     %d\n",__func__,__LINE__);
    if(image_data!=NULL)
    {
        if(IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL,image_data)==IFX_ERROR)
            return IFX_ERROR;

        return IFX_SUCCESS;
    }
 dprintf("%s     %d\n",__func__,__LINE__);
    return IFX_ERROR;
}

IFX_RETURN_STATUS IFXP_Image_JPEG_Close(IFX_IMAGE_INFO *image_info,
                                            void *image_descriptor)
{
 dprintf("%s     %d\n",__func__,__LINE__);
    IFXE_FILE fileHandle;
    IFX_JPEG_INFO *jpeg_info;

    if(image_descriptor==NULL
            || image_info==NULL)
        return IFX_ERROR;

    jpeg_info=(IFX_JPEG_INFO*)image_descriptor;
    fileHandle=jpeg_info->src_mgr.fileHandle;

    if(fileHandle==NULL)
        return IFX_ERROR;

    if(jpeg_info->completed==IFX_FALSE)
    {
        jpeg_abort_decompress(&jpeg_info->decode_info);
    }
    else
    {
        jpeg_finish_decompress(&jpeg_info->decode_info);
    }
    jpeg_destroy_decompress(&jpeg_info->decode_info);


    if(IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL,jpeg_info)==IFX_ERROR)
            return IFX_ERROR;

    if(IFXE_File_Close(fileHandle)==IFX_ERROR)
        return IFX_ERROR;

 dprintf("%s     %d\n",__func__,__LINE__);
    return IFX_SUCCESS;
}



#endif // IFXP_JPEG_SUPPORT

