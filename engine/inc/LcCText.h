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
#ifndef LcCTextH
#define LcCTextH

#include "inflexionui/engine/inc/LcStl.h"
#include "inflexionui/engine/inc/LcTVector.h"
#include "inflexionui/engine/inc/LcTColor.h"
#include "inflexionui/engine/inc/LcCBase.h"
class LcCFont;
class LcTPlaneRect;
class LcCMarqueeRenderer;

// These define the dimensions of the caret source data.
#define LC_CARET_SRC_WIDTH 1
#define LC_CARET_SRC_HEIGHT 1
#define LC_CARET_DEST_WIDTH 1

/*-------------------------------------------------------------------------*//**
	An item of text rendered by a particular font.  The text is created by
	the font itself, and a string rendered with it using the render() method.
	The string can then be repeatedly painted using draw().  Note that the
	position and size of the text is specified in the rendering step, so
	that draw() doesn't have to recalculate coordinates etc.

	Fonts are owned by the space and reference-counted.  Each text acquires
	a reference to the font it is created with, to guarantee that it is
	available when the text is drawn.
*/
class LcCText : public LcCBase
{
private:

	// Reference-counted
	LcCFont*						m_font;

	// Stored caret for blitting.
	LcTByte							m_caretData[LC_CARET_SRC_WIDTH * LC_CARET_SRC_HEIGHT];

	// Cached index of character the caret is at.
	int								m_caretCharPos;

	// Store the string data.
	LcTScalar						m_textHeight;
	LcTmString						m_stringText;

protected:

	// Effectively abstract
	LC_IMPORT						LcCText(LcCFont* f);

	// Get the caret data.
	LcTByte*						getCaret()		{ return m_caretData; }

	// The caret character position.
	inline			int				getCaretCharPos()					{ return m_caretCharPos; }
	inline			void			setCaretCharPos(int caretCharPos)	{ m_caretCharPos = caretCharPos; }

	// Store the string data.
	inline			LcTScalar		getTextHeight() 								{ return m_textHeight; }
	inline			void			setTextHeight(LcTScalar newTextHeight)			{ m_textHeight = newTextHeight; }
					LcTaString		getStringText()									{ return m_stringText; }
					void			setStringText(const LcTmString& newStringText)	{ m_stringText = newStringText; }

public:

	// Destruction
	LC_VIRTUAL						~LcCText();

	// Render/draw string
	LC_VIRTUAL		void			render(
										const LcTVector&	pos,
										LcTScalar			height,
										const LcTmString&	text,
										int					caretPos,
										IFX_INT32			*rtl)	= 0;
	LC_VIRTUAL		void			draw(
										const LcTPixelRect&	clip,
										LcTColor			color,
										LcTScalar			opacity,
										bool				antiAlias,
										int					meshGridX,
										int					meshGridY,
										LcCMarqueeRenderer *renderer)	= 0;

	// Access methods
	inline			LcCFont*		getFont()		{ return m_font; }
};

#endif // LcCTextH

