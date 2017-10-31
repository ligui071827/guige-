/*
 * ifxui_porting_image_jpeg.h
 *
 *  Created on: Oct 22, 2010
 *      Author: inflexion
 */

#ifndef IFXUI_PORTING_IMAGE_JPEG_H_
#define IFXUI_PORTING_IMAGE_JPEG_H_

#include    "inflexionui/engine/ifxui_porting.h"
#include    "inflexionui/engine/ifxui_engine.h"

IFX_RETURN_STATUS IFXP_Image_JPEG_Open(IFX_IMAGE_INFO *image_info,
									 void **image_descriptor,
									 char const* filename);

IFX_RETURN_STATUS IFXP_Image_JPEG_Get_Data(IFX_IMAGE_INFO *image_info,
											void *image_descriptor,
											  IFX_UINT32 *length,
											  void **image_data);

IFX_RETURN_STATUS IFXP_Image_JPEG_Release_Data(IFX_IMAGE_INFO *image_info,
											void *image_descriptor,
											  void *image_data);

IFX_RETURN_STATUS IFXP_Image_JPEG_Close(IFX_IMAGE_INFO *image_info,
											void *image_descriptor);

#endif /* IFXUI_PORTING_IMAGE_JPEG_H_ */
