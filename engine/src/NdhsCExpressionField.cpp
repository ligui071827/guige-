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
LcTaOwner<NdhsCExpressionField> NdhsCExpressionField::create(const LcTmString& fieldName, NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem * item)
{
	LcTaOwner<NdhsCExpressionField> ref;
	ref.set(new NdhsCExpressionField());
	ref->construct(fieldName, fieldContext, slotNum, item);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCExpressionField::construct(const LcTmString& fieldName, NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem *item)
{
	m_createdWithContext = false;
	m_fieldName = fieldName;

	if (fieldContext)
	{
		m_field = fieldContext->getField(fieldName, slotNum, item);

		if (m_field)
		{
			m_field->addObserver(this);
			m_createdWithContext = true;
		}
		else
		{
			bool found = false;
			LcTaString fieldData = fieldContext->getString(found, fieldName, slotNum, item);

			if (found)
			{
				setValueString(fieldData);
				setDirty(false);
			}
		}
	}
}

#ifdef IFX_SERIALIZATION

ISerializeable * NdhsCExpressionField::getSerializeAble(int &type)
{
	type=2;
	return this;
}

NdhsCExpressionField* NdhsCExpressionField::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCExpressionField*obj=new NdhsCExpressionField();
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCExpressionField::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCExpressionField)+sizeof(IFX_INT32);
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle= NdhsCExpression::serialize(serializeMaster,true);
	EExpressionClassType classType=EExpressionClassTypeField;
	SERIALIZE(classType,serializeMaster,cPtr)
	SERIALIZE(parentHandle,serializeMaster,cPtr)
	SERIALIZE(m_createdWithContext,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_field,serializeMaster,cPtr)
	SERIALIZE_String(m_fieldName,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}

void	NdhsCExpressionField::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummySize=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle=-1;
	DESERIALIZE(dummySize,serializeMaster,cPtr)
	DESERIALIZE(parentHandle,serializeMaster,cPtr)
	NdhsCExpression::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE(m_createdWithContext,serializeMaster,cPtr)
	DESERIALIZE_Ptr(m_field,serializeMaster,cPtr,NdhsCField)
	DESERIALIZE_String(m_fieldName,serializeMaster,cPtr)
}
#endif /* IFX_SERIALIZATION */

/*-------------------------------------------------------------------------*//**
	get field out of the expression field
*/
NdhsCField *NdhsCExpressionField::getField(NdhsIFieldContext* context, int slotnum, NdhsCMenuItem* item)
{
	if (m_createdWithContext)
		return m_field;
	else if (context!=NULL)
		return context->getField(m_fieldName,slotnum, item);
	else
		return NULL;
}

/*-------------------------------------------------------------------------*//**
	Expression evaluation for field expression
*/
NdhsCExpression::ENdhsExpressionType NdhsCExpressionField::evaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item)
{
	if (fieldContext || isDirty())
	{
		NdhsCField* field = NULL;

		// If expression was created with a successful context, always use that in preference to any provided
		if (m_createdWithContext)
		{
			field = m_field;
			if (!field)
			{
				setValueError(ENdhsExpressionErrorUnknownField);
			}
		}
		else if (fieldContext)
		{
			// First check for fields
			field = fieldContext->getField(m_fieldName, slotNum, item);

			if (!field)
			{
				// Ask for string data - static menu data will override other fields
				bool found = false;
				LcTaString fieldData = fieldContext->getString(found, m_fieldName, slotNum, item);

				if (found)
				{
					setValueString(fieldData);
				}
				else
				{
					setValueError(ENdhsExpressionErrorUnknownField);
				}
			}
		}
		else
		{
			setValueError(ENdhsExpressionErrorUnknownField);
		}

	    if (field)
		{
			if (field->isError())
			{
				setValueError(ENdhsExpressionErrorFieldHasError);
			}
			else
			{
				switch(field->getFieldType())
				{
					case IFXI_FIELDDATA_INT:
					{
						int fieldData = (int)field->getRawFieldData(NULL);
						setValueInt(fieldData);
						break;
					}

					case IFXI_FIELDDATA_FLOAT:
					{
						LcTScalar fieldData = field->getRawFieldData(NULL);
						setValueScalar(fieldData);
						break;
					}

					case IFXI_FIELDDATA_STRING:
					{
						LcTaString fieldData = field->getFieldData(NULL);
						setValueString(fieldData);
						break;
					}

					case IFXI_FIELDDATA_BOOL:
					{
						bool fieldData = field->getRawFieldDataBool(NULL);
						setValueBool(fieldData);
						break;
					}

					case IFXI_FIELDDATA_TIME:
					{
						IFX_TIME fieldData = (IFX_TIME) field->getRawFieldData(NULL);
						setValueTime(fieldData);
						break;
					}

					default:
					{
						setValueError(ENdhsExpressionErrorInternal);
						break;
					}
				}
			}

			if (m_createdWithContext)
			{
				setDirty(false);
			}
			else
			{
				// Store field used in case string is queried later
				m_field = field;
			}
		}
	}

	return expressionType();
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCExpressionField::getValueString()
{
	if (expressionType() == ENdhsExpressionTypeString)
	{
		// Correct string already set
		return NdhsCExpression::getValueString();
	}
	else
	{
		// Always ask the field for the string, as it will apply any field data mapping
		return m_field ? m_field->getFieldData(NULL) : "";
	}
}

#ifdef NDHS_TRACE_ENABLED
/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCExpressionField::showExpression(bool showValues) const
{
	LcTaString retVal;

	if (showValues)
	{
		// Check for errors before showing value
		if (m_field && m_field->isError())
		{
			retVal = "[error]";
		}
		else
		{
			retVal = NdhsCExpression::showExpression(true);
		}
	}
	else
	{
		retVal = m_fieldName;
	}

	return retVal;
}
#endif // def NDHS_TRACE_ENABLED

#ifdef NDHS_TRACE_ENABLED
/*-------------------------------------------------------------------------*//**
*/
void NdhsCExpressionField::gatherErrorReports(LcTmString& errorOutput) const
{
	LcTaString errorDescription;

	switch (errorType())
	{
		case ENdhsExpressionErrorUnknownField:
		{
			errorDescription = "Cannot find variable \"" + m_fieldName + "\"";
		}
		break;

		case ENdhsExpressionErrorFieldHasError:
		{
			errorDescription = "Variable \"" + m_fieldName + "\" has an error value";
		}
		break;

		default:
			break;
	}

	if (!errorDescription.isEmpty())
	{
		if (errorOutput.isEmpty())
		{
			errorOutput = errorDescription;
		}
		else
		{
			errorOutput += "; " + errorDescription;
		}
	}
}
#endif // def NDHS_TRACE_ENABLED

/*-------------------------------------------------------------------------*//**
*/
void NdhsCExpressionField::fieldDestroyed(NdhsCField* field)
{
	if (field && field == m_field)
	{
		m_field = NULL;
		setValueError(ENdhsExpressionErrorUnknownField);
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCExpressionField::~NdhsCExpressionField()
{
	if (m_field && m_createdWithContext)
	{
		m_field->removeObserver(this);
	}
}
