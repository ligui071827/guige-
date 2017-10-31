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
#ifndef NdhsCFieldBoolH
#define NdhsCFieldBoolH

#include "inflexionui/engine/inc/NdhsCField.h"

class NdhsCFieldBool : public NdhsCField
{
private:
	bool 						m_currentValue;
	bool						m_currentStringValueBase;

protected:
								NdhsCFieldBool(){}
								NdhsCFieldBool(		NdhsCMenuCon* menuCon,
													const LcTmString& fieldName,
													bool pluginField,
													NdhsCPlugin::NdhsCPluginMenu* menu,
													int menuItemIndex,
													IFXI_FIELD_SCOPE scope,
													bool isInput,
													bool isOutput,
													bool defaultValue)
								: 	NdhsCField(menuCon, fieldName, pluginField, menu, menuItemIndex, IFXI_FIELDDATA_BOOL, scope, isInput, isOutput, ""),
									m_currentValue(defaultValue)
									{ }

	void						construct();

	// Output fields
	virtual	LcTaString			getFieldData(NdhsCElement* element, bool forceBroadcast);
	virtual LcTaString			getFieldData(NdhsCElement* element);

	virtual bool				isAnimating()					{ return false; }

public:
	static LcTaOwner<NdhsCFieldBool> create(	NdhsCMenuCon* menuCon,
												const LcTmString& fieldName,
												bool pluginField,
												NdhsCPlugin::NdhsCPluginMenu* menu,
												int menuItemIndex,
												IFXI_FIELD_SCOPE scope,
												bool isInput,
												bool isOutput,
												bool defaultValue);
#ifdef IFX_SERIALIZATION
	static NdhsCFieldBool* loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual		SerializeHandle	serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual		void	deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */

	/* Data access */
	virtual bool 				getRawFieldDataBool(NdhsCElement* element);
	virtual void				setValueFromExpression(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item);
	virtual void				setValueBool(bool newValue, bool finalUpdate);

	virtual						~NdhsCFieldBool()	{}
};


#endif /* NdhsCFieldBoolH */
