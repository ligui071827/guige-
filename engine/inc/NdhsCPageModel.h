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
#ifndef NdhsCPageModelH
#define NdhsCPageModelH

class NdhsCPageManager;
class NdhsCElementGroup;
class NdhsCScrollPosField;

#include "inflexionui/engine/inc/NdhsCTransitionAgent.h"
#include "inflexionui/engine/inc/NdhsCPageTemplate.h"
#include "inflexionui/engine/inc/NdhsCPage.h"
#include "inflexionui/engine/inc/NdhsCField.h"
#include "inflexionui/engine/inc/NdhsCStateManager.h"
#include "inflexionui/engine/inc/NdhsIFieldContext.h"
#include "inflexionui/engine/inc/NdhsCFieldCache.h"


class NdhsCPageModel : public NdhsCElementGroup, public NdhsCStateManager::IObserver
{

private:
	enum EAnimationGroup
	{
		EAnimationAll,
		EAnimationItemOnly
	};

	NdhsCPageManager*				m_pageManager;
	NdhsCPageTemplate*				m_pageTemplate;

	typedef LcTmMap<LcTmString, LcTmOwner<NdhsCField> >
									TmMPageParams;
	TmMPageParams					m_paramMap;

	LcTmString						m_link;

#if defined(NDHS_JNI_INTERFACE)
	unsigned int					m_identifier;
#endif

	// Contain navigation informatin and menu data set
	NdhsCPage*						m_uiNode;

	// Menu elements and aggregates
	LcTmOwner<NdhsCElementGroup>	m_outerGroup;
	LcTmOwner<NdhsCElementGroup>	m_localScreenAggregate;

	LcTmOwner<NdhsCStateManager>	m_stateManager;
	LcTmOwner<NdhsCFieldCache>		m_fieldCache;

	NdhsCTemplate::CLayout* 		m_currentLayout;

	bool							m_isLayoutChange;
	bool							m_insideChangeState;

	LcTmString						m_templatePath;

	LcTmOwner<NdhsCScrollPosField>	m_stateChange;
	LcTTime							m_terminalAnimTimestamp;
	LcTTime							m_interactiveAnimTimestamp;
	bool							m_scrollPositive;
	ENdhsAnimationType 				m_animType;
	ENdhsAnimationType 				m_previousAnimType;
	bool							m_bMouseDown;
	bool							m_ignoreScrollPosUpdate;
	bool							m_lastFrame;
	bool							m_isEligible;
#ifdef LC_USE_STYLUS
	typedef LcTmArray<NdhsCElement*>	TmAElement;
	NdhsCElement*					m_mouseFocusElement;
#endif

	NdhsCComponent*					m_focusedTargetComponent;
	NdhsCComponent*					m_focusedChildComponent;
	NdhsCElement*					m_focusedElement;

	// Set if additional tasks required on a onTransitionComplete call
	bool							m_bRequiresPostTransitionComplete;

	int								m_backgroundDelay;
	int 							m_transitionDuration;
	int 							m_transitionDelay;
	ENdhsVelocityProfile 			m_transitionVelProfile;
	int								m_primaryLightDelay;
	int								m_primaryLightDuration;

	// States
	ENdhsPageState					m_pageState;
	ENdhsPageState					m_prevPageState;

	// Token replacer and manifest stack level
	int								m_stackLevel;
	// Delayed page refresh flag
	bool							m_refreshNeeded;

	// Cached transition agent
	LcTmOwner<NdhsCTransitionAgent>	m_transitionAgent;

	// Cache transition data for efficiency
	LcTPlacement 					m_primaryLightBaseStart; // cached endpoint for primary light transition
	int								m_primaryLightBaseStartMask; // cached endpoint placement mask for primary light transition
	LcTPlacement 					m_primaryLightBaseEnd; // cached endpoint for primary light transition
	int								m_primaryLightBaseEndMask; // cached endpoint placement mask for primary light transition
	NdhsCPath	 					m_primaryLightBasePath; // Path between m_primaryLightBaseStart and m_primaryLightBaseEnd
	bool							m_primaryLightAnimate; // True if primary light is animating

