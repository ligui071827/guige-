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
#ifndef NdhsCElementGroupH
#define NdhsCElementGroupH

#include "inflexionui/engine/inc/LcCAggregate.h"
#include "inflexionui/engine/inc/LcCSerializeMaster.h"
#include "inflexionui/engine/inc/LcTString.h"
#include "inflexionui/engine/inc/NdhsCPageTemplate.h"
#include "inflexionui/engine/inc/NdhsCField.h"
#include "inflexionui/engine/inc/NdhsIFieldContext.h"

class NdhsCElement;

/*-------------------------------------------------------------------------*//**
	NdhsCElementGroup represents a collection of page model elements.
*/
class NdhsCElementGroup : public LcCAggregate, public NdhsCField::IObserver, public NdhsIFieldContext, public NdhsCTemplate::CIElementDisplacementObserver, public ISerializeable
{
protected:
	enum EAnimationGroup
	{
		EAnimationAll,
		EAnimationItemOnly
	};

	enum ENdhsCElementGroupType
	{
		ENdhsCElementGroupTypeBase,
		ENdhsCElementGroupTypePage,
		ENdhsCElementGroupTypeComponent,
		ENdhsCElementGroupTypeMenuComponent
	};

	typedef LcTmOwnerMap<LcTmString, NdhsCElementGroup>	TmMGroups;

private:
	// Maps element class name to NdhsCElement
	typedef LcTmOwnerMap<LcTmString, NdhsCElement>	TmMElements;
	TmMElements							m_elements;
	// Maps group name to NdhsCElementGroup
	TmMGroups							m_subgroups;
#ifdef LC_USE_STYLUS
	typedef LcTmArray<NdhsCElement*>	TmAElement;
#endif
	bool								m_scheduledForDeletion;
	ENdhsObjectType						m_groupType;
	ENdhsGroupType						m_componentGroupType;
	LcTmString							m_groupName;
	int									m_slot;
	int									m_oldSlot;
	NdhsCElementGroup*					m_page;
	NdhsCMenu*							m_menu;
	NdhsCMenuItem*						m_item;
	NdhsCElementGroup*					m_owner;
	NdhsCPageManager*					m_pageManager;
	int									m_menuIndex;

	bool								m_bTeleportStateChange;
	bool								m_bTeleportScroll;

	// Cache transition data for efficiency
	LcTPlacement						m_baseStart; // cached endpoint for transition
	int									m_baseStartMask; // cached endpoint placement mask for transition
	LcTPlacement						m_baseEnd; // cached endpoint for transition
	int									m_baseEndMask; // cached endpoint placement mask for transition
	NdhsCPath							m_basePath; // Path between baseStart and baseEnd
	bool								m_animate; // enable or disable animation updates for the group
	LcTPlacement						m_baseStartBackup; // cached endpoint for transition
	LcTPlacement						m_baseEndBackup; // cached endpoint for transition
	int									m_baseStartMaskBackup; // cached endpoint mask for transition
	int									m_baseEndMaskBackup; // cached endpoint mask for transition
	LcTPlacement						m_currentPlacement; // most recent calculated placement cache
	bool								m_atStart; // True if element was last positioned at the start of the most recent transition
	bool								m_atEnd; // True if element was last positioned at the end of the most recent transition
	bool								m_transitioning;

	NdhsCKeyFrameList*					m_pCurve; // cached scroll curve
	bool								m_bCurveForwards; // If true, we're traversing the curve in the 'forwards' direction
	bool								m_makeVisible;
	bool								m_hideGroup; // If true, then group is set to disable by the user explicitly in theme. Should'nt be displayed in the layout.

	int									m_drawLayerIndex; // Draw Layer Index
	bool								m_isGroupUnloaded;

#ifdef LC_USE_LIGHTS
	ENdhsLightModel						m_lightModel;
#endif

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	LcTmString 							m_openGLRenderQualitySetting;
#endif

protected:
	NdhsCKeyFrameList*					m_pDecorator; // cached transition decoration
	NdhsCKeyFrameList*					m_pStaticAnim; // cached static decoration
	NdhsCScrollPosField*				m_pStaticAnimField; // pointer to field controlling static transition

	LcTmOwnerArray<NdhsCTemplate::CElementDisplacement>	m_BaseStartDisplacementArray;
	LcTmOwnerArray<NdhsCTemplate::CElementDisplacement>	m_BaseEndDisplacementArray;

private:
	void								subscribeToFields(const LcTmArray<NdhsCTemplate::CDisplacement*>* pDisplacements, bool isBaseEnd);
	void								unsubscribeToFields(const LcTmArray<NdhsCTemplate::CDisplacement*>* pDisplacements, bool isBaseEnd);
	void								applyDisplacements(const LcTmArray<NdhsCTemplate::CElementDisplacement*>& pDisplacements,
													LcTPlacement& placement,
													int& placementMask);

