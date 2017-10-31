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
#ifndef NdhsCScrollPosFieldH
#define NdhsCScrollPosFieldH

#include "inflexionui/engine/inc/NdhsCField.h"

enum ENdhsScrollFieldMode
{
	ENdhsScrollFieldModeNormal,
	ENdhsScrollFieldModeFlick
};

class NdhsCScrollPosField : public NdhsCField
{
private:
	// These are all wrapped to range
	LcTDScalar					m_minValue;
	LcTDScalar 					m_maxValue;
	LcTDScalar					m_minAllowableValue;
	LcTDScalar 					m_maxAllowableValue;
	LcTDScalar					m_origMinValue;
	LcTDScalar 					m_origMaxValue;
	LcTDScalar 					m_currentValue;

	// These are all unwrapped
	LcTDScalar 					m_snapshotValue;
	int							m_lastSettledValue;
	LcTDScalar					m_originalValue;
	LcTDScalar					m_targetValue; // NB - we don't use the NdhsCField version...

	bool 						m_wrap;
	int							m_normalLoop;
	int							m_itemsShown;
	int							m_scrollSpan;
	int							m_wrapCount;
	int							m_wrapStride;
	bool						m_dragValueUpdate;

	ENdhsScrollFieldMode		m_mode;

	// Normal mode state variables
	int							m_duration;
	LcTTime						m_originalTimestamp;
	LcTDScalar					m_distance;
	ENdhsVelocityProfile		m_velocityProfile;

	LcTScalar					m_reboundValue;

protected:
								NdhsCScrollPosField(){}
								NdhsCScrollPosField(NdhsCMenuCon* menuCon,
													NdhsCPlugin::NdhsCPluginMenu* menu,
													LcTScalar minVal,
													LcTScalar maxVal,
													bool wrap,
													LcTScalar rebound,
													int span);

	void						valueUpdated();
	void						construct();
	int 						wrapToRange(LcTDScalar& unwrappedVal);
	LcTScalar 					calculateDistanceFraction(LcTUnitScalar normalizedTime);
	void 						setMode(ENdhsScrollFieldMode mode, ENdhsVelocityProfile velocityProfile);
	LcTScalar					getInitialVelocity(LcTDScalar distance, LcTScalar duration, ENdhsVelocityProfile velocityProfile);
	int 						nearestAllowableInt(LcTDScalar val);

public:
#ifdef IFX_SERIALIZATION
	static NdhsCScrollPosField* loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual		SerializeHandle	serialize(LcCSerializeMaster *serializeMaster, bool force=false);
	virtual		void			deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */

	static LcTaOwner<NdhsCScrollPosField> create(NdhsCMenuCon* menuCon,
													NdhsCPlugin::NdhsCPluginMenu* menu,
													LcTScalar minVal,
													LcTScalar maxVal,
													bool wrap,
													LcTScalar rebound,
													int span = 1);

	/* Data access */
	virtual LcTaString			getFieldData(NdhsCElement* element)		{ LC_UNUSED(element); return LcTaString().fromScalar((LcTScalar)m_currentValue); }
	virtual	LcTaString			getFieldData(NdhsCElement* element, bool forceBroadcast)
																		{ LC_UNUSED(element); LC_UNUSED(forceBroadcast); return LcTaString().fromScalar((LcTScalar)m_currentValue); }
	virtual LcTScalar			getRawFieldData(NdhsCElement* element);
	virtual LcTScalar			getCurrentAllowableValue();
	virtual LcTDScalar			getMinValue()							{ return m_minValue; }
	virtual LcTDScalar			getMaxValue()							{ return m_maxValue; }
	virtual LcTScalar			getNormalizedValue(NdhsCElement* element);
	virtual LcTScalar			getNormalizedValue(NdhsCElement* element, LcTScalar minValue, LcTScalar maxValue) { return getNormalizedValue(element); }
	virtual LcTScalar			getTargetValue();
	virtual bool				isScrollPosField()						{ return true; }
	virtual bool				isWrapping()							{ return m_wrap; }
	virtual LcTScalar			getSensitivityFactor()					{ return (LcTScalar)m_scrollSpan; }

	virtual bool				setTargetValue(LcTScalar target, unsigned int duration);
	virtual bool				setTargetValue(LcTScalar target, LcTScalar minValue, LcTScalar maxValue, unsigned int duration)
																		{ LC_UNUSED(minValue); LC_UNUSED(maxValue); return setTargetValue(target, duration); }
			bool	 			setTargetValue(LcTScalar target, ENdhsFieldDirection dir, bool wrap, unsigned int duration,
															ENdhsScrollFieldMode mode, ENdhsVelocityProfile velocityProfile);
	virtual bool				updateValue(LcTTime timestamp, bool& finalUpdate);
	virtual void				setValue(LcTScalar newValue, bool finalUpdate);
	virtual void				setValue(LcTmString newValue, bool finalUpdate)
																			{ NdhsCField::setValue(newValue, finalUpdate); }
			void				setNormalizedValue(LcTScalar newValue, bool finalUpdate);
	virtual void				takeSnapshot();
	virtual void				addToSnapshot(LcTScalar dValue, LcTScalar minValue, LcTScalar maxValue, bool finalUpdate)
																			{ addToSnapshot((LcTDScalar)dValue, (LcTDScalar)minValue, (LcTDScalar)maxValue, finalUpdate); }
			void				addToSnapshot(LcTDScalar dValue, LcTDScalar minValue, LcTDScalar maxValue, bool finalUpdate);
	virtual void				setVelocity(LcTScalar velocity);
	virtual void				setVelocity(LcTScalar velocity, LcTScalar minValue, LcTScalar maxValue);
	virtual void				setDeceleration(LcTScalar deceleration);
	virtual void				setMaxSpeed(LcTScalar maxSpeed);
			void				setMaxAllowableVal(LcTScalar max)		{ m_maxAllowableValue = max; if (!m_wrap) m_maxValue = max; }
			void				setMinAllowableVal(LcTScalar min)		{ m_minAllowableValue = min; if (!m_wrap) m_minValue = min; }
			LcTDScalar 			getMaxAllowableVal()					{ return m_maxAllowableValue; }
			LcTDScalar 			getMinAllowableVal()					{ return m_minAllowableValue; }
			LcTDScalar			getWrapAdjustedVal() const				{ return m_currentValue +  m_wrapCount * m_wrapStride; }
			LcTDScalar			getWrappedVal() const					{ return m_currentValue; }
			void 				updateBounds(LcTScalar min, LcTScalar max, bool wrap, LcTScalar rebound, int span);

			void				setLoop(int loop)						{ m_normalLoop = loop; }

			void				setAbsolute();
			void				setRelative();
			bool				isAnimating()							{ return !isSmallValue(getWrapAdjustedVal() - m_targetValue); }
	virtual void				setDragValueUpdate(bool val)			{ m_dragValueUpdate = val ; }
			bool				isDragValueUpdate()						{ return m_dragValueUpdate; }

			void				jumpValueTo(int value);

	virtual						~NdhsCScrollPosField()	{}
};


#endif // NdhsCScrollPosFieldH
