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

/*-------------------------------------------------------------------------*//**
	Should be called by the Space class.

	@param sp A pointer to the encompassing space class.
*/
LC_EXPORT LcCBitmap::LcCBitmap(LcCSpace* sp)
{
	m_space			= sp;
//	m_iRefCount		= 0;
//	setMargins(0, 0, 0, 0);
	m_iFrameCount	= 1;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCBitmap::~LcCBitmap()
{
}

/*-------------------------------------------------------------------------*//**
	Releases a reference to this image.

	Images should not be deleted directly but will be auto-deleted by the
	reference counting mechanism.

	@see LcCBitmap#acquire
*/
LC_EXPORT_VIRTUAL bool	LcCBitmap::release()
{
	// NB: we don't assume that we CAN delete the object.... the space
	// implementation may pool them
	if (--m_iRefCount <= 0)
		m_space->unloadBitmap(this);

	return false;
}

/*-------------------------------------------------------------------------*//**
	Because the bulk of the work in draw/hit-test ops concerns calculating
	the tiling regions, the same function is used for both.  Pass bDraw = true
	to draw the region, or bDraw = false to test transparency of the point
	within the dest region
*/
bool LcCBitmap::tileOp(
	bool					bDraw, // false = transparency test
	int						frameNo,
	const LcTPlaneRect&		dest,
	const LcTPixelRect*		destClip,
	LcTColor				color,
	LcTScalar				fOpacity,
	const LcTVector*		hitPos,
	bool					antiAlias,
	int						meshGridX,
	int						meshGridY)
{
	if (bDraw == false && hitPos == NULL)
		return true;

	forceLoad();

	LcTPixelDim frame  = getSize();

	if (frameNo < 0)
		frameNo = 0;

	if (frameNo > m_iFrameCount-1)
		frameNo = m_iFrameCount-1;

	int yOffset			= frameNo * frame.height;

	// A simple blit means that we only have to do one blit to draw the image,
	// rather than the nine that we may have to do. Even if we have margins
	// configured, we can perform a simple blit if there is no scaling
	// and the dimension of the target are the same as the source.
	bool simpleBlit=false;

	if (dest.getWidth() == frame.width && dest.getHeight() == frame.height)
	{
		simpleBlit = true;
	}

	// If no margins
	if (simpleBlit || !(m_iLeftMargin || m_iRightMargin || m_iTopMargin || m_iBottomMargin))
	{
		// Quick simple draw
		if (bDraw)
		{
			// Quick blit
			drawRegion(
				LcTScalarRect((LcTScalar)0,
					(LcTScalar)yOffset,
					(LcTScalar)0,
					(LcTScalar)frame.width,
					(LcTScalar)yOffset + frame.height),
				dest,
				*destClip,
				color,
				fOpacity,
				antiAlias,
				meshGridX,
				meshGridY);
		}
		else
		{
			// Quick simple mapping from hit point to frame

			LcTScalar fx = LcTScalar(hitPos->x - dest.getLeft()) / dest.getWidth();
			LcTScalar fy = LcTScalar(hitPos->y - dest.getBottom()) / dest.getHeight();

			fy = 1 - fy;

			return isPointTransparent(
				(int)(fx * frame.width),
				(int)(yOffset + (fy * frame.height)));
		}
	}
	else
	{
		// Discover the size of the four margins after applying the
		// scale factor.
		LcTScalar left		= (LcTScalar)m_iLeftMargin;
		LcTScalar right		= (LcTScalar)m_iRightMargin;
		LcTScalar top		= (LcTScalar)m_iTopMargin;
		LcTScalar bottom	= (LcTScalar)m_iBottomMargin;

		LcTScalar l_destXCoords[3];
		LcTScalar l_destYCoords[3];
		LcTScalar l_destHeight[3];
		LcTScalar l_destWidth[3];
		LcTScalar l_srcXCoords[3];
		LcTScalar l_srcYCoords[3];
		LcTScalar l_srcHeight[3];
		LcTScalar l_srcWidth[3];

		l_destXCoords[0] = dest.getLeft();
		l_destXCoords[1] = dest.getLeft() + left;
		l_destXCoords[2] = dest.getLeft() + dest.getWidth() - right;

		l_destYCoords[0] = dest.getBottom();
		l_destYCoords[1] = dest.getBottom() + bottom;
		l_destYCoords[2] = dest.getBottom() + dest.getHeight() - top;
		
		l_destHeight[0] = bottom;
		l_destHeight[1] = dest.getHeight() - top - bottom;
		l_destHeight[2] = top;

		l_destWidth[0] = left;
		l_destWidth[1] = dest.getWidth() - left - right;
		l_destWidth[2] = right;

		l_srcXCoords[0] = 0;
		l_srcXCoords[1] = (LcTScalar)m_iLeftMargin;
		l_srcXCoords[2] = (LcTScalar)(frame.width - m_iRightMargin);

		l_srcYCoords[0] = (LcTScalar)(yOffset + frame.height - m_iBottomMargin);
		l_srcYCoords[1] = (LcTScalar)(yOffset + m_iTopMargin);
		l_srcYCoords[2] = (LcTScalar)yOffset;

		l_srcHeight[0] = (LcTScalar)m_iBottomMargin;
		l_srcHeight[1] = (LcTScalar)(frame.height - m_iTopMargin - m_iBottomMargin);
		l_srcHeight[2] = (LcTScalar)m_iTopMargin;

		l_srcWidth[0] = (LcTScalar)m_iLeftMargin;
		l_srcWidth[1] = (LcTScalar)(frame.width - m_iLeftMargin - m_iRightMargin);
		l_srcWidth[2] = (LcTScalar)m_iRightMargin;

		if (bDraw)
		{
			// Draw each of the nine regions (unless zero sized)
			LcTPlaneRect	newDest;
			LcTScalarRect	newSource;
			for(int x=0 ; x < 3 ; x++)
			{
				for(int y=0 ; y < 3 ; y++)
				{
					if (l_srcWidth[x] > 0 && l_srcHeight[y] > 0)
					{
						newDest = LcTPlaneRect(l_destXCoords[x],
												l_destYCoords[y] + l_destHeight[y],
												dest.getZDepth(),
												l_destXCoords[x] + l_destWidth[x],
												l_destYCoords[y]);

						// Find out if this section needs clipping

						newSource = LcTScalarRect(	l_srcXCoords[x],
													l_srcYCoords[y],
													0.0,
													l_srcXCoords[x] + l_srcWidth[x],
													l_srcYCoords[y] + l_srcHeight[y]);

						// Margin bitmaps always drawn with a 1x1 mesh grid
						drawRegion(
							newSource,
							newDest,
							*destClip,
							color,
							fOpacity,
							antiAlias,
							1,
							1);
					}
				}
			}
		}
		else
		{
			// Find the X of the dest region our point falls in (0, 1 or 2)
			// NB: we assume the dest rectangle DOES contain the point
			for (int x = 0; x < 3; x++)
			{
				if (x == 2 || hitPos->x < l_destXCoords[x + 1])
				{
					// Find the Y of the dest region our point falls in (0, 1 or 2)
					for (int y = 0; y < 3; y++)
					{
						if (y == 2 || hitPos->y < l_destYCoords[y + 1])
						{
							// Calc the fractional position within this region
							LcTScalar fx = LcTScalar(hitPos->x - l_destXCoords[x]) / l_destWidth[x];
							LcTScalar fy = LcTScalar(hitPos->y - l_destYCoords[y]) / l_destHeight[y];
									  fy = 1 - fy;

							// Map this to the source region
							return isPointTransparent(
								(int)(l_srcXCoords[x] + (fx * l_srcWidth[x])),
								(int)(l_srcYCoords[y] + (fy * l_srcHeight[y])));
						}
					}
				}
			}
		}
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
	Sets the border margins for this image.

	<p>
	Margins, specified in image pixel co-ordinates, are used to split
	an image up into nine sections.
	</p>

	<pre>
	 left
	+---+---+---+
	| 1 | 2 | 3 |
	+---+---+---+ < top
	| 4 | 5 | 6 |
	+---+---+---+ < bottom
	| 7 | 8 | 9 |
	+---+---+---+
		  ^
		right

	</pre>

	When the image is drawn on the screen with a different size or scale, the
	margins determine how the image is stretched.

	<pre>

		  Extent x 2				           Scale x 2

	+---+---------------+---+            +-------+-------+-------+
	| 1 |       2       | 3 |            |       |       |       |
	+---+---------------+---+            |   1   |   2   |   3   |
	|   |               |   |            |       |       |       |
	|   |               |   |            +-------+-------+-------+
	|   |               |   |            |       |       |       |
	| 4 |       5       | 6 |            |   4   |   5   |   6   |
	|   |               |   |            |       |       |       |
	|   |               |   |            +-------+-------+-------+
	|   |               |   |            |       |       |       |
	+---+---------------+---+            |   7   |   8   |   9   |
	| 7 |       8       | 9 |            |       |       |       |
	+---+---------------+---+            +-------+-------+-------+

	</pre>

	<p>
	With no scale, the four corners (1, 3, 7, 9) will be drawn at original size
	with the other regions stretching to fit between them. With scale, the four
	corners will be drawn with the relevant scale, then the other regions will
	be drawn around them.
	</p>

	<p>
	This technique allows you to paint widgets using bitmaps that have special
	rounded edges or similar artifacts that would not respond well to scaling,
	but require the image to be resized to draw arbitrarily sized widgets such
	as buttons.
	</p>

	<p>
	Opposing margins must not overlap, and room must be allowed for all regions
	to have a distinct area. Formally "(left+right) < width" and
	"(top+bottom) < height".
	</p>

	@see #getMarginRight
	@see #getMarginLeft
	@see #getMarginTop
	@see #getMarginBottom
*/
LC_EXPORT void LcCBitmap::setMargins(int left, int right, int top, int bottom)
{
	m_iLeftMargin   = left;
	m_iRightMargin  = right;
	m_iTopMargin	= top;
	m_iBottomMargin = bottom;
}

/*-------------------------------------------------------------------------*//**
	Return the size of a single frame within this image.

	@return The size of a single frame within this image.
	@see #setFrameCount
	@see #getFrameCount
*/
LC_EXPORT_VIRTUAL LcTPixelDim LcCBitmap::getSize()
{
	forceLoad();

	LcTPixelDim frameSize = m_size;
	frameSize.height /= m_iFrameCount;
	return frameSize;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCBitmap::draw(
	int 					frameNo,
	const LcTPlaneRect& 	dest,
	const LcTPixelRect&		pixClip,
	LcTColor				color,
	LcTScalar 				fOpacity,
	bool					antiAlias,
	int						meshGridX,
	int						meshGridY)
{
	tileOp(
		true,
		frameNo,
		dest,
		&pixClip,
		color,
		fOpacity,
		0,
		antiAlias,
		meshGridX,
		meshGridY);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCBitmap::isTransparent(
	int						frameNo,
	const LcTPlaneRect&		dest,
	const LcTVector&		scale,
	const LcTPlaneRect*		clip,
	const LcTVector&		hitPos)
{
	LC_UNUSED(scale)
	LC_UNUSED(clip)
	
	return tileOp(false, frameNo, dest, NULL, 0, 0, &hitPos, false, 1, 1);
}
