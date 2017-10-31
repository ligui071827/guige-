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
*    FILE NAME
*
*        ifxui_porting_image_dds.c
*
*    COMPONENT
*
*        Inflexion UI Porting Layer DDS Image Unpacking Interface for Engine
*
*    DESCRIPTION
*
*        Inflexion UI Engine Porting API Image Common DDS Implementation.
*
*
*    FUNCTIONS
*
*    IFXP_Image_DDS_Open
*    IFXP_Image_DDS_Get_Data
*    IFXP_Image_DDS_Release_Data
*    IFXP_Image_DDS_Close
*
*    IFXP_Image_DDS_Validate_Header
*    IFXP_Image_DDS_Get_Dimension
*    IFXP_Image_DDS_Check_For_Power_Of_Two
*    IFXP_Image_DDS_Get_MipMap_Count
*    IFXP_Image_DDS_Get_Compression_Format
*    IFXP_Image_DDS_Get_Block_Info
*
*    DATA STRUCTURES
*
*    DDSPixelFormat
*    DDSCaps
*    DDSHeader10
*    DDSHeader
*
*    DEPENDENCIES
*
*    ifxui_porting_image_dds.h
*
*************************************************************************/

/* Includes */
#include    "inflexionui/porting/inc/ifxui_porting_image_dds.h"

#if !defined(MAKEFOURCC)
    #define MAKEFOURCC(ch0, ch1, ch2, ch3) \
    (   (ch0) | (ch1 << 8) | \
        (ch2 << 16) | (ch3 << 24 ) )
#endif

/* DDS Identifiers */
#define FOURCC_DDS                      MAKEFOURCC('D', 'D', 'S', ' ')
#define DDS_HEADER_SIZE                 124
#define DDS_HEADER_SIZE_EX              148 /* Includes DX10 header */

/* OpenGL ES 1.1/2.0 specific */

/* AMD Compressantor 1.50 */

/* ETC1 */
#define FOURCC_ETC1_AMD                 MAKEFOURCC('E', 'T', 'C', ' ')

/* ATC RGB */
#define FOURCC_ATC_AMD                  MAKEFOURCC('A', 'T', 'C', ' ')

/* ATC RGBA (Explicit Alpha) */
#define FOURCC_ATCA_AMD                 MAKEFOURCC('A', 'T', 'C', 'A')

/* ATC RGBA (Interpolated Alpha) */
#define FOURCC_ATCI_AMD                 MAKEFOURCC('A', 'T', 'C', 'I')

/* Imagination PowerVR PVRTexTool */
#define FOURCC_ETC1_PVR                 MAKEFOURCC('E', 'T', 'C', '0')
#define FOURCC_PVRTC2_PVR               MAKEFOURCC('P', 'T', 'C', '2')
#define FOURCC_PVRTC4_PVR               MAKEFOURCC('P', 'T', 'C', '4')

/* Block factor for each supported compression format */
#define BLOCK_FACTOR_ETC1               8
#define BLOCK_FACTOR_PVRTC4_PVR         8
#define BLOCK_FACTOR_PVRTC2_PVR         4
#define BLOCK_FACTOR_ATC_AMD            8
#define BLOCK_FACTOR_ATCA_AMD           16
#define BLOCK_FACTOR_ATCI_AMD           16

/* Minimum Data Size for each supported compression format */
#define MIN_DATA_SIZE_ETC1              8
#define MIN_DATA_SIZE_PVRTC4_PVR        32
#define MIN_DATA_SIZE_PVRTC2_PVR        32
#define MIN_DATA_SIZE_ATC_AMD           8
#define MIN_DATA_SIZE_ATCA_AMD          16
#define MIN_DATA_SIZE_ATCI_AMD          16

/* DDS Caps */
#define DDSD_CAPS                       0x00000001U
#define DDSD_PIXELFORMAT                0x00001000U
#define DDSD_WIDTH                      0x00000004U
#define DDSD_HEIGHT                     0x00000002U
#define DDSD_PITCH                      0x00000008U
#define DDSD_MIPMAPCOUNT                0x00020000U
#define DDSD_LINEARSIZE                 0x00080000U
#define DDSD_DEPTH                      0x00800000U

#define DDSCAPS_COMPLEX                 0x00000008U
#define DDSCAPS_TEXTURE                 0x00001000U
#define DDSCAPS_MIPMAP                  0x00400000U
#define DDSCAPS2_VOLUME                 0x00200000U
#define DDSCAPS2_CUBEMAP                0x00000200U

