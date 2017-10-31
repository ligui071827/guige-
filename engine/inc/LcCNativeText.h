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

#ifndef LcCNativeTextH
#define LcCNativeTextH

#include "inflexionui/engine/inc/LcCText.h"
class LcCNativeFont;

/*-------------------------------------------------------------------------*//**
*/
class LcCNativeText : public LcCText
{
private:

	// Cached rendering info
	LcTVector						m_tlPos;

	IFXP_TEXT*						m_textCanvas;

	// Text area height & width
	LcTScalar						m_height;
	LcTScalar						m_width;

protected:

	// Two-phase construction
									LcCNativeText(LcCNativeFont* f)
										: LcCText(f) {}

	// Cached rendering info
	inline			LcTVector&		getTlPos()								{ return m_tlPos; }
	inline			void			setTlPos(const LcTVector& newTlPos)		{ m_tlPos = newTlPos; }
	
	inline			IFXP_TEXT*		getTextCanvas()							{ return m_textCanvas; }
	inline			void			setTextCanvas(IFXP_TEXT* newTextCanvas)	{ m_textCanvas = newTextCanvas; }
	
	// Text area height & width
	inline			LcTScalar		getHeight()								{ return m_height; }
	inline			void			setHeight(LcTScalar newHeight)			{ m_height = newHeight; }
	inline			LcTScalar		getWidth()								{ return m_width; }
	inline			void			setWidth(LcTScalar newWidth)			{ m_width = newWidth; }
									
public:

	// Construction/destruction
	virtual							~LcCNativeText();

	// LcCText methods
	LC_VIRTUAL		void			render(
										const LcTVector&	pos, 
										LcTScalar			height, 
										const LcTmString&	text,
										int					caretPos,
										IFX_INT32			*rtlOut);
};

#endif //LcCNativeTextH
