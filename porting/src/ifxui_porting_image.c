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
*       ifxui_porting_image.c
*
*   COMPONENT
*
*       Inflexion UI Porting Layer.
*
*   DESCRIPTION
*
*       Inflexion UI Engine Porting Layer Common Image API Implementation.
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

/* Inflexion UI Engine includes. */
#include    "inflexionui/engine/ifxui_porting.h"
#include    "inflexionui/engine/ifxui_engine.h"

/* Porting Layer configuration */
#include    "ifxui_porting_cfg.h"

/* Includes for custom handlers */
#include    "inflexionui/porting/inc/ifxui_porting_image_dds.h"
#ifdef IFXP_PNG_SUPPORT
    #include    "inflexionui/porting/inc/ifxui_porting_image_png.h"
#endif

#ifdef IFXP_JPEG_SUPPORT
    #include    "inflexionui/porting/inc/ifxui_porting_image_jpeg.h"
#endif

#define     PL_IMAGE_MAX_EXTENSION_LENGTH   10

typedef enum _ifxp_image_internal_format
{
    IFX_IMAGE_INTERNAL_FORMAT_UNSUPPORTED = 0,
    IFX_IMAGE_INTERNAL_FORMAT_DDS,
#ifdef IFXP_PNG_SUPPORT
    IFX_IMAGE_INTERNAL_FORMAT_PNG,
#endif

#ifdef IFXP_JPEG_SUPPORT
    IFX_IMAGE_INTERNAL_FORMAT_JPEG,
#endif

    IFX_IMAGE_INTERNAL_FORMAT_BMP, //xg++
} IFX_IMAGE_INTERNAL_FORMAT;

typedef struct _ifxp_image_internal_session
{
    IFX_IMAGE_INTERNAL_FORMAT format;
    void*                     image_descriptor;

} IFX_IMAGE_INTERNAL_SESSION;

/* Declaration of helper functions */
static void extractFileExtension(const char *fileName, char *fileExtension, int len);

IFX_RETURN_STATUS IFXP_Image_Open (IFX_IMAGE_INFO *image_info,const char *image_file)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    IFX_IMAGE_INTERNAL_FORMAT format = IFX_IMAGE_INTERNAL_FORMAT_UNSUPPORTED;
    char fileExtension[PL_IMAGE_MAX_EXTENSION_LENGTH] = "";

    /* If not a valid input, return right from here */
    if (!image_info)
    {
        return status;
    }

    /* First of all extract the file extension */
    extractFileExtension(image_file, fileExtension, PL_IMAGE_MAX_EXTENSION_LENGTH);

    if(lc_strlen(fileExtension) == 0)
    {
        return status;
    }

    if(lc_strcmpi(fileExtension, "DDS") == 0)
    {
        format = IFX_IMAGE_INTERNAL_FORMAT_DDS;
    }
#ifdef IFXP_PNG_SUPPORT
    else if(lc_strcmpi(fileExtension, "PNG") == 0)
    {
        format = IFX_IMAGE_INTERNAL_FORMAT_PNG;
    }
#endif

#ifdef IFXP_JPEG_SUPPORT
    else if(lc_strcmpi(fileExtension, "JPG") == 0 || lc_strcmpi(fileExtension, "JPEG") == 0)
    {
        format = IFX_IMAGE_INTERNAL_FORMAT_JPEG;
    }