	LcTPlacement 					m_currentPrimaryLightPlacement; // most recent calculated primary light placement cache
	LcTPlacement 					m_activePrimaryLightPlacement; // most recent active layout primary light placement cache
	int			 					m_activePrimaryLightMask; // most recent active layout primary light placement mask

	LcTmOwnerArray<NdhsCScrollPosField> m_staticAnimFields; // array of all fields controlling static animations
	bool							m_stopStaticAnimationAll; // if true, all static animation should terminate before next loop
	bool							m_stopStaticAnimationItem; // if true, item static animation should terminate before next loop
	NdhsCPageTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* m_pAnimTriggerList;
	NdhsCPageTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* m_pStaticTriggerList;
	NdhsCScrollPosField* 			m_pStaticAnimFieldTrigger; // pointer to field controlling static transition triggers
	LcTScalar						m_lastTriggerPos;
	LcTScalar						m_lastStaticTriggerPos;
	EAnimationGroup					m_staticAnimGroup;

	// Slot/item assignment functions

#ifdef LC_USE_STYLUS
	// Widget to Element map
	typedef LcTmMap<LcCWidget*, NdhsCElement*>	TmMElemMap;
#endif

public:

LC_PRIVATE_INTERNAL_PUBLIC:

#ifdef LC_USE_STYLUS
	virtual void					populateWidgetElementMap(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList);
	virtual void					populateElementList(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList);
	virtual bool					isCatchAll()		{ return m_stateManager ? m_stateManager->isCatchAll() : false; }
#endif

	LC_IMPORT virtual void			componentsJumpTransitionToEnd(bool setIdle);
	LC_IMPORT virtual bool			componentDoPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame);
	LC_IMPORT virtual void 			componentRefreshField(const LcTmString& field, int item);

#if defined(NDHS_JNI_INTERFACE)
	LC_IMPORT virtual void			componentOnTransitionComplete(bool setIdle);
#endif

	bool							executeChainedAction(const LcTmString& action, int slotNum);
	void							updatePageStateField(ENdhsPageState pageState);
	bool							parsePageParamters();

protected:
	// Allow only 2-phase construction
									NdhsCPageModel(NdhsCPageManager* pageManager, NdhsCPageTemplate* templateFile, NdhsCPage* page, const LcTmString& link, int stackLevel, LcTmString& templatePath);

	virtual	void					onRetire();

	bool 							trySetFocus(const LcTmString& className);
	void 							setFocusIntrinsics(const LcTmString& focusedClass);

