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
#ifndef LcCCustomBitmapH
#define LcCCustomBitmapH

class LcCCustomBitmapFile;
class LcCNdiGraphics;

/*-------------------------------------------------------------------------*//**
	Implementation of Custom bitmap for drawing to LcCCustomGraphics surface
*/
class LcCCustomBitmap : public LcCBitmap
{
private:

	// Draw direct from 24bpp decoded file data
	LcTmOwner<LcCCustomBitmapFile>		m_custom;

	// Graphics object to draw to
	LcCNdiGraphics*					m_gfx;

	// LcCBitmap methods
	// NB: we draw the entire bitmap, rather than defining drawRegion to draw 
	// segments according to the margins.  This is because the NDI blit routines
	// cater for the margins internally, to avoid boundary artifacts
	LC_VIRTUAL		void			draw(
										int 					frameNo,
										const LcTPlaneRect&		dest,
										const LcTPixelRect& 	clip,
										LcTColor				color,
										LcTScalar 				opacity);

	LC_VIRTUAL		bool			isPointTransparent(int iX, int iY);
	LC_VIRTUAL		bool			isOpaque();
	virtual			bool			canBeClipped()  { return true; }

	// Two-phase construction
	inline							LcCCustomBitmap(LcCSpace* sp, LcCNdiGraphics* gfx)
										: LcCBitmap(sp) { m_gfx = gfx; }


public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcCCustomBitmap> create(LcCSpace* sp, LcCNdiGraphics* gfx);

	// Local methods
	LC_IMPORT		bool			open(const LcTmString& sFile,
												int marginLeft=0,
												int marginRight=0,
												int marginTop=0,
												int marginBottom=0,
												int frameCount=0);
};

#endif // LcCCustomBitmapH
