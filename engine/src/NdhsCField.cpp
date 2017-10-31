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


/*-------------------------------------------------------------------------*//**
	General factory function for creating an input / output field
	Use NdhsCScrollPosField::create() for scrollpos field
*/
LcTaOwner<NdhsCField> NdhsCField::create(NdhsCMenuCon* menuCon,
										const LcTmString& fieldName,
										bool pluginField,
										NdhsCPlugin::NdhsCPluginMenu* menu,
										int menuItemIndex,
										NdhsCElement* element,
										IFXI_FIELD_MODE mode,
										IFXI_FIELD_SCOPE scope,
										IFXI_FIELDDATA_TYPE variant,
										const LcTaString defaultValue)
{
	bool isInput  = (IFXI_FIELD_MODE_INPUT == mode)  || (IFXI_FIELD_MODE_INPUT_OUTPUT == mode);
	bool isOutput = (IFXI_FIELD_MODE_OUTPUT == mode) || (IFXI_FIELD_MODE_INPUT_OUTPUT == mode);

	LC_ASSERT(isInput || isOutput); // Fields must be able to be modified

	if (IFXI_FIELDDATA_INT == variant)
	{
		int defaultValue2 = 0;
		if(!defaultValue.isEmpty())
			defaultValue2 = defaultValue.toInt();

		LcTaOwner<NdhsCFieldInt> retField = NdhsCFieldInt::create(menuCon, fieldName, pluginField, menu, menuItemIndex, scope, isInput, isOutput, defaultValue2);
		return retField;
	}
	else if (IFXI_FIELDDATA_FLOAT == variant)
	{
		LcTScalar defaultValue2 = 0.0f;
		if(!defaultValue.isEmpty())
			defaultValue2 = defaultValue.toScalar();

		LcTaOwner<NdhsCFieldFloat> retField = NdhsCFieldFloat::create(menuCon, fieldName, pluginField, menu, menuItemIndex, scope, isInput, isOutput, defaultValue2);
		return retField;
	}
	else if (IFXI_FIELDDATA_STRING == variant)
	{
		LcTaOwner<NdhsCField> ref;
		ref.set(new NdhsCField(menuCon, fieldName, pluginField, menu, menuItemIndex, IFXI_FIELDDATA_STRING, scope, isInput, isOutput, defaultValue));
		return ref;
	}
	else if (IFXI_FIELDDATA_BOOL == variant)
	{
		bool defaultValue2 = false;
		if(!defaultValue.isEmpty())
			defaultValue2 = defaultValue.compareNoCase("true") == 0;

		LcTaOwner<NdhsCFieldBool> retField = NdhsCFieldBool::create(menuCon, fieldName, pluginField, menu, menuItemIndex, scope, isInput, isOutput, defaultValue2);
		return retField;
	}
	else if (IFXI_FIELDDATA_TIME == variant)
	{
		int defaultValue2 = 0;
		if(!defaultValue.isEmpty())
			defaultValue2 = defaultValue.toInt();

		LcTaOwner<NdhsCFieldTime> retField = NdhsCFieldTime::create(menuCon, fieldName, pluginField, menu, menuItemIndex, scope, isInput, isOutput, defaultValue2);
		return retField;
	}

	// If we get here, there's been a serious error.
	LC_ASSERT(false);

	return LcTaOwner<NdhsCField>();
}

NdhsCField::NdhsCField(NdhsCMenuCon* menuCon,
			const LcTmString& fieldName,
			bool pluginField,
			NdhsCPlugin::NdhsCPluginMenu* menu,
			int menuItemIndex,
			IFXI_FIELDDATA_TYPE type,
			IFXI_FIELD_SCOPE scope,
			bool isInput,
			bool isOutput,
			LcTaString defaultValue)
:	m_menuItemIndex(menuItemIndex),
	m_fieldName(fieldName),
	m_menu(menu),
	m_fieldType(type),
	m_basicScope(scope),
	m_menuCon(menuCon),
	m_isInput(isInput),
	m_isOutput(isOutput),
	m_needPluginUpdate(pluginField),
	m_userLockCount(0),
	m_pluginField(pluginField),
	m_dirty(false),
	m_ignoreLaundry(false),
	m_currentStringValue(defaultValue),
	m_lastUpdatedValid(false),
	m_currentVelocity(0),
	m_currentDeceleration(-1.0),
	m_currentMaxSpeed(-1.0)
{
}