	void								applyDisplacements(const LcTmArray<NdhsCTemplate::CElementDisplacement*> & pDisplacements,
													LcTPlacement& placement,
													int& placementMask,NdhsCTemplate::CElementDisplacement* currentDisplacement);
	void								applyDisplacement(NdhsCTemplate::CElementDisplacement*displacement);

	int									calculatePosition(LcTScalar position, LcTPlacement& placement);

protected:
										NdhsCElementGroup();

	// Two-phase construction
	LC_IMPORT							NdhsCElementGroup(const LcTmString& name,
																	NdhsCElementGroup* page,
																	NdhsCMenu*		menu,
																	NdhsCMenuItem*	menuItem,
																	int		drawLayerIndex,
																	bool	bTeleportStateChange = false,
																	bool	bTeleportScroll = false);

	LC_IMPORT		void				construct();
	bool								updateGroupStatus(ENdhsAnimationType animType, bool finalFrame);
	NdhsCElementGroup*					getOwner()										{ return m_owner; }
	NdhsCMenu*							getMenu()				{ return m_menu; }
	NdhsCMenuItem*						getMenuItem()			{ return m_item; }
	NdhsCMenuItem*						getLocalMenuItem()		{ return ((m_item && m_item->getOwner() == m_menu) ? m_item : NULL ); }

public:
#ifdef IFX_SERIALIZATION
	static			NdhsCElementGroup*	loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual			SerializeHandle		serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual			void				deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
					ISerializeable *	getSerializeAble(int &type);
	virtual			bool				isMenuItemChild(){return m_item!=NULL || (m_page && m_page->isMenuItemChild());}
#endif /* IFX_SERIALIZATION */

	LC_IMPORT static LcTaOwner<NdhsCElementGroup> create(const LcTmString& name,
																	NdhsCElementGroup* page,
																	NdhsCMenu*		menu,
																	NdhsCMenuItem*	menuItem,
																	int		drawLayerIndex);
	LC_IMPORT		virtual				~NdhsCElementGroup();
	LC_IMPORT		virtual void		onRealize();
	LC_IMPORT		virtual void		onRetire();
	LC_IMPORT		int 				getAnimateMask();
	LC_IMPORT		int 				getStaticAnimateMask();

	inline			void				scheduleForDeletion(bool schedule)				{ m_scheduledForDeletion = schedule; }
	inline			bool				isScheduledForDeletion()						{ return m_scheduledForDeletion; }

	inline			bool				visibleOnTransitionComplete()					{ return m_makeVisible; }

	LC_IMPORT		virtual LcTaString			getClassFromWidget(LcCWidget* widget);
	LC_IMPORT		virtual NdhsCElement*		getItem(const LcTmString& elementClass);
	LC_IMPORT		virtual NdhsCElementGroup*	getGroup(const LcTmString& groupName);

	LC_IMPORT		bool				addItem(const LcTmString& elementClass, LcTmOwner<NdhsCElement>& element);
	LC_IMPORT		void				removeItem(LcTmString& elementClass);
	LC_IMPORT		bool				addGroup(const LcTmString& groupName, LcTmOwner<NdhsCElementGroup>& group);
	LC_IMPORT		bool				removeGroup(const LcTmString& groupName);
	LC_VIRTUAL		void				onAggregateEvent(LcCWidget::TAggregateEvent* e);

	LC_IMPORT		void				setSlot(int newSlot);
	LC_IMPORT		int					getSlot()										{ return m_slot; }
	LC_IMPORT		int					getOldSlot()									{ return m_oldSlot; }
	LC_IMPORT		ENdhsObjectType		getGroupType()									{ return m_groupType; }
	LC_IMPORT		NdhsCElementGroup*	getPage()										{ return m_page; }
	LC_IMPORT		void				setGroupType(ENdhsObjectType type)				{ m_groupType = type; }
	LC_IMPORT		LcTaString			getGroupName()									{ return m_groupName; }

#ifdef LC_USE_STYLUS
	LC_IMPORT		virtual void		populateElementList(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList);
#endif

	void 								resetTransitionCache();
	void								updatePosition(ENdhsAnimationType animType,
													LcTScalar position,
													bool positionIncreasing,
													bool updateCache,
													bool forceNonSketchy,
													bool aggregateAnimating = false,
													bool finalFrame = false);

