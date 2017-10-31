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
#ifndef NdhsCFieldTimeH
#define NdhsCFieldTimeH

#include "inflexionui/engine/inc/NdhsCField.h"


enum ENdhsFieldTimeFormat
{
	ENdhsFieldTimeFormatDefault,
	ENdhsFieldTimeFormatLongDate,
	ENdhsFieldTimeFormatShortDate,
	ENdhsFieldTimeFormatShortTime,
	ENdhsFieldTimeFormatLongTime
};

class NdhsCFieldTime : public NdhsCField
{
private:
	IFX_TIME					m_animMinValue;
	IFX_TIME					m_animMaxValue;
	bool						m_animRangeDefined;

	IFX_TIME					m_currentValue;
	LcTScalar					m_currentValueOffset; // -1 to 1, add to m_currentValue for real value
	struct tm					m_currentTimeUTC;
	bool						m_timeUTCUpdated;

	IFX_TIME					m_snapshotValue;
	LcTScalar					m_snapshotValueOffset; // -1 to 1, add to m_snapshotValue for real value

	// Output fields
	IFX_TIME					m_currentStringValueBase;

	bool						m_animating;

protected:
								NdhsCFieldTime(){}
								NdhsCFieldTime(		NdhsCMenuCon* menuCon,
													const LcTmString& fieldName,
													bool pluginField,
													NdhsCPlugin::NdhsCPluginMenu* menu,
													int menuItemIndex,
													IFXI_FIELD_SCOPE scope,
													bool isInput,
													bool isOutput,
													int defaultValue)
								: 	NdhsCField(menuCon, fieldName, pluginField, menu, menuItemIndex, IFXI_FIELDDATA_TIME, scope, isInput, isOutput, ""),
									m_animRangeDefined(false),
									m_currentValue(defaultValue),
									m_currentStringValueBase(-1)
									{ }

	void						construct();
	void						valueUpdated(bool finalUpdate);

	// Output fields
	IFX_TIME					getRawFieldDataTime(NdhsCElement* element);
	virtual	LcTaString			getFieldData(NdhsCElement* element, bool forceBroadcast);
			LcTaString 			getFieldData(ENdhsFieldTimeFormat format);

	virtual bool				isAnimating()					{ return m_animating; }
	virtual void				setAnimating(bool anim)			{ m_animating = anim; if (!anim) m_animRangeDefined = false; }

public:
#ifdef IFX_SERIALIZATION
	static	NdhsCFieldTime*		loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual	SerializeHandle		serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual	void				deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */

	static LcTaOwner<NdhsCFieldTime> create(		NdhsCMenuCon* menuCon,
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
																			{ return maxValue <=  minValue ? minValue : (LcTScalar)(getRawFieldDataTime(element) - minValue) / (LcTScalar)(maxValue - minValue); }
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

	virtual						~NdhsCFieldTime()	{}
};


#endif /* NdhsCFieldTimeH */