#endif

    //xg add start
    else if(lc_strcmpi(fileExtension, "BMP") == 0)
    {
        format = IFX_IMAGE_INTERNAL_FORMAT_BMP;
    }
    //xg add end 

    /* Proceed if a supported format is encountered */
    if(format != IFX_IMAGE_INTERNAL_FORMAT_UNSUPPORTED)
    {
        IFX_IMAGE_INTERNAL_SESSION *session = NULL;

        if(IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,
                             sizeof(IFX_IMAGE_INTERNAL_SESSION),
                             (void **)&session) == IFX_SUCCESS)
        {
            /* Update session info */
            session->format = format;
            session->image_descriptor = NULL;

            /* Set it to be our session data for later referral */
            image_info->internal = session;

            /* Assume we have no error condition prior to invoking custom handler */
            status = IFX_SUCCESS;

            /* Invoke format-specific handler, if available! */
            switch (session->format)
            {
                case IFX_IMAGE_INTERNAL_FORMAT_DDS:
                {
                    /* Invoke DDS handler */
                    status = IFXP_Image_DDS_Open (image_info,
                                                  image_file,
                                                  &session->image_descriptor);
                }
                break;

#ifdef IFXP_PNG_SUPPORT
                case IFX_IMAGE_INTERNAL_FORMAT_PNG:
                {
                    status = IFXP_Image_PNG_Open(image_info,&session->image_descriptor,image_file);
                }
                break;
#endif

#ifdef IFXP_JPEG_SUPPORT
                case IFX_IMAGE_INTERNAL_FORMAT_JPEG:
                {
                    status = IFXP_Image_JPEG_Open(image_info,&session->image_descriptor,image_file);
                }
                break;
#endif

//xg add start
                case IFX_IMAGE_INTERNAL_FORMAT_BMP:
                {
    printf("\033[31m%s:%d %s\033[0m\n", __func__, __LINE__,image_file);
                    status = IFXP_Image_BMP_Open(image_info,&session->image_descriptor,image_file);
    printf("\033[31m%s:status %d %d\033[0m\n", __func__, __LINE__,status);
                }
                break;
//xg add end

                default:
                {
                    /* Unsupported format */
                    status = IFX_ERROR;
                }
                break;
            }
        }
    }

    /* We have an error condition */
    if (status != IFX_SUCCESS)
    {
        /* Free resources before returning */
        if (image_info->internal)
        {
            IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, image_info->internal);
            image_info->internal = NULL;
        }
    }

    return status;
}

IFX_RETURN_STATUS IFXP_Image_Get_Data(IFX_IMAGE_INFO *image_info,
                                      IFX_UINT32 level,
                                      IFX_UINT32 *length,
                                      void **image_data)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    IFX_IMAGE_INTERNAL_FORMAT format = IFX_IMAGE_INTERNAL_FORMAT_UNSUPPORTED;

    if(image_info && image_info->internal)
    {
        IFX_IMAGE_INTERNAL_SESSION *session = (IFX_IMAGE_INTERNAL_SESSION *) image_info->internal;
        format = session->format;

        /* Proceed if a supported format is encountered */
        if(format != IFX_IMAGE_INTERNAL_FORMAT_UNSUPPORTED)
        {
            /* Invoke format-specific handler, if available! */
            switch (session->format)
            {
                case IFX_IMAGE_INTERNAL_FORMAT_DDS:
                {
                    /* Invoke DDS handler */
                    status = IFXP_Image_DDS_Get_Data (image_info,
                                                      level,
                                                      length,
                                                      image_data,
                                                      session->image_descriptor);
                }
                break;

#ifdef IFXP_PNG_SUPPORT
                case IFX_IMAGE_INTERNAL_FORMAT_PNG:
                {
                    status = IFXP_Image_PNG_Get_Data (image_info,
                                                      session->image_descriptor,
                                                      length,
                                                      image_data);
                }
                break;
#endif


#ifdef IFXP_JPEG_SUPPORT
                case IFX_IMAGE_INTERNAL_FORMAT_JPEG:
                {
                    status = IFXP_Image_JPEG_Get_Data (image_info,
                                                      session->image_descriptor,
                                                      length,
                                                      image_data);
                }
                break;
#endif

//xg add start
                case IFX_IMAGE_INTERNAL_FORMAT_BMP:
                {
                    status = IFXP_Image_BMP_Get_Data (image_info,
                                                      session->image_descriptor,
                                                      length,
                                                      image_data);
    printf("\033[31m%s:status %d %d\033[0m\n", __func__, __LINE__,status);
                }
                break;
//xg add end

                default:
                {
                    /* Unsupported format */
                    status = IFX_ERROR;
                }
                break;
            }

            status = IFX_SUCCESS;
        }
    }

    return status;
}

IFX_RETURN_STATUS IFXP_Image_Release_Data (IFX_IMAGE_INFO *image_info, void *image_data)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    IFX_IMAGE_INTERNAL_FORMAT format = IFX_IMAGE_INTERNAL_FORMAT_UNSUPPORTED;

    if(image_info && image_info->internal)
    {
        IFX_IMAGE_INTERNAL_SESSION *session = (IFX_IMAGE_INTERNAL_SESSION *) image_info->internal;
        format = session->format;

        /* Proceed if a supported format is encountered */
        if(format != IFX_IMAGE_INTERNAL_FORMAT_UNSUPPORTED)
        {
            /* Invoke format-specific handler, if available! */
            switch (session->format)
            {
                case IFX_IMAGE_INTERNAL_FORMAT_DDS:
                {
                    /* Invoke DDS handler */
                    status = IFXP_Image_DDS_Release_Data (image_info, image_data);
                }
                break;

#ifdef IFXP_PNG_SUPPORT
                case IFX_IMAGE_INTERNAL_FORMAT_PNG:
                {
                    status=IFXP_Image_PNG_Release_Data(image_info,session->image_descriptor,image_data);
                }
                break;
#endif

#ifdef IFXP_JPEG_SUPPORT
                case IFX_IMAGE_INTERNAL_FORMAT_JPEG:
                {
                    status=IFXP_Image_JPEG_Release_Data(image_info,session->image_descriptor,image_data);
                }
                break;
#endif

//xg add start
                case IFX_IMAGE_INTERNAL_FORMAT_BMP:
                {
                    status=IFXP_Image_BMP_Release_Data(image_info,session->image_descriptor,image_data);
                }
                break;
//xg add end

                default:
                {
                    /* Unsupported format */
                    status = IFX_ERROR;
                }
                break;
            }

            status = IFX_SUCCESS;
        }
    }

    return status;
}