	// For caching animation properties
	void								setTransitionBasePlacements(LcTPlacement start,
													int startMask,
													LcTPlacement end,
													int endMask);
	void								setTransitionBaseDisplacements(const LcTmArray<NdhsCTemplate::CDisplacement*>* pStart,
														const LcTmArray<NdhsCTemplate::CDisplacement*>* pEnd);
	inline void							setTransitionScrollCurve(NdhsCKeyFrameList* pCurve, bool bCurveForwards)
																	{ m_pCurve = pCurve; m_bCurveForwards = bCurveForwards; }
	inline LcTPlacement					getBasePlacement()				{ return m_baseEndBackup; }

	void 								startStaticAnimations();
	void 								stopStaticAnimations();
	inline void							setTransitionDecoration(NdhsCKeyFrameList* pDec)
																	{ m_pDecorator = pDec; }
	void								setStaticAnimation(NdhsCKeyFrameList* pDec,
													unsigned int staticTime,
													int loopCount,
													ENdhsVelocityProfile velocityProfile);
	virtual void						resumeStaticAnimations();

	int 								applyStaticDisplacement(LcTPlacement& placement);
	inline void							resetPlacement() { m_currentPlacement = LcTPlacement(); }

	// NdhsCField::IObserver callback
	virtual void						fieldValueUpdated(NdhsCField* field);
	virtual void 						fieldDirty(NdhsCField* field) 			{}
	virtual void 						fieldDestroyed(NdhsCField* field);


			void						onWidgetEvent(LcTWidgetEvent* e);

	void								setElementGroup(NdhsCElementGroup* owner)
																	{ m_owner = owner; }

	virtual		NdhsCTemplate*			getTemplate() 	{ return NULL; }
	virtual		NdhsCField*				getFieldValue(const LcTmString& fieldName,
														NdhsCMenu* menu,
														int item,
														NdhsCElement* element)
												{ LC_UNUSED(fieldName) LC_UNUSED(menu)
												  LC_UNUSED(item) LC_UNUSED(element)
												  return NULL;
												}

	virtual		NdhsCTransitionAgent*	getTransitionAgent() 		{ return NULL; }
	virtual		inline ENdhsPageState	getPageState() { return ENdhsPageStatePreOpen; }
	virtual		inline ENdhsPageState	getPrevPageState() { return ENdhsPageStatePreOpen; }
	virtual		inline bool				areStaticAnimationsStopping() { return false; }
	virtual		void					setPrimaryLightTransitionBasePlacements(LcTPlacement start,
																					int startMask,
																					LcTPlacement end,
																					int endMask) { return; }
	virtual bool						processTrigger(
														int code,
														int slot,
														LcTmString& elementClass,
														LcTmArray<NdhsCTemplate::CAction::CAttempt*>* optionsAttempts,
														bool fromModule)
													{
														 LC_UNUSED(code) LC_UNUSED(slot) LC_UNUSED(elementClass)
														 LC_UNUSED(optionsAttempts) LC_UNUSED(fromModule)
														 return false;
													}
#ifdef LC_USE_STYLUS
	virtual bool						onMouseDown(NdhsCPageManager::TPageWidgetElem*, const LcTPixelPoint& pt) { return false; }
	virtual bool						onMouseMove(NdhsCPageManager::TPageWidgetElem*, const LcTPixelPoint& pt) { return false; }
	virtual bool						onMouseUp(NdhsCPageManager::TPageWidgetElem*, const LcTPixelPoint& pt) { return false; }
#endif

	virtual inline void					setPageTransitionDecoration(NdhsCPageTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* pTriggerList)
																{ LC_UNUSED(pTriggerList) return; }

	virtual bool 						startStaticAnimations(EAnimationGroup groupToStart) { LC_UNUSED(groupToStart) return false; }
	virtual void 						stopStaticAnimations(bool immediateStop, EAnimationGroup groupToStop) { LC_UNUSED(groupToStop) }
	virtual void						setStaticAnimationTrigger(NdhsCPageTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* pTriggerList)
																{ LC_UNUSED(pTriggerList) }
	virtual NdhsCScrollPosField*		getStaticAnimationField(NdhsCField::IObserver* observer)
																{ LC_UNUSED(observer) return NULL; }
	virtual void						releaseStaticAnimationField(NdhsCScrollPosField* field)
																{ LC_UNUSED(field) }

