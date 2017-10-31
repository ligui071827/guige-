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
#ifndef LcOglCFontH
#define LcOglCFontH

#include "inflexionui/engine/inc/LcCBitmappedFont.h"

/*-------------------------------------------------------------------------*//**
*/
class LcOglCFont : public LcCBitmappedFont
{
private:

	// Texture ID
	GLuint							m_tex;

protected:

	// Called by derived class once texture created
	inline			void			setTexture(GLuint t)	{ m_tex = t; }

	// Effectively abstract so keep constructors protected
	inline							LcOglCFont(LcCSpace* sp)
										: LcCBitmappedFont(sp) {}

LC_PRIVATE_INTERNAL_PUBLIC:

	// Enables LcOglCText to access texture for drawing											
	inline			GLuint			getTexture()			{ return m_tex; }

public:

	// Destruction
	LC_VIRTUAL						~LcOglCFont();
									
	// LcCFont methods
	LC_VIRTUAL	LcTaOwner<LcCText>	createText();
};

#endif //LcOglCFontH
