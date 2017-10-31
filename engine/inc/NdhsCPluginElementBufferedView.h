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
#ifndef NdhsCPluginElementBufferedViewH
#define NdhsCPluginElementBufferedViewH

#include "inflexionui/engine/inc/LcTPlaneRect.h"
#include "inflexionui/engine/inc/NdhsCPluginElementView.h"
class NdhsCPageManager;
class NdhsCMemoryImage;

/*-------------------------------------------------------------------------*//**
	Normal Buffered rendering element view
*/
class NdhsCPluginElementBufferedView : public NdhsCPluginElementView
{
	LC_DECLARE_RTTI(NdhsCPluginElementBufferedView, NdhsCPluginElementView)

	// pointer to memory image that will be rendered to by the plug-in element
	LcTmOwner<NdhsCMemoryImage> m_image;

	// pointer to buffered rendering context struct - shared between this view and the plug-in element
	LcTmAlloc<IFX_BUFFERED_RENDER_CONTEXT> m_context;

	// if true stop the widget from preparing the layer to draw
	bool						m_skipDrawing;

    // Indicate if this is an internal or external element.
    IFX_ELEMENT_MODE            m_mode;

    // Pointer to element buffer in external mode
	void*						m_buffer;

	bool						m_repaint;

	// Allow only 2-phase construction
								NdhsCPluginElementBufferedView(NdhsCPlugin::NdhsCPluginHElement* w,
															IFX_ELEMENT_MODE elementMode);
	void						construct();

public:

	static LcTaOwner<NdhsCPluginElementBufferedView> create(NdhsCPlugin::NdhsCPluginHElement* w,
															IFX_ELEMENT_MODE elementMode);
	virtual 					~NdhsCPluginElementBufferedView();

	virtual void 				prepareForPaint(LcwCPlugin* pCurrentWidget);
	virtual bool				paintElement();
	bool						hasImage()					{return (m_image.ptr() != NULL);}
	virtual	void				releaseResources();
	virtual	void				loadResources();
	virtual void				attachWidget(LcwCPlugin* pWidget);
	virtual void				detachWidget(LcwCPlugin* pWidget);
	virtual bool				setBufferSize(int width, int height);
	virtual bool				setBufferFormat(IFX_BUFFER_FORMAT format);
	virtual void				refreshBuffer();

	virtual void				setTranslucency(bool isTranslucent);
	virtual void*				getRenderContext()			{ return (void*)m_context; }
};

#endif // NdhsCPluginElementBufferedViewH
