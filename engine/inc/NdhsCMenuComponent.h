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
#ifndef NdhsCMenuComponentH
#define NdhsCMenuComponentH

class NdhsCPageManager;
class NdhsCMenu;
class NdhsCElementGroup;
class NdhsCScrollPosField;

#include "inflexionui/engine/inc/NdhsCTransitionAgent.h"
#include "inflexionui/engine/inc/NdhsCMenuComponentTemplate.h"
#include "inflexionui/engine/inc/NdhsCField.h"

enum ENdhsActiveItem
{
	ENdhsActiveItemOld,
	ENdhsActiveItemCurrent,
	ENdhsActiveItemTarget
};

enum ENdhsScrollUpdateType
{
	ENdhsScrollUpdateNormal,
	ENdhsScrollUpdateInstant
};

class NdhsCMenuComponent : public NdhsCComponent, public NdhsIExpressionObserver
{
private:

	friend class CItem;

	class CItem : public LcCBase
	{
		friend class NdhsCMenuComponent;

	private:
	#ifdef LC_USE_STYLUS
		typedef LcTmArray<NdhsCElement*>	TmAElement;
	#endif

		LcTmOwner<NdhsCElementGroup>	m_egItem;
		LcTmOwner<NdhsCElementGroup>	m_egDetailPage;
		LcTmOwner<NdhsCElementGroup>	m_egDetailMenu;
		bool							m_unloadDetail;
		bool							m_detailRequiresCacheUpdate;

		NdhsCMenuComponent*				m_parent;
		NdhsCMenuItem*					m_menuItem;

	protected:
										CItem(NdhsCMenuComponent* page, NdhsCMenuItem* menuItem);

	public:
		static LcTaOwner<CItem>			create(NdhsCMenuComponent* page, NdhsCMenuItem* menuItem);
		virtual 						~CItem();

		void							loadElementGroups(bool resume);
		virtual bool					componentDoPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame);
		virtual void					componentsJumpTransitionToEnd(bool setIdle);
		virtual void 					componentRefreshField(const LcTmString& field, int item);
		virtual int 					getHMenuActiveItemIndex(IFX_HMENU hMenu);
		virtual	bool					getFullFilePath(	IFX_HMENU hMenu,
																const LcTmString& searchFile,
																LcTmString& returnFilePath,
																int menuIndex);
		virtual bool					isComponentTransitioning();

#if defined(NDHS_JNI_INTERFACE)
		virtual void					componentOnTransitionComplete(bool setIdle);
#endif

		inline NdhsCMenuItem*			getMenuItem()	{ return m_menuItem; }
		inline NdhsCElementGroup*		getItemGroup()	{ return m_egItem.ptr(); }

		LC_IMPORT void					destroyUnwantedElements();

		LC_IMPORT void					loadDetail();
		LC_IMPORT void					unloadDetail();
		LC_IMPORT void					scheduleUnloadDetail();

		LC_IMPORT void					updatePosition(ENdhsAnimationType animType,
													LcTScalar position,
													bool positionIncreasing,
													bool updateCache,
													bool forceNonSketchy,
													bool aggregateAnimating,
													bool finalFrame);
		LC_IMPORT void					updateDetailPosition(ENdhsAnimationType animType,
													LcTScalar position,
													bool positionIncreasing,
													bool updateCache,
													bool forceNonSketchy,
													bool aggregateAnimating,
													bool finalFrame);

		LC_IMPORT void					resetTransitionCache();

		LC_IMPORT void					setSlot(int newSlot, int oldSlot);
		LC_IMPORT void					checkDetails();
		LC_IMPORT int					getSlot()		{return m_egItem->getSlot();}
		LC_IMPORT int					getOldSlot()	{return m_egItem->getOldSlot();}

		LC_IMPORT LcTaString			getItemClassFromWidget(LcCWidget* widget);
		LC_IMPORT LcTaString			getDetailClassFromWidget(LcCWidget* widget);
		LC_IMPORT void					scheduleForDeletion(bool schedule);
		LC_IMPORT bool					isScheduledForDeletion();

	#ifdef LC_USE_STYLUS
		LC_IMPORT bool					populateElementList(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList);
	#endif

		void 							startStaticAnimations();
		void 							stopStaticAnimations();
		virtual void					resumeStaticAnimations();

