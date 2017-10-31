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
#ifndef NdhsCPluginElementOGLTextureViewH
#define NdhsCPluginElementOGLTextureViewH

#if defined(LC_PLAT_OGL)

#include "inflexionui/engine/inc/LcTPlaneRect.h"
#include "inflexionui/engine/inc/NdhsCPluginElementView.h"
class NdhsCPageManager;
class NdhsCMemoryImage;

/*-------------------------------------------------------------------------*//**
	External Buffered rendering element view
*/
class NdhsCPluginElementOGLTextureView : public NdhsCPluginElementView
{
	LC_DECLARE_RTTI(NdhsCPluginElementOGLTextureView, NdhsCPluginElementView)

	// pointer to memory image that will be rendered to by the plug-in element
	LcTmOwner<NdhsCMemoryImage> m_image;

	// pointer to opengl rendering context struct - shared between this view and the plug-in element
	LcTmAlloc<IFX_OPENGL_RENDER_CONTEXT> m_context;

	// if true stop the widget from preparing the layer to draw
	bool						m_skipDrawing;

	GLuint						m_texHandles[IFX_OGL_MAX_TEXTURES];
	
	bool 						m_repaint;

	// Allow only 2-phase construction
								NdhsCPluginElementOGLTextureView(NdhsCPlugin::NdhsCPluginHElement* v);
	void						construct();

public:

	static LcTaOwner<NdhsCPluginElementOGLTextureView> create(NdhsCPlugin::NdhsCPluginHElement* v);
	virtual 					~NdhsCPluginElementOGLTextureView();

	virtual void 				prepareForPaint(LcwCPlugin* pCurrentWidget);
	virtual bool				paintElement();
	bool						hasImage()					{return (m_image.ptr() != NULL);}
	virtual void				attachWidget(LcwCPlugin* pWidget);
	virtual void				detachWidget(LcwCPlugin* pWidget);
	virtual bool				setBufferSize(int width, int height);
	virtual bool				setBufferFormat(IFX_BUFFER_FORMAT format);
	virtual void				refreshBuffer();
	virtual	void 				loadResources();
	virtual	void				releaseResources();

	virtual void				setTranslucency(bool isTranslucent);
	virtual void*				getRenderContext()			{ return (void*)m_context; }
};

#endif // #if defined(LC_PLAT_OGL)

#endif // NdhsCPluginElementOGLTextureViewH
