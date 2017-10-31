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


#ifdef IFX_USE_PLUGIN_ELEMENTS

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPluginElementBufferedView> NdhsCPluginElementBufferedView::create(
														NdhsCPlugin::NdhsCPluginHElement* w,
														IFX_ELEMENT_MODE elementMode)
{
	LcTaOwner<NdhsCPluginElementBufferedView> ref;
	ref.set(new NdhsCPluginElementBufferedView(w, elementMode));
	ref->construct();

	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPluginElementBufferedView::NdhsCPluginElementBufferedView(
														NdhsCPlugin::NdhsCPluginHElement* w,
														IFX_ELEMENT_MODE elementMode)
							 : NdhsCPluginElementView(w)
{
	m_mode = elementMode;
	m_repaint = true;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementBufferedView::construct()
{
	// Create the buffer rendering context on the heap
	m_context.alloc(1);
	lc_memset((void*)m_context, 0, sizeof(IFX_BUFFERED_RENDER_CONTEXT));

	// create initial buffer
	loadResources();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementBufferedView::attachWidget(LcwCPlugin* pWidget)
{
	if (pWidget && m_image && m_context)
	{
		// Attach image to widget
		pWidget->setImage(m_image.ptr());
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementBufferedView::detachWidget(LcwCPlugin* pWidget)
{
	if (!pWidget || !m_image)
		return;

	if (pWidget->getImage() == m_image.ptr())
	{
		// detach image from widget
		pWidget->setImage(NULL);
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPluginElementBufferedView::~NdhsCPluginElementBufferedView()
{
	if (m_image)
	{
		if(m_mode==IFX_MODE_ELEMENT_BUFFERED_NORMAL)
			m_image->releaseImageData();

		m_image.destroy();
	}

	// Free render context
	m_context.free();
}

/*-------------------------------------------------------------------------*//**
	We're about to draw the image - prepare the layer.
*/
void NdhsCPluginElementBufferedView::prepareForPaint(LcwCPlugin* pCurrentWidget)
{
	if (!pCurrentWidget)
		return;

	// Don't bother if we have no image configured
	if (false == hasImage())
		return;

	LcTScalar width		= pCurrentWidget->getExtent().x;
	LcTScalar height	= pCurrentWidget->getExtent().y;
	LcTVector location	= LcTVector(-(width / 2), -(height / 2), 0);

	if (m_skipDrawing == true)
	{
		// We don't want this layer to be drawn, so give the layer's paint rect a negative height and width
		height = -1;
		width = -1;
	}

	pCurrentWidget->setPaintRect(LcTPlaneRect(location.x,
										location.y + height,
										location.z,
										location.x + width,
										location.y));
}

/*-------------------------------------------------------------------------*//**
	Ask the plugin element to render to the layer.
*/
bool NdhsCPluginElementBufferedView::paintElement()
{
	bool retVal = true;

	if (getPluginHandle() != NULL)
	{
		m_skipDrawing = false;

		if (m_repaint)
			retVal = getPluginHandle()->paintElement();

		m_repaint = false;

		// On failure, stop the widget from rendering the frame
		if (retVal == false)
		{
			m_skipDrawing = true;
		}
		else if (m_mode == IFX_MODE_ELEMENT_BUFFERED)
		{
			if (m_image && m_buffer != m_context->pBuffer)
			{
				m_image->setBufferData(m_context->pBuffer);

				// Cache the buffer pointer to remove superfluous refreshes.
				m_buffer = m_context->pBuffer;
			}
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Resize rendering buffer
*/
bool NdhsCPluginElementBufferedView::setBufferSize(int width, int height)
{
	bool retVal = true;
	bool updateContext = false;

	// NdhsCMemoryImage requires a space pointer - ignore buffer size requests before
	// widget realized
	if (getPluginHandle()->getSpace() == NULL || !m_context)
		return false;

	if (retVal == true)
	{
		// Either create the buffer, if not already created, or resize the buffer as required
		if (!m_image)
		{
		    NdhsCMemoryImage::EImageType imageType;

		    if (m_mode == IFX_MODE_ELEMENT_BUFFERED)
                imageType = NdhsCMemoryImage::EImageTypeExternalBuffer;
            else
                imageType = NdhsCMemoryImage::EImageTypeInternalBuffer;

			LcTaOwner<NdhsCMemoryImage> newMemImage = NdhsCMemoryImage::create(getPluginHandle()->getSpace(),
																			width,
																			height,
																			m_context->format,
																			imageType);
			if (newMemImage)
			{
				m_image = newMemImage;
				m_image->setPreMultipliedAlpha(getPreMultipliedAlpha());
				updateContext = true;
			}
			else
			{
				retVal = false;
			}
		}
		else if (width != m_context->width || height != m_context->height)
		{
			retVal = m_image->resize(width, height);
			if (retVal)
				updateContext = true;
		}

		if (updateContext == true)
		{
            if (m_mode == IFX_MODE_ELEMENT_BUFFERED_NORMAL)
            {
                // Assign the pointer to the image data buffer (in case the
                // resize call had to re-alloc the buffer).
                m_context->pBuffer = m_image->data();
            }

			// Assign the height and width of the image
			m_context->height = height;
			m_context->width  = width;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementBufferedView::setTranslucency(bool isTranslucent)
{ 
	if(m_image) 
	{
		m_image->setTranslucency(isTranslucent); 
	}
}

/*-------------------------------------------------------------------------*//**
	Change buffer format
*/
bool NdhsCPluginElementBufferedView::setBufferFormat(IFX_BUFFER_FORMAT format)
{
	bool retVal = true;

	// NdhsCMemoryImage requires a space pointer - ignore buffer size requests before
	// widget realized
	if (getPluginHandle()->getSpace() == NULL || !m_context)
		return false;

#if !defined(LC_PLAT_OGL)
	format = IFX_32BPP_RGBA;
#endif

	// Assign the format information
	m_context->format = format;

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Let buffered image know that it's dirty
*/
void NdhsCPluginElementBufferedView::refreshBuffer()
{
	if (m_image)
	{
		m_image->setDirty();
		m_repaint = true;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementBufferedView::releaseResources()
{
	if (getPluginHandle() == NULL)
		return;

	if (m_mode == IFX_MODE_ELEMENT_BUFFERED_NORMAL)
		m_image->releaseImageData();

	// Free buffer
	m_image.destroy();

	return;
}

/*-------------------------------------------------------------------------*//**
	Recreate rendering Surface
*/
void NdhsCPluginElementBufferedView::loadResources()
{
	if (getPluginHandle()->getSpace() == NULL)
		return;

	if (!m_context)
		return;

	NdhsCMemoryImage::EImageType imageType;

	if (m_mode == IFX_MODE_ELEMENT_BUFFERED)
		imageType = NdhsCMemoryImage::EImageTypeExternalBuffer;
	else
		imageType = NdhsCMemoryImage::EImageTypeInternalBuffer;

	LcTaOwner<NdhsCMemoryImage> newMemImage = NdhsCMemoryImage::create(getPluginHandle()->getSpace(),
																	m_context->width,
																	m_context->height,
																	m_context->format,
																	imageType);
	if (newMemImage)
	{
		if (m_mode == IFX_MODE_ELEMENT_BUFFERED_NORMAL)
		{
			newMemImage->setImageData((LcTByte*)m_context->pBuffer);
		}
		else if(m_mode == IFX_MODE_ELEMENT_BUFFERED)
		{
			newMemImage->setBufferData(m_context->pBuffer);
		}

		m_image = newMemImage;
	}
}

#endif // IFX_USE_PLUGIN_ELEMENTS