		void							onSuspend();
		void							onResume();
		void							loadResources();
		void							UnloadResources();
		void							unLoadGroup();
		void							loadGroup();
	};

	// Wrapper class for active-item dependent fields.
	// (If an expression refers to an item field, but without item context,
	// the current active item's context is used instead. This class allows the
	// field in use to update as the active item changes)

	class CMetaField : public NdhsCField, public NdhsCField::IObserver
	{
	private:
		// If a field is set, it is used in preference to the menu / itemIndex data
		NdhsCField*					m_field;
		LcTmString					m_fieldName;
		NdhsCMenu*					m_menu;
		int							m_itemIndex;

	protected:
									CMetaField();
				void				construct(const LcTmString& fieldName);

	public:
		static LcTaOwner<CMetaField>	create(const LcTmString& fieldName);

				void				setField(NdhsCField* field);
				void				setMenuInfo(NdhsCMenu* menu, int itemIndex);

		// NdhsCField::IObserver implementation:
		virtual void 				fieldValueUpdated(NdhsCField* field);
		virtual void 				fieldDirty(NdhsCField* field);
		virtual void 				fieldDestroyed(NdhsCField* field);
#ifdef IFX_SERIALIZATION
		static	CMetaField*			loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
		virtual	SerializeHandle		serialize(LcCSerializeMaster *serializeMaster,bool force=false);
		virtual	void				deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
				ISerializeable*		getSerializeAble(int &type){type=3; return this;}
#endif /* IFX_SERIALIZATION */

		// NdhsCField implementation:
		//   Nearly all of this interface simply forwards to m_field, if set;
		//   otherwise act as a field representing a static menu item's data
		virtual bool				isInput()
										{ return m_field ? m_field->isInput() : false; }
		virtual bool				isOutput()
										{ return m_field ? m_field->isOutput() : true; }

		// Field context
		virtual int					getMenuItemIndex()
										{ return m_field ? m_field->getMenuItemIndex() : -1; }
		virtual NdhsCPlugin::NdhsCPluginMenu* getMenu()
										{ return m_field ? m_field->getMenu() : NULL; }
		virtual LcTaString			getFieldName()
										{ return m_field ? m_field->getFieldName() : m_fieldName; }
		virtual IFXI_FIELD_SCOPE	getBasicScope()
										{ return m_field ? m_field->getBasicScope() : IFXI_FIELD_SCOPE_ITEM; }

		// Data access
		virtual LcTaString			getFieldData(NdhsCElement* element);
		virtual	LcTaString			getFieldData(NdhsCElement* element, bool forceBroadcast);
		virtual LcTScalar			getRawFieldData(NdhsCElement* element)
										{ return m_field ? m_field->getRawFieldData(element) : 0.0f; }
		virtual bool 				getRawFieldDataBool(NdhsCElement* element)
										{ return m_field ? m_field->getRawFieldDataBool(element) : false; }
		virtual LcTScalar			getNormalizedValue(NdhsCElement* element, LcTScalar minValue, LcTScalar maxValue)
										{ return m_field ? m_field->getNormalizedValue(element, minValue, maxValue) : 0.0f; }
		virtual LcTScalar			getTargetValue()
										{ return m_field ? m_field->getTargetValue() : 0.0f; }
		virtual LcTScalar			getVelocity()
										{ return m_field ? m_field->getVelocity() : 0.0f; }
		virtual IFXI_FIELDDATA_TYPE	getFieldType()
										{ return m_field ? m_field->getFieldType() : IFXI_FIELDDATA_STRING; }
		virtual LcTScalar			getSensitivityFactor()
										{ return m_field ? m_field->getSensitivityFactor() : 0.0f; }
		virtual	void				refresh()
										{ if (m_field) m_field->refresh(); }
		virtual void 				setDirtyByPlugin(bool requiresLaundry)
										{ if (m_field) m_field->setDirtyByPlugin(requiresLaundry); }
		virtual void 				setDirtyByAssignment(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item)
										{ if (m_field) m_field->setDirtyByAssignment(expr, context, slot, item); }
		virtual bool				isScrollPosField()
										{ return m_field ? m_field->isScrollPosField() : false; }
		virtual bool				isDraggable()
										{ return m_field ? m_field->isDraggable() : false; }
		virtual void				setUserControlLock(bool lock)
										{ if (m_field) m_field->setUserControlLock(lock); }

		// Field value scrolling
		virtual void				setVelocity(LcTScalar velocity)
										{ if (m_field) m_field->setVelocity(velocity); }
		virtual void				setVelocity(LcTScalar velocity, LcTScalar minValue, LcTScalar maxValue)
										{ if (m_field) m_field->setVelocity(velocity, minValue, maxValue); }
		virtual void				setMaxSpeed(LcTScalar maxSpeed)
										{ if (m_field) m_field->setMaxSpeed(maxSpeed); }
		virtual void				setDeceleration(LcTScalar deceleration)
										{ if (m_field) m_field->setDeceleration(deceleration); }
		virtual void				setInfiniteDeceleration()
										{ if (m_field) m_field->setInfiniteDeceleration(); }
		virtual void				setInfiniteMaxSpeed()
										{ if (m_field) m_field->setInfiniteMaxSpeed(); }
		virtual bool				setTargetValue(LcTScalar target, unsigned int duration)
										{ return m_field ? m_field->setTargetValue(target, duration) : false; }
		virtual bool				setTargetValue(LcTScalar target, LcTScalar minValue, LcTScalar maxValue, unsigned int duration)
										{ return m_field ? m_field->setTargetValue(target, minValue, maxValue, duration) : false; }
		virtual bool				updateValue(LcTTime timestamp, bool& finalUpdate)
										{ return m_field ? m_field->updateValue(timestamp, finalUpdate) : false; }
		virtual void				setValue(LcTScalar newValue, bool finalUpdate)
										{ if (m_field) m_field->setValue(newValue, finalUpdate); }

		virtual void				setValue(LcTmString newValue, bool finalUpdate)
										{ if (m_field) m_field->setValue(newValue, finalUpdate); }
		virtual void				setValueBool(bool newValue, bool finalUpdate)
										{ if (m_field) m_field->setValueBool(newValue, finalUpdate); }
		virtual void				takeSnapshot()
										{ if (m_field) m_field->takeSnapshot(); }
		virtual void				addToSnapshot(LcTScalar dValue, LcTScalar minValue, LcTScalar maxValue, bool finalUpdate)
										{ if (m_field) m_field->addToSnapshot(dValue, minValue, maxValue, finalUpdate); }
		virtual void				setValueFromExpression(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item)
										{ if (m_field) m_field->setValueFromExpression(expr, context, slot, item); }

		virtual void                cancelMovement()
										{ if (m_field) m_field->cancelMovement(); }
		virtual bool				atRest()
										{ return m_field ? m_field->atRest() : true; }

		virtual void				bindExpression(LcTmOwner<NdhsCExpression>& expr)
										{ if (m_field && expr) m_field->bindExpression(expr); }

		virtual void				setIgnoreLaundry(bool ignoreLaundry)
										{ if (m_field) m_field->setIgnoreLaundry(ignoreLaundry); }

		// Expression observer
		virtual void 				expressionDirty(NdhsCExpression* expr)
										{ if (m_field) m_field->expressionDirty(expr); }
		virtual void 				expressionDestroyed(NdhsCExpression* expr)
										{ if (m_field) m_field->expressionDestroyed(expr); }

		// NdhsILaundryItem
		virtual bool 				cleanLaundryItem(LcTTime timestamp)
										{ return m_field ? m_field->cleanLaundryItem(timestamp) : false; }
		virtual bool 				stillDirty()
										{ return m_field ? m_field->stillDirty() : false; }
		virtual void 				addedToLaundry()
										{ if (m_field) m_field->addedToLaundry(); }

		virtual 					~CMetaField();
	};

	// Menu pointer that the current component using
	NdhsCMenu*						m_menu;


	NdhsCElementGroup*				m_furnitureElementGroup;
	NdhsCMenuComponentTemplate*		m_menuTemplate;

	// Animators
	bool							m_scrollPositive;
	bool							m_ignoreScrollPosUpdate;
	bool							m_ignoreScrollPosNextItem;
	ENdhsScrollUpdateType			m_scrollUpdateType;
	LcTMessage						m_replacePlaceholdersMessage;
	LcTMessage						m_reloadDataSourceMessage;
	bool							m_scrollPosIntrinsicStale;


	LcTmOwner<NdhsCScrollPosField>	m_scrollPos;
	LcTmOwner<NdhsCScrollPosField>	m_lastActiveItemScrollPos;

	bool							m_menuLoaded;

	// Set if additional tasks required on a onTransitionComplete call
	bool							m_bRequiresPostTransitionComplete;

	// Items (ordered)
	typedef LcTmOwnerArray<CItem>	TmOAItems;
	TmOAItems						m_items;

	// Items (ordered)
	typedef LcTmOwnerArray<NdhsCField>	TmOAPlaceHolderField;
	TmOAPlaceHolderField			m_placeHolderFields;

	typedef LcTmArray<CItem*>		TmAItems;

	int								m_itemCount;
	int								m_windowStart;
	int								m_windowSize;
	bool							m_placeholderUsed;

	// Slots
	typedef LcTmArray<int>			TmASlots;
	TmASlots						m_slotRefs;
	TmASlots						m_oldSlotRefs;
	TmASlots						m_cachedSlotRefs;
	TmAItems						m_prevSlotItems;

	int								m_activeSlot;
	int								m_prevActiveSlot;
	int 							m_pendingHideStateSlotActive;

	// Last active remembers which slot was active
	// before going to an inactive hide/selected
	int								m_lastActiveSlot;

	LcTmOwner<NdhsCExpression>		m_dataSourceExp;
	LcTmString						m_dataSourceString;
	LcTmOwnerMap<LcTmString, CMetaField> m_metaFields;

	// General static slot configuration
	int								m_slotCount;
	int								m_firstActiveSlot;
	int								m_firstSelectableSlot;
	int								m_lastSelectableSlot;
	int								m_firstPopulateSlot;
	bool							m_fullMode;
	bool							m_circularMode;
	bool							m_scrollWrap;
	int								m_scrollSpan;


	// Token replacer and manifest stack level
	int								m_stackLevel;

	// The active item index we last notified the menu about
	int								m_lastReportedActiveItem;

