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


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcwCImage> LcwCImage::create()
{
	LcTaOwner<LcwCImage> ref;
	ref.set(new LcwCImage);
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcwCImage::LcwCImage()
{
	m_keepAspectRatio = false;
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCImage::reloadResources()
{	
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	configEffectUniforms(getSpace(),"");
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCImage::onRealize()
{
	LcCSpace* sp = getSpace();

	// Validate configured image against new space
	if (getImage() && getImage()->getSpace() != sp) {
		setImage(NULL);
		return;
	}

	sp->addWidget(this);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCImage::onPrepareForPaint()
{
	// Don't bother if we have no image configured
	if (!m_image)
		return;

	LcTScalar width		= getExtent().x;
	LcTScalar height	= getExtent().y;
	LcTVector location	= LcTVector(-(width/2), -(height/2), 0);

	// Re-jig bounding box if image aspect ratio is different
	if (m_keepAspectRatio)
	{
		// Calculate aspect ratios (W/H)
		LcTPixelDim dim = m_image->getSize();
		LcTScalar arim = LcTScalar(dim.width) / dim.height;
		LcTScalar arbb = LcTScalar(width) / height;

		// Decide whether to shrink width or height
		if (arim > arbb)
		{
			// Image is "more" landscape-shaped than BB so reduce BB height
			LcTScalar h = width / arim;
			location.add(LcTVector(0, (height - h) / 2, 0));
			height = h;
		}
		else
		{
			// Image is "more" portrait-shaped than BB so reduce BB width
			LcTScalar w = height * arim;
			location.add(LcTVector((width - w) / 2, 0, 0));
			width = w;
		}
	}

	// Inform the layer where to paint itself
	// Origin is Bottom Left.
	m_recPaint = LcTPlaneRect(location.x,
				  location.y + height,
				  location.z,
				  location.x + width,
				  location.y);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCImage::onWantBoundingBox()
{
	LcCSpace* sp = getSpace();
	sp->extendBoundingBox(m_recPaint);
}

#if defined(LC_PLAT_OGL)
/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL void LcwCImage::onPaintOpaque(const LcTPixelRect& clip)
{
	if (m_image)
	{
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
		if (getEffectMakesTranslucent() == EEffectTranslucencyCacheUndefined)
		{
			m_effectMakesTranslucent = EEffectTranslucencyCacheOpaque;

			LcOglCContext *context = getSpace()->getOglContext();

			if ((context != NULL) && context->getCustomEffectTranslucency(TEXLIGHT00_EFFECT_INDEX, this))
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

		m_image->draw(
			m_recPaint,
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
LC_EXPORT_VIRTUAL void LcwCImage::onPaint(const LcTPixelRect& clip)
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

		m_image->draw(
			m_recPaint,
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
LC_EXPORT_VIRTUAL bool LcwCImage::contains(const LcTVector& loc, LcTScalar expandRectEdge)
{
	if (m_tappable == EFull)
	{
		// If tappable is full then use the tap tolerance setting.
		return  m_image && m_recPaint.contains(loc, expandRectEdge);
	}
	else if (m_tappable == EPartial)
	{
		return m_image && m_recPaint.contains(loc) && !m_image->isTransparent(
			m_recPaint,
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
LC_EXPORT void LcwCImage::setImage(LcIImage* img)
{
	// Do nothing if image isn't changing
	if (m_image == img)
		return;

	// Detach and release previous image
	if (m_image)
	{
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
void LcwCImage::setMeshGrid(int meshGridX, int meshGridY)
{
	m_meshGridX = max(1, meshGridX); 
	m_meshGridY = max(1, meshGridY); 
}
