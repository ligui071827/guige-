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
LcTaOwner<NdhsCFieldBool> NdhsCFieldBool::create(			NdhsCMenuCon* menuCon,
															const LcTmString& fieldName,
															bool pluginField,
															NdhsCPlugin::NdhsCPluginMenu* menu,
															int menuItemIndex,
															IFXI_FIELD_SCOPE scope,
															bool isInput,
															bool isOutput,
															bool defaultValue)
{
	LcTaOwner<NdhsCFieldBool> ref;
	ref.set(new NdhsCFieldBool(menuCon, fieldName, pluginField, menu, menuItemIndex, scope, isInput, isOutput, defaultValue));
	ref->construct();
	return ref;
}
#ifdef IFX_SERIALIZATION
NdhsCFieldBool* NdhsCFieldBool::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCFieldBool *obj=new NdhsCFieldBool();
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCFieldBool::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCFieldBool) - sizeof(NdhsCField)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCField::serialize(serializeMaster,true);
	ENdhsCFieldType dataType=ENdhsCFieldTypeBool;
	SERIALIZE(dataType,serializeMaster,cPtr)
	SERIALIZE(parentHandle,serializeMaster,cPtr)
	SERIALIZE(m_currentValue,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCFieldBool::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummySize=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;
	DESERIALIZE(dummySize,serializeMaster,cPtr)
	DESERIALIZE(parentHandle,serializeMaster,cPtr)
	NdhsCField::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE(m_currentValue,serializeMaster,cPtr)
}
#endif /* IFX_SERIALIZATION */
/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldBool::construct()
{
	if (isInput())
	{
		IFXI_VARIANT_TYPE value;
		
		if (getMenuCon())
		{
			if (isPluginField() && getMenuCon()->getPlugin()->getRawElementData(getMenuItemIndex(), getFieldName(), NULL, getMenu(), value, NULL))
			{
				m_currentValue = (value.b == IFX_FALSE ? false : true);
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCFieldBool::getFieldData(NdhsCElement* element)
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
			val.b = m_currentValue;
			
			if (getMenuCon())
			{
				getMenuCon()->getPlugin()->translateRawElementData(getFieldName(), getMenu(), val, retVal);
			}
		}
		else
		{
			retVal = m_currentValue ? "true" : "false";
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCFieldBool::getFieldData(NdhsCElement* element, bool forceBroadcast)
{
	LcTaString retVal = "";
	
	if (isOutput())
	{
		bool oldValue = m_currentValue;
		bool data = getRawFieldDataBool(element);
		
		// Check if the cached string needs updating
		if (m_currentStringValueBase != m_currentValue || m_currentStringValue.isEmpty())
		{
			m_currentStringValueBase = m_currentValue;
			
			if (isPluginField())
			{
				IFXI_VARIANT_TYPE val;
				val.b = m_currentValue;
				getMenuCon()->getPlugin()->translateRawElementData(getFieldName(), getMenu(), val, m_currentStringValue);
			}
			else
			{
				m_currentStringValue = m_currentValue ? "true" : "false";
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
				val.b = data;
				getMenuCon()->getPlugin()->translateRawElementData(getFieldName(), getMenu(), val, retVal);
			}
			else
			{
				retVal = data ? "true" : "false";
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
void NdhsCFieldBool::setValueFromExpression(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item)
{
	if (expr)
	{
		bool oldCachedValue = m_currentValue;
		bool oldIsError = isError();

		expr->evaluate(context, slot, item);

		if (expr->isError())
		{
			setErrorStatus(true);
			expr->errorDiagnostics("Variable \"" + getFieldName() + "\" value", false);
		}
		else if (!expr->isBool())
		{
			setErrorStatus(true);
			expr->errorDiagnostics("Boolean variable \"" + getFieldName() + "\" value set to non-boolean", false);
		}
		else
		{
			setErrorStatus(false);
			m_currentValue = expr->getValueBool();
		}

		if (oldCachedValue != m_currentValue || oldIsError != isError())
		{
			// Check whether we are about to give the module erroneous data
			if (isInput() && isPluginField() && isError())
			{
				expr->errorDiagnostics("Module field", true);
			}
			else
			{
				if (isInput() && isPluginField())
				{
					IFXI_VARIANT_TYPE val;
					val.b = m_currentValue;
					
					if (getMenuCon())
					{
						getMenuCon()->getPlugin()->notifyFieldValueRequest(getMenuItemIndex(), getFieldName(), getMenu(), val, true);
					}
				}
				
				broadcastFieldValueUpdate();
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldBool::setValueBool(bool newValue, bool finalUpdate)
{
	LC_UNUSED(finalUpdate);

	if (newValue != m_currentValue)
	{
		m_currentValue = newValue;
		broadcastFieldValueUpdate();
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldBool::getRawFieldDataBool(NdhsCElement* element)
{ 
	bool retVal = m_currentValue;

	if (isOutput())
	{
#ifdef IFX_USE_PLUGIN_ELEMENTS
		if (isPluginField() && element && element->getPluginElement())
		{
			IFXI_FIELD_SCOPE scope;
			IFXI_VARIANT_TYPE value;
			
			if (getMenuCon()->getPlugin()->getRawElementData(getMenuItemIndex(), getFieldName(), element, getMenu(), value, &scope))
			{
				retVal = (value.b == IFX_FALSE ? false : true);
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
					m_currentValue = (value.b == IFX_FALSE ? false : true);
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