#ifdef IFX_USE_PLUGIN_ELEMENTS
	// Current tab order index. (Vector index stored in NdhsCTemplate).
	// Focus Data.
	int								m_tabOrderIndex;
	NdhsCTemplate::TTabData			m_currentFocusClassData;
	NdhsCElement*					m_currentFocusElement;
	NdhsCElement*					m_previousFocusElement;
#endif

	LcTmString						m_previouslyFocusedClass;

	// Delayed page refresh flag
	bool							m_refreshNeeded;

	inline NdhsCElementGroup*		getMenuAggregate() { if (m_outerGroup.ptr() != NULL) return m_outerGroup->getGroup("menu"); else return NULL; }

	int								getItemIndex(int slotNum, NdhsCMenuItem* item);
	void 							updateMetaFields();
	void							backupFocus();
	void							restoreFocus();

	int								getInactivePopulateSlot();

public:

	// Slot/item assignment functions
	void							assignItemsToSlots(int firstActiveItem,int restoreActiveSlot=-1);
	void							updateSlots(bool wrap);
	bool							scrollItems(NdhsCMenuComponentTemplate::CAction::CScrollBy* scrollAttempt);
	bool							isScrollDirectionPositive() {return m_scrollPositive;}
	void							deleteItems();

	void 							prepareComponentForMenu();
	void 							reloadComponentForMenu();
	virtual void					prepareComponent();
	virtual void					reloadFurniture();
	virtual void					reloadMenuAggregate(bool realize);
	void							reloadItems(int firstActiveItem=-1,int restoreActiveSlot=-1);
	void							doLoadOnDemand(int activeItem,int currentIndex=-1);
	void							loadItems(int start, int amount, bool usePlaceholder,bool doingLOD=false);
	void							unloadItems(int start, int amount);
	bool							isWrappable();
	int								wrapToRange(int val, int max);
	bool 							nearOriginSlot();
	void							setParent(NdhsCElementGroup* parent) { m_parent = parent; }
	NdhsCElementGroup*				getParent() { return m_parent; }
	bool 							loadDataSource();
	bool							getSlotAndClassFromWidget(LcCWidget* widget, LcTmString &elementClass, int* slot);

	virtual NdhsCElement*			getItem(const LcTmString& elementClass);
	virtual NdhsCElementGroup*		getGroup(const LcTmString& groupName);

