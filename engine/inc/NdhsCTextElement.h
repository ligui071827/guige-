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
#ifndef NdhsCTextElementH
#define NdhsCTextElementH

#include "inflexionui/engine/inc/NdhsCElement.h"
#include "inflexionui/engine/inc/NdhsCPageTemplate.h"

/*-------------------------------------------------------------------------*//**
	NdhsCTextElement is the class for elements that represent a formatted
	textual string.
*/
class NdhsCTextElement : public NdhsCElement
{
private:
	bool						m_hasFocus;

	LcTmOwner<NdhsCMenuWidgetFactory::CSettings> m_settings;
	NdhsCTemplate::CTextElement* m_TE;

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	LcTmOwner<NdhsCExpression>  m_useCustomEffect;
	bool 						m_bUseCustomEffect;
	bool						m_customEffectDirty;
#endif

	LcTmOwner<NdhsCExpression>  m_verticalMarqueeExpr;
	LcTmOwner<NdhsCExpression>  m_horizontalMarqueeExpr;

	// marquee attributes
	NdhsCScrollPosField*			m_marqueeField;	
	bool							m_verticalMarquee;
	bool							m_horizontalMarquee;
	int								m_scrollVelocity;
	int								m_currentDelta;
	bool							m_verticalMarqDirty;
	bool							m_horizontalMarqDirty;
	bool							m_marqueeFieldPending;

#ifdef IFX_USE_PLUGIN_ELEMENTS
	// event handler object pointer
	NdhsCPlugin*				m_pPlugin;

	// event handler object handle
	NdhsCPlugin::NdhsCPluginHElement*	m_hPluginElement;
#endif

	LcTmOwner<NdhsCExpression>  m_fontExpr;
	bool						m_fontExpressionDirty;
	bool						m_fontExpressionDirtyReload;
	// Two-phase construction
								NdhsCTextElement();
	void						construct(
									ENdhsObjectType						use,
									const LcTmString&					className,
									NdhsCMenu*							menu,
									NdhsCMenuItem*						menuItem,
									NdhsCTemplate::CTextElement*		pTE,
									NdhsCPageManager*					pPageManager,
									NdhsCElementGroup*					pPage,
									int									stackLevel,
									NdhsCManifest*						parentPaletteMan,
									NdhsCElementGroup*					parentGroup);

public:
#ifdef IFX_SERIALIZATION
	static	NdhsCTextElement*		loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual	SerializeHandle			serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual	void					deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */

	static LcTaOwner<NdhsCTextElement> create(
									ENdhsObjectType						use,
									const LcTmString&					className,
									NdhsCMenu*							menu,
									NdhsCMenuItem*						menuItem,
									NdhsCTemplate::CTextElement*		pTE,
									NdhsCPageManager*					pPageManager,
									NdhsCElementGroup*					pPage,
									int									stackLevel,
									NdhsCManifest*						parentPaletteMan,
									NdhsCElementGroup*					parentGroup);

	virtual						~NdhsCTextElement();
	virtual void				reloadElement();

#ifdef IFX_USE_PLUGIN_ELEMENTS
	virtual bool		processKeyDownEvent(int key);
	virtual bool		processKeyUpEvent(int key);
	virtual NdhsCPlugin::NdhsCPluginHElement* getPluginElement()	{ return m_hPluginElement; }
	virtual bool		setElementFocus(bool enableFocus);
#endif
	
	inline	NdhsCManifest*		getParentPaletteMan()				{ return (m_settings ? m_settings->parentPaletteMan : NULL); }

	// NdhsIExpressionObserver implementation
	virtual void 				expressionDirty(NdhsCExpression* expr);
	virtual void 				expressionDestroyed(NdhsCExpression* expr);

	virtual void				startStaticAnimations();
	virtual void				stopStaticAnimations();
			void				stopMarqueeAnimations();

	virtual bool				onWidgetEvent(LcTWidgetEvent* e);
			void				startMarqueeAnimation();

	// NdhsILaundryItem support
	virtual bool 				cleanLaundryItem(LcTTime timestamp);

			void				startHorizontalMarqueeAnimation(LcTScalar width, bool textRTL);
			void				startVerticalMarqueeAnimation(LcTScalar height);
			void				fieldValueUpdated(NdhsCField* field);

};
#endif // NdhsCTextElementH