public:
	// Creation/destruction
	static LcTaOwner<NdhsCPageModel> create(NdhsCPageManager* pageManager, NdhsCPageTemplate* templateFile, NdhsCPage* page, const LcTmString& link, int stackLevel, LcTmString& templatePath);
	virtual							~NdhsCPageModel();
									NdhsCPageModel(){}
	void							postConstruct();
	virtual void 					onRealize();
	void							createFieldCache(NdhsCTemplate* templ);
	inline ENdhsPageState			getPageState()				{ return m_pageState; }
	inline ENdhsPageState			getPrevPageState()			{ return m_prevPageState; }
	inline NdhsCPage*				getPage()					{ return m_uiNode; }
	inline NdhsCTemplate*			getTemplate()				{ return m_pageTemplate; }
	inline NdhsCPageManager*		getPageManager()			{ return m_pageManager; }
	inline NdhsCElementGroup*		getPageAggregate()			{ if (m_outerGroup.ptr() != NULL )return m_outerGroup->getGroup("furniture"); else return NULL;}
	inline NdhsCElementGroup*		getOuterGroupAggregate()	{ return m_outerGroup.ptr(); }
	inline LcCAggregate*			getLocalScreenAggregate()	{ return m_localScreenAggregate.ptr(); }
	inline int						getStackLevel()				{ return m_stackLevel; }
	inline NdhsCTransitionAgent*	getTransitionAgent()		{ return m_transitionAgent.ptr(); }
	inline int						getBackgroundDelay()		{ return m_backgroundDelay; }

	inline ENdhsParentVisibility	getParentVisibility()		{ return m_parentVisibility; }
	inline bool						getShowWhenInSub()			{ return m_showWhenInSub; }
 	inline int						getDefaultTerminalTime()			{ return m_pageTemplate->getDefaultTerminalTime(); }
	inline ENdhsVelocityProfile		getDefaultTerminalVelocityProfile()	{ return m_pageTemplate->getDefaultTerminalVelocityProfile(); }
	bool							isTransitioning();

	inline int						getPrimaryLightDelay()				{ return m_primaryLightDelay; }
	inline int						getPrimaryLightDuration()			{ return m_primaryLightDuration; }
	inline int 						getTransitionDuration()				{ return m_transitionDuration; }
	inline int 						getTransitionDelay()				{ return m_transitionDelay; }

	inline ENdhsAnimationType		getAnimType()		{ return (m_animType == ENdhsAnimationTypeDrag
																	|| m_animType == ENdhsAnimationTypeScroll
																	|| m_animType == ENdhsAnimationTypeScrollKick) ? ENdhsAnimationTypeScroll : m_animType; }
	inline bool						getStateChangeAnimComplete()	{ return m_stateChangeAnimComplete; }
	inline bool						canBeDeleted()		{ return (m_pageState == ENdhsPageStateClose); }
	void							destroyUnwantedElements();
	void 							resetTransitionCache();

	bool							updateFields(LcTTime timestamp);

	LC_IMPORT void					refreshPage(IFX_HMENU hMenu, bool immediate);

	LC_IMPORT NdhsCField*			getFieldValue(const LcTmString& fieldName,
													NdhsCMenu* menu,
													int item,
													NdhsCElement* element);

	LC_IMPORT bool					processTrigger(
										int code,
										int slot,
										LcTmString& elementClass,
										LcTmArray<NdhsCTemplate::CAction::CAttempt*>* optionsAttempts,
										bool fromModule);

	LC_IMPORT void					processKeyUp(int code);

	void							reloadFurniture();

#ifdef LC_USE_STYLUS
	bool							onMouseDown(NdhsCPageManager::TPageWidgetElem*, const LcTPixelPoint& pt);
	bool							onMouseMove(NdhsCPageManager::TPageWidgetElem*, const LcTPixelPoint& pt);
	bool							onMouseUp(NdhsCPageManager::TPageWidgetElem*, const LcTPixelPoint& pt);
