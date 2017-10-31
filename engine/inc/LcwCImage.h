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
#ifndef LcwCImageH
#define LcwCImageH

#include "inflexionui/engine/inc/LcCWidget.h"

/*-------------------------------------------------------------------------*//**
	A simple, generic image widget.  The location
	specifies the image top left corner and the scale determines the dimensions.
	By default an image will discard mouse events that occur over it.  However
	it can be configured to trap and discard all events that occur at its z-plane,
	which is useful for showing the background of modal message boxes.
*/
class LcwCImage : public LcCWidget
{
	LC_DECLARE_RTTI(LcwCImage, LcCWidget)

private:

	LcIImage*						m_image;
	
	// Cached intermediate info
	LcTPlaneRect					m_recPaint;

	// Configured info
	bool							m_keepAspectRatio;
	bool							m_antiAlias;
	int								m_meshGridX;
	int								m_meshGridY;

	// LcCWidget methods
	LC_VIRTUAL		void			onRealize();
	LC_VIRTUAL		void			onWantBoundingBox();
	LC_VIRTUAL		void			onPaint(const LcTPixelRect& clip);
#if defined(LC_PLAT_OGL)
	LC_VIRTUAL		void			onPaintOpaque(const LcTPixelRect& clip);
#endif

protected:
					void			setAntiAlias(bool antiAlias)	{ m_antiAlias = antiAlias; }
					void			setMeshGrid(int meshGridX, int meshGridY);

	// Derived classes that override this need to call base
	LC_VIRTUAL		void			onPrepareForPaint();

	// Allow only 2-phase construction
	LC_IMPORT						LcwCImage();

LC_PROTECTED_INTERNAL_PUBLIC:
	// This indicates whether the widget entirely obscures anything behind
	// it within its bounding box and is used for optimization on repaint
	LC_VIRTUAL		bool			isOpaque() { return m_image ? (m_image->isOpaque() && (m_useCachedOpacity ? m_cachedOpacity >= LC_OPAQUE_THRESHOLD : getOverallOpacity() >= LC_OPAQUE_THRESHOLD)) : false; }
	// Indicates whether clipping in pixel space can be applied on paint
	LC_VIRTUAL		bool			canBeClipped() { return m_image? m_image->canBeClipped() : false; }

public:

	// Construction
	LC_IMPORT		static LcTaOwner<LcwCImage> create();

	// Configuration
					void			setImage(LcIImage* img);
	inline			LcIImage*		getImage() 						{ return m_image; }
	inline			void			setKeepAspectRatio(bool b)		{ m_keepAspectRatio = b; }
	LC_VIRTUAL 		void			reloadResources();
	LC_VIRTUAL		bool			contains(const LcTVector& loc, LcTScalar expandRectEdge);
					bool			isAntiAliased()			{ return m_antiAlias; }
};

#endif
