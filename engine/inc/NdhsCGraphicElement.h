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
#ifndef NdhsCGraphicElementH
#define NdhsCGraphicElementH

#include "inflexionui/engine/inc/NdhsCElement.h"
#include "inflexionui/engine/inc/NdhsCPageTemplate.h"

/*-------------------------------------------------------------------------*//**
	NdhsCGraphicElement is the class for elements that have a graphical representation
	on a menu page.
*/
class NdhsCGraphicElement : public NdhsCElement
{
private:
	LcTmOwner<NdhsCMenuWidgetFactory::CSettings> m_settings;
	NdhsCPageTemplate::CGraphicElement*	m_GE;

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	LcTmOwner<NdhsCExpression>  m_useCustomEffect;
	bool 						m_bUseCustomEffect;
	bool						m_customEffectDirty;
#endif

#ifdef IFX_USE_PLUGIN_ELEMENTS
	// event handler object pointer
	NdhsCPlugin*				m_pPlugin;

	// event handler object handle
	NdhsCPlugin::NdhsCPluginHElement* m_hPluginElement;
#endif

	// Two-phase construction
								NdhsCGraphicElement();
	void						construct(
									ENdhsObjectType						use,
									const LcTmString&					className,
									NdhsCMenu*							menu,
									NdhsCMenuItem*						menuItem,
									NdhsCTemplate::CGraphicElement*		pGE,
									NdhsCPageManager*					pPageManager,
									NdhsCElementGroup*					pPage,
									int									stackLevel,
									NdhsCManifest*						parentPaletteMan,
									NdhsCElementGroup*					parentGroup);

public:
#ifdef IFX_SERIALIZATION
	static	NdhsCGraphicElement*	loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual	SerializeHandle			serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual	void					deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */

	static LcTaOwner<NdhsCGraphicElement> create(
									ENdhsObjectType						use,
									const LcTmString&					className,
									NdhsCMenu*							menu,
									NdhsCMenuItem*						menuItem,
									NdhsCTemplate::CGraphicElement*		pGE,
									NdhsCPageManager*					pPageManager,
									NdhsCElementGroup*					pPage,
									int									stackLevel,
									NdhsCManifest*						parentPaletteMan,
									NdhsCElementGroup*					parentGroup);

	virtual						~NdhsCGraphicElement();
	virtual void				reloadElement();

	virtual inline LcTVector&	getDefaultExtent()					{ return m_settings->defaultExtent; }

#ifdef IFX_USE_PLUGIN_ELEMENTS
	virtual bool				processKeyDownEvent(int key);
	virtual bool				processKeyUpEvent(int key);
	virtual NdhsCPlugin::NdhsCPluginHElement* getPluginElement()	{ return m_hPluginElement; }
	virtual bool				setElementFocus(bool enableFocus);
#endif
	
	inline	NdhsCManifest*		getParentPaletteMan()				{ return (m_settings ? m_settings->parentPaletteMan : NULL); }

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	// NdhsIExpressionObserver implementation
	virtual void 				expressionDirty(NdhsCExpression* expr);
	virtual void 				expressionDestroyed(NdhsCExpression* expr);

	// NdhsILaundryItem support
	virtual bool 				cleanLaundryItem(LcTTime timestamp);
#endif

};
#endif // NdhsCGraphicElementH

// End of File
