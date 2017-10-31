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
#ifndef NdhsCElementH
#define NdhsCElementH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTString.h"
#include "inflexionui/engine/inc/NdhsCTransitionAgent.h"
#include "inflexionui/engine/inc/NdhsCElementGroup.h"
#include "inflexionui/engine/inc/NdhsCField.h"
#include "inflexionui/engine/inc/LcCSerializeMaster.h"

/*-------------------------------------------------------------------------*//**
	NdhsCElement is the base class for specialist elements that represent user
	experience items on a particular menu page.
*/

class NdhsCElement : public LcCBase, public NdhsCField::IObserver, public NdhsCTemplate::CIElementDisplacementObserver, public NdhsIExpressionObserver, public NdhsILaundryItem, public ISerializeable
{

private:

	typedef LcTmArray<NdhsCField*> TmMFieldTokens;

	LcTmOwner<LcCWidget>			m_widget;
	ENdhsElementType				m_elementType;
	LcTmString						m_elementParent;
	ENdhsObjectType					m_elementUsage;
	bool							m_isDetail;
	LcTmString						m_elementClassName;
	LcTmString						m_packagePath;
	LcTmOwner<NdhsCExpression>		m_resourceData;
	LcTmOwner<NdhsCExpression>		m_enableFocus;
	NdhsCMenu*						m_menu;
	NdhsCMenuItem*					m_menuItem;
	NdhsCPageManager*				m_pageManager;
	NdhsCElementGroup*				m_page;
	int								m_stackLevel;
	bool							m_bTeleportStateChange;
	bool							m_bTeleportScroll;
	bool							m_bKeepNonSketchy;
	NdhsCElementGroup*				m_owningGroup;
	TmMFieldTokens					m_fieldTokens;

	bool							m_touchdownActive;

	// Cache transition data for efficiency
	LcTPlacement					m_baseStart; // cached endpoint for transition
	int								m_baseStartMask; // cached endpoint placement mask for transition
	bool							m_baseStartUsingLayoutExtent; // Determines whether the extent was from the layout OR extentHint/Default.
	LcTPlacement					m_baseEnd; // cached endpoint for transition
	int								m_baseEndMask; // cached endpoint placement mask for transition
	bool							m_baseEndUsingLayoutExtent; // Determines whether the extent was from the layout OR extentHint/Default.
	NdhsCPath						m_basePath; // Path between baseStart and baseEnd

	LcTPlacement					m_baseStartBackup;
	LcTPlacement					m_baseEndBackup;
	int								m_baseStartMaskBackup;
	int								m_baseEndMaskBackup;

	LcTPlacement					m_currentPlacement; // most recent calculated placement cache
	bool							m_atStart; // True if element was last positioned at the start of the most recent transition
	bool							m_atEnd; // True if element was last positioned at the end of the most recent transition
	bool							m_transitioning;

	NdhsCKeyFrameList*				m_pCurve; // cached scroll curve
	bool							m_bCurveForwards; // If true, we're traversing the curve in the 'forwards' direction
	bool							m_makeVisible;
	bool							m_isVisible; // Widget is visible (if it exists!)
	int								m_drawLayerIndex;

	NdhsCKeyFrameList*				m_pDecorator; // cached transition decoration
	NdhsCKeyFrameList*				m_pStaticAnim; // cached static decoration
	NdhsCScrollPosField*			m_pStaticAnimField; // pointer to field controlling static transition

	LcTVector						m_defaultExtent;
	bool							m_animate; // Whether element is currently tweening.

	ETappable						m_tappable;
	LcTScalar						m_tapTolerance;

	bool							m_startExtentFromWidget;
	bool							m_endExtentFromWidget;

	bool							m_hasFocus;

	bool							m_needsReload;
	bool							m_needsFocusCheck;
	bool							m_focusEnabled;

	NdhsCLaundry*					m_laundry;

	bool							m_suspendWidgetExists;
	bool							m_suspendWidgetRealized;
	LcTPlacement					m_suspendPlacement;
	bool							m_suspendVisible;

	bool							m_hideElement; // If true, then element is set to disabled in the layout by the user.
	bool							m_unloaded;
protected:

	LcTmOwnerArray<NdhsCTemplate::CElementDisplacement>	m_BaseStartDisplacementArray;
	LcTmOwnerArray<NdhsCTemplate::CElementDisplacement>	m_BaseEndDisplacementArray;

	bool							m_isRealized;

#ifdef LC_USE_LIGHTS
	ENdhsLightModel					m_lightModel;
#endif

private:
	void							subscribeToFields(const LcTmArray<NdhsCTemplate::CDisplacement*>* pDisplacements, bool isBaseEnd);
	void							unsubscribeToFields(const LcTmArray<NdhsCTemplate::CDisplacement*>* pDisplacements, bool isBaseEnd);

	void							replaceDisplacements(const LcTmArray<NdhsCTemplate::CDisplacement*>* source, LcTmMap<LcTmString, LcTmString>& dest);

protected:

	// Two-phase construction
									NdhsCElement();