#define DDSCAPS2_CUBEMAP_POSITIVEX      0x00000400U
#define DDSCAPS2_CUBEMAP_NEGATIVEX      0x00000800U
#define DDSCAPS2_CUBEMAP_POSITIVEY      0x00001000U
#define DDSCAPS2_CUBEMAP_NEGATIVEY      0x00002000U
#define DDSCAPS2_CUBEMAP_POSITIVEZ      0x00004000U
#define DDSCAPS2_CUBEMAP_NEGATIVEZ      0x00008000U
#define DDSCAPS2_CUBEMAP_ALL_FACES      0x0000FC00U

/* DDS Pixel Formats */
#define DDPF_ALPHAPIXELS                0x00000001U
#define DDPF_ALPHA                      0x00000002U
#define DDPF_FOURCC                     0x00000004U
#define DDPF_RGB                        0x00000040U
#define DDPF_PALETTEINDEXED1            0x00000800U
#define DDPF_PALETTEINDEXED2            0x00001000U
#define DDPF_PALETTEINDEXED4            0x00000008U
#define DDPF_PALETTEINDEXED8            0x00000020U
#define DDPF_LUMINANCE                  0x00020000U
#define DDPF_ALPHAPREMULT               0x00008000U
#define DDPF_NORMAL                     0x80000000U

/* Internal Structures */

/* DDS Pixel Format */
typedef struct tagDDSPixelFormat
{
    IFX_UINT32 size;
    IFX_UINT32 flags;
    IFX_UINT32 fourcc;
    IFX_UINT32 bitcount;
    IFX_UINT32 rmask;
    IFX_UINT32 gmask;
    IFX_UINT32 bmask;
    IFX_UINT32 amask;

} DDSPixelFormat;

/* DDS Caps */
typedef struct tagDDSCaps
{
    IFX_UINT32 caps1;
    IFX_UINT32 caps2;
    IFX_UINT32 caps3;
    IFX_UINT32 caps4;

} DDSCaps;

/* DDS file header for DX10. */
typedef struct tagDDSHeader10
{
    IFX_UINT32 dxgiFormat;
    IFX_UINT32 resourceDimension;
    IFX_UINT32 miscFlag;
    IFX_UINT32 arraySize;
    IFX_UINT32 reserved;

} DDSHeader10;

/* DDS file header. */
typedef struct tagDDSHeader
{
    IFX_UINT32 fourcc;
    IFX_UINT32 size;
    IFX_UINT32 flags;
    IFX_UINT32 height;
    IFX_UINT32 width;
    IFX_UINT32 pitch;
    IFX_UINT32 depth;
    IFX_UINT32 mipmapcount;
    IFX_UINT32 reserved[11];

    DDSPixelFormat pf;
    DDSCaps caps;

    IFX_UINT32 notused;

    /* DDSHeader10 header10; */

} DDSHeader;

/* Declaration of helper functions */
IFX_RETURN_STATUS IFXP_Image_DDS_Validate_Header (DDSHeader dds_header);

IFX_RETURN_STATUS IFXP_Image_DDS_Get_Dimension (DDSHeader dds_header,
                                                IFX_UINT32 *image_width,
                                                IFX_UINT32 *image_height);

IFX_RETURN_STATUS IFXP_Image_DDS_Check_For_Power_Of_Two (IFX_UINT32 image_width,
                                                         IFX_UINT32 image_height);

IFX_RETURN_STATUS IFXP_Image_DDS_Get_MipMap_Count (DDSHeader dds_header,
                                                IFX_UINT32 *image_levels);

IFX_RETURN_STATUS IFXP_Image_DDS_Get_Compression_Format (DDSHeader dds_header,
                                                         IFX_UINT32 *compression_format,
                                                         IFX_IMAGE_TRANSLUCENCY *translucency);

IFX_RETURN_STATUS IFXP_Image_DDS_Get_Block_Info (IFX_UINT32 compression_format,
                                                 IFX_UINT32 *block_factor,
                                                 IFX_UINT32 *min_data_size);

