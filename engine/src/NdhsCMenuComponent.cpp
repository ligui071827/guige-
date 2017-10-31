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

#include "inflexionui/engine/inc/LcAll.h"
#include "inflexionui/engine/inc/NdhsCScrollPosField.h"
#include "inflexionui/engine/inc/NdhsCTemplate.h"
#include "inflexionui/engine/inc/NdhsCMenuComponentTemplate.h"

#include <math.h>

#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


#define ANIM_NON_SKETCHY_THRESHOLD .001
#define AGG_MASK (LcTPlacement::ELocation | LcTPlacement::EScale | LcTPlacement::EOrientation | LcTPlacement::EOpacity)
#define PRIMARY_LIGHT_MASK (LcTPlacement::EColor | LcTPlacement::EColor2 | LcTPlacement::EOrientation)

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuComponent::CItem> NdhsCMenuComponent::CItem::create(NdhsCMenuComponent* page,
																NdhsCMenuItem* menuItem)
{
	LcTaOwner<NdhsCMenuComponent::CItem> ref;
	ref.set(new CItem(page, menuItem));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuComponent::CItem::CItem(NdhsCMenuComponent* parent, NdhsCMenuItem* menuItem)
{
	m_parent = parent;
	m_menuItem = menuItem;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuComponent::CItem::~CItem()
{
	if (m_egDetailPage)
	{
		m_egDetailPage->retire();
		m_egDetailMenu->retire();
	}

	if (m_egItem)
	{
		m_egItem->retire();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CItem::loadElementGroups(bool resume)
{

	if (m_egDetailPage)
	{
		m_egDetailPage->retire();
		m_egDetailPage.destroy();
		m_egDetailMenu->retire();
		m_egDetailMenu.destroy();
	}

	if (m_egItem)
	{
		m_egItem->retire();
		m_egItem.destroy();
	}

	// Create the item element group
	LcTaOwner<NdhsCElementGroup> newEgItem = m_parent->getTemplate()->createElementGroup(	m_parent,
																							m_parent->getMenu(),
																							m_menuItem,
																							"slot",
																							m_parent->getStackLevel(),
																							m_parent->getDrawLayerIndex(),
																							m_parent);
	m_egItem = newEgItem;


	if(resume)
		m_egItem->onResume();

	m_egItem->realize(m_parent->getMenuAggregate());

	m_egItem->setVisible(false);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::CItem::componentDoPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame)
{

	bool rechedule = false;

	if (m_egItem)
	{
		if (m_egItem->componentDoPrepareForFrameUpdate(timestamp, finalFrame))
			rechedule = true;
	}

	return rechedule;

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CItem::componentsJumpTransitionToEnd(bool setIdle)
{

	if (m_egItem)
	{
		m_egItem->componentsJumpTransitionToEnd(setIdle);
	}

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CItem::resumeStaticAnimations()
{

	if (m_egItem)
	{
		m_egItem->resumeStaticAnimations();
	}

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CItem::componentRefreshField(const LcTmString& field, int item)
{

	if (m_egItem)
	{
		m_egItem->componentRefreshField(field, item);
	}

}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCMenuComponent::CItem::getHMenuActiveItemIndex(IFX_HMENU hMenu)
{

	if (m_egItem)
	{
		int index = m_egItem->getHMenuActiveItemIndex(hMenu);
		if (index != -1)
			return index;
	}
	return -1;

}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::CItem::getFullFilePath(	IFX_HMENU hMenu,
														const LcTmString& searchFile,
														LcTmString& returnFilePath,
														int menuIndex)
{

	if (m_egItem)
	{
		if (m_egItem->getFullFilePath(hMenu, searchFile, returnFilePath, menuIndex))
			return true;
	}
	return false;

}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::CItem::isComponentTransitioning( )
{

	if (m_egItem)
	{
		if (m_egItem->isComponentTransitioning())
			return true;
	}
	return false;

}

#if defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CItem::componentOnTransitionComplete(bool setIdle)
{

	if (m_egItem)
	{
		m_egItem->componentOnTransitionComplete(setIdle);
	}

}
#endif

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::loadDetail()
{
	// If the detail group already exists, ignore this request
	if (m_egDetailPage)
		return;

	// Create the detail element group for the page
	m_egDetailPage = m_parent->getTemplate()->createElementGroup(	 m_parent,
																 m_parent->getMenu(),
																 m_menuItem,
																 "detail_page",
																 m_parent->getStackLevel(),
																 m_parent->getDrawLayerIndex(),
																 m_parent);

	// Configure the detail
	if (m_egDetailPage)
	{
		m_egDetailPage->onResume();
		m_egDetailPage->setVisible(true);
		m_egDetailPage->realize(m_parent->getFurnitureAggregate());

		// We must call set slot twice to make sure that its previous
		// slot is set correctly.  This will make sure it uses the correct
		// animation settings
		m_egDetailPage->setSlot(m_egItem->getOldSlot());
		m_egDetailPage->setSlot(m_egItem->getSlot());

		m_unloadDetail = false;
		m_detailRequiresCacheUpdate = true;
	}

	// Create the detail element group for the menu
	m_egDetailMenu = m_parent->getTemplate()->createElementGroup(	 m_parent,
																 m_parent->getMenu(),
																 m_menuItem,
																 "detail_menu",
																 m_parent->getStackLevel(),
																 m_parent->getDrawLayerIndex(),
																 m_parent);

	// Configure the detail
	if (m_egDetailMenu)
	{
		m_egDetailMenu->onResume();
		m_egDetailMenu->setVisible(true);
		m_egDetailMenu->realize(m_parent->getMenuAggregate());

		// We must call set slot twice to make sure that its previous
		// slot is set correctly.  This will make sure it uses the correct
		// animation settings
		m_egDetailMenu->setSlot(m_egItem->getOldSlot());
		m_egDetailMenu->setSlot(m_egItem->getSlot());
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::unloadDetail()
{
	if (m_egDetailPage)
	{
		m_egDetailPage->retire();
		m_egDetailPage.destroy();
		m_egDetailMenu->retire();
		m_egDetailMenu.destroy();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::scheduleUnloadDetail()
{
	if (m_egDetailPage)
		m_unloadDetail = true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::updatePosition(ENdhsAnimationType animType,
													LcTScalar position,
													bool positionIncreasing,
													bool updateCache,
													bool forceNonSketchy,
													bool aggregateAnimating,
													bool finalFrame)
{
	m_egItem->updatePosition(animType, position, positionIncreasing, updateCache,
										forceNonSketchy, aggregateAnimating, finalFrame);

	if (m_egDetailPage)
		updateDetailPosition(animType, position, positionIncreasing, updateCache,
										forceNonSketchy, aggregateAnimating, finalFrame);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::updateDetailPosition(ENdhsAnimationType animType,
													LcTScalar position,
													bool positionIncreasing,
													bool updateCache,
													bool forceNonSketchy,
													bool aggregateAnimating,
													bool finalFrame)
{
	if (m_egDetailPage)
	{
		if (finalFrame)
		{
			if (m_unloadDetail)
			{
				unloadDetail();
				return;
			}
		}

		if (m_detailRequiresCacheUpdate)
		{
			updateCache = true;
			m_detailRequiresCacheUpdate = false;
		}

		m_egDetailPage->updatePosition(animType, position, positionIncreasing, updateCache,
											forceNonSketchy, aggregateAnimating, finalFrame);
		m_egDetailMenu->updatePosition(animType, position, positionIncreasing, updateCache,
											forceNonSketchy, aggregateAnimating, finalFrame);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CItem::resetTransitionCache()
{
	m_egItem->resetTransitionCache();

	if (m_egDetailPage)
	{
		m_egDetailPage->resetTransitionCache();
		m_egDetailMenu->resetTransitionCache();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::setSlot(int newSlot, int oldSlot)
{
	m_egItem->setSlot(oldSlot);
	m_egItem->setSlot(newSlot);

	if (m_egDetailPage)
	{
		m_egDetailPage->setSlot(oldSlot);
		m_egDetailPage->setSlot(newSlot);
		m_egDetailMenu->setSlot(oldSlot);
		m_egDetailMenu->setSlot(newSlot);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::checkDetails()
{
	// Check to see if detail group should be destroyed
	if (m_egDetailPage)
	{
		if (m_egDetailPage->getSlot() == m_parent->getActiveSlot() || m_egDetailPage->getOldSlot() == m_parent->getActiveSlot())
		{
			if (m_egDetailPage->getOldSlot() == m_parent->getActiveSlot())
				scheduleUnloadDetail();
		}
		else
		{
			// If start and end slot not active, get rid of detail immediately
			unloadDetail();
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaString NdhsCMenuComponent::CItem::getItemClassFromWidget(LcCWidget* widget)
{
	return m_egItem->getClassFromWidget(widget);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaString NdhsCMenuComponent::CItem::getDetailClassFromWidget(LcCWidget* widget)
{
	LcTaString elementClass;

	if (m_egDetailPage)
	{
		elementClass = m_egDetailPage->getClassFromWidget(widget);

		if (elementClass.isEmpty())
			elementClass = m_egDetailMenu->getClassFromWidget(widget);
	}

	return elementClass;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::scheduleForDeletion(bool schedule)
{
	if (m_egItem)
		m_egItem->scheduleForDeletion(schedule);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenuComponent::CItem::isScheduledForDeletion()
{
	bool scheduled = false;

	if (m_egItem)
		scheduled = m_egItem->isScheduledForDeletion();

	return scheduled;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::destroyUnwantedElements()
{
	// Just cleanup any unwanted detail elements
	if (m_egDetailPage && m_egDetailPage->isScheduledForDeletion())
	{
		// Destroy the Detail
		m_egDetailPage->retire();
		m_egDetailPage.destroy();
		m_egDetailMenu->retire();
		m_egDetailMenu.destroy();
	}
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenuComponent::CItem::populateElementList(	NdhsCPageManager::TmAWidgets& widgets,
															NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList)
{
	if (m_egDetailPage)
	{
		m_egDetailPage->populateElementList(widgets, pageWidgetElemList);
		m_egDetailMenu->populateElementList(widgets, pageWidgetElemList);
	}

	if (m_egItem)
		m_egItem->populateElementList(widgets, pageWidgetElemList);

	return true;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CItem::startStaticAnimations()
{
	if (m_egItem && m_egItem->getSlot() != -1)
		m_egItem->startStaticAnimations();

	if (m_egDetailPage)
	{
		m_egDetailPage->startStaticAnimations();
		m_egDetailMenu->startStaticAnimations();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CItem::stopStaticAnimations()
{
	if (m_egItem)
		m_egItem->stopStaticAnimations();

	if (m_egDetailPage)
	{
		m_egDetailPage->stopStaticAnimations();
		m_egDetailMenu->stopStaticAnimations();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuComponent> NdhsCMenuComponent::create(	NdhsCPageManager*					pageManager,
															NdhsCTemplate::CComponentElement*	menuComp,
															NdhsCElementGroup*					parent,
															int									stackLevel,
															NdhsCMenu*							menu,
															NdhsCMenuItem*						menuItem,
															NdhsCElementGroup*					parentGroup)
{
	LcTaOwner<NdhsCMenuComponent> ref;
	ref.set(new NdhsCMenuComponent(pageManager, menuComp, parent, stackLevel, menu, menuItem, parentGroup));
	//ref->construct();
	return ref;
}

#ifdef IFX_SERIALIZATION
NdhsCField::IObserver* NdhsCMenuComponent::loadMetaFieldState(SerializeHandle handle,
															  LcCSerializeMaster *serializeMaster,
															  NdhsCField *&field)
{
	NdhsCMenuComponent::CMetaField *metaField=(NdhsCMenuComponent::CMetaField*)serializeMaster->getPointer(handle);
	if(metaField==NULL)
	{
		metaField=NdhsCMenuComponent::CMetaField::loadState(handle,serializeMaster);
	}
	field=metaField;
	return metaField;
}

NdhsCMenuComponent* NdhsCMenuComponent::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCMenuComponent * comp=new NdhsCMenuComponent();
	comp->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,comp);
	return comp;
}
SerializeHandle	NdhsCMenuComponent::serialize(LcCSerializeMaster *serializeMaster,bool force)
{
	SerializeHandle handle=-1;
	if(!force)
	{
		handle=serializeMaster->getHandle(this);
		if(handle!=-1 && serializeMaster->isSerialized(handle))
		{
			return handle;
		}
		else if(handle==-1)
		{
			handle=serializeMaster->newHandle(this);
		}
	}
	else
	{
		handle=serializeMaster->newHandle(this);
	}
	int outputSize = sizeof(NdhsCMenuComponent) - sizeof(NdhsCComponent)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCComponent::serialize(serializeMaster,true);
	ENdhsCElementGroupType dataType=ENdhsCElementGroupTypeMenuComponent;
	SERIALIZE(dataType,serializeMaster,cPtr)
	SERIALIZE(parentHandle,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_menu,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_menuTemplate,serializeMaster,cPtr)
	SERIALIZE(m_scrollPositive,serializeMaster,cPtr)
	SERIALIZE(m_ignoreScrollPosUpdate,serializeMaster,cPtr)
	SERIALIZE(m_ignoreScrollPosNextItem,serializeMaster,cPtr)
	SERIALIZE_Owner(m_scrollPos,serializeMaster,cPtr)
	SERIALIZE_Owner(m_lastActiveItemScrollPos,serializeMaster,cPtr)
	SERIALIZE(m_menuLoaded,serializeMaster,cPtr)
	SERIALIZE(m_bRequiresPostTransitionComplete,serializeMaster,cPtr)
	SERIALIZE_Array_INT(m_slotRefs,serializeMaster,cPtr)
	SERIALIZE_Array_INT(m_oldSlotRefs,serializeMaster,cPtr)
	SERIALIZE_Array_INT(m_cachedSlotRefs,serializeMaster,cPtr)
	SERIALIZE(m_activeSlot,serializeMaster,cPtr)
	SERIALIZE(m_prevActiveSlot,serializeMaster,cPtr)
	SERIALIZE(m_pendingHideStateSlotActive,serializeMaster,cPtr)
	SERIALIZE(m_lastActiveSlot,serializeMaster,cPtr)
	SERIALIZE_String(m_dataSourceString,serializeMaster,cPtr)
	SERIALIZE_Owner(m_dataSourceExp,serializeMaster,cPtr)
	SERIALIZE(m_slotCount,serializeMaster,cPtr)
	SERIALIZE(m_firstActiveSlot,serializeMaster,cPtr)
	SERIALIZE(m_firstSelectableSlot,serializeMaster,cPtr)
	SERIALIZE(m_lastSelectableSlot,serializeMaster,cPtr)
	SERIALIZE(m_firstPopulateSlot,serializeMaster,cPtr)
	SERIALIZE(m_fullMode,serializeMaster,cPtr)
	SERIALIZE(m_circularMode,serializeMaster,cPtr)
	SERIALIZE(m_scrollWrap,serializeMaster,cPtr)
	SERIALIZE(m_scrollSpan,serializeMaster,cPtr)
	SERIALIZE(m_chainedAction,serializeMaster,cPtr)
	SERIALIZE(m_chainedActionSlot,serializeMaster,cPtr)
	SERIALIZE(m_stackLevel,serializeMaster,cPtr)
	SERIALIZE(m_lastReportedActiveItem,serializeMaster,cPtr)
#ifdef IFX_USE_PLUGIN_ELEMENTS
	SERIALIZE(m_tabOrderIndex,serializeMaster,cPtr)
	SERIALIZE(m_currentFocusClassData,serializeMaster,cPtr)
	SERIALIZE_Ptr(m_currentFocusElement,serializeMaster,cPtr)
	SERIALIZE_Ptr(m_previousFocusElement,serializeMaster,cPtr)
#endif
	SERIALIZE_String(m_previouslyFocusedClass,serializeMaster,cPtr)
	SERIALIZE(m_refreshNeeded,serializeMaster,cPtr)
	SERIALIZE_MapString(m_metaFields,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::deflate()
{
	m_itemCount=0;

	if(m_menu)
	{
		// If this fails, we will have an empty menu,
		// but there isn't much we can do about it
		m_menu->reloadMenu();
		m_menu->setLODRequired(m_slotCount);

		// set _itemCount field on refresh
		NdhsCField* field = m_fieldCache->getField("_itemCount");
		if (field)
		{
			field->setValue((LcTScalar)m_menu->getItemCount(), true);
		}

		reloadComponentForMenu();
		m_outerGroup->removeGroup("menu");

		// Create the menu aggregate
		LcTaOwner<NdhsCElementGroup> menuElementGroupPH = NdhsCElementGroup::create("menu_placeholder", this, m_menu, NULL, NDHS_LOWER_INVALID_DRAW_LAYER_INDEX);
		LcTaOwner<NdhsCElementGroup> menuElementGroup = m_menuTemplate->createElementGroup(	this,
																							m_menu,
																							getMenuItem(),
																							"menu",
																							m_stackLevel,
																							getDrawLayerIndex(),
																							this);
		menuElementGroup->setVisible(true);
		menuElementGroup->addGroup("_menu_placeholder", menuElementGroupPH);
		menuElementGroup->setElementGroup(m_outerGroup.ptr());
		menuElementGroup->realize(m_outerGroup.ptr());
		m_outerGroup->addGroup("menu", menuElementGroup);

		int firstActiveItem=-1;
		if(m_scrollPos)
			firstActiveItem = (int)m_scrollPos->getWrappedVal();

		reloadItems(firstActiveItem,m_activeSlot);
	}
}

void NdhsCMenuComponent::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummy=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;
	DESERIALIZE(dummy,serializeMaster,cPtr);
	DESERIALIZE(parentHandle,serializeMaster,cPtr);
	NdhsCComponent::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE_Reserve(m_menu,serializeMaster,cPtr,NdhsCMenu)
	m_pageManager->incMenuRefCount(m_menu, this);
	DESERIALIZE_Reserve(m_menuTemplate,serializeMaster,cPtr,NdhsCMenuComponentTemplate)
	DESERIALIZE(m_scrollPositive,serializeMaster,cPtr)
	DESERIALIZE(m_ignoreScrollPosUpdate,serializeMaster,cPtr)
	DESERIALIZE(m_ignoreScrollPosNextItem,serializeMaster,cPtr)
	DESERIALIZE_Owner(m_scrollPos,serializeMaster,cPtr,NdhsCScrollPosField)
	DESERIALIZE_Owner(m_lastActiveItemScrollPos,serializeMaster,cPtr,NdhsCScrollPosField)
	DESERIALIZE(m_menuLoaded,serializeMaster,cPtr)
	DESERIALIZE(m_bRequiresPostTransitionComplete,serializeMaster,cPtr)
	m_itemCount=0;
	m_scrollUpdateType=ENdhsScrollUpdateNormal;
	m_windowStart=0;
	m_windowSize=0;
	DESERIALIZE_Array_INT(m_slotRefs,serializeMaster,cPtr)
	DESERIALIZE_Array_INT(m_oldSlotRefs,serializeMaster,cPtr)
	DESERIALIZE_Array_INT(m_cachedSlotRefs,serializeMaster,cPtr)
	DESERIALIZE(m_activeSlot,serializeMaster,cPtr)
	DESERIALIZE(m_prevActiveSlot,serializeMaster,cPtr)
	DESERIALIZE(m_pendingHideStateSlotActive,serializeMaster,cPtr)
	DESERIALIZE(m_lastActiveSlot,serializeMaster,cPtr)
	DESERIALIZE_String(m_dataSourceString,serializeMaster,cPtr)
	DESERIALIZE_Owner(m_dataSourceExp,serializeMaster,cPtr,NdhsCExpression)
	DESERIALIZE(m_slotCount,serializeMaster,cPtr)
	m_prevSlotItems.resize(m_slotCount, NULL);
	DESERIALIZE(m_firstActiveSlot,serializeMaster,cPtr)
	DESERIALIZE(m_firstSelectableSlot,serializeMaster,cPtr)
	DESERIALIZE(m_lastSelectableSlot,serializeMaster,cPtr)
	DESERIALIZE(m_firstPopulateSlot,serializeMaster,cPtr)
	DESERIALIZE(m_fullMode,serializeMaster,cPtr)
	DESERIALIZE(m_circularMode,serializeMaster,cPtr)
	DESERIALIZE(m_scrollWrap,serializeMaster,cPtr)
	DESERIALIZE(m_scrollSpan,serializeMaster,cPtr)
	DESERIALIZE(m_chainedAction,serializeMaster,cPtr)
	DESERIALIZE(m_chainedActionSlot,serializeMaster,cPtr)
	DESERIALIZE(m_stackLevel,serializeMaster,cPtr)
	DESERIALIZE(m_lastReportedActiveItem,serializeMaster,cPtr)
#ifdef IFX_USE_PLUGIN_ELEMENTS
	DESERIALIZE(m_tabOrderIndex,serializeMaster,cPtr)
	DESERIALIZE(m_currentFocusClassData,serializeMaster,cPtr)
	DESERIALIZE_Ptr(m_currentFocusElement,serializeMaster,cPtr,NdhsCElement)
	DESERIALIZE_Ptr(m_previousFocusElement,serializeMaster,cPtr,NdhsCElement)
#endif
	DESERIALIZE_String(m_previouslyFocusedClass,serializeMaster,cPtr)
	DESERIALIZE(m_refreshNeeded,serializeMaster,cPtr)
	DESERIALIZE_MapString(m_metaFields,serializeMaster,cPtr)
	serializeMaster->requiresDeflate(this);
}
#endif /* IFX_SERIALIZATION */

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::reloadElement()
{
	prepareComponent();
	prepareComponentForMenu();
	reloadFurniture();
	reloadItems();
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuComponent::NdhsCMenuComponent(	NdhsCPageManager*					pageManager,
										NdhsCTemplate::CComponentElement*	menuComp,
										NdhsCElementGroup*					parent,
										int									stackLevel,
										NdhsCMenu*							menu,
										NdhsCMenuItem*						menuItem,
										NdhsCElementGroup*					parentGroup)
: NdhsCComponent(pageManager, menuComp, parent, stackLevel, menu, menuItem, parentGroup),
  m_replacePlaceholdersMessage(this,EReplacePlaceholdersMsg),
  m_reloadDataSourceMessage(this,EReloadDataSource)
{
	LC_ASSERT(pageManager != NULL);
	LC_ASSERT(menuComp != NULL);
	LC_ASSERT(menuComp->componentFile != NULL);

	// Set the menu component file
	m_menuTemplate = (NdhsCMenuComponentTemplate*)menuComp->componentFile;
	m_template = m_menuTemplate;
	m_isCreated  = false;
	m_isEnabled = false;
	m_groupUnloaded = true;

	m_stackLevel			= stackLevel;
#ifdef IFX_USE_PLUGIN_ELEMENTS
	m_tabOrderIndex			= -1;
	m_currentFocusElement	= NULL;
	m_previousFocusElement	= NULL;
#endif

	m_activeSlot     = -1;
	m_prevActiveSlot = -1;
	m_lastActiveSlot = -1;
	m_pendingHideStateSlotActive = -1;

	m_itemCount = 0;

	m_refreshNeeded = false;
	m_staticTransitionDone = false;
	m_jumpingToEnd = false;
	m_lastReportedActiveItem = -2;
	m_ignoreScrollPosNextItem=false;
	m_scrollUpdateType=ENdhsScrollUpdateNormal;
	m_trySetFocus = false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::doPostConstruct()
{

	if (m_menuTemplate->getDataSource())
	{
		// If the dataSource expression is valid, we create and evaluate the expression.
		// If not, we assume that it's just a raw string
		if (m_menuTemplate->getDataSource()->isBad())
		{
			m_dataSourceString = m_menuTemplate->getDataSource()->toString();
		}
		else
		{
			LcTaOwner<NdhsCExpression> exp = m_menuTemplate->getDataSource()->createExpression(this, getSlot(), getMenuItem());

			if (exp)
			{
				m_dataSourceExp = exp;
				m_dataSourceExp->setObserver(this);
			}
		}
	}

	// Load menu component
	loadMenuComponent();

}

/*-------------------------------------------------------------------------*//**
																			 */
void NdhsCMenuComponent::createIntrinsicFields()
{
	// Create component/menu component intirnsic fields,
	// need to create before transition manager, so placed here
	if (m_fieldCache)
	{
		m_fieldCache->createComponentIntrinsicFields();
		m_fieldCache->createMenuComponentIntrinsicFields();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::loadMenuComponent()
{
	// Set slot count first so that it can be used for LOD
	if (m_menuTemplate)
		m_slotCount           = m_menuTemplate->getSlotCount();

	// If DS is valid prepare component, other wise
	// wait for second turn
	if (m_isCreated == false)
	{
		// try to load data source and open menu
		loadDataSource();

		// In  case more add-ons manisfest files have been loaded reset
		m_stackLevel = m_pageManager->getManifestStack()->getStackHeight() - 1;
		reloadElement();
		m_isCreated = true;

		if (m_menu)
		{
			NdhsCField* field = m_fieldCache->getField("_dataSourceLoaded");
			if (field)
			{
				field->setValueBool(true, true);
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::prepareComponent()
{
	NdhsCComponent::prepareComponent();

	// See if template present
	if (m_menuTemplate)
	{
		m_slotCount           = m_menuTemplate->getSlotCount();
		m_firstActiveSlot     = m_menuTemplate->getFirstActiveSlot();
		m_firstSelectableSlot = m_menuTemplate->getFirstSelectableSlot();
		m_lastSelectableSlot  = m_menuTemplate->getLastSelectableSlot();
		m_firstPopulateSlot   = m_menuTemplate->getFirstPopulateSlot();
		m_fullMode		 	  = m_menuTemplate->fullMode();
		m_scrollWrap		  = m_menuTemplate->scrollWrap();
		m_scrollSpan          = m_menuTemplate->scrollSpan();

		NdhsCField* field = m_fieldCache->getField("_activeSlot");
		if (field)
		{
			if (m_firstActiveSlot == -1)
			{
				field->setValue((LcTScalar)m_firstActiveSlot, true);
			}
			else
			{
				field->setValue((LcTScalar)m_firstActiveSlot + 1, true);
			}
		}

		field = m_fieldCache->getField("_transitioning");
		if (field)
		{
			field->setValueBool(false, true);
		}

		field = m_fieldCache->getField("_firstSelectableSlot");
		if (field)
		{
			field->setValue((LcTScalar)m_firstSelectableSlot + 1, true);
		}

		field = m_fieldCache->getField("_lastSelectableSlot");
		if (field)
		{
			field->setValue((LcTScalar)m_lastSelectableSlot + 1, true);
		}

		field = m_fieldCache->getField("_span");
		if (field)
		{
			field->setValue((LcTScalar)m_scrollSpan, true);
		}

		field = m_fieldCache->getField("_full");
		if (field)
		{
			field->setValueBool(m_fullMode, true);
		}

		m_slotRefs.resize(m_slotCount, -1);
		m_oldSlotRefs.resize(m_slotCount, -1);
		m_cachedSlotRefs.resize(m_slotCount, -1);
		m_prevSlotItems.resize(m_slotCount, NULL);
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuComponent::~NdhsCMenuComponent()
{
	doCleanup();
}

/*-------------------------------------------------------------------------*//**
	Creating it such a way so that it can be used in component
	re-loading/unloading i.e. destroying aggregates explicitely
*/
void NdhsCMenuComponent::doCleanup()
{
	retire();

	deleteItems();
	m_slotRefs.clear();
	m_oldSlotRefs.clear();
	m_cachedSlotRefs.clear();
	m_prevSlotItems.clear();

#ifdef IFX_USE_STYLUS
	if (m_pageManager)
	{
		m_pageManager->ignoreEntry(this);
	}
#endif

	if (m_menu != NULL)
		m_pageManager->releaseMenu(	m_menu, this );

	// Reset menu pointer
	m_menu = NULL;

	if(m_pStaticAnimField)
	{
		m_pStaticAnimField->removeObserver(this);
	}

	if (m_scrollPos)
	{
		m_scrollPos->removeObserver(this);
		m_scrollPos.destroy();
	}

	if (m_layoutPos)
	{
		m_layoutPos.destroy();
	}

	if (m_postTransitionCompleteMessage.isScheduled())
	{
		m_postTransitionCompleteMessage.cancel();
	}

	if(m_replacePlaceholdersMessage.isScheduled())
	{
		m_replacePlaceholdersMessage.cancel();
	}

	if(m_reloadDataSourceMessage.isScheduled())
	{
		m_reloadDataSourceMessage.cancel();
	}

	if (m_fieldCache)
	{
		m_fieldCache.destroy();
	}

	if (m_transitionAgent)
	{
		m_transitionAgent.destroy();
	}

	if (m_stateManager)
	{
		m_stateManager.destroy();
	}

	if (m_localScreenAggregate)
	{
		m_localScreenAggregate->retire();
		m_localScreenAggregate.destroy();
	}

	if (m_outerGroup)
	{
		m_outerGroup->retire();
		m_outerGroup.destroy();
	}

	m_metaFields.clear();

#ifdef IFX_USE_STYLUS
	m_mouseFocusElement = NULL;
#endif
	m_focusedElement = NULL;
	m_focusedChildComponent = NULL;
	m_inFocus = false;
	m_focusEnabledField = NULL;
	m_parentFocusEnabledField = NULL;

	m_activeSlot     = -1;
	m_prevActiveSlot = -1;
	m_lastActiveSlot = -1;
	m_pendingHideStateSlotActive = -1;

	m_itemCount = 0;

	m_refreshNeeded = false;
	m_staticTransitionDone = false;
	m_jumpingToEnd = false;
	m_lastReportedActiveItem = -2;
	m_ignoreScrollPosNextItem=false;
	m_scrollUpdateType=ENdhsScrollUpdateNormal;
	m_trySetFocus = false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::loadDataSource()
{
	volatile bool menuLoaded = false;
	LcTaString menuUri = m_dataSourceString;

	NdhsCMenu* menu = NULL;
	NdhsCManifest* palManifest = NULL;

	m_menu = NULL;

	if (m_template)
	{
		palManifest = m_template->getPaletteManifest();
	}

	// If the dataSource expression is valid, we create and evaluate the expression.
	// If not, we assume that it's just a raw string
	if (menuUri.isEmpty())
	{
		if (m_dataSourceExp)
		{
			m_dataSourceExp->evaluate(this, getSlot(), getMenuItem());
			menuUri = m_dataSourceExp->getValueString();
		}
	}

	// load menu
	menu = m_pageManager->getMenu(menuUri, menuLoaded, palManifest, m_stackLevel, this);

	// First flag verify that menu populated correctly, second verify menu created properly
	if (menu != NULL && menuLoaded)
	{
		menu->setLODRequired(m_slotCount);

		// Save for internal use
		m_menu = menu;

		NdhsCField* field = m_fieldCache->getField("_itemCount");
		if (field)
		{
			field->setValue((LcTScalar)m_menu->getItemCount(), true);
		}
	}

	return menuLoaded;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::destroyUnwantedElements()
{
	// If for some reason we dont have menu
	// no need to go further
	if (m_menu == NULL || !m_outerGroup)
		return;

	// If item is out of the window range then delete complete item otherwise
	// just detail may be deleted

	TmOAPlaceHolderField::iterator it2 = m_placeHolderFields.begin();
	TmAItems::iterator it = m_items.begin();
	for (; it != m_items.end(); it++, it2++)
	{
		NdhsCMenuComponent::CItem* item = *it;
		if (item)
		{
			if (item->isScheduledForDeletion())
			{
				m_menu->releaseMenuItem(item->getMenuItem()->getIndex());
				delete (*it);
				*it = NULL;

				delete (*it2);
				*it2 = NULL;
			}
			else
			{
				item->destroyUnwantedElements();
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::replacePlaceholders()
{
	if (m_placeholderUsed == false || !m_outerGroup)
		return;

	m_placeholderUsed = false;

	bool setPlaceHolderField = m_menu && m_menu->isLodRequired();

	if ((m_itemCount > 0) && setPlaceHolderField)
	{
		int offset	= wrapToRange(m_windowStart, m_itemCount);
		int amount	= m_windowSize;
		NdhsCField* placeHolderField = NULL;

		for (; amount > 0; amount--)
		{
			placeHolderField = m_placeHolderFields[offset];
			if (placeHolderField)
				placeHolderField->setValueBool(false, true);

			offset++;
			offset = wrapToRange(offset, m_itemCount);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::resetTransitionCache()
{
	if(!m_outerGroup)
		 return;

	m_outerGroup->resetTransitionCache();

	// items and details
	if (m_itemCount > 0)
	{
		CItem* item;
		int offset = wrapToRange(m_windowStart, m_itemCount);

		for (int i = 0; i < m_windowSize; i++)
		{
			item = m_items[offset];
			if(item)
				item->resetTransitionCache();

				offset++;
				offset = wrapToRange(offset, m_itemCount);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::deleteItems()
{
	m_items.clear();

	m_placeHolderFields.clear();

	m_prevSlotItems.clear();
	m_prevSlotItems.resize(m_slotCount, NULL);

	m_placeholderUsed = false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::prepareComponentForMenu()
{
	// If for some reasond we dont have menu
	// no need to go further
	if (m_menu == NULL)
		return;

	m_itemCount = m_menu->getItemCount();
	m_firstPopulateSlot = m_menuTemplate->getFirstPopulateSlot();

	if (m_scrollPos)
	{
		m_scrollPos->removeObserver(this);
		m_scrollPos.destroy();
	}

	// Create the scroll pos field
	LcTaOwner<NdhsCScrollPosField> tempPos;

	// if full mode and wrapping is specified, but the number of items is less
	// than the number of selectable slots + 'span', we replicate the menu items
	// until we have sufficient items to allow for wrapping to work
	if (m_itemCount > 0)
	{
		if(m_fullMode)
		{
			if (m_scrollWrap && (m_scrollSpan == 1) && (getSelectableSlotCount() == m_slotCount) && (m_itemCount == m_slotCount))
			{
				// Special case...in this case, enable circular mode
				m_circularMode = true;
			}
			else if (m_scrollWrap && (m_itemCount < getSlotCount() || m_itemCount % m_scrollSpan != 0))
			{
				int factor = 2;
				int slotCount = getSlotCount();

				while ((m_itemCount * factor < slotCount || (m_itemCount * factor) % m_scrollSpan != 0))
				{
					factor++;
				}

				m_menu->replicateMenuItems(factor);
				m_itemCount = m_menu->getItemCount();
			}
		}
	}

	tempPos = NdhsCScrollPosField::create(m_pageManager->getCon(),
								NULL,
								0,
								(LcTScalar)(m_itemCount - 1),
								m_scrollWrap && isWrappable(),
								m_menuTemplate->scrollRebound(),
								m_scrollSpan);
	m_scrollPos = tempPos;
	m_scrollPos->setIgnoreLaundry(true);

	// create another, to store the last active item position information
	tempPos = NdhsCScrollPosField::create(m_pageManager->getCon(),
								NULL,
								0,
								(LcTScalar)(m_itemCount - 1),
								m_scrollWrap && isWrappable(),
								m_menuTemplate->scrollRebound(),
								m_scrollSpan);
	m_lastActiveItemScrollPos = tempPos;
	m_lastActiveItemScrollPos->setIgnoreLaundry(true);

	// We must observe the scrollPos field, so that drag regions can modify it
	m_scrollPos->addObserver(this);

	NdhsCField* field = m_fieldCache->getField("_wrap");
	if (field)
	{
		field->setValueBool(m_scrollWrap && isWrappable(), true);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::reloadComponentForMenu()
{
	// If for some reason we dont have menu
	// no need to go further
	if (!m_outerGroup || !m_menu)
		return;

	m_itemCount = m_menu->getItemCount();
	m_firstPopulateSlot = m_menuTemplate->getFirstPopulateSlot();

	if (m_itemCount <= 0)
		return;

	// Configure Scroll pos field
	LcTaOwner<NdhsCScrollPosField> tempPos;

	// if full mode and wrapping is specified, but the number of items is less
	// than the number of selectable slots + 'span', we replicate the menu items
	// until we have sufficient items to allow for wrapping to work
	if (m_fullMode)
	{
		if (m_scrollWrap && (m_scrollSpan == 1) && (getSelectableSlotCount() == m_slotCount) && (m_itemCount == m_slotCount))
		{
			// Special case...in this case, enable circular mode
			m_circularMode = true;
		}
		else if (m_scrollWrap && (m_itemCount < getSlotCount() || m_itemCount % m_scrollSpan != 0))
		{
			int factor = 2;
			int slotCount = getSlotCount();

			while ((m_itemCount * factor < slotCount || (m_itemCount * factor) % m_scrollSpan != 0))
			{
				factor++;
			}

			m_menu->replicateMenuItems(factor);
			m_itemCount = m_menu->getItemCount();
		}
	}

	if (!m_scrollPos)
	{
		LcTaOwner<NdhsCScrollPosField> tempPos = NdhsCScrollPosField::create(m_pageManager->getCon(),
							NULL,
							0,
							(LcTScalar)(m_itemCount - 1),
							m_scrollWrap && isWrappable(),
							m_menuTemplate->scrollRebound(),
							m_scrollSpan);

		m_scrollPos = tempPos;
		m_scrollPos->setIgnoreLaundry(true);

		// We must observe the scrollPos field, so that drag regions can modify it
		m_scrollPos->addObserver(this);

		// create another, to store the last active item position information
		tempPos = NdhsCScrollPosField::create(m_pageManager->getCon(),
									NULL,
									0,
									(LcTScalar)(m_itemCount - 1),
									m_scrollWrap && isWrappable(),
									m_menuTemplate->scrollRebound(),
									m_scrollSpan);
		m_lastActiveItemScrollPos = tempPos;
		m_lastActiveItemScrollPos->setIgnoreLaundry(true);
	}
	else
	{
		m_scrollPos->updateBounds(0,
			(LcTScalar)(m_itemCount - 1),
			m_scrollWrap && isWrappable(),
			m_menuTemplate->scrollRebound(),
			m_scrollSpan);

		m_lastActiveItemScrollPos->updateBounds(0,
			(LcTScalar)(m_itemCount - 1),
			m_scrollWrap && isWrappable(),
			m_menuTemplate->scrollRebound(),
			m_scrollSpan);
	}

	m_scrollPos->setMaxAllowableVal((LcTScalar)(m_itemCount - 1));
	m_scrollPos->setMinAllowableVal(0);

	m_lastActiveItemScrollPos->setMaxAllowableVal((LcTScalar)(m_itemCount - 1));
	m_lastActiveItemScrollPos->setMinAllowableVal(0);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::reloadItems(int firstActiveItem,int restoreActiveSlot)
{

	// If for some reason we dont have menu
	// no need to go further
	if (!m_outerGroup || !m_menu)
		return;

	// Clear any existing items first
	deleteItems();

	// If no menu items to load, no point continuing
	if (m_itemCount <= 0)
		return;

	m_items.resize(m_itemCount, NULL);

	m_placeHolderFields.resize(m_itemCount, NULL);

	if (m_menu->isLodRequired())
	{
		// We'll set this in assignItemsToSlots
		m_windowStart	= -1;
		m_windowSize	= m_slotCount * LOD_THRESHOLD;
	}
	else
	{
		// Load all
		m_windowStart	= 0;
		m_windowSize	= m_itemCount;
	}

	// Place some of the items in the slots
	if ((firstActiveItem < 0) || (firstActiveItem > m_itemCount))
		firstActiveItem = m_menu->getFirstActiveItem();

	assignItemsToSlots(firstActiveItem, restoreActiveSlot);

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::loadItems(int start, int amount, bool usePlaceholder,bool doingLOD)
{
	CItem* item = NULL;
	NdhsCField *field = NULL;

	// If for some reason we dont have menu
	// no need to go further
	if (!m_outerGroup || !m_menu || m_itemCount <= 0)
		return;

	int offset = wrapToRange(start, m_itemCount);

	// Iterate through the actual menu items
	for (; amount > 0; amount--)
	{
		if (m_items[offset] == NULL)
		{
			// Create a new item
			LcTaOwner<CItem> itemRef = CItem::create(this, m_menu->getMenuItem(offset));
			item = itemRef.ptr();
			m_items[offset] = itemRef.release();

			LcTaOwner<NdhsCField> placeHolderField = NdhsCField::create(getPageManager()->getCon(), "_placeholderActive", false, NULL, -1, NULL,
															IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_BOOL);

			field = placeHolderField.ptr();
			field->setValueBool(false, true);
			m_placeHolderFields[offset] = placeHolderField.release();

			// acquire item
			m_menu->acquireMenuItem(offset);
		}
		else
		{
			item = m_items[offset];
			field = m_placeHolderFields[offset];
		}

		// if item is scheduled for deletion just unset the deletion mark
		if (item->isScheduledForDeletion())
		{
			field->setValueBool(false, true);
			item->scheduleForDeletion(false);
		}
		else
		{
			if (usePlaceholder)
			{
				field->setValueBool(true, true);
			}

			item->loadElementGroups(doingLOD);
			item->setSlot(-1, -1);
			if (!m_groupUnloaded && doingLOD)
			{
				item->loadGroup();
			}
		}

		offset++;
		offset = wrapToRange(offset, m_itemCount);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::unloadItems(int start, int amount)
{
	if (m_itemCount <= 0)
		return;

	int offset = wrapToRange(start, m_itemCount);

	for (; amount > 0; amount--)
	{
		if (m_items[offset])
		{
			m_items[offset]->scheduleForDeletion(true);
		}

		offset++;
		offset = wrapToRange(offset, m_itemCount);
	}
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCMenuComponent::getActiveItem(ENdhsActiveItem timepoint, bool clampValue)
{
	// The active item is the first active item, plus the scroll pos value wrapped
	// to the item count range when we're not scrolling.  If we're scrolling, then
	// we need to see which direction we're scrolling to add or subtract 'scroll span'
	// before wrapping to range as we're animating to the new active item.

	if (!m_scrollPos || !m_outerGroup)
		return -1;

	if (m_itemCount <= 0)
		return 0;

	// Get relevant scrollpos value
	LcTDScalar scrollposVal = 0;
	switch (timepoint)
	{
		case ENdhsActiveItemOld:
			scrollposVal = m_lastActiveItemScrollPos->getWrappedVal();
			break;
		case ENdhsActiveItemCurrent:
			scrollposVal = m_scrollPos->getWrappedVal();
			break;
		case ENdhsActiveItemTarget:
			scrollposVal = m_scrollPos->getTargetValue();
			break;
	}

	bool isAnimating = m_scrollPos->isAnimating();

	int activeItem = (int)scrollposVal;

	if (scrollposVal < 0)
		activeItem--;

	// For 'old' and 'target', job is done so return straight away
	if (timepoint != ENdhsActiveItemCurrent)
		return activeItem;

	// If we're not drag scrolling and the animation has finished, we don't need to go
	// further either
	if ((isAnimating == false) && (m_animType != ENdhsAnimationTypeDrag || !m_scrollPos->isUserLocked()))
		return activeItem;

	// For 'current', we may need to adjust the active item based on
	// whether we're animating or not.
	LcTDScalar currPosUnwrapped = m_scrollPos->getWrapAdjustedVal();
	LcTDScalar lastActiveItemPosUnwrapped = m_lastActiveItemScrollPos->getWrapAdjustedVal();

	// In the grid menu case, the current scrollpos value may indicate that
	// any item within m_scrollSpan of the last active value is 'active',
	// but this isn't true - only items in the same 'column' as the last
	// active item are valid (i.e. activeItem - lastActiveItem must be
	// an exact multiple of m_scrollSpan.  The following code ensures
	// that we don't mis-represent the active item in this case, else we
	// see all sorts of nasty glitching.
	if (m_scrollSpan > 1)
	{
		int lastActiveItemUnwrapped = (int)lastActiveItemPosUnwrapped;
		int currActiveItemUnwrapped = (int)currPosUnwrapped;

		// Check for currPosUnwrapped being < 0 and non-integral (lastActiveItemPosUnwrapped
		// is always integral, of course)
		if ((LcTScalar)currActiveItemUnwrapped > currPosUnwrapped)
			currActiveItemUnwrapped--;

		// This will be 0 if correctly aligned
		int alignment = abs(lastActiveItemUnwrapped - currActiveItemUnwrapped)%m_scrollSpan;
		if (alignment != 0)
		{
			// re-align the active item towards the next safe item 'down'
			if ((lastActiveItemUnwrapped - currActiveItemUnwrapped) < 0)
				currActiveItemUnwrapped -= alignment;
			else
				currActiveItemUnwrapped -= (m_scrollSpan - alignment);

			activeItem = currActiveItemUnwrapped;

			if (m_scrollWrap && isWrappable())
			{
				activeItem = wrapToRange(activeItem, m_itemCount);
			}
		}
	}

	if ((m_animType == ENdhsAnimationTypeScroll) || (m_animType == ENdhsAnimationTypeDrag)
		|| (m_animType == ENdhsAnimationTypeScrollKick))
	{
		if (!m_ignoreScrollPosNextItem  && (m_scrollUpdateType == ENdhsScrollUpdateNormal))
		{
			// units of scaledScrollPosValue are points where active item changes
			LcTDScalar scaledScrollPosValue = (currPosUnwrapped - lastActiveItemPosUnwrapped) / m_scrollSpan;

			if(!isAnimating && m_animType != ENdhsAnimationTypeDrag)
			{
				if (m_scrollPositive)
				{
					// Going 'upwards' we
					// want the next item.
					if ((scaledScrollPosValue - (int)scaledScrollPosValue) != 0)
					{
						activeItem += m_scrollSpan;
					}
				}
			}
			else
			{
				if (m_scrollPositive)
				{
					// Going 'upwards' we
					// want the next item.
					activeItem += m_scrollSpan;
				}
				else if ((scaledScrollPosValue - (int)scaledScrollPosValue) == 0)
				{
					// Going 'downwards', on int boundary,
					// subtract full span.
					activeItem -= m_scrollSpan;
				}
			}
		}
	}

	// Note that we don't clamp the unwrapped item to the valid range,
	// so that 'rebounds' at the end of the range work properly
	if (m_scrollWrap && isWrappable())
	{
		// Wrap active item
		activeItem = wrapToRange(activeItem, m_itemCount);
	}
	else if (clampValue)
	{
		if (activeItem < 0)
			activeItem = 0;
		else if (activeItem > m_itemCount - 1)
			activeItem = m_itemCount - 1;
	}

	return activeItem;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::setActiveItem(int activeItem)
{
	if (!m_outerGroup || m_itemCount<=0)
		return;

	int previousActiveItem = getActiveItem(ENdhsActiveItemCurrent);

	// Adjust the scrollpos field to make the requested active item be the one returned
	// by getActiveItem.

	if ((m_animType == ENdhsAnimationTypeScroll) || (m_animType == ENdhsAnimationTypeDrag)
			|| (m_animType == ENdhsAnimationTypeScrollKick))
	{
		if (m_scrollPositive)
		{
			// Going 'upwards' - we
			// need to adjust.
			activeItem -= m_scrollSpan;
		}
		else
		{
			// Going 'downwards' - we
			// need to adjust.
			activeItem += m_scrollSpan;
		}
	}

	if (m_scrollWrap && isWrappable())
	{
		// Wrap active item
		activeItem = wrapToRange(activeItem, m_itemCount);
	}
	else
	{
		// Clamp to range
		if (activeItem > (m_itemCount - 1))
			activeItem = m_itemCount - 1;

		if (activeItem < 0)
			activeItem = 0;
	}

	m_ignoreScrollPosUpdate = true;
	m_scrollPos->jumpValueTo(activeItem);
	m_ignoreScrollPosUpdate = false;

	// Update our cached value
	m_lastActiveItemScrollPos->jumpValueTo(activeItem);
	// Update any metafields
	updateMetaFields();

	if ((m_activeSlot != -1) && (previousActiveItem != activeItem))
	{
		if (previousActiveItem >= 0)
		{
			if (m_items[previousActiveItem])
				m_items[previousActiveItem]->unloadDetail();
		}

		if (activeItem >= 0)
		{
			if (m_items[activeItem])
				m_items[activeItem]->loadDetail();
		}
	}

}

/*-------------------------------------------------------------------------*//**
	In the inactive state, we have some freedom in which slot to use to 
	start populating items from.
*/
int NdhsCMenuComponent::getInactivePopulateSlot()
{
	// Initially, if the user has specified a 'populate from' slot, use that.
	// Otherwise, use the first selectable slot
	int slot = (m_firstPopulateSlot == -1) ? m_firstSelectableSlot : m_firstPopulateSlot;

	// If the 'populate from' slot is less than the first selectable slot, shift the slot
	// by a suitable multiple of scrollspan
	if (slot < m_firstSelectableSlot)
		slot += m_scrollSpan * ((m_firstSelectableSlot - slot)/m_scrollSpan);

	// If the 'populate from' slot is greater than the last selectable slot, shift the slot
	// by a suitable multiple of scrollspan.  Note that we assume that the last selectable
	// slot will be at the end of a row/column of selectable slots, so m_lastSelectableSlot + 1
	// would be on a new row/column and should be shifted by one scrollspan - this is why we
	// add on m_scrollSpan - 1 in the below calculation.
	if (slot > m_lastSelectableSlot)
		slot -= m_scrollSpan * ((slot - m_lastSelectableSlot + m_scrollSpan - 1)/m_scrollSpan);

	// In full mode where we have too few items to fill the selectable slot range, we take the
	// decision to fill the slots from the last selectable slot going forward.
	// In the span > 1 case, we try and maintain the row/column the populate from slot was on when
	// we do this.
	if (m_fullMode && m_itemCount < getSelectableSlotCount())
		slot = m_lastSelectableSlot - (m_scrollSpan - 1 - slot%m_scrollSpan);

	return slot;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::setActiveSlot(int slot)
{
	bool setAI = false;

	bool inactiveLayout=false;

	if (!m_outerGroup || !m_fieldCache)
		return;

	if (m_activeSlot == slot)
		return;

	backupFocus();

	// Allow update slots to work properly on a deactivate
	if (slot != -1)
		m_firstPopulateSlot = slot;
	else
	{
		// Set the active slot to the 'top' selectable slot, and update the active item
		if ((m_firstPopulateSlot % m_scrollSpan) > 0)
		{
			m_firstPopulateSlot = m_firstPopulateSlot - (m_firstPopulateSlot % m_scrollSpan);
			setAI = true;
		}
	}

	m_activeSlot = slot;

	NdhsCField* field = m_fieldCache->getField("_activeSlot");
	if (field)
	{
		if (m_activeSlot == -1)
		{
			field->setValue(-1.0f, true);
		}
		else
		{
			field->setValue((LcTScalar)m_activeSlot + 1, true);
		}
	}

	// We need to constrain the scrollpos value to let drag scrolling work properly
	if ((m_itemCount > 0 ) && (!m_scrollWrap || isWrappable() == false))
	{
		if (slot == -1)
		{
			slot = getInactivePopulateSlot();
			inactiveLayout = true;
		}

		if(slot<m_firstSelectableSlot)
		{
			slot = m_firstSelectableSlot + (m_firstSelectableSlot%m_scrollSpan);
		}
		else if(slot>m_lastSelectableSlot)
		{
			slot = m_lastSelectableSlot - (m_firstSelectableSlot%m_scrollSpan);
		}

		int maxVal;
		int minVal;

		if (m_fullMode)
		{
			// We limit the max / min values accessible within the active item range for
			// non-wrapping full mode menus so that drags cannot leave a selectable slot empty
			// (unless span > 1, where we relax the constraint a little)
			if (m_scrollSpan == 1)
			{
				if (m_itemCount < getSelectableSlotCount())
				{
					// The case where we've not enough items to fill the selectable
					// slots needs special treatment
					if(inactiveLayout)
					{
						maxVal = slot - m_firstSelectableSlot;
						minVal = slot - m_lastSelectableSlot + m_itemCount-1;
					}
					else
					{
						maxVal = min(slot - m_firstSelectableSlot, m_itemCount - 1);
						minVal = max(slot - m_lastSelectableSlot + m_itemCount - 1, 0);
					}
				}
				else
				{
					maxVal = m_itemCount - 1 - (m_lastSelectableSlot - slot);
					minVal = slot - m_firstSelectableSlot;
				}
			}
			else
			{
				// Set the max / min values of the scroll pos to prevent the active
				// slot being left empty
				int activeItem = m_slotRefs[slot];
				if(activeItem==-1)
					activeItem=0;

				if (m_itemCount < getSelectableSlotCount())
				{
					// if we don't have enough items to fill the slots we have some special rules
					if (inactiveLayout)
					{
						maxVal = slot - m_firstSelectableSlot;
						minVal = slot - m_lastSelectableSlot + m_itemCount-1;
						maxVal = maxVal - (maxVal%m_scrollSpan);
						minVal = minVal - (minVal%m_scrollSpan);
					}
					else
					{
						minVal = activeItem - (m_scrollSpan * (activeItem / m_scrollSpan));
						maxVal = activeItem + (m_scrollSpan * ((m_itemCount - 1 - activeItem) / m_scrollSpan));
					}
				}
				else
				{
					int firstRowFirstItem = (activeItem - (slot % m_scrollSpan)) % m_scrollSpan;
					int lastRowFirstItem = m_itemCount - 1 - ((m_itemCount - 1 - firstRowFirstItem) % m_scrollSpan);

					if (firstRowFirstItem > 0)
						firstRowFirstItem -=  m_scrollSpan;

					// Constrain the minimum to keep the active slot full on a scroll
					if ((firstRowFirstItem + (slot % m_scrollSpan) < 0))
					{
						// Ok...be careful that we don't make the active slot empty
						if (slot - m_firstSelectableSlot < m_scrollSpan)
							minVal = firstRowFirstItem + m_scrollSpan + (slot - m_firstSelectableSlot -(slot -m_firstSelectableSlot) % m_scrollSpan);
						else
							minVal = firstRowFirstItem + (slot - m_firstSelectableSlot -(slot -m_firstSelectableSlot) % m_scrollSpan);
					}
					else
					{
						minVal = firstRowFirstItem + (slot - m_firstSelectableSlot -(slot -m_firstSelectableSlot) % m_scrollSpan);
					}
					minVal += (slot % m_scrollSpan);

					if (lastRowFirstItem + (slot % m_scrollSpan) > m_itemCount - 1)
					{
						// Ok...be careful that we don't make the active slot empty
						if (m_lastSelectableSlot - slot < m_scrollSpan)
							maxVal = lastRowFirstItem + (slot % m_scrollSpan) - m_scrollSpan - (m_lastSelectableSlot - slot - (m_scrollSpan - 1 - slot%m_scrollSpan));
						else
							maxVal = lastRowFirstItem + (slot % m_scrollSpan) - (m_lastSelectableSlot - slot - (m_scrollSpan - 1 - slot%m_scrollSpan));
					}
					else
					{
						maxVal = lastRowFirstItem + (slot % m_scrollSpan) - (m_lastSelectableSlot - slot - (m_scrollSpan - 1 - slot%m_scrollSpan));
					}
				}
			}
		}
		else
		{
			// Set the max / min values of the scroll pos to prevent the active
			// slot being left empty
			if (m_scrollSpan == 1)
			{
				maxVal = m_itemCount - 1;
				minVal = 0;
			}
			else
			{
				int activeItem = m_slotRefs[slot];

				int firstRowFirstItem = (activeItem - (slot % m_scrollSpan)) % m_scrollSpan;
				int lastRowFirstItem = m_itemCount - 1 - ((m_itemCount - 1 - firstRowFirstItem) % m_scrollSpan);

				if (firstRowFirstItem > 0)
					firstRowFirstItem -=  m_scrollSpan;

				if ((firstRowFirstItem + (slot % m_scrollSpan) < 0))
					minVal = firstRowFirstItem + (slot % m_scrollSpan) + m_scrollSpan;
				else
					minVal = firstRowFirstItem + (slot % m_scrollSpan);

				if (lastRowFirstItem + (slot % m_scrollSpan) > m_itemCount - 1)
					maxVal = lastRowFirstItem + (slot % m_scrollSpan) - m_scrollSpan;
				else
					maxVal = lastRowFirstItem + (slot % m_scrollSpan);
			}
		}

		if (m_scrollPos)
		{
			m_scrollPos->setMaxAllowableVal((LcTScalar)maxVal);
			m_scrollPos->setMinAllowableVal((LcTScalar)minVal);
			// Extend the max/min values of m_lastActiveItemScrollPos if necessary
			if ((LcTDScalar)maxVal > m_lastActiveItemScrollPos->getMaxAllowableVal())
				m_lastActiveItemScrollPos->setMaxAllowableVal((LcTScalar)maxVal);
			if ((LcTDScalar)minVal < m_lastActiveItemScrollPos->getMinAllowableVal())
				m_lastActiveItemScrollPos->setMinAllowableVal((LcTScalar)minVal);
		}
	}

	if(setAI)
		setActiveItem(m_slotRefs[m_firstPopulateSlot]);

	updateMetaFields();
	restoreFocus();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::reloadMenuAggregate(bool realize)
{
	//remove old group
	if (m_outerGroup)
	{
		m_outerGroup->removeGroup("menu");

		// Create the menu aggregate
		LcTaOwner<NdhsCElementGroup> menuElementGroupPH = NdhsCElementGroup::create("menu_placeholder", this, m_menu, NULL, NDHS_LOWER_INVALID_DRAW_LAYER_INDEX);

		LcTaOwner<NdhsCElementGroup> menuElementGroup = m_menuTemplate->createElementGroup(	this,
																							m_menu,
																							getMenuItem(),
																							"menu",
																							m_stackLevel,
																							getDrawLayerIndex(),
																							this);
		menuElementGroup->setVisible(true);
		menuElementGroup->addGroup("_menu_placeholder", menuElementGroupPH);

		menuElementGroup->setElementGroup(m_outerGroup.ptr());
		if(realize)
			menuElementGroup->realize(m_outerGroup.ptr());
		m_outerGroup->addGroup("menu", menuElementGroup);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::reloadFurniture()
{

	if (m_outerGroup)
	{
		m_outerGroup->retire();
		m_outerGroup.destroy();
	}

	if(m_localScreenAggregate)
	{
		m_localScreenAggregate->retire();
		m_localScreenAggregate.destroy();
	}

	m_localScreenAggregate = m_template->createElementGroup(this,
																						getMenu(),
																						getMenuItem(),
																						"screen",
																						m_stackLevel,
																						getDrawLayerIndex(),
																						this);
	// Create the furniture element group
	LcTaOwner<NdhsCElementGroup> furnitureElementGroup = m_menuTemplate->createElementGroup(this,
																							m_menu,
																							getMenuItem(),
																							"page",
																							m_stackLevel,
																							getDrawLayerIndex(),
																							this);
	m_furnitureElementGroup = furnitureElementGroup.ptr();
	furnitureElementGroup->setVisible(true);

	m_outerGroup = m_template->createElementGroup(this,
																						getMenu(),
																						getMenuItem(),
																						"outergroup",
																						m_stackLevel,
																						getDrawLayerIndex(),
																						this);
	m_outerGroup->setVisible(true);
	m_outerGroup->setElementGroup(this);
	furnitureElementGroup->setElementGroup(m_outerGroup.ptr());
	furnitureElementGroup->realize(m_outerGroup.ptr());
	m_outerGroup->addGroup("furniture", furnitureElementGroup);
	m_outerGroup->realize(this);

	reloadMenuAggregate(true);

}

/*-------------------------------------------------------------------------*//**
	Indicates whether wrapping while scrolling is possible, which depends
	on the numbers of available items and selectable slots.
*/
bool NdhsCMenuComponent::isWrappable()
{
	return (m_itemCount > getSelectableSlotCount()) || m_circularMode;
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCMenuComponent::wrapToRange(int val, int max)
{
	while (val < 0) val += max;
	val = val % max;

	return val;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::assignItemsToSlots(int firstActiveItem,int restoreActiveSlot)
{
	bool inactiveLayout = false;
	int inactiveLayoutActiveItem = -1;
	int s, r;
	bool wrap = m_scrollWrap && isWrappable();

	int firstActiveSlot = (restoreActiveSlot==-1)?m_firstActiveSlot:restoreActiveSlot;

	if (m_itemCount <= 0 && restoreActiveSlot==-1)
	{
		// If the menu is empty use any inactive layout
		m_activeSlot = -1;
	}
	else if(restoreActiveSlot==-1)
	{
		// Work out which item should be active initially
		if ((m_scrollSpan > 1) && (m_firstActiveSlot != -1) && (firstActiveItem != -1))
		{
			// Check that the requested active item is on the correct 'row',
			// otherwise change the active slot to suit
			if ((firstActiveItem % m_scrollSpan) != (m_firstActiveSlot % m_scrollSpan))
			{
				firstActiveSlot += (firstActiveItem % m_scrollSpan) - (m_firstActiveSlot % m_scrollSpan);

				// Make sure active slot is with in range
				if(firstActiveSlot < m_firstSelectableSlot)
				{
					firstActiveSlot = m_firstSelectableSlot + (m_firstSelectableSlot % m_scrollSpan);
				}
				else if(firstActiveSlot > m_lastSelectableSlot)
				{
					firstActiveSlot = m_lastSelectableSlot - (m_firstSelectableSlot % m_scrollSpan);
				}
			}
		}

		m_activeSlot = firstActiveSlot;
	}

	// If active slot is -1 (inactive layout), the slot
	// assignment wont work properly, so we need to trick
	// it by giving it an active slot while the items are
	// assigned, then resetting it to -1 after
	if (m_activeSlot == -1)
	{
		inactiveLayout = true;
		m_activeSlot = getInactivePopulateSlot();

		// adjust the 'first active item' to preserve the intention of the first populate slot
		if (m_firstPopulateSlot != -1 && m_firstPopulateSlot < m_firstSelectableSlot)
			firstActiveItem = min(m_firstSelectableSlot - m_firstPopulateSlot, m_itemCount - 1);

		// Full mode is a little special - lets make sure that
		// as many selectable slots as possible are filled
		if (m_fullMode && m_itemCount<getSelectableSlotCount())
			firstActiveItem = m_itemCount - 1;

		if (firstActiveItem != -1)
		{
			if (m_scrollSpan > 1)
			{
				// Make sure that the new active item doesn't skew the item-slot assignments
				firstActiveItem -= firstActiveItem%m_scrollSpan;
			}

			inactiveLayoutActiveItem = firstActiveItem;
		}

		// Make sure that at least one item is in the selectable slot range - 'full' mode
		// menus will adjust this further below
		if (!wrap && ((m_activeSlot + m_itemCount - 1) < m_firstSelectableSlot))
		{
			m_activeSlot = m_firstSelectableSlot - (m_itemCount - 1);
		}
		else if (!wrap && (m_activeSlot > m_lastSelectableSlot))
		{
			m_activeSlot = m_lastSelectableSlot;
		}

		if (firstActiveItem != -1)
		{
			// If we have a requested first active item, we have flexibility to choose the 'active slot'
			// that will allow us to fill as many of the selectable slots as possible - a loose 'full mode'
			// if you will

			if (m_scrollSpan > 1)
			{
				// Check that the requested active item is on the correct 'row',
				// otherwise change the active slot to suit
				if ((firstActiveItem % m_scrollSpan) != (m_activeSlot % m_scrollSpan))
				{
					m_activeSlot += (firstActiveItem % m_scrollSpan) - (m_activeSlot % m_scrollSpan);
				}
			}

		}
	}

	// Empty all slots to start with
	for (s = 0; s < m_slotCount; s++)
		m_slotRefs[s] = -1;

	// Set up the item - slot assignment
	if (wrap)
	{
		// What is the item index in the first selectable slot?
		if (firstActiveItem != -1)
			r = firstActiveItem - m_activeSlot + m_firstSelectableSlot;
		else if (m_firstPopulateSlot != -1)
			r = -m_firstPopulateSlot + m_firstSelectableSlot;
		else
			r = 0 - m_activeSlot + m_firstSelectableSlot;

		// Now go through the slots - assign items to slots

		// Fill the selectable slot range first, to guarantee at least one is full
		s = m_firstSelectableSlot;
		int itemCount = 0;

		while ((s <= m_lastSelectableSlot) && (itemCount < m_itemCount))
		{
			m_slotRefs[s] = wrapToRange(r, m_itemCount);

			s++;
			r++;
			itemCount++;
		}

		// Slots after selectable slot range (if any)
		if (m_lastSelectableSlot < (m_slotCount-1))
		{
			while ((s < m_slotCount) && (itemCount < m_itemCount))
			{
				m_slotRefs[s] = wrapToRange(r, m_itemCount);

				s++;
				r++;
				itemCount++;
			}
		}

		// Slots before selectable slot range (if any)
		if (m_firstSelectableSlot > 0)
		{
			s = m_firstSelectableSlot - 1;
			r = m_slotRefs[m_firstSelectableSlot] - 1;
			while ((s >= 0) && (itemCount < m_itemCount))
			{
				m_slotRefs[s] = wrapToRange(r, m_itemCount);

				s--;
				r--;
				itemCount++;
			}
		}
	}
	else
	{
		// Non-wrapping, or we don't have sufficient items to fill all selectable slots
		if (m_fullMode)
		{
			// The rule is: while populating as many selectable slots as
			// possible, move the first active item (if set) as near to
			// the active slot as possible, then change the active slot
			// to be the one containing the first active item (if set).
			if (m_itemCount <= getSelectableSlotCount())
			{
				// We haven't got enough items to fill all selectable slots

				// Ideally, what is the item index in the first slot
				if (firstActiveItem != -1)
				{
					r = firstActiveItem - m_activeSlot;
				}
				else if (m_firstPopulateSlot != -1)
				{
					r = -m_firstPopulateSlot;
					if (r + m_activeSlot >= m_itemCount)
						r -= m_scrollSpan * ((r + m_activeSlot - m_itemCount + m_scrollSpan)/m_scrollSpan);
					else if (r + m_activeSlot < 0)
						r = -m_activeSlot;
				}
				else
				{
					r = -m_activeSlot;
				}

				// Make sure that all the items are in the selectable slot range
				if (r + m_firstSelectableSlot > 0)
					r = -m_firstSelectableSlot;
				else if ((r + m_lastSelectableSlot) < (m_itemCount-1))
					r = m_itemCount - m_lastSelectableSlot - 1;

				// Now...if there is an active item specified, we move it as
				// close as possible to the active slot, then change the active
				// slot, otherwise just make sure the active slot has an item
				if (firstActiveItem == -1)
				{
					if (r + m_activeSlot >= m_itemCount)
						r = m_itemCount - m_activeSlot - 1;
					else if (r + m_activeSlot < 0)
						r = - m_activeSlot;
				}
				else
				{
					if (r + m_activeSlot > firstActiveItem)
						r = max(firstActiveItem - m_activeSlot, m_itemCount - 1 - m_lastSelectableSlot);
					if (r + m_activeSlot < firstActiveItem)
						r = min(firstActiveItem - m_activeSlot, -m_firstSelectableSlot);

					// If the active slot isn't the one with the active item in,
					// change the active slot
					if (r + m_activeSlot != firstActiveItem)
						setActiveSlot(firstActiveItem - r);
				}

				if (inactiveLayout)
				{
					m_ignoreScrollPosUpdate = true;
					m_scrollPos->jumpValueTo((int)m_scrollPos->getMaxAllowableVal());
					m_ignoreScrollPosUpdate = false;
					m_lastActiveItemScrollPos->jumpValueTo((int)m_scrollPos->getMaxAllowableVal());
				}
			}
			else
			{
				// Try to ensure that all the selectable slots are
				// populated...note that this is full mode, so we can leave non-selectable slots
				// unfilled if it moves the active item closer to the active slot

				// now work out the item in the first slot
				if (firstActiveItem != -1)
				{
					r = firstActiveItem - m_activeSlot;
				}
				else if (m_firstPopulateSlot != -1)
				{
					r = -m_firstPopulateSlot;
					if (r + m_activeSlot >= m_itemCount)
						r -= m_scrollSpan * ((r + m_activeSlot - m_itemCount + m_scrollSpan)/m_scrollSpan);
					else if (r + m_activeSlot < 0)
						r = -m_activeSlot;
				}
				else
				{
					r = -m_activeSlot;
				}

				if (m_scrollSpan == 1)
				{
					// Must obey the 'fill all selectable slots' rule
					if ((r + m_firstSelectableSlot) < 0)
						r = -m_firstSelectableSlot;
				}
				else
				{
					// in the grid scroll case, we shift the start point by multiples of m_scrollSpan
					// until at least some of the first row of selectable slots are filled.
					if ((r + m_firstSelectableSlot) < 0)
					{
						r -= m_scrollSpan * ((r + m_firstSelectableSlot) / m_scrollSpan);
					}
				}

				if ((r + m_lastSelectableSlot) >= m_itemCount)
				{
					r = m_itemCount - m_lastSelectableSlot - 1;

					if (m_scrollSpan > 1)
					{
						while (r % m_scrollSpan)
						{
							r++;
						}
					}
				}

				// If the active slot isn't the one with the active item in,
				// change the active slot
				if (firstActiveItem != -1)
				{
					if (r + m_activeSlot != firstActiveItem)
						setActiveSlot(firstActiveItem - r);
				}
			}

			s = 0;

			// Now populate the slots
			while ((r < m_itemCount) && (s < m_slotCount))
			{
				if (r >= 0)
					m_slotRefs[s] = r;

				s++;
				r++;
			}
		}
		else
		{
			// Simplest case...start with the first item in the first populate slot
			// What is the item in the 0th slot?
			if (firstActiveItem != -1)
			{
				r = firstActiveItem - m_activeSlot;
			}
			else if (m_firstPopulateSlot != -1)
			{
				r = -m_firstPopulateSlot;
				if (r + m_activeSlot >= m_itemCount)
				{
					r -= m_scrollSpan * ((r + m_activeSlot - m_itemCount + m_scrollSpan)/m_scrollSpan);
				}
				else if (r + m_activeSlot < 0)
				{
					r = -m_activeSlot;
				}
			}
			else
			{
				r = -m_activeSlot;
			}

			s = 0;

			while ((r < m_itemCount) && (s < m_slotCount))
			{
				if(r >= 0)
					m_slotRefs[s] = r;

				s++;
				r++;
			}
		}
	}

	// For LOD menus, we need to set the start of the window.  Arrange for us to
	// always be on the middle 'page' of items in the window, so that there is
	// m_slotCount worth of items in the positive and negative scroll directions
	if (m_windowStart < 0)
		m_windowStart = wrapToRange(m_slotRefs[m_activeSlot] - m_activeSlot - m_slotCount, m_itemCount);

	// Load the set of item required
	loadItems(m_windowStart, m_windowSize, false);

	// Let the items know which slot they're in
	for (s = 0; s < m_slotCount; s++)
	{
		r = m_slotRefs[s];
		m_oldSlotRefs[s] = r;

		if (r < 0 || r >= m_itemCount)
			continue;

		if (m_items[r])
			m_items[r]->setSlot(s, -1);
	}

	int cachedActiveSlot = m_activeSlot;

	// force setActiveSlot to apply a slot change
	// so ensure that cachedActiveSlot is not equal
	// to m_activeSlot
	if (inactiveLayout == true)
		m_activeSlot = 0;
	else
		m_activeSlot = -1;

	// Now we've filled the item-slot assignment, we can formally set the
	// active slot (which in turn constrains scrollpos in some circumstances
	setActiveSlot(inactiveLayout ? -1 : cachedActiveSlot);

	// Note that the active item isn't necessarily the first item (note that this will
	// also set up a detail element on the active item if the page is in an active state
	if (wrap)
	{
		if (firstActiveItem != -1)
			setActiveItem(firstActiveItem);
		else
			setActiveItem(m_slotRefs[cachedActiveSlot]);
	}
	else if (!(inactiveLayout && m_fullMode))
	{
		setActiveItem(m_slotRefs[cachedActiveSlot]);
	}
	else if (inactiveLayout && inactiveLayoutActiveItem != -1)
	{
		// We have had to change the initial active item when laying out
		// an inactive menu...lets update scrollpos...
		setActiveItem(inactiveLayoutActiveItem);
	}
}

/*-------------------------------------------------------------------------*//**
	Update all slots
*/
void NdhsCMenuComponent::updateSlots(bool wrap)
{
	int s, r;
	bool inactiveLayout = false;

	if (!m_outerGroup)
		return;

	// Only wrap if we can
	wrap = wrap && isWrappable();

	// Empty all slots to start with
	for (s = 0; s < m_slotCount; s++)
	{
		m_cachedSlotRefs[s] = m_slotRefs[s];
		m_slotRefs[s] = -1;
	}

	// What is the active item? Allow non-wrapping menus
	// to not clamp to a valid item to support rebound...
	r = getActiveItem(ENdhsActiveItemCurrent, false);
	// Move all the items into the 'aether'
	for (int i = 0; i < m_itemCount; i++)
	{
		if (m_items[i])
			m_items[i]->setSlot(-1, -1);
	}

	// For grids, check that the current active item aligns
	// with the previous active item, to prevent reassignment
	// glitches when we're part way through a scroll
	if (m_scrollSpan > 1)
	{
		int previousActiveItem = getActiveItem(ENdhsActiveItemOld, false);
		int tempActiveItem = r;

		// Adjust for any potential wrapping
		if (wrap)
		{
			if (m_scrollPositive && (tempActiveItem < previousActiveItem))
				tempActiveItem += m_itemCount;
			else if (!m_scrollPositive && (tempActiveItem > previousActiveItem))
				tempActiveItem -= m_itemCount;
		}

		int distance = m_scrollPositive ? tempActiveItem - previousActiveItem : previousActiveItem - tempActiveItem;

		if (distance % m_scrollSpan)
		{
			r = m_scrollPositive ? r - distance % m_scrollSpan : r + distance % m_scrollSpan;

			if (wrap)
				r = wrapToRange(r, m_itemCount);
		}
	}

	if (m_activeSlot == -1)
	{
		inactiveLayout = true;
		m_activeSlot = getInactivePopulateSlot();

		// Make sure that at least one item is in the selectable slot range - 'full' mode
		// menus will adjust this further below
		if (!wrap && ((m_activeSlot + m_itemCount - 1) < m_firstSelectableSlot))
		{
			m_activeSlot = m_firstSelectableSlot - (m_itemCount - 1);
		}
		else if (!wrap && (m_activeSlot > m_lastSelectableSlot))
		{
			m_activeSlot = m_lastSelectableSlot;
		}
	}

	if (wrap)
	{
		// What is the item index in the first selectable slot?
		int firstSlotIndex = r - m_activeSlot + m_firstSelectableSlot;
		int firstFilledSlot = m_firstSelectableSlot;
		int lastFilledSlot = m_lastSelectableSlot;

		// Now go through the slots - assign items to selectable slots
		for (s = m_firstSelectableSlot; s <= m_lastSelectableSlot; s++)
		{
			m_slotRefs[s] = wrapToRange(firstSlotIndex + s - m_firstSelectableSlot, m_itemCount);
		}

		// We may not have sufficient items to fill all slots
		if (m_itemCount < m_slotCount)
		{
			int firstUnusedItem = wrapToRange(firstSlotIndex + getSelectableSlotCount(), m_itemCount);
			int lastUnusedItem = wrapToRange(firstSlotIndex - 1, m_itemCount);
			int itemsToPlace = m_itemCount - getSelectableSlotCount();

			// Fill in the non-selectable slots in the direction of travel
			// with the spare items
			if (!m_scrollPositive)
			{
				if (m_lastSelectableSlot < m_slotCount - m_scrollSpan)
				{
					// Put first unused items in the first non-selectable slot row after the last selectable
					// slot row
					for (int i = 0; (i < m_scrollSpan) && (itemsToPlace > 0); i++)
					{
						m_slotRefs[m_lastSelectableSlot + 1 + i] = firstUnusedItem;
						lastFilledSlot++;
						itemsToPlace--;
						firstUnusedItem++;
						firstUnusedItem = wrapToRange(firstUnusedItem, m_itemCount);
					}
				}

				if (m_firstSelectableSlot > m_scrollSpan - 1)
				{
					// Put last unused items in the non-selectable slot row before the first selectable
					// slot row
					for (int i = 0; (i < m_scrollSpan) && (itemsToPlace > 0); i++)
					{
						m_slotRefs[m_firstSelectableSlot - 1 - i] = lastUnusedItem;
						firstFilledSlot--;
						itemsToPlace--;
						lastUnusedItem--;
						lastUnusedItem = wrapToRange(lastUnusedItem, m_itemCount);
					}
				}

				// If there are any more items, distribute them in the remaining unfilled slots
				if (itemsToPlace > 0)
				{
					s = firstFilledSlot - 1;
					r = lastUnusedItem;

					while (itemsToPlace > 0)
					{
						m_slotRefs[wrapToRange(s, m_slotCount)] = wrapToRange(r, m_itemCount);
						s--;
						r--;
						itemsToPlace--;
					}
				}
			}
			else
			{
				if (m_firstSelectableSlot > m_scrollSpan - 1)
				{
					// Put last unused items in the non-selectable slot row before the first selectable
					// slot row
					for (int i = 0; (i < m_scrollSpan) && (itemsToPlace > 0); i++)
					{
						m_slotRefs[m_firstSelectableSlot - 1 - i] = lastUnusedItem;
						firstFilledSlot--;
						itemsToPlace--;
						lastUnusedItem--;
						lastUnusedItem = wrapToRange(lastUnusedItem, m_itemCount);
					}
				}

				if (m_lastSelectableSlot < m_slotCount - m_scrollSpan)
				{
					// Put first unused items in the first non-selectable slot row after the last selectable
					// slot row
					for (int i = 0; (i < m_scrollSpan) && (itemsToPlace > 0); i++)
					{
						m_slotRefs[m_lastSelectableSlot + 1 + i] = firstUnusedItem;
						lastFilledSlot++;
						itemsToPlace--;
						firstUnusedItem++;
						firstUnusedItem = wrapToRange(firstUnusedItem, m_itemCount);
					}
				}

				// If there are any more items, distribute them in the remaining unfilled slots
				if (itemsToPlace > 0)
				{
					s = lastFilledSlot + 1;
					r = firstUnusedItem;

					while (itemsToPlace > 0)
					{
						m_slotRefs[wrapToRange(s, m_slotCount)] = wrapToRange(r, m_itemCount);
						s++;
						r++;
						itemsToPlace--;
					}
				}
			}
		}
		else
		{
			// We have sufficient items to populate all slots
			// Sort any empty slots at the start of the slot array
			s = m_firstSelectableSlot - 1;
			r = firstSlotIndex - 1;
			while (s >= 0)
			{
				m_slotRefs[s] = wrapToRange(r, m_itemCount);
				s--;
				r--;
			}

			// Sort any empty slots at the end of the slot array
			s = m_lastSelectableSlot + 1;
			r = firstSlotIndex + getSelectableSlotCount();
			while (s < m_slotCount)
			{
				m_slotRefs[s] = wrapToRange(r, m_itemCount);
				s++;
				r++;
			}
		}
	}
	else
	{
		// Non-wrapping case...
		s = m_activeSlot - r;
		if (s < 0)
		{
			s = 0;
			r = r - m_activeSlot;
		}
		else
		{
			s = m_activeSlot - r;
			r = 0;
		}

		while ((r < m_itemCount) && (s < m_slotCount))
		{
			m_slotRefs[s] = r;
			s++;
			r++;
		}
	}

	int oldSlot;
	// Let the slots know
	for (s = 0; s < m_slotCount; s++)
	{
		r = m_slotRefs[s];

		if (r < 0 || r >= m_itemCount)
			continue;

		// Note that it's not permitted for items to tween
		// between non-adjacent slots
		if (m_scrollPositive)
			oldSlot = (s < m_slotCount - m_scrollSpan) ? (s + m_scrollSpan) : -1;
		else
			oldSlot = (s >= m_scrollSpan) ? (s - m_scrollSpan) : -1;

		// The special case of 'circular' mode - all the items are always on
		// screen, so the oldSlot for one item will be wrong
		if (m_circularMode)
		{
			if (oldSlot == -1)
				oldSlot = m_scrollPositive ? 0 : m_slotCount - 1;
		}

		if (m_items[r])
			m_items[r]->setSlot(s, oldSlot);
	}

	if (m_animType == ENdhsAnimationTypeDrag)
	{
		// There are up to 'm_scrollSpan' worth of items leaving slots...we
		// need to set their 'oldSlot' values here in the drag case, just
		// in case the scroll pos drifts back to the 'old' active item
		// (unless the oldSlot is already set, of course!)
		if (m_scrollPositive)
		{
			if (m_oldSlotRefs[m_scrollSpan - 1] != -1)
			{
				for (int i = m_scrollSpan - 1; i >= 0; i--)
				{
					r = m_oldSlotRefs[i];
					if (r >= 0)
					{
						if (m_items[r] && (m_items[r]->getOldSlot() == -1))
							m_items[r]->setSlot(m_items[r]->getSlot(), i);
					}
				}
			}
		}
		else
		{
			if (m_oldSlotRefs[m_slotCount-1] != -1)
			{
				for (int i = m_slotCount - m_scrollSpan; i < m_slotCount; i++)
				{
					r = m_oldSlotRefs[i];
					if (r >= 0)
					{
						if (m_items[r] && (m_items[r]->getOldSlot() == -1))
							m_items[r]->setSlot(m_items[r]->getSlot(), i);
					}
				}
			}
		}
	}

	// Store the slot/item assignments for next time for the loop above
	// Note that for load-on-demand menus, the item may have been deleted
	// already!
	for (s = 0; s < m_slotCount; s++)
	{
		m_oldSlotRefs[s] = m_slotRefs[s];
	}

	for (int i = 0; i < m_itemCount; i++)
	{
		if (m_items[i])
			m_items[i]->checkDetails();
	}

	if (inactiveLayout == true)
		m_activeSlot = -1;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::scrollItems(NdhsCMenuComponentTemplate::CAction::CScrollBy* scrollAttempt)
{
	// Fail if nothing to do
	if (!scrollAttempt || scrollAttempt->amount == 0 || m_itemCount<=0 || !m_outerGroup)
		return false;

	// Check wrap status
	bool wrap = m_scrollWrap && isWrappable();

	// Also fail if we're wrapping and we'd end up at the same item
	if (wrap)
	{
		// Also fail if we're wrapping and we'd end up at the same item
		if (abs((int)scrollAttempt->amount * m_scrollSpan)%m_itemCount == 0)
			return false;
	}
	else
	{
		// Also fail if we're not wrapping and we'd exceed the item count bounds
		int val;
		if (scrollAttempt->type == NdhsCMenuComponentTemplate::CAction::CScrollBy::EScrollByTypeKick)
			val = getActiveItem(ENdhsActiveItemTarget);
		else
			val = getActiveItem(ENdhsActiveItemCurrent);

		if ( (val + (int)scrollAttempt->amount) > (int)m_scrollPos->getMaxAllowableVal() ||
				(val + (int)scrollAttempt->amount) < (int)m_scrollPos->getMinAllowableVal())
			return false;
	}

	// If there are any statics, tell them they should complete next loop
	// Note that it's only item statics affected by this!
	stopStaticAnimations(false, EAnimationItemOnly);

	LcTScalar val;
	ENdhsVelocityProfile velocityProfile;
	if (scrollAttempt->type == NdhsCMenuComponentTemplate::CAction::CScrollBy::EScrollByTypeKick)
	{
		// Get target value
		val = (LcTScalar)getActiveItem(ENdhsActiveItemTarget);

		// Apply kick max velocity
		m_scrollPos->setMaxSpeed(m_menuTemplate->scrollKickMaxVelocity());

		// Kicks cannot use any other velocity profile
		velocityProfile = ENdhsVelocityProfileDecelerate;
	}
	else
	{
		// Get current value
		val = (LcTScalar)getActiveItem(ENdhsActiveItemCurrent);

		// Unset any kick max velocity
		m_scrollPos->setMaxSpeed(-1);

		// Apply any velocity profile
		velocityProfile = scrollAttempt->velocityProfile;

		// Store current value...need to preserve unwrapped value,
		// so use 'snapshot' feature
		m_lastActiveItemScrollPos->jumpValueTo(0);
		m_lastActiveItemScrollPos->takeSnapshot();
		m_lastActiveItemScrollPos->addToSnapshot(m_scrollPos->getWrapAdjustedVal(),
													m_lastActiveItemScrollPos->getMinValue(),
													m_lastActiveItemScrollPos->getMaxValue(),
													true);
		// Update any metafields
		updateMetaFields();
	}

	// Add required change
	val += scrollAttempt->amount * m_scrollSpan;

	bool localScrollPositive = (scrollAttempt->amount > 0) ? true : false;

	// Check direction of kick if already kicking
	if ((scrollAttempt->type == NdhsCMenuComponentTemplate::CAction::CScrollBy::EScrollByTypeKick)
			&& (m_animType == ENdhsAnimationTypeScrollKick))
	{
		if ((!localScrollPositive && m_scrollPositive)
				|| (localScrollPositive && !m_scrollPositive))
		{
			// Work out last active item
			int nearest = getActiveItem(ENdhsActiveItemOld);

			// Check that the target isn't actually this already
			if (getActiveItem(ENdhsActiveItemTarget) != nearest)
			{
				// Remove velocity and target value from scrollpos field
				LcTDScalar currentPos = m_scrollPos->getWrapAdjustedVal();

				m_ignoreScrollPosUpdate = true;
				m_scrollPos->setValue((LcTScalar)currentPos, true);
				m_scrollPos->setTargetValue((LcTScalar)m_scrollPos->getWrappedVal(),0);
				m_scrollPos->setVelocity(0, 0, 0);
				m_ignoreScrollPosUpdate = false;
				return true;
			}
			else
			{
				// Must be a second kick following the first direction change
				// kick...apply kick value
				val = getActiveItem(ENdhsActiveItemTarget) + scrollAttempt->amount * m_scrollSpan;
			}
		}
	}

	// Do we need to wrap it into the valid scrollpos range?
	if (wrap)
		val = (LcTScalar)wrapToRange((int)val, m_itemCount);

	m_scrollPositive = localScrollPositive;

	// Set new target value
	return m_scrollPos->setTargetValue( val,
						localScrollPositive ? ENdhsFieldDirectionIncreasing : ENdhsFieldDirectionDecreasing,
						m_scrollWrap,
						scrollAttempt->duration,
						ENdhsScrollFieldModeNormal,
						velocityProfile);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::updatePosition(LcTScalar position,
										bool positionIncreasing,
										bool updateCache,
										bool finalFrame)
{
	LcTPlacement placement;
	LcTScalar animationFrac = position;

	if (!m_outerGroup)
		return;

	// check we are tweening between layouts?
	if (m_isLayoutChange == true)
	{
		NdhsCField* field = m_fieldCache->getField("_transitioning");
		if (field)
		{
			field->setValueBool(true, true);
		}
	}

	// Apply any triggers that have fired in the last frame
	if (m_pAnimTriggerList && (m_animType != ENdhsAnimationTypeDrag))
	{
		// Check for looping
		if (animationFrac < m_lastTriggerPos)
		{
			m_lastTriggerPos = -1;
		}

		int listSize = m_pAnimTriggerList->triggerList.size();
		int i = 0;

		if (listSize > 0)
		{
			while ((i < listSize) &&
					(m_pAnimTriggerList->triggerList[i]->position <= m_lastTriggerPos))
			{
				i++;
			}

			if ((i < listSize) && (m_pAnimTriggerList->triggerList[i]->position <= animationFrac))
			{
				// Do this trigger
				simulateKeyDown(m_pAnimTriggerList->triggerList[i]->key);
				m_lastTriggerPos = m_pAnimTriggerList->triggerList[i]->position;
			}
			else
			{
				m_lastTriggerPos = animationFrac;
			}
		}
	}

	// Update the scroll position velocity field
	NdhsCField* field = m_fieldCache->getField("_scrollvelocity");
	if (field && m_scrollPos && m_itemCount>0)
	{
		// Note we multiply by 1000 here; internally this velocity is represented as
		// units per millisecond; to the user, it is presented as units per second, as that
		// is how it is configured (see e.g. "maxVelocity" attribute) and velocity can
		// never be negative
		LcTScalar fieldVal = lc_fabs(m_scrollPos->getVelocity()) * 1000.0f;

		if (m_scrollPos->isAnimating())
			field->setValue(fieldVal, true);
		else
			field->setValue(0.0f, true);
	}

	// Should we force non-sketchy drawing?
	bool forceNonSketchy = false;
	switch(m_animType)
	{
		case ENdhsAnimationTypeDrag:
		{
			// If the mouse is still down, always draw in high quality
			if (m_bMouseDown)
			{
				forceNonSketchy = true;
			}
			else
			{
				// If the scroll pos velocity is below the threshold, draw in high quality
				if (lc_fabs(m_scrollPos->getVelocity()) < ANIM_NON_SKETCHY_THRESHOLD)
					forceNonSketchy = true;
			}

			break;
		}

		case ENdhsAnimationTypeScroll:
		case ENdhsAnimationTypeScrollKick:
		{
			if (lc_fabs(m_scrollPos->getVelocity()) < ANIM_NON_SKETCHY_THRESHOLD)
				forceNonSketchy = true;

			break;
		}

		// All other animations let the elements decide on their
		// draw quality
		default:
			break;
	}

	// Position the menu group
	if (m_outerGroup)
	{
		m_outerGroup->updatePosition(getAnimType(), animationFrac, positionIncreasing, updateCache,
															forceNonSketchy, false, finalFrame);
	}

	// Screen aggregate position
	if(m_localScreenAggregate)
	{
		((NdhsCElementGroup*)m_localScreenAggregate.ptr())->updatePosition(getAnimType(), animationFrac, positionIncreasing, updateCache,
															false, false, finalFrame);
	}

	// Position the individual slots, items and details
	if (m_itemCount > 0)
	{
		CItem* item;
		int offset = wrapToRange(m_windowStart, m_itemCount);

		for (int i = 0; i < m_windowSize; i++)
		{
			item = m_items[offset];

			if(item)
				item->updatePosition(getAnimType(), animationFrac, positionIncreasing, updateCache,
												forceNonSketchy, m_outerGroup.ptr()->getAnimateMask() != 0, finalFrame);

			offset++;
			offset = wrapToRange(offset, m_itemCount);
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Check whether we need to change the visibility status of the group
*/
bool NdhsCMenuComponent::doPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame)
{
	bool reschedule = false;
	bool finalUpdate = false;
	bool skipUpdateSlots = false;
	bool dragScrollAtStart = false;

	if (!m_outerGroup)
		return false;

	// Note if this is the last frame
	if (m_animType == ENdhsAnimationTypeNone)
		finalFrame = true;
	else
		finalFrame = m_lastFrame;

	switch (m_animType)
	{
		case ENdhsAnimationTypeScroll:
		case ENdhsAnimationTypeScrollKick:
		case ENdhsAnimationTypeDrag:
		{
			if (m_menu != NULL && m_itemCount > 0)
			{
				// If we're scrolling normally, or flick scrolling, update the scrollpos value here
				// drag scrolling updates scrollpos directly.
				if ((m_animType != ENdhsAnimationTypeDrag) || !m_scrollPos->isUserLocked())
				{
					if (m_scrollPos->isAnimating())
					{
						// We're about to (possibly) change the value of scrollpos.  If fieldValueUpdated
						// wasn't expecting this change, it'd think we've started a drag scroll, so set
						// this bool.
						m_ignoreScrollPosUpdate = true;
						m_scrollPos->updateValue(timestamp, finalUpdate);
						m_ignoreScrollPosUpdate = false;

						// when we arrive at the scrollpos target we don't want to go through 'updateSlots'
						if (finalUpdate)
						{
							skipUpdateSlots = true;

							// For drag scrolling, we need to know whether we've got past half way to the next active item
							if (m_animType == ENdhsAnimationTypeDrag)
								dragScrollAtStart = fabs(m_scrollPos->getWrapAdjustedVal() - m_lastActiveItemScrollPos->getWrapAdjustedVal()) < 0.5f;
						}
					}
					else
					{
						finalUpdate = true;
					}
				}

				LcTDScalar currentPos = m_scrollPos->getWrappedVal();

				// Broadcast the update to the scroll pos field here if necessary
				if (m_scrollPosIntrinsicStale)
				{
					// set the current scroll position value
					NdhsCField* themefield = m_fieldCache->getField("_scrollpos");
					if (themefield)
					{
						themefield->setValue((LcTScalar)currentPos, true);
					}

					m_scrollPosIntrinsicStale = false;
				}

				bool updateCache = false;
				int currentActiveItem;
				int previousActiveItem;
				LcTScalar animationFrac = 0;

				bool menuWrapping = m_scrollWrap && isWrappable();
				LcTDScalar currentPosUnwrapped = m_scrollPos->getWrapAdjustedVal();
				LcTDScalar lastActiveItemPosUnwrapped = m_lastActiveItemScrollPos->getWrapAdjustedVal();

				// Check for change of direction
				if (m_scrollPositive)
				{
					if (currentPosUnwrapped < lastActiveItemPosUnwrapped)
					{
						m_scrollPositive = false;
						updateCache = true;
					}
				}
				else
				{
					if (currentPosUnwrapped > lastActiveItemPosUnwrapped)
					{
						m_scrollPositive = true;
						updateCache = true;
					}
				}

				// Note that we don't ask for the active item to be clamped, to
				// allow for 'rebound' behaviour
				currentActiveItem = getActiveItem(ENdhsActiveItemCurrent, false);
				previousActiveItem = getActiveItem(ENdhsActiveItemOld, false);

				int activeItem = currentActiveItem;

				// Adjust for any potential wrapping
				if (menuWrapping)
				{
					if (m_scrollPositive && (activeItem < previousActiveItem))
						activeItem += m_itemCount;
					else if (!m_scrollPositive && (activeItem > previousActiveItem))
						activeItem -= m_itemCount;
				}

				// We need to refresh the cache if we've hit a slot boundary and it's not
				// the final update, or if we're on the final update and we're not within
				// 'span' of the last update position
				bool cacheRefreshCheckRequired = false;

				if ((activeItem < (previousActiveItem - m_scrollSpan))
					|| (activeItem > (previousActiveItem + m_scrollSpan)))
					cacheRefreshCheckRequired = true;

				if (cacheRefreshCheckRequired)
				{
					// element animation caches will require refreshing
					updateCache = true;

					// If we've passed through a slot, we need to update the last active item scroll pos
					// We may have jumped multiple items, so be careful when deciding what the old active item was,
					// particularly for grid menus
					int newPrevActiveItem = m_scrollPositive ? activeItem - m_scrollSpan : activeItem + m_scrollSpan;

					if (newPrevActiveItem != previousActiveItem)
					{
						int delta = (int)(newPrevActiveItem - previousActiveItem);
						delta = (delta/m_scrollSpan)*m_scrollSpan;
						m_lastActiveItemScrollPos->takeSnapshot();
						m_lastActiveItemScrollPos->addToSnapshot((LcTDScalar)delta,
														m_lastActiveItemScrollPos->getMinValue(),
														m_lastActiveItemScrollPos->getMaxValue(),
														true);
						// Update any metafields
						updateMetaFields();
						// OK - in this scenario we need to update slots even on the final
						// frame...
						skipUpdateSlots = false;
					}
				}

				if (updateCache && !skipUpdateSlots)
				{
					// Any LOD required?
					doLoadOnDemand(currentActiveItem);

					// Propagate changes to the items unless this is the end of the scroll
					updateSlots(m_scrollWrap);

					// Only create the new active slot detail if it's the final active slot
					// Also note we don't create the detail if the mouse is down (i.e. drag scrolling),
					// or if we're in the inactive state
					if (!m_bMouseDown && (m_activeSlot != -1) && ((m_scrollPositive && ((m_scrollPos->getTargetValue() - (LcTScalar)currentPos) <= 1))
						|| (!m_scrollPositive && (((LcTScalar)currentPos - m_scrollPos->getTargetValue()) <= 1))) )
					{
						if (m_slotRefs[m_activeSlot] > -1 && m_slotRefs[m_activeSlot] < m_itemCount)
						{
							if (m_items[m_slotRefs[m_activeSlot]])
								m_items[m_slotRefs[m_activeSlot]]->loadDetail();
						}
					}
				}

				// Now update all the positions
				// we want the fractional part through the animation phase.
				animationFrac = (LcTScalar)(m_scrollPos->getWrapAdjustedVal() - m_lastActiveItemScrollPos->getWrapAdjustedVal());

				animationFrac = m_scrollPositive ? animationFrac/m_scrollSpan : -animationFrac/m_scrollSpan;

				if (!finalUpdate)
					updatePosition(animationFrac, m_scrollPositive, updateCache);

				// Check for animation complete
				if (finalUpdate)
				{
					if (m_lastFrame)
					{
						// Transition complete!
						onTransitionComplete(false, true);
						m_lastFrame = false;
					}
					else
					{
						// Next Frame is last frame
						animationFrac = dragScrollAtStart ? 0.0f : animationFrac;

						updatePosition(animationFrac, m_scrollPositive, updateCache);
						m_lastFrame = true;
						reschedule = true;
					}
				}
				else
				{
					if (m_lastFrame)
						m_lastFrame = false;

					reschedule = true;
				}

			}
			break;
		}

		case ENdhsAnimationTypeInteractiveState:
		{
			if (timestamp >= m_decorationDelayTimestamp)
			{
				m_layoutPos->updateValue(timestamp, finalUpdate);

				LcTScalar currentPos = m_layoutPos->getRawFieldData(NULL);

				// Check for animation complete
				if (m_lastFrame)
				{
					// Transition complete!
					onTransitionComplete(false, true);
					m_lastFrame = false;
				}
				else if (finalUpdate)
				{
					// Next Frame is last frame
					m_lastFrame = true;
					reschedule = true;
				}
				else
				{
					reschedule = true;
				}

				// Now update all the positions
				if (reschedule)
					updatePosition(currentPos, true, false);
			}
			else
			{
				// We're waiting for a transition to start...
				reschedule = true;
			}
			break;
		}

		case ENdhsAnimationTypeStatic:
		{
			reschedule = true;

			break;
		}
		default:
			break;
	}

	// Check fields
	reschedule |= updateFields(timestamp);

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::componentDoPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame)
{
	bool reschedule = false;

	// If group unloaded no need to go further
	if (!m_outerGroup || isGroupUnloaded())
		return false;

	if (NdhsCComponent::componentDoPrepareForFrameUpdate(timestamp, finalFrame))
		reschedule = true;

	if (m_itemCount <= 0)
		return reschedule;

	CItem* item;
	int offset = wrapToRange(m_windowStart, m_itemCount);

	for (int i = 0; i < m_windowSize; i++)
	{
		item = m_items[offset];
		if(item)
		{
			if (item->componentDoPrepareForFrameUpdate(timestamp, finalFrame))
				reschedule = true;
		}

		offset++;
		offset = wrapToRange(offset, m_itemCount);
	}

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::componentsJumpTransitionToEnd(bool setIdle)
{
	if (!m_outerGroup || isGroupUnloaded())
		return;

	NdhsCComponent::componentsJumpTransitionToEnd(setIdle);

	if (m_itemCount <= 0)
		return;

	CItem* item;
	int offset = wrapToRange(m_windowStart, m_itemCount);

	for (int i = 0; i < m_windowSize; i++)
	{
		item = m_items[offset];

		if(item)
			item->componentsJumpTransitionToEnd(setIdle);

		offset++;
		offset = wrapToRange(offset, m_itemCount);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::resumeStaticAnimations()
{
	if (!m_outerGroup || isGroupUnloaded())
		return;

	NdhsCComponent::resumeStaticAnimations();

	if (m_itemCount <= 0)
		return;

	CItem* item;
	int offset = wrapToRange(m_windowStart, m_itemCount);

	for (int i = 0; i < m_windowSize; i++)
	{
		item = m_items[offset];

		if(item)
			item->resumeStaticAnimations();

		offset++;
		offset = wrapToRange(offset, m_itemCount);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::componentRefreshField(const LcTmString& field, int item)
{
	if (!m_outerGroup || isGroupUnloaded())
		return;

	NdhsCComponent::componentRefreshField(field, item);

	if (m_itemCount <= 0)
		return;

	CItem* cItem;
	int offset = wrapToRange(m_windowStart, m_itemCount);

	for (int i = 0; i < m_windowSize; i++)
	{
		cItem = m_items[offset];

		if(cItem)
			cItem->componentRefreshField(field, item);

		offset++;
		offset = wrapToRange(offset, m_itemCount);
	}
}


/*-------------------------------------------------------------------------*//**
	Retrieve the plug-in index of the active item on a specified plug-in menu.
	Returns -1 for an error.
*/
int NdhsCMenuComponent::getHMenuActiveItemIndex(IFX_HMENU hMenu)
{
	if (!m_outerGroup)
		return -1;

	int index = -1;

	if(m_menu==NULL || isGroupUnloaded())
		return index;

	if (m_menu != NULL && NdhsCMenu::EPlugin == m_menu->getMenuSource())
	{
		// Check the current menu
		if (m_menu->getMenuPlugin() != NULL && hMenu == m_menu->getMenuPlugin()->getMenuSession())
			return getActiveMenuItem()->getIndex();
	}

	if (m_itemCount <= 0)
		return index;

	CItem* cItem;
	int offset = wrapToRange(m_windowStart, m_itemCount);

	for (int i = 0; i < m_windowSize; i++)
	{
		cItem = m_items[offset];

		if(cItem)
		{
			index = cItem->getHMenuActiveItemIndex(hMenu);;
			if (index != -1)
				return index;
		}

		offset++;
		offset = wrapToRange(offset, m_itemCount);
	}

	return index;
}

/*-------------------------------------------------------------------------*//**
	Retrieve the full path of a file for a specified plug-in menu manifest file.
	Returns false for an error.
*/
bool NdhsCMenuComponent::getFullFilePath(	IFX_HMENU hMenu,
										const LcTmString& searchFile,
										LcTmString& returnFilePath,
										int menuIndex)
{
	if (!m_outerGroup || !m_menu || isGroupUnloaded())
		return false;

	if (m_menu != NULL && NdhsCMenu::EPlugin == m_menu->getMenuSource())
	{
		if (m_menu->getMenuPlugin()->getMenuSession() == hMenu)
		{
			bool requiresWildCard = false;
			LcTaString ext = searchFile.getWord(-1, '.');
			LcTaArray<NdhsCManifest::CManifestFile*> fileData;
			int index=0;

			if (ext.compareNoCase("png") == 0)
			{
				requiresWildCard=true;
			}

			if (m_pageManager->getManifestStack()->findFile(searchFile, returnFilePath,  m_parent->getTemplate()->getPaletteManifest(), menuIndex,NULL,requiresWildCard,&fileData))
			{
				if(requiresWildCard && m_pageManager->findBitmapFile(&fileData,index))
					returnFilePath=fileData[index]->absolutePath;
				return true;
			}
		}
	}

	if (m_itemCount <= 0)
		return false;

	CItem* cItem;
	int offset = wrapToRange(m_windowStart, m_itemCount);

	for (int i = 0; i < m_windowSize; i++)
	{
		cItem = m_items[offset];

		if(cItem)
		{
			if (cItem->getFullFilePath(hMenu, searchFile, returnFilePath, menuIndex))
				return true;
		}

		offset++;
		offset = wrapToRange(offset, m_itemCount);
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::isComponentTransitioning()
{
	if (!m_outerGroup || isGroupUnloaded())
		return false;

	if (NdhsCComponent::isComponentTransitioning())
		return true;

	if (m_itemCount <= 0 || m_menu==NULL)
		return false;

	CItem* item;
	int offset = wrapToRange(m_windowStart, m_itemCount);

	for (int i = 0; i < m_windowSize; i++)
	{
		item = m_items[offset];

		if(item)
		{
			if (item->isComponentTransitioning())
				return true;
		}

		offset++;
		offset = wrapToRange(offset, m_itemCount);
	}
	return false;
}

#if defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::componentOnTransitionComplete(bool setIdle)
{
	if (!m_outerGroup)
		return;

	NdhsCComponent::componentOnTransitionComplete(setIdle);

	if (m_itemCount <= 0 || m_menu==NULL)
		return;

	CItem* item;
	int offset = wrapToRange(m_windowStart, m_itemCount);

	for (int i = 0; i < m_windowSize; i++)
	{
		item = m_items[offset];

		if(item)
			item->componentOnTransitionComplete(setIdle);

		offset++;
		offset = wrapToRange(offset, m_itemCount);
	}
}
#endif

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::updateFields(LcTTime timestamp)
{
	bool reschedule = false;

	if (!m_outerGroup)
		return false;

	// Now check the static animation fields
	if (m_staticAnimFields.size())
	{
		bool finalUpdate = false;
		LcTScalar oldVal = 0;
		LcTScalar newVal;

		LcTmOwnerArray<NdhsCScrollPosField>::iterator it = m_staticAnimFields.begin();

		for (; it != m_staticAnimFields.end(); it++)
		{
			if (m_stopStaticAnimationAll || m_stopStaticAnimationItem)
			{
				oldVal = (*it)->getRawFieldData(NULL);
				oldVal = oldVal - (int)oldVal;
			}

			(*it)->updateValue(timestamp, finalUpdate);

			if (m_stopStaticAnimationAll ||
				(m_stopStaticAnimationItem && (*it != m_pStaticAnimFieldTrigger)))
			{
				newVal = (*it)->getRawFieldData(NULL);
				newVal = newVal - (int)newVal;

				// this animator should be stopped on a 'wrap'.
				if (newVal < oldVal)
					(*it)->setValue(0, true);
				else
					reschedule |= !finalUpdate;
			}
			else
			{
				// We want another timestep if any of the static animators are still going;
				reschedule |= !finalUpdate;
			}
		}

		// If all animations have completed, clear them up.
		if (!reschedule)
			stopStaticAnimations(true, EAnimationAll);
	}

	// If for some reasond we dont have menu
	// no need to go further
	if (m_menu != NULL)
		reschedule |= m_menu->updateFields(timestamp);

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenuComponent::nearOriginSlot()
{
	bool retVal = false;

	if (m_itemCount <= 0 || !m_outerGroup)
		return false;

	// check current scrollpos value
	LcTDScalar currVal = m_scrollPos->getWrappedVal();
	LcTDScalar lastActiveItemVal = m_lastActiveItemScrollPos->getWrappedVal();

	// work out what the next valid active item scroll pos value is in the direction of travel
	int targetVal = m_scrollPositive ? (int)(lastActiveItemVal + m_scrollSpan) : (int)(lastActiveItemVal - m_scrollSpan);
	targetVal = wrapToRange(targetVal, m_itemCount);

	// Now work out if we're closer to the last active item scroll pos val or the next one
	if (m_scrollPositive)
	{
		if (currVal < lastActiveItemVal)
			retVal = (currVal + m_itemCount - lastActiveItemVal) < (.5*m_scrollSpan);
		else
			retVal = (currVal - lastActiveItemVal) < (.5*m_scrollSpan);
	}
	else
	{
		if (currVal > lastActiveItemVal)
			retVal = (lastActiveItemVal - currVal + m_itemCount) < (.5*m_scrollSpan);
		else
			retVal = (lastActiveItemVal - currVal) < (.5*m_scrollSpan);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::doLoadOnDemand(int activeItem,int currentIndex)
{
#define DEBUG_SWITCH 0
#define EXTERNAL_CODE

#if DEBUG_SWITCH /*ligui added here*/
printf("\033[0;32;31m""%s %d",__func__,__LINE__);
#endif
	if (m_menu != NULL && m_menu->isLodRequired())
	{
#if DEBUG_SWITCH /*ligui added here*/
printf("%s %d  variables:: activeItem:%d m_activeSlot:%d m_slotCount:%d m_itemCount:%d\n",__func__,__LINE__,activeItem,m_activeSlot,m_slotCount,m_itemCount);
#endif
		int newWindowStart = wrapToRange(activeItem - m_activeSlot - m_slotCount, m_itemCount);
		int tempWindowStart = newWindowStart;

#if defined(EXTERNAL_CODE) /*ligui added here*/
			int loadBegin = activeItem - m_activeSlot - m_slotCount;
#endif
		if (newWindowStart != m_windowStart)
		{
			int amount;
			int unloadIndex;
			int loadIndex;

			// Check for wrapping
			if (m_scrollPositive && (tempWindowStart < m_windowStart))
				tempWindowStart += m_itemCount;
			else if (!m_scrollPositive && (tempWindowStart > m_windowStart))
				tempWindowStart -= m_itemCount;

#if DEBUG_SWITCH /*ligui added here*/
printf("%s %d  newWindowStart: %d m_windowStart:%d tempWindowStart:%d m_windowSize:%d\n",__func__,__LINE__,newWindowStart,m_windowStart,tempWindowStart,m_windowSize);
#endif
			if (tempWindowStart > m_windowStart)
			{
				amount = tempWindowStart/*26*/ - m_windowStart/*20*/;
				unloadIndex	= m_windowStart/*20*/;/*go 34*/
				loadIndex	= m_windowStart + m_windowSize;/*41*/

				if (tempWindowStart > loadIndex)
				{
					loadIndex = tempWindowStart;
					amount = m_windowSize;
				}
			}
			else
			{
				amount = m_windowStart - tempWindowStart;
				unloadIndex	= m_windowStart + m_windowSize - amount;
				loadIndex	= m_windowStart - amount;

				if (amount > m_windowSize)
				{
					loadIndex = tempWindowStart;
					amount = m_windowSize;
				}
			}
			m_windowStart = wrapToRange(newWindowStart, m_itemCount);
			
#if DEBUG_SWITCH /*ligui added here*/
printf("%s %d activeItem:%d unloadIndex:%d loadIndex:%d amount:%d\n",__func__,__LINE__,activeItem,unloadIndex,loadIndex,amount);
printf("%s %d loadBegin:%d amount:%d\n",__func__,__LINE__,loadBegin,amount);

printf("%s %d currentIndex:%d m_itemCount:%d\n",__func__,__LINE__,currentIndex-m_activeSlot+1,m_itemCount);
#endif
			unloadItems(unloadIndex, amount);

#if defined(EXTERNAL_CODE) /*ligui added here*/
			if(currentIndex != -1)
			{
				int slotUnloadBegin = currentIndex-m_activeSlot+1;
				int slotUnloadEnd = slotUnloadBegin+m_slotCount-1;
				int loadEnd;

				loadEnd = loadBegin+m_windowSize-1;
				slotUnloadBegin = wrapToRange(slotUnloadBegin, m_itemCount);
				slotUnloadEnd = wrapToRange(slotUnloadEnd, m_itemCount);
				loadBegin = wrapToRange(loadBegin, m_itemCount);
				loadEnd = wrapToRange(loadEnd, m_itemCount);
				
				if(loadBegin > loadEnd)
				{
//74 loadEnd:6 slotUnloadBegin:85  slotUnloadEnd:93
					if(slotUnloadBegin >= loadBegin)
					{
						slotUnloadBegin -= m_itemCount;
					}
					if(slotUnloadEnd >= loadBegin)
					{
						slotUnloadEnd -= m_itemCount;
					}
					loadBegin -= m_itemCount;
				}
				if(slotUnloadBegin > slotUnloadEnd)
				{
//loadBegin:24 loadEnd:44 slotUnloadBegin:196 slotUnloadEnd:5
					slotUnloadBegin -= m_itemCount;
				}
#if DEBUG_SWITCH /*ligui added here*/
printf("%s %d loadBegin:%d loadEnd:%d slotUnloadBegin:%d\tslotUnloadEnd:%d\n",__func__,__LINE__,
loadBegin,loadEnd,slotUnloadBegin,slotUnloadEnd);
#endif
				if(slotUnloadBegin >= loadBegin && slotUnloadEnd > loadEnd/*loadIndex+amount*/)
				{
					slotUnloadBegin = slotUnloadBegin > loadEnd?slotUnloadBegin:loadEnd+1;
					unloadItems(slotUnloadBegin,slotUnloadEnd-slotUnloadBegin+1);
				}
				else if(slotUnloadBegin < loadBegin && slotUnloadEnd < loadEnd/*loadIndex+amount*/)
				{
					slotUnloadEnd = slotUnloadEnd < loadBegin?slotUnloadEnd:loadBegin-1;
					unloadItems(slotUnloadBegin,slotUnloadEnd -slotUnloadBegin+1);
				}
			}
#endif
			loadItems(loadIndex, amount, true, true);
#if 1 /*ligui added here*/
			if(amount > m_slotCount)
			{
				loadItems(activeItem-m_activeSlot+1, m_slotCount, true, true);
			}
#endif
			// Now delete obsolete items
			destroyUnwantedElements();

			m_placeholderUsed = true;
		}
	}
}

/*-------------------------------------------------------------------------*//**
	This function is used for stylus taps, and as such, only checks items
	that are actually in a slot.
*/
bool NdhsCMenuComponent::getSlotAndClassFromWidget(LcCWidget* widget, LcTmString &elementClass, int* slot)
{
	if (!m_outerGroup)
		return false;

	// Only check items currently in slots
	for (int s = 0; s < m_slotCount; s++)
	{
		CItem* item = getItemInSlot(s);
		NdhsCField* field = NULL;

		if (item)
		{
			// Check the item element group first
			elementClass = item->getItemClassFromWidget(widget);

			if (elementClass.length() > 0)
			{
				*slot = s;

				// Update the slot number field
				field = m_fieldCache->getField("_slotNumber");
				if (field)
				{
					if (s != -1)
					{
						field->setValue(((LcTScalar)s + 1), true);
					}
					else
					{
						field->setValue((LcTScalar)s, true);
					}
				}
				return true;
			}

			// Now check the detail element group
			elementClass = item->getDetailClassFromWidget(widget);

			if (elementClass.length() > 0)
			{
				// Detail is identified by class name and not slot
				// so set the slot to -1
				*slot = -1;

				// Update the slot number field
				field = m_fieldCache->getField("_slotNumber");
				if (field)
				{
					if (*slot != -1)
					{
						field->setValue(((LcTScalar)(*slot) + 1), true);
					}
					else
					{
						field->setValue((LcTScalar)(*slot), true);
					}
				}

				return true;
			}

		}
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCMenuComponent::getSlotContainingItem(NdhsCMenuComponent::CItem* item)
{
	if (!item || !m_outerGroup)
		return -1;

	for (int s = 0; s < m_slotCount; s++)
	{
		if (item == getItemInSlot(s))
			return s;
	}

	// Will reach here only if no slot holds the specified item
	return -1;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuComponent::CItem* NdhsCMenuComponent::getItemInSlot(int s)
{
	int r = getRefFromSlot(s);

	// One special case: when 'drag scrolling', items are associated with their
	// nearest slot, but the transition will have set up the next slot in the
	// direction of travel as the current slot.  If we're nearer the old slot
	// use the old item distribution
	if ((s != -1) && (m_animType == ENdhsAnimationTypeDrag))
	{
		if (nearOriginSlot())
		{
			r = m_cachedSlotRefs[s];
		}
	}

	if (r != -1 && m_itemCount > 0)
	{
		return m_items[wrapToRange(r, m_itemCount)];
	}
	else
	{
		return NULL;
	}
}

/*-------------------------------------------------------------------------*//**
	Get the ref (if any) being held by the specified slot
*/
int NdhsCMenuComponent::getRefFromSlot(int s)
{
	if (s == -1)
		return -1;
	else
		return m_slotRefs[s];
}

/*-------------------------------------------------------------------------*//**
	Executes a chained action
*/
bool NdhsCMenuComponent::executeChainedAction(const LcTmString& action, int slotNum)
{
	bool retVal = false;

	if (!m_outerGroup)
		return false;

	if (!action.isEmpty())
	{
		NdhsCTemplate::CAction * pActionObj= m_menuTemplate->getAction(action);

		if(pActionObj)
		{
			doAction(pActionObj, slotNum);
		}
		else
		{
			m_chainedAction = "";
		}
	}

	return retVal;
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::populateWidgetElementMap(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList)
{
	if (!m_outerGroup)
		return;

	NdhsCComponent::populateWidgetElementMap(widgets, pageWidgetElemList);

	CItem* 			item;
	TmOAItems::iterator itemIt;

	// Iterate through the detail item elements
	for (itemIt = m_items.begin(); itemIt != m_items.end(); itemIt++)
	{
		item = *itemIt;
		if (item)
			item->populateElementList(widgets, pageWidgetElemList);
	}
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::fieldValueUpdated(NdhsCField* field)
{
	if (!m_outerGroup || !m_menu)
		return;

	// Check static trigger
	if (field == m_pStaticAnimFieldTrigger && m_pStaticTriggerList)
	{
		LcTScalar val = m_pStaticAnimFieldTrigger->getRawFieldData(NULL);
		LcTScalar frac = val - (int)val;

		// Check for endpoint of static animation...
		if (m_pStaticAnimFieldTrigger->atRest())
			frac = 1.0;

		// Check for looping
		if (frac < m_lastStaticTriggerPos)
		{
			m_lastStaticTriggerPos = -1;
		}

		if (m_stopStaticAnimationAll && (m_pStaticAnimFieldTrigger->atRest() == false))
		{
			// Execute cleanup trigger
			if (m_pStaticTriggerList && m_pStaticTriggerList->hasCleanupTrigger)
				simulateKeyDown(m_pStaticTriggerList->cleanupTrigger);

			// Stop us receiving any further updates - note that we can't destroy the field, as
			// we're being called by it a the moment!
			m_pStaticAnimFieldTrigger->setTargetValue(val, 0);
		}
		else
		{
			int listSize = m_pStaticTriggerList->triggerList.size();
			int i = 0;

			if (listSize > 0)
			{
				while ((i < listSize) &&
						(m_pStaticTriggerList->triggerList[i]->position <= m_lastStaticTriggerPos))
				{
					i++;
				}

				if ((i < listSize) && (m_pStaticTriggerList->triggerList[i]->position <= frac))
				{
					// Do this trigger
					simulateKeyDown(m_pStaticTriggerList->triggerList[i]->key);
					m_lastStaticTriggerPos = m_pStaticTriggerList->triggerList[i]->position;
				}
				else
				{
					m_lastStaticTriggerPos = frac;
				}
			}
		}
	}
	else
	// check the scrollpos field
	if (field == m_scrollPos.ptr())
	{
		if (m_itemCount <= 0)
		{
			// there are no items then no need to start transition
			m_scrollPos->setValue(0,false);
			return;
		}

		// Note that the value of scrollpos has updated.  Do not update the _scrollPos
		// intrinsic here, however, as the expression re-evaluation may take significant
		// time - a performance killer in the context of an 'onMouseMove' call!
		m_scrollPosIntrinsicStale = true;

		// If we shouldn't update positions, etc, don't go any further.
		if (m_ignoreScrollPosUpdate)
			return;

		// set scroll update type
		if (!m_scrollPos->isDragValueUpdate())
		{
			// If it is not dragging then instant update
			m_scrollUpdateType = ENdhsScrollUpdateInstant;
		}
		else
		{
			// Switch to normal.. we might be interrupting a scroll animation
			// ahead or starting a new drag animation
			m_scrollUpdateType = ENdhsScrollUpdateNormal;
		}

		switch (m_animType)
		{
			case ENdhsAnimationTypeNone:
			case ENdhsAnimationTypeStatic:
			{
				stopStaticAnimations(true, EAnimationItemOnly);

				// Destroy any detail group
				int currentItem = getActiveItem(ENdhsActiveItemOld);
				if (currentItem > -1 && currentItem < m_itemCount)
				{
					if (m_items[currentItem])
						m_items[currentItem]->unloadDetail();
				}

				// Work out initial direction of movement
				// Check for wrapping of scrollpos
				if (m_scrollPos->getWrapAdjustedVal() > m_lastActiveItemScrollPos->getWrapAdjustedVal())
					m_scrollPositive = true;
				else
					m_scrollPositive = false;

				// We need to set up the cached transition data on the elements
				startTransition(ENdhsAnimationTypeDrag, true);
			}
			// Drop through...
			case ENdhsAnimationTypeDrag:
			{
				// Schedule a refresh event, otherwise we won't be asked to position
				// the elements in their new locations.
				m_pageManager->startAnimation();

				break;
			}

			case ENdhsAnimationTypeScrollKick:
			case ENdhsAnimationTypeScroll:
			{
				// We must intercept the scroll, and begin a drag scroll
				// Destroy any detail group
				int currentItem = getActiveItem(ENdhsActiveItemCurrent);
				int oldItem = m_scrollPositive ? currentItem - 1 : currentItem + 1;

				if (m_scrollWrap && isWrappable())
				{
					oldItem = wrapToRange(oldItem, m_itemCount);
				}

				if (currentItem > -1 && currentItem < m_itemCount)
				{
					if (m_items[currentItem])
						m_items[currentItem]->unloadDetail();
				}

				if (oldItem > -1 && oldItem < m_itemCount)
				{
					if (m_items[oldItem])
						m_items[oldItem]->unloadDetail();
				}

				// Change mode
				m_animType = ENdhsAnimationTypeDrag;

				// Remove velocity and target value from scrollpos field
				LcTScalar currentPos = (LcTScalar)m_scrollPos->getWrappedVal();

				m_scrollPos->setValue(currentPos, false);
				m_scrollPos->setVelocity(0, 0, 0);

				break;
			}

			// If we're state transitioning, or scrolling, ignore
			default:
				break;
		}
	}
	else
	{
		NdhsCElementGroup::fieldValueUpdated(field);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::startStaticAnimations(EAnimationGroup groupToStart)
{
	// Initialize
	m_stopStaticAnimationAll = false;
	m_stopStaticAnimationItem = false;

	if (!m_outerGroup)
		return false;

	// Clear out any existing statics that are still going
	stopStaticAnimations(true, groupToStart);

	// Query transition agent for any static animations
	if (!m_pStaticTriggerList)
		m_transitionAgent->getStaticTriggers(this);

	if (groupToStart == EAnimationAll)
	{
		// Set up the statics on the page group (this will propagate
		// down through the other furniture groups)
		m_outerGroup->startStaticAnimations();
	}

	// Set up the statics on the individual slots, items and details
	if (m_itemCount > 0)
	{
		CItem* item;
		int offset = wrapToRange(m_windowStart, m_itemCount);

		for (int i = 0; i < m_windowSize; i++)
		{
			item = m_items[offset];
			if(item)
				item->startStaticAnimations();

			offset = wrapToRange(++offset, m_itemCount);
		}
	}

	// If there are any static fields, then we have some static animations to do
	return (m_staticAnimFields.size() != 0);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::stopStaticAnimations(bool immediateStop, EAnimationGroup groupToStop)
{
	// If there are no static animation fields, there's nothing to do.
	if (m_staticAnimFields.size() == 0 && !m_pStaticAnimFieldTrigger && !m_pStaticTriggerList)
		return;

	if (!m_outerGroup)
		return;

	if (immediateStop)
	{
		if (groupToStop == EAnimationAll)
		{
			m_outerGroup->stopStaticAnimations();

			// Update animation states
			if (m_animType == ENdhsAnimationTypeStatic)
				m_animType = ENdhsAnimationTypeNone;
			m_previousAnimType = ENdhsAnimationTypeNone;
		}

		// Stop the static animation on the individual slots, items and details - always done
		if (m_itemCount > 0)
		{
			CItem* item;
			int offset = wrapToRange(m_windowStart, m_itemCount);

			for (int i = 0; i < m_windowSize; i++)
			{
				item = m_items[offset];
				if(item)
					item->stopStaticAnimations();

				offset = wrapToRange(++offset, m_itemCount);
			}
		}

		// Stop the static triggers - always done
		if (m_pStaticAnimFieldTrigger)
		{
			// Call cleanup trigger if required
			if (m_pStaticAnimFieldTrigger->atRest() == false)
			{
				if (m_pStaticTriggerList && m_pStaticTriggerList->hasCleanupTrigger)
					simulateKeyDown(m_pStaticTriggerList->cleanupTrigger);
			}

			releaseStaticAnimationField(m_pStaticAnimFieldTrigger);
			m_pStaticAnimFieldTrigger = NULL;
		}
		m_pStaticTriggerList = NULL;
	}
	else
	{
		if (groupToStop == EAnimationAll)
		{
			m_stopStaticAnimationAll = true;
		}
		else
		{
			// Note that next time animation field wraps, we should terminate
			m_stopStaticAnimationItem = true;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::requestSetActiveItem(int item)
{
	if(!m_menu || isGroupUnloaded())
		return;
	if (m_outerGroup && item >= 0 && item < m_itemCount)
	{
		switch(m_animType)
		{
		case ENdhsAnimationTypeTerminalState:
		case ENdhsAnimationTypeInteractiveState:
		case ENdhsAnimationTypeScroll:
		case ENdhsAnimationTypeScrollKick:
		case ENdhsAnimationTypeStatic:
		{
			// Terminate any static animations
			stopStaticAnimations(true, EAnimationAll);

			jumpTransitionToEnd(false);
			break;
		}

		case ENdhsAnimationTypeDrag:
#ifdef LC_USE_STYLUS
			// Cancel any current drag operation
			m_pageManager->onMouseCancel(NULL, NULL, false);
			m_mouseFocusElement = NULL;
#endif // LC_USE_STYLUS
			break;

		case ENdhsAnimationTypeNone:
		default:
			// Do nothing
			break;
		}

#ifdef IFX_USE_PLUGIN_ELEMENTS
		if (m_currentFocusElement)
		{
			m_previousFocusElement = m_currentFocusElement;
			m_currentFocusElement = NULL;

			m_previousFocusElement->setElementFocus(false);
		}
#endif

		// Check if we're already OK...
		int currentIndex = getActiveItem(ENdhsActiveItemCurrent);
		if (item == currentIndex)
			return;

		if (m_scrollWrap && isWrappable())
		{
			// Wrap active item
			item = wrapToRange(item, m_itemCount);
		}
		else
		{
			// Clamp to range
			if (item > (m_itemCount - 1))
				item = m_itemCount - 1;

			if (item < 0)
				item = 0;
		}

		m_ignoreScrollPosNextItem=true;
		backupFocus();

		doLoadOnDemand(item,currentIndex);

		m_scrollPos->setValue((LcTScalar)(item),true);

		onTransitionComplete(true, false, true);
		restoreFocus();
		m_ignoreScrollPosNextItem=false;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::setVisible(bool b)
{
	// Propagate to base class
	NdhsCComponent::setVisible(b);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT NdhsCField* NdhsCMenuComponent::getFieldValue(const LcTmString& fieldName,
													NdhsCMenu* menu,
													int item,
													NdhsCElement* element)
{
	NdhsCField* retVal = NULL;

	// Special case is the '_scrollpos' field
	if (fieldName.compareNoCase("_scrollPos") == 0)
	{
		retVal = m_scrollPos.ptr();
	}
	else
	{
		if (menu)
		{
			if (fieldName.compareNoCase("_placeholderActive") == 0
				&& (item >= 0) && (item < m_itemCount))
			{
				retVal = m_placeHolderFields[item];
			}
			else
			{
				// Go via the menu where possible
				retVal = menu->getField(fieldName, menu->getMenuItemIndex(item) , element);
			}
		}

		if (retVal == NULL && m_fieldCache)
		{
			retVal = m_fieldCache->getField(fieldName);
		}

		if (retVal == NULL)
		{
			// Otherwise get the global field
			if (getMenu() && getMenu()->getPlugin())
				retVal = getMenu()->getPlugin()->getField(fieldName, NULL, item, element);
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::startTransition(ENdhsAnimationType animType,
									bool animate)
{
	bool willAnimate = true;
	bool bKickAgain = false;

	// If we are not ready no need to go further
	if (isComponentRetired() || !m_outerGroup)
		return false;

	if (m_transitionAgent)
	{
		// Organize detail elements first
		switch(animType)
		{
			case ENdhsAnimationTypeLayoutChange:
			case ENdhsAnimationTypeInteractiveState:
			{
				// Schedule existing detail for destruction, and create the
				// new detail
				if(m_prevActiveSlot != m_activeSlot)
				{
					if (m_activeSlot != -1)
					{
						if (m_slotRefs[m_activeSlot] > -1 && m_slotRefs[m_activeSlot] < m_itemCount)
						{
							if(m_items[m_slotRefs[m_activeSlot]])
								m_items[m_slotRefs[m_activeSlot]]->loadDetail();
						}
					}

					if (m_prevActiveSlot!=-1)
					{
						if (m_slotRefs[m_prevActiveSlot] > -1 && m_slotRefs[m_prevActiveSlot] < m_itemCount)
						{
							if(m_items[m_slotRefs[m_prevActiveSlot]])
								m_items[m_slotRefs[m_prevActiveSlot]]->scheduleUnloadDetail();
						}
					}
				}

				break;
			}

			case ENdhsAnimationTypeTerminalState:
			{
				// May need to load the detail items
				if ((m_activeSlot != -1) && (m_pendingHideStateSlotActive == -1))
				{
					if (m_slotRefs[m_activeSlot] > -1 && m_slotRefs[m_activeSlot] < m_itemCount)
					{
						if(m_items[m_slotRefs[m_activeSlot]])
							m_items[m_slotRefs[m_activeSlot]]->loadDetail();
					}
				}

				break;
			}

			case ENdhsAnimationTypeScroll:
			case ENdhsAnimationTypeScrollKick:
			{
				// Schedule existing detail for destruction, and create the new detail
				// only if we're scrolling by a single item
				if (m_activeSlot != -1)
				{
					backupFocus();

					int currentItem = getActiveItem(ENdhsActiveItemCurrent);
					int targetItem = getActiveItem(ENdhsActiveItemTarget);

					int distance = m_scrollPositive ? targetItem - currentItem : currentItem - targetItem;

					if (m_scrollWrap && isWrappable())
						distance = wrapToRange(distance, m_itemCount);

					if ((targetItem > -1) && (targetItem < m_itemCount) && (distance == 1))
					{
						if (m_items[targetItem])
							m_items[targetItem]->loadDetail();
					}

					if ((currentItem > -1) && (currentItem < m_itemCount))
					{
						if (m_items[currentItem])
							m_items[currentItem]->scheduleUnloadDetail();
					}
				}
				break;
			}

			case ENdhsAnimationTypeDrag:
			{
				backupFocus();

				// Destroy any detail group
				int currentItem = getActiveItem(ENdhsActiveItemCurrent, false);
				int oldItem = getActiveItem(ENdhsActiveItemOld, false);

				if ((currentItem > -1) && (currentItem < m_itemCount))
				{
					if (m_items[currentItem])
						m_items[currentItem]->unloadDetail();
				}

				if ((oldItem > -1) && (oldItem < m_itemCount))
				{
					if (m_items[oldItem])
						m_items[oldItem]->unloadDetail();
				}

				break;
			}

			default:
				break;
		}

		switch(animType)
		{
			case ENdhsAnimationTypeLayoutChange:
			case ENdhsAnimationTypeInteractiveState:
			case ENdhsAnimationTypeTerminalState:
			{
				// Tell any ongoing statics to terminate at the end
				// of their current animation
				if (m_animType == ENdhsAnimationTypeStatic)
					stopStaticAnimations(true, EAnimationAll);
			}
			// Drop through...
			case ENdhsAnimationTypeScroll:
			case ENdhsAnimationTypeScrollKick:
			case ENdhsAnimationTypeDrag:
			{
				// We're about to make a whole bunch of pointers stale, so we need to clear them up first
				resetTransitionCache();

				// Update the starting and ending layout objects.

				m_transitionAgent->prepareTransition(m_stateManager->getCurrentLayout(), m_prevActiveSlot, m_activeSlot);

				// If we're already kick scrolling, note that we don't need to update slots or set
				// the initial position
				if (m_animType == ENdhsAnimationTypeScrollKick)
					bKickAgain = true;

				// Record transition type
				m_previousAnimType = m_animType;
				m_animType = animType;

				{
					int tempBackgroundDelay;
					int tempPrimaryLightDelay;
					int tempPrimaryLightDuration;

					// Get info for the transition to come - note that this may replace duration/delay/profile
					// and bg delay for terminal state transitions if decorations are supported.
					m_transitionAgent->getPageAnimationDetails(this,
															getAnimType(),
															m_transitionDuration,
															m_transitionDelay,
															m_transitionVelProfile,
															tempBackgroundDelay,
															tempPrimaryLightDelay,
															tempPrimaryLightDuration);
				}

				if(animType == ENdhsAnimationTypeLayoutChange)
				{
					animType = ENdhsAnimationTypeInteractiveState;
					m_animType = animType;
				}

				// OK...we've got the timing details, we need to set up the relevant field to control animation
				bool finalUpdate;

				LcTTime timestamp = getSpace()->getTimestamp();

				if (animType == ENdhsAnimationTypeInteractiveState)
				{
					m_layoutPos->setValue(0.0, false);
					m_layoutPos->setTargetValue(1.0, ENdhsFieldDirectionIncreasing, false, m_transitionDuration,
												ENdhsScrollFieldModeNormal, m_transitionVelProfile);

					// We need to cache the timestamp locally...Note that we don't
					// call 'updateValue' here, as we don't want the animator to start
					// until the delay has finished, unless delay is 0.
					if (m_transitionDelay == 0)
					{
						m_layoutPos->updateValue(timestamp, finalUpdate);
					}

					m_decorationDelayTimestamp = timestamp + m_transitionDelay;
					m_stateChangeAnimComplete = false;
				}
#ifdef IFX_USE_PLUGIN_ELEMENTS
				// Clear the value on a non static interactive animation.
				// Clear the focus pointer so messages are not passed down.
				if (m_currentFocusElement)
				{
					m_previousFocusElement = m_currentFocusElement;
					m_currentFocusElement = NULL;

					m_previousFocusElement->setElementFocus(false);
				}
#endif

				if ((m_animType == ENdhsAnimationTypeScroll) || (m_animType == ENdhsAnimationTypeDrag)
					|| (m_animType == ENdhsAnimationTypeScrollKick))
				{
					if (bKickAgain == false)
					{
						updateSlots(m_scrollWrap);

						updatePosition(0.0, m_scrollPositive, true);

						// Call update value once, to initialize the timestamp cache - we must
						// do this after the update slots call in case the scroll takes 0 time.
						if ((m_animType == ENdhsAnimationTypeScroll) || (m_animType == ENdhsAnimationTypeScrollKick))
						{
							m_ignoreScrollPosUpdate = true;
							m_scrollPos->updateValue(timestamp, finalUpdate);
							m_ignoreScrollPosUpdate = false;
						}
					}
				}
				else
				{
					updatePosition(0.0, true, true);
				}

#ifdef IFX_GENERATE_SCRIPTS

		// Inform script generator that we need screen capture events in transition.
		if (NdhsCScriptGenerator::getInstance())
			NdhsCScriptGenerator::getInstance()->setTransitionCaptureEvents(timestamp, m_transitionDelay,
																			m_transitionDuration);
#endif //IFX_GENERATE_SCRIPTS

				updateMetaFields();
				break;
			}

			case ENdhsAnimationTypeStatic:
			{
				// Set up the static animations...note that if we've recently
				// finished a scroll, it's only the slots/items/detail that will
				// start a static animation (although the furniture/page/menu might
				// still have an active static ongoing...)
				if (startStaticAnimations(m_staticAnimGroup))
					m_animType = ENdhsAnimationTypeStatic;

				break;
			}

			default:
				break;
		}

	}

	if ((m_animType != ENdhsAnimationTypeStatic) && (m_transitionDuration == 0))
		willAnimate = false;

	if (willAnimate)
		m_pageManager->startAnimation();

	return willAnimate;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenuComponent::changeState(ENdhsPageState state, int activeSlot, const NdhsCTemplate::CAction::CAttempt* attempt, bool animate)
{
	ENdhsAnimationType animType = ENdhsAnimationTypeInteractiveState;
	ENdhsVelocityProfile velocityProfile = ENdhsVelocityProfileUnknown;
	int duration = -1;
	int delay = -1;
	int newActiveItem = -1;
	bool willAnimate = false;

	// If we are not ready no need to go further
	if (isComponentRetired() || !m_outerGroup)
		return false;

	// Do not do any post-transition complete processing
	if(m_bRequiresPostTransitionComplete)
		m_bRequiresPostTransitionComplete = false;

	// Do we need to interrupt a transition?
	switch (m_animType)
	{
		// No animation, or scrolling - in the latter case, a decision
		// to interrupt transitions is taken in processTrigger
		case ENdhsAnimationTypeScroll:
		case ENdhsAnimationTypeDrag:
		case ENdhsAnimationTypeScrollKick:
		case ENdhsAnimationTypeStatic:
		case ENdhsAnimationTypeNone:
			break;

		// All other transitions
		default:
			jumpTransitionToEnd(false);
			break;
	}

	// Check if slot reassignment is required
	if (attempt)
	{
		switch (attempt->attemptType)
		{
			case NdhsCMenuComponentTemplate::CAction::CAttempt::EScrollBy:
			{
				NdhsCMenuComponentTemplate::CAction::CScrollBy* scrollAttempt = (NdhsCMenuComponentTemplate::CAction::CScrollBy*) attempt;

				if (scrollItems(scrollAttempt))
				{
					if (scrollAttempt->type == NdhsCMenuComponentTemplate::CAction::CScrollBy::EScrollByTypeKick)
						animType = ENdhsAnimationTypeScrollKick;
					else
						animType = ENdhsAnimationTypeScroll;

					velocityProfile = scrollAttempt->velocityProfile;
					duration = scrollAttempt->duration;
				}
				else
				{
					// Scroll has failed
					return false;
				}

				break;
			}

			case NdhsCMenuComponentTemplate::CAction::CAttempt::EJumpTo:
			{
				newActiveItem = getRefFromSlot(activeSlot);
				velocityProfile = ((NdhsCMenuComponentTemplate::CMenuAction::CJumpTo*)attempt)->velocityProfile;
				duration = ((NdhsCMenuComponentTemplate::CMenuAction::CJumpTo*)attempt)->duration;

				break;
			}

			case NdhsCMenuComponentTemplate::CAction::CAttempt::EJumpBy:
			{
				newActiveItem = getRefFromSlot(activeSlot);
				velocityProfile = ((NdhsCMenuComponentTemplate::CMenuAction::CJumpBy*)attempt)->velocityProfile;
				duration = ((NdhsCMenuComponentTemplate::CMenuAction::CJumpBy*)attempt)->duration;

				break;
			}

			case NdhsCMenuComponentTemplate::CAction::CAttempt::EDeactivate:
			{
				velocityProfile = ((NdhsCMenuComponentTemplate::CMenuAction::CDeactivate*)attempt)->velocityProfile;
				duration = ((NdhsCMenuComponentTemplate::CMenuAction::CDeactivate*)attempt)->duration;

				break;
			}

			default:
				break;
		}
	}

	m_transitionAgent->ovverideTransitionDetails(velocityProfile, duration, delay);

	if (state == ENdhsPageStateOpen)
	{
		animType = ENdhsAnimationTypeTerminalState;
	}

	// Setup the new active slot
	m_prevActiveSlot = m_activeSlot;

	// Staying in the interactive layout,
	// so used the supplied value
	setActiveSlot(activeSlot);

	// Reset the static animation flag
	m_staticTransitionDone = false;

	// Check if we are doing the static animations
	if (state == ENdhsPageStateNone
		&& (m_prevActiveSlot == m_activeSlot)
		&& (animType != ENdhsAnimationTypeScroll)
		&& (animType != ENdhsAnimationTypeDrag)
		&& (animType != ENdhsAnimationTypeScrollKick))
	{
		animType = ENdhsAnimationTypeStatic;

		if (animate == true)
			m_staticTransitionDone = true;
	}

	// Note that we should postpone any layout changes until after we've completed the start transition call
	m_preparingTransition = true;

	m_transitionDuration = duration;
	m_transitionDelay = delay;
	m_transitionVelProfile = velocityProfile;

	// Note the special case where we're kicking again...in that case, we're
	// already transitioning, and should not call startTransition again.
	if ((animType != ENdhsAnimationTypeScrollKick) || (m_animType != ENdhsAnimationTypeScrollKick))
		willAnimate = startTransition(animType, animate);
	else
		willAnimate = true;

	// Set the scroll pos value
	if (newActiveItem != -1)
	{
		m_ignoreScrollPosUpdate = true;
		m_scrollPos->jumpValueTo(newActiveItem);
		m_ignoreScrollPosUpdate = false;
		m_lastActiveItemScrollPos->jumpValueTo(newActiveItem);
		// Update any metafields
		updateMetaFields();
	}

	// If no animators have started, the transition
	// must be complete
	if (!willAnimate)
	{
		onTransitionComplete(false, true, true);
	}
#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
	else
	{
		if (m_isEligible == false)
		{
			m_isEligible = true;
			getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_STARTED);
		}
	}
#endif

	// Propagate any changes immediately
	m_pageManager->getCon()->getLaundry()->cleanAll();

	// Now reset the ovveriden transition details
	m_transitionAgent->resetOvveridenTransitionDetails();

	// Now check for any pending layout change
	m_preparingTransition = false;
	if (m_pendingLayout)
	{
		// This should set up the new transition, cancelling the current if necessary
		layoutChanged(m_pendingLayout, m_pendingLayoutAnimateFlag);
		m_pendingLayout = NULL;
	}

	// We tell the caller if we're in a terminal state transition or not
	return (m_animType == ENdhsAnimationTypeTerminalState);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::jumpTransitionToEnd(bool setIdle)
{
	m_jumpingToEnd = true;
	if (!m_outerGroup)
		return;

#ifdef LC_USE_STYLUS
	if (setIdle)
	{
		// Cancel any current drag operation
		m_pageManager->onMouseCancel(this, NULL, false);
		m_mouseFocusElement = NULL;
	}
#endif

	// Execute cleanup trigger if necessary
	if ((m_animType != ENdhsAnimationTypeNone) && (m_animType != ENdhsAnimationTypeDrag))
	{
		if (m_pAnimTriggerList && m_pAnimTriggerList->hasCleanupTrigger)
		{
			// Let this happen even when setIdle is true.  It could start
			// a transition, but the cleanup trigger was intended for
			// sending a sync link to a plugin to tell it to cleanup

			// Do this trigger
			simulateKeyDown(m_pAnimTriggerList->cleanupTrigger);
		}

		m_pAnimTriggerList = NULL;
	}

	// Interrupt - Jump any previous transition to the end
	switch (m_animType)
	{
		case ENdhsAnimationTypeDrag:
		{
			// If we get here, we are interrupting the transition
			// so the chained action will not execute
			m_chainedAction = "";

			// Now...a drag scroll should snap to the nearest active item
			// that might be the one at the start of the transition, or the
			// end.  Let's have a look
			LcTScalar lastActiveItemVal = (LcTScalar)m_lastActiveItemScrollPos->getWrappedVal();
			int targetVal = m_scrollPositive ? (int)(lastActiveItemVal + m_scrollSpan) : (int)(lastActiveItemVal - m_scrollSpan);

			if (m_scrollWrap && isWrappable())
			{
				targetVal = wrapToRange(targetVal, m_itemCount);
			}
			else
			{
				if (targetVal < (int)m_scrollPos->getMinAllowableVal())
					targetVal = (int)m_scrollPos->getMinAllowableVal();
				else if (targetVal > (int)m_scrollPos->getMaxAllowableVal())
					targetVal  = (int)m_scrollPos->getMaxAllowableVal();
			}

			int activeItem = nearOriginSlot() ? (int)lastActiveItemVal : targetVal;

			// Set the scrollpos value to the active item
			m_ignoreScrollPosUpdate = true;
			m_scrollPos->jumpValueTo(activeItem);
			m_ignoreScrollPosUpdate = false;

			// Now do the 'on transition complete' step.
			onTransitionComplete(setIdle);

			// Finally update the last active item pos
			m_lastActiveItemScrollPos->jumpValueTo(activeItem);

			// Update any metafields
			updateMetaFields();
			break;
		}

		case ENdhsAnimationTypeScrollKick:
		case ENdhsAnimationTypeScroll:
		{
			// If we get here, we are interrupting the transition
			// so the chained action will not execute
			m_chainedAction = "";

			// Get target active item value
			int activeItem = getActiveItem(ENdhsActiveItemTarget);
			bool forceSlotUpdate = false;

			// Check that we aren't jumping multiple items - if so, we'll need to
			// update the item-slot assignment in onTransitionComplete
			if (activeItem != getActiveItem(ENdhsActiveItemCurrent))
				forceSlotUpdate = true;

			// Set the scrollpos value to the active item
			m_ignoreScrollPosUpdate = true;
			m_scrollPos->jumpValueTo(activeItem);
			m_ignoreScrollPosUpdate = false;

			// Now do the 'on transition complete' step.
			onTransitionComplete(setIdle, false, forceSlotUpdate);

			// Finally update the last active item pos
			m_lastActiveItemScrollPos->jumpValueTo(activeItem);

			// Update any metafields
			updateMetaFields();
			break;
		}

		case ENdhsAnimationTypeInteractiveState:
		{
			// Not reseting chained action, because we
			// will be here during layout change and will
			// result in reseting chained action

			// Set the layout field to 'end'
			m_layoutPos->setValue(1.0, true);

			// Now do the 'on transition complete' step.
			onTransitionComplete(setIdle);

			break;
		}

		case ENdhsAnimationTypeStatic:
		{
			if (setIdle)
			{
				// Need to stop any static decorations
				stopStaticAnimations(true, EAnimationAll);
			}

			break;
		}

		default:
			break;
	}

	m_jumpingToEnd = false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::scheduleReplacePlaceholders()
{
	if (!m_replacePlaceholdersMessage.isScheduled())
		m_replacePlaceholdersMessage.schedule(getSpace()->getTimer(), 0, 0);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::onTransitionComplete(bool setIdle, bool generateSignal, bool forceSlotUpdate)
{
	bool notifyActiveItemChange = false;

	// Broadcast the update to the scroll pos field here if necessary
	if (m_scrollPosIntrinsicStale)
	{
		// set the current scroll position value
		NdhsCField* themefield = m_fieldCache->getField("_scrollpos");
		if (themefield)
		{
			themefield->setValue((LcTScalar)m_scrollPos->getWrappedVal(), true);
		}

		m_scrollPosIntrinsicStale = false;
	}

	// If outer group not created not need to go further
	if (!m_outerGroup)
		return;

#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
	if (m_isEligible == true && generateSignal)
	{
		m_isEligible = false;
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_COMPLETE);
	}
#endif

#ifdef IFX_GENERATE_SCRIPTS
	if (!m_bRequiresPostTransitionComplete)
		if (NdhsCScriptGenerator::getInstance())
				NdhsCScriptGenerator::getInstance()->cancelCaptureMove();
#endif

	// Stop static animation...the group affected depends on
	// whether we're stopping a scroll or a state transition
	// If setting idle, just stop the lot
	if (!setIdle && (m_animType == ENdhsAnimationTypeScroll || m_animType == ENdhsAnimationTypeDrag
			|| m_animType == ENdhsAnimationTypeScrollKick))
		m_staticAnimGroup = EAnimationItemOnly;
	else
		m_staticAnimGroup = EAnimationAll;

	stopStaticAnimations(true, m_staticAnimGroup);

	m_stateChangeAnimComplete = true;
	m_lastFrame = false;

	// Position the page and menu in the final location
	switch (m_animType)
	{
		case ENdhsAnimationTypeScrollKick:
		case ENdhsAnimationTypeScroll:
		case ENdhsAnimationTypeDrag:
		{
			if (m_menu != NULL)
			{
				// Let's have a look to see if we position at the start or end
				// of the animation
				LcTScalar animFrac = 1;

				if (m_scrollPos->getWrapAdjustedVal() == m_lastActiveItemScrollPos->getWrapAdjustedVal())
				{
					animFrac = 0; // the active item is the start of the transition
				}

				int activeItem = getActiveItem(ENdhsActiveItemCurrent);

				// Any LOD required?  Note that if the anim frac is < 1 there's no need
				// to update the item window
				if (animFrac > 0)
					doLoadOnDemand(activeItem);

				// A jump to end may mean that the item-slot assignment is currently stale
				if (forceSlotUpdate)
					updateSlots(m_scrollWrap);

				// Position the elements correctly - final frame!  If we've updated slots, force a cache update too
				updatePosition(animFrac, m_scrollPositive, forceSlotUpdate, true);

				m_animType = (m_previousAnimType == ENdhsAnimationTypeStatic) ? ENdhsAnimationTypeStatic : ENdhsAnimationTypeNone;
				m_previousAnimType = ENdhsAnimationTypeNone;

				if (animFrac < 1)
				{
					// The slots - items array will be stale...so update it
					updateSlots(m_scrollWrap);
				}

				// Ensure that the active item detail is loaded, if necessary
				if ((m_activeSlot != -1) && (activeItem > -1) && (activeItem < m_itemCount))
				{
					if (m_items[activeItem])
					{
						m_items[activeItem]->loadDetail();

						// Position the detail element
						m_items[activeItem]->updateDetailPosition(ENdhsAnimationTypeScroll, animFrac, m_scrollPositive,
														true, true, false, true);
					}
				}

				// Make sure scroll pos is integral
				m_ignoreScrollPosUpdate = true;
				m_scrollPos->jumpValueTo(activeItem);
				m_ignoreScrollPosUpdate = false;

				// Update our cached value
				m_lastActiveItemScrollPos->jumpValueTo(activeItem);
				// metafields updated at end of function...

				// Allow any static animation to be started
				m_staticTransitionDone = false;

				// Always notify modules of any change to the active item
				notifyActiveItemChange = true;
				m_scrollUpdateType = ENdhsScrollUpdateNormal;
			}
			break;
		}

		case ENdhsAnimationTypeTerminalState:
		case ENdhsAnimationTypeInteractiveState:
		{
			notifyActiveItemChange = true;

			// Final frame!
			updatePosition(1.0, true, false, true);
			m_animType = ENdhsAnimationTypeNone;

			break;
		}

		default:
		{
			m_animType = ENdhsAnimationTypeNone;

			break;
		}
	}

	// Make sure we are back in a decent state before replacing placeholders
	// with actual graphics
	if (m_placeholderUsed && (m_jumpingToEnd == false))
	{
		scheduleReplacePlaceholders();
	}

	// make sure we are back in a decent state before
	// performing the delayed page refresh
	if (!setIdle && m_refreshNeeded && getMenu() != NULL)
	{
		// Just trigger an immediate refresh
		NdhsCPlugin::NdhsCPluginMenu* pluginMenu = getMenu()->getMenuPlugin();
		if (pluginMenu != NULL)
		{
			// Must be scheduled via the page manager to make sure that
			// it comes on a fresh execution path.  This function is
			// ultimately called by a widget, and we are about to destroy
			// the widgets
			m_pageManager->refreshPage(pluginMenu->getMenuSession(), true);
		}
	}
	// Check to see if there is a chained action, or a static animation that might need
	// to be started, or a focus element that needs updating.
	else if (!setIdle
		&& ((!m_chainedAction.isEmpty())
		|| ((m_staticTransitionDone == false) && (m_jumpingToEnd == false))
#ifdef IFX_USE_PLUGIN_ELEMENTS
		|| (m_currentFocusElement == NULL)
#endif
		))
	{
		// Ok...we need to do some more actions
		m_bRequiresPostTransitionComplete = true;

		// Request that the Page Manager schedule the post-TC event
		schedulePostTransitionComplete();
	}

	// Check for any active item update, and report it to the menu
	if (notifyActiveItemChange && m_menu)
	{
		int activeItem = -1;

		if (m_activeSlot != -1)
		{
			activeItem = getActiveItem(ENdhsActiveItemCurrent);
		}

		if (activeItem != m_lastReportedActiveItem)
		{
			m_lastReportedActiveItem = activeItem;
			m_menu->activeItemUpdated(activeItem);
		}

		updateMetaFields();
	}
}

void NdhsCMenuComponent::scheduleReloadDataSource()
{
	if(m_reloadDataSourceMessage.isScheduled())
	{
		m_reloadDataSourceMessage.cancel();
	}
	m_reloadDataSourceMessage.schedule( getSpace()->getTimer(), 0, 0);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::onMessage(int iID, int iParam)
{
	NdhsCComponent::onMessage(iID,iParam);	// Messages for parent

	switch (iID)
	{
		case EReplacePlaceholdersMsg:
		{
			replacePlaceholders();
			break;
		}
		case EReloadDataSource:
		{
			retire();
			loadDataSource();
			reloadElement();
			onResume();
			realize(getOwner());
			if(!isGroupUnloaded())
			{
				// The positioning code requires some sort of transition, so
				// we set up a 'dummy' terminal state transition to the current
				// state, taking no time.
				changeState(ENdhsPageStateOpen, m_firstActiveSlot, NULL, false);
				changeState(ENdhsPageStateInteractive, m_firstActiveSlot, NULL, false);

				// Complete the dummy state transition to correctly position all the
				// elements
				onTransitionComplete(false);

				if (m_menu)
				{
					NdhsCField* field = m_fieldCache->getField("_dataSourceLoaded");
					if (field)
					{
						field->setValueBool(true, true);
					}
				}
			}

			break;
		}

		default:
			break;
	}
}


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::postTransitionComplete()
{
	// Only do anything if we've recorded that we need to...
	// And we have valid field cache (can be invalid in case
	// of component relaoding/unloading)
	if (!m_bRequiresPostTransitionComplete || !m_fieldCache)
		return;

	volatile int memError = 0;

	m_isLayoutChange = false;

	NdhsCField* field = m_fieldCache->getField("_transitioning");
	if (field)
	{
		field->setValueBool(false, true);
	}

	// Note we no longer need to perform any actions.
	m_bRequiresPostTransitionComplete = false;

	// Prepare cleanup frame
	LC_CLEANUP_PUSH_FRAME(memError);

	if (memError == 0)
	{
		// Now do the chained action
		if (!m_chainedAction.isEmpty())
		{
			executeChainedAction(m_chainedAction, m_chainedActionSlot);
		}

		// Only carry on if a chained action didn't start a transition, or schedule another post transition complete
		// message (as it would if the chained action duration was 0ms)
		if (!m_bRequiresPostTransitionComplete && ((m_animType == ENdhsAnimationTypeNone) || (m_animType == ENdhsAnimationTypeStatic)))
		{
			if ((m_staticTransitionDone == false) && (m_jumpingToEnd == false))
			{
				// Start the static transition, but never when the transition completes
				// because we jumped to the end of a transition
				(void)changeState(ENdhsPageStateNone, m_activeSlot, NULL, true);
			}

			restoreFocus();
		}
	}
	else
	{
		// This should not be reached, as the memory needed should have been factored in
		// to the memory headroom calculation
		LC_ASSERT(false);

		// State may be partially changed, so dangerous to continue
		LC_CLEANUP_THROW(IFX_ERROR_RESTART);
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memError);
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::onMouseUp(NdhsCPageManager::TPageWidgetElem* entry, const LcTPixelPoint& pt)
{
	// We must ask the element for the widget because a
	// field refresh might have changed the original
	// widget pointer

	bool bConsumed = false;
	LcCWidget* widget = NULL;
	bool triggersAllowed = true;
	bool widgetHit = false;

	m_bMouseDown = false;

	if (!m_outerGroup)
		return false;

	if (entry && entry->element)
	{
		widget = entry->element->getWidget();

		// Check to see if widget should handle mouse up - do this before
		// notifying the element of the mouse up in case there's a touchdown
		// displacement.
		if (widget && getSpace())
		{
			LcTVector loc;

			// Get the local co-ordinates.
			loc = getSpace()->mapCanvasToLocal(pt, *(widget));

			LcTScalar expandBorder = widget->getTapTolerance();

			// Check to see if this point is within the widget.
			if (widget->contains(loc, expandBorder))
			{
#if defined(LC_PLAT_OGL_20)
				// Calculate the tap position.
				widget->calcTapPosition(pt);
#endif
				widgetHit = true;
			}
		}

		bConsumed = entry->element->onMouseUp(pt);

		// Do not call process triggers if the element is untappable
		triggersAllowed = entry->element->isTappable();
	}

	// The element did not consume the event, try the page
	if (!bConsumed && triggersAllowed)
	{
		int selectedSlot = -1;
		LcTaString elementClass;

		// Check that the widget is valid, and that we have a space.
		if (widget && widgetHit)
		{
			// check furniture first for selected widget
			elementClass = m_outerGroup->getClassFromWidget(widget);

			// check slot and detail items if not previously found in furniture
			if (elementClass.length() == 0)
				getSlotAndClassFromWidget(widget, elementClass, &selectedSlot);
		}

		// This is executed even if selectedSlot and elementClass are not populated.
		// The may execute the catch all case if necessary
		bConsumed = processTrigger(ENdhsNavigationStylusTap, selectedSlot, elementClass, NULL, false);
	}

	if (m_mouseFocusElement)
		m_mouseFocusElement = NULL;

	updateTouchDownIntrinsicField("", -1);

	return bConsumed;
}
#endif // LC_USE_STYLUS

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::refreshMenuFirstStep()
{
	if(!m_outerGroup || isGroupUnloaded())
		return;

	// Do not do any post-transition complete processing if pending
	// wait for the page refresh process to schedule anew if necessary.
	if (m_bRequiresPostTransitionComplete)
		m_bRequiresPostTransitionComplete = false;

	// If we dont have a menu, no need to refresh current menu component
	if (getMenu() == NULL)
		return;

#ifdef LC_USE_STYLUS
	// Cancel any current drag operation
	m_pageManager->onMouseCancel(this, NULL, false);
	m_mouseFocusElement = NULL;
#endif

	// make sure that onTransitionComplete
	// does not start any new transitions
	m_refreshNeeded = false;
	m_chainedAction = "";
	m_placeholderUsed = false;
	m_staticTransitionDone = true;

	// We must be in a static state to reload a menu
	m_pageManager->jumpTransitionToEnd();
	jumpTransitionToEnd(false);

	// Terminate any static animations
	stopStaticAnimations(true, EAnimationAll);

	m_staticTransitionDone = false;

	backupFocus();

	getMenuAggregate()->retire();

	// For safety, lets delete the items first
	// because they have pointers to the menu
	// items we are about to delete
	deleteItems();

	m_itemCount=0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::refreshMenuSecondStep()
{
	if(!m_outerGroup || isGroupUnloaded())
		return;

	// If we dont have a menu, no need to refresh current menu component
	if (getMenu() == NULL)
		return;

	m_menu->setLODRequired(m_slotCount);

	// set _itemCount field on refresh
	NdhsCField* field = m_fieldCache->getField("_itemCount");
	if (field)
	{
		field->setValue((LcTScalar)m_menu->getItemCount(), true);
	}

	// Now recreate the menu
	reloadComponentForMenu();
	reloadMenuAggregate(false);
	reloadItems();

	// Let menu child elements acquire resources
	TmAItems::iterator it = m_items.begin();
	for (; it != m_items.end(); it++)
	{
		if((*it)!=NULL)
			(*it)->onResume();
	}
	getMenuAggregate()->onResume();

	getMenuAggregate()->realize(m_outerGroup.ptr());
	restoreFocus();

	// The positioning code requires some sort of transition, so
	// we set up a 'dummy' terminal state transition to the current
	// state, taking no time.
	changeState(ENdhsPageStateOpen, m_firstActiveSlot, NULL, false);
	changeState(ENdhsPageStateInteractive, m_firstActiveSlot, NULL, false);

	// Complete the dummy state transition to correctly position all the
	// elements
	onTransitionComplete(false);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::refreshPage(IFX_HMENU hMenu, bool immediate)
{
	if(isGroupUnloaded() || !m_outerGroup)
		return;

	// Do not do any post-transition complete processing if pending -
	// wait for the page refresh process to schedule anew if necessary.
	if (m_bRequiresPostTransitionComplete)
		m_bRequiresPostTransitionComplete = false;

	// If we dont have a menu, no need to refresh current menu component
	if (getMenu() == NULL)
		return;

	// Return if the menu context is NULL
	// or it is not our menu context, or
	// its not a plugin menu
	if (hMenu == NULL)
	{
#if !defined( NDHS_PREVIEWER ) && !defined( NDHS_JNI_INTERFACE )
		return;
#endif
	}
	else
	{
		NdhsCPlugin::NdhsCPluginMenu* pluginMenu = getMenu()->getMenuPlugin();
		if (pluginMenu == NULL || pluginMenu->getMenuSession() != hMenu)
			return;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::doAction(NdhsCTemplate::CAction* action, int slotNum)
{
	// Only cancel any existing chained action if we have something to do
	m_chainedAction = "";

	// Now we have one or more things to attempt: if the first isn't
	// possible, we fall back to the second, and so forth until
	// either we have found something we can do or we have run out
	// of options
	bool attemptOk = false;

	typedef LcTmArray<NdhsCTemplate::CAction::CAttempt*> TmAAttempts;
	TmAAttempts::iterator itA = action->attempts.begin();

	for (; itA != action->attempts.end() && !attemptOk; itA++)
	{
		attemptOk = doAttempt(*itA, slotNum);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponent::doAttempt(NdhsCTemplate::CAction::CAttempt* attemp, int slotNum)
{
	if (attemp == NULL)
		return false;

	bool attemptOk = false;

	if (!m_outerGroup)
		return false;

	// What we do depends on attempt type
	switch (attemp->attemptType)
	{
		// EScrollBy shifts field by the specified amount
		//
		//    amount : offset to change field by
		//    wrap : whether to 'wrap' between min and max value
		case NdhsCMenuComponentTemplate::CAction::CAttempt::EScrollBy:
		{
			NdhsCMenuComponentTemplate::CAction::CScrollBy* scrollAction = (NdhsCMenuComponentTemplate::CAction::CScrollBy*)(attemp);
			bool isKick = (scrollAction->type == NdhsCMenuComponentTemplate::CAction::CScrollBy::EScrollByTypeKick);
			ENdhsFieldDirection dir = (scrollAction->amount > 0) ? ENdhsFieldDirectionIncreasing : ENdhsFieldDirectionDecreasing;

			// nothing to do if there are no items
			if(m_itemCount <=0)
				break;

			// We may be scrolling the scroll pos field, or any other field
			if (scrollAction->field.compareNoCase("_scrollpos") == 0)
			{
				// Check on any current transitions
				switch (m_animType)
				{
					case ENdhsAnimationTypeScroll:
					case ENdhsAnimationTypeScrollKick:
					case ENdhsAnimationTypeDrag:
					{
						// Only interrupt a transition if menu is not in kick-scrolling mode
						if ((m_animType != ENdhsAnimationTypeScrollKick) ||
									(scrollAction->type == NdhsCMenuComponentTemplate::CAction::CScrollBy::EScrollByTypeNormal))
							jumpTransitionToEnd(false);
						break;
					}

					// No animation
					case ENdhsAnimationTypeNone:
						break;

					default:
						jumpTransitionToEnd(false);
						break;
				}

				int amount = (int)scrollAction->amount * m_scrollSpan;

				// Optional initial jump
				bool initialJump = false;
				LcTaOwner<NdhsCMenuComponentTemplate::CMenuAction::CJumpBy> jumpAttempt = NdhsCMenuComponentTemplate::CMenuAction::CJumpBy::create();

				// Is scrolling permitted?
				// Check state - note that module-generated scrolls are permitted in the terminal state
				bool canScroll = true;

				// Is there anything to do?
				if ((amount == 0) || (m_itemCount == 0))
					canScroll = false;

				// We can only scroll in the inactive state if we're in 'full' mode
				if ((m_activeSlot == -1) && !m_fullMode)
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleGeneral, "Scroll by command failed for element: " + getGroupName() + " menu component must either have an active item, or be in 'full' mode.");
					canScroll = false;
				}

				if (canScroll)
				{
					attemptOk = true;

					bool wrap = m_scrollWrap && isWrappable();
					if (!wrap || getSelectableSlotCount() > m_itemCount)
					{
						// See if we can set a new ref
						int newRefActive;
						int lastItem = m_itemCount - 1;
						int firstItem = 0;
						int currActiveItem = getActiveItem(ENdhsActiveItemCurrent);

						newRefActive = getActiveItem(ENdhsActiveItemCurrent) + amount;

						if (m_scrollSpan == 1)
						{
							// Do not leave the active slot empty!
							if ((m_activeSlot != -1) && ((newRefActive < 0) || (newRefActive > m_itemCount - 1)))
								attemptOk = false;

							// If menu is in 'full' mode, do not allow any scroll that would leave a
							// selectable slot empty (unless the span > 1)
							if (m_fullMode)
							{
								if (m_activeSlot != -1)
								{
									lastItem = m_itemCount - 1 - m_lastSelectableSlot + m_activeSlot;
									firstItem = m_activeSlot - m_firstSelectableSlot;
								}
								else
								{
									lastItem = m_itemCount - 1 - m_lastSelectableSlot + m_firstSelectableSlot;
									firstItem = 0;
								}
							}

							if (m_fullMode && (m_activeSlot != -1) &&
															(getSelectableSlotCount() > m_itemCount))
							{
								// lastItem and firstItem are meaningless in this case.  We want to
								// allow scrolling within the selectable slot range provided that the
								// active slot is filled (checked above) and that no items leave the
								// selectable slot range.
								if ((newRefActive - m_activeSlot + m_firstSelectableSlot > 0)
									|| (newRefActive + m_lastSelectableSlot - m_activeSlot - m_itemCount > 0))
									attemptOk = false;
							}
							else
							{
								// Scroll should drop through if we're on the start or end row
								if ((currActiveItem == firstItem && dir == ENdhsFieldDirectionDecreasing)
									|| (currActiveItem == lastItem && dir == ENdhsFieldDirectionIncreasing))
									attemptOk = false;
							}
						}
						else if (m_scrollSpan > 1)
						{
							// Now check for grid behavior
							int validScrollRowsUp;
							int validScrollRowsDown;

							// If menu is in 'full' mode, do not allow any scroll that would leave a
							// whole row of selectable slots empty
							if (m_fullMode)
							{
								if (m_itemCount > getSelectableSlotCount())
								{
									int assumedFirstRowFirstSlot = m_firstSelectableSlot;
									int assumedLastRowFirstSlot = m_lastSelectableSlot - (m_scrollSpan - 1);
									int firstRowFirstSlotCurrItem;
									int lastRowFirstSlotCurrItem;

									if (m_activeSlot != -1)
									{
										firstRowFirstSlotCurrItem = currActiveItem - (m_activeSlot - assumedFirstRowFirstSlot);
										lastRowFirstSlotCurrItem = currActiveItem + (assumedLastRowFirstSlot - m_activeSlot);
									}
									else
									{
										int activeSlot = getInactivePopulateSlot();

										firstRowFirstSlotCurrItem = currActiveItem - (activeSlot - assumedFirstRowFirstSlot);
										lastRowFirstSlotCurrItem = currActiveItem + (assumedLastRowFirstSlot - activeSlot);
									}

									validScrollRowsUp = ((firstRowFirstSlotCurrItem - 1)/m_scrollSpan + 1);
									validScrollRowsDown = ((m_itemCount - 1 - lastRowFirstSlotCurrItem)/m_scrollSpan);
								}
								else
								{
									int slot = m_activeSlot;
									// In this case, we can't keep all selectable slots full, so we keep as many as we can full
									if (slot == -1)
									{
										slot = getInactivePopulateSlot();
										validScrollRowsDown = (slot-m_firstSelectableSlot - currActiveItem)/m_scrollSpan;
										validScrollRowsUp = (m_lastSelectableSlot - slot)/m_scrollSpan -(m_itemCount - 1 - currActiveItem - ((m_itemCount - 1)%m_scrollSpan - currActiveItem%m_scrollSpan))/m_scrollSpan;
									}
									else
									{
										validScrollRowsDown = min((m_itemCount-1-currActiveItem)/m_scrollSpan, (slot-m_firstSelectableSlot - currActiveItem)/m_scrollSpan);
										validScrollRowsUp = min(currActiveItem/m_scrollSpan, (m_lastSelectableSlot - slot)/m_scrollSpan -(m_itemCount - 1 - currActiveItem - ((m_itemCount - 1)%m_scrollSpan - currActiveItem%m_scrollSpan))/m_scrollSpan);
									}
								}
							}
							else
							{
								int activeSlot = m_activeSlot;

								if (activeSlot == -1)
								{
									activeSlot = getInactivePopulateSlot();
									validScrollRowsDown = (activeSlot-m_firstSelectableSlot - currActiveItem)/m_scrollSpan;
									validScrollRowsUp = (m_lastSelectableSlot - activeSlot)/m_scrollSpan -(m_itemCount - 1 - currActiveItem - ((m_itemCount - 1)%m_scrollSpan - currActiveItem%m_scrollSpan))/m_scrollSpan;
								}
								else
								{
									// Don't leave current row empty
									int currRowFirstSlotCurrItem = currActiveItem - (activeSlot % m_scrollSpan);

									validScrollRowsUp = ((currRowFirstSlotCurrItem - 1)/m_scrollSpan + 1);
									validScrollRowsDown = ((m_itemCount - 1 - currRowFirstSlotCurrItem)/m_scrollSpan);
								}
							}

							firstItem = currActiveItem - (validScrollRowsUp * m_scrollSpan);
							lastItem = currActiveItem + (validScrollRowsDown * m_scrollSpan);

							if (m_activeSlot != -1)
							{
								if (firstItem < 0)
									firstItem = 0;
								if (lastItem > m_itemCount - 1)
									lastItem = m_itemCount - 1;

								if (newRefActive < firstItem || newRefActive > lastItem)
								{
									// attempt may not be OK
									int currentCol;
									if (m_activeSlot != -1)
										currentCol = m_activeSlot % m_scrollSpan;
									else
										currentCol = m_firstSelectableSlot % m_scrollSpan;
									int maxCol = m_scrollSpan - 1;
									int startOfCurrentRowItem = getActiveItem(ENdhsActiveItemCurrent) - currentCol;
									int startOfDestRowItem = startOfCurrentRowItem + amount;
									int endOfDestRowItem = startOfDestRowItem + maxCol;

									if (endOfDestRowItem < firstItem || startOfDestRowItem > lastItem)
									{
										// Trying to scroll to an empty row
										attemptOk = false;
									}
									else
									{
										// Destination item is currently empty, so snap to first or last item column
										// (depending on scroll direction) before scrolling
										int newCol = currentCol;
										if (amount < 0)
										{
											// Scrolling up, snap to first item's column
											newCol = maxCol - endOfDestRowItem;
										}
										else if (amount > 0)
										{
											// Scrolling down, snap to last item's column
											newCol = lastItem - startOfDestRowItem;
										}

										if ((currActiveItem + (newCol - currentCol)) < 0
											|| (currActiveItem + (newCol - currentCol)) > m_itemCount - 1)
										{
											// The jumpby would cause a glitch, so lets disallow this scroll
											attemptOk = false;
										}
										else
										{
											// Set up jump to new column
											initialJump = true;
											jumpAttempt->attemptType = NdhsCMenuComponentTemplate::CAction::CAttempt::EJumpBy;
											jumpAttempt->amount = newCol - currentCol;
											jumpAttempt->wrap = false;
											jumpAttempt->action	= "";
										}
									}
								}
							}
							else
							{
								// Scroll would leave an empty row
								if (newRefActive < firstItem || newRefActive > lastItem)
								{
									attemptOk = false;
								}
							}
						}
					}

					if (attemptOk)
					{
						// Interrupt any inter-page animations
						m_pageManager->jumpTransitionToEnd();

						if (initialJump)
						{
							(void)changeState(ENdhsPageStateInteractive, m_activeSlot + jumpAttempt->amount, (NdhsCTemplate::CAction::CAttempt*)jumpAttempt.ptr(), false);
						}

						m_chainedAction = scrollAction->action;

						(void)changeState(ENdhsPageStateInteractive, m_activeSlot, (NdhsCTemplate::CAction::CAttempt*)scrollAction, true);
					}
				}
			}
			else
			{
				// The scrollBy affects a named field
				int activeItem = getActiveItem(ENdhsActiveItemCurrent);

				// If we're in 'rebound' territory, correct the active item
				if (!(m_scrollWrap && isWrappable()))
				{
					if (activeItem < 0)
						activeItem = 0;
					else if (activeItem > m_itemCount - 1)
						activeItem = m_itemCount - 1;
				}

				// Get the field value object
				NdhsCField* pField = getFieldValue(scrollAction->field, m_menu, activeItem, NULL);

				if (pField)
				{
					LcTScalar currentValue = pField->getRawFieldData(NULL);
					LcTScalar targetValue = pField->getTargetValue();

						LcTScalar minValue = 0;
						bool minDefined = false;

						LcTScalar maxValue = 0;
						bool maxDefined = false;

						if (scrollAction->minDefined && scrollAction->minExpr)
						{
							NdhsCExpression* minExpr = scrollAction->minExpr->getContextFreeExpression();

							if (minExpr)
							{
								minExpr->evaluate(this, slotNum, NULL);

								if (minExpr->isError() || !minExpr->isNumeric())
								{
									minExpr->errorDiagnostics("ScrollBy minimum value", true);
								}
								else
								{
									minValue = minExpr->getValueScalar();
									minDefined = true;
								}
							}
						}

						if (scrollAction->maxDefined && scrollAction->maxExpr)
						{
							NdhsCExpression* maxExpr = scrollAction->maxExpr->getContextFreeExpression();

							if (maxExpr)
							{
								maxExpr->evaluate(this, slotNum, NULL);

								if (maxExpr->isError() || !maxExpr->isNumeric())
								{
									maxExpr->errorDiagnostics("ScrollBy maximum value", true);
								}
								else
								{
									maxValue = maxExpr->getValueScalar();
									maxDefined = true;
								}
							}
						}

						// A scroll will succeed provided that we're not at one of the limits and the scroll would take us
						// into out of bounds territory
						if ( ((dir == ENdhsFieldDirectionIncreasing) && (!maxDefined || currentValue < maxValue) )
								|| ((dir == ENdhsFieldDirectionDecreasing) && (!minDefined || currentValue > minValue)) )
						{
							attemptOk = true;
						}
						else
						{
							attemptOk = false;
						}

						if (isKick && (scrollAction->duration > 0))
						{
							// Kick scroll - we either start a kick scroll, add to an existing kick scroll, or cancel a kick scroll
							// depending on the state of the field
							if (pField->atRest())
							{
								// Start kick
								if (minDefined && maxDefined)
								{
									pField->setTargetValue(currentValue + scrollAction->amount, minValue, maxValue, scrollAction->duration);
								}
								else
								{
									pField->setTargetValue(currentValue + scrollAction->amount, scrollAction->duration);
								}

								// We need to start animating to allow the field to update
								m_pageManager->startAnimation();
							}
							else
							{
								// Field already kicked...if this kick is in the same direction we increase the kick, otherwise
								// we stop it dead.
								if ( ((dir == ENdhsFieldDirectionIncreasing) && (pField->getVelocity() > 0))
									|| ((dir == ENdhsFieldDirectionDecreasing) && (pField->getVelocity() < 0)))
								{
									// Start kick
									if (minDefined && maxDefined)
									{
										pField->setTargetValue(targetValue + scrollAction->amount, minValue, maxValue, scrollAction->duration);
									}
									else
									{
										// Set target value
										pField->setTargetValue(targetValue + scrollAction->amount, scrollAction->duration);
									}
								}
								else
								{
									// Stop field changing.
									if (minDefined && maxDefined)
									{
										pField->setValue(min(maxValue, max(minValue, currentValue)), true);
									}
									else
									{
										pField->setValue(currentValue, true);
									}
								}
							}
						}
						else if (scrollAction->duration > 0)
						{
							// Field already animating...
							if (pField->atRest() == false)
								pField->setValue(pField->getTargetValue(), false);

							// Start
							if (minDefined && maxDefined)
							{
								pField->setTargetValue(pField->getRawFieldData(NULL) + scrollAction->amount, minValue, maxValue, scrollAction->duration);
							}
							else
							{
								pField->setTargetValue(pField->getRawFieldData(NULL) + scrollAction->amount, scrollAction->duration);
							}

							// We need to start animating to allow the field to update
							m_pageManager->startAnimation();
						}
						else
						{
							// Instantaneous field change
							LcTScalar newValue;

							if (pField->atRest() == false)
							{
								// We will interrupt any kick in progress...note that we don't want to
								// call 'setValue' here as that would notify all observers of a change
								// and we're going to do that shortly anyway
								LcTScalar target = pField->getTargetValue();

								// Obey limits (as target may be outside range)
								if (maxDefined && target > maxValue)
									target = maxValue;
								else if (minDefined && target < minValue)
									target = minValue;

								newValue = target + scrollAction->amount;
							}
							else
							{
								newValue = currentValue + scrollAction->amount;
							}

							// ensure that max/min limits obeyed
							if (maxDefined && newValue > maxValue)
								newValue = maxValue;
							else if (minDefined && newValue < minValue)
								newValue = minValue;

						// Apply the change in value - note that wrapping is not permitted, and that the 'finalUpdate' flag
						// should be set to 'true' so that modules are notified.
						pField->setValue(newValue, true);
					}
				}
			}
			break;
		}

		// EDeactivate
		//
		case NdhsCMenuComponentTemplate::CAction::CAttempt::EDeactivate:
		{
			// Can't deactivate in the inactive layout
			if (m_activeSlot >= 0)
			{
				attemptOk = true;

				// Terminate any current scroll
				if (getAnimType() == ENdhsAnimationTypeScroll)
					jumpTransitionToEnd(false);

				// Interrupt any inter-page animations
				m_pageManager->jumpTransitionToEnd();

				m_chainedAction = ((NdhsCMenuComponentTemplate::CMenuAction::CDeactivate*)(attemp))->action;

				(void)changeState(ENdhsPageStateInteractive, -1, attemp, true);

				updateMetaFields();
			}

			break;
		}

		// EJumpBy jumps by a set amount from the current active position
		//
		//    amount : offset to jump by
		//    wrap : whether to 'wrap' between 0 and getSlotCount()-1
		case NdhsCMenuComponentTemplate::CAction::CAttempt::EJumpBy:
		{
			int amount = ((NdhsCMenuComponentTemplate::CMenuAction::CJumpBy*)(attemp))->amount;

			// Can't jumpBy in the inactive layout
			if (amount != 0 && m_activeSlot >= 0)
			{
				int  pos  = m_activeSlot + amount;
				bool wrap = ((NdhsCMenuComponentTemplate::CMenuAction::CJumpBy*)(attemp))->wrap;

				// Apply 'wrap'ping if appropriate
				if (wrap)
					pos = wrapToRange(pos - m_firstSelectableSlot, getSelectableSlotCount())
						+ m_firstSelectableSlot;

				// If the new slot is visible, we can break as the attempt is OK
				if (isSlotSelectable(pos) && getItemInSlot(pos))
				{
					attemptOk = true;

					// Interrupt any scrolling on this page
					if (getAnimType() == ENdhsAnimationTypeScroll)
						jumpTransitionToEnd(false);

					// Interrupt any inter-page animations
					m_pageManager->jumpTransitionToEnd();

					m_chainedAction = ((NdhsCMenuComponentTemplate::CMenuAction::CJumpBy*)(attemp))->action;

					(void)changeState(ENdhsPageStateInteractive, pos, (NdhsCTemplate::CAction::CAttempt*)attemp, true);
				}
			}

			break;
		}

		// EJumpTo jumps to a new active position
		//
		//    slot : new active slot id
		case NdhsCMenuComponentTemplate::CAction::CAttempt::EJumpTo:
		{
			int slot = ((NdhsCMenuComponentTemplate::CMenuAction::CJumpTo*)(attemp))->slot;

			// Don't let jumpTo change to the inactive layout
			// Jumping to the currently active slot causes a drop through
			if ((slot >= 0 ) && (slot != m_activeSlot)
				&& (isSlotSelectable(slot) && getItemInSlot(slot)))
			{
				attemptOk = true;

				// Interrupt any scrolling on this page
				if (getAnimType() == ENdhsAnimationTypeScroll)
					jumpTransitionToEnd(false);

				// Interrupt any inter-page animations
				m_pageManager->jumpTransitionToEnd();

				m_chainedAction = ((NdhsCMenuComponentTemplate::CMenuAction::CJumpTo*)(attemp))->action;

				(void)changeState(ENdhsPageStateInteractive, slot, (NdhsCTemplate::CAction::CAttempt*)attemp, true);
			}

			break;
		}

		// ELink
		//
		//    uri : The link to launch in the browser
		case NdhsCMenuComponentTemplate::CAction::CAttempt::ELink:
		{
			bool moduleLink = false;

			LcTaString link = ((NdhsCMenuComponentTemplate::CAction::CLink*)(attemp))->uriString;

			NdhsCExpression::CExprSkeleton* uriCFExpr = (((NdhsCMenuComponentTemplate::CAction::CLink*)(attemp))->uri).ptr();
			if (link.isEmpty())
			{
				if (uriCFExpr)
				{
					NdhsCExpression* uriExpr = uriCFExpr->getContextFreeExpression();
					if (uriExpr)
					{
						uriExpr->evaluate(this, slotNum, NULL);

						if (uriExpr->isError())
						{
							uriExpr->errorDiagnostics("Link action URI", true);
						}
						else
						{
							link = uriExpr->getValueString();
						}
					}
				}
			}
			else
			{
				moduleLink = true;
			}

			LcTaString expandedLink;
			LcTaString linkPrefix;

			// Check for modules trying to navigate via the IFXI_RequestExecuteLink route
			if (moduleLink)
			{
				bool linkValid = m_pageManager->isValidURIToExecuteFromCallBack(link);

				// We entry point ID, or a module link
				if (linkValid && link.find("://") == -1)
				{
					// We got an entry point ID
					link = m_pageManager->getEntryPointMapStack()->getEntryPoint(link);
				}
			}

			// Token replace the URI
			m_pageManager->getTokenStack()->replaceTokens(link,	expandedLink, NULL, getMenu(), getMenuItemInSlot(slotNum), this, m_stackLevel);

			// Determine the link prefix
			linkPrefix = expandedLink.subString(0, expandedLink.find(":"));

			ENdhsLinkType linkType = m_pageManager->getCon()->getTRLinkType(linkPrefix);

			switch (linkType)
			{
				case ENdhsLinkTypeSyncLinkPlugin:
				{
					// The link will not result in a menu being opened, so we
					// should just fire it now without jumping to end

					// Need to pass in a menu item, so create one
					LcTaOwner<NdhsCMenuItem> mi = NdhsCMenuItem::create(NULL);
					mi->setLinkAttr(expandedLink);

					// launch the link
					attemptOk = m_pageManager->getCon()->launchLink(mi.ptr(), m_stackLevel);

					if(attemptOk)
					{
						executeChainedAction(((NdhsCMenuComponentTemplate::CAction::CLink*)(attemp))->action, slotNum);
					}

					break;
				}

				case ENdhsLinkTypeTheme:
				{
					if (slotNum == -1)
						slotNum = m_activeSlot;

					if (isSlotSelectable(slotNum) && getItemInSlot(slotNum))
					{
						// Determine the link prefix
						NdhsCMenuItem* item = getMenuItemInSlot(slotNum);

						if (item)
						{
							// Handle trigger
							attemptOk = true;

							// Do slot change
							// Interrupt any scrolling on this page
							if (getAnimType() == ENdhsAnimationTypeScroll)
								jumpTransitionToEnd(false);

							// Interrupt any inter-page animations
							m_pageManager->jumpTransitionToEnd();

							// set link attribute and launch item
							item->setLinkAttr(link);
							attemptOk = m_pageManager->launchItem(item, false, 0);
						}
					}
					break;
				}

				// If the link is unknown, ignore
				case ENdhsLinkTypeUnknown:
					break;

				// Otherwise pass on to page manager
				default:
				{
					// The link will open a menu, so we must stop the transition
					// and ask the page manager to handle the link
					if (m_animType != ENdhsAnimationTypeNone)
						jumpTransitionToEnd(true);

						attemptOk = m_pageManager->processAttempt(attemp, m_stackLevel, expandedLink);
				}
			}

			break;
		}

		// ESignal
		//
		//
		case NdhsCMenuComponentTemplate::CAction::CAttempt::ESignal:
		{
			// Interrupt any scrolling on this page
			if (getAnimType() == ENdhsAnimationTypeScroll)
				jumpTransitionToEnd(false);

			attemptOk = m_parent->processTrigger(ENdhsNavigationOnSignal,
										getSlot(),
										m_componentName,
										NULL,
										false);
			break;
		}

		default:
			// Allow general component attempts
			attemptOk = NdhsCComponent::doAttempt(attemp, slotNum);
			break;
	}

	if (!m_chainedAction.isEmpty())
	{
		m_chainedActionSlot = slotNum;
	}

	// Make sure placeholders are replaced in the case where an action has
	// interrupted a scroll but not started a new scroll.
	if (m_placeholderUsed
		&& (m_animType != ENdhsAnimationTypeScroll && m_animType != ENdhsAnimationTypeScrollKick))
	{
		scheduleReplacePlaceholders();
	}

	return attemptOk;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::stateManagerDestroyed(NdhsCStateManager* sm)
{
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCMenuComponent::getField(const LcTmString& field, int slotNum, NdhsCMenuItem* item)
{
	NdhsCField*	retVal = NULL;

	if (field.compareNoCase("_scrollpos") == 0)
	{
		retVal = m_scrollPos.ptr();
	}

	if (field.compareNoCase("_placeholderActive") == 0)
	{
		if (m_menu && item && item->getOwner() == m_menu)
		{
			int index = getItemIndex(-1, item);

			if (index != -1)
			{
				retVal = m_placeHolderFields[index];
			}
		}
		else
		{
			// Check parent - note that we replace the passed-in item context
			// with the item context for the menu component, as that will be
			// understood by a parent menu
			retVal =  m_parent->getField(field, slotNum, getMenuItem());
		}
	}

	if (!retVal)
	{
		retVal = NdhsCComponent::getField(field, slotNum, item);
	}

	if (!retVal && m_menu)
	{
		// Check if no item context has been given
		if (slotNum < 0 && !item)
		{
			// Convert search string to lower case.
			LcTaString lowerFieldName = field.toLower();

			// Check meta-field cache first
			LcTmOwnerMap<LcTmString, CMetaField>::iterator it = m_metaFields.find(lowerFieldName);

			if (m_metaFields.end() != it)
			{
				retVal = it->second;
			}
			else if (m_menu->isItemField(lowerFieldName))
			{
				LcTaOwner<CMetaField> mf = CMetaField::create(lowerFieldName);

				mf->setField(m_menu->getField(lowerFieldName, m_menu->getMenuItemIndex(getActiveItem(ENdhsActiveItemTarget)), NULL));

				retVal = mf.ptr();
				m_metaFields.add_element(lowerFieldName, mf);
			}
		}

		if (!retVal)
		{
			retVal = m_menu->getField(field, getItemIndex(slotNum, item), NULL);
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
int	NdhsCMenuComponent::getItemIndex(int slotNum, NdhsCMenuItem* item)
{
	int itemIndex = -1;

	if (!m_outerGroup || !m_menu)
		return -1;

	if (item)
	{
		itemIndex = item->getIndex();
	}
	else if (slotNum >= 0)
	{
		CItem* slotItem = getItemInSlot(slotNum);

		if (slotItem)
		{
			NdhsCMenuItem* menuItem = slotItem->getMenuItem();
			if (menuItem)
			{
				itemIndex = menuItem->getIndex();
			}
		}
	}

	return itemIndex;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCMenuComponent::getString(bool& found, const LcTmString& field, int slotNum, NdhsCMenuItem* item)
{
	LcTaString retVal = "";

	NdhsCMenuItem* menuItem = item;

	if (!m_outerGroup)
		return retVal;

	if (slotNum >= 0 && m_itemCount > 0)
	{
		CItem* slotItem = getItemInSlot(slotNum);

		if (slotItem)
		{
			menuItem = slotItem->getMenuItem();
		}
	}

	found = false;
	if (menuItem)
	{
		found = menuItem->getFieldData(NULL, field, retVal);
	}

	if (!found && m_menu)
	{
		found = m_menu->getFieldData(NULL, field, retVal);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::unLoadGroup()
{
	if (m_outerGroup && m_isEnabled && m_groupUnloaded == false)
	{
		doCleanup();
		m_isEnabled = false;
		m_isCreated = false;
		m_groupUnloaded = true;
		m_trySetFocus = true;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::loadGroup()
{
	if (m_isEnabled == false && m_groupUnloaded)
	{
		construct();

		m_groupUnloaded = false;
		m_outerGroup->loadResources();
		getLocalScreenAggregate()->loadResources();
		TmAItems::iterator it = m_items.begin();
		for (; it != m_items.end(); it++)
		{
			if((*it)!=NULL)
				(*it)->loadResources();
		}
		realize(getOwner());
		m_isEnabled = true;

		if (m_trySetFocus)
		{
			NdhsCPageModel* pageModel = m_parent->findParentPageModel();

			if (pageModel)
			{
				pageModel->trySetFocus();
			}
		}
		m_trySetFocus = false;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::updateTouchDownIntrinsicField(const LcTmString& element, int slot)
{
	if (m_fieldCache)
	{
		NdhsCField* field = m_fieldCache->getField("_touchElement");
		if (field)
		{
			field->setValue(element, true);
		}

		NdhsCField* slotField = m_fieldCache->getField("_touchSlot");
		if (slotField)
		{
			slotField->setValue(max(0.0f, slot + 1.0f), true);
		}
	}
}

#ifdef LC_USE_MOUSEOVER
/*-------------------------------------------------------------------------*//**
																			 */
void NdhsCMenuComponent::updateMouseOverIntrinsicField(const LcTmString& element, int slot)
{
	if (m_fieldCache)
	{
		NdhsCField* slotField = m_fieldCache->getField("_mouseoverslot");
		if (slotField)
		{
			slotField->setValue(max(0.0f, slot + 1.0f), true);
		}

		NdhsCComponent::updateMouseOverIntrinsicField(element, slot);
	}
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::updateMetaFields()
{
	if (!m_outerGroup)
		return;

	// When the active item changes, update all meta-fields to point to new active item
	if (m_menu)
	{
		LcTmOwnerMap<LcTmString, CMetaField>::iterator it = m_metaFields.begin();

		int itemIndex = getActiveItem(ENdhsActiveItemTarget);
		if (m_activeSlot == -1)
		{
			itemIndex = -1;
		}

		if (m_menu->getMenuSource() == NdhsCMenu::EXml || m_menu->getMenuSource() == NdhsCMenu::ENodeXml)
		{
			for (; it != m_metaFields.end(); it++)
			{
				it->second->setMenuInfo(m_menu, itemIndex);
			}
		}
		else if (m_menu->getMenuSource() == NdhsCMenu::EPlugin)
		{
			for (; it != m_metaFields.end(); it++)
			{
				it->second->setField(m_menu->getField(it->first, m_menu->getMenuItemIndex(itemIndex), NULL));
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuComponent::CMetaField> NdhsCMenuComponent::CMetaField::create(const LcTmString& fieldName)
{
	LcTaOwner<CMetaField> ref;
	ref.set(new CMetaField());
	ref->construct(fieldName);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuComponent::CMetaField::CMetaField()
:	m_field(NULL),
	m_menu(NULL),
	m_itemIndex(-1)
{
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CMetaField::construct(const LcTmString& fieldName)
{
	m_fieldName = fieldName;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CMetaField::fieldValueUpdated(NdhsCField* field)
{
	broadcastFieldValueUpdate();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CMetaField::fieldDirty(NdhsCField* field)
{
	broadcastFieldDirty();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CMetaField::fieldDestroyed(NdhsCField* field)
{
	if (field == m_field)
	{
		m_field = NULL;
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCMenuComponent::CMetaField::getFieldData(NdhsCElement* element)
{
	LcTaString retVal;

	if (m_field)
	{
		retVal = m_field->getFieldData(element);
	}
	else if (m_menu && m_itemIndex >= 0)
	{
		NdhsCMenuItem* item = m_menu->getMenuItem(m_itemIndex);
		if (item)
		{
			if (!item->getFieldData(element, m_fieldName, retVal))
			{
				retVal = "";
			}
		}
	}
	else if(m_menu)
	{
		if (!m_menu->getFieldData(element, m_fieldName, retVal))
		{
			retVal = "";
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCMenuComponent::CMetaField::getFieldData(NdhsCElement* element, bool forceBroadcast)
{
	LcTaString retVal;

	if (m_field)
	{
		retVal = m_field->getFieldData(element, forceBroadcast);
	}
	else if (m_menu && m_itemIndex >= 0)
	{
		NdhsCMenuItem* item = m_menu->getMenuItem(m_itemIndex);
		if (item)
		{
			if (!item->getFieldData(element, m_fieldName, retVal))
			{
				retVal = "";
			}
		}
	}
	else if(m_menu)
	{
		if (!m_menu->getFieldData(element, m_fieldName, retVal))
		{
			retVal = "";
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CMetaField::setField(NdhsCField* field)
{
	if (m_field)
	{
		m_field->removeObserver(this);
		m_field = NULL;
	}

	m_menu = NULL;
	m_itemIndex = -1;

	if (field)
	{
		m_field = field;
		field->addObserver(this);
	}

	broadcastFieldValueUpdate();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::CMetaField::setMenuInfo(NdhsCMenu* menu, int itemIndex)
{
	if (m_field)
	{
		m_field->removeObserver(this);
		m_field = NULL;
	}

	m_menu = menu;
	m_itemIndex = itemIndex;

	broadcastFieldValueUpdate();
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuComponent::CMetaField::~CMetaField()
{
	if (m_field)
	{
		m_field->removeObserver(this);
		m_field = NULL;
	}
}

#ifdef IFX_SERIALIZATION
NdhsCMenuComponent::CMetaField* NdhsCMenuComponent::CMetaField::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	if(handle==-1)
	{
		NdhsCMenuComponent::CMetaField *obj=new NdhsCMenuComponent::CMetaField();
		return obj;
	}
	NdhsCMenuComponent::CMetaField *obj=new NdhsCMenuComponent::CMetaField();
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCMenuComponent::CMetaField::serialize(LcCSerializeMaster *serializeMaster,bool force)
{
	SerializeHandle handle=-1;
	if(!force)
	{
		handle=serializeMaster->getHandle(this);
		if(handle!=-1 && serializeMaster->isSerialized(handle))
		{
			return handle;
		}
		else if(handle==-1)
		{
			handle=serializeMaster->newHandle(this);
		}
	}
	else
	{
		handle=serializeMaster->newHandle(this);
	}

	int outputSize = sizeof(NdhsCMenuComponent::CMetaField) - sizeof(NdhsCField)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCField::serialize(serializeMaster,true);
	ENdhsCFieldType dataType=ENdhsCFieldTypeMeta;
	SERIALIZE(dataType,serializeMaster,cPtr)
	SERIALIZE(parentHandle,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_field,serializeMaster,cPtr)
	SERIALIZE_String(m_fieldName,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_menu,serializeMaster,cPtr)
	SERIALIZE(m_itemIndex,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCMenuComponent::CMetaField::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummySize=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;
	DESERIALIZE(dummySize,serializeMaster,cPtr)
	DESERIALIZE(parentHandle,serializeMaster,cPtr)
	NdhsCField::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE_Reserve(m_field,serializeMaster,cPtr,NdhsCField)
	DESERIALIZE_String(m_fieldName,serializeMaster,cPtr)
	DESERIALIZE_Reserve(m_menu,serializeMaster,cPtr,NdhsCMenu)
	DESERIALIZE(m_itemIndex,serializeMaster,cPtr)
}
#endif /* IFX_SERIALIZATION */

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCMenuComponent::getPageParamValue(const LcTmString& key)
{
	// Request parent to provide the value
	return m_parent->getPageParamValue(key);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::onSuspend()
{
	if (m_egItem)
		m_egItem->onSuspend();

	if (m_egDetailPage)
	{
		m_egDetailPage->onSuspend();
		m_egDetailMenu->onSuspend();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::onResume()
{
	if (m_egItem)
		m_egItem->onResume();

	if (m_egDetailPage)
	{
		m_egDetailPage->onResume();
		m_egDetailMenu->onResume();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::loadResources()
{
	if (m_egItem)
		m_egItem->loadResources();

	if (m_egDetailPage)
	{
		m_egDetailPage->loadResources();
		m_egDetailMenu->loadResources();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::UnloadResources()
{
	if (m_egItem)
		m_egItem->UnloadResources();

	if (m_egDetailPage)
	{
		m_egDetailPage->UnloadResources();
		m_egDetailMenu->UnloadResources();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::loadGroup()
{
	if (m_egItem)
		m_egItem->loadGroup();

	if (m_egDetailPage)
	{
		m_egDetailPage->loadGroup();
		m_egDetailMenu->loadGroup();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenuComponent::CItem::unLoadGroup()
{
	m_egItem->unLoadGroup();

	if (m_egDetailPage)
	{
		m_egDetailPage->unLoadGroup();
		m_egDetailMenu->unLoadGroup();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::refreshField(const LcTmString& field, int item)
{
	// Refresh field
	if (m_menu)
		m_menu->refreshField(field, item);
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCElement* NdhsCMenuComponent::getItem(const LcTmString& elementClass)
{
	NdhsCElement* retVal = NULL;

	if (!m_outerGroup)
		return NULL;

	if (m_activeSlot != -1 && m_itemCount > 0)
	{
		CItem* activeItem = getItemInSlot(m_activeSlot);

		if (activeItem)
		{
			NdhsCElementGroup* activeItemElementGroup = activeItem->getItemGroup();

			if (activeItemElementGroup)
			{
				retVal = activeItemElementGroup->getItem(elementClass);
			}
		}
	}

	if (!retVal && m_furnitureElementGroup)
	{
		retVal = m_furnitureElementGroup->getItem(elementClass);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCElementGroup* NdhsCMenuComponent::getGroup(const LcTmString& groupName)
{
	NdhsCElementGroup* retVal = NULL;

	if (!m_outerGroup)
		return NULL;

	if (m_activeSlot != -1 && m_itemCount > 0)
	{
		CItem* activeItem = getItemInSlot(m_activeSlot);

		if (activeItem)
		{
			NdhsCElementGroup* activeItemElementGroup = activeItem->getItemGroup();

			if (activeItemElementGroup)
			{
				retVal = activeItemElementGroup->getGroup(groupName);
			}
		}
	}

	if (!retVal && m_furnitureElementGroup)
	{
		retVal = m_furnitureElementGroup->getGroup(groupName);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::backupFocus()
{
	if (!m_outerGroup)
		return;

	NdhsCElement* focusElem = getFocusedElement();
	LcTaString currentFocusClass = "";

	if (focusElem)
	{
		if (focusElem->getElementUse() == ENdhsObjectTypeItem)
		{
			currentFocusClass = focusElem->getElementClassName();
		}
	}
	else
	{
		NdhsCComponent* focusComponent = getFocusedChildComponent();

		if (focusComponent)
		{
			if (focusComponent->getGroupType() == ENdhsObjectTypeItem
				|| focusComponent->getGroupType() == ENdhsObjectTypeItemComponent)
			{
				currentFocusClass = focusComponent->getGroupName();
			}
		}
	}

	if (!currentFocusClass.isEmpty())
	{
		unsetFocus();
		m_previouslyFocusedClass = currentFocusClass;
		focusSetOnChild(this, this);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::restoreFocus()
{
	if (!m_outerGroup)
		return;

	if (!m_previouslyFocusedClass.isEmpty())
	{
		if(trySetFocus(m_previouslyFocusedClass))
		{
			m_previouslyFocusedClass = "";
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::unsetFocus()
{
	// focus is being reassigned - if we have a m_previouslyFocusedClass
	// stored, we need to forget it or we'll stamp over the new
	// focus element
	m_previouslyFocusedClass = "";

	// Now do base class action
	NdhsCComponent::unsetFocus();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::onSuspend()
{
	if (!m_outerGroup)
		return;

	TmAItems::iterator it = m_items.begin();
	for (; it != m_items.end(); it++)
	{
		if((*it)!=NULL)
			(*it)->onSuspend();
	}

	NdhsCComponent::onSuspend();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::onResume()
{
	if (!m_outerGroup)
		return;

	TmAItems::iterator it = m_items.begin();
	for (; it != m_items.end(); it++)
	{
		if((*it)!=NULL)
			(*it)->onResume();
	}

	NdhsCComponent::onResume();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::expressionDirty(NdhsCExpression* expr)
{
	// If this is the menu data source expression, ignore the update if
	// we've successfully loaded already (for now)
	if (expr == m_dataSourceExp.ptr())
	{
		if (m_menu==NULL)
		{
			// cannot reloaddatasource here because there can
			// be objects on stack that initiated this call and
			// are going to be deleted.
			scheduleReloadDataSource();
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponent::expressionDestroyed(NdhsCExpression* expr)
{
}

#if defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager::CStaticPreviewCache> NdhsCMenuComponent::takeSnapShot()
{
	LcTaOwner<NdhsCPageManager::CStaticPreviewCache> info = NdhsCPageManager::CStaticPreviewCache::create();
	int index = 0;

	// Take snapshot of current component
	if (info)
	{
		info->slotNumber = getSlot();

		info->componentClassName = m_componentName;

		index = m_path.toLower().find("b:/");
		if (index != -1)
		{
			info->componentPath = m_path.subString(index + 3, m_path.length());

			index = info->componentPath.find("Packages/");

			if (index != -1)
			{
				info->componentPath = m_path.subString(index + 12, m_path.length());

				index = info->componentPath.find("/");
				if (index != -1)
				{
					info->componentPath = info->componentPath.tail(index + 1);
				}
			}
		}
		else
		{
			info->componentPath = m_path;
		}

		if (m_currentLayout)
			info->layoutName = m_currentLayout->name;
		info->identifier = m_identifier;
	}
	return info;
}
#endif