	void							construct(
										ENdhsObjectType		use,
										ENdhsElementType	type,
										const LcTmString&	parent,
										const LcTmString&	className,
										NdhsCExpression::CExprSkeleton*	resourceData,
										NdhsCExpression::CExprSkeleton*	enableFocusSkel,
										NdhsCMenu*			menu,
										NdhsCMenuItem*		menuItem,
										NdhsCPageManager*	pPageManager,
										NdhsCElementGroup*	pPage,
										int					stackLevel,
										int					drawLayerIndex,
										bool				teleportStateChange	= false,
										bool				teleportScroll		= false,
										bool				isDetail			= false);

	NdhsCPageManager*				getPageManager()							{ return m_pageManager; }
	NdhsCElementGroup*				getPage()									{ return m_page; }
	NdhsCMenu*						getMenu()									{ return m_menu; }
	NdhsCMenuItem*					getMenuItem()								{ return m_menuItem; }
	NdhsCMenuItem*					getLocalMenuItem()							{ return ((m_menuItem && m_menuItem->getOwner() == m_menu) ? m_menuItem : NULL ); }
	int								getStackLevel()								{ return m_stackLevel; }
	LcTaString						getElementData();
	LcTaString						getPackagePath()							{ return m_packagePath; }
	void							clearFieldTokens();
	NdhsCLaundry*					getLaundry()								{ return m_laundry; }

	void							setWidget(LcTmOwner<LcCWidget>& widget)		{ m_widget = widget; }
	void							destroyWidget()								{ if(m_widget.ptr() != NULL) m_widget.destroy(); }
	int								calculatePosition(LcTScalar position, LcTPlacement& placement);
	void							applyDisplacements(const LcTmArray<NdhsCTemplate::CElementDisplacement*>& pDisplacements,
															LcTPlacement& placement,
															int& placementMask);
	void							applyDisplacements(const LcTmArray<NdhsCTemplate::CElementDisplacement*> & pDisplacements,
															LcTPlacement& placement,
															int& placementMask,NdhsCTemplate::CElementDisplacement* currentDisplacement);
	bool							updateElementStatus(ENdhsAnimationType animType, bool finalFrame);
	NdhsCField*						getField(const LcTmString& fieldName);


	bool							getTouchdownStatus()						{ return m_touchdownActive; }
	void							needsReload(bool needsReload)				{ m_needsReload = needsReload; }
#ifdef LC_USE_MOUSEOVER
	bool							setMouseoverStatus(bool mouseover, bool updateField);
#endif

	void							setTappable(ETappable tappable)				{ m_tappable = tappable; }
	void							setTapTolerance(LcTScalar tapTolerance)		{ m_tapTolerance = tapTolerance; }

	void							updateWidgetPlacement();

	int								getAnimateMask();
	int								getStaticAnimateMask();
	int								applyStaticDisplacement(LcTPlacement& placement);
	void							teleportElement();
	bool							widgetVisible()								{ return m_isVisible; }
	int								getDrawLayerIndex()							{return m_drawLayerIndex; }

public:
#ifdef IFX_SERIALIZATION
	static		NdhsCElement*		loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual		SerializeHandle		serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual		void				deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
				ISerializeable*		getSerializeAble(int &type);
	virtual		bool				isMenuItemChild(){return m_menuItem!=NULL || (m_page && m_page->isMenuItemChild());}
	void							deflate();
#endif /* IFX_SERIALIZATION */

	void applyDisplacement(NdhsCTemplate::CElementDisplacement*displacement);
	virtual							~NdhsCElement();
	void							realize();
	void							retire();
	virtual void					reloadElement()								=0;
	void							setVisible(bool bVisible);
	bool							isHidden()									{ return m_widget ? m_widget->isHidden() : true; }
	inline LcCWidget*				getWidget()									{ return m_widget.ptr(); }
	inline ENdhsElementType			getElementType()							{ return m_elementType; }
	inline ENdhsObjectType			getElementUse()								{ return m_elementUsage; }
	inline LcTaString				getElementClassName()						{ return m_elementClassName; }
	virtual inline bool				isTappable()								{ return (m_tappable == EFull || m_tappable == EPartial); }
	virtual inline ETappable		getTappable()								{ return m_tappable; }
	virtual inline LcTScalar		getTapTolerance()							{ return m_tapTolerance; }
	inline bool						checkElementVisiblity()						{ return (m_makeVisible || m_isVisible);}
	void							registerFieldToken(NdhsCField* field);
	void							refreshField(NdhsCField* field);
	void							setTouchdownStatus(bool touchdown, bool updateField);

	int								getSlot();
	int								getOldSlot();

	void							resetTransitionCache();
	virtual void					updatePosition(ENdhsAnimationType animType,
										LcTScalar position,
										bool positionIncreasing,
										bool updateCache,
										bool forceNonSketchy,
										bool aggregateAnimating = false,
										bool finalFrame = false);

	// Return a reference to a local instance of this vector to work around a RealView ARM11/VFP compiler issue.
	virtual inline LcTVector&		getDefaultExtent()							{ return m_defaultExtent; }

