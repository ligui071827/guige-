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
#ifndef NdhsCFieldFloatH
#define NdhsCFieldFloatH

#include "inflexionui/engine/inc/NdhsCField.h"


class NdhsCFieldFloat : public NdhsCField
{
private:
	LcTScalar					m_animMinValue;
	LcTScalar					m_animMaxValue;
	bool						m_animRangeDefined;

	LcTScalar 					m_currentValue;

	// Input fields
	LcTScalar 					m_snapshotValue;

	// Output fields
	LcTScalar					m_currentStringValueBase;

	bool						m_animating;

protected:
								NdhsCFieldFloat(){}
								NdhsCFieldFloat(		NdhsCMenuCon* menuCon,
														const LcTmString& fieldName,
														bool pluginField,
														NdhsCPlugin::NdhsCPluginMenu* menu,
														int menuItemIndex,
														IFXI_FIELD_SCOPE scope,
														bool isInput,
														bool isOutput,
														LcTScalar defaultValue)
								: 	NdhsCField(menuCon, fieldName, pluginField, menu, menuItemIndex, IFXI_FIELDDATA_FLOAT, scope, isInput, isOutput, ""),
									m_animRangeDefined(false),
									m_currentValue(defaultValue),
									m_currentStringValueBase(-1.0f)
									{ }

	void						construct();

	// Input fields
	void						valueUpdated(bool finalUpdate);

	// Output fields
	virtual	LcTaString			getFieldData(NdhsCElement* element, bool forceBroadcast);
	virtual bool				isAnimating()					{ return m_animating; }
	virtual void				setAnimating(bool anim)			{ m_animating = anim; ; if (!anim) m_animRangeDefined = false; }

public:
	static LcTaOwner<NdhsCFieldFloat> create(		NdhsCMenuCon* menuCon,
													const LcTmString& fieldName,
													bool pluginField,
													NdhsCPlugin::NdhsCPluginMenu* menu,
													int menuItemIndex,
													IFXI_FIELD_SCOPE scope,
													bool isInput,
													bool isOutput,
													LcTScalar defaultValue);

#ifdef IFX_SERIALIZATION
	static NdhsCFieldFloat* loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual		SerializeHandle	serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual		void	deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */

	/* Data access */
	virtual LcTaString			getFieldData(NdhsCElement* element);
	virtual LcTScalar			getRawFieldData(NdhsCElement* element);
	virtual LcTScalar			getNormalizedValue(NdhsCElement* element, LcTScalar minValue, LcTScalar maxValue)
																			{ return maxValue <= minValue ? minValue : (getRawFieldData(element) - minValue) / (maxValue - minValue); }
	virtual LcTScalar			getTargetValue()							{ return m_targetValue; }

	virtual void				setVelocity(LcTScalar velocity);
	virtual void				setVelocity(LcTScalar velocity, LcTScalar minValue, LcTScalar maxValue);
	virtual bool				setTargetValue(LcTScalar target, unsigned int duration);
	virtual bool				setTargetValue(LcTScalar target, LcTScalar minValue, LcTScalar maxValue, unsigned int duration);
	virtual bool				updateValue(LcTTime timestamp, bool& finalUpdate);
	virtual void				setValue(LcTScalar newValue, bool finalUpdate);
	virtual void				setValue(LcTmString newValue, bool finalUpdate)
																			{ NdhsCField::setValue(newValue, finalUpdate); }
	virtual void				takeSnapshot();
	virtual void				addToSnapshot(LcTScalar dValue, LcTScalar minValue, LcTScalar maxValue, bool finalUpdate);

	virtual void				setValueFromExpression(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item);

	virtual						~NdhsCFieldFloat()	{}
};


#endif /* NdhsCFieldFloatH */
