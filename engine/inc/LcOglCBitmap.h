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
#ifndef LcOglCBitmapH
#define LcOglCBitmapH

#include "inflexionui/engine/inc/LcCBitmap.h"
#include "inflexionui/engine/inc/LcOglCContext.h"
#include "inflexionui/engine/inc/LcOglCTexture.h"

/*-------------------------------------------------------------------------*//**
*/
class LcOglCBitmap : public LcCBitmap
{
private:
	LcTmAlloc<LcTByte>				m_transData;

LC_PRIVATE_INTERNAL_PROTECTED:

	LcTmOwner<LcOglCTexture>		m_texture;

LC_PRIVATE_INTERNAL_PUBLIC:
#ifdef LC_OGL_DIRECT
	// Reset the texture size parameters.
	inline			void			resetTexture() { m_texture->resetTexture(); }
#endif

protected:

	// Effectively abstract so keep constructor protected
	inline							LcOglCBitmap(LcCSpace* sp) : LcCBitmap(sp) {}
					void			construct();

	virtual			void			createTransparencyData(LcTByte* data, int numberPixels);
	virtual			bool			isPointTransparent(int iX, int iY);
	virtual			bool			canBeClipped()  { return true; }

public:

	// Destruction
	virtual							~LcOglCBitmap();

	LC_VIRTUAL		LcOglCTexture*	getTexture(LcTScalarRect& src);

	// LcCBitmap methods
	LC_VIRTUAL		void			drawRegion(
										const LcTScalarRect&	srcRect,
										const LcTPlaneRect&		destRect,
										const LcTPixelRect&		clipRect,
										LcTColor				color,
										LcTScalar				opacity,
										bool					antiAlias,
										int						meshGridX,
										int						meshGridY);
};

#endif //LcOglCBitmapH