#ifdef LC_USE_STYLUS
	virtual bool					onMouseUp(NdhsCPageManager::TPageWidgetElem*, const LcTPixelPoint& pt);
#endif

	// Access functions
	CItem*							getItemInSlot(int s);
	int								getRefFromSlot(int s);

	int								getActiveItem(ENdhsActiveItem timepoint, bool clampItem = true);
	void 							setActiveItem(int activeItem);

	int								getSelectableSlotCount()	{ return m_lastSelectableSlot - m_firstSelectableSlot + 1; }
	bool							isSlotSelectable(int s)		{ return (s >= m_firstSelectableSlot && s <= m_lastSelectableSlot); }

	bool							executeChainedAction(const LcTmString& action, int slotNum);

#ifdef LC_USE_STYLUS
	// Widget to Element map
	typedef LcTmMap<LcCWidget*, NdhsCElement*>	TmMElemMap;
#endif

LC_PRIVATE_INTERNAL_PUBLIC:

#ifdef LC_USE_STYLUS
	virtual void					populateWidgetElementMap(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList);
#endif

protected:

	// Allow only 2-phase construction, default constructor for Serialization
									NdhsCMenuComponent(	NdhsCPageManager*					pageManager,
														NdhsCTemplate::CComponentElement*	menuComp,
														NdhsCElementGroup*					parent,
														int									stackLevel,
														NdhsCMenu*							menu,
														NdhsCMenuItem*						menuItem,
														NdhsCElementGroup*					parentGroup);

	virtual void					doPostConstruct();

