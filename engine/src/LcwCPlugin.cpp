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
Construction
*/
LC_EXPORT LcwCPlugin::LcwCPlugin()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCPlugin::onWantBoundingBox()
{
	LcCSpace* sp = getSpace();
	LcTPlaneRect paintRect;

	if (isFullScreen())
		paintRect = LcTPlaneRect(-sp->getGlobalExtent().x / 2, sp->getGlobalExtent().y / 2, 0, sp->getGlobalExtent().x / 2, -sp->getGlobalExtent().y / 2);
	else
		paintRect = m_recPaint;

	sp->extendBoundingBox(paintRect);
}

#if defined(LC_PLAT_OGL)
/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL void LcwCPlugin::onPaintOpaque(const LcTPixelRect& clip)
{
	if (m_image)
	{
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
		if (getEffectMakesTranslucent() == EEffectTranslucencyCacheUndefined)
		{
			m_effectMakesTranslucent = EEffectTranslucencyCacheOpaque;

			LcOglCContext *context = getSpace()->getOglContext();

			if ((context!=NULL) && context->getCustomEffectTranslucency(TEXLIGHT00_EFFECT_INDEX, this))
			{
				m_effectMakesTranslucent = EEffectTranslucencyCacheTranslucent;
			}
		}
#endif
		LcTScalar opacity = getOverallOpacity();
		m_cachedOpacity = opacity;
		m_useCachedOpacity = true;
		if(isTranslucent() || opacity  < LC_OPAQUE_THRESHOLD)
			return;

		LcCSpace* sp = getSpace();
		LcTPlaneRect paintRect;

		if (isFullScreen())
			paintRect = LcTPlaneRect(-sp->getGlobalExtent().x / 2, sp->getGlobalExtent().y / 2, 0, sp->getGlobalExtent().x / 2, -sp->getGlobalExtent().y / 2);
		else
			paintRect = m_recPaint;

		m_image->draw(
			paintRect,
			clip,
			findFontColor(), // misnomer!
			opacity,
			m_antiAlias,
			m_meshGridX,
			m_meshGridY);
	}
}
#endif

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCPlugin::onPaint(const LcTPixelRect& clip)
{
	if (m_image)
	{
		LcTScalar opacity = getOverallOpacity();
		m_useCachedOpacity = false;

#if defined(LC_PLAT_OGL)
		if(!isTranslucent() && opacity  >= LC_OPAQUE_THRESHOLD)
			return;

		bool useDepthBuffer = m_meshGridX > 1 || m_meshGridY > 1;

		// Only use depth buffer for mesh grid cases
		if (useDepthBuffer)
			getSpace()->getOglContext()->setDepthMask(GL_TRUE);
#endif

		LcCSpace* sp = getSpace();
		LcTPlaneRect paintRect;

		if (isFullScreen())
			paintRect = LcTPlaneRect(-sp->getGlobalExtent().x / 2, sp->getGlobalExtent().y / 2, 0, sp->getGlobalExtent().x / 2, -sp->getGlobalExtent().y / 2);
		else
			paintRect = m_recPaint;

		m_image->draw(
			paintRect,
			clip,
			findFontColor(), // misnomer!
			opacity,
			m_antiAlias,
			m_meshGridX,
			m_meshGridY);

#if defined(LC_PLAT_OGL)
		if (useDepthBuffer)
			getSpace()->getOglContext()->setDepthMask(GL_FALSE);
#endif
	}
}

/*-------------------------------------------------------------------------*//**
	Return whether the image is visible at the specified point.

	@return true if 'loc' is on an opaque pixel.

	@param loc The point on the images to test against.
*/
LC_EXPORT_VIRTUAL bool LcwCPlugin::contains(const LcTVector& loc, LcTScalar expandRectEdge)
{
	LcCSpace* sp = getSpace();
	LcTPlaneRect paintRect;

	if (isFullScreen())
		paintRect = LcTPlaneRect(-sp->getGlobalExtent().x / 2, sp->getGlobalExtent().y / 2, 0, sp->getGlobalExtent().x / 2, -sp->getGlobalExtent().y / 2);
	else
		paintRect = m_recPaint;

	if (m_tappable == EFull)
	{
		// If tappable is full then use the tap tolerance setting.
		return  m_image && paintRect.contains(loc, expandRectEdge);
	}
	else if (m_tappable == EPartial)
	{
		return m_image && paintRect.contains(loc) && !m_image->isTransparent(
			paintRect,
			getOverallScale(),
			NULL,
			loc);
	}
	else 
	{
		// Image not tappable
		return false;
	}
}

/*-------------------------------------------------------------------------*//**
	Set the image for the layer to display.

	@param img The image for the layer to display.

	@see #getImage
*/
LC_EXPORT void LcwCPlugin::setImage(LcIImage* img)
{
	// Do nothing if image isn't changing
	if (m_image == img)
		return;

	// Detach and release previous image
	if (m_image)
	{
		//m_image->detachLayer(this);
		m_image->release();
		m_image = NULL;
	}

	// If we are setting a new image rather than NULL
	if (img)
	{
		// Validate the image against the space if we are shown
		LcCSpace* sp = getSpace();
		if (!sp || img->getSpace() == sp)
		{
			// ...and if valid, acquire and attach it
			m_image = img;
			m_image->acquire();
		}
	}

	// Need to repaint
	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCPlugin::setPaintRect(const LcTPlaneRect& bbox)
{
	// Don't bother if we have no image configured
	if (!m_image)
		return;

	// Save position
	m_recPaint = bbox;
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCPlugin::setMeshGrid(int meshGridX, int meshGridY)
{
	m_meshGridX = max(1, meshGridX); 
	m_meshGridY = max(1, meshGridY); 
}

#endif // IFX_USE_PLUGIN_ELEMENTS