	virtual void						unLoadGroup();
	virtual void						loadGroup();
	virtual bool						isGroupUnloaded() { return m_isGroupUnloaded; }
	virtual bool						isParentUnloaded() { return m_page->isParentUnloaded(); }
	virtual void						doAction(NdhsCTemplate::CAction* action, int slotNum = -1) { LC_UNUSED(action) LC_UNUSED(slotNum) }
	virtual bool						doAttempt(NdhsCTemplate::CAction::CAttempt* attemp, int slotNum = -1)
												{
													LC_UNUSED(attemp) LC_UNUSED(slotNum)
													return false;
												}
	// Field context
	virtual NdhsCField*					getField(const LcTmString& field, int slotNum, NdhsCMenuItem* item);
	virtual void						createFieldCache(NdhsCTemplate* templ)
															{ LC_UNUSED(templ) return; }
	virtual void						updateTouchDownIntrinsicField(const LcTmString& element, int slot)
															{ LC_UNUSED(element); LC_UNUSED(slot); return; }

#ifdef LC_USE_MOUSEOVER
	virtual void						updateMouseOverIntrinsicField(const LcTmString& element, int slot);
#endif

	virtual NdhsCField*					getPageParamValue(const LcTmString& key);
	// Needed for menu components
	TmMGroups::iterator					begin() { return m_subgroups.begin(); }
	TmMGroups::iterator					end() { return m_subgroups.end(); }

	// Draw Layer function
	int									getDrawLayerIndex();
	void								setDrawLayerIndex(int drawLayerIndex) {m_drawLayerIndex = drawLayerIndex; }

	//Hide functionality for groups.
	void								setHideGroupFlag(bool value){m_hideGroup = value;}
	bool								getHideGroupFlag(){return m_hideGroup;}

#ifdef LC_USE_LIGHTS
	inline void							setLightModel(ENdhsLightModel value) {m_lightModel = value;}
	inline void							applyLightModel(ENdhsLightModel value) {m_lightModel = ((value == ENdhsLightModelSimple) ? value : m_lightModel);}
	inline ENdhsLightModel				getLightModel(){return m_lightModel;}
#endif

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	LC_IMPORT	LcTmString				getOpenGLRenderQualitySetting();
	LC_IMPORT	void					setOpenGLRenderQualitySetting (const LcTmString& qualitySetting)	{	m_openGLRenderQualitySetting = qualitySetting;	}
#endif

	virtual void						onSuspend();
	virtual void						onResume();
	virtual void						loadResources();
	virtual void						UnloadResources();

	virtual LcTPlacement*				getPlacementForComponent(int					slot,
																	LcTmString&			className,
																	int&				mask)
										{
											LC_UNUSED(slot) LC_UNUSED(slot)
											LC_UNUSED(mask)
											return NULL;
										}


	virtual bool						trySetFocus()		{ return false; }
	virtual void						unsetFocus();
			void						globalUnsetFocus();
	virtual void						focusSetOnChild(NdhsCComponent* childComponent, NdhsCComponent* focusComponent);
	virtual NdhsCComponent*				findParentComponent();
	virtual NdhsCPageModel*				findParentPageModel();
	virtual NdhsCPageManager*			getPageManager() { return m_page->getPageManager(); }
			void 						doCheckVisibilityChanges();

	virtual bool						componentDoPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame);
	virtual void						componentsJumpTransitionToEnd(bool setIdle);
	virtual void						componentRefreshField(const LcTmString& field, int item);
	virtual int							getHMenuActiveItemIndex(IFX_HMENU hMenu);
	virtual bool						getFullFilePath(	IFX_HMENU hMenu,
																const LcTmString& searchFile,
																LcTmString& returnFilePath,
																int menuIndex);
	virtual bool						isComponentTransitioning();

#if defined(NDHS_JNI_INTERFACE)
	virtual void						componentOnTransitionComplete(bool setIdle);
	virtual unsigned int				distanceFromPage();
	virtual unsigned int				getIdentifier();
	virtual LcTaString					getFocusChain()						{ return m_owner ? m_owner->getFocusChain() : (m_page ? m_page->getFocusChain() : "");}
	virtual LcTaString					getObjectWithFocus()				{ return ""; }
#endif

	// From LCCWidget
	LC_VIRTUAL		void				setVisible(bool b);

	virtual	NdhsCPageModel*				getParentPageRef();
	virtual bool						isImmediateMenu(NdhsCMenuItem *menuItem){LC_UNUSED(menuItem) return false;}
	virtual LcTaString					getTouchDownElement();
};
#endif // NdhsCElementGroupH