public:

	// Creation/destruction
	static LcTaOwner<NdhsCMenuComponent> create(NdhsCPageManager*					pageManager,
												NdhsCTemplate::CComponentElement*	menuComp,
												NdhsCElementGroup*					parent,
												int									stackLevel,
												NdhsCMenu*							menu,
												NdhsCMenuItem*						menuItem,
												NdhsCElementGroup*					parentGroup);

	virtual							~NdhsCMenuComponent();
	virtual void					doCleanup();

	void							setActiveSlot(int activeSlot);
	inline int						getActiveSlot()		{ return m_activeSlot; }
	inline int						getPrevActiveSlot()	{ return m_prevActiveSlot; }
	int								getSlotContainingItem(CItem* item);

	inline NdhsCMenuItem*			getActiveMenuItem()	{ return getItemInSlot(m_activeSlot) ? getItemInSlot(m_activeSlot)->getMenuItem() : NULL; }
	inline NdhsCMenuItem*			getMenuItemInSlot(int slot)	{ if (slot == -1 || getItemInSlot(slot)==NULL) return NULL; else return getItemInSlot(slot)->getMenuItem(); }

	LC_IMPORT NdhsCField*			getFieldValue(const LcTmString& fieldName,
													NdhsCMenu* menu,
													int item,
													NdhsCElement* element);

	virtual NdhsCTemplate*			getTemplate()			{ return m_menuTemplate; }
	virtual NdhsCMenuComponentTemplate*
									getMenuTemplate()			{ return m_menuTemplate; }
	virtual void					setMenuTemplate(NdhsCMenuComponentTemplate* menuTemplate)
																{ m_menuTemplate = menuTemplate; }

	inline NdhsCMenu*				getMenu()					{ return m_menu; }
	inline int						getStackLevel()				{ return m_stackLevel; }
	inline NdhsCTransitionAgent*	getTransitionAgent()		{ return m_transitionAgent.ptr(); }
	inline int						getSlotCount()				{ return m_slotCount; }

	void							destroyUnwantedElements();
	void							replacePlaceholders();
	void							onMessage(int iID, int iParam);
	void 							scheduleReplacePlaceholders();
	void							resetTransitionCache();

	void							loadMenuComponent();

	virtual void					refreshPage(IFX_HMENU hMenu, bool immediate);
			void					refreshMenuFirstStep();
			void					refreshMenuSecondStep();
			void					scheduleReloadDataSource();
