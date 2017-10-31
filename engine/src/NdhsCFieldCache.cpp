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
LcTaOwner<NdhsCFieldCache>	NdhsCFieldCache::create(NdhsCMenuCon* menuCon)
{
	LcTaOwner<NdhsCFieldCache> ref;
	ref.set(new NdhsCFieldCache(menuCon));
	//ref->construct();
	return ref;
}
#ifdef IFX_SERIALIZATION
NdhsCFieldCache* NdhsCFieldCache::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCFieldCache *obj=new NdhsCFieldCache(NULL);
	if(handle==-1)
		return obj;
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}
SerializeHandle	NdhsCFieldCache::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCFieldCache)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SERIALIZE_Reserve(m_menuCon,serializeMaster,cPtr)
	SERIALIZE_MapString(m_fieldMap,serializeMaster,cPtr)
	SERIALIZE_MapString(m_referencedFieldMap,serializeMaster,cPtr)
	SERIALIZE(m_menuItemChild,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;

}

void NdhsCFieldCache::setMenuItemChild(bool val)
{
	if(m_menuItemChild!=val)
	{
		m_menuItemChild=val;
		LcTmOwnerMap<LcTmString, NdhsCField>::iterator it = m_fieldMap.begin();
		for(;it!=m_fieldMap.end();it++)
		{
			it->second->setMenuItemChild(m_menuItemChild);
		}

		it = m_referencedFieldMap.begin();
		for(;it!=m_referencedFieldMap.end();it++)
		{
			it->second->setMenuItemChild(m_menuItemChild);
		}
	}
}
void NdhsCFieldCache::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	DESERIALIZE_Reserve(m_menuCon,serializeMaster,cPtr,NdhsCMenuCon)
	DESERIALIZE_MapString(m_fieldMap,serializeMaster,cPtr)
	DESERIALIZE_MapString(m_referencedFieldMap,serializeMaster,cPtr)
	DESERIALIZE(m_menuItemChild,serializeMaster,cPtr)
}
#endif /* IFX_SERIALIZATION */
/*-------------------------------------------------------------------------*//**
*/
NdhsCFieldCache::~NdhsCFieldCache()
{
	m_fieldMap.clear();
}
/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldCache::addLocalVariables(NdhsCTemplate* templ, NdhsIFieldContext* context)
{
	if (templ)
	{
		NdhsCTemplate::TmMVariableInfoList& varList = templ->getVariableInfoList();

		// Add all fields to map
		NdhsCTemplate::TmMVariableInfoList::iterator varIt = varList.begin();

		for (; varIt != varList.end(); varIt++)
		{
			NdhsCTemplate::CVariableInfo* varInfo = (NdhsCTemplate::CVariableInfo*)(*varIt);

			IFXI_FIELD_MODE mode = varInfo->boundExpression->isEmpty() ? IFXI_FIELD_MODE_INPUT_OUTPUT : IFXI_FIELD_MODE_OUTPUT;

			LcTaOwner<NdhsCField> field = NdhsCField::create(m_menuCon, varInfo->name, false, NULL, -1, NULL,
					mode, IFXI_FIELD_SCOPE_LOCAL, varInfo->type);

			m_fieldMap.add_element(varInfo->name, field);
		}

		// Set up expressions
		varIt = varList.begin();

		for (; varIt != varList.end(); varIt++)
		{
			NdhsCTemplate::CVariableInfo* varInfo = (NdhsCTemplate::CVariableInfo*)(*varIt);

			if (!varInfo->boundExpression->isEmpty())
			{
				// Set up bound variables
				LcTaOwner<NdhsCExpression> expr = varInfo->boundExpression->createExpression(context);

				if (expr)
				{
					m_fieldMap[varInfo->name]->bindExpression(expr);
				}
			}
			else if (!varInfo->defaultValue->isEmpty())
			{
				// Set up assignable variables
				NdhsCExpression *expr2 = varInfo->defaultValue->getContextFreeExpression();
				m_fieldMap[varInfo->name]->setDirtyByAssignment(expr2, context, -1, NULL);
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldCache::addInputParameters(NdhsCTemplate* templ, NdhsIFieldContext* context, NdhsCElementGroup* aggregate)
{
	if (templ)
	{
		// Get parameter list
		NdhsCTemplate::TmMParameterMap& paramsMap = templ->getParametersMap();

		NdhsCTemplate::TmMParameterMap::iterator paramIt = paramsMap.begin();

		for (; paramIt != paramsMap.end(); paramIt++)
		{
			NdhsCTemplate::CParameters* param = (NdhsCTemplate::CParameters*)(paramIt->second);

			if (param)
			{
				// We have to add 'inputOutput' params in fieldMap also so that their 'defaultValue'
				// attribute work
				if (IFXI_FIELD_MODE_OUTPUT == param->mode || IFXI_FIELD_MODE_INPUT_OUTPUT == param->mode)
				{
					LcTaOwner<NdhsCField> field = NdhsCField::create(m_menuCon, param->name, false, NULL, -1, NULL,
							param->mode, param->scope, param->type);

					if (field)
					{
						field->addObserver(aggregate);
						m_fieldMap.add_element(param->name, field);

						if (!param->defaultValue->isEmpty())
						{
							NdhsCExpression *expr2 = param->defaultValue->getContextFreeExpression();
							m_fieldMap[param->name]->setDirtyByAssignment(expr2, context, -1, NULL);
						}
					}
				}
			}
		}

		// Clean all of the expressions
		if (m_menuCon)
		{
			m_menuCon->getLaundry()->cleanAll(true);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCFieldCache::getField(const LcTmString& name)
{
	NdhsCField* retVal = NULL;

	// Convert search string to lower case.
	LcTaString lowerName = name.toLower();

	LcTmMap<LcTmString, NdhsCField*>::iterator refIt = m_referencedFieldMap.find(lowerName);

	if (refIt != m_referencedFieldMap.end())
	{
		retVal = refIt->second;
	}

	if (!retVal)
	{
		LcTmOwnerMap<LcTmString, NdhsCField>::iterator it = m_fieldMap.find(lowerName);

		if (it != m_fieldMap.end())
		{
			retVal = it->second;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldCache::bindInputParameter(const LcTmString& param, NdhsCExpression::CExprSkeleton* value, NdhsIFieldContext* expressionContext, int slotNum, NdhsCMenuItem* menuItem)
{
	NdhsCField* cachedField = getField(param);
	if (expressionContext && value && cachedField)
	{
		LcTaOwner<NdhsCExpression> expr = value->createExpression(expressionContext, slotNum, menuItem);

		if(expr && cachedField->isOutput())
		{
			cachedField->bindExpression(expr);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCFieldCache::bindOutputParameter(const LcTmString& param, NdhsCExpression::CExprSkeleton* value, NdhsIFieldContext* expressionContext, int slotNum, NdhsCMenuItem* menuItem)
{
	if (expressionContext && value)
	{
		LcTaOwner<NdhsCExpression> expr = value->createExpression(expressionContext, slotNum, menuItem);

		if (expr)
		{
			bool isLvalue = expr->isLvalue();
			NdhsCField* field = expr->getField();

			if(isLvalue && field)
			{
				m_referencedFieldMap[param.toLower()] = field;
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldCache::createPageIntrinsicFields()
{
	LcTaOwner<NdhsCField> field;

	// True when ever page there is page state change
	field = NdhsCField::create(m_menuCon, "_pagestate", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_STRING);

	if (field)
	{
		field->setValue("PreOpen", true);
		m_fieldMap.add_element("_pagestate", field);
	}
	else
	{
		return false;
	}

	// True when ever page is transitioning between layouts
	field = NdhsCField::create(m_menuCon, "_transitioning", false, NULL, -1, NULL,
		IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_BOOL);

	if (field)
	{
		m_fieldMap.add_element("_transitioning", field);
	}
	else
	{
		return false;
	}

	// Set to the element being touched down
	field = NdhsCField::create(m_menuCon, "_touchelement", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_STRING);

	if (field)
	{
		m_fieldMap.add_element("_touchelement", field);
	}
	else
	{
		return false;
	}

	// Set to the element in focus
	field = NdhsCField::create(m_menuCon, "_focuselement", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_STRING);

	if (field)
	{
		m_fieldMap.add_element("_focuselement", field);
	}
	else
	{
		return false;
	}
#ifdef LC_USE_MOUSEOVER
	// Set to the element being mouse over
	field = NdhsCField::create(m_menuCon, "_mouseoverelement", false, NULL, -1, NULL,
		IFXI_FIELD_MODE_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_STRING);

	if (field)
	{
		m_fieldMap.add_element("_mouseoverelement", field);
	}
	else
	{
		return false;
	}
#endif

	// Dummy field for menu slot placeholder feature - this will always return 'false' and 
	// never be updated
	field = NdhsCField::create(m_menuCon, "_placeholderactive", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_BOOL);

	if (field)
	{
		field->setValueBool(false, true);
		m_fieldMap.add_element("_placeholderactive", field);
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldCache::createComponentIntrinsicFields()
{
	LcTaOwner<NdhsCField> field;

	// True when ever component is transitioning between layouts
	field = NdhsCField::create(m_menuCon, "_transitioning", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_BOOL);

	if (field)
	{
		m_fieldMap.add_element("_transitioning", field);
	}
	else
	{
		return false;
	}

	// True whenever component is focus enabled
	field = NdhsCField::create(m_menuCon, "_focusenabled", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_BOOL);

	if (field)
	{
		field->setValueBool(true, true);
		m_fieldMap.add_element("_focusenabled", field);
	}
	else
	{
		return false;
	}

	// True when ever component is focused
	field = NdhsCField::create(m_menuCon, "_focused", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_BOOL);

	if (field)
	{
		field->setValueBool(false, true);
		m_fieldMap.add_element("_focused", field);
	}
	else
	{
		return false;
	}

	// Set to the element in focus
	field = NdhsCField::create(m_menuCon, "_focuselement", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_STRING);

	if (field)
	{
		m_fieldMap.add_element("_focuselement", field);
	}
	else
	{
		return false;
	}

	// Set to the element being touched down
	field = NdhsCField::create(m_menuCon, "_touchelement", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_STRING);

	if (field)
	{
		m_fieldMap.add_element("_touchelement", field);
	}
	else
	{
		return false;
	}

	// This field contains the extent X
	field = NdhsCField::create(m_menuCon, "_extentx", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_FLOAT);

	if (field)
	{
		m_fieldMap.add_element("_extentx", field);
	}
	else
	{
		return false;
	}

	// This field contains the extent Y
	field = NdhsCField::create(m_menuCon, "_extenty", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_FLOAT);

	if (field)
	{
		m_fieldMap.add_element("_extenty", field);
	}
	else
	{
		return false;
	}

	// This field contains the extent Z
	field = NdhsCField::create(m_menuCon, "_extentz", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_FLOAT);

	if (field)
	{
		m_fieldMap.add_element("_extentz", field);
	}
	else
	{
		return false;
	}

	// Component special intrinsic
	field = NdhsCField::create(m_menuCon, "_pagestate", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_STRING);

	if (field)
	{
		m_fieldMap.add_element("_pagestate", field);
	}
	else
	{
		return false;
	}
#ifdef LC_USE_MOUSEOVER
	// Set to the element being mouse over
	field = NdhsCField::create(m_menuCon, "_mouseoverelement", false, NULL, -1, NULL,
		IFXI_FIELD_MODE_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_STRING);

	if (field)
	{
		m_fieldMap.add_element("_mouseoverelement", field);
	}
	else
	{
		return false;
	}
#endif

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCFieldCache::createMenuComponentIntrinsicFields()
{
	LcTaOwner<NdhsCField> field;

	// Create item count field
	field = NdhsCField::create(m_menuCon, "_itemcount", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_INT);

	if (field)
	{
		m_fieldMap.add_element("_itemcount", field);
	}
	else
	{
		return false;
	}

	// Create active slot field
	field = NdhsCField::create(m_menuCon, "_activeslot", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_INT);

	if (field)
	{
		field->setValue(-1, true);
		m_fieldMap.add_element("_activeslot", field);
	}
	else
	{
		return false;
	}

	// Create scroll position field
	field = NdhsCField::create(m_menuCon, "_scrollpos", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_FLOAT);

	if (field)
	{
		m_fieldMap.add_element("_scrollpos", field);
	}
	else
	{
		return false;
	}

	// This gives the scrollpos velocity
	field = NdhsCField::create(m_menuCon, "_scrollvelocity", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_FLOAT);

	if (field)
	{
		m_fieldMap.add_element("_scrollvelocity", field);
	}
	else
	{
		return false;
	}

	// Set to the slot being touched down, if any
	field = NdhsCField::create(m_menuCon, "_touchslot", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_INT);

	if (field)
	{
		field->setValue(0, true);
		m_fieldMap.add_element("_touchslot", field);
	}
	else
	{
		return false;
	}

	// This field contains the slot number
	field = NdhsCField::create(m_menuCon, "_slotnumber", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_INT);

	if (field)
	{
		field->setValue(-1, true);
		m_fieldMap.add_element("_slotnumber", field);
	}
	else
	{
		return false;
	}

	// This field contains the first selectable slot
	field = NdhsCField::create(m_menuCon, "_firstselectableslot", false, NULL, -1, NULL, 
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_INT);

	if (field)
	{
		field->setValue(-1, true);
		m_fieldMap.add_element("_firstselectableslot", field);
	}
	else
	{
		return false;
	}

	// This field contains the last selectable slot
	field = NdhsCField::create(m_menuCon, "_lastselectableslot", false, NULL, -1, NULL, 
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_INT);

	if (field)
	{
		field->setValue(-1, true);
		m_fieldMap.add_element("_lastselectableslot", field);
	}
	else
	{
		return false;
	}

	// This field contains the menu span
	field = NdhsCField::create(m_menuCon, "_span", false, NULL, -1, NULL, 
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_INT);

	if (field)
	{
		m_fieldMap.add_element("_span", field);
	}
	else
	{
		return false;
	}

	// This field indicates whether wrapping is enabled or not
	field = NdhsCField::create(m_menuCon, "_wrap", false, NULL, -1, NULL, 
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_BOOL);

	if (field)
	{
		m_fieldMap.add_element("_wrap", field);
	}
	else
	{
		return false;
	}

	// This field indicates whether full mode is enabled or not
	field = NdhsCField::create(m_menuCon, "_full", false, NULL, -1, NULL, 
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_BOOL);

	if (field)
	{
		m_fieldMap.add_element("_full", field);
	}
	else
	{
		return false;
	}

#ifdef LC_USE_MOUSEOVER
	// Field containing the 1-based slot index containing _mouseover element if appropriate, or 0 if not set
	field = NdhsCField::create(m_menuCon, "_mouseoverslot", false, NULL, -1, NULL,
			IFXI_FIELD_MODE_INPUT_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_INT);

	if (field)
	{
		field->setValue(0, true);
		m_fieldMap.add_element("_mouseoverslot", field);
	}
	else
	{
		return false;
	}
#endif

	// This field indicates whether wrapping the data source has been loaded or not
	field = NdhsCField::create(m_menuCon, "_datasourceloaded", false, NULL, -1, NULL, 
			IFXI_FIELD_MODE_INPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_BOOL);

	if (field)
	{
		m_fieldMap.add_element("_datasourceloaded", field);
	}
	else
	{
		return false;
	}

	return true;
}
