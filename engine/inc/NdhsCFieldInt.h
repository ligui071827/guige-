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
#ifndef NdhsCFieldIntH
#define NdhsCFieldIntH

#include "inflexionui/engine/inc/NdhsCField.h"

class NdhsCFieldInt : public NdhsCField
{
private:
	int							m_animMinValue;
	int							m_animMaxValue;
	bool						m_animRangeDefined;

	int 						m_currentValue;
	LcTScalar					m_currentValueOffset; // -1 to 1, add to m_currentValue for real value

	int 						m_snapshotValue;
	LcTScalar					m_snapshotValueOffset; // -1 to 1, add to m_snapshotValue for real value

	// Output fields
	int							m_currentStringValueBase;

	bool						m_animating;

protected:
								NdhsCFieldInt(){}
								NdhsCFieldInt(		NdhsCMenuCon* menuCon,
													const LcTmString& fieldName,
													bool pluginField,
													NdhsCPlugin::NdhsCPluginMenu* menu,
													int menuItemIndex,
													IFXI_FIELD_SCOPE scope,
													bool isInput,
													bool isOutput,
													int defaultValue)
								: 	NdhsCField(menuCon, fieldName, pluginField, menu, menuItemIndex, IFXI_FIELDDATA_INT, scope, isInput, isOutput, ""),
									m_animRangeDefined(false),
									m_currentValue(defaultValue),
									m_currentStringValueBase(-1)
									{ }

	void						construct();
	void						valueUpdated(bool finalUpdate);

	// Output fields
	int							getRawFieldDataInt(NdhsCElement* element);
	virtual	LcTaString			getFieldData(NdhsCElement* element, bool forceBroadcast);

	virtual bool				isAnimating()					{ return m_animating; }
	virtual void				setAnimating(bool anim)			{ m_animating = anim; if (!anim) m_animRangeDefined = false; }

public:
#ifdef IFX_SERIALIZATION
	static	NdhsCFieldInt*		loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual	SerializeHandle		serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual	void				deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */

	static LcTaOwner<NdhsCFieldInt> create(		NdhsCMenuCon* menuCon,
												const LcTmString& fieldName,
												bool pluginField,
												NdhsCPlugin::NdhsCPluginMenu* menu,
												int menuItemIndex,
												IFXI_FIELD_SCOPE scope,
												bool isInput,
												bool isOutput,
												int defaultValue);

	/* Data access */
	virtual LcTaString			getFieldData(NdhsCElement* element);
	virtual LcTScalar			getRawFieldData(NdhsCElement* element);
	virtual LcTScalar			getNormalizedValue(NdhsCElement* element, LcTScalar minValue, LcTScalar maxValue)
																			{ return maxValue <=  minValue ? minValue : (LcTScalar)(getRawFieldDataInt(element) - minValue) / (LcTScalar)(maxValue - minValue); }
	virtual LcTScalar			getTargetValue()							{ return (LcTScalar)((int)(m_targetValue + 2 * OVERSHOOT_MARGIN)); }

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

	virtual						~NdhsCFieldInt()	{}
};


#endif /* NdhsCFieldIntH */
