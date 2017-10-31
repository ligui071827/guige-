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

#ifndef LcOglCNativeFontH
#define LcOglCNativeFontH

#include "inflexionui/engine/inc/LcCNativeFont.h"

class LcCSpace;

/*-------------------------------------------------------------------------*//**
*/
class LcOglCNativeFont : public LcCNativeFont
{
private:

	// Two-phase construction
									LcOglCNativeFont(LcCSpace* s)
										: LcCNativeFont(s) {}

LC_PRIVATE_INTERNAL_PUBLIC:

	#ifdef LC_OGL_DIRECT
		// Reload the font internals.
		inline			void		reloadFont(const LcTmString& fileName)	{ open(fileName, m_style); }
	#endif

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcOglCNativeFont> create(LcCSpace* s);
	LC_VIRTUAL						~LcOglCNativeFont();

	// LcCFont methods
	LC_VIRTUAL	LcTaOwner<LcCText>	createText();
};

#endif //LcOglCNativeFontH
