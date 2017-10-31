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


#if defined(IFX_USE_NATIVE_FONTS) && defined(LC_PLAT_OGL)

extern "C"
{
	#include <math.h>
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcOglCNativeText> LcOglCNativeText::create(LcOglCNativeFont* f)
{
	LcTaOwner<LcOglCNativeText> ref;
	ref.set(new LcOglCNativeText(f));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCNativeText::construct()
{
	m_texture = LcOglCTexture::create(getFont()->getSpace()->getOglContext());
	LcCNativeText::construct();
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
LcOglCNativeText::~LcOglCNativeText()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcOglCNativeText::render(
	const LcTVector&	pos,
	LcTScalar			height,
	const LcTmString&	text,
	int					caretPos,
	IFX_INT32			*rtl)
{
	// Call base function.
	LcCNativeText::render(pos, height, text, caretPos, rtl);

	// Create texture and initialize from data
	if (getTextCanvas())
	{
		if (!m_texture->isCreated())
		{
			m_texture->createTexture(LcTPixelDim(getTextCanvas()->width, getTextCanvas()->height), GL_UNSIGNED_BYTE, GL_ALPHA);
		}
		m_texture->updateTexture(getTextCanvas()->alphaBuffer);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcOglCNativeText::draw(
	const LcTPixelRect& clip,
	LcTColor			color,
	LcTScalar			opacity,
	bool				antiAlias,
	int					meshGridX,
	int					meshGridY,
	LcCMarqueeRenderer *renderer)
{
	// Verify the inputs.
	if (!getTextCanvas())
		return;

	if (opacity < LC_TRANSPARENT_THRESHOLD)
		return;

	if (opacity > LC_OPAQUE_THRESHOLD)
		opacity = 1;
		
	getFont()->getSpace()->getOglContext()->setCullFace(GL_FALSE);
	getFont()->getSpace()->transformsChanged();

	// Origin is Top Left
	LcTVector localTlPos = getTlPos();
	LcTPlaneRect destRect(	localTlPos.x,
	                      	localTlPos.y + getHeight(),
	                      	localTlPos.z,
	                      	localTlPos.x + getWidth(),
	                      	localTlPos.y);

	LcTScalarRect srcRect (0, 0, 0, (LcTScalar)getTextCanvas()->width, (LcTScalar)getTextCanvas()->height);

	if(renderer != NULL)
	{
		LcTPlaneRect destRectTemp = renderer->getDestinationRect(srcRect, destRect);

		srcRect = renderer->getSourceRect(srcRect, destRect);
		destRect = destRectTemp;

		if (srcRect.getHeight() == 0)
			return;
	}

	// Thunderbirds are GO...
	m_texture->drawRectangle(
		srcRect,
		destRect,
		color,
		opacity,
		antiAlias,
		meshGridX,
		meshGridY);

	// If we need to draw a caret
	if ((getCaretCharPos() >= 0) && (getCaretCharPos() <= getStringText().length()))
	{
		drawCaret(clip, color, opacity, antiAlias);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcOglCNativeText::drawCaret(
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
	LcTVector localTlPos = getTlPos();
	LcTScalar leftPos = localTlPos.x
						+ getFont()->getTextWidth(	getTextHeight(),
													getStringText().subString(0, getCaretCharPos()));

	// Ensure there is enough space to blit the caret.
	// At the left and right edge, it needs moving in.
	if (leftPos <= localTlPos.x)
		leftPos = localTlPos.x + 0.5f;
	else if ((leftPos + LC_CARET_DEST_WIDTH) >= localTlPos.x + getWidth())
		leftPos = localTlPos.x + getWidth() - LC_CARET_DEST_WIDTH;
						
	// Define the caret position from cached coords.
	// Note: Origin is Top Left
	LcTPlaneRect caretDest( leftPos,
	                        localTlPos.y + getTextHeight(),
	                        localTlPos.z + 0.01f,
							leftPos + LC_CARET_DEST_WIDTH,
							localTlPos.y);
	
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

#endif	// defined(IFX_USE_NATIVE_FONTS) && defined(LC_PLAT_OGL)

