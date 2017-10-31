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

#include "inflexionui/engine/inc/LcAll.h"
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif

#ifdef LcTOwnerH
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCCustomBitmapFile> LcCCustomBitmapFile::create()
{
	LcTaOwner<LcCCustomBitmapFile> ref;
	ref.set(new LcCCustomBitmapFile);
	ref->construct();
	return ref;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCCustomBitmapFile::~LcCCustomBitmapFile()
{
	m_data.free();
}

LC_EXPORT bool LcCCustomBitmapFile::open(const LcTmString& fileName,
												int marginLeft,
												int marginRight,
												int marginTop,
												int marginBottom,
												int frameCount)
{
	if(fileName.isEmpty())
		return false;

	m_imageInfo.translucency = IFX_TRANSLUCENCY_IMAGE_OPAQUE;
	if(IFXP_Image_Open(&m_imageInfo, fileName.bufUtf8()) == IFX_ERROR)
		return false;

	m_size.height = m_imageInfo.height;
	m_size.width = m_imageInfo.width;
	
	m_frameCount = frameCount;
	
	// Bitmap margins
	m_marginBottom = marginBottom;
	m_marginLeft = marginLeft;
	m_marginRight = marginRight;
	m_marginTop = marginTop;
	
	// Decided about bitmap format
	switch (m_imageInfo.type)
	{
		case IFX_IMAGE_TYPE_RGBA_DATA:
			m_format = KFormatGraphicTranslucent;
			break;
		case IFX_IMAGE_TYPE_RGB_DATA:
			m_format = KFormatGraphicOpaque;
			break;
		case IFX_IMAGE_TYPE_OPENGL_COMPRESSED_TEXTURE:
			m_format = KFormatCompressedOpenGL;
			break;
		default:
			m_format=KFormatAny;
			break;
	}
	
	// Update compressed bitmap related information
	if (m_format == KFormatCompressedOpenGL)
	{
		setLevelCount(m_imageInfo.last_mipmap_level - m_imageInfo.first_mipmap_level + 1); 
		setLevelFirst(m_imageInfo.first_mipmap_level);
		setLevelLast(m_imageInfo.last_mipmap_level); 
		setOGLCompressionFormat(m_imageInfo.texture_format); 
	}

	return true;
}

// get Custom Bitmap data via custom handler in porting layer
LC_EXPORT bool LcCCustomBitmapFile::readData(void **readData, unsigned level, IFX_UINT32 *length, bool cacheData)
{
	void *image_data = NULL;

	if(readData == NULL)
		return false;

	if(IFXP_Image_Get_Data(&m_imageInfo, level, length, &image_data) == IFX_ERROR)
		return false;

	if(image_data == NULL)
		return false;

	*readData = image_data;
	
	if(cacheData)
	{
		if(m_data != NULL)
			m_data.free();

		m_data.alloc(*length);
		memcpy((LcTByte*)m_data, image_data, *length);
	}
	
	return true;
}

// release Custom Bitmap data via custom handler in porting layer
LC_EXPORT void LcCCustomBitmapFile::releaseData(void *data)
{
	if(data != NULL)
	{
		IFXP_Image_Release_Data(&m_imageInfo, data);
	}
}

// release Custom Bitmap data via custom handler in porting layer
LC_EXPORT bool LcCCustomBitmapFile::close()
{
	return (IFXP_Image_Close(&m_imageInfo) == IFX_SUCCESS);
}

// Check whether extension is supported or not 
LC_EXPORT bool LcCCustomBitmapFile::checkExtension(const LcTmString& fileName)
{
	if(fileName.isEmpty())
		return false;

	if(IFXP_Image_Check_Extension(fileName.bufUtf8()) == IFX_ERROR)
		return false;
	return true;
}
