/************************************************************************
*
*               Copyright Mentor Graphics Corporation 2006
*                         All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
*************************************************************************

*************************************************************************
* FILE NAME
*
*        ifxui_porting_image_dds.h
*
* COMPONENT
*
*        Inflexion UI
*
* DESCRIPTION
*
*        Inflexion UI Engine Porting API for DDS handling
*
* DEPENDENCIES
*
*        ifxui_porting.h
*        ifxui_engine.h
*
************************************************************************/
#ifndef _IFXUI_PORTING_IMAGE_DDS_H_
#define _IFXUI_PORTING_IMAGE_DDS_H_

/* Includes */
#include    "inflexionui/engine/ifxui_porting.h"
#include    "inflexionui/engine/ifxui_engine.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Open */
IFX_RETURN_STATUS IFXP_Image_DDS_Open (IFX_IMAGE_INFO *image_info, 
                                       const char *image_dds_file,
									   void **internal);

/* Data retrieval */
IFX_RETURN_STATUS IFXP_Image_DDS_Get_Data (IFX_IMAGE_INFO *image_info, 
										   IFX_UINT32 level,
										   IFX_UINT32 *bytes_read,
										   void **image_data,
										   void *internal);
										   
/* Resource clean-up */
IFX_RETURN_STATUS IFXP_Image_DDS_Release_Data (IFX_IMAGE_INFO *image_info, 
										       void *image_data);
									
/* Close */
IFX_RETURN_STATUS IFXP_Image_DDS_Close (IFX_IMAGE_INFO *image_info, 
                                        void *internal);

#ifdef __cplusplus
}
#endif

#endif /* _IFXUI_PORTING_IMAGE_DDS_H_ */