/* Open */
IFX_RETURN_STATUS IFXP_Image_DDS_Open (IFX_IMAGE_INFO *image_info,
                                       const char *image_dds_file,
                                       void **internal)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    IFXE_FILE file_handle;
    IFX_UINT32 file_size = 0;
    IFX_UINT32 bytes_read = 0;
    DDSHeader dds_header;

    /* Initialize header */
    memset (&dds_header, 0, sizeof(DDSHeader));

    /* Open the DDS file with specified name */
    status = IFXE_File_Open(&file_handle, (const char *) image_dds_file);

    /* File successfully opened ... now proceed with the rest */
    if (status == IFX_SUCCESS)
    {
        /* Store the handle for later use in internal session info */
        *internal = file_handle;

        /* Determine File Size */
        status = IFXE_File_Size (file_handle, &file_size);

        if (file_size)
        {
            /* Read DDS file header */
            status = IFXE_File_Read (file_handle, &dds_header, sizeof(DDSHeader), &bytes_read);

            /* Make sure to check if there is DirectX10 header present before the payload data */
            if (dds_header.notused)
            {
                DDSHeader10 dds_header_10;

                /* Read the additional DX10 header */
                status = IFXE_File_Read (file_handle, &dds_header_10, sizeof(DDSHeader10), &bytes_read);
            }
        }
    }

    /* Make sure to validate the header */
    if ( (status == IFX_SUCCESS) &&
         (IFXP_Image_DDS_Validate_Header(dds_header) == IFX_SUCCESS) )
    {
        /* No problems so far, then extract the info from header */
        IFX_UINT32 image_width = 0;
        IFX_UINT32 image_height = 0;
        IFX_UINT32 image_levels = 0;
        IFX_UINT32 compression_format = 0;

        /* Get dimensions */
        status = IFXP_Image_DDS_Get_Dimension(dds_header, &image_width, &image_height);

        if (status == IFX_SUCCESS)
        {
            /* Get count for number of pre-mip-map levels */
            IFXP_Image_DDS_Get_MipMap_Count(dds_header, &image_levels);

            status = IFXP_Image_DDS_Get_Compression_Format (dds_header,
                                                            &compression_format,
                                                            &(image_info->translucency));
            if (status == IFX_SUCCESS)
            {
                /* Fill in the information into the image info structure */
                image_info->type = IFX_IMAGE_TYPE_OPENGL_COMPRESSED_TEXTURE;
                image_info->width = image_width;
                image_info->height = image_height;
                image_info->length = file_size - sizeof(DDSHeader);
                image_info->texture_format = compression_format;
                image_info->first_mipmap_level = 0;
                image_info->last_mipmap_level = (image_levels > 0) ? (image_levels - 1) : (0) ;
            }
        }
    }

    if (status == IFX_SUCCESS)
    {
        /* Before returing the info, make sure that DDS contains supported
           texture compression format and dimensional attributes are
           valid
        */
        status = ( (image_info->width < 1) ||
                   (image_info->height < 1) ||
                   (image_info->length == 0) ||
                   (image_info->texture_format == 0) );
    }

    return status;
}

/* Data retrieval */
IFX_RETURN_STATUS IFXP_Image_DDS_Get_Data (IFX_IMAGE_INFO *image_info,
                                           IFX_UINT32 level,
                                           IFX_UINT32 *bytes_read,
                                           void **image_data,
                                           void *internal)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;
    IFXE_FILE file_handle;
    IFX_UINT32 width = 0;
    IFX_UINT32 height = 0;
    IFX_UINT32 mipmap_size = 0;
    IFX_UINT32 block_factor = 0;
    IFX_UINT32 min_data_size = 0;

    if ( (image_info == NULL) ||
         (image_data == NULL) ||
         (image_info->internal == NULL) )
    {
        status = IFX_ERROR;
    }

    if (status == IFX_SUCCESS)
    {
        /* Get the file handle from the internal session data */
        file_handle = (IFXE_FILE) internal;

        /* Get block size for this compression format */
        IFXP_Image_DDS_Get_Block_Info (image_info->texture_format,
                                       &block_factor,
                                       &min_data_size);

        /* Calculate length for this mip-map level */
        width = image_info->width;
        height = image_info->height;

        /* Shift according to specified mip-map level */
        width >>= level;
        height >>= level;

        /* Calculate data size for this mip-map level */
        mipmap_size = block_factor * ((width + 3) >> 2) * ((height + 3) >> 2);

        /* Validate calculated data size against the minimum bound */
        mipmap_size = (mipmap_size < min_data_size) ?
                      (min_data_size) : (mipmap_size) ;

        /* Allocate the data buffer for this mip-map level */
        status = IFXP_Mem_Allocate (IFXP_MEM_EXTERNAL, mipmap_size, image_data);
    }

    /* Read the mip-map image contents */
    if (status == IFX_SUCCESS)
    {
        /* Reset before proceeding with read */
        *bytes_read = 0;

        /* Now read the image compressed data */
        status = IFXE_File_Read(file_handle, *image_data, mipmap_size, bytes_read);

        /* Make sure that data read is equal to mipmap size */
        if (*bytes_read != mipmap_size)
        {
            status = IFX_ERROR;
        }
    }

    return status;
}

