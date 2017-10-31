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
#ifndef NdhsCComponentH
#define NdhsCComponentH

class NdhsCPageManager;
class NdhsCElementGroup;
class NdhsCScrollPosField;

#include "inflexionui/engine/inc/NdhsCTransitionAgent.h"
#include "inflexionui/engine/inc/NdhsCPageTemplate.h"
#include "inflexionui/engine/inc/NdhsCField.h"

class NdhsCComponent : public NdhsCElementGroup, public NdhsCStateManager::IObserver, public LcTMessage::IHandler
{

private:
	bool							m_componentRetire;

protected:

	NdhsCPageManager*				m_pageManager;
	NdhsCTemplate*					m_template;
	NdhsCElementGroup*				m_parent;
	bool							m_isCreated;
	bool							m_isEnabled;
	bool							m_groupUnloaded;
	LcTmString						m_path;

	// Chained action
	LcTmString						m_chainedAction;
	int								m_chainedActionSlot;

#if defined(NDHS_JNI_INTERFACE)
	unsigned int					m_identifier;
#endif

	bool							m_componentActive;

	NdhsCTemplate::CLayout* 		m_currentLayout;
	NdhsCTemplate::CLayout* 		m_pendingLayout;
	bool							m_pendingLayoutAnimateFlag;
	bool							m_preparingTransition;

	// Furniture elements and aggregates

	LcTMessage						m_postTransitionCompleteMessage;
	LcTMessage						m_simulateKeyMessage;
	LcTmOwner<NdhsCElementGroup>	m_outerGroup;

	ENdhsAnimationType 				m_animType;
	ENdhsAnimationType 				m_previousAnimType;
	bool							m_stateChangeAnimComplete;
	LcTTime							m_decorationDelayTimestamp;
	bool							m_bMouseDown;
	bool							m_lastFrame;
	bool							m_isLayoutChange;
	bool							m_isEligible;

#ifdef LC_USE_STYLUS
	typedef LcTmArray<NdhsCElement*>	TmAElement;
	NdhsCElement*					m_mouseFocusElement;
#endif

	int 							m_transitionDuration;
	int 							m_transitionDelay;
	ENdhsVelocityProfile 			m_transitionVelProfile;

	// Static transition flag
	bool							m_staticTransitionDone;
	bool							m_jumpingToEnd;
	LcTmString 						m_componentName;

	// Cache transition data for efficiency
	LcTmOwnerArray<NdhsCScrollPosField> m_staticAnimFields; // array of all fields controlling static animations
	bool							m_stopStaticAnimationAll; // if true, all static animation should terminate before next loop
	bool							m_stopStaticAnimationItem; // if true, item static animation should terminate before next loop
	NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* m_pAnimTriggerList;
	NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* m_pStaticTriggerList;
	NdhsCScrollPosField* 			m_pStaticAnimFieldTrigger; // pointer to field controlling static transition triggers
	LcTScalar						m_lastTriggerPos;
	LcTScalar						m_lastStaticTriggerPos;
	EAnimationGroup					m_staticAnimGroup;

	LcTmOwner<NdhsCStateManager>	m_stateManager;
	LcTmOwner<NdhsCFieldCache>		m_fieldCache;

	// Cached transition agent
	LcTmOwner<NdhsCTransitionAgent>	m_transitionAgent;
	LcTmOwner<NdhsCScrollPosField>	m_layoutPos;
	LcTmOwner<NdhsCElementGroup>	m_localScreenAggregate;

private:

	LcTmOwnerMap<LcTmString, NdhsCExpression::CExprSkeleton>* m_parameters; // NB this is a pointer to externally owned map

	// Set if additional tasks required on a onTransitionComplete call
	bool							m_bRequiresPostTransitionComplete;