	inline bool						teleportOnScroll()							{ return m_bTeleportScroll; }
	inline bool						teleportOnStateChange()						{ return m_bTeleportStateChange; }

	void							setElementGroup(NdhsCElementGroup* pGroup);
	inline NdhsCElementGroup*		getElementGroup()							{ return m_owningGroup; }

#ifdef IFX_USE_PLUGIN_ELEMENTS
	// Event handler type plugin element support
	virtual bool					processKeyDownEvent(int key)				{ LC_UNUSED(key); return false; }
	virtual bool					processKeyUpEvent(int key)					{ LC_UNUSED(key); return false; }
	virtual NdhsCPlugin::NdhsCPluginHElement* getPluginElement()				{ return NULL; }
#endif // IFX_USE_PLUGIN_ELEMENTS

	virtual bool					setElementFocus(bool enableFocus)			{ LC_UNUSED(enableFocus); return true; }

#ifdef LC_USE_STYLUS
	virtual bool					onMouseDown(const LcTPixelPoint& pt);
	virtual bool					onMouseMove(const LcTPixelPoint& pt);
	virtual bool					onMouseUp(const LcTPixelPoint& pt);
	virtual void					onMouseCancel();
	virtual bool					dragScrolling()								{ return false; }
	virtual bool					isDragging()								{ return false; }

	virtual	void					populateElementList(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidget);
#endif


	// For caching animation properties
			void					setTransitionBasePlacements(LcTPlacement start,
																int startMask,
																bool startUsingLayoutExtent,
																LcTPlacement end,
																int endMask,
																bool endUsingLayoutExtent);
			void					setTransitionBaseDisplacements(const LcTmArray<NdhsCTemplate::CDisplacement*>* pStart,
																		const LcTmArray<NdhsCTemplate::CDisplacement*>* pEnd);
	inline void						setTransitionScrollCurve(NdhsCKeyFrameList* pCurve, bool bCurveForwards)
																				{ m_pCurve = pCurve; m_bCurveForwards = bCurveForwards; }
	inline LcTPlacement				getBasePlacement()							{ return m_baseEndBackup; }
	inline void						resetPlacement()							{ m_currentPlacement = LcTPlacement(); }

	void							setStartExtentIsFromWidget(bool isFromWidget) { m_startExtentFromWidget = isFromWidget; }
	void							setEndExtentIsFromWidget(bool isFromWidget)	{ m_endExtentFromWidget = isFromWidget; }

	virtual void					onSuspend();
	virtual void					onResume();
	virtual bool					forceDestroyOnSuspend()						{return false;}
	bool							isElementSuspended()						{ return m_suspendWidgetExists; }

	virtual void					startStaticAnimations();
	virtual void					stopStaticAnimations();
	inline void						setTransitionDecoration(NdhsCKeyFrameList* pDec)
																				{ m_pDecorator = pDec; }
	void							setStaticAnimation(NdhsCKeyFrameList* pDec,
											unsigned int staticTime,
											int loopCount,
											ENdhsVelocityProfile velocityProfile);

	// NdhsCField::IObserver callback
	virtual void					fieldValueUpdated(NdhsCField* field);
	virtual void					fieldDirty(NdhsCField* field)				{}
	virtual void					fieldDestroyed(NdhsCField* field);

	// NdhsIExpressionObserver implementation
	virtual void					expressionDirty(NdhsCExpression* expr);
	virtual void					expressionDestroyed(NdhsCExpression* expr);

	// NdhsILaundryItem support
	virtual bool					cleanLaundryItem(LcTTime timestamp);

	virtual bool					isUsingLayoutExtent()						{ return m_widget ? m_widget->isUsingLayoutExtent() : m_startExtentFromWidget | m_endExtentFromWidget; }

	virtual bool					trySetFocus();
	virtual void					unsetFocus();

	virtual bool					onWidgetEvent(LcTWidgetEvent* e) { LC_UNUSED(e) return false; }

	//Hide functionality for Elements
	void							setHideElementFlag(bool value)				{m_hideElement = value;}
	bool							getHideElementFlag()						{return m_hideElement;}

	virtual void					setUnloaded(bool value);
	bool							getUnloaded()								{return m_unloaded;}

#ifdef LC_USE_LIGHTS
	inline void						setLightModel(ENdhsLightModel value) {m_lightModel = value;}
	inline void						applyLightModel(ENdhsLightModel value) {m_lightModel = ((value == ENdhsLightModelSimple) ? value : m_lightModel);}
	inline ENdhsLightModel			getLightModel(){return m_lightModel;}
#endif

#ifdef NDHS_JNI_INTERFACE
	LcTaString						getFocusChain()								{
																					LcTaString item;
																					if (m_menuItem != NULL)
																						item = ", item = " + item.fromInt(m_menuItem->getIndex());
																					return m_owningGroup->getFocusChain() + "->" + m_elementClassName + item;
																				}
#endif
};
#endif // NdhsCElementH

// End of File
