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


#if defined(IFX_USE_BITMAPPED_FONTS) && defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcOglCText> LcOglCText::create(LcOglCFont* f)
{
	LcTaOwner<LcOglCText> ref;
	ref.set(new LcOglCText(f));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcOglCText::cacheInit(
	int				maxLength,
	LcTScalar		destLeft,
	LcTScalar		destTop,
	LcTScalar		destBottom,
	LcTScalar		destZ)
{
	m_length		= 0;

	// Keep fixed values pre-converted to required type
	m_destLeft		= LC_OGL_FROM_SCALAR(destLeft);
	m_destTop		= LC_OGL_FROM_SCALAR(destTop);
	m_destBottom	= LC_OGL_FROM_SCALAR(destBottom);
	m_destZ			= LC_OGL_FROM_SCALAR(destZ);

	m_destRight = m_destLeft;

	if (maxLength > 0)
	{
		// Allocate arrays for direct access by OpenGL
		m_pVertices		.free();
		m_pNormals		.free();
		m_pTexCoords	.free();
		m_pIndex		.free();
		m_pVertices		.alloc(12 * maxLength);
		m_pNormals		.alloc(12 * maxLength);
		m_pTexCoords	.alloc(8 * maxLength);
		m_pIndex		.alloc(6 * maxLength * sizeof(GLushort));
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcOglCText::cacheChar(
	LcTScalar		srcLeft,
	LcTScalar		srcRight,
	LcTScalar		srcTop,
	LcTScalar		srcBottom,
	LcTScalar		destRight)
{
	// Convert to correct type
	LcOglTScalar destRightOgl	= LC_OGL_FROM_SCALAR(destRight);

	// Populate vertex cache (local 3D coordinates, as passed)
	// We are adding the text to the left of the right edge.
	int vertBase				= 12 * m_length;

	m_pVertices[vertBase + 0]	= m_destRight;
	m_pVertices[vertBase + 1]	= m_destBottom;
	m_pVertices[vertBase + 2]	= m_destZ;

	m_pVertices[vertBase + 3]	= destRightOgl;
	m_pVertices[vertBase + 4]	= m_destBottom;
	m_pVertices[vertBase + 5]	= m_destZ;

	m_pVertices[vertBase + 6]	= m_destRight;
	m_pVertices[vertBase + 7]	= m_destTop;
	m_pVertices[vertBase + 8]	= m_destZ;

	m_pVertices[vertBase + 9]	= destRightOgl;
	m_pVertices[vertBase + 10]	= m_destTop;
	m_pVertices[vertBase + 11]	= m_destZ;

	// Store the new right edge.
	m_destRight					= destRightOgl;

	// Scale bitmap pixel vales to texture coords (note pixel offset vertically)
	LcOglTScalar texLeft	= LC_OGL_FROM_SCALAR(srcLeft / LC_BMF_BITMAP_WIDTH);
	LcOglTScalar texRight	= LC_OGL_FROM_SCALAR(srcRight / LC_BMF_BITMAP_WIDTH);
	LcOglTScalar texTop		= LC_OGL_FROM_SCALAR(srcTop / LC_BMF_BITMAP_HEIGHT);
	LcOglTScalar texBottom	= LC_OGL_FROM_SCALAR(srcBottom / LC_BMF_BITMAP_HEIGHT);

	// Populate texture cache (scale pixel coordinates down to texture range)
	int texBase					= 8 * m_length;

	// Set up texture coord array for OpenGL
	m_pTexCoords[texBase + 0]	= texLeft;
	m_pTexCoords[texBase + 1]	= texBottom;

	m_pTexCoords[texBase + 2]	= texRight;
	m_pTexCoords[texBase + 3]	= texBottom;

	m_pTexCoords[texBase + 4]	= texLeft;
	m_pTexCoords[texBase + 5]	= texTop;

	m_pTexCoords[texBase + 6]	= texRight;
	m_pTexCoords[texBase + 7]	= texTop;

	// Now finally we set up the indices for this char
	int indexBase				= 6 * m_length;
	int indexOffset				= 4 * m_length;

	// Have to draw quad as two triangles because OpenGL ES doesn't support quads
	m_pIndex[indexBase + 0]		= GLushort(indexOffset + 0);
	m_pIndex[indexBase + 1]		= GLushort(indexOffset + 1);
	m_pIndex[indexBase + 2]		= GLushort(indexOffset + 3);
	m_pIndex[indexBase + 3]		= GLushort(indexOffset + 0);
	m_pIndex[indexBase + 4]		= GLushort(indexOffset + 3);
	m_pIndex[indexBase + 5]		= GLushort(indexOffset + 2);

	// Done
	m_length++;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcOglCText::draw(
	const LcTPixelRect&	clip,
	LcTColor			color,
	LcTScalar			opacity,
	bool				antiAlias,
	int					meshGridX,
	int					meshGridY,
	LcCMarqueeRenderer *renderer)
{
	LC_UNUSED(renderer)

	if (opacity < LC_TRANSPARENT_THRESHOLD)
		return;

	if (opacity > LC_OPAQUE_THRESHOLD)
		opacity = 1;

	LcCFont* font = getFont();
	LcCSpace* space = font->getSpace();
	LcOglCContext* oglContext = space->getOglContext();
	IFX_DISPLAY* display = space->getDisplay();

	space->transformsChanged();

#if defined (LC_PLAT_OGL_20)

	LcOglCGlobalState* globalState = oglContext->getGlobalState();

	// ALPHA Texture with Lighting enabled
	if (globalState->getGlobalLightingStatus() == true)
	{
		// Switch to ALPHA Texture + Lighting will take place here
		oglContext->switchEffect(ALPHALIGHT00_EFFECT_INDEX, space->getPaintWidget());
	}
	else
	{
		// ALPHA Texture without lighting
		bool bHighQuality = false;
		oglContext->switchEffect(ALPHALIGHT00_EFFECT_INDEX, bHighQuality);
	}

	LcOglCEffect* currentEffect = oglContext->getCurrentEffect();
#endif

	oglContext->setCullFace(GL_FALSE);

	// Populate normal cache
	LcOglTScalar zNormal = space->isEyeFacing() ? 1.0f : -1.0f;

	for (int normIdx = 0; normIdx < 12 * m_length; normIdx += 3)
	{
		m_pNormals[normIdx + 0]	= 0;
		m_pNormals[normIdx + 1]	= 0;
		m_pNormals[normIdx + 2]	= zNormal;
	}

#if defined (LC_PLAT_OGL_20)

	if (currentEffect != NULL)
	{
		// Set Texture Unit 0 as active texture unit. This will be used for multiple texture.
		glActiveTexture(GL_TEXTURE0);

		currentEffect->bindTextures(NULL, EBitmap, ((LcOglCFont*)font)->getTexture());

		// Get attribute location for non-fixed attributes
		GLint iLocPosition = currentEffect->getEnumeratedLocation(IFX_VERTEX_COORDS);
		GLint iLocTexture  = currentEffect->getEnumeratedLocation(IFX_TEXTURE_COORDS);
		GLint iLocNormal   = currentEffect->getEnumeratedLocation(IFX_NORMAL_COORDS);

		// Load the vertex, tangent and normal coodinates

		// Load vertex coodinates only if valid attribute index exists
		if (iLocPosition >= 0)
        	glVertexAttribPointer (iLocPosition, 3, LC_OGL_SCALAR_TYPE, GL_FALSE, 0, m_pVertices);

		// Load normal coodinates only if valid attribute index exists
		if (iLocNormal >= 0)
			glVertexAttribPointer (iLocNormal, 3, LC_OGL_SCALAR_TYPE, GL_FALSE, 0, m_pNormals);

		// Load texture coodinates only if valid attribute index exists
		if (iLocTexture >= 0)
			glVertexAttribPointer (iLocTexture, 2, LC_OGL_SCALAR_TYPE, GL_FALSE, 0, m_pTexCoords);
	}

#else
	// Pass constructed arrays to OpenGL
	glVertexPointer(3, LC_OGL_SCALAR_TYPE, 0, m_pVertices);
	glNormalPointer(LC_OGL_SCALAR_TYPE, 0, m_pNormals);
	glTexCoordPointer(2, LC_OGL_SCALAR_TYPE, 0, m_pTexCoords);

	glBindTexture(GL_TEXTURE_2D, ((LcOglCFont*)font)->getTexture());

#endif /* defined (LC_PLAT_OGL_20) */

	// No color means no modulation, which means white
	if (color == LcTColor::NONE)
		color = LcTColor::WHITE;

	// Textures are modulated onto a pre-lit base rectangle subject to
	// lighting, using materials to avoid color/material state changes
	// NB: we always call this, as even opaque white requires a reset
	oglContext->setMaterialFlatColor(color, opacity);

	// Turn on MSAA if it is supported
	if (display->msaa_samples > 1 && antiAlias)
		glEnable(GL_MULTISAMPLE);

	// At long last!
	glDrawElements(
		GL_TRIANGLES,
		2 * 3 * m_length,
		GL_UNSIGNED_SHORT,
		m_pIndex);

	// Turn off MSAA if used
	if (display->msaa_samples > 1 && antiAlias)
		glDisable(GL_MULTISAMPLE);

	// If we need to draw a caret
	if ((getCaretCharPos() >= 0) && (getCaretCharPos() <= getStringText().length()))
	{
		drawCaret(clip, color, opacity, antiAlias);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcOglCText::drawCaret(
	const LcTPixelRect&	clip,
	LcTColor			color,
	LcTScalar			opacity,
	bool				antiAlias)
{
	// Create a caret texture.
	LcTaOwner<LcOglCTexture> caretTexture = LcOglCTexture::create(getFont()->getSpace()->getOglContext());
	caretTexture->createTexture(LcTPixelDim(LC_CARET_SRC_WIDTH, LC_CARET_SRC_HEIGHT), GL_UNSIGNED_BYTE, GL_ALPHA);
	caretTexture->updateTexture(getCaret());

	// Work out the left position of the text.
	LcTScalar leftPos = LC_OGL_TO_SCALAR(m_destLeft)
						+ getFont()->getTextWidth(	getTextHeight(),
													getStringText().subString(0, getCaretCharPos()));

	// Ensure there is enough space to blit the caret.
	// At the left and right edge, it needs moving in.
	if (leftPos <= LC_OGL_TO_SCALAR(m_destLeft))
		leftPos = LC_OGL_TO_SCALAR(m_destLeft) + (LcTScalar)0.5;
	else if ((leftPos + LC_CARET_DEST_WIDTH) >= LC_OGL_TO_SCALAR(m_destRight))
		leftPos = LC_OGL_TO_SCALAR(m_destRight) - LC_CARET_DEST_WIDTH;

	// Define the caret position from cached coords.
	// Note: this is done in local 3D space, not canvas space
	LcTScalar topPos = LC_OGL_TO_SCALAR(m_destTop);
	LcTPlaneRect caretDest( leftPos,
	                        topPos,
	                        (LcTScalar)(LC_OGL_TO_SCALAR(m_destZ) + 0.01),
							(LcTScalar)(leftPos + LC_CARET_DEST_WIDTH),
							topPos - getTextHeight());

	LcTScalarRect srcRect(0, 0, 0, LC_CARET_SRC_WIDTH, LC_CARET_SRC_HEIGHT);

	// Thunderbirds are GO...
	// NB: in 3D modes, the color may have already been grayed to apply lighting
	caretTexture->drawRectangle(
		srcRect,
		caretDest,
		color,
		opacity,
		antiAlias,
		1,
		1);
}

#endif	// #if defined(IFX_USE_BITMAPPED_FONTS) && defined(LC_PLAT_OGL)
