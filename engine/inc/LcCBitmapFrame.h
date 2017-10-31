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
#ifndef LcCBitmapFrameH
#define LcCBitmapFrameH

#include "inflexionui/engine/inc/LcCBitmap.h"

/*-------------------------------------------------------------------------*//**
	Utility that wraps an LcCBitmap to provide an LcIImage interface that draws
	a specific frame
*/
class LcCBitmapFrame : public LcCBase, public LcIImage
{
private:

	LcCBitmap*						m_bitmap;
	int								m_frame;

	// Allow only 2-phase construction
	inline							LcCBitmapFrame()		{}

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcCBitmapFrame> create();
	virtual							~LcCBitmapFrame()		{}

	// Config
	inline			void			setBitmap(LcCBitmap* i)	{ m_bitmap = i; }
	inline			void			setFrame(int f)			{ m_frame = f; }

	// LcIImage interface
	inline			LcCSpace*		getSpace()				{ return m_bitmap->getSpace(); }
	virtual			LcTPixelDim		getSize()				{ return m_bitmap->getSize(); }
	virtual			bool			isOpaque()				{ return m_bitmap->isOpaque(); }
	virtual			bool			canBeClipped()			{ return m_bitmap->canBeClipped(); }
	virtual			void			draw(
										const LcTPlaneRect& dest,
										const LcTPixelRect& clip,
										LcTColor			color,
										LcTScalar 			fOpacity,
										bool				antiAlias,
										int					meshGridX,
										int					meshGridY)
															{ m_bitmap->draw(m_frame, dest, clip, color, fOpacity, antiAlias, meshGridX, meshGridY); }
	virtual			bool			isTransparent(
										const LcTPlaneRect&	dest,
										const LcTVector&	scale,
										const LcTPlaneRect*	clip,
										const LcTVector&	hitPos)
															{ return m_bitmap->isTransparent(m_frame, dest, scale, clip, hitPos); }
	virtual			void			acquire()				{ m_bitmap->acquire(); }
	virtual			bool			release()				{ m_bitmap->release(); return true; }

	virtual			bool  			isTranslucent()			{ return m_bitmap->isTranslucent(); }
};

#endif // LcCBitmapFrameH
