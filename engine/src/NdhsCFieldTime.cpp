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
LcTaOwner<NdhsCFieldTime> NdhsCFieldTime::create(			NdhsCMenuCon* menuCon,
															const LcTmString& fieldName,
															bool pluginField,
															NdhsCPlugin::NdhsCPluginMenu* menu,
															int menuItemIndex,
															IFXI_FIELD_SCOPE scope,
															bool isInput,
															bool isOutput,
															int defaultValue)
{
	LcTaOwner<NdhsCFieldTime> ref;
	ref.set(new NdhsCFieldTime(menuCon, fieldName, pluginField, menu, menuItemIndex, scope, false, isOutput, defaultValue));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldTime::construct()
{
/*	if (isInput())
	{
		IFXI_VARIANT_TYPE value;
		if (isPluginField() && getMenuCon()->getPlugin()->getRawElementData(getMenuItemIndex(), getFieldName(), NULL, getMenu(), value, NULL))
		{
			m_currentValue = value.t;
		}
	}*/
}

LcTaString NdhsCFieldTime::getFieldData(ENdhsFieldTimeFormat format)
{
	LcTWChar wcoutput[1024];

	if(!m_timeUTCUpdated)
	{
		struct tm *tim=lc_gmtime(&m_currentValue);
		if(tim)
		{
			m_currentTimeUTC=*tim;
			m_timeUTCUpdated=true;
		}
	}

	if(m_timeUTCUpdated)
	{
		switch (format)
		{
			case ENdhsFieldTimeFormatDefault:
			case ENdhsFieldTimeFormatShortDate:
			{
				lc_wcsftime(wcoutput, 1000,(const IFX_WCHAR*) DEFAULT_TIME_FORMAT, &m_currentTimeUTC);
				m_currentStringValue.fromBufWChar(wcoutput,lc_wcslen(wcoutput));
			}
			break;

			default:
				break;
		}
	}
	return m_currentStringValue;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCFieldTime::getFieldData(NdhsCElement* element)
{
	LcTaString retVal = "";

	if (isOutput())
	{
		retVal = getFieldData(element, false);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCFieldTime::getFieldData(NdhsCElement* element, bool forceBroadcast)
{
	LcTaString retVal = "";

	if (isOutput())
	{
		IFX_TIME oldValue = m_currentValue;

		IFX_TIME data = getRawFieldDataTime(element);

		// Check if the cached string needs updating
		if (m_currentStringValueBase != m_currentValue || m_currentStringValue.isEmpty())
		{
			m_currentStringValueBase = m_currentValue;

			if (isPluginField())
			{
				// Query IL based on m_currentValue
				IFXI_VARIANT_TYPE val;
				val.t = m_currentValue;
				getMenuCon()->getPlugin()->translateRawElementData(getFieldName(), getMenu(), val, m_currentStringValue);
			}
			else
			{
				retVal = getFieldData(ENdhsFieldTimeFormatDefault);
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
				val.t = data;
				getMenuCon()->getPlugin()->translateRawElementData(getFieldName(), getMenu(), val, retVal);
			}
			else
			{
				if (m_currentStringValueBase != m_currentValue || m_currentStringValue.isEmpty())
				{
					m_currentStringValueBase = m_currentValue;
					retVal = getFieldData(ENdhsFieldTimeFormatDefault);
				}
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
LcTScalar NdhsCFieldTime::getRawFieldData(NdhsCElement* element)
{
	return (LcTScalar)getRawFieldDataTime(element);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldTime::setTargetValue(LcTScalar target, unsigned int duration)
{
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldTime::setTargetValue(LcTScalar target, LcTScalar minValue, LcTScalar maxValue, unsigned int duration)
{
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldTime::updateValue(LcTTime timestamp, bool& finalUpdate)
{
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldTime::setValue(LcTScalar newValue, bool finalUpdate)
{

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldTime::takeSnapshot()
{

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldTime::addToSnapshot(LcTScalar dValue, LcTScalar minValue, LcTScalar maxValue, bool finalUpdate)
{

}


/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldTime::valueUpdated(bool finalUpdate)
{

}


/*-------------------------------------------------------------------------*//**
*/
IFX_TIME NdhsCFieldTime::getRawFieldDataTime(NdhsCElement* element)
{
	IFX_TIME retVal = m_currentValue;

	if (isOutput() && atRest())
	{
#ifdef IFX_USE_PLUGIN_ELEMENTS
		if (element && element->getPluginElement())
		{
			IFXI_VARIANT_TYPE value;
			IFXI_FIELD_SCOPE scope;
			if (getMenuCon()->getPlugin()->getRawElementData(getMenuItemIndex(), getFieldName(), element, getMenu(), value, &scope))
			{
				retVal = value.t;
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
					m_currentValue = value.t;
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
void NdhsCFieldTime::setValueFromExpression(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item)
{
	if (expr)
	{
		IFX_TIME oldCachedValue = m_currentValue;
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
				}
				break;

				case NdhsCExpression::ENdhsExpressionTypeTime:
				{
					m_currentValue = (int)expr->getValueTime();
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
			broadcastFieldValueUpdate();
		}
	}
}
#ifdef IFX_SERIALIZATION
NdhsCFieldTime* NdhsCFieldTime::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCFieldTime *obj=new NdhsCFieldTime();
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCFieldTime::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCFieldTime) - sizeof(NdhsCField)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCField::serialize(serializeMaster,true);
	ENdhsCFieldType dataType=ENdhsCFieldTypeTime;
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
void	NdhsCFieldTime::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
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
	SERIALIZE(m_animMinValue,serializeMaster,cPtr)

	m_timeUTCUpdated = false;
}
#endif /* IFX_SERIALIZATION */
/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldTime::setVelocity(LcTScalar velocity)
{

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldTime::setVelocity(LcTScalar velocity, LcTScalar minValue, LcTScalar maxValue)
{

}
