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

#ifndef LcOglCNativeTextH
#define LcOglCNativeTextH

#include "inflexionui/engine/inc/LcCText.h"
class LcOglCNativeFont;

/*-------------------------------------------------------------------------*//**
*/
class LcOglCNativeText : public LcCNativeText
{
private:

	LcTmOwner<LcOglCTexture>		m_texture;

#ifdef LC_OGL_DIRECT
	// Store a creation number which is incremented if the screen context is
	// recreated.
	// This allows the textures to verify if they need to reload themselves.
	int								m_contextNumber;
#endif

	// Two-phase construction
									LcOglCNativeText(LcOglCNativeFont* f)
										: LcCNativeText(f) {}

LC_PRIVATE_INTERNAL_PUBLIC:

#ifdef LC_OGL_DIRECT
	// Set the context number, done on initialization.
	inline			void			setContextNumber(int contextValue)	{ m_contextNumber = contextValue; }
#endif

protected:
					void			construct();

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcOglCNativeText> create(LcOglCNativeFont* f);
	LC_VIRTUAL						~LcOglCNativeText();

	// LcCText methods
	LC_VIRTUAL		void			render(
										const LcTVector&	pos, 
										LcTScalar			height, 
										const LcTmString&	text,
										int					caretPos,
										IFX_INT32			*rtlOut);

	LC_VIRTUAL		void			draw(
										const LcTPixelRect&	clip,
										LcTColor			color, 
										LcTScalar			opacity,
										bool				antiAlias,
										int					meshGridX,
										int					meshGridY,
										LcCMarqueeRenderer *renderer);
					void			drawCaret(
    									const LcTPixelRect&	clip,
    									LcTColor			color,
    									LcTScalar			opacity,
										bool				antiAlias);
};

#endif //LcOglCNativeTextH
