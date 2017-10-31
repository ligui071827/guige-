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
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


#include <ctype.h>

#define AGG_MASK (LcTPlacement::ELocation | LcTPlacement::EScale | LcTPlacement::EOrientation | LcTPlacement::EOpacity | LcTPlacement::EOffset)

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CVariableInfo>	NdhsCTemplate::CVariableInfo::create()
{
	LcTaOwner<CVariableInfo> ref;
	ref.set(new CVariableInfo());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CParameters>	NdhsCTemplate::CParameters::create()
{
	LcTaOwner<CParameters> ref;
	ref.set(new CParameters());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CState>	NdhsCTemplate::CState::create()
{
	LcTaOwner<CState> ref;
	ref.set(new CState());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CEventInfo>	NdhsCTemplate::CEventInfo::create()
{
	LcTaOwner<CEventInfo> ref;
	ref.set(new CEventInfo());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
																			 */
LcTaOwner<NdhsCTemplate::CSlotClassTrigger>	NdhsCTemplate::CSlotClassTrigger::create()
{
	LcTaOwner<CSlotClassTrigger> ref;
	ref.set(new CSlotClassTrigger());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CConditionBlock>	NdhsCTemplate::CConditionBlock::create()
{
	LcTaOwner<CConditionBlock> ref;
	ref.set(new CConditionBlock());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CAction> NdhsCTemplate::CAction::create()
{
	LcTaOwner<CAction> ref;
	ref.set(new CAction());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CDisplacement>	NdhsCTemplate::CDisplacement::create()
{
	LcTaOwner<CDisplacement> ref;
	ref.set(new CDisplacement());
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTScalar NdhsCTemplate::CDisplacement::getNormalizedValue(NdhsCExpression* expr)
{
	LcTScalar val = 0.0f;

	if (expr)
	{
		bool isScrollPos = false;

		NdhsCField* field = expr->getField();
		if (field)
		{
			if (field->isScrollPosField())
			{
				isScrollPos = true;
			}
		}

		NdhsCExpression::ENdhsExpressionType expressionType=expr->evaluate();
		if (isScrollPos)
		{
			// Scrollpos is a special case, we don't need max and min values
			val = ((NdhsCScrollPosField*)field)->getNormalizedValue(NULL);
		}
		else
		{
			switch(expressionType)
			{
				case NdhsCExpression::ENdhsExpressionTypeBool:
				{
					val = expr->getValueBool() ? 1.0f : 0.0f;
				}
				break;

				case NdhsCExpression::ENdhsExpressionTypeInt:
				case NdhsCExpression::ENdhsExpressionTypeScalar:
				{
					if (maxValue <= minValue)
					{
						val = 0.0f;
					}
					else
					{
						val = (expr->getValueScalar() - minValue) / (maxValue - minValue);
						val = min(1.0f, max(0.0f, val));
					}
				}
				break;

				default:
					expr->errorDiagnostics("Displacement value", true);
					break;
			}
		}
	}

	return val;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CElementDisplacement>	NdhsCTemplate::CElementDisplacement::create()
{
	LcTaOwner<CElementDisplacement> ref;
	ref.set(new CElementDisplacement());
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CAction::CAttempt>	NdhsCTemplate::CAction::CAttempt::create()
{
	LcTaOwner<CAttempt> ref;
	ref.set(new CAttempt());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CAction::CScrollBy> NdhsCTemplate::CAction::CScrollBy::create()
{
	LcTaOwner<CScrollBy> ref;
	ref.set(new CScrollBy());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CAction::CBack> NdhsCTemplate::CAction::CBack::create()
{
	LcTaOwner<CBack> ref;
	ref.set(new CBack());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CAction::CLink>				NdhsCTemplate::CAction::CLink::create()
{
	LcTaOwner<CLink> ref;
	ref.set(new CLink());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CAction::CSetFocus>			NdhsCTemplate::CAction::CSetFocus::create()
{
	LcTaOwner<CSetFocus> ref;
	ref.set(new CSetFocus());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CAction::CUnsetFocus>		NdhsCTemplate::CAction::CUnsetFocus::create()
{
	LcTaOwner<CUnsetFocus> ref;
	ref.set(new CUnsetFocus());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CAction::CTabFocus>			NdhsCTemplate::CAction::CTabFocus::create()
{
	LcTaOwner<CTabFocus> ref;
	ref.set(new CTabFocus());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::CLayout::TmElementLayout::configureDisplacementsFromXml(LcCXmlElem* elem)
{
	if(elem)
	{
		LcCXmlElem* eDisplacements = elem->find(NDHS_TP_XML_DISPLACEMENTS);

		if(eDisplacements)
		{
			LcTScalar minS = 0.0f;
			LcTScalar maxS = 0.0f;
			LcCXmlElem* eDisplacement = eDisplacements->getFirstChild();

			for( ; eDisplacement; eDisplacement = eDisplacement->getNext())
			{
				if (eDisplacement->getName().compareNoCase(NDHS_TP_XML_DISPLACEMENT) != 0)
					continue;

				LcTaString value = eDisplacement->getAttr(NDHS_TP_XML_VALUE);

				if(value.isEmpty())
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "No value given for displacement");
					continue;
				}

				LcTaOwner<NdhsCExpression::CExprSkeleton> des = NdhsCExpression::CExprSkeleton::create(value);

				if(des->isEmpty())
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Invalid value expression given for displacement");
					continue;
				}

				// _scrollPos is special case, we ignore min and max
				if (value.compareNoCase(NDHS_TP_XML__SCROLLPOS) != 0)
				{
					LcCXmlAttr* maxAttr = eDisplacement->findAttr(NDHS_TP_XML_MAX);
					LcCXmlAttr* minAttr = eDisplacement->findAttr(NDHS_TP_XML_MIN);

					if (maxAttr && minAttr)
					{
						LcTaString maxValue = maxAttr->getVal();
						LcTaString minValue = minAttr->getVal();

						maxS = maxValue.toScalar();
						minS = minValue.toScalar();

						if((maxS - minS) <= 0)
						{
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Max value in displacement is less than or equal to the minimum value");
							continue;
						}
					}
					else if (maxAttr && !minAttr)
					{
						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Max value in displacement specified without minimum");
						continue;
					}
					else if (!maxAttr && minAttr)
					{
						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Min value in displacement specified without maximum");
						continue;
					}
				}

				LcTaString animation = eDisplacement->getAttr(NDHS_TP_XML_ANIMATION).toLower();

				LcTaOwner<CDisplacement> displacementObject = CDisplacement::create();

				displacementObject->minValue = minS;
				displacementObject->maxValue = maxS;
				displacementObject->value = value;
				displacementObject->animation = animation;
				displacementObject->exprSkeleton=des;

				if (owningTemplate)
				{
					// owningTemplate should always be set. But if it isn't don't leak memory!
					displacementObjectArray.push_back(displacementObject.ptr());
					owningTemplate->storeDisplacement(displacementObject);
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::CLayout::TmElementLayout::mergeDisplacements(TmElementLayout& layout)
{
	LcTmArray<CDisplacement*>::iterator it = layout.displacementObjectArray.begin();
	for(; it != layout.displacementObjectArray.end(); it++)
	{
		if (inheritDisplacement(*it))
		{
			// copy this displacement - if the displacement exists in the base layout, then
			// the animation is overwritten, otherwise the displacement is inserted.
			displacementObjectArray.push_back(*it);;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::CLayout::TmElementLayout::inheritDisplacement(CDisplacement* disp)
{
	if (disp == NULL)
		return false;

	if (disp->animation.isEmpty() == false)
		return true;

	LcTmArray<CDisplacement*>::iterator it = displacementObjectArray.begin();
	CDisplacement* tempDisp = NULL;

	for(; it != displacementObjectArray.end(); it++)
	{
		tempDisp = *it;
		// if we inherit from layout A, and want to remove a displacement from an element in A,
		// we need to add a displacement to that element in your new layout with the same 'value',
		// but no animation set.
		if (tempDisp != NULL && disp->value.compareNoCase(tempDisp->value) == 0 && disp->animation.isEmpty())
		{
			displacementObjectArray.erase(it);
			return false;
		}
	}
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCTemplate::CLayout::TmElementLayout NdhsCTemplate::CLayout::TmElementLayout::operator=
																(NdhsCTemplate::CLayout::TmElementLayout& elem2)
{
	hide = elem2.hide;
	mask = elem2.mask;
	layout = elem2.layout;
	unload = elem2.unload;
	owningTemplate = elem2.owningTemplate;

	// Clear out any existing displacements
	displacementObjectArray.clear();

	// Copy the displacements across

	LcTmArray<CDisplacement*>::iterator it = elem2.displacementObjectArray.begin();

	for(; it != elem2.displacementObjectArray.end(); it++)
	{
		// copy this displacement - if the displacement exists in the base layout, then
		// the animation is overwritten, otherwise the displacement is inserted.
		displacementObjectArray.push_back(*it);
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCTemplate::CLayout::TaElementLayout NdhsCTemplate::CLayout::TaElementLayout::operator=
																(NdhsCTemplate::CLayout::TmElementLayout& elem2)
{
	hide = elem2.hide;
	mask = elem2.mask;
	unload = elem2.unload;
	layout = elem2.layout;
	owningTemplate = elem2.owningTemplate;

	// Clear out any existing displacements
	displacementObjectArray.clear();

	// Copy the displacements across
	LcTmArray<CDisplacement*>::iterator it = elem2.displacementObjectArray.begin();

	for(; it != elem2.displacementObjectArray.end(); it++)
	{
		// copy this displacement - if the displacement exists in the base layout, then
		// the animation is overwritten, otherwise the displacement is inserted.
		displacementObjectArray.push_back(*it);;
	}
	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CLayout::CItemLayout>		NdhsCTemplate::CLayout::CItemLayout::create(NdhsCTemplate* owner)
{
	LcTaOwner<CItemLayout> ref;
	ref.set(new CItemLayout(owner));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CLayout>			NdhsCTemplate::CLayout::create(bool masterLayout, NdhsCTemplate* owner)
{
	LcTaOwner<CLayout> ref;
	ref.set(new CLayout(masterLayout, owner));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::CLayout::construct()
{
	stateInfo = CState::create();
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CGroup>		NdhsCTemplate::CGroup::create()
{
	LcTaOwner<CGroup> ref;
	ref.set(new CGroup());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CElement>			NdhsCTemplate::CElement::create()
{
	LcTaOwner<CElement> ref;
	ref.set(new CElement());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CComponentElement>			NdhsCTemplate::CComponentElement::create()
{
	LcTaOwner<CComponentElement> ref;
	ref.set(new CComponentElement());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::CComponentElement::releaseTemplate()
{
	// Component file may contain NULL i.e. component is in
	// palette and palette is not included with base project
	if (componentFile)
		componentFile->m_pageManager->releaseTemplate(componentFile);
}


/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CGraphicElement> NdhsCTemplate::CGraphicElement::create()
{
	LcTaOwner<CGraphicElement> ref;
	ref.set(new CGraphicElement());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CTextElement> NdhsCTemplate::CTextElement::create()
{
	LcTaOwner<CTextElement> ref;
	ref.set(new CTextElement());
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::CTextElement::construct()
{
	LcTaOwner<NdhsCTemplate::CTextElement::CFont> tempFont = NdhsCTemplate::CTextElement::CFont::create();
	font = tempFont;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CTextElement::CFont> NdhsCTemplate::CTextElement::CFont::create()
{
	LcTaOwner<CFont> ref;
	ref.set(new CFont());
	//ref->construct();
	return ref;
}

#ifdef LC_USE_LIGHTS
/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CSecondaryLightElement> NdhsCTemplate::CSecondaryLightElement::create()
{
	LcTaOwner<CSecondaryLightElement> ref;
	ref.set(new CSecondaryLightElement());
	//ref->construct();
	return ref;
}
#endif // def LC_USE_LIGHTS

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CLayoutDecoration> NdhsCTemplate::CLayoutDecoration::create()
{
	LcTaOwner<CLayoutDecoration> ref;
	ref.set(new CLayoutDecoration());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CLayoutDecoration::CDecorationInfo> NdhsCTemplate::CLayoutDecoration::CDecorationInfo::create()
{
	LcTaOwner<CDecorationInfo> ref;
	ref.set(new CDecorationInfo());
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::CLayoutDecoration::CDecorationInfo::construct()
{
	LcTaOwner<CAnimationRef> tempAnim = CAnimationRef::create();
	pageAnimation = tempAnim;

	tempAnim = CAnimationRef::create();
	menuAnimation = tempAnim;

	tempAnim = CAnimationRef::create();
	defaultFurnitureAnimation = tempAnim;

	LcTaOwner<CDecorationItem> tempDec = CDecorationItem::create();
	defaultItemAnimations = tempDec;

	LcTaOwner<CTriggersList> tempTriggers = CTriggersList::create();
	triggers = tempTriggers;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CAnimationRef> NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CAnimationRef::create()
{
	LcTaOwner<CAnimationRef> ref;
	ref.set(new CAnimationRef());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CDecorationItem> NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CDecorationItem::create()
{
	LcTaOwner<CDecorationItem> ref;
	ref.set(new CDecorationItem());
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CDecorationItem::construct()
{
	LcTaOwner<CAnimationRef> tempAnim = CAnimationRef::create();
	slotAnimation = tempAnim;
	tempAnim = CAnimationRef::create();
	defaultElementAnimation = tempAnim;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CFromSlot> NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CFromSlot::create()
{
	LcTaOwner<CFromSlot> ref;
	ref.set(new CFromSlot());
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTrigger> NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTrigger::create()
{
	LcTaOwner<CTrigger> ref;
	ref.set(new CTrigger());
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList> NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList::create()
{
	LcTaOwner<CTriggersList> ref;
	ref.set(new CTriggersList());
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList::addTrigger(int key, const LcTScalar position)
{
	LcTaOwner<CTrigger> tempTrigger = CTrigger::create();

	// Convert the key string into one of our key values
	tempTrigger->key = key;
	tempTrigger->position = position;

	triggerList.push_back(tempTrigger);
}

#ifdef IFX_USE_PLUGIN_ELEMENTS
/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CPluginElement>	NdhsCTemplate::CPluginElement::create()
{
	LcTaOwner<CPluginElement> ref;
	ref.set(new CPluginElement());
	//ref->construct();
	return ref;
}
#endif // def IFX_USE_PLUGIN_ELEMENTS

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CDragRegionElement> NdhsCTemplate::CDragRegionElement::create()
{
	LcTaOwner<CDragRegionElement> ref;
	ref.set(new CDragRegionElement());
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::CDragRegionElement::construct()
{
	LcTaOwner<NdhsCTemplate::CDragRegionElement::CDragRegionSetting> tempxDrag = NdhsCTemplate::CDragRegionElement::CDragRegionSetting::create();
	LcTaOwner<NdhsCTemplate::CDragRegionElement::CDragRegionSetting> tempyDrag = NdhsCTemplate::CDragRegionElement::CDragRegionSetting::create();

	xDrag = tempxDrag;
	yDrag = tempyDrag;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate::CDragRegionElement::CDragRegionSetting> NdhsCTemplate::CDragRegionElement::CDragRegionSetting::create()
{
	LcTaOwner<CDragRegionSetting> ref;
	ref.set(new CDragRegionSetting());
	//ref->construct();
	return ref;
}
#endif //LC_USE_STYLUS

/*-------------------------------------------------------------------------*//**
*/
NdhsCTemplate::CLayout::~CLayout()
{
	furnitureLayouts.clear();
	itemLayouts.clear();

	if(m_isMaster)
		m_ownedSlotCurves.clear();
	else
		m_slotCurves.clear();

	if(m_isMaster)
		m_ownedItemCurves.clear();
	else
		m_itemCurves.clear();
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::CLayout::addCurve(int curveId, const LcTmString& className, LcTmOwner<NdhsCKeyFrameList>& keyframeList)
{
	LC_ASSERT(m_isMaster);

	if(className.isEmpty())		// add to slot-map
	{
		m_ownedSlotCurves.add_element(curveId, keyframeList);
	}
	else						//  add to item-map
	{
		// find if an entry already exists?
		TmMOwnedItemCurves::iterator iter = m_ownedItemCurves.find(className);

		if(iter == m_ownedItemCurves.end())
		{
			// not found; add new entry
			CLayout::TmMOwnedSlotCurves tempSlotMap;

			TmMOwnedItemCurves::value_type tempPair(className, tempSlotMap);
			iter = m_ownedItemCurves.insert(m_ownedItemCurves.end(), tempPair);

			if(iter == m_ownedItemCurves.end())
			{
				// insertion failed
				return false;
			}

			// clear the newly entered map
			iter->second.clear();
		}

		(iter->second).add_element(curveId, keyframeList);
	}

	// Successful insertion
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCKeyFrameList* NdhsCTemplate::CLayout::getSlotCurve(int fromSlot, int toSlot)
{
	NdhsCKeyFrameList* pCurve = NULL;
	TmMSlotCurves::iterator iter;

	int curveId = makeCurveId(fromSlot, toSlot);

	iter = slotCurves().find(curveId);

	if (iter != slotCurves().end())
	{
		pCurve = iter->second;
	}

	return pCurve;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCKeyFrameList* NdhsCTemplate::CLayout::getItemCurve(const LcTmString& className,
															int fromSlot,
															int toSlot)
{
	NdhsCKeyFrameList* pCurve = NULL;
	TmMItemCurves::iterator iter1;
	TmMSlotCurves::iterator iter2;

	if(className.isEmpty())
	{
		return NULL;
	}

	int curveId = makeCurveId(fromSlot, toSlot);

	iter1 = itemCurves().find(className);

	if (iter1 != itemCurves().end())
	{
		iter2 = iter1->second.find(curveId);

		if (iter2 != iter1->second.end())
		{
			pCurve = iter2->second;
		}
	}

	return pCurve;
}

/*-------------------------------------------------------------------------*//**
*/
LcTPlacement* NdhsCTemplate::CLayout::getPlacementForClass(ENdhsObjectType elementUse,
																int slot,
																const LcTmString& className,
																int& mask,
																bool& ishide,
																bool& unloadElement)
{
	LcTPlacement* pRetVal = NULL;
	mask = 0;

	// Default to hiding / not unloading if class is not found
	ishide = true;
	unloadElement = false;

	switch(elementUse)
	{
		case ENdhsObjectTypeFurniture:
		{
			LcTmMap<LcTmString, CLayout::TmElementLayout>::iterator it;

			it = furnitureLayouts.find(className.toLower());
			if (it != furnitureLayouts.end())
			{
				if (it->second.hide == false)
				{
					pRetVal = &it->second.layout;
					mask = it->second.mask;
				}

				ishide = it->second.hide;
				unloadElement = ishide && it->second.unload;
			}

			break;
		}

		case ENdhsObjectTypeItem:
		{
			if (slot >= 0 && slot < (int)itemLayouts.size())
			{
				CLayout::CItemLayout* itemLayout = itemLayouts[slot];
				if (itemLayout)
				{
					LcTmMap<LcTmString, CLayout::TmElementLayout>::iterator it = itemLayout->elementLayouts.find(className.toLower());
					if (it != itemLayout->elementLayouts.end())
					{
						if (it->second.hide == false)
						{
							pRetVal = &it->second.layout;
							mask = it->second.mask;
						}

						ishide = it->second.hide;
						unloadElement = ishide && it->second.unload;
					}
				}
			}

			break;
		}

		case ENdhsObjectTypeFurnitureComponent:
		{
			// In case component is a furniture element
			LcTmMap<LcTmString, CLayout::TmElementLayout>::iterator it;

			it = furnitureLayouts.find(className.toLower());
			if (it != furnitureLayouts.end())
			{
				if (it->second.hide == false)
				{
					pRetVal = &it->second.layout;
					mask = it->second.mask;
				}

				ishide = it->second.hide;
				unloadElement = ishide && it->second.unload;
			}

			break;
		}

		case ENdhsObjectTypeItemComponent:
		{
			// In case component is an item element
			if (slot >= 0 && slot < (int)itemLayouts.size())
			{
				CLayout::CItemLayout* itemLayout = itemLayouts[slot];
				if (itemLayout)
				{
					LcTmMap<LcTmString, CLayout::TmElementLayout>::iterator it = itemLayout->elementLayouts.find(className.toLower());
					if (it != itemLayout->elementLayouts.end())
					{
						if (it->second.hide == false)
						{
							pRetVal = &it->second.layout;
							mask = it->second.mask;
						}

						ishide = it->second.hide;
						unloadElement = ishide && it->second.unload;
					}
				}
			}

			break;
		}

		default:
			break;
	}

	return pRetVal;
}

/*-------------------------------------------------------------------------*//**
*/
const LcTmArray<NdhsCTemplate::CDisplacement*>* NdhsCTemplate::CLayout::getDisplacements(ENdhsObjectType		elementUse,
																							int					slot,
																							const LcTmString&	className)
{
	const LcTmArray<CDisplacement*>* pRet = NULL;

	switch (elementUse)
	{
		case ENdhsObjectTypeFurnitureComponent:
		{
			if (furnitureLayouts.find(className) != furnitureLayouts.end())
			{
				if (!(furnitureLayouts[className].displacementObjectArray.empty()))
					pRet = &(furnitureLayouts[className].displacementObjectArray);
			}
			break;
		}
		case ENdhsObjectTypeItemComponent:
		{
			if ((slot >= 0) && (slot < (int)itemLayouts.size()))
			{
				if (itemLayouts[slot] && itemLayouts[slot]->elementLayouts.find(className) != itemLayouts[slot]->elementLayouts.end())
				{
					if (!(itemLayouts[slot]->elementLayouts[className].displacementObjectArray.empty()))
						pRet = &(itemLayouts[slot]->elementLayouts[className].displacementObjectArray);
				}
			}
			break;
		}
		case ENdhsObjectTypeFurniture:
		{
			if (furnitureLayouts.find(className) != furnitureLayouts.end())
			{
				if (!(furnitureLayouts[className].displacementObjectArray.empty()))
					pRet = &(furnitureLayouts[className].displacementObjectArray);
			}
			break;
		}

		case ENdhsObjectTypeItem:
		{
			if ((slot >= 0) && (slot < (int)itemLayouts.size()))
			{
				if (itemLayouts[slot] && itemLayouts[slot]->elementLayouts.find(className) != itemLayouts[slot]->elementLayouts.end())
				{
					if (!(itemLayouts[slot]->elementLayouts[className].displacementObjectArray.empty()))
						pRet = &(itemLayouts[slot]->elementLayouts[className].displacementObjectArray);
				}
			}

			break;
		}

		default:
			break;
	}

	return pRet;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::CLayoutDecoration::CDecorationInfo::cleanupDecorationItemMap(LcTmOwnerMap<int, CDecorationItem>& itemMap)
{
	itemMap.clear();
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCTemplate::CLayoutDecoration::CDecorationInfo::~CDecorationInfo()
{
	typedef LcTmOwnerMap<int, CLayoutDecoration::CDecorationInfo::CFromSlot> TmMFromSlot;
	TmMFromSlot::iterator itSlot;

	cleanupDecorationItemMap(slotItemAnimations);

	itSlot = toSlotItemAnimations.begin();
	for (; itSlot != toSlotItemAnimations.end(); itSlot++)
	{
		CLayoutDecoration::CDecorationInfo::CFromSlot* fromSlot = itSlot->second;
		if (fromSlot)
		{
			cleanupDecorationItemMap(fromSlot->fromSlotItemAnimations);
			cleanupDecorationItemMap(fromSlot->offsetSlotItemAnimations);
		}
	}
}

#ifdef IFX_USE_PLUGIN_ELEMENTS
/*-------------------------------------------------------------------------*//**
	Comparison function called by sort
*/
bool itemCompare(NdhsCTemplate::TTabData a, NdhsCTemplate::TTabData b)
{
	return a.tabOrder < b.tabOrder;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCTemplate> NdhsCTemplate::create(NdhsCPageManager* pageManager, LcTmOwner<LcCXmlElem>& root)
{
	LcTaOwner<NdhsCTemplate> ref;
	ref.set(new NdhsCTemplate(pageManager, root));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCTemplate::NdhsCTemplate()
{
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCTemplate::NdhsCTemplate(NdhsCPageManager* pageManager, LcTmOwner<LcCXmlElem>& root)
{
	m_pageManager = pageManager;
	m_root = root;

	// First active of -1 used for inactive layout
	// If first active is not specified in the XML
	// file, leave it as -1
	m_firstActive = -1;

#ifdef NDHS_JNI_INTERFACE
	m_skipFirstActiveCheck = false;
#endif //NDHS_JNI_INTERFACE
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCTemplate::~NdhsCTemplate()
{
	cleanup();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::cleanup()
{
	//
	// Actions Element
	//
	m_actions.clear();

	// Clear the default select action
	m_defaultSelectAttempt.clear();

	// Clear the default back action
	m_defaultBackAttempt.clear();

	//
	// Layouts Element
	//
	m_layouts.clear();

	//
	// Classes Element
	//
	m_furnitureClasses.clear();

	//
	// Animations Element
	//
	m_animations.clear();

	//
	// Decorations Element
	//

	m_layoutDecorationsMap.clear();

	m_decorations.clear();

#ifdef IFX_USE_PLUGIN_ELEMENTS
	//
	// Tab Data
	//

	m_tabOrderList.clear();
#endif

	m_variableInfoList.clear();

	// Clear parameter list
	m_paramterMap.clear();

	// Clear stored displacements
	m_displacementObjectStore.clear();

	// clear event dump	
	m_eventDump.clear();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmArray<NdhsCTemplate::CAction::CAttempt*>*
NdhsCTemplate::getAttemptsForAction(const LcTmString& action)
{
	TmMActions::iterator itA = m_actions.find(action);
	if (itA != m_actions.end())
		return &(itA->second->attempts);

	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT NdhsCTemplate::CAction*
NdhsCTemplate::getAction(const LcTmString& action)
{
	TmMActions::iterator itA = m_actions.find(action);
	if (itA != m_actions.end())
		return (itA->second);

	return NULL;
}

/*-------------------------------------------------------------------------*//**
	Finds the tab order data for a particular index.
	Returns true if successful.
*/
bool NdhsCTemplate::getTabOrderData(int index, TTabData& tabData)
{
	// Find the data.
	if ((index >= 0) && (index < (int)m_tabOrderList.size()))
	{
		tabData = m_tabOrderList.at(index);
		return true;
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
	Finds the tab index for a class.
	Returns -1 if not found.
*/
int NdhsCTemplate::getTabIndex(const LcTmString& className, ENdhsGroupType elementType)
{
	TmATabOrderList::iterator it = m_tabOrderList.begin();
	for (int i = 0; it != m_tabOrderList.end(); i++, it++)
	{
		if (((*it).className.compare(className) == 0)
			&& ((*it).elementType == elementType))
		{
			return i;
		}
	}

	return -1;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<NdhsCTemplate::CLayout> NdhsCTemplate::getMergedLayout(CLayout* unmergedLayout)
{
	if (!unmergedLayout)
		return CLayout::create(false, this);

	LcTaOwner<CLayout> mergedLayout = CLayout::create(false, this);
	CLayout* targetLayout = unmergedLayout;

	// We have the target layout, now lets merge down the hierarchy

	// First, build up a list of layouts to merge (max 10)

	LcTaArray<CLayout*> ml;
	ml.push_back(targetLayout);

	CLayout* tl = targetLayout;
	bool done = false;
	int i;
	for (i = 0; i < 10 && !done; i++)
	{
		if (tl->inherits.length() <= 0)
		{
			// No inheritance, so done
			done = true;
		}
		else
		{
			TmMLayouts::iterator itL = m_layouts.find(tl->inherits);
			if (itL == m_layouts.end())
			{
				// Layout doesn't exist, so were done
				done = true;
			}
			else
			{
				tl = itL->second;
				ml.push_back(tl);
			}
		}
	}

	// Finally, lets merge the layouts
	for (i = (int)ml.size() - 1; i >= 0; i--)
	{
		mergeLayouts(*mergedLayout, (ml[i]));
	}

	mergedLayout->name = unmergedLayout->name;
	ml.clear();

	setDefaultValues(*mergedLayout);

	return mergedLayout;
}

/*-------------------------------------------------------------------------*//**
	Applies layout2 on to layout1
*/
LC_EXPORT void NdhsCTemplate::mergeLayouts(CLayout& layout1, CLayout* layout2)
{

	if (layout2 == NULL)
		return;

	//
	// Curve
	//

	// layout1 must not be a master layout
	LC_ASSERT(layout1.isMaster() == false);

	// merge slot curves

	CLayout::TmMSlotCurves::iterator slotIter2 = layout2->slotCurves().begin();

	for(; slotIter2 != layout2->slotCurves().end(); slotIter2++)
	{
		layout1.slotCurves()[slotIter2->first] = slotIter2->second;
	}

	//  merge item curves

	CLayout::TmMItemCurves::iterator itemIter1;
	CLayout::TmMItemCurves::iterator itemIter2 = layout2->itemCurves().begin();

	for(; itemIter2 != layout2->itemCurves().end(); itemIter2++)
	{
		itemIter1 = layout1.itemCurves().find(itemIter2->first);

		if(itemIter1 == layout1.itemCurves().end())       // no match found
		{
			// add new entry
			CLayout::TmMSlotCurves tempSlotMap;

			CLayout::TmMItemCurves::value_type tempVal(itemIter2->first, tempSlotMap);
			itemIter1 = layout1.itemCurves().insert(layout1.itemCurves().end(), tempVal);

			// clear the newly entered map
			itemIter1->second.clear();
		}

		// simply copy all items from the inner map

		slotIter2 = itemIter2->second.begin();

		for(; slotIter2 != itemIter2->second.end(); slotIter2++)
		{
			itemIter1->second[slotIter2->first] = slotIter2->second;
		}
	}

	//
	// Primary light
	//

	layout1.primaryLightLayout.mask |= layout2->primaryLightLayout.mask;
	layout1.primaryLightLayout.layout.assign(layout2->primaryLightLayout.layout, layout2->primaryLightLayout.mask);

	//
	// Page
	//

	layout1.pageLayout.mask |= layout2->pageLayout.mask;
	layout1.pageLayout.hide = true;
	layout1.pageLayout.layout.assign(layout2->pageLayout.layout, layout2->pageLayout.mask);
	layout1.pageLayout.mergeDisplacements(layout2->pageLayout);

	//
	// Menu
	//

	layout1.menuLayout.mask |= layout2->menuLayout.mask;
	layout1.menuLayout.hide = true;
	layout1.menuLayout.layout.assign(layout2->menuLayout.layout, layout2->menuLayout.mask);
	layout1.menuLayout.mergeDisplacements(layout2->menuLayout);

	//
	// Furniture
	//

	LcTmMap<LcTmString, CLayout::TmElementLayout>::iterator itE1;
	LcTmMap<LcTmString, CLayout::TmElementLayout>::iterator itE2 = layout2->furnitureLayouts.begin();
	for (; itE2 != layout2->furnitureLayouts.end(); itE2++)
	{
		CLayout::TmElementLayout& el = itE2->second;

		itE1 = layout1.furnitureLayouts.find(itE2->first);
		if (itE1 != layout1.furnitureLayouts.end())
		{
			// Merge
			itE1->second.hide = el.hide;
			itE1->second.unload = el.unload;
			itE1->second.mask |= el.mask;
			itE1->second.layout.assign(el.layout, el.mask);
			itE1->second.mergeDisplacements(el);
		}
		else
		{
			// Not in the destination map
			layout1.furnitureLayouts[itE2->first] = el;
		}
	}

	//
	// Item
	//

	// Make sure the item array is the correct size
	layout1.itemLayouts.resize(layout2->itemLayouts.size(), NULL);

	// Both itemLayouts arrays should be the same length
	for (int i = 0; i < (int)layout2->itemLayouts.size(); i++)
	{
		CLayout::CItemLayout* il1 = layout1.itemLayouts[i];
		CLayout::CItemLayout* il2 = layout2->itemLayouts[i];

		if (il2)
		{
			if (il1)
			{
				// Merge

				// Slot
				il1->slotLayout.mask |= il2->slotLayout.mask;
				il1->slotLayout.hide = true;
				il1->slotLayout.layout.assign(il2->slotLayout.layout, il2->slotLayout.mask);
				il1->slotLayout.mergeDisplacements(il2->slotLayout);


				// Items
				itE2 = il2->elementLayouts.begin();
				for (; itE2 != il2->elementLayouts.end(); itE2++)
				{
					CLayout::TmElementLayout& el = itE2->second;

					itE1 = il1->elementLayouts.find(itE2->first);
					if (itE1 != il1->elementLayouts.end())
					{
						// Merge
						itE1->second.hide = el.hide;
						itE1->second.mask |= el.mask;
						itE1->second.layout.assign(el.layout, el.mask);
						itE1->second.mergeDisplacements(el);
					}
					else
					{
						// Not in the destination map
						il1->elementLayouts[itE2->first] = el;
					}
				}
			}
			else
			{
				// Copy

				LcTaOwner<CLayout::CItemLayout> itemLayout = CLayout::CItemLayout::create(layout1.owner());
				itemLayout->slotLayout = il2->slotLayout;
				itemLayout->elementLayouts = il2->elementLayouts;
				layout1.itemLayouts[i] = itemLayout.release();
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Sets any default values in a fully merged layout that have not already been
	manually configured by the XML layout
*/
void NdhsCTemplate::setDefaultValues(CLayout& layout)
{
	// Set default primary light ambiance if not already set
	if (!(layout.primaryLightLayout.mask & LcTPlacement::EColor2))
	{
		layout.primaryLightLayout.mask |= LcTPlacement::EColor2;
		layout.primaryLightLayout.layout.color2 = LcTColor::GRAY50;
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCTemplate::CElement * NdhsCTemplate::getElementClass(LcTaString elementClassName, ENdhsObjectType elementType)
{
	TmMElements* pElements = NULL;
	if (elementType == ENdhsObjectTypeItem || elementType == ENdhsObjectTypeItemComponent)
	{
		pElements = &m_itemClasses;
	}
	else if (elementType == ENdhsObjectTypeFurniture || elementType == ENdhsObjectTypeFurnitureComponent)
	{
		pElements = &m_furnitureClasses;
	}

	if (pElements)
	{
		TmMElements::iterator iter = pElements->find(elementClassName);
		if(iter != pElements->end())
			return iter->second;
		else
			return NULL;
	}

	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCElementGroup>
NdhsCTemplate::createElementGroup(	NdhsCElementGroup*	page,
									NdhsCMenu*			menu,
									NdhsCMenuItem*		menuItem,
									const LcTmString&	groupName,
									int					stackLevel,
									int					drawLayerIndex,
									NdhsCElementGroup*	parentGroup)
{
	LcTaOwner<NdhsCElementGroup> newGroup;
	bool parentUnloaded=page->isParentUnloaded();

	TmMGroups::iterator group = m_groupClasses.find(groupName);

	if (group != m_groupClasses.end())
	{
		newGroup = NdhsCElementGroup::create(groupName, page, menu, menuItem, drawLayerIndex);

		TmMElements* pElements = NULL;
		// iterate through the class list
		if (group->second->groupType == ENdhsObjectTypeItem)
		{
			pElements = &m_itemClasses;
			newGroup->setGroupType(ENdhsObjectTypeItem);

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
			// Set the Item Group's OpenGL Render Quality settings
			newGroup->setOpenGLRenderQualitySetting (group->second->openGLRenderQuality);
#endif
		}
		else if (group->second->groupType == ENdhsObjectTypeFurniture)
		{
			pElements = &m_furnitureClasses;
			newGroup->setGroupType(ENdhsObjectTypeFurniture);

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
			// Set the Furniture Group's OpenGL Render Quality settings
			newGroup->setOpenGLRenderQualitySetting (group->second->openGLRenderQuality);
#endif
		}

#ifdef LC_USE_LIGHTS
		newGroup->setLightModel(group->second->lightModel);
		if (parentGroup != NULL)
			newGroup->applyLightModel(parentGroup->getLightModel());
#endif
		if (pElements)
		{
			TmMElements::iterator elemIt = pElements->begin();

			for (; elemIt != pElements->end(); elemIt++)
			{
				if (elemIt->second->elementParent.compareNoCase(groupName) == 0
					&& elemIt->second->elementType != ENdhsElementTypeComponent)
				{
					LcTaOwner<NdhsCElement> newElement;
					CElement* pElement = pElements->operator[](elemIt->first);

					// Create element
					newElement = createElement(elemIt->first,
												pElement,
												group->second->groupType,
												page,
												menu,
												menuItem,
												stackLevel,
												newGroup.ptr());

					if(newElement.ptr() != NULL)
					{
						// Add to element group
						newElement->setUnloaded(parentUnloaded);
						newGroup->addItem(elemIt->first, newElement);
					}
				}
				else if (elemIt->second->elementType == ENdhsElementTypeComponent
						&&	elemIt->second->elementParent.compareNoCase(groupName) == 0)
				{
					LcTaOwner<NdhsCComponent> comp;
					LcTaOwner<NdhsCElement> newElement;
					CElement* pElement = pElements->operator[](elemIt->first);

					// To avoid any potential problem later on
					if (pElement != NULL && ((CComponentElement*)(pElement))->componentFile != NULL)
					{
						if (((CComponentElement*)(elemIt->second))->templateType == ETemplateTypeComponent)
						{
							comp = NdhsCComponent::create(m_pageManager, ((CComponentElement*)pElement), page, stackLevel, menu, menuItem, newGroup.ptr());

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
							// Set OpenGL Render Quality settings
							comp->setOpenGLRenderQualitySetting (((CComponentElement*)pElement)->openGLRenderQuality);
#endif
						}
						else if (((CComponentElement*)(elemIt->second))->templateType == ETemplateTypeMenuComponent)
						{
							comp = NdhsCMenuComponent::create(m_pageManager, ((CComponentElement*)pElement), page, stackLevel, menu, menuItem, newGroup.ptr());

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
							// Set OpenGL Render Quality settings
							comp->setOpenGLRenderQualitySetting (((CComponentElement*)pElement)->openGLRenderQuality);
#endif
						}

						if (comp.ptr() != NULL)
						{
							newGroup->addGroup(elemIt->first, (LcTaOwner<NdhsCElementGroup>&)comp);
						}
					}
				}
			}
		}

		// Now go through the group list and create and attach any children
		TmMGroups::iterator grIt = m_groupClasses.begin();

		for (; grIt != m_groupClasses.end(); grIt++)
		{
			if ((grIt->second->groupType != ENdhsObjectTypeFurnitureComponent && grIt->second->groupType != ENdhsObjectTypeItemComponent)
					&&(group->second->groupType == grIt->second->groupType)
					&& (groupName.compareNoCase(grIt->second->parentGroup) == 0))
			{
				LcTaOwner<NdhsCElementGroup> newElementGroup;

				// Create sub-group
				newElementGroup = createElementGroup(	page,
														menu,
														menuItem,
														grIt->first,
														stackLevel,
														grIt->second->m_drawLayerIndex,
														newGroup.ptr());

				if (newElementGroup.ptr() != NULL)
				{
					// Add to element group
					newGroup->addGroup(grIt->first, newElementGroup);
				}
			}
		}
	}

	return newGroup;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCElement>
NdhsCTemplate::createElement(	const LcTmString&	className,
								CElement* 			pElement,
								ENdhsObjectType 	usage,
								NdhsCElementGroup*	page,
								NdhsCMenu*			menu,
								NdhsCMenuItem*		menuItem,
								int					stackLevel,
								NdhsCElementGroup*	parentGroup)
{
	LcTaOwner<NdhsCElement> newElement;

	if (pElement == NULL)
		return newElement;

	// What type of element are we creating?
	switch(pElement->elementType)
	{
		case ENdhsElementTypeText:
		{
			CTextElement* pTE = reinterpret_cast<CTextElement*> (pElement);
			LC_ASSERT(pTE);
			LcTaOwner<NdhsCTextElement> tempElement = NdhsCTextElement::create(usage, className, menu, menuItem, pTE, m_pageManager, page, stackLevel, m_paletteManifest, parentGroup);
			newElement = tempElement;

			break;
		}

		case ENdhsElementTypeGraphic:
		{
			CGraphicElement* pGE = reinterpret_cast<CGraphicElement*> (pElement);
			LC_ASSERT(pGE);
			LcTaOwner<NdhsCGraphicElement> tempElement = NdhsCGraphicElement::create(usage, className, menu, menuItem, pGE, m_pageManager, page, stackLevel, m_paletteManifest, parentGroup);
			newElement = tempElement;

			break;
		}

#ifdef IFX_USE_PLUGIN_ELEMENTS
		case ENdhsElementTypePlugin:
		{
			CPluginElement* pPE = reinterpret_cast<CPluginElement*> (pElement);
			LC_ASSERT(pPE);
			LcTaOwner<NdhsCPluginElement> tempElement = NdhsCPluginElement::create(usage, className, menu, menuItem, pPE, m_pageManager, page, stackLevel, parentGroup);
			newElement = tempElement;

			break;
		}
#endif

#ifdef LC_USE_STYLUS
		case ENdhsElementTypeDragRegion:
		{
			CDragRegionElement* pDE = reinterpret_cast<CDragRegionElement*> (pElement);
			LC_ASSERT(pDE);
			LcTaOwner<NdhsCDragRegionElement> tempElement = NdhsCDragRegionElement::create(usage, className, menu, menuItem, pDE, m_pageManager, page, stackLevel);
			newElement = tempElement;

			break;
		}
#endif //LC_USE_STYLUS

#ifdef LC_USE_LIGHTS
		case ENdhsElementTypeLight:
		{
			CSecondaryLightElement* pLE = reinterpret_cast<CSecondaryLightElement*> (pElement);
			LC_ASSERT(pLE);
			LcTaOwner<NdhsCLightElement> tempElement = NdhsCLightElement::create(usage, className, menu, menuItem, pLE, m_pageManager, page, stackLevel);
			newElement = tempElement;

			break;
		}
#endif //LC_USE_LIGHTS

		default:
		{
			// We should never get here
			LC_ASSERT(false);

			break;
		}
	}

	return newElement;
}

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCTemplate::configureCustomEffectFromXml(const LcTmString& effectFileName, LcTmString& effectName, NdhsCManifest* paletteMan, int stackLevel)
{
	bool status = false;
	LcTaString err;
	LcTaString fileName, usage, name, fName, makesTranslucent;
	LcTaString shaderPath, vertShaderPath, fragShaderPath;
	LcTaString effectNameFull;

	LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(effectFileName, err);

	if (root)
	{
		usage = root->getAttr(NDHS_TP_XML_USAGE);
		name = root->getAttr(NDHS_TP_XML_NAME);
		makesTranslucent = root->getAttr(NDHS_TP_XML_MAKES_TRANSLUCENT);

		if(makesTranslucent.compareNoCase("false") == 0)
		{
			makesTranslucent = "";
		}

		fName = name;
		name = effectFileName;
		effectName = name;

		m_customEffect = m_pageManager->getSpace()->getOglContext()->addCustomEffect(name);

		// Ensure that this custom effect exists but does not exist in the cache
		if (m_customEffect && !m_customEffect->isCached())
		{
			effectNameFull = effectFileName;

			m_customEffect->setEffectName(effectNameFull);
			m_customEffect->setUsage(usage);
			m_customEffect->setMakesTranslucent(!makesTranslucent.isEmpty());
		}
		else
		{
			return false;
		}

		LcTaArray<NdhsCManifest::CManifestFile*> fileData;

#ifndef NDHS_JNI_INTERFACE

		// Make shader file name of extension .vfbin
		int fLen = effectFileName.getWord(-1, '/').length();

		LcTaString unifiedBinaryFileName = effectFileName.subString(3,effectFileName.length()- fLen -3) + fName + ".vfbin";

		// First lookup unified binary shader file exist, if it fails then lookup for binary shader or source file
		if(m_pageManager->getManifestStack()->findFile(unifiedBinaryFileName, shaderPath, paletteMan, stackLevel, NULL, false, &fileData))
		{
			// Set it as unified binary
			m_customEffect->setUnifiedBinary(true);
			m_customEffect->setBinary(true);

			// Assigning vertex and fragment shader file and format.
			m_customEffect->setVShaderFile(shaderPath);
			m_customEffect->setBinaryFormat(fileData[0]->m_compiledShaderFormat);

			m_customEffect->setEffectSignature(fileData[0]->m_signature);

			status = true;
		}
		else

#endif /* NDHS_JNI_INTERFACE */
		{
			LcTaString vertexSourceFileName, fragmentSourceFileName;
			NdhsCEffect* effect = m_pageManager->getEffect();

			LcCXmlElem* vertexShader = root->find(NDHS_TP_XML_VERTEX_SHADER);
			LcCXmlElem* fragmentShader = root->find(NDHS_TP_XML_FRAGMENT_SHADER);

			if(vertexShader && fragmentShader)
			{
				// Vertex shader configuration
				status = effect->configureShaderFromXml(vertexShader, &vertexSourceFileName);

				if(status)
				{
					// Fragment shader configuration
					status = effect->configureShaderFromXml(fragmentShader, &fragmentSourceFileName);
				}
			}

			if(!status)
			{
				return false;
			}

			status = false;

			if (vertexSourceFileName.isEmpty() == 0 && fragmentSourceFileName.isEmpty() == 0)
			{
#ifndef NDHS_JNI_INTERFACE

				LcTaString extVert = vertexSourceFileName.getWord(-1, '.');
				LcTaString extFrag = fragmentSourceFileName.getWord(-1, '.');

				// Make shader file name with extension .vbin and .fbin
				LcTaString vertexBinaryFileName = vertexSourceFileName.subString(0, vertexSourceFileName.length() - extVert.length() - 1) + ".vbin";
				LcTaString fragmentBinaryFileName = fragmentSourceFileName.subString(0, fragmentSourceFileName.length() - extFrag.length() - 1) + ".fbin";

				// Lookup binary shader file, if it fails then lookup source file
				if(m_pageManager->getManifestStack()->findFile(vertexBinaryFileName, vertShaderPath, paletteMan, stackLevel, NULL, false, &fileData)
					&& m_pageManager->getManifestStack()->findFile(fragmentBinaryFileName, fragShaderPath, paletteMan, stackLevel))
				{
					// Set it as pre-compiled binary
					m_customEffect->setBinary(true);

					// Assigning vertex and fragment shader file and format.
					m_customEffect->setVShaderFile(vertShaderPath);
					m_customEffect->setFShaderFile(fragShaderPath);
					m_customEffect->setBinaryFormat(fileData[0]->m_compiledShaderFormat);

					m_customEffect->setEffectSignature(fileData[0]->m_signature);

					status = true;
				}
				else

#endif /* NDHS_JNI_INTERFACE */

				if(m_pageManager->getManifestStack()->findFile(vertexSourceFileName, vertShaderPath, paletteMan, stackLevel))
				{
					if(m_pageManager->getManifestStack()->findFile(fragmentSourceFileName, fragShaderPath, paletteMan, stackLevel, NULL, false, &fileData))
					{
						// Set as source shader
						m_customEffect->setBinary(false);

						// Assigning vertex and fragment shader file.
						m_customEffect->setVShaderFile(vertShaderPath);
						m_customEffect->setFShaderFile(fragShaderPath);

						m_customEffect->setEffectSignature(fileData[0]->m_signature);

						status = true;
					}
				}
			}
		}

		NdhsCEffect* customEffect = m_pageManager->getEffect();

		if(status)
		{
			LcCXmlElem* attributes = root->find(NDHS_TP_XML_ATTRIBUTES);
			status = customEffect->configureAttributesFromXml(attributes, m_customEffect);

			if(status)
			{
				LcCXmlElem* uniforms = root->find(NDHS_TP_XML_UNIFORMS);
				status = customEffect->configureUniformsFromXml(uniforms, m_customEffect);
			}
			// Load the effect
			if (status)
			{
				status = m_pageManager->getSpace()->getOglContext()->loadCustomEffect (name);
			}
		}
	}
	else
	{
		m_customEffect = NULL;
		status = false;
	}

	return status;
}

#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT bool NdhsCTemplate::configureFromXml(const LcTmString& designSize, int stackLevel, int& nestedComponentLevel)
{
	LcTaString err;
	bool result = false;
	bool success = true;

	cleanup();

	if (m_root)
	{
		// Loop simply allow us to break out if an error occurs
		do
		{
			//
			// Settings Element
			//

			if (false == (success = configureSettingsFromXml(m_root->find(NDHS_TP_XML_SETTINGS))))
			{
				break;
			}


			//
			// Actions Element
			//

			LcCXmlElem* eActions = m_root->find(NDHS_TP_XML_ACTIONS);
			if (eActions == NULL)
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Actions section missing");
			}

			LcCXmlElem* eAction = eActions ? eActions->getFirstChild() : NULL;
			for (; eAction && success; eAction = eAction->getNext())
			{
				if (eAction->getName().compareNoCase(NDHS_TP_XML_ACTION) != 0)
					continue;

				success = configureActionFromXml(eAction);
			}


			if (!success)
				break;


			// Configure any default actions
			success = configureDefaultActions();


			if (!success)
				break;

			//
			// Classes Element
			//

			if (false == (success = configureClassesFromXml(m_root->find(NDHS_TP_XML_CLASSES), stackLevel, nestedComponentLevel)))
			{
				break;
			}


			//
			// Layouts Element
			//

			LcCXmlElem* eLayouts = m_root->find(NDHS_TP_XML_LAYOUTS);
			LcCXmlElem* eLayout = eLayouts ? eLayouts->getFirstChild() : NULL;

			success = true;
			for (; eLayout && success; eLayout = eLayout->getNext())
			{
				if (eLayout->getName().compareNoCase(NDHS_TP_XML_LAYOUT) != 0)
					continue;

				success = configureLayoutFromXml(eLayout, stackLevel);
			}



			if (!success || !eLayouts)
			{
				NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Layouts section missing");
				success = false;
				break;
			}

			//
			// Animations Element
			//

			LcCXmlElem* eAnimations = m_root->find(NDHS_TP_XML_ANIMATIONS);
			LcCXmlElem* eAnimation = eAnimations ? eAnimations->getFirstChild() : NULL;
			for (; eAnimation && success; eAnimation = eAnimation->getNext())
			{
				if (eAnimation->getName().compareNoCase(NDHS_TP_XML_ANIMATION) != 0)
					continue;

				success = configureAnimationFromXml(eAnimation, stackLevel);
			}


			if (!success)
				break;

			//
			// Decorations Element
			//

			LcCXmlElem* eDecorations = m_root->find(NDHS_TP_XML_DECORATIONS);
			LcCXmlElem* rootNode = eDecorations ? eDecorations->getFirstChild() : NULL;
			for (; rootNode && success; rootNode = rootNode->getNext())
			{
				configureDecorationFromXml(rootNode);
				// Not critical if this fails, just go onto the next one
			}
			if (!success)
				break;

		} while (false);

		result = success;

		m_root.destroy();
	}


	if (result == false)
		cleanup();

	return result;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureDecorationFromXml(LcCXmlElem* decorationRoot)
{
	// Don't need a decorations section, so
	// just return true if there isn't one
	if (!decorationRoot)
		return true;

	LcTaOwner<CLayoutDecoration> initialState = CLayoutDecoration::create();

	if(configureDecorationTypesFromXml(decorationRoot, initialState.ptr()))
	{
		m_layoutDecorationsMap.push_back(initialState);
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCTemplate::configureDecorationTypesFromXml(LcCXmlElem* eInitialState, CLayoutDecoration* initialState)
{
	typedef enum {
		ETypeStateChange = 0,
		ETypeItemSlot,
		ETypeStatic,
		ETypeTransition
	} EType;

	EType type;

	// Look for child state changes
	LcCXmlElem* eDecorationType = eInitialState;

	LcTaString decorationType = eDecorationType->getName().toLower();

	LcTaOwner<CLayoutDecoration::CDecorationInfo> info = CLayoutDecoration::CDecorationInfo::create();
	CLayoutDecoration::CDecorationInfo* infoPtr = info.ptr();


	if(decorationType.compareNoCase(NDHS_TP_XML_TRANSITION) == 0)
	{
		type = ETypeTransition;

		LcTaString toLayout = "";
		LcCXmlAttr* attr = eDecorationType->findAttr(NDHS_TP_XML_TO_LAYOUT);
		if (attr)
		{
			toLayout = attr->getVal();

			if(toLayout.length()<=0)
			{
				toLayout = "";
			}
		}

		attr = eDecorationType->findAttr(NDHS_TP_XML_LAYOUT);
		if (!attr)
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration's transition has no from layout specified");

			// Not a valid element, so carry on to the next one
			info.destroy();
			return false;
		}

		LcTaString fromLayout = attr->getVal();

		if(fromLayout.length()<=0)
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration's layout name invalid");

			// Not a valid element, so carry on to the next one
			info.destroy();
			return false;
		}

		initialState->layout=toLayout;
		initialState->fromLayout=fromLayout;

		// timing

		LcCXmlElem* eTiming = eDecorationType->find(NDHS_TP_XML_TIMING);
		if (eTiming)
		{
			info->timingDataSet = true;

			// A '-1' value denotes that this field is not present.
			info->timingData.delay				= eTiming->getAttr(NDHS_TP_XML_DELAY, "0").toInt();
			info->timingData.velocityProfile	= strToVelocityProfile(eTiming->getAttr(NDHS_TP_XML_VELOCITY_PROFILE).toLower());

			if (info->timingData.delay < 0)
			{
				info->timingData.delay = 0;

				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Timing delay for a terminal transition must not be negative");
			}

			attr = eTiming->findAttr(NDHS_TP_XML_DURATION);

			if (attr)
			{
				info->timingData.duration = attr->getVal().toInt();

				if (info->timingData.duration < 0)
				{
					// Set to -1 so that we use the default
					info->timingData.duration = -1;

					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Timing duration for a terminal transition must not be negative");
				}
			}
			else
			{
				// Set to -1 so that we use the default
				info->timingData.duration = -1;
			}

			attr = eTiming->findAttr(NDHS_TP_XML_BACKGROUND_DELAY);

			if (attr)
			{
				info->timingData.backgroundDelay = attr->getVal().toInt();

				if (info->timingData.backgroundDelay < 0)
				{
					// Set to -1 so that we use the default
					info->timingData.backgroundDelay = -1;

					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Background delay for a terminal transition must not be negative");
				}
			}
			else
			{
				// Set to -1 so that we use the default
				info->timingData.backgroundDelay = -1;
			}

			attr = eTiming->findAttr(NDHS_TP_XML_PRIMARY_LIGHT_DELAY);

			if (attr)
			{
				info->timingData.primaryLightDelay = attr->getVal().toInt();

				if (info->timingData.primaryLightDelay < 0)
				{
					// Set to -1 so that we use the default
					info->timingData.primaryLightDelay = -1;

					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Primary light delay for a terminal transition must not be negative");
				}
			}
			else
			{
				// Set to -1 so that we use the default
				info->timingData.primaryLightDelay = -1;
			}

			attr = eTiming->findAttr(NDHS_TP_XML_PRIMARY_LIGHT_DURATION);

			if (attr)
			{
				info->timingData.primaryLightDuration = attr->getVal().toInt();

				if (info->timingData.primaryLightDuration < 0)
				{
					// Set to -1 so that we use the default
					info->timingData.primaryLightDuration = -1;

					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Primary light duration for a terminal transition must not be negative");
				}
			}
			else
			{
				// Set to -1 so that we use the default
				info->timingData.primaryLightDuration = -1;
			}
		}

		initialState->stateDecorations.push_back(info);


	}
	else if (decorationType.compareNoCase(NDHS_TP_XML_SCROLL) == 0)
	{
		LcCXmlAttr* attr = eDecorationType->findAttr(NDHS_TP_XML_LAYOUT);
		if (!attr)
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration's has no layout specified");
			return false;
		}

		LcTaString strLayout = attr->getVal();

		initialState->layout=strLayout;

		type = ETypeItemSlot;

		if (!initialState->scrollDecoration)
		{
			initialState->scrollDecoration = info;
		}
		else
		{
			// Discard any duplicates
			info.destroy();
			return false;
		}
	}
	else if (decorationType.compareNoCase(NDHS_TP_XML_STATIC) == 0)
	{
		LcCXmlAttr* attr = eDecorationType->findAttr(NDHS_TP_XML_LAYOUT);
		if (!attr)
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration's has no layout specified");
			return false;
		}

		LcTaString strLayout = attr->getVal();

		initialState->layout=strLayout;

		if (!initialState->staticDecoration)
		{
			type = ETypeStatic;
			initialState->staticDecoration = info;
		}
		else
		{
			// If a second static is found, just ignore it
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "You can only have one 'static' within an initial state in the decorations section");

			info.destroy();
			return false;
		}
	}
	else
	{
		// Not a valid element, so carry on to the next one
		info.destroy();
		return false;
	}

	// Look for the child components
	LcCXmlElem* eComponent = eDecorationType->getFirstChild();

	for (; eComponent; eComponent = eComponent->getNext())
	{
		LcTaString component = eComponent->getName().toLower();

		if (component.compareNoCase(NDHS_TP_XML_PAGE) == 0)
		{
			LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

			if (configureDecorationRefFromXml(eComponent, type == ETypeStatic, animRef.ptr()))
			{
				infoPtr->furnitureAnimations.add_element("page", animRef);
			}
		}
		else if (component.compareNoCase(NDHS_TP_XML_MENU) == 0)
		{
			LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

			if (configureDecorationRefFromXml(eComponent, type == ETypeStatic, animRef.ptr()))
			{
				infoPtr->furnitureAnimations.add_element("menu", animRef);
			}
		}
		else if (component.compareNoCase(NDHS_TP_XML_FURNITURE) == 0)
		{
			LcCXmlElem* eElement = eComponent->getFirstChild();

			for (; eElement; eElement = eElement->getNext())
			{
				LcTaString element = eElement->getName().toLower();

				if (element.compareNoCase(NDHS_TP_XML_ELEMENT) == 0)
				{
					LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

					if (configureDecorationRefFromXml(eElement, type == ETypeStatic, animRef.ptr()))
					{
						LcTaString elementClass = eElement->getAttr(NDHS_TP_XML_CLASS).toLower();

						if (elementClass.length() > 0)
						{
							infoPtr->furnitureAnimations.add_element(elementClass, animRef);
						}
						else
						{
							infoPtr->defaultFurnitureAnimation.destroy();
							infoPtr->defaultFurnitureAnimation = animRef;
						}
					}
				}
			}
		}
		else if(component.compareNoCase(NDHS_TP_XML_OUTER_GROUP) == 0)
		{
			LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

			if (configureDecorationRefFromXml(eComponent, type == ETypeStatic, animRef.ptr()))
			{
				infoPtr->outerGroupAnimations=animRef;
			}
		}
		else if (component.compareNoCase(NDHS_TP_XML_ITEMS) == 0)
		{
			// Look for child state changes
			LcCXmlElem* eItem = eComponent->getFirstChild();

			for (; eItem; eItem = eItem->getNext())
			{
				LcTaString item = eItem->getName().toLower();

				if (item.compareNoCase(NDHS_TP_XML_DEFAULTS) == 0)
				{
					// Look for child state changes
					LcCXmlElem* eElement = eItem->getFirstChild();

					for (; eElement; eElement = eElement->getNext())
					{
						LcTaString element = eElement->getName().toLower();

						if (element.compareNoCase(NDHS_TP_XML_SLOT) == 0)
						{
							LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

							if (configureDecorationRefFromXml(eElement, type == ETypeStatic, animRef.ptr()))
							{
								infoPtr->defaultItemAnimations->elementAnimations.add_element("slot", animRef);
							}
						}
						else if (element.compareNoCase(NDHS_TP_XML_ELEMENT) == 0)
						{
							LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

							if (configureDecorationRefFromXml(eElement, type == ETypeStatic, animRef.ptr()))
							{
								LcTaString elementClass = eElement->getAttr(NDHS_TP_XML_CLASS).toLower();

								if (elementClass.length() > 0)
								{
									infoPtr->defaultItemAnimations->elementAnimations.add_element(elementClass, animRef);
								}
								else
								{
									infoPtr->defaultItemAnimations->defaultElementAnimation.destroy();
									infoPtr->defaultItemAnimations->defaultElementAnimation = animRef;
								}
							}
						}
					}
				}
				else if (item.compareNoCase(NDHS_TP_XML_ITEM) == 0)
				{
					LcTaOwner<CLayoutDecoration::CDecorationInfo::CDecorationItem> decorationItem = CLayoutDecoration::CDecorationInfo::CDecorationItem::create();
					CLayoutDecoration::CDecorationInfo::CDecorationItem* decorationItemPtr = decorationItem.ptr();

					LcCXmlAttr* attr = NULL;
					if ((type != ETypeItemSlot) && ((attr = eItem->findAttr(NDHS_TP_XML_SLOT)) != NULL))
					{
						LcTaString strSlot = attr->getVal();

						if (strSlot.compareNoCase(NDHS_TP_XML_ACTIVE) == 0)
						{
							if (infoPtr->slotItemAnimations.find(CLayoutDecoration::EActive) == infoPtr->slotItemAnimations.end())
							{
								infoPtr->slotItemAnimations.add_element(CLayoutDecoration::EActive, decorationItem);
							}
							else
							{
								decorationItem.destroy();
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's slot value has been specified twice");
								continue;
							}
						}
						else if (strSlot.compareNoCase(NDHS_TP_XML_TO_ACTIVE) == 0)
						{
							if (infoPtr->slotItemAnimations.find(CLayoutDecoration::EToActive) == infoPtr->slotItemAnimations.end())
							{
								infoPtr->slotItemAnimations.add_element(CLayoutDecoration::EToActive, decorationItem);
							}
							else
							{
								decorationItem.destroy();
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's slot value has been specified twice");
								continue;
							}
						}
						else if (strSlot.compareNoCase(NDHS_TP_XML_FROM_ACTIVE) == 0)
						{
							if (infoPtr->slotItemAnimations.find(CLayoutDecoration::EFromActive) == infoPtr->slotItemAnimations.end())
							{
								infoPtr->slotItemAnimations.add_element(CLayoutDecoration::EFromActive, decorationItem);
							}
							else
							{
								decorationItem.destroy();
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's slot value has been specified twice");
								continue;
							}
						}
						else
						{
							int slot = strSlot.toInt() - 1;

							if (slot >= 0  && slot < m_slotCount)
							{
								if (infoPtr->slotItemAnimations.find(slot) == infoPtr->slotItemAnimations.end())
								{
									infoPtr->slotItemAnimations.add_element(slot, decorationItem);
								}
								else
								{
									decorationItem.destroy();
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's slot value has been specified twice");
									continue;
								}
							}
							else
							{
								decorationItem.destroy();
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's slot value is invalid");
								continue;
							}
						}
					}
					else if (type != ETypeItemSlot)
					{
						decorationItem.destroy();
						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item has no slot specified");
						continue;
					}
					else
					{
						LcTaString strToSlot = eItem->getAttr(NDHS_TP_XML_TO_SLOT, "any");
						LcTaString strFromSlot = eItem->getAttr(NDHS_TP_XML_FROM_SLOT, "any");

						LcTmOwnerMap<int, CLayoutDecoration::CDecorationInfo::CFromSlot>::iterator itFromSlot;

						CLayoutDecoration::CDecorationInfo::CFromSlot* fromSlotInfo;

						if (strToSlot.compareNoCase(NDHS_TP_XML_ACTIVE) == 0)
						{
							if ((itFromSlot = infoPtr->toSlotItemAnimations.find(CLayoutDecoration::EActive)) == infoPtr->toSlotItemAnimations.end())
							{
								LcTaOwner<CLayoutDecoration::CDecorationInfo::CFromSlot> newFromSlot = CLayoutDecoration::CDecorationInfo::CFromSlot::create();
								fromSlotInfo = newFromSlot.ptr();
								infoPtr->toSlotItemAnimations.add_element(CLayoutDecoration::EActive, newFromSlot);
							}
							else
							{
								fromSlotInfo = itFromSlot->second;
							}
						}
						else if (strToSlot.compareNoCase(NDHS_TP_XML_ANY) == 0)
						{
							if ((itFromSlot = infoPtr->toSlotItemAnimations.find(CLayoutDecoration::ESlotAny)) == infoPtr->toSlotItemAnimations.end())
							{
								LcTaOwner<CLayoutDecoration::CDecorationInfo::CFromSlot> newFromSlot = CLayoutDecoration::CDecorationInfo::CFromSlot::create();
								fromSlotInfo = newFromSlot.ptr();
								infoPtr->toSlotItemAnimations.add_element(CLayoutDecoration::ESlotAny, newFromSlot);
							}
							else
							{
								fromSlotInfo = itFromSlot->second;
							}
						}
						else
						{
							int slot = strToSlot.toInt() - 1;

							if (slot >= 0  && slot < m_slotCount)
							{
								if ((itFromSlot = infoPtr->toSlotItemAnimations.find(slot)) == infoPtr->toSlotItemAnimations.end())
								{
									LcTaOwner<CLayoutDecoration::CDecorationInfo::CFromSlot> newFromSlot = CLayoutDecoration::CDecorationInfo::CFromSlot::create();
									fromSlotInfo = newFromSlot.ptr();
									infoPtr->toSlotItemAnimations.add_element(slot, newFromSlot);
								}
								else
								{
									fromSlotInfo = itFromSlot->second;
								}
							}
							else
							{
								decorationItem.destroy();
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's toSlot value is invalid");
								continue;
							}
						}

						if (strFromSlot.compareNoCase(NDHS_TP_XML_ACTIVE) == 0)
						{
							if (fromSlotInfo->fromSlotItemAnimations.find(CLayoutDecoration::EActive) == fromSlotInfo->fromSlotItemAnimations.end())
							{
								fromSlotInfo->fromSlotItemAnimations.add_element(CLayoutDecoration::EActive, decorationItem);
							}
							else
							{
								decorationItem.destroy();
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's toSlot/fromSlot pair has been specified twice");
								continue;
							}
						}
						else if (strFromSlot.compareNoCase(NDHS_TP_XML_ANY) == 0)
						{
							if (fromSlotInfo->fromSlotItemAnimations.find(CLayoutDecoration::ESlotAny) == fromSlotInfo->fromSlotItemAnimations.end())
							{
								fromSlotInfo->fromSlotItemAnimations.add_element(CLayoutDecoration::ESlotAny, decorationItem);
							}
							else
							{
								decorationItem.destroy();
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's toSlot/fromSlot pair has been specified twice");
								continue;
							}
						}
						else if (strFromSlot.subString(0, 1).compare("+") == 0
							|| strFromSlot.subString(0, 1).compare("-") == 0)
						{
							int offset = strFromSlot.toInt();

							if (fromSlotInfo->offsetSlotItemAnimations.find(offset) == fromSlotInfo->offsetSlotItemAnimations.end())
							{
								fromSlotInfo->offsetSlotItemAnimations.add_element(offset, decorationItem);
							}
							else
							{
								decorationItem.destroy();
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's toSlot/fromSlot pair has been specified twice");
								continue;
							}
						}
						else
						{
							int slot = strFromSlot.toInt() - 1;

							if (slot >= 0  && slot < m_slotCount)
							{
								if (fromSlotInfo->fromSlotItemAnimations.find(slot) == fromSlotInfo->fromSlotItemAnimations.end())
								{
									fromSlotInfo->fromSlotItemAnimations.add_element(slot, decorationItem);
								}
								else
								{
									decorationItem.destroy();
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's toSlot/fromSlot pair has been specified twice");
									continue;
								}
							}
							else
							{
								decorationItem.destroy();
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's fromSlot value is invalid");
								continue;
							}
						}
					}

					// Look for child state changes
					LcCXmlElem* eElement = eItem->getFirstChild();

					for (; eElement; eElement = eElement->getNext())
					{
						LcTaString element = eElement->getName().toLower();

						if (element.compareNoCase(NDHS_TP_XML_SLOT) == 0)
						{
							LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

							if (configureDecorationRefFromXml(eElement, type == ETypeStatic, animRef.ptr()))
							{
								decorationItemPtr->elementAnimations.add_element("slot", animRef);
							}
						}
						else if (element.compareNoCase(NDHS_TP_XML_ELEMENT) == 0)
						{
							LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

							if (configureDecorationRefFromXml(eElement, type == ETypeStatic, animRef.ptr()))
							{
								LcTaString elementClass = eElement->getAttr(NDHS_TP_XML_CLASS).toLower();

								if (elementClass.length() > 0)
								{
									decorationItemPtr->elementAnimations.add_element(elementClass, animRef);
								}
								else
								{
									decorationItemPtr->defaultElementAnimation.destroy();
									decorationItemPtr->defaultElementAnimation = animRef;
								}
							}
						}
					}
				}
			}
		}
		else if (component.compareNoCase(NDHS_TP_XML_TRIGGERS) == 0)
		{
			infoPtr->triggersSet = true;

			if (type == ETypeStatic)
			{
				// Static trigger decorations have additional attributes
				LcTaString duration = eComponent->getAttr(NDHS_TP_XML_DURATION);

				if(!duration.isEmpty())
				{
					int d = duration.toInt();
					if (d > 0)
					{
						infoPtr->triggers->transitionTime = d;
					}
					else
					{
						// duration is mandatory, so ignore the triggers altogether
						infoPtr->triggersSet = false;
						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'duration' attribute of 'trigger' is not valid");
						continue;
					}
				}
				else
				{
					// duration is mandatory, so ignore the triggers altogether
					infoPtr->triggersSet = false;
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'trigger' for a static animation is missing the mandatory attribute 'duration'");
					continue;
				}

				LcTaString loop = eComponent->getAttr(NDHS_TP_XML_LOOP, "1");

				if(loop.compareNoCase("infinite") == 0)
				{
					infoPtr->triggers->loopCount = LcCLinearAnimator::EInfiniteLoop;
				}
				else
				{
					int l = loop.toInt();
					if (l > 0)
					{
						infoPtr->triggers->loopCount = l;
					}
					else
					{
						// Default to one loop in error case
						infoPtr->triggers->loopCount = 1;

						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'triggers' loop count must not be negative");
					}
				}
			}

			// Look for cleanup trigger
			LcTaString cleanupTrigger = eComponent->getAttr(NDHS_TP_XML_CLEANUP_TRIGGER).toLower();

			if (cleanupTrigger.length() > 0)
			{
				LcTmArray<int>* keyCodes = m_pageManager->getKeyCodes(cleanupTrigger);

				if (keyCodes != NULL)
				{
					// There may be more that one scan code associated
					// with the same key name but they'll all resolve to
					// the same action, so we just store the first in the
					// array
					if (keyCodes->size() > 0)
						infoPtr->triggers->cleanupTrigger = (*keyCodes)[0];
				}
				else
				{
					bool catchAll;

					(void) convertKeyToInt(cleanupTrigger, infoPtr->triggers->cleanupTrigger, catchAll);
				}

				infoPtr->triggers->hasCleanupTrigger = true;
			}
			else
			{
				infoPtr->triggers->hasCleanupTrigger = false;
			}

			// Look for triggers
			LcCXmlElem* eTrigger = eComponent->getFirstChild();

			for (; eTrigger; eTrigger = eTrigger->getNext())
			{
				if (eTrigger->getName().compareNoCase(NDHS_TP_XML_TRIGGER) == 0)
				{
					LcTaString key = eTrigger->getAttr(NDHS_TP_XML_KEY).toLower();
					int convertedKey = 0;
					LcTScalar position = eTrigger->getAttr(NDHS_TP_XML_POSITION, "-1").toScalar();

					if (key.length() > 0)
					{
						LcTmArray<int>* keyCodes = m_pageManager->getKeyCodes(key);

						if (keyCodes != NULL)
						{
							// There may be more that one scan code associated
							// with the same key name but they'll all resolve to
							// the same action, so we just store the first in the
							// array
							if (keyCodes->size() > 0)
								convertedKey = (*keyCodes)[0];
						}
						else
						{
							bool catchAll;

							if ((convertKeyToInt(key, convertedKey, catchAll) == false) || (catchAll == true))
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "decoration trigger has an invalid key specified");
							}
						}
					}
					else
					{
						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "decoration trigger is missing the mandatory attribute 'key'");
					}

					if (position > -1)
					{
						if ((position > 100) || (position < 0))
						{
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'position' attribute of 'trigger' cannot be less than 0 or greater than 100");
							continue;
						}

						infoPtr->triggers->addTrigger(convertedKey, position / 100.0f);
					}
					else
					{
						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "decoration trigger is missing the mandatory attribute 'position'");
					}
				}
			}
			// Also sort the trigger list
			if (infoPtr->triggers)
				IFX_ShellSort(infoPtr->triggers->triggerList.begin(), infoPtr->triggers->triggerList.end(), triggerCompare);
		}
	}
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureSettingsFromXml(LcCXmlElem* eSettings)
{
	if (!eSettings)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Settings section missing");
		return false;
	}

	LcTaString templateType = eSettings->getAttr(NDHS_TP_XML_TEMPLATE_TYPE);
	m_templateType = determineTemplateType(templateType);

#ifdef LC_USE_LIGHTS
	m_lightModel = ENdhsLightModelNormal;
	LcTaString lightModel = eSettings->getAttr(NDHS_TP_XML_LIGHT_MODEL, "normal");
	if (lightModel.compareNoCase("simple") == 0)
	{
		m_lightModel = ENdhsLightModelSimple;
	}
#endif

	// Bounding box default to 0,0,0
	m_boundingBox = LcTVector(0,0,0);

	// Configure bounding box
	configureBoundingBox(eSettings->find(NDHS_TP_XML_BOUNDING_BOX));

	// Configure timing information
	configureTimingFromXML(eSettings->find(NDHS_TP_XML_TIMING));

	// Configure focus information
	configureFocusSettingsFromXML(eSettings->find(NDHS_TP_XML_FOCUS));

	// Configure variables now
	configureVariablesFromXML(eSettings->find(NDHS_TP_XML_VARIABLES));

	// Configure parameters now
	configureParameterFromXML(eSettings->find(NDHS_TP_XML_PARAMETERS));

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureBoundingBox(LcCXmlElem* eBoundingBox)
{
	// Check bounding box is present
	if (eBoundingBox)
	{
		m_boundingBox.x = eBoundingBox->getAttr(NDHS_TP_XML_X, "0").toScalar();
		m_boundingBox.y = eBoundingBox->getAttr(NDHS_TP_XML_Y, "0").toScalar();
		m_boundingBox.z = eBoundingBox->getAttr(NDHS_TP_XML_Z, "0").toScalar();
	}
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureVariablesFromXML(LcCXmlElem* eVars)
{
	if (eVars)
	{
		LcCXmlElem* eVar  = eVars->getFirstChild();

		for (; eVar; eVar = eVar->getNext())
		{
			bool varError = false;
			LcTaOwner<CVariableInfo> varInfo = CVariableInfo::create();

			varInfo->name = eVar->getAttr(NDHS_TP_XML_NAME).toLower();

			if (!isValidVariableName(varInfo->name))
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Variable name missing or invalid");
				varError = true;
			}

			if (!varError)
			{
				// Check varaible for uniqueness
				if (checkNameUniqueness(varInfo->name) == false)
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Varaible with same name already exists");
					// Move to next varaible
					continue;
				}
			}

			if (!varError)
			{
				LcTaString typeStr = eVar->getAttr(NDHS_TP_XML_DATA_TYPE, "string").toLower();

				if (typeStr.compare("string") == 0)
				{
					varInfo->type = IFXI_FIELDDATA_STRING;
				}
				else if (typeStr.compare("int") == 0)
				{
					varInfo->type = IFXI_FIELDDATA_INT;
				}
				else if (typeStr.compare("float") == 0)
				{
					varInfo->type = IFXI_FIELDDATA_FLOAT;
				}
				else if (typeStr.compare("boolean") == 0)
				{
					varInfo->type = IFXI_FIELDDATA_BOOL;
				}
				else if (typeStr.compare("time") == 0)
				{
					varInfo->type = IFXI_FIELDDATA_TIME;
				}
				else
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Unknown variable type");
					varError = true;
				}
			}

			if (!varError)
			{
				varInfo->boundExpression = NdhsCExpression::CExprSkeleton::create(eVar->getAttr(NDHS_TP_XML_EXPRESSION));
				varInfo->defaultValue = NdhsCExpression::CExprSkeleton::create(eVar->getAttr(NDHS_TP_XML_DEFAULT_VALUE));

				if (!varInfo->boundExpression->isEmpty() && !varInfo->defaultValue->isEmpty())
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Cannot both bind an expression and supply a default value to a variable");
					varError = true;
				}
			}

			if (!varError)
			{
				m_variableInfoList.push_back(varInfo);
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureParameterFromXML(LcCXmlElem* eParams)
{
	if (eParams)
	{
		LcCXmlElem* eParam  = eParams->getFirstChild();

		for (; eParam; eParam = eParam->getNext())
		{
			LcTaString attr;
			LcTaOwner<CParameters> param = CParameters::create();

			if (param)
			{
				param->name = eParam->getAttr(NDHS_TP_XML_NAME).toLower();

				if (!isValidVariableName(param->name))
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Parameter name missing or invalid");
					// Move to next parameter
					continue;
				}

				// Verify there is no varaible with current parameter name
				if (checkNameUniqueness(param->name) == false)
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Parameter name conflict with varaible name");
					// Move to next parameter
					continue;
				}

				if (m_paramterMap.find(param->name) != m_paramterMap.end())
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Parameter name already exists");
					// Move to next parameter
					continue;
				}

				attr = eParam->getAttr(NDHS_TP_XML_MODE, "inputoutput").toLower();

				if (attr.isEmpty())
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Parameter mode missing");
					// Move to next parameter
					continue;
				}

				if (attr.compareNoCase("inputoutput") == 0)
				{
					param->mode = IFXI_FIELD_MODE_INPUT_OUTPUT;
				}
				else if (attr.compareNoCase("input") == 0)
				{
					param->mode  = IFXI_FIELD_MODE_OUTPUT;
				}
				else
				{
					// Move to next parameter
					continue;
				}

				LcTaString attr = eParam->getAttr(NDHS_TP_XML_DATA_TYPE, "string").toLower();

				if (attr.compare("string") == 0)
				{
					param->type = IFXI_FIELDDATA_STRING;
				}
				else if (attr.compare("int") == 0)
				{
					param->type = IFXI_FIELDDATA_INT;
				}
				else if (attr.compare("float") == 0)
				{
					param->type = IFXI_FIELDDATA_FLOAT;
				}
				else if (attr.compare("boolean") == 0)
				{
					param->type = IFXI_FIELDDATA_BOOL;
				}
				else if (attr.compare("time") == 0)
				{
					param->type = IFXI_FIELDDATA_TIME;
				}
				else
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Unknown variable type");
					// Move to next parameter
					continue;
				}

				// Parameters scope is local
				param->scope = IFXI_FIELD_SCOPE_LOCAL;

				param->defaultValue = NdhsCExpression::CExprSkeleton::create(eParam->getAttr(NDHS_TP_XML_DEFAULT_VALUE));

				// We are here because we had no error
				m_paramterMap.add_element(param->name, param);
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::checkNameUniqueness(LcTmString& name)
{
	TmMVariableInfoList::iterator varIt = m_variableInfoList.begin();

	for (; varIt != m_variableInfoList.end(); varIt++)
	{
		if (((CVariableInfo*)(*varIt))->name.compareNoCase(name) == 0)
		{
			return false;
		}
	}
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureTimingFromXML(LcCXmlElem* eTiming)
{
	if (eTiming)
	{
		// Layout, Scroll and Terminal Transition Times
		m_defaultLayoutTime   = eTiming->getAttr(NDHS_TP_XML_LAYOUT_TIME, "0").toInt();
		m_defaultScrollTime   = eTiming->getAttr(NDHS_TP_XML_SCROLL_TIME, "0").toInt();
		m_defaultTerminalTime = eTiming->getAttr(NDHS_TP_XML_TERMINAL_TIME, "0").toInt();

		if (m_defaultLayoutTime < 0)
		{
			m_defaultLayoutTime = 0;

			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "layoutTime cannot be negative");
		}

		if (m_defaultScrollTime < 0)
		{
			m_defaultScrollTime = 0;

			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "scrollTime cannot be negative");
		}

		if (m_defaultTerminalTime < 0)
		{
			m_defaultTerminalTime = 0;

			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "terminalTime cannot be negative");
		}

		// Default velocity profiles
		m_defaultLayoutVelocityProfile	= strToVelocityProfile(eTiming->getAttr(NDHS_TP_XML_LAYOUT_VELOCITY_PROFILE, "linear").toLower());
		m_defaultScrollVelocityProfile	= strToVelocityProfile(eTiming->getAttr(NDHS_TP_XML_SCROLL_VELOCITY_PROFILE, "linear").toLower());
		m_defaultTerminalVelocityProfile = strToVelocityProfile(eTiming->getAttr(NDHS_TP_XML_TERMINAL_VELOCITY_PROFILE, "linear").toLower());
	}
	else
	{
		return false;
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureFocusSettingsFromXML(LcCXmlElem* eFocus)
{
	if (eFocus)
	{
		m_focusStop = LcCXmlAttr::strToBool(eFocus->getAttr(NDHS_TP_XML_FOCUS_STOP, "true"));
		m_defaultFocus = eFocus->getAttr(NDHS_TP_XML_DEFAULT_FOCUS, "").toLower();
	}
	else
	{
		m_focusStop = true;
		m_defaultFocus = "";
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureActionFromXml(LcCXmlElem* eAction)
{
	// Get the action name - mandatory
	LcTaString actionName = eAction->getAttr(NDHS_TP_XML_NAME).toLower();
	if (actionName.length() <= 0)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Action name is missing");
		return false;
	}

	if (m_actions.find(actionName) != m_actions.end())
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Action name already exists");

		// Return true so that it still loads and just uses the first action of that name
		return true;
	}

	LcTaOwner<CAction> action = CAction::create();

	// We could end up with an action  with no attempts,
	// but that wont cause a problem

	// Look for child tags of this action, and add them in order
	// (order is important in determining precedence of actions to take!)
	LcCXmlElem* eAttempt = eAction->getFirstChild();

	if (eAttempt == NULL)
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Action has no attempts specified");
	}

	for (; eAttempt; eAttempt = eAttempt->getNext())
	{
		configureAttemptFromXml(eAttempt, action.ptr());
	}

	m_actions.add_element(actionName, action);

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::configureAttemptFromXml(LcCXmlElem* eAttempt, CAction* action)
{
	LcTaString attemptName = eAttempt->getName().toLower();

	if (attemptName.compare(NDHS_TP_XML_SCROLL_BY) == 0)
	{
		LcTScalar amount = eAttempt->getAttr(NDHS_TP_XML_AMOUNT, "0").toScalar();

		// Fail if amount is zero, which does nothing
		if (amount != 0)
		{
			LcTaOwner<CAction::CScrollBy> attempt = CAction::CScrollBy::create();

			attempt->attemptType = CAction::CAttempt::EScrollBy;
			attempt->amount = amount;
			attempt->action	= eAttempt->getAttr(NDHS_TP_XML_ACTION).toLower();
			attempt->velocityProfile = strToVelocityProfile(eAttempt->getAttr(NDHS_TP_XML_VELOCITY_PROFILE).toLower());

			// Retrieve the duration.
			LcCXmlAttr* attr = eAttempt->findAttr(NDHS_TP_XML_DURATION);
			if (attr)
			{
				attempt->duration = attr->getVal().toInt();

				if (attempt->duration < 0)
				{
					// Set to -1 so that we use the default
					attempt->duration = -1;
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "ScrollBy duration must not be negative");
				}
			}
			else
			{
				// Set to -1 so that we use the default
				attempt->duration = -1;
			}

			// Retrieve the field.
			attr = eAttempt->findAttr(NDHS_TP_XML_FIELD);
			if (attr && (attr->getVal().isEmpty() == false))
			{
				attempt->field = attr->getVal().toLower();
			}
			else
			{
				// Set to the default _scrollpos.
				attempt->field = NDHS_TP_XML__SCROLLPOS;
			}

			attr = eAttempt->findAttr(NDHS_TP_XML_MIN);
			if (attr)
			{
				attempt->minDefined = true;
				attempt->minExpr = NdhsCExpression::CExprSkeleton::create(attr->getVal());
			}
			else
			{
				attempt->minDefined = false;
			}

			attr = eAttempt->findAttr(NDHS_TP_XML_MAX);
			if (attr)
			{
				attempt->maxDefined = true;
				attempt->maxExpr = NdhsCExpression::CExprSkeleton::create(attr->getVal());
			}
			else
			{
				attempt->maxDefined = false;
			}

			// Establish the scrollBy Type.
			LcTaString attrType = eAttempt->getAttr(NDHS_TP_XML_TYPE).toLower();
			if (!attrType.isEmpty())
			{
				if (0 == attrType.compare("normal"))
				{
					attempt->type = CAction::CScrollBy::EScrollByTypeNormal;
				}
				else if (0 == attrType.compare("kick"))
				{
					attempt->type = CAction::CScrollBy::EScrollByTypeKick;
				}
				else
				{
					attempt->type = CAction::CScrollBy::EScrollByTypeNormal;
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Invalid scrollBy type specified");
				}
			}
			else
			{
				// The default is normal.
				attempt->type = CAction::CScrollBy::EScrollByTypeNormal;
			}

			// Fill in the defaults for velocity profile and duration if unspecified
			if (ENdhsVelocityProfileUnknown == attempt->velocityProfile)
			{
				attempt->velocityProfile = m_defaultScrollVelocityProfile;
			}

			if (-1 == attempt->duration)
			{
				attempt->duration = m_defaultScrollTime;
			}

			action->attempts.push_back((LcTaOwner<CAction::CAttempt>&)attempt);
		}
		else
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'scrollBy' amount 0 is not permitted");
		}
	}
	else if (attemptName.compare(NDHS_TP_XML_BACK) == 0)
	{
		LcTaOwner<CAction::CBack> attempt = CAction::CBack::create();

		attempt->attemptType	= NdhsCTemplate::CAction::CAttempt::EBack;
		attempt->page			= eAttempt->getAttr(NDHS_TP_XML_PAGE).toLower();
		attempt->action			= eAttempt->getAttr(NDHS_TP_XML_ACTION).toLower();
		attempt->backToLevel	= -1;

		action->attempts.push_back((LcTaOwner<CAction::CAttempt>&)attempt);
	}
	else if (attemptName.compare(NDHS_TP_XML_LINK) == 0)
	{
		LcTaString uri = eAttempt->getAttr(NDHS_TP_XML_URI);

		if (uri.length() > 0)
		{
			LcTaOwner<CAction::CLink> attempt = CAction::CLink::create();

			attempt->attemptType = CAction::CAttempt::ELink;
			attempt->uri = NdhsCExpression::CExprSkeleton::create(uri);
			attempt->action	= eAttempt->getAttr(NDHS_TP_XML_ACTION).toLower();

			action->attempts.push_back((LcTaOwner<CAction::CAttempt>&)attempt);
		}
		else
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'link' action uri has no value specified");
		}
	}
	else if (attemptName.compare(NDHS_TP_XML_SUSPEND) == 0)
	{
		LcTaOwner<CAction::CAttempt> attempt = CAction::CAttempt::create();
		attempt->attemptType = CAction::CAttempt::ESuspend;

		action->attempts.push_back(attempt);
	}
	else if (attemptName.compare(NDHS_TP_XML_SIGNAL) == 0)
	{
		LcTaOwner<CAction::CAttempt> attempt = CAction::CAttempt::create();
		attempt->attemptType = CAction::CAttempt::ESignal;

		action->attempts.push_back(attempt);
	}
	else if (attemptName.compare(NDHS_TP_XML_EXIT) == 0)
	{
		LcTaOwner<CAction::CAttempt> attempt = CAction::CAttempt::create();
		attempt->attemptType = CAction::CAttempt::EExit;

		action->attempts.push_back(attempt);
	}
	else if (attemptName.compare(NDHS_TP_XML_SET_FOCUS) == 0)
	{
		LcTaString className = eAttempt->getAttr(NDHS_TP_XML_CLASS).toLower();

		LcTaOwner<CAction::CSetFocus> attempt = CAction::CSetFocus::create();
		attempt->attemptType = CAction::CAttempt::ESetFocus;
		attempt->className = className;
		attempt->action	= eAttempt->getAttr(NDHS_TP_XML_ACTION).toLower();

		action->attempts.push_back((LcTaOwner<CAction::CAttempt>&)attempt);
	}
	else if (attemptName.compare(NDHS_TP_XML_UNSET_FOCUS) == 0)
	{
		LcTaOwner<CAction::CUnsetFocus> attempt = CAction::CUnsetFocus::create();

		attempt->attemptType = CAction::CAttempt::EUnsetFocus;
		attempt->action	= eAttempt->getAttr(NDHS_TP_XML_ACTION).toLower();

		action->attempts.push_back((LcTaOwner<CAction::CAttempt>&)attempt);
	}
	else if (attemptName.compare(NDHS_TP_XML_MOVE_FOCUS) == 0)
	{
		int amount = eAttempt->getAttr(NDHS_TP_XML_AMOUNT, "0").toInt();
		bool wrap = LcCXmlAttr::strToBool(eAttempt->getAttr(NDHS_TP_XML_WRAP));

		// Fail if amount is zero, which does nothing
		if (amount == 0)
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'movefocus' amount 0 is not permitted");
		}
		else if (amount < -1 || amount > 1)
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'movefocus' amount must be 1 or -1");
		}
		else
		{
			LcTaOwner<CAction::CTabFocus> attempt = CAction::CTabFocus::create();

			attempt->attemptType = CAction::CAttempt::EMoveFocus;
			attempt->amount	= amount;
			attempt->wrap	= wrap;
			attempt->action	= eAttempt->getAttr(NDHS_TP_XML_ACTION).toLower();

			action->attempts.push_back((LcTaOwner<CAction::CAttempt>&)attempt);
		}
	}
	else if (attemptName.compare(NDHS_TP_XML_STOP) == 0)
	{
		LcTaOwner<CAction::CAttempt> attempt = CAction::CAttempt::create();
		attempt->attemptType = CAction::CAttempt::EStop;

		action->attempts.push_back(attempt);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureDefaultActions()
{
	// Setup an attempt for for the default select action
	LcTaOwner<CAction::CBack> backAttempt = CAction::CBack::create();
	backAttempt->attemptType = NdhsCTemplate::CAction::CAttempt::EBack;
	backAttempt->page = "";
	backAttempt->action = "";
	m_defaultBackAttempt.push_back((LcTaOwner<CAction::CAttempt>&)backAttempt);

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureStateInfoFromXml(LcCXmlElem* eState, CState* state, CLayout* layout)
{
	if (!eState)
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "States section missing");
		return false;
	}

	state->layout = layout;
	state->condition = NdhsCExpression::CExprSkeleton::create(eState->getAttr(NDHS_TP_XML_CONDITION, "true"));

	LcCXmlElem* eEvents = eState->find(NDHS_TP_XML_EVENT_HANDLER);

	if (eEvents)
	{
		LcCXmlElem* eTrigger = eEvents->getFirstChild();

		for (; eTrigger; eTrigger = eTrigger->getNext())
		{
			LcTaOwner<CEventInfo> eventInfo = CEventInfo::create();
			configureEventInfoFromXML(eTrigger, eventInfo.ptr());

			if (eTrigger->getName().compareNoCase(NDHS_TP_XML_ON_TAP) == 0)
			{
				// Try the slot attr first - it takes priority
				int slot = eTrigger->getAttr(NDHS_TP_XML_SLOT, "-1").toInt();

				if (slot == 0)
				{
					// Set the slot to -1 so that it is ignored
					slot = -1;
				}
				else if ( slot > 0 )
				{
					slot--;
				}

				if ((slot >= m_firstSelectable) && (slot <= m_lastSelectable))
				{
					state->tapSlotTriggers[slot] = eventInfo.ptr();
					m_eventDump.push_back(eventInfo);
				}
				else
				{
					// See if there is an element class specified
					LcTaString elementClass = eTrigger->getAttr(NDHS_TP_XML_CLASS).toLower();
					if (elementClass.length() > 0)
					{
						state->tapClassTriggers[elementClass] = eventInfo.ptr();
						m_eventDump.push_back(eventInfo);
					}
					else
					{
						// Only configure the catch-all if no slot was specified at all
						if (slot == -1)
						{
							state->catchAllStylusTap = eventInfo.ptr();
							m_eventDump.push_back(eventInfo);
						}
					}
				}
			}
			else if (eTrigger->getName().compareNoCase(NDHS_TP_XML_ON_SIGNAL) == 0)
			{
				int slot = eTrigger->getAttr(NDHS_TP_XML_SLOT, "-1").toInt();
				LcTaString elementClass = eTrigger->getAttr(NDHS_TP_XML_CLASS).toLower();

				if (slot == 0)
				{
					// Set the slot to -1 so that it is ignored
					slot = -1;
				}
				else if ( slot > 0 )
				{
					slot--;
				}

				if (slot >= 0 && slot < m_slotCount && elementClass.length() > 0)
				{
					LcTaOwner<CSlotClassTrigger> onSignalInfo = CSlotClassTrigger::create();
					onSignalInfo->slot = slot;
					onSignalInfo->elementClass = elementClass;
					onSignalInfo->eventInfo = eventInfo.ptr();

					state->signalSlotClassTriggers.push_back(onSignalInfo);
					m_eventDump.push_back(eventInfo);
				}
				else
				{
					if (slot >= 0 && slot < m_slotCount)
					{
						state->signalSlotTriggers[slot] = eventInfo.ptr();
						m_eventDump.push_back(eventInfo);
					}
					else
					{
						// See if there is an element class specified
						if (elementClass.length() > 0)
						{
							state->signalClassTriggers[elementClass] = eventInfo.ptr();
							m_eventDump.push_back(eventInfo);
						}
					}
				}
			}
			else if (eTrigger->getName().compareNoCase(NDHS_TP_XML_ON_PRESS) == 0)
			{
				// Try the slot attr first - it takes priority
				LcTaString key = eTrigger->getAttr(NDHS_TP_XML_KEY).toLower();

				if (key.length() > 0)
				{
					LcTmArray<int>* keyCodes = m_pageManager->getKeyCodes(key);

					if (keyCodes != NULL)
					{
						// There may be more that one scan code associated
						// with the same key name, so add them all

						typedef LcTmArray<int> TmAKeyCodes;
						TmAKeyCodes::iterator itKeyCode = keyCodes->begin();

						for (; itKeyCode != keyCodes->end(); itKeyCode++)
						{
							LcTaOwner<CEventInfo> localEventInfo = CEventInfo::create();
							configureEventInfoFromXML(eTrigger, localEventInfo.ptr());

							state->pressTriggers[(int)*itKeyCode] = localEventInfo.ptr();
							m_eventDump.push_back(localEventInfo);
						}
					}
					else
					{
						int convertedKey = 0;
						bool catchAll;

						if (convertKeyToInt(key, convertedKey, catchAll))
						{
							if (catchAll)
								state->catchAllKeyPress = eventInfo.ptr();
							else
								state->pressTriggers[convertedKey] = eventInfo.ptr();

							m_eventDump.push_back(eventInfo);
						}
						else
						{
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "''press' trigger has an invalid key specified");
						}
					}
				}
				else
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'press' trigger has no key specified");
				}
			}
		}
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::convertKeyToInt(LcTmString& key, int& convertedKey, bool& catchAll)
{
	bool retVal = true;
	// Assume not catchall
	catchAll = false;

	// Check to see if key is a scancode
	if ((key.length() > 5) && (key.subString(0,5).compare("scan_") == 0))
	{
		convertedKey = key.subString(5, key.length() - 5).toInt();
	}
	else
	{
		// Predefined key names
		if (key.compare("any")==0)
		{
			catchAll = true;
		}
		else if (key.compare("select")==0)
		{
			convertedKey = (int)ENdhsNavigationKeySelect;
		}
		else if (key.compare("back")==0)
		{
			convertedKey = ENdhsNavigationSoftKeyBack;
		}
		else if (key.compare("up")==0)
		{
			convertedKey = (int)ENdhsNavigationKeyUp;
		}
		else if(key.compare("down")==0)
		{
			convertedKey = (int)ENdhsNavigationKeyDown;
		}
		else if(key.compare("right")==0)
		{
			convertedKey = ENdhsNavigationKeyRight;
		}
		else if(key.compare("left")==0)
		{
			convertedKey = (int)ENdhsNavigationKeyLeft;
		}
		else if(key.compare("#")==0)
		{
			convertedKey = (int)ENdhsNavigationKeyHash;
		}
		else if(key.compare("*")==0)
		{
			convertedKey = (int)ENdhsNavigationKeyAsterisk;
		}
		else if(key.toInt() >= 0 && key.toInt() <= 9)
		{
			// +48 so always stored in as standard ASCII codes for numbers
			convertedKey = key.toInt() + 48;
		}
		else
		{
			retVal = false;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureLayoutFromXml(LcCXmlElem* eLayout, int stackLevel)
{
	LcTaString name = eLayout->getAttr(NDHS_TP_XML_NAME).toLower();

	if (name.length() <= 0)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Layout has no name specified");
		return false;
	}

	if (m_layouts.find(name) != m_layouts.end())
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Layout name already exists");

		// Return true so that it still loads and just uses the first layout of that name
		return true;
	}

	// Create the new layout
	// it's the master copy, and owns the key-frame list pointers.
	LcTaOwner<CLayout> layout = CLayout::create(true, this);

	configureStateInfoFromXml(eLayout, layout->stateInfo.ptr(), layout.ptr());

	layout->name = name;
	layout->inherits = eLayout->getAttr(NDHS_TP_XML_INHERITS).toLower();

	//
	// Curves
	//
	LcCXmlElem* eCurves = eLayout->find(NDHS_TP_XML_CURVES);

	if(eCurves)
	{
		LcCXmlElem* eCurve = eCurves->getFirstChild();

		for(; eCurve; eCurve = eCurve->getNext())
		{
			if (eCurve->getName().compareNoCase(NDHS_TP_XML_CURVE) != 0)
				continue;

			configureCurveFromXml(eCurve, layout.ptr(), stackLevel);
		}
	}

	//
	// Primary light
	//

	LcCXmlElem* ePrimaryLight = eLayout->find(NDHS_TP_XML_PRIMARY_LIGHT);

	if (ePrimaryLight)
	{
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
		layout->primaryLightLayout.mask = layout->primaryLightLayout.layout.configureFromXml(m_customEffect, ePrimaryLight, LcTPlacement::EOrientation, false);
#else
		layout->primaryLightLayout.mask = layout->primaryLightLayout.layout.configureFromXml(NULL, ePrimaryLight, LcTPlacement::EOrientation, false);
#endif
		// Configure color
		if (getColorFromXML(ePrimaryLight, NDHS_TP_XML_COLOR, stackLevel, layout->primaryLightLayout.layout.color))
		{
			layout->primaryLightLayout.mask |= LcTPlacement::EColor;
		}

		// Configure color2
		if (getColorFromXML(ePrimaryLight, NDHS_TP_XML_COLOR2, stackLevel, layout->primaryLightLayout.layout.color2))
		{
			layout->primaryLightLayout.mask |= LcTPlacement::EColor2;
		}
	}

	layout->primaryLightLayout.hide = false;

	//
	// Screen
	//
	CLayout::TaElementLayout screenLayout(this);
	screenLayout.hide = false;
	layout->furnitureLayouts.operator[]("screen") = screenLayout;

	//
	// Page
	//
	CLayout::TaElementLayout pageLayout(this);

	LcCXmlElem* ePage = eLayout->find(NDHS_TP_XML_PAGE);
	if (ePage)
	{
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
		pageLayout.mask = pageLayout.layout.configureFromXml(m_customEffect, ePage, AGG_MASK, false);
#else
		pageLayout.mask = pageLayout.layout.configureFromXml(NULL, ePage, AGG_MASK, false);
#endif
		pageLayout.configureDisplacementsFromXml(ePage);
	}

	pageLayout.hide = false;
	layout->furnitureLayouts.operator[]("page") = pageLayout;

	//
	// OuterGroup
	//
	CLayout::TaElementLayout outerGroupLayout(this);

	LcCXmlElem* eOuterGroup = eLayout->find(NDHS_TP_XML_OUTER_GROUP);
	if (eOuterGroup)
	{
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
		outerGroupLayout.mask = outerGroupLayout.layout.configureFromXml(m_customEffect, eOuterGroup, AGG_MASK, false);
#else
		outerGroupLayout.mask = outerGroupLayout.layout.configureFromXml(NULL, eOuterGroup, AGG_MASK, false);
#endif
		outerGroupLayout.configureDisplacementsFromXml(eOuterGroup);
	}

	outerGroupLayout.hide = false;
	layout->furnitureLayouts.operator[]("outergroup") = outerGroupLayout;

	//
	// Menu
	//
	CLayout::TaElementLayout menuLayout(this);

	LcCXmlElem* eMenu = eLayout->find(NDHS_TP_XML_MENU);
	if (eMenu)
	{
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
		menuLayout.mask = menuLayout.layout.configureFromXml(m_customEffect, eMenu, AGG_MASK, false);
#else
		menuLayout.mask = menuLayout.layout.configureFromXml(NULL, eMenu, AGG_MASK, false);
#endif

		menuLayout.configureDisplacementsFromXml(eMenu);
	}

	menuLayout.hide = false;
	layout->furnitureLayouts.operator[]("menu") = menuLayout;

	// Detail elements
	CLayout::TaElementLayout detailLayout(this);
	detailLayout.hide = false;
	layout->furnitureLayouts.operator[]("detail") = detailLayout;

	//
	// Furniture
	//

	configureElementListFromXml(eLayout->find(NDHS_TP_XML_FURNITURE), ENdhsGroupTypeFurniture, layout.ptr(), NULL, stackLevel);

	//
	// Items
	//

	// Make sure the item array is the correct size
	layout->itemLayouts.resize(m_slotCount, NULL);

	LcCXmlElem* eItems = eLayout->find(NDHS_TP_XML_ITEMS);

	if (eItems)
	{
		// First, load in the defaults
		LcTaOwner<CLayout::CItemLayout> defaultItemLayout = CLayout::CItemLayout::create(this);
		if (configureItemFromXml(eItems->find(NDHS_TP_XML_DEFAULTS), defaultItemLayout.ptr(), stackLevel))
		{
			// If we have a default layout, we must assign it to all items
			for (int i = 0; i < m_slotCount; i++)
			{
				LcTaOwner<CLayout::CItemLayout> itemLayout = CLayout::CItemLayout::create(this);
				itemLayout->slotLayout = defaultItemLayout->slotLayout;
				itemLayout->elementLayouts = defaultItemLayout->elementLayouts;
				layout->itemLayouts[i] = itemLayout.release();
			}
		}

		// Now load the actual items
		LcCXmlElem* eItem = eItems ? eItems->getFirstChild() : NULL;

		for (; eItem; eItem = eItem->getNext())
		{
			if (eItem->getName().compareNoCase(NDHS_TP_XML_ITEM) != 0)
				continue;

			int slot = eItem->getAttr(NDHS_TP_XML_SLOT, "-1").toInt();

			if (slot == 0)
			{
				// Set the slot to -1 so that it is ignored
				slot = -1;
			}
			else if ( slot > 0 )
			{
				slot--;
			}


			if (slot >= 0  && slot < m_slotCount)
			{
				if (!layout->itemLayouts[slot])
				{
					LcTaOwner<CLayout::CItemLayout> itemLayout = CLayout::CItemLayout::create(this);
					layout->itemLayouts[slot] = itemLayout.release();
				}

				configureItemFromXml(eItem, layout->itemLayouts[slot], stackLevel);
			}
			else
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "The slot specified for an item in layouts must be between 1 and slot count");
			}
		}
	}

	m_layoutList.push_back(layout.ptr());
	m_layouts.add_element(name, layout);

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureItemFromXml(LcCXmlElem* eItem, CLayout::CItemLayout* itemLayout, int stackLevel)
{
	if (!eItem || !itemLayout)
		return false;

	// Get the default slot layout
	CLayout::TaElementLayout slotLayout(this);

	LcCXmlElem* eSlot = eItem->find(NDHS_TP_XML_SLOT);
	if (eSlot)
	{
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
		slotLayout.mask = slotLayout.layout.configureFromXml(m_customEffect, eSlot, AGG_MASK, false);
#else
		slotLayout.mask = slotLayout.layout.configureFromXml(NULL, eSlot, AGG_MASK, false);
#endif
		slotLayout.configureDisplacementsFromXml(eSlot);
	}

	slotLayout.hide = false;
	itemLayout->elementLayouts.operator[]("slot") = slotLayout;

	// Get the default item element layouts
	configureElementListFromXml(eItem, ENdhsGroupTypeItem, NULL, itemLayout, stackLevel);

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureElementListFromXml(LcCXmlElem* eElementList, ENdhsGroupType groupType,
													CLayout* layout, CLayout::CItemLayout* itemLayout, int stackLevel)
{
	// If the elements don't exist, its OK, so return true
	if (!eElementList)
		return true;

	LcTmMap<LcTmString, CLayout::TmElementLayout>* layoutList;

	switch (groupType)
	{
		case ENdhsGroupTypeItem:
		{
			layoutList = &itemLayout->elementLayouts;

			break;
		}

		case ENdhsGroupTypeFurniture:
		{
			layoutList = &layout->furnitureLayouts;

			break;
		}

		default:
			// Not much we can do, just return true
			return true;
	}

	LcCXmlElem* eElement = eElementList ? eElementList->getFirstChild() : NULL;

	for (; eElement; eElement = eElement->getNext())
	{
		LcTaString elementClass = eElement->getAttr(NDHS_TP_XML_CLASS).toLower();
		LcTaString elementName = eElement->getName().toLower();

		if (elementClass.length() > 0)
		{
			CLayout::TaElementLayout elementLayout(this);
			elementLayout.hide=false;

			LcTmMap<LcTmString, CLayout::TmElementLayout>::iterator it = layoutList->find(elementClass);

			if (it != layoutList->end())
				elementLayout = it->second;

			if (elementName.compare(NDHS_TP_XML_ELEMENT) == 0)
			{
				LcTPlacement tempPlacement;


#if defined(IFX_RENDER_DIRECT_OPENGL_20)
				TmMElements::iterator itClass;
				LcTaString effectName;
				LcOglCEffect *effect = NULL;

				switch (groupType)
				{
					case ENdhsGroupTypeItem:
					{
						itClass = m_itemClasses.find(elementClass);
						if (itClass != m_itemClasses.end())
						{
							if ((itClass->second)->elementType == ENdhsElementTypeText)
							{
								NdhsCTemplate::CTextElement *elem = (NdhsCTemplate::CTextElement *) itClass->second;
								effectName = elem->visualEffect;
								effect = m_pageManager->getSpace()->getOglContext()->getEffectByName(effectName);
							}
							else
							if ((itClass->second)->elementType == ENdhsElementTypeGraphic)
							{
								NdhsCTemplate::CGraphicElement *elem = (NdhsCTemplate::CGraphicElement *) itClass->second;
								effectName = elem->visualEffect;
								effect = m_pageManager->getSpace()->getOglContext()->getEffectByName(effectName);
							}
#ifdef IFX_USE_PLUGIN_ELEMENTS
							else
							if ((itClass->second)->elementType == ENdhsElementTypePlugin)
							{
								NdhsCTemplate::CPluginElement *elem = (NdhsCTemplate::CPluginElement *) itClass->second;
								effectName = elem->visualEffect;
								effect = m_pageManager->getSpace()->getOglContext()->getEffectByName(effectName);
							}
#endif
						}
						break;
					}

					case ENdhsGroupTypeFurniture:
					{
						itClass = m_furnitureClasses.find(elementClass);
						if (itClass != m_furnitureClasses.end())
						{
							if ((itClass->second)->elementType == ENdhsElementTypeText)
							{
								NdhsCTemplate::CTextElement *elem = (NdhsCTemplate::CTextElement *) itClass->second;
								effectName = elem->visualEffect;
								effect = m_pageManager->getSpace()->getOglContext()->getEffectByName(effectName);
							}
							else
							if ((itClass->second)->elementType == ENdhsElementTypeGraphic)
							{
								NdhsCTemplate::CGraphicElement *elem = (NdhsCTemplate::CGraphicElement *) itClass->second;
								effectName = elem->visualEffect;
								effect = m_pageManager->getSpace()->getOglContext()->getEffectByName(effectName);
							}
#ifdef IFX_USE_PLUGIN_ELEMENTS
							else
							if ((itClass->second)->elementType == ENdhsElementTypePlugin)
							{
								NdhsCTemplate::CPluginElement *elem = (NdhsCTemplate::CPluginElement *) itClass->second;
								effectName = elem->visualEffect;
								effect = m_pageManager->getSpace()->getOglContext()->getEffectByName(effectName);
							}
#endif
						}

						break;
					}

					default:
					break;
				}

				int mask = tempPlacement.configureFromXml(effect, eElement, LcTPlacement::EAll ^ (LcTPlacement::EColor | LcTPlacement::EColor2), true);
#else
				int mask = tempPlacement.configureFromXml(NULL, eElement, LcTPlacement::EAll ^ (LcTPlacement::EColor | LcTPlacement::EColor2), true);
#endif
				LcTaString strColor;

				LcCXmlAttr* attr = eElement->findAttr(NDHS_TP_XML_COLOR);
				if (attr)
				{
					strColor = attr->getVal();
				}

				if (strColor.length() == 0)
				{
					TmMElements::iterator itClass;

					switch (groupType)
					{
						case ENdhsGroupTypeItem:
						{
							itClass = m_itemClasses.find(elementClass);
							if (itClass != m_itemClasses.end())
							{
								if ((itClass->second)->elementType == ENdhsElementTypeText)
								{
									strColor = ((NdhsCTemplate::CTextElement*)(itClass->second))->font->color;
								}
							}

							break;
						}

						case ENdhsGroupTypeFurniture:
						{
							itClass = m_furnitureClasses.find(elementClass);
							if (itClass != m_furnitureClasses.end())
							{
								if ((itClass->second)->elementType == ENdhsElementTypeText)
								{
									strColor = ((NdhsCTemplate::CTextElement*)(itClass->second))->font->color;
								}
							}

							break;
						}

						default:
							break;
					}
				}

				if (strColor.length() != 0)
				{
					LcTaString strOutColor = "";
					m_pageManager->getTokenStack()->replaceTokens(strColor, strOutColor, NULL, NULL, NULL, NULL, stackLevel);

					// Check the color has a valid prefix before converting it.
					if (strOutColor.bufUtf8()[0] == '#')
					{
						tempPlacement.color = strOutColor.toInt();
						mask |= LcTPlacement::EColor;
					}
				}


				// Configure color2
				if (getColorFromXML(eElement, NDHS_TP_XML_COLOR2, stackLevel, tempPlacement.color2))
				{
					mask |= LcTPlacement::EColor2;
				}

				elementLayout.mask |= mask;
				elementLayout.layout.assign(tempPlacement, mask);

				// Check for any displacements
				elementLayout.configureDisplacementsFromXml(eElement);


				layoutList->operator[](elementClass) = elementLayout;
			}
			else if (elementName.compare(NDHS_TP_XML_HIDE) == 0)
			{
				// By default unload is false
				if (eElement->getAttr(NDHS_TP_XML_UNLOAD, "false").toLower().compare("true") == 0)
					elementLayout.unload = true;

				elementLayout.hide = true;

				layoutList->operator[](elementClass) = elementLayout;
			}

			// Don't add the the new element to the list here
			// because there might be a rogue element in the
			// xml section
		}
		else
		{
			if (elementName.compare(NDHS_TP_XML_SLOT) != 0)
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'element' in layouts section has no class name specified");
			}
		}
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureClassesFromXml(LcCXmlElem* eClasses, int stackLevel, int& nestedComponentLevel)
{
	m_furnitureClassNames.clear();
	m_itemClassNames.clear();

	if (!eClasses)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Classes section is missing");
		return false;
	}

	// Set the LAF defaults first
	LcTaOwner<CTextElement::CFont> font = CTextElement::CFont::create();

	font->style = m_pageManager->getDefaultFontStyle();
	font->color = m_pageManager->getDefaultFontColor();

	LcCXmlElem* eDefaults = eClasses->find(NDHS_TP_XML_DEFAULTS);
	if (eDefaults)
		configureFontFromXml(eDefaults->find(NDHS_TP_XML_FONT), font.ptr());

#ifdef IFX_USE_PLUGIN_ELEMENTS
	// Clear the tab order list.
	m_tabOrderList.clear();
#endif

	// Groups loaded first, as all subsequent elements with a parent will need to be attached to an
	// group
	configureGroupsFromXml(eClasses);
	configureElementClassesFromXml(eClasses->find(NDHS_TP_XML_ITEM), ENdhsGroupTypeItem, font.ptr(), stackLevel, nestedComponentLevel);
	configureElementClassesFromXml(eClasses->find(NDHS_TP_XML_FURNITURE), ENdhsGroupTypeFurniture, font.ptr(), stackLevel, nestedComponentLevel);

#ifdef IFX_USE_PLUGIN_ELEMENTS
	IFX_ShellSort(m_tabOrderList.begin(), m_tabOrderList.end(), itemCompare);
#endif

	return true;
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureDragRegionFromXml(LcCXmlElem* eDrag, CDragRegionElement::CDragRegionSetting* drag)
{
	// Not a problem if it doesn't exist
	if (!eDrag || !drag)
		return true;

	drag->field = eDrag->getAttr(NDHS_TP_XML_FIELD).toLower();

	// Not a problem if it doesn't exist
	if (drag->field.isEmpty())
		return true;

	drag->minValue = NdhsCExpression::CExprSkeleton::create(eDrag->getAttr(NDHS_TP_XML_MIN));
	drag->maxValue = NdhsCExpression::CExprSkeleton::create(eDrag->getAttr(NDHS_TP_XML_MAX));;

	// If sensitivity is missing or <= 0 axis is absolute
	LcCXmlAttr* attr = eDrag->findAttr(NDHS_TP_XML_SENSITIVITY);
	if (attr)
	{
		drag->sensitivity = attr->getVal().toScalar();
		if (drag->sensitivity <= 0)
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Sensitivity must be > 0");
			drag->isAbsolute = true;
		}
	}
	else
	{
		drag->isAbsolute = true;
	}

	drag->inverted = LcCXmlAttr::strToBool(eDrag->getAttr(NDHS_TP_XML_INVERTED, "false"));

	drag->threshold = eDrag->getAttr(NDHS_TP_XML_THRESHOLD, "0").toScalar();
	// Threshold must be >= 0
	if (drag->threshold < 0)
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Threshold must be >= 0");
		drag->threshold = 0;
	}
	else
	{
		// Scale threshold in line with current theme
		LcTScalar screenWidth = (LcTScalar)m_pageManager->getSpace()->getCanvasBounds().getWidth();
		LcTScalar globalWidth = m_pageManager->getSpace()->getGlobalExtent().x;

		if (globalWidth > 0)
		{
			LcTScalar scaleFactor = screenWidth / globalWidth;
			drag->threshold *= scaleFactor;
		}
	}

	drag->deceleration = NdhsCExpression::CExprSkeleton::create(eDrag->getAttr(NDHS_TP_XML_DECELERATION));
	drag->maxVelocity = NdhsCExpression::CExprSkeleton::create(eDrag->getAttr(NDHS_TP_XML_MAX_VELOCITY));

	drag->isDragEnabled = true;

	return true;
}
#endif // LC_USE_STYLUS

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureFontFromXml(LcCXmlElem* eFont, CTextElement::CFont* font)
{
	// Not a problem if it doesn't exist
	if (!eFont || !font)
		return true;

	font->face = NdhsCExpression::CExprSkeleton::create(eFont->getAttr(NDHS_TP_XML_FACE));

	LcTaString style = eFont->getAttr(NDHS_TP_XML_STYLE).toLower();
	if (style.length() > 0)
		font->style = LcCXmlAttr::strToFontStyle(style);

	LcTaString color = eFont->getAttr(NDHS_TP_XML_COLOR).toLower();
	if (color.length() > 0)
		font->color = color;

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureGroupsFromXml(LcCXmlElem* eClasses)
{
	// Add the 'default' groups for page, menu, slot, screen

	m_groupClassNames.push_back("screen");
	LcTaOwner<CGroup> newGroup = CGroup::create();
	m_groupClasses.add_element("screen", newGroup);

	m_groupClassNames.push_back("page");
	newGroup = CGroup::create();
	m_groupClasses.add_element("page", newGroup);

	m_groupClassNames.push_back("menu");
	newGroup = CGroup::create();
	newGroup->groupType = ENdhsObjectTypeFurniture;
	m_groupClasses.add_element("menu", newGroup);

	m_groupClassNames.push_back("outergroup");
	newGroup = CGroup::create();
	newGroup->groupType = ENdhsObjectTypeFurniture;
	m_groupClasses.add_element("outergroup", newGroup);

	m_groupClassNames.push_back("slot");
	newGroup = CGroup::create();
	newGroup->groupType = ENdhsObjectTypeItem;
	m_groupClasses.add_element("slot", newGroup);

	// Detail groups
	m_groupClassNames.push_back("detail_page");
	newGroup = CGroup::create();
	m_groupClasses.add_element("detail_page", newGroup);

	m_groupClassNames.push_back("detail_menu");
	newGroup = CGroup::create();
	m_groupClasses.add_element("detail_menu", newGroup);

	if (!eClasses)
		return true;

	// Now check for other groups...items and furniture
	for (int i=0; i<2; i++)
	{
		LcCXmlElem* eSubClasses = (i==0) ? eClasses->find(NDHS_TP_XML_ITEM) : eClasses->find(NDHS_TP_XML_FURNITURE);

		if (!eSubClasses)
			continue;

		LcCXmlElem* eClass =  eSubClasses->getFirstChild();

		for (; eClass; eClass = eClass->getNext())
		{
			if (eClass->getName().compareNoCase(NDHS_TP_XML_GROUP) != 0)
				continue;

			LcTaString groupClass = eClass->getAttr(NDHS_TP_XML_CLASS).toLower();

			if (groupClass.length() > 0)
			{
				if (m_groupClasses.find(groupClass) == m_groupClasses.end())
				{
					LcTaString groupParent = eClass->getAttr(NDHS_TP_XML_PARENT).toLower();

					LcTaOwner<CGroup> newGroup = CGroup::create();

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
					// Read in the OpenGL rendering quality settings (if specified) [normal/high]
					LcTaString openGLRenderQuality = eClass->getAttr(NDHS_TP_XML_OPENGL_RENDER_QUALITY, "normal").toLower();

					if (openGLRenderQuality.compare("high") == 0)
					{
						newGroup->openGLRenderQuality = "high";
					}
					else if (openGLRenderQuality.compare("normal") == 0)
					{
						newGroup->openGLRenderQuality = "normal";
					}
#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

					if (i==0)
						newGroup->groupType = ENdhsObjectTypeItem;

					if (groupParent.length() > 0)
					{
						newGroup->parentGroup = groupParent;
					}
					else
					{
						if (newGroup->groupType == ENdhsObjectTypeItem)
							newGroup->parentGroup = "slot";
						else
							newGroup->parentGroup = "page";
					}

#ifdef LC_USE_LIGHTS
					newGroup->lightModel = ENdhsLightModelNormal;
					LcTaString lightModel = eClass->getAttr(NDHS_TP_XML_LIGHT_MODEL, "normal");
					if (lightModel.compareNoCase("simple") == 0)
					{
						newGroup->lightModel = ENdhsLightModelSimple;
					}
#endif
					newGroup->m_drawLayerIndex = eClass->getAttr(NDHS_TP_XML_DRAW_PLANE, NDHS_LOWER_INVALID_DRAW_LAYER_INDEX_STRING).toInt();

					// Add it to the group names list
					// The groups name should only appear once in the
					// xml section
					m_groupClassNames.push_back(groupClass);

					m_groupClasses.add_element(groupClass, newGroup);
				}
				else
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Duplicate group class name specified in classes section");
				}
			}
			else
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'group' in classes section has no class name specified");
			}
		}
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureElementClassesFromXml(LcCXmlElem* eElementClasses, ENdhsGroupType groupType,
														CTextElement::CFont* defaultFont, int stackLevel, int& nestedComponentLevel )
{
	if (!eElementClasses || !defaultFont)
		return true;

	TmMElements* classes;
	TmAClasses* classNames;

	switch (groupType)
	{
		case ENdhsGroupTypeItem:
		{
			classes = &m_itemClasses;
			classNames = &m_itemClassNames;

			break;
		}

		case ENdhsGroupTypeFurniture:
		{
			classes = &m_furnitureClasses;
			classNames = &m_furnitureClassNames;

			break;
		}

		default:
			// Not much we can do, just return true
			return true;
	}

	LcCXmlElem* eElement = eElementClasses->getFirstChild();

	for (; eElement; eElement = eElement->getNext())
	{
		if (eElement->getName().find(NDHS_TP_XML_UID_CLASS_SETTINGS) == -1)
		{
			LcTaString elementClass = eElement->getAttr(NDHS_TP_XML_CLASS).toLower();

			if (elementClass.length() > 0)
			{
				if (classes->find(elementClass) == classes->end())
				{
					// Add it to the class names list
					// The class names should only appear once in the
					// xml section
					classNames->push_back(elementClass);

					LcTScalar tapTolerance = eElement->getAttr(NDHS_TP_XML_TAP_TOLERANCE, "0.0").toScalar();
					ETappable tappable = EPartial;

					// Detail and furniture do not support the tappable flag,
					// they are always tappable if a trigger is defined.
					// So if its an item, lets read the tappable from xml
					LcTaString tap = eElement->getAttr(NDHS_TP_XML_TAPPABLE, "partial");

					if (tap.compareNoCase("full") == 0)
						tappable = EFull;
					else if (tap.compareNoCase("off") == 0)
						tappable = EOff;

					bool isDetail = false;
					LcTaString detail = "";

					detail = eElement->getAttr(NDHS_TP_XML_DETAIL, "false");

					if (detail.compareNoCase("true") == 0)
						isDetail = true;

					LcTaString quality = eElement->getAttr(NDHS_TP_XML_QUALITY, "normal").toLower();

					// Calculate sketchy mode, defaults to allow sketchy.
					LcCWidget::ESketchyMode sketchyMode = LcCWidget::ESketchyAllowed;

					if (quality.compare("high") == 0)
						sketchyMode = LcCWidget::ESketchyDisabled;
					else if (quality.compare("low") == 0)
						sketchyMode = LcCWidget::ESketchyForced;

					// Get the parent group for the element
					LcTaString parent;

					if (groupType == ENdhsGroupTypeItem)
					{
						parent = eElement->getAttr(NDHS_TP_XML_PARENT, "slot").toLower();
					}
					// Detail element
					else if (groupType == ENdhsGroupTypeFurniture && isDetail)
					{
						LcTaString parentAttr = eElement->getAttr(NDHS_TP_XML_PARENT, "page").toLower();
						if (parentAttr.compare("menu") == 0)
							parent = "detail_menu";
						else
							parent = "detail_page";
					}
					else
					{
						parent = eElement->getAttr(NDHS_TP_XML_PARENT, "page").toLower();
					}

					///////////////////////////////////////////////////////
					// Text Element
					///////////////////////////////////////////////////////
					if (eElement->getName().compareNoCase("text") == 0)
					{
						LcTaOwner<CTextElement> newElem = CTextElement::create();
						CTextElement* pElement = newElem.ptr();
						LcTaOwner<CElement> element(newElem);

						// Read in the material specular color property (if specified)
						if (!getColorFromXML(eElement, NDHS_TP_XML_MATERIAL_SPECULAR_COLOR, stackLevel, pElement->materialSpecularColor))
						{
							pElement->materialSpecularColor = LcTColor::BLACK;
						}

						// Read in the material emissive color property (if specified)
						if (!getColorFromXML(eElement, NDHS_TP_XML_MATERIAL_EMISSIVE_COLOR, stackLevel, pElement->materialEmissiveColor))
						{
							pElement->materialEmissiveColor = LcTColor::BLACK;
						}

						// Material specular shininess factor (if specified)
						LcTaString materialSpecularShininess = eElement->getAttr(NDHS_TP_XML_MATERIAL_SPECULAR_SHININESS);

						if (materialSpecularShininess.isEmpty() == 0)
						{
							pElement->materialSpecularShininess = materialSpecularShininess.toScalar();
						}
						else
						{
							pElement->materialSpecularShininess = 0.0;
						}

	#if defined(IFX_RENDER_DIRECT_OPENGL_20)
						LcTaString atr = eElement->getAttr(NDHS_TP_XML_VISUAL_EFFECT);
						LcTaString effectPath;

						if (atr.isEmpty() == 0)
						{
							// Get palette manifest
							NdhsCManifest* paletteMan = m_pageManager->getPaletteManifest(atr);

							// Read effect from palette manifest
							if (m_pageManager->getManifestStack()->findFile(atr, effectPath, paletteMan, stackLevel))
							{
								configureCustomEffectFromXml(effectPath, pElement->visualEffect, paletteMan, stackLevel);
							}
						}

						// Read in the OpenGL rendering quality settings (if specified) [normal/high]
						LcTaString openGLRenderQuality = eElement->getAttr(NDHS_TP_XML_OPENGL_RENDER_QUALITY, "normal").toLower();

						if (openGLRenderQuality.compare("high") == 0)
						{
							pElement->openGLRenderQuality = "high";
						}
						else if (openGLRenderQuality.compare("normal") == 0)
						{
							pElement->openGLRenderQuality = "normal";
						}

						pElement->useCustomEffect = NdhsCExpression::CExprSkeleton::create(eElement->getAttr(NDHS_TP_XML_USE_CUSTOM_EFFECT, "true"));

	#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

						// Screen
						if (parent.compareNoCase("screen") == 0)
						{
							parent = "page";
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'parent' group 'screen' may only be applied to lights: defaulting to 'page'");
						}

						pElement->m_drawLayerIndex = eElement->getAttr(NDHS_TP_XML_DRAW_PLANE, NDHS_LOWER_INVALID_DRAW_LAYER_INDEX_STRING).toInt();
						pElement->elementType	= ENdhsElementTypeText;
						pElement->elementParent	= parent;
						pElement->isDetail		= isDetail;
						pElement->tappable		= tappable;
						pElement->tapTolerance	= tapTolerance;
						pElement->sketchyMode	= sketchyMode;
						pElement->antiAlias		= LcCXmlAttr::strToBool(eElement->getAttr(NDHS_TP_XML_ANTI_ALIAS, "false"));

#ifdef LC_USE_LIGHTS
						LcTaString lightModel = eElement->getAttr(NDHS_TP_XML_LIGHT_MODEL, "normal");
						if (lightModel.compareNoCase("simple") == 0)
						{
							pElement->lightModel = ENdhsLightModelSimple;
						}
#endif

						// This is used to tab focus around the controls.
						LcTaString tabOrderStr = eElement->getAttr(NDHS_TP_XML_FOCUS_ORDER);
						if (!tabOrderStr.isEmpty())
						{
							int tabOrder = tabOrderStr.toInt() - 1;
							if (tabOrder >= 0)
							{
								// Add the tab item to the list if it is valid.
								TTabData newData;
								newData.className	= elementClass;
								newData.elementType	= groupType;
								newData.tabOrder	= tabOrder;
								m_tabOrderList.push_back(newData);
							}
							else
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'focusOrder' index in classes section must be 1 or more");
							}
						}

						pElement->enableFocusExprSkel = NdhsCExpression::CExprSkeleton::create(eElement->getAttr(NDHS_TP_XML_ENABLE_FOCUS));

						pElement->font->height = eElement->getAttr(NDHS_TP_XML_HEIGHT, "0").toInt();
						pElement->font->hAlign = eElement->getAttr(NDHS_TP_XML_HALIGN, "center").toLower();
						pElement->font->vAlign = eElement->getAttr(NDHS_TP_XML_VALIGN, "center").toLower();

						// Validate the text alignments
						if (pElement->font->hAlign.compare("left") != 0
							&& pElement->font->hAlign.compare("center") != 0
							&& pElement->font->hAlign.compare("right") != 0)
						{
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "hAlign specified in classes section must be 'left', 'center' or 'right'");
							pElement->font->hAlign = "center";
						}

						if (pElement->font->vAlign.compare("top") != 0
							&& pElement->font->vAlign.compare("center") != 0
							&& pElement->font->vAlign.compare("bottom") != 0)
						{
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "vAlign specified in classes section must be 'top', 'center' or 'bottom'");
							pElement->font->vAlign = "center";
						}

						pElement->font->abbrevSuffix = eElement->getAttr(NDHS_TP_XML_ABBREV_SUFFIX, "...");

						pElement->font->style = defaultFont->style;
						pElement->font->color = defaultFont->color;
						configureFontFromXml(eElement->find(NDHS_TP_XML_FONT), pElement->font.ptr());

						LcTaString marq = eElement->getAttr(NDHS_TP_XML_VERTICAL_MARQUEE, "false");

						pElement->verticalMarquee = NdhsCExpression::CExprSkeleton::create(marq);

						marq = eElement->getAttr(NDHS_TP_XML_HORIZONTAL_MARQUEE, "false");

						pElement->horizontalMarquee = NdhsCExpression::CExprSkeleton::create(marq);

						pElement->marqueeSpeed = eElement->getAttr(NDHS_TP_XML_MARQUEE_SPEED, "60").toInt();

						LcTaString format = eElement->getAttr(NDHS_TP_XML_FORMAT);

						LcCXmlElem* eResource = eElement->find(NDHS_TP_XML_RESOURCE);
						if (eResource)
						{
							LcTaString resourceData = eResource->getAttr(NDHS_TP_XML_DATA);
							if(!format.isEmpty())
							{
								resourceData = "format(" + resourceData + ",\"" + format + "\")";
							}
							pElement->resourceData = NdhsCExpression::CExprSkeleton::create(resourceData);
	#ifdef IFX_USE_PLUGIN_ELEMENTS
							pElement->eventHandler = NdhsCExpression::CExprSkeleton::create(eResource->getAttr(NDHS_TP_XML_EVENT_HANDLER));
	#endif
						}

	#if defined (IFX_RENDER_DIRECT_OPENGL_20)

						LcCXmlElem* effectUniforms = eElement->find(NDHS_TP_XML_EFFECT_UNIFORMS);
						LcTaString name, type, uniformPath;
						bool bRequireManifestLookup = false;

						if(effectUniforms && m_customEffect)
						{
							LcCXmlElem* uniform = effectUniforms->getFirstChild();

							for(; uniform; uniform = uniform->getNext())
							{
								if (uniform->getName().compareNoCase("uniform") != 0)
									continue;

								name = uniform->getAttr(NDHS_TP_XML_NAME);
								type = uniform->getAttr(NDHS_TP_XML_UNIFORM_TYPE);

								// See if we need a manifest lookup
								bRequireManifestLookup = (type.compareNoCase("sampler2D") == 0) ||
														 (type.compareNoCase("samplerCube") == 0);

								LcTaString	values[6];
								LcTaString	value = "value#";

								// Default Values
								LcCXmlElem* child = uniform->find(NDHS_TP_XML_UNIFORM_VALUE);

								if(child)
								{
									for(int i = 0; i < 6; i++)
									{
										value = "value#";
										value.replace('#', ('0' + i+1));
										values[i] = child->getAttr(value);

										if (bRequireManifestLookup)
										{
											if (values[i].isEmpty() == 0)
											{
												// Get palette manifest
												NdhsCManifest* paletteMan = m_pageManager->getPaletteManifest(values[i]);

												LcTaArray<NdhsCManifest::CManifestFile*> fileData;

												if (m_pageManager->getManifestStack()->findFile(values[i],
																								uniformPath,
																								paletteMan,
																								stackLevel,
																								NULL,
																								true,
																								&fileData))
												{
													int index=-1;
													// Update uniform value with full path for later use during loading
													if(m_pageManager->findBitmapFile(&fileData,index))
													{
														values[i] = fileData[index]->absolutePath;
														fileData.clear();
													}
												}
											}
										}
									}
								}

								m_customEffect->addUniformFromTemplate(&pElement->effectUniMap, name, type, values);
							}
						}

	#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

						LcCXmlElem* eTeleport = eElement->find(NDHS_TP_XML_TELEPORT);
						if (eTeleport)
						{
							pElement->layoutTeleport = LcCXmlAttr::strToBool(eTeleport->getAttr(NDHS_TP_XML_LAYOUT));
							pElement->scrollTeleport = LcCXmlAttr::strToBool(eTeleport->getAttr(NDHS_TP_XML_SCROLL));
						}

						LcCXmlElem* eMeshGrid = eElement->find(NDHS_TP_XML_MESHGRID);
						if (eMeshGrid)
						{
							pElement->meshGridX = eMeshGrid->getAttr(NDHS_TP_XML_MESHGRID_X, "1").toInt();
							pElement->meshGridY = eMeshGrid->getAttr(NDHS_TP_XML_MESHGRID_Y, "1").toInt();

							if (pElement->meshGridX < 1)
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'meshGrid' X setting in classes section must be 1 or more");
								pElement->meshGridX = 1;
							}

							if (pElement->meshGridY < 1)
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'meshGrid' Y setting in classes section must be 1 or more");
								pElement->meshGridY = 1;
							}
						}
						else
						{
							pElement->meshGridX = 1;
							pElement->meshGridY = 1;
						}

						classes->add_element(elementClass, element);
					}

					///////////////////////////////////////////////////////
					// Component
					///////////////////////////////////////////////////////
					else if (eElement->getName().compareNoCase("component") == 0)
					{
						LcTaOwner<CComponentElement> newElem = CComponentElement::create();
						CComponentElement* pElement = newElem.ptr();
						LcTaOwner<CElement> element(newElem);

						pElement->className = elementClass;
						pElement->path = eElement->getAttr(NDHS_TP_XML_PATH);

						LcCXmlElem* eTeleport = eElement->find(NDHS_TP_XML_TELEPORT);
						if (eTeleport)
						{
							pElement->layoutTeleport = LcCXmlAttr::strToBool(eTeleport->getAttr(NDHS_TP_XML_LAYOUT));
							pElement->scrollTeleport = LcCXmlAttr::strToBool(eTeleport->getAttr(NDHS_TP_XML_SCROLL));
						}

						pElement->m_drawLayerIndex = eElement->getAttr(NDHS_TP_XML_DRAW_PLANE, NDHS_LOWER_INVALID_DRAW_LAYER_INDEX_STRING).toInt();
						if (pElement->path.isEmpty() == false)
						{
							nestedComponentLevel--;
							pElement->componentFile = m_pageManager->getComponentTemplate(pElement->path, stackLevel, m_paletteManifest, nestedComponentLevel);

							nestedComponentLevel++;
							if(nestedComponentLevel > NESTED_COMPONENT_LEVEL)
								nestedComponentLevel = NESTED_COMPONENT_LEVEL;

							// Get the data source for menu component, data source for menu component
							// is specified in settings of a component
							if (pElement->componentFile)
							{
								pElement->templateType = pElement->componentFile->getTemplateType();
							}
						}
						else
						{
							// We should get the component path
							continue;
						}

	#if defined(IFX_RENDER_DIRECT_OPENGL_20)

						// Read in the OpenGL rendering quality settings (if specified) [normal/high]
						LcTaString openGLRenderQuality = eElement->getAttr(NDHS_TP_XML_OPENGL_RENDER_QUALITY, "normal").toLower();

						if (openGLRenderQuality.compare("high") == 0)
						{
							pElement->openGLRenderQuality = "high";
						}
						else if (openGLRenderQuality.compare("normal") == 0)
						{
							pElement->openGLRenderQuality = "normal";
						}

	#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

						// Decide about the parent of the menu component
						pElement->elementParent = parent;

#ifdef LC_USE_LIGHTS
						LcTaString lightModel = eElement->getAttr(NDHS_TP_XML_LIGHT_MODEL, "normal");
						if (lightModel.compareNoCase("simple") == 0)
						{
							pElement->lightModel = ENdhsLightModelSimple;
						}
#endif

						// Screen
						if (pElement->elementParent.compareNoCase("screen") == 0)
						{
							pElement->elementParent = "page";
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'parent' group 'screen' may only be applied to lights: defaulting to 'page'");
						}

						pElement->m_drawLayerIndex = eElement->getAttr(NDHS_TP_XML_DRAW_PLANE, NDHS_LOWER_INVALID_DRAW_LAYER_INDEX_STRING).toInt();
						LcCXmlElem* eClipArea = eElement->find(NDHS_TP_XML_CLIPPING_AREA);
						if(eClipArea)
						{
							pElement->clipArea = LcTVector::createFromXml(eClipArea);
						}

						// This is used to tab focus around the controls.
						LcTaString tabOrderStr = eElement->getAttr(NDHS_TP_XML_FOCUS_ORDER);
						if (!tabOrderStr.isEmpty())
						{
							int tabOrder = tabOrderStr.toInt() - 1;
							if (tabOrder >= 0)
							{
								// Add the tab item to the list if it is valid.
								TTabData newData;
								newData.className	= elementClass;
								newData.elementType	= groupType;
								newData.tabOrder	= tabOrder;
								m_tabOrderList.push_back(newData);
							}
							else
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'focusOrder' index in classes section must be 1 or more");
							}
						}

						// Now read paramters
						LcCXmlElem* eParamters = eElement->find(NDHS_TP_XML_PARAMETER_BINDINGS);
						if(eParamters)
						{
							LcCXmlElem* eParam = eParamters->getFirstChild();

							for( ; eParam; eParam = eParam->getNext())
							{
								if (eParam->getName().compareNoCase(NDHS_TP_XML_PARAMETER_BINDING) != 0)
									continue;

								LcTaString paramName = eParam->getAttr(NDHS_TP_XML_NAME).toLower();
								LcTaString paramValue = eParam->getAttr(NDHS_TP_XML_VALUE);

								// Param name and param values are necessary
								if(paramName.isEmpty() == false && paramValue.isEmpty() == false)
								{
									LcTaOwner<NdhsCExpression::CExprSkeleton> exprSkeleton = NdhsCExpression::CExprSkeleton::create(paramValue);
									pElement->bindingParameters.add_element(paramName, exprSkeleton);
								}
							}
						}

						// Add special case parameter
						LcTaString enableFocusStr = eElement->getAttr(NDHS_TP_XML_ENABLE_FOCUS, "true");

						LcTaOwner<NdhsCExpression::CExprSkeleton> enableFocusSkel = NdhsCExpression::CExprSkeleton::create(enableFocusStr);
						pElement->bindingParameters.add_element("_focusEnabled", enableFocusSkel);

						// Add special _pageState Intrinsic for component
						LcTaOwner<NdhsCExpression::CExprSkeleton> exprPageState = NdhsCExpression::CExprSkeleton::create("_pagestate");
						pElement->bindingParameters.add_element("_pagestate", exprPageState);

						classes->add_element(elementClass, element);
					}

					///////////////////////////////////////////////////////
					// Graphic Element
					///////////////////////////////////////////////////////
					else if (eElement->getName().compareNoCase("graphic") == 0)
					{
						LcTaOwner<CGraphicElement> newElem = CGraphicElement::create();
						CGraphicElement* pElement = newElem.ptr();
						LcTaOwner<CElement> element(newElem);

						// Read in the material specular color property (if specified)
						if (!getColorFromXML(eElement, NDHS_TP_XML_MATERIAL_SPECULAR_COLOR, stackLevel, pElement->materialSpecularColor))
						{
							pElement->materialSpecularColor = LcTColor::BLACK;
						}

						// Read in the material emissive color property (if specified)
						if (!getColorFromXML(eElement, NDHS_TP_XML_MATERIAL_EMISSIVE_COLOR, stackLevel, pElement->materialEmissiveColor))
						{
							pElement->materialEmissiveColor = LcTColor::BLACK;
						}

						// Material specular shininess factor (if specified)
						LcTaString materialSpecularShininess = eElement->getAttr(NDHS_TP_XML_MATERIAL_SPECULAR_SHININESS);

						if (materialSpecularShininess.isEmpty() == 0)
						{
							pElement->materialSpecularShininess = materialSpecularShininess.toScalar();
						}
						else
						{
							pElement->materialSpecularShininess = 0.0;
						}

	#if defined(IFX_RENDER_DIRECT_OPENGL_20)
						LcTaString atr = eElement->getAttr(NDHS_TP_XML_VISUAL_EFFECT);
						LcTaString effectPath;

						if (atr.isEmpty() == 0)
						{
							// Get palette manifest
							NdhsCManifest* paletteMan = m_pageManager->getPaletteManifest(atr);

							// Read effect from palette manifest
							if (m_pageManager->getManifestStack()->findFile(atr, effectPath, paletteMan, stackLevel))
							{
								configureCustomEffectFromXml(effectPath, pElement->visualEffect, paletteMan, stackLevel);
							}
						}

						// Read in the OpenGL rendering quality settings (if specified) [normal/high]
						LcTaString openGLRenderQuality = eElement->getAttr(NDHS_TP_XML_OPENGL_RENDER_QUALITY, "normal").toLower();

						if (openGLRenderQuality.compare("high") == 0)
						{
							pElement->openGLRenderQuality = "high";
						}
						else if (openGLRenderQuality.compare("normal") == 0)
						{
							pElement->openGLRenderQuality = "normal";
						}

						pElement->useCustomEffect = NdhsCExpression::CExprSkeleton::create(eElement->getAttr(NDHS_TP_XML_USE_CUSTOM_EFFECT, "true"));

	#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

						// Screen
						if (parent.compareNoCase("screen") == 0)
						{
							parent = "page";
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'parent' group 'screen' may only be applied to lights: defaulting to 'page'");
						}

						pElement->m_drawLayerIndex = eElement->getAttr(NDHS_TP_XML_DRAW_PLANE, NDHS_LOWER_INVALID_DRAW_LAYER_INDEX_STRING).toInt();
						pElement->elementType	= ENdhsElementTypeGraphic;
						pElement->elementParent	= parent;
						pElement->isDetail		= isDetail;
						pElement->tappable		= tappable;
						pElement->tapTolerance	= tapTolerance;
						pElement->sketchyMode	= sketchyMode;
						pElement->antiAlias		= LcCXmlAttr::strToBool(eElement->getAttr(NDHS_TP_XML_ANTI_ALIAS, "false"));

#ifdef LC_USE_LIGHTS
						LcTaString lightModel = eElement->getAttr(NDHS_TP_XML_LIGHT_MODEL, "normal");
						if (lightModel.compareNoCase("simple") == 0)
						{
							pElement->lightModel = ENdhsLightModelSimple;
						}
#endif

						// This is used to tab focus around the controls.
						LcTaString tabOrderStr = eElement->getAttr(NDHS_TP_XML_FOCUS_ORDER);
						if (!tabOrderStr.isEmpty())
						{
							int tabOrder = tabOrderStr.toInt() - 1;
							if (tabOrder >= 0)
							{
								// Add the tab item to the list if it is valid.
								TTabData newData;
								newData.className	= elementClass;
								newData.elementType	= groupType;
								newData.tabOrder	= tabOrder;
								m_tabOrderList.push_back(newData);
							}
							else
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'focusOrder' index in classes section must be 1 or more");
							}
						}

						pElement->enableFocusExprSkel = NdhsCExpression::CExprSkeleton::create(eElement->getAttr(NDHS_TP_XML_ENABLE_FOCUS));

						LcCXmlElem* eExtent  = eElement->find(NDHS_TP_XML_EXTENT_HINT);
						if (eExtent)
						{
							pElement->extentHint = LcTVector::createFromXml(eExtent);

							if (pElement->extentHint.x < 0)
								pElement->extentHint.x = 0;

							if (pElement->extentHint.y < 0)
								pElement->extentHint.y = 0;

							if (pElement->extentHint.z < 0)
								pElement->extentHint.z = 0;
						}

						LcCXmlElem* eResource = eElement->find(NDHS_TP_XML_RESOURCE);
						if (eResource)
						{
							pElement->resourceData = NdhsCExpression::CExprSkeleton::create(eResource->getAttr(NDHS_TP_XML_DATA));
	#ifdef IFX_USE_PLUGIN_ELEMENTS
							pElement->eventHandler = NdhsCExpression::CExprSkeleton::create(eResource->getAttr(NDHS_TP_XML_EVENT_HANDLER));
	#endif
							pElement->frameCount = eResource->getAttr(NDHS_TP_XML_FRAME_COUNT, "1").toInt();
							if (pElement->frameCount < 1)
							{
								pElement->frameCount = 1;
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'resource' frame count in classes section must be 1 or more");
							}
						}

	#if defined (IFX_RENDER_DIRECT_OPENGL_20)

						LcCXmlElem* effectUniforms = eElement->find(NDHS_TP_XML_EFFECT_UNIFORMS);
						LcTaString name, type, uniformPath;
						bool bRequireManifestLookup = false;

						if(effectUniforms && m_customEffect)
						{
							LcCXmlElem* uniform = effectUniforms->getFirstChild();

							for(; uniform; uniform = uniform->getNext())
							{
								if (uniform->getName().compareNoCase("uniform") != 0)
									continue;

								name = uniform->getAttr(NDHS_TP_XML_NAME);
								type = uniform->getAttr(NDHS_TP_XML_UNIFORM_TYPE);

								// See if we need a manifest lookup
								bRequireManifestLookup = (type.compareNoCase("sampler2D") == 0) ||
														 (type.compareNoCase("samplerCube") == 0);

								LcTaString	values[6];
								LcTaString	value = "value#";

								// Default Values
								LcCXmlElem* child = uniform->find(NDHS_TP_XML_UNIFORM_VALUE);

								if(child)
								{
									for(int i = 0; i < 6; i++)
									{
										value = "value#";
										value.replace('#', ('0' + i+1));
										values[i] = child->getAttr(value);

										if (bRequireManifestLookup)
										{
											if (values[i].isEmpty() == 0)
											{
												// Get palette manifest
												NdhsCManifest* paletteMan = m_pageManager->getPaletteManifest(values[i]);

												LcTaArray<NdhsCManifest::CManifestFile*> fileData;

												if (m_pageManager->getManifestStack()->findFile(values[i],
																								uniformPath,
																								paletteMan,
																								stackLevel,
																								NULL,
																								true,
																								&fileData))
												{
													int index=-1;
													// Update uniform value with full path for later use during loading
													if(m_pageManager->findBitmapFile(&fileData,index))
													{
														values[i] = fileData[index]->absolutePath;
														fileData.clear();
													}
												}
											}
										}
									}
								}

								m_customEffect->addUniformFromTemplate(&pElement->effectUniMap, name, type, values);
							}
						}

	#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

						LcCXmlElem* eTeleport = eElement->find(NDHS_TP_XML_TELEPORT);
						if (eTeleport)
						{
							pElement->layoutTeleport = LcCXmlAttr::strToBool(eTeleport->getAttr(NDHS_TP_XML_LAYOUT));
							pElement->scrollTeleport = LcCXmlAttr::strToBool(eTeleport->getAttr(NDHS_TP_XML_SCROLL));
						}

						LcCXmlElem* eMeshGrid = eElement->find(NDHS_TP_XML_MESHGRID);
						if (eMeshGrid)
						{
							pElement->meshGridX = eMeshGrid->getAttr(NDHS_TP_XML_MESHGRID_X, "1").toInt();
							pElement->meshGridY = eMeshGrid->getAttr(NDHS_TP_XML_MESHGRID_Y, "1").toInt();

							if (pElement->meshGridX < 1)
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'meshGrid' X setting in classes section must be 1 or more");
								pElement->meshGridX = 1;
							}

							if (pElement->meshGridY < 1)
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'meshGrid' Y setting in classes section must be 1 or more");
								pElement->meshGridY = 1;
							}
						}
						else
						{
							pElement->meshGridX = 1;
							pElement->meshGridY = 1;
						}

						classes->add_element(elementClass, element);
					}

	#ifdef IFX_USE_PLUGIN_ELEMENTS
					///////////////////////////////////////////////////////
					// Plug-in Element
					///////////////////////////////////////////////////////
					else if (eElement->getName().compareNoCase("plugin") == 0)
					{
						LcTaOwner<CPluginElement> newElem = CPluginElement::create();
						CPluginElement* pElement = newElem.ptr();
						LcTaOwner<CElement> element(newElem);

						// Read in the material specular color property (if specified)
						if (!getColorFromXML(eElement, NDHS_TP_XML_MATERIAL_SPECULAR_COLOR, stackLevel, pElement->materialSpecularColor))
						{
							pElement->materialSpecularColor = LcTColor::BLACK;
						}

						// Read in the material emissive color property (if specified)
						if (!getColorFromXML(eElement, NDHS_TP_XML_MATERIAL_EMISSIVE_COLOR, stackLevel, pElement->materialEmissiveColor))
						{
							pElement->materialEmissiveColor = LcTColor::BLACK;
						}

						// Material specular shininess factor (if specified)
						LcTaString materialSpecularShininess = eElement->getAttr(NDHS_TP_XML_MATERIAL_SPECULAR_SHININESS);

						if (materialSpecularShininess.isEmpty() == 0)
						{
							pElement->materialSpecularShininess = materialSpecularShininess.toScalar();
						}
						else
						{
							pElement->materialSpecularShininess = 0.0;
						}

	#if defined(IFX_RENDER_DIRECT_OPENGL_20)
						LcTaString atr = eElement->getAttr(NDHS_TP_XML_VISUAL_EFFECT);
						LcTaString effectPath;

						if (atr.isEmpty() == 0)
						{
							// Get palette manifest
							NdhsCManifest* paletteMan = m_pageManager->getPaletteManifest(atr);

							// Read effect from palette manifest
							if (m_pageManager->getManifestStack()->findFile(atr, effectPath, paletteMan, stackLevel))
							{
								configureCustomEffectFromXml(effectPath, pElement->visualEffect, paletteMan, stackLevel);
							}
						}

						// Read in the OpenGL rendering quality settings (if specified) [normal/high]
						LcTaString openGLRenderQuality = eElement->getAttr(NDHS_TP_XML_OPENGL_RENDER_QUALITY, "normal").toLower();

						if (openGLRenderQuality.compare("high") == 0)
						{
							pElement->openGLRenderQuality = "high";
						}
						else if (openGLRenderQuality.compare("normal") == 0)
						{
							pElement->openGLRenderQuality = "normal";
						}

						pElement->useCustomEffect = NdhsCExpression::CExprSkeleton::create(eElement->getAttr(NDHS_TP_XML_USE_CUSTOM_EFFECT, "true"));

	#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

						// Screen
						if (parent.compareNoCase("screen") == 0)
						{
							parent = "page";
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'parent' group 'screen' may only be applied to lights: defaulting to 'page'");
						}

						pElement->m_drawLayerIndex = eElement->getAttr(NDHS_TP_XML_DRAW_PLANE, NDHS_LOWER_INVALID_DRAW_LAYER_INDEX_STRING).toInt();
						pElement->elementType	= ENdhsElementTypePlugin;
						pElement->elementParent	= parent;
						pElement->isDetail		= isDetail;
						pElement->tappable		= tappable;
						pElement->tapTolerance	= tapTolerance;
						pElement->antiAlias		= LcCXmlAttr::strToBool(eElement->getAttr(NDHS_TP_XML_ANTI_ALIAS, "false"));

#ifdef LC_USE_LIGHTS
						LcTaString lightModel = eElement->getAttr(NDHS_TP_XML_LIGHT_MODEL, "normal");
						if (lightModel.compareNoCase("simple") == 0)
						{
							pElement->lightModel = ENdhsLightModelSimple;
						}
#endif

						// This is used to tab focus around the controls.
						LcTaString tabOrderStr = eElement->getAttr(NDHS_TP_XML_FOCUS_ORDER);
						if (!tabOrderStr.isEmpty())
						{
							int tabOrder = tabOrderStr.toInt() - 1;
							if (tabOrder >= 0)
							{
								// Add the tab item to the list if it is valid.
								TTabData newData;
								newData.className	= elementClass;
								newData.elementType	= groupType;
								newData.tabOrder	= tabOrder;
								m_tabOrderList.push_back(newData);
							}
							else
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'focusOrder' index in classes section must be 1 or more");
							}
						}

						pElement->enableFocusExprSkel = NdhsCExpression::CExprSkeleton::create(eElement->getAttr(NDHS_TP_XML_ENABLE_FOCUS));

						LcCXmlElem* eResource = eElement->find(NDHS_TP_XML_RESOURCE);
						if (eResource)
						{
							pElement->eventHandler = NdhsCExpression::CExprSkeleton::create(eResource->getAttr(NDHS_TP_XML_EVENT_HANDLER));
						}

						LcCXmlElem* eTeleport = eElement->find(NDHS_TP_XML_TELEPORT);
						if (eTeleport)
						{
							pElement->layoutTeleport = LcCXmlAttr::strToBool(eTeleport->getAttr(NDHS_TP_XML_LAYOUT));
							pElement->scrollTeleport = LcCXmlAttr::strToBool(eTeleport->getAttr(NDHS_TP_XML_SCROLL));
						}

	#if defined (IFX_RENDER_DIRECT_OPENGL_20)

						LcCXmlElem* effectUniforms = eElement->find(NDHS_TP_XML_EFFECT_UNIFORMS);
						LcTaString name, type, uniformPath;
						bool bRequireManifestLookup = false;

						if(effectUniforms && m_customEffect)
						{
							LcCXmlElem* uniform = effectUniforms->getFirstChild();

							for(; uniform; uniform = uniform->getNext())
							{
								if (uniform->getName().compareNoCase("uniform") != 0)
									continue;

								name = uniform->getAttr(NDHS_TP_XML_NAME);
								type = uniform->getAttr(NDHS_TP_XML_UNIFORM_TYPE);

								// See if we need a manifest lookup
								bRequireManifestLookup = (type.compareNoCase("sampler2D") == 0) ||
														 (type.compareNoCase("samplerCube") == 0);

								LcTaString	values[6];
								LcTaString	value = "value#";

								// Default Values
								LcCXmlElem* child = uniform->find(NDHS_TP_XML_UNIFORM_VALUE);

								if(child)
								{
									for(int i = 0; i < 6; i++)
									{
										value = "value#";
										value.replace('#', ('0' + i+1));
										values[i] = child->getAttr(value);

										if (bRequireManifestLookup)
										{
											if (values[i].isEmpty() == 0)
											{
												// Get palette manifest
												NdhsCManifest* paletteMan = m_pageManager->getPaletteManifest(values[i]);

												LcTaArray<NdhsCManifest::CManifestFile*> fileData;

												if (m_pageManager->getManifestStack()->findFile(values[i],
																								uniformPath,
																								paletteMan,
																								stackLevel,
																								NULL,
																								true,
																								&fileData))
												{
													int index=-1;
													// Update uniform value with full path for later use during loading
													if(m_pageManager->findBitmapFile(&fileData,index))
													{
														values[i] = fileData[index]->absolutePath;
														fileData.clear();
													}
												}
											}
										}
									}
								}

								m_customEffect->addUniformFromTemplate(&pElement->effectUniMap, name, type, values);
							}
						}

	#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

						LcCXmlElem* eExtent  = eElement->find(NDHS_TP_XML_EXTENT_HINT);
						if (eExtent)
						{
							pElement->extentHint = LcTVector::createFromXml(eExtent);

							if (pElement->extentHint.x < 0)
								pElement->extentHint.x = 0;

							if (pElement->extentHint.y < 0)
								pElement->extentHint.y = 0;

							if (pElement->extentHint.z < 0)
								pElement->extentHint.z = 0;
						}

						LcCXmlElem* eMeshGrid = eElement->find(NDHS_TP_XML_MESHGRID);
						if (eMeshGrid)
						{
							pElement->meshGridX = eMeshGrid->getAttr(NDHS_TP_XML_MESHGRID_X, "1").toInt();
							pElement->meshGridY = eMeshGrid->getAttr(NDHS_TP_XML_MESHGRID_Y, "1").toInt();

							if (pElement->meshGridX < 1)
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'meshGrid' X setting in classes section must be 1 or more");
								pElement->meshGridX = 1;
							}

							if (pElement->meshGridY < 1)
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'meshGrid' Y setting in classes section must be 1 or more");
								pElement->meshGridY = 1;
							}
						}
						else
						{
							pElement->meshGridX = 1;
							pElement->meshGridY = 1;
						}

						classes->add_element(elementClass, element);
					}
	#endif
	#ifdef LC_USE_STYLUS
					///////////////////////////////////////////////////////
					// Touch Region
					///////////////////////////////////////////////////////
					else if (eElement->getName().compareNoCase("touchRegion") == 0 && isDetail == false)
					{
						LcTaOwner<CDragRegionElement> newElem = CDragRegionElement::create();
						CDragRegionElement* pElement = newElem.ptr();
						LcTaOwner<CElement> element(newElem);

						// Screen
						if (parent.compareNoCase("screen") == 0)
						{
							parent = "page";
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'parent' group 'screen' may only be applied to lights: defaulting to 'page'");
						}

						pElement->m_drawLayerIndex = eElement->getAttr(NDHS_TP_XML_DRAW_PLANE, NDHS_LOWER_INVALID_DRAW_LAYER_INDEX_STRING).toInt();
						pElement->elementType 	= ENdhsElementTypeDragRegion;
						pElement->elementParent = parent;
						pElement->isDetail 		= isDetail;
						pElement->tappable		= tappable;
						pElement->tapTolerance	= tapTolerance;

						configureDragRegionFromXml(eElement->find(NDHS_TP_XML_X_DRAG), pElement->xDrag.ptr());
						configureDragRegionFromXml(eElement->find(NDHS_TP_XML_Y_DRAG), pElement->yDrag.ptr());

						// Both axis must be absolute or relative, so if one is configured as absolute and the other as relative,
						// then XML is not valid and both axis are treated as absolute
						if (pElement->xDrag->isDragEnabled && pElement->yDrag->isDragEnabled
							&& (pElement->xDrag->isAbsolute != pElement->yDrag->isAbsolute))
						{
							pElement->isAbsolute = true;
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Both axes must be of the same type, either absolute or relative");
						}
						else
						{
							// If we get here, either both axes are
							// the same or only one is enabled
							if (pElement->xDrag->isDragEnabled)
							{
								pElement->isAbsolute = pElement->xDrag.ptr()->isAbsolute;
							}
							else if (pElement->yDrag->isDragEnabled)
							{
								pElement->isAbsolute = pElement->yDrag.ptr()->isAbsolute;
							}
							else
							{
								// No axis configured
								pElement->isAbsolute = true;
							}
						}

						// _scrollPos field can't be attached with Drag Region of type Item
						if ((pElement->xDrag->field.compare("_scrollpos") == 0
							|| pElement->yDrag->field.compare("_scrollpos") == 0) && groupType == ENdhsGroupTypeItem )
						{
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "_scrollPos can't be attached with touch region of class type Item");
						}
						else
						{
							// Absolute drag regions cant control _scrollpos if wrap is enabled
							// We don't know for sure until this point
							if (m_scrollWrap && pElement->isAbsolute
								&& (pElement->xDrag->field.compare("_scrollpos") == 0
								|| pElement->yDrag->field.compare("_scrollpos") == 0))
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "You must not use an absolute touch region to control _scrollpos");
							}
							else
							{
								classes->add_element(elementClass, element);
							}
						}
					}
	#endif //LC_USE_STYLUS
	#ifdef LC_USE_LIGHTS
					///////////////////////////////////////////////////////
					// Light Element
					///////////////////////////////////////////////////////
					else if (eElement->getName().compareNoCase("light") == 0)
					{
						if (groupType != ENdhsGroupTypeFurniture)
						{
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Secondary lights must be furniture, ignoring light definition.");
						}
						else
						{
							LcTaOwner<CSecondaryLightElement> newElem = CSecondaryLightElement::create();
							CSecondaryLightElement* pElement = newElem.ptr();
							LcTaOwner<CElement> element(newElem);

							pElement->elementParent = parent;
							pElement->elementType = ENdhsElementTypeLight;

							if (!getColorFromXML(eElement, NDHS_TP_XML_SPECULAR_COLOR, stackLevel, pElement->specularColor))
							{
								pElement->specularColor = LcTColor::WHITE;
							}

							pElement->m_drawLayerIndex = eElement->getAttr(NDHS_TP_XML_DRAW_PLANE, NDHS_LOWER_INVALID_DRAW_LAYER_INDEX_STRING).toInt();
							LcCXmlElem* eTeleport = eElement->find(NDHS_TP_XML_TELEPORT);
							if (eTeleport)
							{
								pElement->layoutTeleport = LcCXmlAttr::strToBool(eTeleport->getAttr(NDHS_TP_XML_LAYOUT));
								pElement->scrollTeleport = LcCXmlAttr::strToBool(eTeleport->getAttr(NDHS_TP_XML_SCROLL));
							}

							LcCXmlElem* eAttenuation = eElement->find(NDHS_TP_XML_ATTENUATION);
							if (eAttenuation)
							{
								pElement->attenuationConstant = eAttenuation->getAttr(NDHS_TP_XML_CONSTANT, "1").toScalar();
								if (pElement->attenuationConstant <= 0)
								{
									pElement->attenuationConstant = 1;
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Constant attenuation must be greater than 0: defaulting to 1.");
								}

								pElement->attenuationLinear = eAttenuation->getAttr(NDHS_TP_XML_LINEAR, "0").toScalar();
								if (pElement->attenuationLinear < 0)
								{
									pElement->attenuationLinear = 0;
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Linear attenuation must be non-negative: defaulting to 0.");
								}

								pElement->attenuationQuadratic = eAttenuation->getAttr(NDHS_TP_XML_QUADRATIC, "0").toScalar();
								if (pElement->attenuationQuadratic < 0)
								{
									pElement->attenuationQuadratic = 0;
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Quadratic attenuation must be non-negative: defaulting to 0.");
								}
							}

							LcCXmlAttr* typeAttr = eElement->findAttr(NDHS_TP_XML_TYPE);

							if (typeAttr)
							{
								LcTaString type = typeAttr->getVal();

								if (type.compareNoCase("bulb") == 0)
								{
									pElement->lightType = ENdhsLightTypeBulb;
								}
								else if (type.compareNoCase("directional") == 0)
								{
									pElement->lightType = ENdhsLightTypeDirectional;
								}
								else
								{
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Invalid light type specified.");
								}
							}
							else
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Light type not specified.");
							}

							classes->add_element(elementClass, element);
						}
					}
	#endif // def LC_USE_LIGHTS
				}
				else
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Duplicate class name specified in classes section");
				}
			}
			else
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'element' in classes section has no class name specified");
			}
		}
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
ENdhsVelocityProfile NdhsCTemplate::strToVelocityProfile(const LcTmString& profile)
{
	ENdhsVelocityProfile retVal = ENdhsVelocityProfileUnknown;

	if (!profile.isEmpty())
	{
		if (0 == profile.compare("accelerate"))
		{
			retVal = ENdhsVelocityProfileAccelerate;
		}
		else if (0 == profile.compare("decelerate"))
		{
			retVal = ENdhsVelocityProfileDecelerate;
		}
		else if (0 == profile.compare("bounce"))
		{
			retVal = ENdhsVelocityProfileBounce;
		}
		else if (0 == profile.compare("halfsine"))
		{
			retVal = ENdhsVelocityProfileHalfsine;
		}
		else if (0 == profile.compare("catapult"))
		{
			retVal = ENdhsVelocityProfileCatapult;
		}
		else if (0 == profile.compare("linear"))
		{
			retVal = ENdhsVelocityProfileLinear;
		}
		else
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Invalid velocity profile specified");
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureAnimationFromXml(LcCXmlElem* eAnimation, int stackLevel)
{
	// Get the action name - mandatory
	LcTaString animationName = eAnimation->getAttr(NDHS_TP_XML_NAME).toLower();

	// Animation name cannot be empty
	if (animationName.isEmpty())
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Animation name is missing");

		// Return true so that it still loads
		return true;
	}

	// Animation name must be unique
	if (m_animations.find(animationName) != m_animations.end())
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Animation name already exists");

		// Return true so that it still loads
		return true;
	}
	// There should be at least one key frame
	if (!eAnimation->getFirstChild())
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "There should be at least one key frame inside an animation element");

		// Return true so that it still loads
		return true;
	}

	LcTaOwner<NdhsCKeyFrameList> keyFrameList = NdhsCKeyFrameList::create();

	// Populate Key Frame List
	if (!configureKeyFrameListFromXml(eAnimation, keyFrameList.ptr(), LcTPlacement::EAll, stackLevel))
	{
		return false;
	}

	// Add frame list in the map
	m_animations.add_element(animationName, keyFrameList);

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureCurveFromXml(LcCXmlElem* eCurve, CLayout* pLayout, int stackLevel)
{
	int fromSlot;
	int toSlot;
	int curveId;
	LcTaString className;

	//  Get the fromSlot - mandatory
	fromSlot = eCurve->getAttr(NDHS_TP_XML_FROM_SLOT).toInt() - 1;
	if (fromSlot < 0)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Curve attribute 'fromSlot' is invalid.");
		return false;
	}

	//  Get the toSlot - mandatory
	toSlot = eCurve->getAttr(NDHS_TP_XML_TO_SLOT).toInt() - 1;
	if (toSlot < 0)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Curve attribute 'toSlot' is invalid.");
		return false;
	}

	curveId = CLayout::makeCurveId(fromSlot, toSlot);

	//  Get the class name - optional, default to "slot"
	className = eCurve->getAttr(NDHS_TP_XML_CLASS, "slot").toLower();

	LcTaOwner<NdhsCKeyFrameList> keyFrameList = NdhsCKeyFrameList::create();

	// get key frame list
	if (!configureKeyFrameListFromXml(eCurve, keyFrameList.ptr(), LcTPlacement::ELocation | LcTPlacement::EOrientation, stackLevel))
	{
		return false;
	}

	// add it to map
	return pLayout->addCurve(curveId, className, keyFrameList);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureKeyFrameListFromXml(LcCXmlElem* eAnimation, NdhsCKeyFrameList* keyFrameList, const int maskToRead, int stackLevel)
{
	LcTScalar position;
	int mask;
	LcTPlacement placement;

	LcCXmlElem* eKeyFrame = eAnimation->getFirstChild();

	for (; eKeyFrame; eKeyFrame = eKeyFrame->getNext())
	{
		if (eKeyFrame->getName().compareNoCase("keyframe") != 0)
		{
			continue;
		}

		LcCXmlAttr* attr = eKeyFrame->findAttr(NDHS_TP_XML_POSITION);
		if (attr)
		{
			position =  (LcTScalar)(attr->getVal().toScalar() / 100.0);

			if ((position < 0.0) || (position > 1.0))
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Invalid range of 'position' attribute in 'keyframe'");
				continue;
			}

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
			mask = placement.configureFromXml(m_customEffect, eKeyFrame, maskToRead & (~(LcTPlacement::EColor & LcTPlacement::EFrame)), false);
#else
			mask = placement.configureFromXml(NULL, eKeyFrame, maskToRead & (~(LcTPlacement::EColor & LcTPlacement::EFrame)), false);
#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

			if (maskToRead & LcTPlacement::EFrame)
			{
				// Must read frame separately because the placement
				// class subtracts one from the frame, and it
				// can also be negative here
				LcCXmlAttr* attr = eKeyFrame->findAttr(NDHS_TP_XML_FRAME);
				if (attr)
				{
					placement.frame = attr->getVal().toInt();
					mask |= LcTPlacement::EFrame;
				}
			}

			if (maskToRead & LcTPlacement::EColor)
			{
				LcTaString strColor;

				LcCXmlAttr* attr = eKeyFrame->findAttr(NDHS_TP_XML_COLOR);
				if (attr)
				{
					strColor = attr->getVal();
				}

				if (strColor.length() != 0)
				{
					LcTaString strOutColor = "";
					m_pageManager->getTokenStack()->replaceTokens(strColor, strOutColor, NULL, NULL, NULL, NULL, stackLevel);

					// Check the color has a valid prefix before converting it.
					if (strOutColor.bufUtf8()[0] == '#')
					{
						placement.color = strOutColor.toInt();
						mask |= LcTPlacement::EColor;
					}
				}
			}

			keyFrameList->addKeyFrame(position, mask, placement);
		}
		else
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'keyframe' has not specified mandatory attribute 'position'");
		}
	}

	keyFrameList->processKeyFrames();

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool triggerCompare(const NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTrigger* lhs, const NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTrigger* rhs)
{
	return (lhs->position <  rhs->position);
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCTemplate::convertTriggerKey(const LcTmString& key)
{
	int result = -1;

	if (key.length() > 0)
	{
		// check to see if key is a scancode
		if ((key.length() > 5) && (key.subString(0,5).compare("scan_") == 0))
		{
			result = key.subString(5, key.length() - 5).toInt();
		}
		else
		{
			// Predefined key names
			if (key.compare("select")==0)
			{
				result = (int)ENdhsNavigationKeySelect;
			}
			else if (key.compare("back")==0)
			{
				result = (int)ENdhsNavigationSoftKeyBack;
			}
			else if (key.compare("up")==0)
			{
				result = (int)ENdhsNavigationKeyUp;
			}
			else if (key.compare("down")==0)
			{
				result = (int)ENdhsNavigationKeyDown;
			}
			else if (key.compare("right")==0)
			{
				result = (int)ENdhsNavigationKeyRight;
			}
			else if (key.compare("left")==0)
			{
				result = (int)ENdhsNavigationKeyLeft;
			}
			else if (key.compare("#")==0)
			{
				result = (int)ENdhsNavigationKeyHash;
			}
			else if (key.compare("*")==0)
			{
				result = (int)ENdhsNavigationKeyAsterisk;
			}
			else if (key[0] >= 'a' && key[0] <= 'z')
			{
				// standard ASCII key values, always generic cross platform
				result = (int)key[0];
			}
			else if (key.toInt() >= 0 && key.toInt() <= 9)
			{
				// +48 so always stored in as standard ASCII codes for numbers
				result = key.toInt() + 48;
			}
			else
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "''trigger' animator has an invalid key specified");
			}
		}
	}
	else
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'trigger' animator has no key specified");
	}

	return result;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureDecorationRefFromXml(LcCXmlElem* eComponent, bool bStatic, CLayoutDecoration::CDecorationInfo::CAnimationRef* animationRef)
{
	if (!animationRef)
	{
		return false;
	}

	// Mode Attribute
	animationRef->animation = eComponent->getAttr(NDHS_TP_XML_ANIMATION).toLower();
	if (animationRef->animation.length() == 0)
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration has no animation specified");
		return false;
	}

	// If its the static animation, we need to get the timing info
	if (bStatic)
	{
		LcCXmlAttr* attr = eComponent->findAttr(NDHS_TP_XML_DURATION);
		if (!attr)
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Static decoration has no transition time specified");
			return false;
		}

		animationRef->duration = attr->getVal().toInt();

		if (animationRef->duration < 0)
			animationRef->duration = 0;

		LcTaString strLoopCount = eComponent->getAttr(NDHS_TP_XML_LOOP, "1").toLower();
		if (strLoopCount.compare("infinite") == 0)
		{
			animationRef->loopCount = LcCLinearAnimator::EInfiniteLoop;
		}
		else
		{
			animationRef->loopCount = strLoopCount.toInt();

			if (animationRef->loopCount < 1)
				animationRef->loopCount = 1;
		}

		animationRef->velocityProfile = strToVelocityProfile(eComponent->getAttr(NDHS_TP_XML_VELOCITY_PROFILE, "linear").toLower());
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT NdhsCKeyFrameList* NdhsCTemplate::getDecorationAnimation(int initialSlotActive,
																ENdhsAnimationType animType,
																int finalSlotActive,
																ENdhsObjectType objectType, const LcTmString& elementClass,
																int fromSlot, int toSlot,
																int* transitionTime, int* loopCount, ENdhsVelocityProfile* velocityProfile,const LcTmString& oLayout,
																const LcTmString &nLayout)
{
	NdhsCKeyFrameList* result = NULL;

	typedef LcTaArray<CLayoutDecoration*> TaADecorations;
	TaADecorations initDecStateList;

	//  Change for the layouts decorations

	TmADecorations::iterator itDecList = m_layoutDecorationsMap.begin();

	for(;itDecList!=m_layoutDecorationsMap.end();itDecList++)
	{
		CLayoutDecoration * state = *itDecList;

		if(state!=NULL)
		{
			if(animType==ENdhsAnimationTypeTerminalState || animType==ENdhsAnimationTypeInteractiveState)
			{
				if(state->fromLayout.compareNoCase(oLayout)==0)
				{
					initDecStateList.push_back(state);
				}
			}
			else if(state->layout.compareNoCase(nLayout)==0)
			{
				initDecStateList.push_back(state);
			}
		}
	}

	// Must try all the possible matching initial states for a decoration

	TaADecorations::iterator itInitDecState = initDecStateList.begin();

	for (; result == NULL && itInitDecState != initDecStateList.end(); itInitDecState++)
	{
		CLayoutDecoration* state = *itInitDecState;

		// If no match was found, carry on to a less specific match
		if (state == NULL)
			continue;

		switch(animType)
		{
			case ENdhsAnimationTypeTerminalState:
			case ENdhsAnimationTypeInteractiveState:
			{
				typedef LcTaArray<CLayoutDecoration::CDecorationInfo*> TaADecInfo;
				TaADecInfo::iterator itDecInfo;
				TaADecInfo decInfoList;

				if(state->layout.length()!=0 && nLayout.compareNoCase(state->layout)!=0)
					continue;

				itDecInfo = state->stateDecorations.begin();

				for (; itDecInfo != state->stateDecorations.end(); itDecInfo++)
				{
					CLayoutDecoration::CDecorationInfo* state = *itDecInfo;

					if (state != NULL)
					{
						result = getComponentDecorationAnimation(state, initialSlotActive, animType, finalSlotActive,
																objectType, elementClass, fromSlot, toSlot, transitionTime, loopCount, velocityProfile);
					}
				}

				break;
			}

			case ENdhsAnimationTypeScroll:
			{
				if (state->scrollDecoration)
				{
					result = getComponentDecorationAnimation(state->scrollDecoration.ptr(), initialSlotActive, animType, finalSlotActive,
															objectType, elementClass, fromSlot, toSlot, transitionTime, loopCount, velocityProfile);
				}

				break;
			}

			case ENdhsAnimationTypeStatic:
			{
				if (state->staticDecoration)
				{
					result = getComponentDecorationAnimation(state->staticDecoration.ptr(), initialSlotActive, animType, finalSlotActive,
															objectType, elementClass, fromSlot, toSlot, transitionTime, loopCount, velocityProfile);
				}

				break;
			}

			default:
				break;
		}
	}

	return result;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCTemplate::getTerminalDecorationInfo(int&														delay,
															int&													duration,
															ENdhsVelocityProfile&									velocityProfile,
															int&													backgroundDelay,
															CLayoutDecoration::CDecorationInfo::CTriggersList**		pTriggerList,
															int&													primaryLightDelay,
															int&													primaryLightDuration,const LcTmString& oLayout,
															const LcTmString &nLayout)
{
	CLayoutDecoration::CDecorationInfo::TTimingData* pTiming = NULL;

	// Set up the default "unfound" values
	backgroundDelay = -1;
	*pTriggerList = NULL;

	getBestDecInfo(	ENdhsAnimationTypeTerminalState,
					&pTiming, pTriggerList,oLayout,nLayout);

	if(NULL != pTiming)
	{
		delay = pTiming->delay;
		duration = pTiming->duration;
		backgroundDelay = pTiming->backgroundDelay;
		velocityProfile = pTiming->velocityProfile;
		primaryLightDelay = pTiming->primaryLightDelay;
		primaryLightDuration = pTiming->primaryLightDuration;
	}

	// If values aren't found, use the defaults
	if (-1 == delay)
	{
		delay = 0;
	}

	if (-1 == duration)
	{
		duration = m_pageManager->getDefaultTerminalTime();

		// If duration is not specified, ignore any specified delay
		delay = 0;
	}

	if (ENdhsVelocityProfileUnknown == velocityProfile)
	{
		velocityProfile = m_pageManager->getDefaultTerminalVelocityProfile();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCTemplate::getLayoutDecorationInfo(int&															delay,
															int&													duration,
															ENdhsVelocityProfile&									velocityProfile,
															int&													backgroundDelay,
															CLayoutDecoration::CDecorationInfo::CTriggersList** 	pTriggerList,
															int&													primaryLightDelay,
															int&													primaryLightDuration,const LcTmString& oLayout,
															const LcTmString&										nLayout)
{
	CLayoutDecoration::CDecorationInfo::TTimingData* pTiming = NULL;

	// Set up the default "unfound" values
	int del = -1;
	int dur = -1;
	ENdhsVelocityProfile vP = ENdhsVelocityProfileUnknown;
	int bD = -1;
	int pld = -1;
	int pldu=-1;
	*pTriggerList = NULL;

	getBestDecInfo(	ENdhsAnimationTypeTerminalState,
					&pTiming, pTriggerList,oLayout,nLayout);

	if(NULL != pTiming)
	{
		del = pTiming->delay;
		dur = pTiming->duration;
		bD = pTiming->backgroundDelay;
		vP = pTiming->velocityProfile;
		pld = pTiming->primaryLightDelay;
		pldu = pTiming->primaryLightDuration;


		delay = ( del == -1 ) ? delay : del;
		duration =( dur == -1 ) ? duration : dur;
		velocityProfile = ( vP == ENdhsVelocityProfileUnknown ) ? velocityProfile : vP;
		primaryLightDelay = ( pld == -1 ) ? primaryLightDelay : pld;
		primaryLightDuration = ( pldu == -1 ) ? primaryLightDuration : pldu;
		backgroundDelay = ( bD == -1 ) ? backgroundDelay : bD;
	}

	// If values aren't found, use the defaults
	if (-1 == delay)
	{
		delay = 0;
	}

	if (-1 == duration)
	{
		duration = m_defaultLayoutTime;

		// If duration is not specified, ignore any specified delay
		delay = 0;
	}

	if (ENdhsVelocityProfileUnknown == velocityProfile)
	{
		velocityProfile = m_defaultLayoutVelocityProfile;
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCTemplate::getStateDecorationInfo( CLayoutDecoration::CDecorationInfo::CTriggersList**	pTriggerList,
													const LcTmString&										oLayout,
													const LcTmString&										nLayout)
{
	*pTriggerList = NULL;

	getBestDecInfo(	ENdhsAnimationTypeInteractiveState,
					NULL, pTriggerList, oLayout, nLayout);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCTemplate::getStaticDecorationInfo(int												slotActive,
													CLayoutDecoration::CDecorationInfo::CTriggersList**	pTriggerList,
													const LcTmString&									oLayout,
													const LcTmString&									nLayout)
{
	*pTriggerList = NULL;

	getBestDecInfo(	ENdhsAnimationTypeStatic,
					NULL, pTriggerList,oLayout,nLayout);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::getBestDecInfo(	ENdhsAnimationType	animationType,
										CLayoutDecoration::CDecorationInfo::TTimingData**		pTimingData,
										CLayoutDecoration::CDecorationInfo::CTriggersList** 	pTriggers,
										const LcTmString&										oLayout,
										const LcTmString&										nLayout)
{
	CLayoutDecoration::CDecorationInfo* pDecInfo = NULL;
	bool foundTiming;
	bool foundTriggers;

	if (pTimingData == NULL)
	{
		// Not interested in timing
		foundTiming= true;
	}
	else
	{
		*pTimingData = NULL;
		foundTiming = false;
	}

	if (pTriggers == NULL)
	{
		// Not interested in triggers
		foundTriggers= true;
	}
	else
	{
		*pTriggers = NULL;
		foundTriggers = false;
	}

	typedef LcTaArray<CLayoutDecoration*> TaADecorations;
	TaADecorations initDecStateList;

	// build an init-decoration-list

	TmADecorations::iterator itDecList = m_layoutDecorationsMap.begin();

	for(;itDecList!=m_layoutDecorationsMap.end();itDecList++)
	{
		CLayoutDecoration * state = *itDecList;

		if(state!=NULL)
		{
			if(animationType==ENdhsAnimationTypeTerminalState || animationType==ENdhsAnimationTypeInteractiveState)
			{
				if(state->fromLayout.compareNoCase(oLayout)==0)
				{
					initDecStateList.push_back(state);
				}
			}
			else if(state->layout.compareNoCase(nLayout)==0)
			{
				initDecStateList.push_back(state);
			}
		}
	}

	// find first valid entry

	TaADecorations::iterator itInitDecState = initDecStateList.begin();
	CLayoutDecoration* initState = NULL;

	for (; (!foundTiming || !foundTriggers) && itInitDecState != initDecStateList.end(); itInitDecState++)
	{
		initState = *itInitDecState;

		if (initState == NULL)
			continue;

		switch(animationType)
		{
			case ENdhsAnimationTypeTerminalState:
			case ENdhsAnimationTypeInteractiveState:
			{
				typedef LcTaArray<CLayoutDecoration::CDecorationInfo*> TaADecInfo;
				TaADecInfo::iterator itDecInfo;
				TaADecInfo decInfoList;

				// get dec-info-list
				if(initState->layout.length()!=0 && nLayout.compareNoCase(initState->layout)!=0)
					continue;

				itDecInfo = initState->stateDecorations.begin();

				for (;
					(!foundTiming || !foundTriggers) && itDecInfo != initState->stateDecorations.end();
					itDecInfo++)
				{
					pDecInfo = *itDecInfo;

					if (pDecInfo != NULL)
					{
						// Interactive shouldn't have timing data
						// but its just as quick to check timingDataSet
						// as it is to check the state
						if (!foundTiming && pDecInfo->timingDataSet)
						{
							foundTiming = true;
							*pTimingData = &(pDecInfo->timingData);
						}

						if (!foundTriggers && pDecInfo->triggersSet)
						{
							foundTriggers = true;
							*pTriggers = pDecInfo->triggers.ptr();
						}
					}
				}

				break;
			}

			case ENdhsAnimationTypeScroll:
			{
				if (initState->scrollDecoration)
				{
					if (!foundTriggers  && initState->scrollDecoration->triggersSet)
					{
						foundTriggers = true;
						*pTriggers = initState->scrollDecoration->triggers.ptr();

						// Not interested in timing, so set it to false
						// to make sure we don't loop any further
						foundTiming = true;
					}
				}

				break;
			}

			case ENdhsAnimationTypeStatic:
			{
				if (initState->staticDecoration)
				{
					if (!foundTriggers  && initState->staticDecoration->triggersSet)
					{
						foundTriggers = true;
						*pTriggers = initState->staticDecoration->triggers.ptr();

						// Not interested in timing, so set it to false
						// to make sure we don't loop any further
						foundTiming = true;
					}
				}

				break;
			}

			default:
				break;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCKeyFrameList* NdhsCTemplate::getFurnitureDecorationAnimation(CLayoutDecoration::CDecorationInfo* info,
																	int initialSlotActive,
																	ENdhsAnimationType animType,
																	int finalSlotActive,
																	ENdhsObjectType objectType,
																	const LcTmString& elementClass,
																	int fromSlot,
																	int toSlot,
																	int* transitionTime,
																	int* loopCount,
																	ENdhsVelocityProfile* velocityProfile)
{
	typedef LcTmOwnerMap<LcTmString, CLayoutDecoration::CDecorationInfo::CAnimationRef> TmMAnimRefs;
	typedef LcTmOwnerMap<int, CLayoutDecoration::CDecorationInfo::CDecorationItem> TmMItem;
	typedef LcTmOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> TmAnimRef;
	TmMAnimRefs::iterator itRef;
	TmAnimRef outerGroupAnimation;
	TmMItem::iterator itItem;

	NdhsCKeyFrameList* result = NULL;

	itRef = info->furnitureAnimations.find(elementClass);
	if (itRef != info->furnitureAnimations.end())
	{
		result = getActualDecorationAnimation(itRef->second, itRef->second->animation, animType, transitionTime, loopCount, velocityProfile);
	}
	else if(info->outerGroupAnimations.ptr()!=NULL)
	{
		if(isOuterGroup(elementClass))
		{
			result = getActualDecorationAnimation(info->outerGroupAnimations.ptr(), info->outerGroupAnimations->animation, animType, transitionTime, loopCount, velocityProfile);
		}
	}
	else
	{
		result = getActualDecorationAnimation(info->defaultFurnitureAnimation.ptr(), info->defaultFurnitureAnimation->animation, animType, transitionTime, loopCount, velocityProfile);
	}

	return result;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCKeyFrameList* NdhsCTemplate::getItemDecorationAnimation(CLayoutDecoration::CDecorationInfo* info, int initialSlotActive,
																ENdhsAnimationType animType,
																int finalSlotActive, ENdhsObjectType objectType,
																const LcTmString& elementClass,
																int fromSlot, int toSlot,
																int* transitionTime, int* loopCount, ENdhsVelocityProfile* velocityProfile)
{
	typedef LcTmOwnerMap<LcTmString, CLayoutDecoration::CDecorationInfo::CAnimationRef> TmMAnimRefs;
	typedef LcTmOwnerMap<int, CLayoutDecoration::CDecorationInfo::CDecorationItem> TmMItem;
	typedef LcTmOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> TmAnimRef;
	TmMAnimRefs::iterator itRef;
	TmAnimRef outerGroupAnimation;
	TmMItem::iterator itItem;

	NdhsCKeyFrameList* result = NULL;

	switch (animType)
	{
		case ENdhsAnimationTypeTerminalState:
		case ENdhsAnimationTypeInteractiveState:
		case ENdhsAnimationTypeStatic:
		{
			int slot = 0;

			if (initialSlotActive == finalSlotActive && fromSlot == initialSlotActive)
			{
				// Active
				slot = CLayoutDecoration::EActive;
			}
			else if (initialSlotActive != finalSlotActive && toSlot == finalSlotActive)
			{
				// To active
				slot = CLayoutDecoration::EToActive;
			}
			else if (initialSlotActive != finalSlotActive && fromSlot == initialSlotActive)
			{
				// from active
				slot = CLayoutDecoration::EFromActive;
			}

			if (slot < 0)
			{
				// First try for the dynamic active slot position
				itItem = info->slotItemAnimations.find(slot);
				if (itItem != info->slotItemAnimations.end())
				{
					result = getDecorationItemAnimation(itItem->second, animType, objectType, elementClass, transitionTime, loopCount, velocityProfile);
				}
			}

			if (result == NULL)
			{
				// Now try that actual slot number
				itItem = info->slotItemAnimations.find(fromSlot);
				if (itItem != info->slotItemAnimations.end())
				{
					result = getDecorationItemAnimation(itItem->second, animType, objectType, elementClass, transitionTime, loopCount, velocityProfile);
				}
			}

			if (result == NULL)
			{
				// Finally, try the default settings
				result = getDecorationItemAnimation(info->defaultItemAnimations.ptr(), animType, objectType, elementClass, transitionTime, loopCount, velocityProfile);

				if (result == NULL)
				{
					result = getActualDecorationAnimation(info->defaultItemAnimations->defaultElementAnimation.ptr(), info->defaultItemAnimations->defaultElementAnimation->animation, animType, transitionTime, loopCount, velocityProfile);
				}
			}

			break;
		}
		case ENdhsAnimationTypeScroll:
		{
			typedef LcTmOwnerMap<int, CLayoutDecoration::CDecorationInfo::CFromSlot> TmMFromSlot;
			TmMFromSlot::iterator itFrom;

			if (toSlot == finalSlotActive)
			{
				// To active
				itFrom = info->toSlotItemAnimations.find(CLayoutDecoration::EActive);
				if (itFrom != info->toSlotItemAnimations.end())
				{
					result = getDecorationFromSlotAnimation(itFrom->second, initialSlotActive, animType, objectType, elementClass, fromSlot, toSlot, transitionTime, loopCount, velocityProfile);
				}
			}

			if (result == NULL)
			{
				// To slot
				itFrom = info->toSlotItemAnimations.find(toSlot);
				if (itFrom != info->toSlotItemAnimations.end())
				{
					result = getDecorationFromSlotAnimation(itFrom->second, initialSlotActive, animType, objectType, elementClass, fromSlot, toSlot, transitionTime, loopCount, velocityProfile);
				}
			}

			if (result == NULL)
			{
				// To any
				itFrom = info->toSlotItemAnimations.find(CLayoutDecoration::ESlotAny);
				if (itFrom != info->toSlotItemAnimations.end())
				{
					result = getDecorationFromSlotAnimation(itFrom->second, initialSlotActive, animType, objectType, elementClass, fromSlot, toSlot, transitionTime, loopCount, velocityProfile);
				}
			}

			if (result == NULL)
			{
				// Finally, try the default settings
				result = getDecorationItemAnimation(info->defaultItemAnimations.ptr(), animType, objectType, elementClass, transitionTime, loopCount, velocityProfile);

				if (result == NULL)
				{
					result = getActualDecorationAnimation(info->defaultItemAnimations->defaultElementAnimation.ptr(), info->defaultItemAnimations->defaultElementAnimation->animation, animType, transitionTime, loopCount, velocityProfile);
				}
			}

			break;
		}

		default:
			break;
	}

	return result;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCKeyFrameList* NdhsCTemplate::getComponentDecorationAnimation(CLayoutDecoration::CDecorationInfo* info,
																	int initialSlotActive,
																	ENdhsAnimationType animType,
																	int finalSlotActive,
																	ENdhsObjectType objectType,
																	const LcTmString& elementClass,
																	int fromSlot,
																	int toSlot,
																	int* transitionTime,
																	int* loopCount,
																	ENdhsVelocityProfile* velocityProfile)
{

	NdhsCKeyFrameList* result = NULL;

	switch (objectType)
	{
		case ENdhsObjectTypeFurnitureComponent:
		{
			// Get furniture decoration animation key frame list
			result = getFurnitureDecorationAnimation(info, initialSlotActive, animType, finalSlotActive, objectType, elementClass, fromSlot, toSlot, transitionTime, loopCount, velocityProfile);
			break;
		}
		case ENdhsObjectTypeItemComponent:
		{
			// Get item decoration animation key frame list
			result = getItemDecorationAnimation(info, initialSlotActive, animType, finalSlotActive, objectType, elementClass, fromSlot, toSlot, transitionTime, loopCount, velocityProfile);
			break;
		}

		case ENdhsObjectTypeFurniture:
		{
			// Get furniture decoration animation key frame list
			result = getFurnitureDecorationAnimation(info, initialSlotActive, animType, finalSlotActive, objectType, elementClass, fromSlot, toSlot, transitionTime, loopCount, velocityProfile);

			break;
		}

		case ENdhsObjectTypeItem:
		{
			// Get item decoration animation key frame list
			result = getItemDecorationAnimation(info, initialSlotActive, animType, finalSlotActive, objectType, elementClass, fromSlot, toSlot, transitionTime, loopCount, velocityProfile);

			break;
		}

		default:
			break;
	}

	return result;
 }

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCTemplate::getScrollDecorationInfo(int								slotActive,
								  CLayoutDecoration::CDecorationInfo::CTriggersList**	pTriggerList,
								  const LcTmString&										oLayout,
								  const LcTmString&										nLayout)
{
	*pTriggerList = NULL;

	getBestDecInfo(	ENdhsAnimationTypeScroll,
					NULL, pTriggerList, oLayout, nLayout);
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCKeyFrameList* NdhsCTemplate::getDecorationFromSlotAnimation(CLayoutDecoration::CDecorationInfo::CFromSlot* from,
																int initialSlotActive,
																ENdhsAnimationType animType,
																ENdhsObjectType objectType,
																const LcTmString& elementClass,
																int fromSlot,
																int toSlot,
																int* transitionTime,
																int* loopCount,
																ENdhsVelocityProfile* velocityProfile)
{
	typedef LcTmOwnerMap<int, CLayoutDecoration::CDecorationInfo::CDecorationItem> TmMItem;
	TmMItem::iterator itItem;

	NdhsCKeyFrameList* result = NULL;

	if (fromSlot == initialSlotActive)
	{
		// From active
		itItem = from->fromSlotItemAnimations.find(CLayoutDecoration::EActive);
		if (itItem != from->fromSlotItemAnimations.end())
		{
			result = getDecorationItemAnimation(itItem->second, animType, objectType, elementClass, transitionTime, loopCount, velocityProfile);
		}
	}

	if (result == NULL)
	{
		// From slot
		itItem = from->fromSlotItemAnimations.find(fromSlot);
		if (itItem != from->fromSlotItemAnimations.end())
		{
			result = getDecorationItemAnimation(itItem->second, animType, objectType, elementClass, transitionTime, loopCount, velocityProfile);
		}
	}

	if (result == NULL && toSlot != fromSlot)
	{
		// From +/-n
		itItem = from->offsetSlotItemAnimations.find(fromSlot - toSlot);
		if (itItem != from->offsetSlotItemAnimations.end())
		{
			result = getDecorationItemAnimation(itItem->second, animType, objectType, elementClass, transitionTime, loopCount, velocityProfile);
		}
	}

	// From any
	if (result == NULL)
	{
		// From slot
		itItem = from->fromSlotItemAnimations.find(CLayoutDecoration::ESlotAny);
		if (itItem != from->fromSlotItemAnimations.end())
		{
			result = getDecorationItemAnimation(itItem->second, animType, objectType, elementClass, transitionTime, loopCount, velocityProfile);
		}
	}

	return result;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCKeyFrameList* NdhsCTemplate::getDecorationItemAnimation(CLayoutDecoration::CDecorationInfo::CDecorationItem* item,
															ENdhsAnimationType animType,
															ENdhsObjectType objectType,
															const LcTmString& elementClass,
															int* transitionTime,
															int* loopCount,
															ENdhsVelocityProfile* velocityProfile)
{
	NdhsCKeyFrameList* result = NULL;

	typedef LcTmOwnerMap<LcTmString, CLayoutDecoration::CDecorationInfo::CAnimationRef> TmMAnimRefs;

	TmMAnimRefs::iterator itRef = item->elementAnimations.find(elementClass);
	if (itRef != item->elementAnimations.end())
	{
		result = getActualDecorationAnimation(itRef->second, itRef->second->animation, animType, transitionTime, loopCount, velocityProfile);
	}

	if (result == NULL)
	{
		result = getActualDecorationAnimation(item->defaultElementAnimation.ptr(), item->defaultElementAnimation->animation, animType, transitionTime, loopCount, velocityProfile);
	}

	return result;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCKeyFrameList* NdhsCTemplate::getActualDecorationAnimation(CLayoutDecoration::CDecorationInfo::CAnimationRef* ref,
																const LcTaString& animation,
																ENdhsAnimationType animType,
																int* transitionTime,
																int* loopCount,
																ENdhsVelocityProfile* velocityProfile)
{
	NdhsCKeyFrameList* result = NULL;

	TmMAnimations::iterator itAnimation = m_animations.find(animation);
	if (itAnimation != m_animations.end())
	{
		result = itAnimation->second;

		if (animType == ENdhsAnimationTypeStatic)
		{
			if (transitionTime != NULL)
			{
				*transitionTime = ref->duration;
			}

			if (loopCount != NULL)
			{
				*loopCount = ref->loopCount;
			}

			if (velocityProfile)
			{
				*velocityProfile = ref->velocityProfile;
			}
		}
	}

	return result;
}

#ifdef NDHS_JNI_INTERFACE
/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::setSelectable( int first, int last, int active )
{
	m_firstActive = active;
	m_firstSelectable = first;
	m_lastSelectable = last;
}

/*-------------------------------------------------------------------------*//**
*/
LcCSpace* NdhsCTemplate::getSpace()
{
	return m_pageManager->getSpace();
}

#endif // NDHS_JNI_INTERFACE

/*-------------------------------------------------------------------------*//**
	color set to the found color information, token-replaced
	Returns true iff attrribute was found
*/
bool NdhsCTemplate::getColorFromXML(LcCXmlElem* placementElem, const LcTmString& colorName, int stackLevel, LcTColor& retColor)
{
	bool retVal = false;

	if (placementElem)
	{
		LcTaString strColor;

		LcCXmlAttr* attr = placementElem->findAttr(colorName);

		if (attr)
		{
			strColor = attr->getVal();
		}

		if (strColor.length() != 0)
		{
			LcTaString strOutColor = "";
			m_pageManager->getTokenStack()->replaceTokens(strColor, strOutColor, NULL, NULL, NULL, NULL, stackLevel);

			// Check the color has a valid prefix before converting it.
			if (strOutColor.bufUtf8()[0] == '#')
			{
				retColor = strOutColor.toInt();
				retVal = true;
			}
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::configureEventInfoFromXML(LcCXmlElem* eEvent, CEventInfo* eventInfo)
{
	bool retVal = false;

	if (eEvent && eventInfo)
	{
		LcCXmlElem* eCondition = eEvent->getFirstChild();

		for( ; eCondition; eCondition = eCondition->getNext())
		{
			if (eCondition->getName().compareNoCase("guard") != 0)
				continue;

			LcTaOwner<CConditionBlock> conditionBlock = CConditionBlock::create();

			conditionBlock->guardExpr = NdhsCExpression::CExprSkeleton::create(eCondition->getAttr(NDHS_TP_XML_CONDITION, "true"));

			LcTaString actionName = eCondition->getAttr(NDHS_TP_XML_ACTION);

			if (actionName.isEmpty() == false)
			{
				if (conditionBlock->action)
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "More than one action specified for event: action ignored");
				}
				else
				{
					TmMActions::const_iterator it = m_actions.find(actionName.toLower());

					if (it != m_actions.end())
					{
						conditionBlock->action = it->second;
					}
				}
			}

			LcCXmlElem* eAction = eCondition->getFirstChild();

			for( ; eAction; eAction = eAction->getNext())
			{
				if (eAction->getName().compareNoCase("set") == 0)
				{
					LcTaString fieldName = eAction->getAttr(NDHS_TP_XML_NAME).toLower();
					LcTaString expression = eAction->getAttr(NDHS_TP_XML_VALUE);

					if (!isValidVariableName(fieldName))
					{
						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Invalid assignment target, assignment ignored");
					}
					else
					{
						TmMExprSkeleton exprSkel = NdhsCExpression::CExprSkeleton::create(expression);
						conditionBlock->assignments.add_element(fieldName, exprSkel);
					}
				}
			}

			eventInfo->conditions.push_back(conditionBlock);
		}

		retVal = true;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
ETemplateType NdhsCTemplate::determineTemplateType(LcTmString& templateType)
{
	if (templateType.isEmpty() == true)
		return ETemplateTypeInvalid;

	if (templateType.compareNoCase("component") == 0)
		return ETemplateTypeComponent;
	else if (templateType.compareNoCase("menuComponent") == 0)
		return ETemplateTypeMenuComponent;
	else if (templateType.compareNoCase("page") == 0)
		return ETemplateTypePage;

	return ETemplateTypeInvalid;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::isOuterGroup(const LcTaString& elementName)
{
	if (elementName.isEmpty())
		return false;
	return elementName.compareNoCase("outerGroup") == 0;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTemplate::isValidVariableName(const LcTmString& varName)
{
	bool retVal = false;

	if (!varName.isEmpty())
	{
		if (lc_isalpha(varName[0]))
		{
			retVal = true;
		}
	}

	return retVal;
}

#ifdef IFX_USE_SCRIPTS
/*-------------------------------------------------------------------------*//**
	Wrapper for convertKeyToInt when using scripts.
*/
bool NdhsCTemplate::convertKeyToIntWrapper(LcTmString& key, int& convertedKey, bool& catchAll)
{
	return (convertKeyToInt(key, convertedKey, catchAll));
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTemplate::storeDisplacement(LcTaOwner<CDisplacement> dispObject)
{
	// Store the object for the lifetime of the template - references to them contained in layouts
	// etc will always be valid as long as they are ultimately owned by the same template object
	m_displacementObjectStore.push_back(dispObject);
}