#ifdef IFX_SERIALIZATION
									NdhsCMenuComponent():m_replacePlaceholdersMessage(this,EReplacePlaceholdersMsg),
														 m_reloadDataSourceMessage(this,EReloadDataSource){}
			void					deflate();
	static	NdhsCMenuComponent*		loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual	SerializeHandle			serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual	void					deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
			ISerializeable*			getSerializeAble(int &type){if(getMenuItem()!=NULL){type=-1; return NULL;}type=5; return this;};
	static	NdhsCField::IObserver*	loadMetaFieldState(SerializeHandle handle,
													   LcCSerializeMaster *serializeMaster,
													   NdhsCField *&field);
#endif /* IFX_SERIALIZATION */

	inline bool						areStaticAnimationsStopping()	{ return m_stopStaticAnimationAll || m_stopStaticAnimationItem; }

	// update page status when transitioning states
	void							updatePosition(LcTScalar position,
													bool positionIncreasing,
													bool updateCache,
													bool finalFrame = false);

	bool							updateFields(LcTTime timestamp);
	virtual void					updateTouchDownIntrinsicField(const LcTmString& element, int slot);
#ifdef LC_USE_MOUSEOVER
	virtual void					updateMouseOverIntrinsicField(const LcTmString& element, int slot);
#endif

	// update page status when scrolling
	virtual bool					doPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame);


	LC_IMPORT virtual bool			componentDoPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame);
	LC_IMPORT virtual void			componentsJumpTransitionToEnd(bool setIdle);
	LC_IMPORT virtual void 			componentRefreshField(const LcTmString& field, int item);
	LC_IMPORT virtual int 			getHMenuActiveItemIndex(IFX_HMENU hMenu);
	LC_IMPORT virtual bool			getFullFilePath(	IFX_HMENU hMenu,
																const LcTmString& searchFile,
																LcTmString& returnFilePath,
																int menuIndex);
	LC_IMPORT virtual bool			isComponentTransitioning();

#if defined(NDHS_JNI_INTERFACE)
	LC_IMPORT virtual void			componentOnTransitionComplete(bool setIdle);
	virtual LcTaOwner<NdhsCPageManager::CStaticPreviewCache>
									takeSnapShot();
#endif

	LC_IMPORT bool					changeState(ENdhsPageState state, int activeSlot, const NdhsCTemplate::CAction::CAttempt* attempt, bool animate = true);
	LC_IMPORT void					jumpTransitionToEnd(bool setIdle);
	LC_IMPORT void					onTransitionComplete(bool setIdle, bool generateSignal = false, bool forceSlotUpdate = false);
	LC_IMPORT void					postTransitionComplete();
	bool							startTransition(ENdhsAnimationType animType,
														bool animate);

	bool 							startStaticAnimations(EAnimationGroup groupToStart);
	void 							stopStaticAnimations(bool immediateStop, EAnimationGroup groupToStop);
	virtual void					resumeStaticAnimations();

	virtual void 					refreshField(const LcTmString& field, int item);

	void							requestSetActiveItem(int item);
	virtual NdhsCField*				getPageParamValue(const LcTmString& key);

	// From LCCWidget
	LC_VIRTUAL		void			setVisible(bool b);

	virtual void 					onSuspend();
	virtual void 					onResume();

	// Unload menu component
	virtual void					unLoadGroup();
	virtual void					loadGroup();

	// Menu component enabled or not
	virtual bool					isGroupUnloaded() { return m_groupUnloaded; }

	// NdhsCField::IObserver callback
	virtual void					fieldValueUpdated(NdhsCField* field);
	virtual void					reloadElement();

	// Implement State Manager observer
	virtual void					doAction(NdhsCTemplate::CAction* action, int slotNum = -1);
	virtual bool					doAttempt(NdhsCTemplate::CAction::CAttempt* attemp, int slotNum = -1);

	virtual void					stateManagerDestroyed(NdhsCStateManager* sm);

	// Field context
	virtual NdhsCField*				getField(const LcTmString& field, int slotNum, NdhsCMenuItem* item);
	virtual LcTaString				getString(bool& found, const LcTmString& field, int slotNum, NdhsCMenuItem* item);

	// NdhsIExpressionObserver interface
	virtual void 					expressionDirty(NdhsCExpression* expr);
	virtual void 					expressionDestroyed(NdhsCExpression* expr);
	virtual bool					isImmediateMenu(NdhsCMenuItem *menuItem){return getMenuItem()!=menuItem;}

	virtual void					createIntrinsicFields();

	virtual void					unsetFocus();
};

#endif
