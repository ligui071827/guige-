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
#ifndef LcwCPluginH
#define LcwCPluginH


/*-------------------------------------------------------------------------*//**
	A widget encapsulating a rectangular region of screen that will be rendered
	by a plugin entity (and not NDE).
*/
class LcwCPlugin : public LcCWidget
{
	LC_DECLARE_RTTI(LcwCPlugin, LcCWidget)

private:
	LcIImage*					m_image;
	// View rect in local coords
	LcTPlaneRect				m_recPaint; 
	bool						m_antiAlias;
	int							m_meshGridX;
	int							m_meshGridY;

	LC_VIRTUAL		void		onWantBoundingBox();

protected:

	// LcCWidget methods
	LC_VIRTUAL		void		onRealize() = 0;
	LC_VIRTUAL		void		onRetire() = 0;
	LC_VIRTUAL		void		onPlacementChange(int mask) = 0;
	LC_VIRTUAL		void		onPrepareForPaint() = 0;
	LC_VIRTUAL		void		doPrepareForPaint() = 0;
	LC_VIRTUAL 		void 		onPaint(const LcTPixelRect& clip);
#if defined(LC_PLAT_OGL)
	LC_VIRTUAL		void		onPaintOpaque(const LcTPixelRect& clip);
#endif
	LC_VIRTUAL		bool		contains(const LcTVector& loc, LcTScalar expandRectEdge);
					void		setAntiAlias(bool antiAlias)		{ m_antiAlias = antiAlias; }
					void		setMeshGrid(int meshGridX, int meshGridY);

	// Allow only 2-phase construction
	LC_IMPORT					LcwCPlugin();


LC_PROTECTED_INTERNAL_PUBLIC:
	// This indicates whether the widget entirely obscures anything behind
	// it within its bounding box and is used for optimization on repaint
	LC_VIRTUAL		bool		isOpaque() { return m_image? m_image->isOpaque() : false; }
	// Indicates whether clipping in pixel space can be applied on paint
	LC_VIRTUAL		bool		canBeClipped() { return m_image? m_image->canBeClipped() : false; }

public:

	// Configuration
				void			setImage(LcIImage* img);
	inline		LcIImage*		getImage()						{ return m_image; }
				void			setPaintRect(const LcTPlaneRect& bbox);
				bool			isAntiAliased()					{ return m_antiAlias; }

	// This provides a mechanism for the view to set the blackout status of the widget
	// depending on whether the current view is 'direct' mode or not.
				void			setBlackoutStatus(bool isBlackout) { setBlackout(isBlackout); revalidate();}
};

#endif // LcwCEmbeddedObjectH