	// Token replacer and manifest stack level
	int								m_stackLevel;

protected:
	NdhsCElement*					m_focusedElement;
	NdhsCComponent*					m_focusedChildComponent;
	bool							m_inFocus;
	NdhsCField*						m_focusEnabledField;
	NdhsCField*						m_parentFocusEnabledField;

	bool							m_trySetFocus;

private:
	// Delayed page refresh flag
	bool							m_refreshNeeded;

LC_PRIVATE_INTERNAL_PUBLIC:

#ifdef LC_USE_STYLUS
	virtual void					populateElementList(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList);
	virtual void					populateWidgetElementMap(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList);

	virtual bool					isCatchAll()		{ return m_stateManager ? m_stateManager->isCatchAll() : false; }
#endif

	bool 							trySetFocus(const LcTmString& className);

protected:

	// Allow only 2-phase construction, 3rd default constructor for serialization
									NdhsCComponent(	NdhsCPageManager*					pageManager,
													NdhsCTemplate::CComponentElement*	component,
													NdhsCElementGroup*					parent,
													int									stackLevel,
													NdhsCMenu*							menu,
													NdhsCMenuItem*						menuItem,
													NdhsCElementGroup*					parentGroup);
#ifdef IFX_SERIALIZATION
									NdhsCComponent():m_postTransitionCompleteMessage(this, EPostTransitionCompleteMsg),
													 m_simulateKeyMessage(this, ESimulateKeyMsg){}
#endif /* IFX_SERIALIZATION */
	void							construct();

	virtual void					doPostConstruct();

	bool							executeChainedAction(const LcTmString& action, int slotNum);

	bool							updateFields(LcTTime timestamp);

	NdhsCElement*					getFocusedElement() 			{ return m_focusedElement; }
	NdhsCComponent*					getFocusedChildComponent()		{ return m_focusedChildComponent; }
	NdhsCField*						getFocusEnabledField()			{ return m_focusEnabledField; }

	void							createFieldCache();
	virtual void					createIntrinsicFields();
	void							bindParameters();

	virtual void					prepareComponent();
	virtual void					reloadFurniture();

	void 							setFocusIntrinsics(bool focused, const LcTmString& focusedClass);

public:

#ifdef IFX_SERIALIZATION
	static	NdhsCComponent*			loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual	SerializeHandle			serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual	void					deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */

	enum EMessages {
		EPostTransitionCompleteMsg,
		EReplacePlaceholdersMsg,
		ESimulateKeyMsg,
		EReloadDataSource
	};

	virtual void onMessage(int iID, int iParam);
	virtual void					onRealize();
	virtual	void					onRetire();

	// Creation/destruction
	static LcTaOwner<NdhsCComponent> create(NdhsCPageManager*					pageManager,
											NdhsCTemplate::CComponentElement*	component,
											NdhsCElementGroup*					parent,
											int									stackLevel,
											NdhsCMenu*							menu,
											NdhsCMenuItem*						menuItem,
											NdhsCElementGroup*					parentGroup);
	virtual							~NdhsCComponent();
	virtual void					doCleanup();

	virtual void					reloadElement();
	void 							schedulePostTransitionComplete();

	virtual NdhsCTemplate*			getTemplate()				{ return m_template; }
	virtual NdhsCMenuComponentTemplate*
									getMenuTemplate()			{ return NULL; }

	inline NdhsCPageManager*		getPageManager()			{ return m_pageManager; }
	inline NdhsCElementGroup*		getFurnitureAggregate()			{ if (m_outerGroup.ptr() != NULL) return m_outerGroup->getGroup("furniture"); else return NULL; }
	inline NdhsCElementGroup*		getOuterGroupAggregate()	{ return m_outerGroup.ptr(); }
	inline NdhsCElementGroup*		getLocalScreenAggregate()	{ return m_localScreenAggregate.ptr(); }
	inline int						getStackLevel()				{ return m_stackLevel; }
	inline NdhsCTransitionAgent*	getTransitionAgent()		{ return m_transitionAgent.ptr(); }

