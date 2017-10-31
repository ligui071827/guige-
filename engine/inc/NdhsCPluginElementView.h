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
#ifndef NdhsCPluginElementViewH
#define NdhsCPluginElementViewH

#include "inflexionui/engine/inc/LcTPlaneRect.h"
#include "inflexionui/engine/inc/LcwCPlugin.h"


class NdhsCPageManager;

/*-------------------------------------------------------------------------*//**
	Embedded Element View base class
*/
class NdhsCPluginElementView : public LcCBase
{
	LC_DECLARE_RTTI_BASE(NdhsCPluginElementView)
private:
	bool								m_hasPreMultipliedAlpha;

	// plugin element handle
	NdhsCPlugin::NdhsCPluginHElement*	m_hPluginElement;

protected:
	// Cannot be instantiated, so constructor is protected
								NdhsCPluginElementView(NdhsCPlugin::NdhsCPluginHElement* v);

	inline NdhsCPlugin::NdhsCPluginHElement* getPluginHandle()	{ return m_hPluginElement; }

public:
	virtual 					~NdhsCPluginElementView();

	virtual bool				onWidgetPlacementChanged(LcwCPlugin* pWidget) { return true; }

	virtual bool				paintElement()								{ return true; }
	virtual bool				viewActive()								{ return m_hPluginElement->isActive(); }
	virtual void 				prepareForPaint(LcwCPlugin* pWidget)		{}
	virtual bool				setBufferSize(int width, int height) 		{return true;}
	virtual bool				setBufferFormat(IFX_BUFFER_FORMAT format)	{return true;}
	virtual void				refreshBuffer()								{}
	virtual	void 				loadResources()								{}
	virtual	void				releaseResources()							{}
	virtual bool				usesCanvasUnits()							{ return false; }

	virtual void				setTranslucency(bool isTranslucent)			{ LC_UNUSED(isTranslucent); }
	virtual void				setPreMultipliedAlpha(bool value)			{ m_hasPreMultipliedAlpha = value; }
	virtual bool				getPreMultipliedAlpha()						{ return m_hasPreMultipliedAlpha; }
	virtual void				attachWidget(LcwCPlugin* pWidget)			= 0;
	virtual void				detachWidget(LcwCPlugin* pWidget)			= 0;
	virtual void*				getRenderContext()							= 0;
};

#endif// NdhsCPluginElementViewH