NdhsCField::~NdhsCField()
{
	if (m_menuCon)
	{
		NdhsCLaundry* laundry = m_menuCon->getLaundry();

		if (laundry)
		{
			laundry->removeItem(this);
		}
	}

	broadcastFieldDestroyed();
}

/*-------------------------------------------------------------------------*//**
	Add a field observer
*/
void NdhsCField::addObserver(IObserver* obs)
{
	if (obs)
		m_observers.push_back(obs);
}

/*-------------------------------------------------------------------------*//**
	Remove a field observer
*/
void NdhsCField::removeObserver(IObserver* obs)
{
	bool observerFound = false;
	LcTmArray<IObserver*>::iterator it = m_observers.begin();

	while (it != m_observers.end() && !observerFound)
	{
		if ((*it) == obs)
		{
			it = m_observers.erase(it);
			observerFound = true;
		}
		else
		{
			it++;
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Broadcast update message
*/
void NdhsCField::broadcastFieldValueUpdate()
{
	// Take a copy of subscriber list, in case anything we call unsubscribes / resubscribes
	// whilst we're processing the list (e.g. a NdhsCElement reloading itself)
	LcTaArray<IObserver*> observerListCopy;
	observerListCopy.assign(m_observers.begin(), m_observers.end());

	LcTmArray<IObserver*>::iterator it = observerListCopy.begin();

	for (;it != observerListCopy.end(); it++)
	{
		IObserver* obs = (*it);

		if (obs &&  findObserver(obs))
		{
			obs->fieldValueUpdated(this);
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Broadcast update message
*/
void NdhsCField::broadcastFieldDirty()
{
	// Take a copy of subscriber list, in case anything we call unsubscribes / resubscribes
	// whilst we're processing the list (e.g. a NdhsCElement reloading itself)
	LcTaArray<IObserver*> observerListCopy;
	observerListCopy.assign(m_observers.begin(), m_observers.end());

	LcTmArray<IObserver*>::iterator it = observerListCopy.begin();

	for (;it != observerListCopy.end(); it++)
	{
		IObserver* obs = (*it);

		if (obs && findObserver(obs))
		{
			obs->fieldDirty(this);
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Broadcast update message
*/
void NdhsCField::broadcastFieldDestroyed()
{
	// Take a copy of subscriber list, in case anything we call unsubscribes / resubscribes
	// whilst we're processing the list (e.g. a NdhsCElement reloading itself)
	LcTaArray<IObserver*> observerListCopy;
	observerListCopy.assign(m_observers.begin(), m_observers.end());

	LcTmArray<IObserver*>::iterator it = observerListCopy.begin();

	for (;it != observerListCopy.end(); it++)
	{
		IObserver* obs = (*it);

		if (obs && findObserver(obs))
		{
			obs->fieldDestroyed(this);
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Used for double-checking an observer is still in the live array before
	contacting them
*/
bool NdhsCField::findObserver(IObserver* obs)
{
	bool observerFound = false;

	LcTmArray<IObserver*>::iterator findObsIt = m_observers.begin();
	for (; findObsIt != m_observers.end() && !observerFound; findObsIt++)
	{
		if ((*findObsIt) == obs)
		{
			observerFound = true;
		}
	}

	return observerFound;
}

/*-------------------------------------------------------------------------*//**
	Get latest field data
*/
LcTaString NdhsCField::getFieldData(NdhsCElement* element, bool forceBroadcast)
{
	LcTaString retVal = "";

	if (m_isOutput)
	{
		LcTaString oldCachedValue = m_currentStringValue;

#ifdef IFX_USE_PLUGIN_ELEMENTS
		if (m_pluginField && element && element->getPluginElement())
		{
			IFXI_FIELD_SCOPE scope;
			m_menuCon->getPlugin()->requestLiveElementData(getMenuItemIndex(), getFieldName(), IFX_FIELD, element, getMenu(), retVal, &scope);

			if (IFXI_FIELD_SCOPE_ELEMENT != scope)
			{
				m_currentStringValue = retVal;
				pluginUpdatedField();
			}
		}
		else
#endif
		{
			if (m_pluginField && fieldNeedsPluginUpdate())
			{
				m_menuCon->getPlugin()->requestLiveElementData(getMenuItemIndex(), getFieldName(), IFX_FIELD, NULL, getMenu(), m_currentStringValue, NULL);
				pluginUpdatedField();

				if (m_assignment)
					cleanSelf();
			}
			else
			{
				cleanSelf();
			}

			retVal = m_currentStringValue;
		}

		// Should we broadcast the update?
#ifdef IFX_USE_PLUGIN_ELEMENTS
		bool hasElementContext = (element && element->getPluginElement());
#else
		bool hasElementContext = false;
#endif
		if (forceBroadcast
			|| (!hasElementContext && oldCachedValue.compare(m_currentStringValue) != 0))
		{
			broadcastFieldValueUpdate();
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::refresh()
{
	if (m_isOutput && !isUserLocked() && atRest())
	{
		setDirtyByPlugin(true);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::setVelocity(LcTScalar velocity)
{
	if (m_isInput && !isUserLocked())
	{
		if (!isMaxSpeedInfinite())
		{
			m_currentVelocity = min(max(velocity, -m_currentMaxSpeed), m_currentMaxSpeed);
		}
		else
		{
			m_currentVelocity = velocity;
		}

		setAnimating(true);
		addToLaundry();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::setMaxSpeed(LcTScalar maxSpeed)
{
	if (m_isInput)
	{
		m_currentMaxSpeed = maxSpeed;
		NdhsCField::setVelocity(m_currentVelocity);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::setInfiniteMaxSpeed()
{
	if (m_isInput)
	{
		m_currentMaxSpeed = -1;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::setDeceleration(LcTScalar deceleration)
{
	if (m_isInput)
	{
		LC_ASSERT(deceleration >= 0);

		if (deceleration >= 0)
		{
			m_currentDeceleration = deceleration;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::setInfiniteDeceleration()
{
	if (m_isInput)
	{
		m_currentDeceleration = -1;
	}
}

/*-------------------------------------------------------------------------*//**
	Updates current timestamp
	Returns time since last update
*/
unsigned int NdhsCField::updateTimestamp(LcTTime newTime)
{
	unsigned int dTime = 0;

	if (m_isInput)
	{
		if (m_lastUpdatedValid)
		{
			// Calc time period
			dTime = newTime - m_lastUpdated;
		}

		// Update internal timestamp
		m_lastUpdated = newTime;
		m_lastUpdatedValid = true;
	}

	return dTime;
}

/*-------------------------------------------------------------------------*//**
	Processes the current movement for updateValue. Modifies current velocity.
	Returns the difference to be applied to current value.
*/
LcTScalar NdhsCField::calcChangeInValue(unsigned int dTime)
{
	LcTScalar dValue = 0.0;

	if (m_isInput)
	{
		LcTScalar directionalDeceleration = (m_currentVelocity < 0) ? (-m_currentDeceleration) : (m_currentDeceleration);

		// This will store a validated version of the input parameter
		LcTScalar validDTime = (LcTScalar)dTime;

		// At most consider the time period up until velocity = 0
		if (directionalDeceleration != 0)
		{
			validDTime = min(validDTime, m_currentVelocity / directionalDeceleration);
		}

		dValue = m_currentVelocity * validDTime - (LcTScalar)(directionalDeceleration * validDTime * validDTime) / 2.0f;
		m_currentVelocity -= directionalDeceleration * validDTime;
	}

	return dValue;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::cancelMovement()
{
	if (m_isInput)
	{
		m_lastUpdatedValid = false;
		m_currentVelocity = 0;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::bindExpression(LcTmOwner<NdhsCExpression>& expr)
{
	// We can have invalid expression, so verify we got valid expression 
	if (!m_boundExpression && expr)
	{
		expr->setObserver(this);
		m_boundExpression = expr;
		addToLaundry();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::expressionDirty(NdhsCExpression* expr)
{
	if (m_boundExpression.ptr() == expr)
	{
		// Field is dirty
		addToLaundry();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::expressionDestroyed(NdhsCExpression* expr)
{
	if (expr == m_assignment)
	{
		m_assignment = NULL;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::addToLaundry()
{
	if (m_menuCon && !m_dirty)
	{
		m_dirty = true;

		if (!m_ignoreLaundry)
		{
			NdhsCLaundry* laundry = m_menuCon->getLaundry();

			if (laundry)
			{
				laundry->addItem(this);
			}
		}

		broadcastFieldDirty();
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCField::cleanLaundryItem(LcTTime timestamp)
{
	bool animating = false;

	if (isAnimating())
	{
		bool finalUpdate;

		// If there is any pending assignment operation, do that first...
		if (m_assignment)
		{
			setValueFromExpression(m_assignment, m_assignmentContext, m_assignmentSlotNum, m_assignemntItem);
			m_assignment = NULL;
		}

		animating = updateValue(timestamp, finalUpdate);
		m_dirty = isAnimating();
	}
	else if (m_dirty)
	{
		if (m_pluginField && m_isOutput)
		{
			getFieldData(NULL, true);
		}
		else
		{
			cleanSelf();
		}

		m_dirty = false;
	}

	return animating;
}

#ifdef IFX_SERIALIZATION
NdhsCField::IObserver*	NdhsCField::IObserver::loadState(int type,SerializeHandle h,LcCSerializeMaster * serializeMaster)
{
	ISerializeable* serializeable=NULL;
	switch(type)
	{
		case 0:
		{
			serializeable=serializeMaster->getPointer(h);
			if(serializeable==NULL)
			{
				return NdhsCElement::loadState(h,serializeMaster);
			}
			return (NdhsCElement*)serializeable;
		}
		break;
		case 1:
		{
			serializeable=serializeMaster->getPointer(h);
			if(serializeable==NULL)
			{
				return NdhsCElementGroup::loadState(h,serializeMaster);
			}
			return (NdhsCElementGroup*)serializeable;
		}
		break;
		case 2:
		{
			serializeable=serializeMaster->getPointer(h);
			if(serializeable==NULL)
			{
				return NdhsCExpressionField::loadState(h,serializeMaster);
			}
			return (NdhsCExpressionField*)serializeable;
		}
		case 3:
		{
			NdhsCField *field;
			return NdhsCMenuComponent::loadMetaFieldState(h,serializeMaster,field);
		}
		break;
		case 5:
		{
			serializeable=serializeMaster->getPointer(h);
			if(serializeable==NULL)
			{
				return NdhsCMenuComponent::loadState(h,serializeMaster);
			}
			return (NdhsCMenuComponent*)serializeable;
		}
		break;
		default:
			return NULL;
	}
}

NdhsCField* NdhsCField::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	if(handle==-1)
	{
		NdhsCField*obj=new NdhsCField();
		return obj;
	}

	void * ptr=serializeMaster->getOffset(handle);
	int type=*((int*)ptr);
	switch(type)
	{
		case ENdhsCFieldTypeBase:
		{
			NdhsCField*obj=new NdhsCField();
			obj->deSerialize(handle,serializeMaster);
			serializeMaster->setPointer(handle,obj);
			return obj;
		}
		case ENdhsCFieldTypeBool:
		{
			return NdhsCFieldBool::loadState(handle,serializeMaster);
		}
		case ENdhsCFieldTypeFloat:
		{
			return NdhsCFieldFloat::loadState(handle,serializeMaster);
		}
		case ENdhsCFieldTypeInt:
		{
			return NdhsCFieldInt::loadState(handle,serializeMaster);
		}
		case ENdhsCFieldTypeScrollPos:
		{
			return NdhsCScrollPosField::loadState(handle,serializeMaster);
		}
		case ENdhsCFieldTypePlugin:
		{
			return NdhsCField::loadStatePluginField(handle,serializeMaster);
		}
		case ENdhsCFieldTypeMeta:
		{
			NdhsCField *field=NULL;
			NdhsCMenuComponent::loadMetaFieldState(handle,serializeMaster,field);
			return field;
		}
		case ENdhsCFieldTypeTime:
		{
			return NdhsCFieldTime::loadState(handle,serializeMaster);
		}
	}
	return NULL;
}

NdhsCField* NdhsCField::loadStatePluginField(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCField *output=NULL;
	void * cPtr=serializeMaster->getOffset(handle);
	if(cPtr!=NULL)
	{
		ENdhsCFieldType typ=ENdhsCFieldTypePlugin;
		DESERIALIZE(typ,serializeMaster,cPtr)

		LcTaString fieldName="";
		NdhsCPlugin::NdhsCPluginMenu * menu=NULL;
		int menuItemIndex=-1;

		DESERIALIZE_String(fieldName,serializeMaster,cPtr)
		DESERIALIZE_Reserve(menu,serializeMaster,cPtr,NdhsCPlugin::NdhsCPluginMenu)
		DESERIALIZE(menuItemIndex,serializeMaster,cPtr)

		NdhsCPageManager *pageManager=(NdhsCPageManager*)serializeMaster->getPointer(0);
		output=pageManager->getPlugin()->getField(fieldName,menu,menuItemIndex,NULL);
		serializeMaster->setPointer(handle,output);
		if(output!=NULL)
			output->deSerializePluginField(handle,serializeMaster);
	}
	return output;
}

SerializeHandle	NdhsCField::serialize(LcCSerializeMaster *serializeMaster,bool force)
{
	SerializeHandle handle=-1;
	if(isPluginField())
		return serializePluginField(serializeMaster,force);

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

	int outputSize = sizeof(NdhsCField)+sizeof(IFX_INT32);
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	ENdhsCFieldType typ=ENdhsCFieldTypeBase;
	SERIALIZE(typ,serializeMaster,cPtr)
	SERIALIZE(m_pluginField,serializeMaster,cPtr)
	SERIALIZE(m_userLockCount,serializeMaster,cPtr)
	SERIALIZE(m_needPluginUpdate,serializeMaster,cPtr)
	SERIALIZE(m_isOutput,serializeMaster,cPtr)
	SERIALIZE(m_isInput,serializeMaster,cPtr)
	SERIALIZE(m_fieldType,serializeMaster,cPtr)
	SERIALIZE(m_menuItemIndex,serializeMaster,cPtr)
	SERIALIZE(m_assignmentSlotNum,serializeMaster,cPtr)
	SERIALIZE(m_ignoreLaundry,serializeMaster,cPtr)
	SERIALIZE(m_dirty,serializeMaster,cPtr)
	SERIALIZE(m_targetValue,serializeMaster,cPtr)
	SERIALIZE(m_currentMaxSpeed,serializeMaster,cPtr)
	SERIALIZE(m_lastUpdatedValid,serializeMaster,cPtr)
	SERIALIZE(m_currentDeceleration,serializeMaster,cPtr)
	SERIALIZE(m_currentVelocity,serializeMaster,cPtr)
	SERIALIZE(m_lastUpdated,serializeMaster,cPtr)
	SERIALIZE(m_menuItemChild,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_menuCon,serializeMaster,cPtr)
	SERIALIZE_String(m_currentStringValue,serializeMaster,cPtr)
	SERIALIZE_String(m_fieldName,serializeMaster,cPtr)
	SERIALIZE_Observer(m_observers,serializeMaster,cPtr)
	SERIALIZE_Owner(m_boundExpression,serializeMaster,cPtr)
	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}

SerializeHandle	NdhsCField::serializePluginField(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCField)+sizeof(IFX_INT32);
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	ENdhsCFieldType typ=ENdhsCFieldTypePlugin;
	SERIALIZE(typ,serializeMaster,cPtr)
	SERIALIZE_String(m_fieldName,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_menu,serializeMaster,cPtr)
	int itemIndex=getMenuItemIndex();
	SERIALIZE(itemIndex,serializeMaster,cPtr)
	SERIALIZE_Observer(m_observers,serializeMaster,cPtr)
	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}


void	NdhsCField::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	DESERIALIZE(m_fieldType,serializeMaster,cPtr)
	DESERIALIZE(m_pluginField,serializeMaster,cPtr)
	DESERIALIZE(m_userLockCount,serializeMaster,cPtr)
	DESERIALIZE(m_needPluginUpdate,serializeMaster,cPtr)
	DESERIALIZE(m_isOutput,serializeMaster,cPtr)
	DESERIALIZE(m_isInput,serializeMaster,cPtr)
	DESERIALIZE(m_fieldType,serializeMaster,cPtr)
	DESERIALIZE(m_menuItemIndex,serializeMaster,cPtr)
	DESERIALIZE(m_assignmentSlotNum,serializeMaster,cPtr)
	DESERIALIZE(m_ignoreLaundry,serializeMaster,cPtr)
	DESERIALIZE(m_dirty,serializeMaster,cPtr)
	DESERIALIZE(m_targetValue,serializeMaster,cPtr)
	DESERIALIZE(m_currentMaxSpeed,serializeMaster,cPtr)
	DESERIALIZE(m_lastUpdatedValid,serializeMaster,cPtr)
	DESERIALIZE(m_currentDeceleration,serializeMaster,cPtr)
	DESERIALIZE(m_currentVelocity,serializeMaster,cPtr)
	DESERIALIZE(m_lastUpdated,serializeMaster,cPtr)
	DESERIALIZE(m_menuItemChild,serializeMaster,cPtr)
	DESERIALIZE_Reserve(m_menuCon,serializeMaster,cPtr,NdhsCMenuCon);
	DESERIALIZE_String(m_currentStringValue,serializeMaster,cPtr)
	DESERIALIZE_String(m_fieldName,serializeMaster,cPtr)
	DESERIALIZE_Observer(m_observers,serializeMaster,cPtr)
	DESERIALIZE_Owner(m_boundExpression,serializeMaster,cPtr,NdhsCExpression)

}

void	NdhsCField::deSerializePluginField(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	if(cPtr!=NULL)
	{
		ENdhsCFieldType typ=ENdhsCFieldTypePlugin;
		DESERIALIZE(typ,serializeMaster,cPtr)

		LcTaString fieldName="";
		NdhsCPluginMenu * menu=NULL;
		int menuItemIndex=-1;

		DESERIALIZE_String(fieldName,serializeMaster,cPtr)
		DESERIALIZE_Reserve(menu,serializeMaster,cPtr,NdhsCPluginMenu)
		DESERIALIZE(menuItemIndex,serializeMaster,cPtr)
		DESERIALIZE_Observer(m_observers,serializeMaster,cPtr)
	}
}

#endif /* IFX_SERIALIZATION */

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::cleanSelf()
{
	if (m_dirty && !isAnimating())
	{
		m_dirty = false;

		if (m_assignment)
		{
			setValueFromExpression(m_assignment, m_assignmentContext, m_assignmentSlotNum, m_assignemntItem);
			m_assignment = NULL;
		}
		else if (m_boundExpression)
		{
			setValueFromExpression(m_boundExpression.ptr(), NULL, -1, NULL);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::setValueFromExpression(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item)
{
	if (expr)
	{
		LcTaString oldCachedValue = m_currentStringValue;
		bool oldIsError = isError();

		expr->evaluate(context, slot, item);

		if (expr->isError())
		{
			setErrorStatus(true);
			expr->errorDiagnostics("Variable \"" + m_fieldName + "\"", false);
		}
		else
		{
			setErrorStatus(false);
			m_currentStringValue = expr->getValueString();
		}

		if (oldCachedValue.compare(m_currentStringValue) != 0 || isError() != oldIsError)
		{
			broadcastFieldValueUpdate();
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::setDirtyByPlugin(bool requiresLaundry)
{
	if (m_pluginField && m_isOutput)
	{
		m_needPluginUpdate = true;
		if (requiresLaundry)
			addToLaundry();

	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCField::setDirtyByAssignment(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item)
{
	LC_ASSERT(m_assignment == NULL);

	m_assignment = expr;
	m_assignmentContext = context;
	m_assignmentSlotNum = slot;
	m_assignemntItem = item;

	addToLaundry();
}