/* Resource clean-up */
IFX_RETURN_STATUS IFXP_Image_DDS_Release_Data (IFX_IMAGE_INFO *image_info,
                                               void *image_data)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    /* Invalid pointer */
    if (image_data == NULL)
    {
        status = IFX_ERROR;
    }

    if (status == IFX_SUCCESS)
    {
        status = IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, image_data);
    }

    return status;
}

/* Close */
IFX_RETURN_STATUS IFXP_Image_DDS_Close (IFX_IMAGE_INFO *image_info,
                                        void *internal)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;
    IFXE_FILE file_handle;

    if ( (image_info == NULL) ||
         (image_info->internal == NULL) )
    {
        status = IFX_ERROR;
    }

    if (status == IFX_SUCCESS)
    {
        /* Get the file handle from the internal session data */
        file_handle = (IFXE_FILE) internal;

        /* Close the file */
        IFXE_File_Close (file_handle);
    }

    return status;
}

/* Helper functions */

/* Test validity of the DDS header */
IFX_RETURN_STATUS IFXP_Image_DDS_Validate_Header(DDSHeader dds_header)
{
    IFX_RETURN_STATUS status = IFX_ERROR;

    if (dds_header.fourcc == FOURCC_DDS)
    {
         if ( (dds_header.size == DDS_HEADER_SIZE) ||
              (dds_header.size == DDS_HEADER_SIZE_EX) )
        {
            status = IFX_SUCCESS;
        }
    }

    return status;
}

/* Get dimension information */
IFX_RETURN_STATUS IFXP_Image_DDS_Get_Dimension(DDSHeader dds_header, IFX_UINT32 *image_width, IFX_UINT32 *image_height)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if ( (!image_width) || (!image_height) )
    {
        status = IFX_ERROR;
        return status;
    }

    status = IFXP_Image_DDS_Check_For_Power_Of_Two (dds_header.width, dds_header.height);

    if(status == IFX_SUCCESS)
    {
        *image_width = dds_header.width;
        *image_height = dds_header.height;
    }
    else
    {
        *image_width = 0;
        *image_height = 0;
    }

    return status;
}

/* Checks whether the image dimensions are POT (Power-Of-2) */
IFX_RETURN_STATUS IFXP_Image_DDS_Check_For_Power_Of_Two (IFX_UINT32 image_width,
                                                         IFX_UINT32 image_height)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;
    IFX_UINT32 isPowerOfTwo = 0;

    /* Test width for power-of-2 */
    isPowerOfTwo = ((image_width != 0) && !(image_width & (image_width - 1)));

    /* Test height only if width is power-of-2 */
    if (isPowerOfTwo)
    {
        isPowerOfTwo = ((image_height != 0) && !(image_height & (image_height - 1)));
    }

    /* Return success only if we have width and height both being Power-Of-2 */
    status = (isPowerOfTwo) ? (IFX_SUCCESS) : (IFX_ERROR) ;

    return (status);
}

/* Get number of pre mip-map levels */
IFX_RETURN_STATUS IFXP_Image_DDS_Get_MipMap_Count(DDSHeader dds_header, IFX_UINT32 *image_levels)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (image_levels)
    {
        *image_levels = dds_header.mipmapcount;
    }

    return status;
}

IFX_RETURN_STATUS IFXP_Image_DDS_Get_Compression_Format (DDSHeader dds_header,
                                                         IFX_UINT32 *compression_format,
                                                         IFX_IMAGE_TRANSLUCENCY *translucency)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (!compression_format || !translucency)
    {
        status = IFX_ERROR;
        return status;
    }

    // Default to unsupported compression format
    *compression_format = 0;

