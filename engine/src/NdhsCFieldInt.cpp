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


#include <math.h>

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCFieldInt> NdhsCFieldInt::create(				NdhsCMenuCon* menuCon,
															const LcTmString& fieldName,
															bool pluginField,
															NdhsCPlugin::NdhsCPluginMenu* menu,
															int menuItemIndex,
															IFXI_FIELD_SCOPE scope,
															bool isInput,
															bool isOutput,
															int defaultValue)
{
	LcTaOwner<NdhsCFieldInt> ref;
	ref.set(new NdhsCFieldInt(menuCon, fieldName, pluginField, menu, menuItemIndex, scope, isInput, isOutput, defaultValue));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldInt::construct()
{
	if (isInput())
	{
		IFXI_VARIANT_TYPE value;
		if (isPluginField() && getMenuCon()->getPlugin()->getRawElementData(getMenuItemIndex(), getFieldName(), NULL, getMenu(), value, NULL))
		{
			m_currentValue = value.i;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCFieldInt::getFieldData(NdhsCElement* element)
{
	LcTaString retVal = "";

	if (isInput())
	{
		if (isPluginField())
		{
			// Query IL based on m_currentValue
			IFXI_VARIANT_TYPE val;
			val.i = m_currentValue;
			getMenuCon()->getPlugin()->translateRawElementData(getFieldName(), getMenu(), val, retVal);
		}
		else
		{
			retVal.fromInt(m_currentValue);
		}
	}
	else if (isOutput())
	{
		retVal = getFieldData(element, false);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCFieldInt::getFieldData(NdhsCElement* element, bool forceBroadcast)
{
	LcTaString retVal = "";

	if (isOutput())
	{
		int oldValue = m_currentValue;

		int data = getRawFieldDataInt(element);

		// Check if the cached string needs updating
		if (m_currentStringValueBase != m_currentValue || m_currentStringValue.isEmpty())
		{
			m_currentStringValueBase = m_currentValue;

			if (isPluginField())
			{
				IFXI_VARIANT_TYPE val;
				val.i = m_currentValue;
				getMenuCon()->getPlugin()->translateRawElementData(getFieldName(), getMenu(), val, m_currentStringValue);
			}
			else
			{
				m_currentStringValue.fromInt(m_currentValue);
			}
		}

		// Work out the string we need to return
		if (data == m_currentValue)
		{
			retVal = m_currentStringValue;
		}
		else
		{
			if (isPluginField())
			{
				IFXI_VARIANT_TYPE val;
				val.i = data;
				getMenuCon()->getPlugin()->translateRawElementData(getFieldName(), getMenu(), val, retVal);
			}
			else
			{
				retVal.fromInt(data);
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
LcTScalar NdhsCFieldInt::getRawFieldData(NdhsCElement* element)
{
	return (LcTScalar)getRawFieldDataInt(element);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldInt::setTargetValue(LcTScalar target, unsigned int duration)
{
	if (isInput()&& !isUserLocked())
	{
		// Slightly overshoot the target to make sure the final int rollover is triggered
		LcTScalar overshotTarget = target;

		if ((m_currentValue + m_currentValueOffset) > target)
		{
			overshotTarget -= OVERSHOOT_MARGIN;
		}
		else
		{
			overshotTarget += OVERSHOOT_MARGIN;
		}

		// Calc distance needed
		LcTScalar distance = overshotTarget - (m_currentValue + m_currentValueOffset);

		// Store target value
		m_targetValue = overshotTarget;

		m_currentVelocity = 2.0f * distance / (LcTScalar)duration;

		LcTScalar decel = m_currentVelocity / (LcTScalar)duration;

		if (decel < 0)
		{
			decel = -decel;
		}

		setDeceleration(decel);

		m_animMinValue = 0;
		m_animMaxValue = 0;
		m_animRangeDefined = false;

		setAnimating(true);

		addToLaundry();
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldInt::setTargetValue(LcTScalar target, LcTScalar minValue, LcTScalar maxValue, unsigned int duration)
{
	bool retVal = setTargetValue(target, duration);

	m_animMinValue = (int)minValue;
	m_animMaxValue = (int)maxValue;
	m_animRangeDefined = true;

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldInt::updateValue(LcTTime timestamp, bool& finalUpdate)
{
	bool reschedule = false;

	if (isInput() && isAnimating())
	{
		finalUpdate = false;
		int oldValue = m_currentValue;

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

				// Update current value
				m_currentValueOffset += calcChangeInValue(dTime);

				if (m_currentValueOffset >= 1.0 || m_currentValueOffset <= -1.0)
				{
					m_currentValue += (int)m_currentValueOffset;
					m_currentValueOffset -= (int)m_currentValueOffset;
				}

				// Checks for final update due to hitting the boundaries
				if (m_animRangeDefined)
				{
					if (m_currentValue > m_animMaxValue || (m_currentValue == m_animMaxValue && m_currentValueOffset >= 0))
					{
						m_currentValue = m_animMaxValue;
						m_currentValueOffset = 0;
						m_currentVelocity = 0;
						finalUpdate = true;
					}
					else if (m_currentValue < m_animMinValue || (m_currentValue == m_animMinValue && m_currentValueOffset <= 0))
					{
						m_currentValue = m_animMinValue;
						m_currentValueOffset = 0;
						m_currentVelocity = 0;
						finalUpdate = true;
					}
				}

				if ((oldVelocity > 0 && m_currentVelocity < 0) || (oldVelocity < 0 && m_currentVelocity > 0) || atRest())
				{
					m_currentVelocity = 0;
					m_currentValueOffset = 0;
					finalUpdate = true;
				}
			}
		}

		if (oldValue != m_currentValue || finalUpdate)
		{
			if (!isLaundryUsed())
			{
				broadcastFieldDirty();
			}

			valueUpdated(finalUpdate);
		}

		if (finalUpdate)
			m_lastUpdatedValid = false;
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
void NdhsCFieldInt::setValue(LcTScalar newValue, bool finalUpdate)
{
	if (isInput())
	{
		int oldValue = m_currentValue;

		m_lastUpdatedValid = false;
		m_currentValue = (int)(lc_floor(newValue));
		m_currentValueOffset = 0;
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
void NdhsCFieldInt::takeSnapshot()
{
	if (isInput())
	{
		m_snapshotValue = m_currentValue;
		m_snapshotValueOffset = m_currentValueOffset;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldInt::addToSnapshot(LcTScalar dValue, LcTScalar minValue, LcTScalar maxValue, bool finalUpdate)
{
	if (isInput())
	{
		int oldValue = m_currentValue;

		m_currentValue = m_snapshotValue;
		m_currentValueOffset = m_snapshotValueOffset + dValue;

		if (m_currentValueOffset >= 1.0 || m_currentValueOffset <= -1.0)
		{
			m_currentValue += (int)m_currentValueOffset;
			m_currentValueOffset -= (int)m_currentValueOffset;
		}

		m_lastUpdatedValid = false;
		m_currentValue = min((int)maxValue, max((int)minValue, m_currentValue));
		m_currentVelocity = 0;
		setAnimating(false);

		if (oldValue != m_currentValue || finalUpdate)
		{
			valueUpdated(finalUpdate);
		}
	}
}


/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldInt::valueUpdated(bool finalUpdate)
{
	if (isInput())
	{
		if (isPluginField())
		{
			IFXI_VARIANT_TYPE val;
			val.i = m_currentValue;
			getMenuCon()->getPlugin()->notifyFieldValueRequest(getMenuItemIndex(), getFieldName(), getMenu(), val, finalUpdate);
		}

		broadcastFieldValueUpdate();
	}
}


/*-------------------------------------------------------------------------*//**
*/
int NdhsCFieldInt::getRawFieldDataInt(NdhsCElement* element)
{
	int retVal = m_currentValue;

	if (isOutput() && atRest())
	{
#ifdef IFX_USE_PLUGIN_ELEMENTS
		if (element && element->getPluginElement())
		{
			IFXI_VARIANT_TYPE value;
			IFXI_FIELD_SCOPE scope;
			if (getMenuCon()->getPlugin()->getRawElementData(getMenuItemIndex(), getFieldName(), element, getMenu(), value, &scope))
			{
				retVal = value.i;
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
			if (fieldNeedsPluginUpdate())
			{
				IFXI_VARIANT_TYPE value;
				if (getMenuCon()->getPlugin()->getRawElementData(getMenuItemIndex(), getFieldName(), NULL, getMenu(), value, NULL))
				{
					m_currentValue = value.i;
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
void NdhsCFieldInt::setValueFromExpression(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item)
{
	if (expr)
	{
		int oldCachedValue = m_currentValue;
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
					m_currentValue = expr->getValueInt();
					m_currentValueOffset = 0;
				}
				break;

				case NdhsCExpression::ENdhsExpressionTypeScalar:
				{
					m_currentValue = (int)expr->getValueScalar();
					m_currentValueOffset = 0;
				}
				break;

				default:
				{
					setErrorStatus(true);
					expr->errorDiagnostics("Integer variable \"" + getFieldName() + "\" value set to non-numeric", false);
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
#ifdef IFX_SERIALIZATION
NdhsCFieldInt* NdhsCFieldInt::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCFieldInt *obj=new NdhsCFieldInt();
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCFieldInt::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCFieldInt) - sizeof(NdhsCField)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCField::serialize(serializeMaster,true);
	ENdhsCFieldType dataType=ENdhsCFieldTypeInt;
	SERIALIZE(dataType,serializeMaster,cPtr)
	SERIALIZE(parentHandle,serializeMaster,cPtr)
	SERIALIZE(m_animating,serializeMaster,cPtr)
	SERIALIZE(m_currentStringValueBase,serializeMaster,cPtr)
	SERIALIZE(m_snapshotValueOffset,serializeMaster,cPtr)
	SERIALIZE(m_snapshotValue,serializeMaster,cPtr)
	SERIALIZE(m_currentValueOffset,serializeMaster,cPtr)
	SERIALIZE(m_currentValue,serializeMaster,cPtr)
	SERIALIZE(m_animRangeDefined,serializeMaster,cPtr)
	SERIALIZE(m_animMaxValue,serializeMaster,cPtr)
	SERIALIZE(m_animMinValue,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCFieldInt::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummySize=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;
	DESERIALIZE(dummySize,serializeMaster,cPtr)
	DESERIALIZE(parentHandle,serializeMaster,cPtr)
	NdhsCField::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE(m_animating,serializeMaster,cPtr)
	DESERIALIZE(m_currentStringValueBase,serializeMaster,cPtr)
	DESERIALIZE(m_snapshotValueOffset,serializeMaster,cPtr)
	DESERIALIZE(m_snapshotValue,serializeMaster,cPtr)
	DESERIALIZE(m_currentValueOffset,serializeMaster,cPtr)
	DESERIALIZE(m_currentValue,serializeMaster,cPtr)
	DESERIALIZE(m_animRangeDefined,serializeMaster,cPtr)
	DESERIALIZE(m_animMaxValue,serializeMaster,cPtr)
	DESERIALIZE(m_animMinValue,serializeMaster,cPtr)
}
#endif /* IFX_SERIALIZATION */
/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldInt::setVelocity(LcTScalar velocity)
{
	NdhsCField::setVelocity(velocity);
	m_animRangeDefined = false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldInt::setVelocity(LcTScalar velocity, LcTScalar minValue, LcTScalar maxValue)
{
	NdhsCField::setVelocity(velocity);

	m_animMinValue = (int)minValue;
	m_animMaxValue = (int)maxValue;
	m_animRangeDefined = true;
}
