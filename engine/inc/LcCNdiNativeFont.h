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

#ifndef LcCNdiNativeFontH
#define LcCNdiNativeFontH

#include "inflexionui/engine/inc/LcCNativeFont.h"

class LcCSpace;
class LcCNdiGraphics;

/*-------------------------------------------------------------------------*//**
*/
class LcCNdiNativeFont : public LcCNativeFont
{
private:

	// Resources
	LcCNdiGraphics*					m_graphics;

	// Two-phase construction
									LcCNdiNativeFont(LcCSpace* s, LcCNdiGraphics* g)
										: LcCNativeFont(s) { m_graphics = g; }

LC_PRIVATE_INTERNAL_PUBLIC:

	// Access graphics pointers.
	inline		LcCNdiGraphics*		getGraphics()	{ return m_graphics; }

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcCNdiNativeFont> create(LcCSpace* s, LcCNdiGraphics* g);
	LC_VIRTUAL						~LcCNdiNativeFont();

	// LcCFont methods
	LC_VIRTUAL	LcTaOwner<LcCText>	createText();
};

#endif //LcCNdiNativeFontH
