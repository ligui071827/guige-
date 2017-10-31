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
#ifndef NdhsCPluginElementDirectViewH
#define NdhsCPluginElementDirectViewH

#include "inflexionui/engine/inc/LcTPlaneRect.h"
#include "inflexionui/engine/inc/NdhsCPluginElementView.h"
class NdhsCPageManager;

/*-------------------------------------------------------------------------*//**
	Direct rendering element view
*/
class NdhsCPluginElementDirectView : public NdhsCPluginElementView
{
	// Embedded widgets use the ImageLayer class, which needs a LcIImage object
	// attaching to it.  We use this subclass to allow us to forward on some
	// key requests to the plugin.
	class CImageWrapper : public LcCBase, public LcIImage
	{
		LcCSpace*		m_space;
		NdhsCPluginElementDirectView* m_pOwner;

	protected:
		// Construction
		CImageWrapper(NdhsCPluginElementDirectView* pOwner);

	public:
		static LcTaOwner<CImageWrapper> create(NdhsCPluginElementDirectView* pOwner);

		// Destruction
		virtual							~CImageWrapper()			{}

		// LcIImage interface
		virtual LcCSpace*				getSpace()					{ return m_space; }
		virtual LcTPixelDim				getSize()					{ return m_pOwner->getViewDim(); }
		virtual bool					isOpaque();
		virtual bool					canBeClipped()				{ return false; }

		// Stubbed - blackout layer functionality should ensure that this will never
		// get called anyway!
		virtual void					draw(
											const LcTPlaneRect& dest,
											const LcTPixelRect& clip,
											LcTColor			color,
											LcTScalar			fOpacity,
											bool				antiAlias,
											int					meshGridX,
											int					meshGridY) {LC_UNUSED(dest); LC_UNUSED(clip); LC_UNUSED(color); LC_UNUSED(fOpacity); 
																			LC_UNUSED(antiAlias); LC_UNUSED(meshGridX); LC_UNUSED(meshGridY); }

		virtual bool					isTransparent(
											const LcTPlaneRect&	dest,
											const LcTVector&	scale,
											const LcTPlaneRect*	clip,
											const LcTVector&	hitPos);
	};

	LC_DECLARE_RTTI(NdhsCPluginElementDirectView, NdhsCPluginElementView)

	// Wrapper round conceptual embedded image
	LcTmOwner<CImageWrapper>	m_image;

	// Output rect relative to tl corner of canvas
	LcTPixelRect				m_recOutput;

	IFX_DIRECT_RENDER_CONTEXT	m_context;

	LcwCPlugin*					m_attachedWidget;

	// Helpers
	void 						calcBoundingBoxCanvas(LcCWidget* pWidget, LcTPixelRect& rect);
	LcTPlaneRect 				calcBoundingBox3D(LcCWidget* pWidget);

	// Allow only 2-phase construction
								NdhsCPluginElementDirectView(NdhsCPlugin::NdhsCPluginHElement* w);
	void						construct();

public:

	static LcTaOwner<NdhsCPluginElementDirectView> create(NdhsCPlugin::NdhsCPluginHElement* w);
	virtual 					~NdhsCPluginElementDirectView();

	virtual bool				onWidgetPlacementChanged(LcwCPlugin* pWidget);
	virtual void				attachWidget(LcwCPlugin* pWidget);
	virtual void				detachWidget(LcwCPlugin* pWidget);
	virtual void 				prepareForPaint(LcwCPlugin* pCurrentWidget);

	virtual bool				usesCanvasUnits()		{ return true; }

	LcTPixelDim					getViewDim()			{ return LcTPixelDim(m_recOutput.getWidth(), m_recOutput.getHeight()); }
	virtual void*				getRenderContext()		{ return (void*)&m_context; }
};

#endif // NdhsCPluginElementDirectViewH