#if defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20)

    switch (dds_header.pf.fourcc)
    {
        /* ETC1 */
        case FOURCC_ETC1_AMD:
        case FOURCC_ETC1_PVR:
        {
            if(IFXP_Check_GL_Extension("GL_OES_compressed_ETC1_RGB8_texture") == IFX_SUCCESS)
            {
                *compression_format = GL_ETC1_RGB8_OES;
            }
        }
        break;

        /* Imagination PowerVR */
        /* PVRTC 2BPP */
        case FOURCC_PVRTC2_PVR:
        {
            if(IFXP_Check_GL_Extension("GL_IMG_texture_compression_pvrtc") == IFX_SUCCESS)
            {
                *compression_format = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
            }
        }
        break;

        /* PVRTC 4BPP */
        case FOURCC_PVRTC4_PVR:
        {
            if(IFXP_Check_GL_Extension("GL_IMG_texture_compression_pvrtc") == IFX_SUCCESS)
            {
                *compression_format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
            }
        }
        break;

        /* AMD Adreno */
        /* ATC RGB */
        case FOURCC_ATC_AMD:
        {
            if(IFXP_Check_GL_Extension("GL_AMD_compressed_ATC_texture") == IFX_SUCCESS)
            {
                *compression_format = GL_ATC_RGB_AMD;
            }
        }
        break;

        /* ATCA (Explicit Alpha) */
        case FOURCC_ATCA_AMD:
        {
            if(IFXP_Check_GL_Extension("GL_AMD_compressed_ATC_texture") == IFX_SUCCESS)
            {
                *compression_format = GL_ATC_RGBA_EXPLICIT_ALPHA_AMD;
            }
        }
        break;

        /* ATCI (Interpolated Alpha) */
        case FOURCC_ATCI_AMD:
        {
            if(IFXP_Check_GL_Extension("GL_AMD_compressed_ATC_texture") == IFX_SUCCESS)
            {
                *compression_format = GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD;
            }
        }
        break;
    }

    if (*compression_format != 0)
        status = IFX_SUCCESS;

#endif /* defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20) */

    return status;
}

IFX_RETURN_STATUS IFXP_Image_DDS_Get_Block_Info (IFX_UINT32 compression_format,
                                                 IFX_UINT32 *block_factor,
                                                 IFX_UINT32 *min_data_size)
{
    IFX_RETURN_STATUS status = IFX_ERROR;

    if ( (!block_factor) || (!min_data_size) )
    {
        return status;
    }

    // Default to unsupported compression format
    *block_factor = 0;
    *min_data_size = 0;

#if defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20)

    switch (compression_format)
    {
        /* ETC1 */
        case GL_ETC1_RGB8_OES:
        {
            if(IFXP_Check_GL_Extension("GL_OES_compressed_ETC1_RGB8_texture") == IFX_SUCCESS)
            {
                *block_factor  = BLOCK_FACTOR_ETC1;
                *min_data_size = MIN_DATA_SIZE_ETC1;
            }
        }
        break;

        /* Imagination PowerVR */
        /* PVRTC 2BPP */
        case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
        case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
        {
            if(IFXP_Check_GL_Extension("GL_IMG_texture_compression_pvrtc") == IFX_SUCCESS)
            {
                *block_factor  = BLOCK_FACTOR_PVRTC2_PVR;
                *min_data_size = MIN_DATA_SIZE_PVRTC2_PVR;
            }
        }
        break;

        /* PVRTC 4BPP */
        case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
        case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
        {
            if(IFXP_Check_GL_Extension("GL_IMG_texture_compression_pvrtc") == IFX_SUCCESS)
            {
                *block_factor  = BLOCK_FACTOR_PVRTC4_PVR;
                *min_data_size = MIN_DATA_SIZE_PVRTC4_PVR;
            }
        }
        break;

        /* AMD Adreno */
        /* ATC RGB */
        case GL_ATC_RGB_AMD:
        {
            if(IFXP_Check_GL_Extension("GL_AMD_compressed_ATC_texture") == IFX_SUCCESS)
            {
                *block_factor  = BLOCK_FACTOR_ATC_AMD;
                *min_data_size = MIN_DATA_SIZE_ATC_AMD;
            }
        }
        break;

        /* ATCA (Explicit Alpha) */
        case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD:
        {
            if(IFXP_Check_GL_Extension("GL_AMD_compressed_ATC_texture") == IFX_SUCCESS)
            {
                *block_factor  = BLOCK_FACTOR_ATCA_AMD;
                *min_data_size = MIN_DATA_SIZE_ATCA_AMD;
            }
        }
        break;

        /* ATCI (Interpolated Alpha) */
        case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD:
        {
            if(IFXP_Check_GL_Extension("GL_AMD_compressed_ATC_texture") == IFX_SUCCESS)
            {
                *block_factor  = BLOCK_FACTOR_ATCI_AMD;
                *min_data_size = MIN_DATA_SIZE_ATCI_AMD;
            }
        }
        break;
    }

    if (*block_factor != 0 || *min_data_size != 0)
        status = IFX_SUCCESS;

#endif /* defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20) */

    return status;
}
