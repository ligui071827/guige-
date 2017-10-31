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
#ifndef NdhsCFieldCacheH
#define NdhsCFieldCacheH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTString.h"
#include "inflexionui/engine/inc/LcCSerializeMaster.h"

class NdhsCFieldCache : public LcCBase, public ISerializeable
{
private:
	LcTmOwnerMap<LcTmString, NdhsCField>	m_fieldMap;
	LcTmMap<LcTmString, NdhsCField*>		m_referencedFieldMap;
	NdhsCMenuCon* 							m_menuCon;

protected:
										NdhsCFieldCache(NdhsCMenuCon* menuCon)
										:	m_menuCon(menuCon)
										{}

#ifdef IFX_SERIALIZATION
	bool m_menuItemChild;	
#endif /* IFX_SERIALIZATION */

public:
	static LcTaOwner<NdhsCFieldCache>	create(NdhsCMenuCon* menuCon);
	virtual 							~NdhsCFieldCache();
	void								addLocalVariables(NdhsCTemplate* templ, NdhsIFieldContext* context);
	void								addInputParameters(NdhsCTemplate* templ, NdhsIFieldContext* context, NdhsCElementGroup* aggregate);
	void								bindInputParameter(const LcTmString& param, NdhsCExpression::CExprSkeleton* value, NdhsIFieldContext* expressionContext, int slotNum, NdhsCMenuItem* menuItem);
	void								bindOutputParameter(const LcTmString& param, NdhsCExpression::CExprSkeleton* value, NdhsIFieldContext* expressionContext, int slotNum, NdhsCMenuItem* menuItem);
	bool								createMenuComponentIntrinsicFields();
	bool								createPageIntrinsicFields();
	bool								createComponentIntrinsicFields();

	NdhsCField*							getField(const LcTmString& name);

#ifdef IFX_SERIALIZATION
	static NdhsCFieldCache* loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	SerializeHandle	serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	void	deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	bool isMenuItemChild(){return m_menuItemChild;}	
	void setMenuItemChild(bool val);
#endif /* IFX_SERIALIZATION */
};

#endif // def NdhsCFieldCacheH
