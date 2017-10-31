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
#ifndef NdhsCPluginElementH
#define NdhsCPluginElementH

#include "inflexionui/engine/inc/NdhsCElement.h"
#include "inflexionui/engine/inc/NdhsCPageTemplate.h"

/*-------------------------------------------------------------------------*//**
	NdhsCPluginElement is the class for elements that have a plug-in-rendered
	graphical presence on the screen.
*/
class NdhsCPluginElement : public NdhsCElement
{
private:
	LcTmOwner<NdhsCMenuWidgetFactory::CSettings> m_settings;
	LcTPlacement				m_widgetPlacement;
	ENdhsWidgetType				m_widgetType;
	bool						m_makeVisibleOnStaticCompletion;

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	LcTmOwner<NdhsCExpression>  m_useCustomEffect;
	bool 						m_bUseCustomEffect;
	bool						m_customEffectDirty;
#endif

	NdhsCPageTemplate::CPluginElement*	m_PE;
	LcTmOwner<NdhsCExpression>  m_eventHandler;
	LcTmString					m_eventHandlerStr;
	bool                        m_eventHandlerExprDirty;
	bool                        m_customEffectExprDirty;
	bool						m_reloadRequired;

	// Two-phase construction
								NdhsCPluginElement();
	void						construct(
									ENdhsObjectType						use,
									const LcTmString&					className,
									NdhsCMenu*							menu,
									NdhsCMenuItem*						menuItem,
									NdhsCTemplate::CPluginElement*		pPE,
									NdhsCPageManager*					pPageManager,
									NdhsCElementGroup*					pPage,
									int									stackLevel,
									NdhsCElementGroup*					parentGroup);

public:

	static LcTaOwner<NdhsCPluginElement> create(
									ENdhsObjectType						use,
									const LcTmString&					className,
									NdhsCMenu*							menu,
									NdhsCMenuItem*						menuItem,
									NdhsCTemplate::CPluginElement*		pPE,
									NdhsCPageManager*					pPageManager,
									NdhsCElementGroup*					pPage,
									int									stackLevel,
									NdhsCElementGroup*					parentGroup);

	virtual						~NdhsCPluginElement();
	virtual void				reloadElement();
	virtual void 				updatePosition(
									ENdhsAnimationType 					animType,
									LcTScalar 							position,
									bool 								positionIncreasing,
									bool 								updateCache,
									bool 								forceNonSketchy,
									bool 								aggregateAnimating,
									bool 								finalFrame);

	virtual inline LcTVector&	getDefaultExtent()			{ return m_settings->defaultExtent; }

	virtual bool				setElementFocus(bool enableFocus);
	virtual bool				processKeyDownEvent(int key);
	virtual bool				processKeyUpEvent(int key);

#ifdef LC_USE_STYLUS
	virtual bool				onMouseDown(const LcTPixelPoint& pt);
	virtual bool				onMouseMove(const LcTPixelPoint& pt);
	virtual bool				onMouseUp(const LcTPixelPoint& pt);
	virtual void				onMouseCancel();
#endif

	virtual void 				startStaticAnimations();
	virtual void 				stopStaticAnimations();

	virtual void  				onSuspend();
	virtual void  				onResume();
	virtual bool				forceDestroyOnSuspend();

#ifdef IFX_SERIALIZATION
	static	NdhsCPluginElement*	loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual	SerializeHandle		serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual	void				deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */


	// NdhsIExpressionObserver implementation
	virtual void 				expressionDirty(NdhsCExpression* expr);
	virtual void 				expressionDestroyed(NdhsCExpression* expr);

	// NdhsILaundryItem support
	virtual bool 				cleanLaundryItem(LcTTime timestamp);
};
#endif // NdhsCPluginElementH
// End of File