IFX_RETURN_STATUS IFXP_Image_Close(IFX_IMAGE_INFO *image_info)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    IFX_IMAGE_INTERNAL_FORMAT format = IFX_IMAGE_INTERNAL_FORMAT_UNSUPPORTED;

    if(image_info && image_info->internal)
    {
        IFX_IMAGE_INTERNAL_SESSION *session = (IFX_IMAGE_INTERNAL_SESSION *) image_info->internal;
        format = session->format;

        /* Proceed if a supported format is encountered */
        if(format != IFX_IMAGE_INTERNAL_FORMAT_UNSUPPORTED)
        {
            /* Invoke format-specific handler, if available! */
            switch (session->format)
            {
                case IFX_IMAGE_INTERNAL_FORMAT_DDS:
                {
                    /* Invoke DDS handler */
                    status = IFXP_Image_DDS_Close (image_info, session->image_descriptor);
                }
                break;


#ifdef IFXP_PNG_SUPPORT
                case IFX_IMAGE_INTERNAL_FORMAT_PNG:
                {
                    status = IFXP_Image_PNG_Close(image_info, session->image_descriptor);
                }
                break;
#endif

#ifdef IFXP_JPEG_SUPPORT
                case IFX_IMAGE_INTERNAL_FORMAT_JPEG:
                {
                    status = IFXP_Image_JPEG_Close(image_info, session->image_descriptor);
                }
                break;
#endif

//xg add start
                case IFX_IMAGE_INTERNAL_FORMAT_BMP:
                {
                    status = IFXP_Image_BMP_Close(image_info, session->image_descriptor);
                }
                break;
//xg add end

                default:
                {
                    /* Unsupported format */
                    status = IFX_ERROR;
                }
                break;
            }
        }
        //IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, session);
        //session = NULL;
        //session = NULL doesn't mean image_info->internal = NULL.
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, image_info->internal);
        image_info->internal = NULL;
    }

    return status;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_Image_Check_Extension
*
*   DESCRIPTION
*
*       Check whether we support image file or not
*
*   INPUTS
*
*       image_file          - Image file to check
*
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS IFXP_Image_Check_Extension (const char *image_file)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    char fileExtension[PL_IMAGE_MAX_EXTENSION_LENGTH] = "";

    /* If not a valid input, return right from here */
    if (!image_file)
    {
        return status;
    }

    /* First of all extract the file extension */
    extractFileExtension(image_file, fileExtension, PL_IMAGE_MAX_EXTENSION_LENGTH);

    if(lc_strlen(fileExtension) == 0)
    {
        return status;
    }

    /* We support below format only */
    if(lc_strcmpi(fileExtension, "DDS") == 0)
    {
        status = IFX_SUCCESS;
    }
#ifdef IFXP_JPEG_SUPPORT
    else if(lc_strcmpi(fileExtension, "JPG") == 0 || lc_strcmpi(fileExtension, "JPEG") == 0)
    {
        status = IFX_SUCCESS;
    }
#endif
#ifdef IFXP_PNG_SUPPORT
    else if(lc_strcmpi(fileExtension, "PNG") == 0)
    {
        status = IFX_SUCCESS;
    }
#endif

//xg add start
    else if(lc_strcmpi(fileExtension, "BMP") == 0)
    {
        status = IFX_SUCCESS;
    }
//xg add end

    return status;
}

/* Helper functions */
static void extractFileExtension(const char *fileName, char *fileExtension, int len)
{
    const char * fileExt = NULL;

    if (!fileName || !fileExtension)
        return;

    fileExt = lc_strrchr(fileName, '.');

    if(fileExt != NULL)
    {
        lc_strncpy(fileExtension, fileExt + 1, len - 1);
        fileExtension[len-1] = '\0';
    }
}
