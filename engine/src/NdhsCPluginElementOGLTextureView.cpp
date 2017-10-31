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


#if defined(IFX_USE_PLUGIN_ELEMENTS) && defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPluginElementOGLTextureView> NdhsCPluginElementOGLTextureView::create(
														NdhsCPlugin::NdhsCPluginHElement* v)
{
	LcTaOwner<NdhsCPluginElementOGLTextureView> ref;
	ref.set(new NdhsCPluginElementOGLTextureView(v));
	ref->construct();

	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPluginElementOGLTextureView::NdhsCPluginElementOGLTextureView(
														NdhsCPlugin::NdhsCPluginHElement* w)
							 : NdhsCPluginElementView(w)
{
	m_repaint = true;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementOGLTextureView::construct()
{
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementOGLTextureView::setTranslucency(bool isTranslucent)
{
	if (m_image)
	{
		m_image->setTranslucency(isTranslucent);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementOGLTextureView::releaseResources()
{
	if (getPluginHandle() == NULL)
		return;

	memset(m_texHandles, 0, sizeof(GLuint) * IFX_OGL_MAX_TEXTURES);

	if (m_image)
		m_image->setExternalTexture(m_texHandles, LcTPixelDim(), LcTPixelRect(), m_context->textureTarget);
}

/*-------------------------------------------------------------------------*//**
	Recreate rendering Surface
*/
void NdhsCPluginElementOGLTextureView::loadResources()
{
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementOGLTextureView::attachWidget(LcwCPlugin* pWidget)
{
	if (pWidget && m_image && m_context)
	{
		// Attach image to widget
		pWidget->setImage(m_image.ptr());
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementOGLTextureView::detachWidget(LcwCPlugin* pWidget)
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
NdhsCPluginElementOGLTextureView::~NdhsCPluginElementOGLTextureView()
{
	m_image.destroy();

	// Free render context
	m_context.free();
}

/*-------------------------------------------------------------------------*//**
	We're about to draw the image - prepare the layer for the specified 
	widget.
*/
void NdhsCPluginElementOGLTextureView::prepareForPaint(LcwCPlugin* pCurrentWidget)
{
	if (getPluginHandle() == NULL)
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
bool NdhsCPluginElementOGLTextureView::paintElement()
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
		else
		{
			GLuint texArray[IFX_OGL_MAX_TEXTURES];
			texArray[0] = (GLuint) m_context->texture;
#if defined(LC_PLAT_OGL_20)
			texArray[1] = (GLuint) m_context->secondaryTextures[0];
			texArray[2] = (GLuint) m_context->secondaryTextures[1];
			texArray[3] = (GLuint) m_context->secondaryTextures[2];
#endif

			if (m_image && memcmp(m_texHandles, texArray, sizeof(GLuint) * IFX_OGL_MAX_TEXTURES) != 0)
			{
				m_image->setExternalTexture(texArray,
											LcTPixelDim(m_context->tex_width, m_context->tex_height),
											LcTPixelRect(m_context->left, m_context->top, m_context->left + m_context->width, m_context->top + m_context->height),
											m_context->textureTarget);

				// Cache the buffer pointer to remove superfluous refreshes.
				memcpy(m_texHandles, texArray, sizeof(GLuint) * IFX_OGL_MAX_TEXTURES);
			}
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Resize rendering buffer
*/
bool NdhsCPluginElementOGLTextureView::setBufferSize(int width, int height)
{
	bool retVal = true;
	bool updateContext = false;

	// NdhsCMemoryImage requires a space pointer - ignore buffer size requests before
	// element realized
	if (getPluginHandle()->getSpace() == NULL)
		return false;

	// We need to create the context object if this is the first call
	if (!m_context)
	{
		// Create the buffer rendering context on the heap
		m_context.alloc(1);

		if (!m_context)
		{
			retVal = false;
		}
	}

	if (retVal == true)
	{
		// Either create the buffer, if not already created, or resize the buffer as required
		if (!m_image)
		{
			LcTaOwner<NdhsCMemoryImage> newMemImage = NdhsCMemoryImage::create(getPluginHandle()->getSpace(),
																			width,
																			height,
																			IFX_32BPP_RGBA, // Ignored for external texture
																			NdhsCMemoryImage::EImageTypeExternalTexture);
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
			// Assign the height and width of the image
			m_context->top        = 0;
			m_context->left       = 0;
			m_context->height     = height;
			m_context->width      = width;
			m_context->tex_height = height;
			m_context->tex_width  = width;

			// Initialize the texture info
			m_context->texture    = 0;
			m_context->textureTarget = GL_TEXTURE_2D;
			m_context->secondaryTextures[0] = 0;
			m_context->secondaryTextures[1] = 0;
			m_context->secondaryTextures[2] = 0;

#ifdef LC_USE_EGL
			m_context->eglDisplay = getPluginHandle()->getSpace()->getEglDisplay();
			m_context->eglSurface = getPluginHandle()->getSpace()->getEglSurface();
			m_context->eglContext = getPluginHandle()->getSpace()->getEglContext();
#endif
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Change buffer format
*/
bool NdhsCPluginElementOGLTextureView::setBufferFormat(IFX_BUFFER_FORMAT format)
{
	bool retVal = true;

	// NdhsCMemoryImage requires a space pointer - ignore buffer size requests before
	// widget realized
	if (getPluginHandle()->getSpace() == NULL)
		return false;

	// We need to create the context object if this is the first call
	if (!m_context)
	{
		// Create the buffer rendering context on the heap
		m_context.alloc(1);

		if (!m_context)
		{
			retVal = false;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Let buffered image know that it's dirty
*/
void NdhsCPluginElementOGLTextureView::refreshBuffer()
{
	if (m_image)
	{
		m_image->setDirty();
		m_repaint = true;
	}
}

#endif // IFX_USE_PLUGIN_ELEMENTS