#endif

	// update page status when transitioning states
	void							updatePosition(LcTScalar position,
													bool positionIncreasing,
													bool updateCache,
													bool finalFrame = false);

	// update page status when scrolling
	virtual bool					doPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame);

	void							setPrimaryLightTransitionBasePlacements(LcTPlacement start,
													int startMask,
													LcTPlacement end,
													int endMask);

	inline void						getActivePrimaryLightConfig(LcTPlacement& placement, int& mask)
																	{ placement = m_activePrimaryLightPlacement; mask = m_activePrimaryLightMask; }


	// From LCCWidget
	LC_VIRTUAL		void			setVisible(bool b);

	// NdhsCField::IObserver callback
	virtual void					fieldValueUpdated(NdhsCField* field);
	virtual void 					fieldDestroyed(NdhsCField* field);

	// Chained action
	LcTmString						m_chainedAction;
	int								m_chainedActionSlot;

	bool							m_stateChangeAnimComplete;
	bool							m_staticTransitionDone;
	bool							m_jumpingToEnd;
	ENdhsParentVisibility			m_parentVisibility;
	bool							m_showWhenInSub;
	LcTmString						m_cleanupLink;

	LC_IMPORT bool					changeState(ENdhsPageState state, int activeSlot, const NdhsCPageTemplate::CAction::CAttempt* attempt, bool animate = true, bool statisPreview = false, bool generateSignal = true);
	LC_IMPORT void					jumpTransitionToEnd(bool setIdle);
	LC_IMPORT void					onTransitionComplete(bool setIdle, bool generateSignal = false);
	LC_IMPORT void					postTransitionComplete();

	bool							startTransition(ENdhsAnimationType animType,
														bool animate);

	inline void						setPageTransitionDecoration(NdhsCPageTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* pTriggerList)
																	{ m_pAnimTriggerList = pTriggerList; m_lastTriggerPos = -1; }
	bool 							startStaticAnimations(EAnimationGroup groupToStart);
	void 							stopStaticAnimations(bool immediateStop, EAnimationGroup groupToStop);
	virtual void					setStaticAnimationTrigger(NdhsCPageTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* pTriggerList);
	NdhsCScrollPosField*			getStaticAnimationField(NdhsCField::IObserver* observer);
	void							releaseStaticAnimationField(NdhsCScrollPosField* field);
	void							resumeStaticAnimations();

	bool							checkNaviation(const LcTmString& uri);
	virtual NdhsCField*				getPageParamValue(const LcTmString& key);

	// This will return the plug-in index of the active item in the specified plug-in menu.
	int								getHMenuActiveItemIndex(IFX_HMENU hMenu);
	bool 							getFullFilePath(	IFX_HMENU hMenu,
										const LcTmString& searchFile,
										LcTmString& returnFilePath,
										int index);
	bool 							requestSetActiveItem(	IFX_HMENU hMenu,
														int itemIndex);
	NdhsCMenu*						getMenu() { return NULL; }

	virtual void					updateTouchDownIntrinsicField(const LcTmString& element, int slot);
#ifdef LC_USE_MOUSEOVER
	virtual void					updateMouseOverIntrinsicField(const LcTmString& element, int slot);
#endif
	virtual void					unLoadGroup(){}
	virtual void					loadGroup() {}
	virtual bool					isGroupUnloaded() { return false; }
	virtual bool					isParentUnloaded() { return false; }

	// Implement State Manager observer
	virtual void					layoutChanged(NdhsCTemplate::CLayout* newLayout,bool animate);
	virtual void					doAction(NdhsCTemplate::CAction* action, int slotNum = -1);
	virtual bool					doAttempt(NdhsCTemplate::CAction::CAttempt* action, int slotNum = -1);
	virtual void					stateManagerDestroyed(NdhsCStateManager* sm);

	// Field context
	virtual NdhsCField*				getField(const LcTmString& field, int slotNum, NdhsCMenuItem* item);

	void 							onSuspend();
	void 							onResume();

	int								getDrawLayerIndex() {return m_pageTemplate->getDrawLayerIndex(); }
#if defined(NDHS_JNI_INTERFACE)
	void							forceLayout(LcTmString layout)	{ m_stateManager->forceLayout(layout); }
	LcTaOwner<NdhsCPageManager::CStaticPreviewCache>
									takeSnapShot();
	virtual unsigned int			distanceFromPage();
	virtual unsigned int			getIdentifier() { return m_identifier; }
	virtual LcTaString				getFocusChain()					{ return m_uiNode->getPageName();}
#endif //defined(NDHS_JNI_INTERFACE)

	virtual bool					trySetFocus();
	virtual void					unsetFocus();
	virtual bool 					moveFocus(int amount, bool wrap);
	virtual void					focusSetOnChild(NdhsCComponent* childComponent, NdhsCComponent* focusComponent);
			void					pageWideUnsetFocus();
	virtual NdhsCComponent* 		findParentComponent() 		{ return NULL; }
	virtual NdhsCPageModel* 		findParentPageModel()		{ return this; }

			void					checkVisibilityChanges();

#ifdef IFX_SERIALIZATION
	static	NdhsCPageModel*			loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
			SerializeHandle			serialize(LcCSerializeMaster *serializeMaster,bool force=false);
			void					deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
			bool					isMenuItemChild(){return false;}
#endif /* IFX_SERIALIZATION */
			virtual	inline NdhsCPageModel*	getParentPageRef() { return this; }
			LcTaString				getTouchDownElement();
};

#endif