	virtual NdhsCElement*			getItem(const LcTmString& elementClass);
	virtual NdhsCElementGroup*		getGroup(const LcTmString& groupName);

	void							destroyUnwantedElements();

	LC_IMPORT NdhsCField*			getFieldValue(const LcTmString& fieldName,
													NdhsCMenu* menu,
													int item,
													NdhsCElement* element);

	virtual bool					componentDoPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame);
	virtual void					componentsJumpTransitionToEnd(bool setIdle);
	virtual void 					componentRefreshField(const LcTmString& field, int item);
	void							simulateKeyDown(int c);

#if defined(NDHS_JNI_INTERFACE)
	LC_IMPORT virtual void			componentOnTransitionComplete(bool setIdle);
#endif


	virtual bool					changeState(ENdhsPageState state, int activeSlot, const NdhsCMenuComponentTemplate::CAction::CAttempt* attempt, bool animate = true);
	virtual void					jumpTransitionToEnd(bool setIdle);
	virtual void					onTransitionComplete(bool setIdle, bool generateSignal = false, bool forceSlotUpdate = false);
	virtual void					postTransitionComplete();
	virtual bool					isComponentTransitioning();

	/* If these have been called from a parent, do not propagate search past component boundary */
	virtual LcTaString				getClassFromWidget(LcCWidget* widget)			{ return ""; }

	inline void						setPageTransitionDecoration(NdhsCPageTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* pTriggerList)
																{ m_pAnimTriggerList = pTriggerList; m_lastTriggerPos = -1; }
	virtual bool 					startStaticAnimations(EAnimationGroup groupToStart);
	virtual void 					stopStaticAnimations(bool immediateStop, EAnimationGroup groupToStop);
	virtual void					setStaticAnimationTrigger(NdhsCPageTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* pTriggerList);
	virtual NdhsCScrollPosField*	getStaticAnimationField(NdhsCField::IObserver* observer);
	virtual void					releaseStaticAnimationField(NdhsCScrollPosField* field);
	virtual void					resumeStaticAnimations();

	virtual bool					startTransition(ENdhsAnimationType animType,
														bool animate);

	virtual void					resetTransitionCache();
	virtual bool					doPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame);

	inline bool						isTransitioning()	{ return (m_animType == ENdhsAnimationTypeTerminalState); }
	inline ENdhsAnimationType		getAnimType()		{ return (m_animType == ENdhsAnimationTypeDrag
																	|| m_animType == ENdhsAnimationTypeScroll
																	|| m_animType == ENdhsAnimationTypeScrollKick) ? ENdhsAnimationTypeScroll : m_animType; }
	inline bool						getStateChangeAnimComplete()	{ return m_stateChangeAnimComplete; }

	LC_IMPORT virtual bool			processTrigger(
										int code,
										int slot,
										LcTmString& elementClass,
										LcTmArray<NdhsCTemplate::CAction::CAttempt*>* optionsAttempts,
										bool fromModule);

	LC_IMPORT void					processKeyUp(int code);

#ifdef LC_USE_STYLUS
	virtual bool					onMouseDown(NdhsCPageManager::TPageWidgetElem*, const LcTPixelPoint& pt);
	virtual bool					onMouseMove(NdhsCPageManager::TPageWidgetElem*, const LcTPixelPoint& pt);
	virtual bool					onMouseUp(NdhsCPageManager::TPageWidgetElem*, const LcTPixelPoint& pt);
#endif

	inline void						isComponentPartiallyCreated(bool created) { m_isCreated = created; }
	inline bool						isComponentReady() { return m_isCreated == true; }

	inline int						getDefaultTerminalTime()			{ return m_template->getDefaultTerminalTime(); }
	inline ENdhsVelocityProfile		getDefaultTerminalVelocityProfile()	{ return m_template->getDefaultTerminalVelocityProfile(); }
	inline int 						getTransitionDuration()				{ return m_transitionDuration; }
	inline int 						getTransitionDelay()				{ return m_transitionDelay; }

	virtual void					updateTouchDownIntrinsicField(const LcTmString& element, int slot);
