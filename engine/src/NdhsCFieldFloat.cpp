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
*/
LcTaOwner<NdhsCFieldFloat> NdhsCFieldFloat::create(				NdhsCMenuCon* menuCon,
																const LcTmString& fieldName,
																bool pluginField,
																NdhsCPlugin::NdhsCPluginMenu* menu,
																int menuItemIndex,
																IFXI_FIELD_SCOPE scope,
																bool isInput,
																bool isOutput,
																LcTScalar defaultValue)
{
	LcTaOwner<NdhsCFieldFloat> ref;
	ref.set(new NdhsCFieldFloat(menuCon, fieldName, pluginField, menu, menuItemIndex, scope, isInput, isOutput, defaultValue));
	ref->construct();
	return ref;
}
#ifdef IFX_SERIALIZATION
NdhsCFieldFloat* NdhsCFieldFloat::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCFieldFloat *obj=new NdhsCFieldFloat();
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCFieldFloat::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCFieldFloat) - sizeof(NdhsCField)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCField::serialize(serializeMaster,true);
	ENdhsCFieldType dataType=ENdhsCFieldTypeFloat;
	SERIALIZE(dataType,serializeMaster,cPtr)
	SERIALIZE(parentHandle,serializeMaster,cPtr)
	SERIALIZE(m_animMinValue,serializeMaster,cPtr)
	SERIALIZE(m_animMaxValue,serializeMaster,cPtr)
	SERIALIZE(m_animRangeDefined,serializeMaster,cPtr)
	SERIALIZE(m_currentValue,serializeMaster,cPtr)
	SERIALIZE(m_snapshotValue,serializeMaster,cPtr)
	SERIALIZE(m_currentStringValueBase,serializeMaster,cPtr)
	SERIALIZE(m_animating,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCFieldFloat::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummySize=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;
	DESERIALIZE(dummySize,serializeMaster,cPtr)
	DESERIALIZE(parentHandle,serializeMaster,cPtr)
	NdhsCField::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE(m_animMinValue,serializeMaster,cPtr)
	DESERIALIZE(m_animMaxValue,serializeMaster,cPtr)
	DESERIALIZE(m_animRangeDefined,serializeMaster,cPtr)
	DESERIALIZE(m_currentValue,serializeMaster,cPtr)
	DESERIALIZE(m_snapshotValue,serializeMaster,cPtr)
	DESERIALIZE(m_currentStringValueBase,serializeMaster,cPtr)
	DESERIALIZE(m_animating,serializeMaster,cPtr)
}
#endif /* IFX_SERIALIZATION */
/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldFloat::construct()
{
	if (isInput())
	{
		IFXI_VARIANT_TYPE value;

		if (getMenuCon())
		{
			if (isPluginField() && getMenuCon()->getPlugin()->getRawElementData(getMenuItemIndex(), getFieldName(), NULL, getMenu(), value, NULL))
			{
				m_currentValue = value.f;
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCFieldFloat::getFieldData(NdhsCElement* element)
{
	LcTaString retVal = "";

	if (isOutput())
	{
		retVal = getFieldData(element, false);
	}
	else if (isInput())
	{
		if (isPluginField())
		{
			// Query IL based on m_currentValue
			IFXI_VARIANT_TYPE val;
			val.f = m_currentValue;

			if (getMenuCon())
			{
				getMenuCon()->getPlugin()->translateRawElementData(getFieldName(), getMenu(), val, retVal);
			}
		}
		else
		{
			retVal.fromScalar(m_currentValue);
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCFieldFloat::getFieldData(NdhsCElement* element, bool forceBroadcast)
{
	LcTaString retVal = "";

	if (isOutput())
	{
		LcTScalar oldValue = m_currentValue;
		LcTScalar data = getRawFieldData(element);

		// Check if the cached string needs updating
		if (m_currentStringValueBase != m_currentValue || m_currentStringValue.isEmpty())
		{
			m_currentStringValueBase = m_currentValue;

			if (isPluginField())
			{
				IFXI_VARIANT_TYPE val;
				val.f = m_currentValue;
				getMenuCon()->getPlugin()->translateRawElementData(getFieldName(), getMenu(), val, m_currentStringValue);
			}
			else
			{
				m_currentStringValue.fromScalar(m_currentValue);
			}
		}

		//  Work out the string we need to return
		if (data == m_currentValue)
		{
			retVal = m_currentStringValue;
		}
		else
		{
			if (isPluginField())
			{
				IFXI_VARIANT_TYPE val;
				val.f = data;
				getMenuCon()->getPlugin()->translateRawElementData(getFieldName(), getMenu(), val, retVal);
			}
			else
			{
				retVal.fromScalar(data);
			}
		}

		// Should we broadcast the update?
#ifdef IFX_USE_PLUGIN_ELEMENTS
		bool hasElementContext = (element && element->getPluginElement());
#else
		bool hasElementContext = false;
#endif
		if (forceBroadcast
			|| (!hasElementContext && oldValue != m_currentValue))
		{
			broadcastFieldValueUpdate();
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTScalar NdhsCFieldFloat::getRawFieldData(NdhsCElement* element)
{
	LcTScalar retVal = m_currentValue;

	if (isOutput() && atRest())
	{
#ifdef IFX_USE_PLUGIN_ELEMENTS
		if (isPluginField() && element && element->getPluginElement())
		{
			IFXI_FIELD_SCOPE scope;
			IFXI_VARIANT_TYPE value;

			if (getMenuCon()->getPlugin()->getRawElementData(getMenuItemIndex(), getFieldName(), element, getMenu(), value, &scope))
			{
				retVal = value.f;
			}

			if (IFXI_FIELD_SCOPE_ELEMENT != scope)
			{
				m_currentValue = retVal;
				pluginUpdatedField();
			}
		}
		else
#endif
		{
			if (isPluginField() && fieldNeedsPluginUpdate())
			{
				IFXI_VARIANT_TYPE value;
				if (getMenuCon()->getPlugin()->getRawElementData(getMenuItemIndex(), getFieldName(), NULL, getMenu(), value, NULL))
				{
					m_currentValue = value.f;
					pluginUpdatedField();
				}

				if (m_assignment)
					cleanSelf();
			}
			else
			{
				cleanSelf();
			}

			retVal = m_currentValue;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldFloat::setTargetValue(LcTScalar target, unsigned int duration)
{
	if (isInput() && !isUserLocked())
	{
		// Calc distance & direction needed
		LcTScalar distance = target - m_currentValue;

		// Store target value
		m_targetValue = m_currentValue + distance;

		m_currentVelocity = 2.0f * distance / (LcTScalar)duration;

		LcTScalar decel = m_currentVelocity / (LcTScalar)duration;

		// Deceleration value always positive
		if (decel < 0)
		{
			decel = -decel;
		}

		setDeceleration(decel);
		setAnimating(true);

		m_animMinValue = 0;
		m_animMaxValue = 0;
		m_animRangeDefined = false;

		addToLaundry();
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldFloat::setTargetValue(LcTScalar target, LcTScalar minValue, LcTScalar maxValue, unsigned int duration)
{
	bool retVal = setTargetValue(target, duration);

	m_animMinValue = minValue;
	m_animMaxValue = maxValue;
	m_animRangeDefined = true;

	return retVal;
}


/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldFloat::updateValue(LcTTime timestamp, bool& finalUpdate)
{
	bool reschedule = false;

	if (isInput() && isAnimating())
	{
		finalUpdate = false;
		LcTScalar oldValue = m_currentValue;

		if (isDecelerationInfinite() || atRest())
		{
			updateTimestamp(timestamp);
			finalUpdate = true;
			if (m_currentVelocity != 0)
				m_currentVelocity = 0;
		}
		else if (  (m_animRangeDefined && m_currentValue >= m_animMaxValue && m_currentVelocity > 0)
				|| (m_animRangeDefined && m_currentValue <= m_animMinValue && m_currentVelocity < 0))
		{
			updateTimestamp(timestamp);
			reschedule = true;
			if (m_currentVelocity != 0)
				m_currentVelocity = 0;
		}
		else
		{
			reschedule = true;

			if (!m_lastUpdatedValid)
			{
				updateTimestamp(timestamp);
			}
			else
			{
				unsigned int dTime = updateTimestamp(timestamp);
				LcTScalar oldVelocity = m_currentVelocity;

				m_currentValue += calcChangeInValue(dTime);

				if (m_animRangeDefined)
				{
					if (m_currentValue > m_animMaxValue)
					{
						// Clip value to range
						m_currentValue = m_animMaxValue;
						m_currentVelocity = 0;
						finalUpdate = true;
					}
					else if (m_currentValue < m_animMinValue)
					{
						// Clip value to range
						m_currentValue = m_animMinValue;
						m_currentVelocity = 0;
						finalUpdate = true;
					}
				}

				if (!finalUpdate && ((oldVelocity > 0 && m_currentVelocity < 0) || (oldVelocity < 0 && m_currentVelocity > 0) || atRest()))
				{
					m_currentVelocity = 0;
					finalUpdate = true;
				}
			}
		}

		if (oldValue != m_currentValue)
		{
			if (!isLaundryUsed())
			{
				broadcastFieldDirty();
			}

			valueUpdated(finalUpdate);
		}

		if (finalUpdate)
		{
			m_lastUpdatedValid = false;
		}
	}
	else
	{
		finalUpdate = true;
	}

	if (finalUpdate)
	{
		setAnimating(false);
	}
	else
	{
		setAnimating(true);
	}

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldFloat::setValue(LcTScalar newValue, bool finalUpdate)
{
	if (isInput())
	{
		LcTScalar oldValue = m_currentValue;

		m_lastUpdatedValid = false;
		m_currentValue = newValue;
		m_currentVelocity = 0;
		setAnimating(false);

		if (oldValue != m_currentValue || finalUpdate)
		{
			broadcastFieldDirty();
			valueUpdated(finalUpdate);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldFloat::takeSnapshot()
{
	if (isInput())
		m_snapshotValue = m_currentValue;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldFloat::addToSnapshot(LcTScalar dValue, LcTScalar minValue, LcTScalar maxValue, bool finalUpdate)
{
	if (isInput())
		setValue(max(minValue, min(maxValue, m_snapshotValue + dValue)), finalUpdate);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldFloat::valueUpdated(bool finalUpdate)
{
	if (isInput())
	{
		if (isPluginField())
		{
			IFXI_VARIANT_TYPE val;
			val.f = m_currentValue;

			if (getMenuCon())
			{
				getMenuCon()->getPlugin()->notifyFieldValueRequest(getMenuItemIndex(), getFieldName(), getMenu(), val, finalUpdate);
			}
		}

		broadcastFieldValueUpdate();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldFloat::setValueFromExpression(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item)
{
	if (expr)
	{
		LcTScalar oldCachedValue = m_currentValue;
		bool oldIsError = isError();

		expr->evaluate(context, slot, item);

		if (expr->isError())
		{
			setErrorStatus(true);
			expr->errorDiagnostics("Variable \"" + getFieldName() + "\" value", false);
		}
		else
		{
			setErrorStatus(false);

			switch (expr->expressionType())
			{
				case NdhsCExpression::ENdhsExpressionTypeInt:
				{
					m_currentValue = (LcTScalar)expr->getValueInt();
				}
				break;

				case NdhsCExpression::ENdhsExpressionTypeScalar:
				{
					m_currentValue = expr->getValueScalar();
				}
				break;

				default:
				{
					setErrorStatus(true);
					expr->errorDiagnostics("Float variable \"" + getFieldName() + "\" value set to non-numeric", false);
				}
				break;
			}
		}

		setAnimating(false);
		setClean();

		if (oldCachedValue != m_currentValue || oldIsError != isError())
		{
			// Check whether we are about to give the module erroneous data
			if (isInput() && isPluginField() && isError())
			{
				expr->errorDiagnostics("Module field", true);
			}
			else
			{
				if (isInput())
				{
					valueUpdated(true);
				}
				else
				{
					broadcastFieldValueUpdate();
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldFloat::setVelocity(LcTScalar velocity)
{
	NdhsCField::setVelocity(velocity);
	m_animRangeDefined = false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldFloat::setVelocity(LcTScalar velocity, LcTScalar minValue, LcTScalar maxValue)
{
	NdhsCField::setVelocity(velocity);

	m_animMinValue = minValue;
	m_animMaxValue = maxValue;
	m_animRangeDefined = true;
}

