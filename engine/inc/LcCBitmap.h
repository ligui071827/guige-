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
#ifndef LcCBitmapH
#define LcCBitmapH

#include "inflexionui/engine/inc/LcTPixelDim.h"
#include "inflexionui/engine/inc/LcTPixelRect.h"
#include "inflexionui/engine/inc/LcTPlaneRect.h"
#include "inflexionui/engine/inc/LcTScalarRect.h"
#include "inflexionui/engine/inc/LcTVector.h"
#include "inflexionui/engine/inc/LcCSpace.h"
#include "inflexionui/engine/inc/LcIImage.h"
class LcCSpace;

#ifdef LC_USE_XML
	class LcCXmlElem;
#endif

// Note that isOpaque() should only return true if no background pixels can show
// through anywhere in the bounding box.  The setOpacity() method might be implemented
// using fade, in which case it won't affect true opacity.  Also, a masked image is
// never opaque regardless of the opacity value set


/*-------------------------------------------------------------------------*//**
	Generic image class to be supported by platform-specific image implementations.

	<p>
	Images should be created by calling {@link LcCSpace#loadImage LcCSpace::loadImage()}.
	Images are reference counted. If you wish to keep a pointer to an image you should
	call {@link LcCBitmap#acquire acquire()} on it. When you are finished with the pointer
	you should call {@link LcCBitmap#release release()}.
	</p>

	<p>
	Platform specific image implementations <b>must implement</b>
	<UL>
	<LI>{@link #drawRegion drawRegion()}
	<LI>{@link #isPointTransparent isPointTransparent()}
	</UL>

	Platform specific image implementations <b>must call</b>
	<UL>
	<LI>{@link #setActualSize setActualSize()}
	</UL>

	Platform specific image implementations <b>may implement</b>
	<UL>
	<LI>{@link #forceLoad forceLoad()}
	<LI>{@link #isOpaque  isOpaque()}
	<LI>{@link #draw(int,constLcTPixelRect&,constLcTVector&,float) draw()}
	<LI>{@link #isTransparent(int,constLcTPixelRect&,constLcTVector&,int,int) isTransparent()}
	</UL>
	</p>
*/
class LcCBitmap : public LcCBase, public LcIImage
{
private:

	LcCSpace*						m_space;
	int								m_iRefCount;

	// Image drawing properties
	int								m_iLeftMargin;
	int								m_iRightMargin;
	int								m_iTopMargin;
	int								m_iBottomMargin;
	int								m_iFrameCount;
	LcTPixelDim						m_size;
	LcTmString						m_filePath;

	// Helper - does both drawing and transparency testing
					bool			tileOp(
										bool				bDraw, // false = transparency test
										int 				frameNo,
										const LcTPlaneRect&	dest,
										const LcTPixelRect*	destClip,
										LcTColor			color,
										LcTScalar			fOpacity,
										const LcTVector*	hitPos,
										bool				antiAlias,
										int					meshGridX,
										int					meshGridY);

protected:

	bool							m_isTranslucent;

	// Either draw(frameNo) or this must be implemented by derived class
	// NB: in 3D modes, the color passed into drawRegion will already have been
	// adjusted to apply lighting if the object is at an angle to the viewer
	virtual			void			drawRegion(
										const LcTScalarRect&	src,
										const LcTPlaneRect&		dest,
										const LcTPixelRect&		clip,
										LcTColor				color,
										LcTScalar				opacity,
										bool					antiAlias,
										int						meshGridX,
										int						meshGridY) { LC_UNUSED(src) LC_UNUSED(dest) LC_UNUSED(clip) LC_UNUSED(color) 
																			 LC_UNUSED(opacity) LC_UNUSED(antiAlias) LC_UNUSED(meshGridX) LC_UNUSED(meshGridY) }

	// Can be implemented by derived class.
	// The derived image class has the option of not loading the image in the
	// constructor. When any methods rely on the image having been loaded
	// already, they will call the forceLoad() method first. If the derived
	// image class wishes to delay loading the image, it must load it in
	// forceLoad(). This technique is useful for minimizing the memory
	// footprint, especially when some factories may load many images which
	// may never be used.
	virtual			void			forceLoad()								{}

	// isPointTransparent must be implemented by the derived class to return
	// whether the point is transparent.
	// createTransparencyData can be implemented on platforms that require
	// some pre-processing to be done at the point the bitmap canvas is
	// created. It takes NDI data as its input.
	virtual			void			createTransparencyData(LcTByte* data, int numberPixels)	{ LC_UNUSED(data) LC_UNUSED(numberPixels) }
	virtual			bool			isPointTransparent(int iX, int iY)		= 0;

	// Must be called by derived class
	inline			void			setActualSize(LcTPixelDim size)  { m_size = size; }

	// Abstract so keep constructor protected
	LC_IMPORT						LcCBitmap(LcCSpace* sp);

public:

	// Destruction
	virtual							~LcCBitmap();

	// Reference counting
	virtual			void			acquire()						{ m_iRefCount++; }
	LC_VIRTUAL		bool			release();

	// Multiple frames (arranged vertically)
	inline			void			setFrameCount(int frameCount)	{ m_iFrameCount = frameCount; }
	inline			int				getFrameCount()					{ return m_iFrameCount; }

	// Might be useful for layout
	LC_IMPORT		void			setMargins(int left, int right, int top, int bottom);
	inline			int				getMarginLeft()		{ return m_iLeftMargin; }
	inline			int				getMarginRight()	{ return m_iRightMargin; }
	inline			int				getMarginTop()		{ return m_iTopMargin; }
	inline			int				getMarginBottom()	{ return m_iBottomMargin; }

	// LcIImage interface
	inline			LcCSpace*		getSpace()			{ return m_space; }
	LC_VIRTUAL		LcTPixelDim		getSize();
	virtual			bool			isOpaque()			{ return false; }
	virtual			void			draw(
										const LcTPlaneRect& dest,
										const LcTPixelRect& clip,
										LcTColor			color,
										LcTScalar 			fOpacity,
										bool				antiAlias,
										int					meshGridX,
										int					meshGridY)
											{ draw(0, dest, clip, color, fOpacity, antiAlias, meshGridX, meshGridY); }
	virtual			bool			isTransparent(
										const LcTPlaneRect&	dest,
										const LcTVector&	scale,
										const LcTPlaneRect*	clip,
										const LcTVector&	hitPos)
											{ return isTransparent(0, dest, scale, clip, hitPos); }

	// Draw and hit test functions that account for frame number
	// NB: frame number is passed only by LcCBitmapFrame, and otherwise defaults to 0
	LC_VIRTUAL		void			draw(
										int 				frameNo,
										const LcTPlaneRect& dest,
										const LcTPixelRect& clip,
										LcTColor			color,
										LcTScalar 			fOpacity,
										bool				antiAlias,
										int					meshGridX,
										int					meshGridY);
	LC_VIRTUAL		bool			isTransparent(
										int					frameNo,
										const LcTPlaneRect&	dest,
										const LcTVector&	scale,
										const LcTPlaneRect*	clip,
										const LcTVector&	hitPos);
	LC_VIRTUAL		bool  			isTranslucent() { return m_isTranslucent; }
					void			setFilePath(LcTaString file){m_filePath=file;}
					LcTaString		getFilePath(){return m_filePath;}
	LC_VIRTUAL		void  			releaseResources() {}
	LC_VIRTUAL		void  			reloadResources() {}
};

#endif // LcCBitmapH