#ifdef LC_USE_MOUSEOVER
	virtual void					updateMouseOverIntrinsicField(const LcTmString& element, int slot);
#endif
	// This will return the plug-in index of the active item in the specified plug-in menu.
	virtual int						getHMenuActiveItemIndex(IFX_HMENU hMenu);
	virtual bool 					getFullFilePath(	IFX_HMENU hMenu,
														const LcTmString& searchFile,
														LcTmString& returnFilePath,
														int menuIndex);
	virtual void 					refreshField(const LcTmString& field, int item)
									{
										LC_UNUSED(field) LC_UNUSED(item)
										return;
									}
	// update component status when transitioning states
	virtual void					updatePosition(LcTScalar position,
													bool positionIncreasing,
													bool updateCache,
													bool finalFrame = false);

	virtual NdhsCField*				getPageParamValue(const LcTmString& key);

	// From LCCWidget
	LC_VIRTUAL		void			setVisible(bool b);

	// Unload component
	virtual void					unLoadGroup();
	virtual void					loadGroup();

	// Component enabled or not
	virtual bool					isGroupUnloaded()	{ return m_groupUnloaded; }
	virtual bool					isParentUnloaded()	{ return m_groupUnloaded; }
	virtual bool					isComponentRetired() { return m_componentRetire; }

	virtual bool					trySetFocus();
	virtual void					unsetFocus();
	virtual void					focusSetOnChild(NdhsCComponent* childComponent, NdhsCComponent* focusComponent);
	virtual NdhsCComponent* 		findParentComponent() 				{ return this; }
			bool					isFocusStop()						{ return getTemplate()->isFocusStop(); }
			LcTaString				getDefaultFocus()					{ return getTemplate()->getDefaultFocus(); }
	virtual bool 					moveFocus(int amount, bool wrap);

	// NdhsCField::IObserver callback
	virtual void					fieldValueUpdated(NdhsCField* field);

	// Implement State Manager observer
	virtual void					layoutChanged(NdhsCTemplate::CLayout* newLayout, bool animate);
	virtual void					doAction(NdhsCTemplate::CAction* action, int slotNum = -1);
	virtual bool					doAttempt(NdhsCTemplate::CAction::CAttempt* attempt, int slotNum = -1);
	virtual void					stateManagerDestroyed(NdhsCStateManager* sm);

	// Field context
	virtual NdhsCField*				getField(const LcTmString& field, int slotNum, NdhsCMenuItem* item);

	virtual void 					onSuspend();
	virtual void 					onResume();

	virtual void					updateExtentIntrinsicVaraibles(LcTVector extent, int mask);

			void					checkVisibilityChanges();

	// Inform widgets of placement change
	virtual	void					onPlacementChange(int mask);

	virtual		NdhsCPageModel*		getParentPageRef();

#if defined(NDHS_JNI_INTERFACE)
	virtual LcTaOwner<NdhsCPageManager::CStaticPreviewCache>
									takeSnapShot();
	virtual unsigned int			distanceFromPage();
	virtual unsigned int			getIdentifier()						{ return m_identifier; }
	virtual LcTaString				getFocusChain()						{
																			LcTaString item;
																			if (getLocalMenuItem() != NULL)
																				item = ", item = " + item.fromInt(getLocalMenuItem()->getIndex());
																			return m_parent ? m_parent->getFocusChain() + "->" + getGroupName() + item: getGroupName() + item;
																		}
	virtual LcTaString				getObjectWithFocus()				{ return m_focusedElement ? m_focusedElement->getFocusChain() : (m_focusedChildComponent ? m_focusedChildComponent->getObjectWithFocus() : getFocusChain()); }
#endif
	LcTaString						getTouchDownElement();
};

#endif
