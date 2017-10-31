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


#include <math.h> // For sqrt() in setTargetValue

#define NEAREST_INT(val) (LC_FLOOR(val + 0.5))

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCScrollPosField> NdhsCScrollPosField::create(NdhsCMenuCon* menuCon,
															NdhsCPlugin::NdhsCPluginMenu* menu,
															LcTScalar minVal,
															LcTScalar maxVal,
															bool wrap,
															LcTScalar rebound,
															int span)
{
	LcTaOwner<NdhsCScrollPosField> ref;
	ref.set(new NdhsCScrollPosField(menuCon, menu, minVal, maxVal, wrap, rebound, span));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCScrollPosField::NdhsCScrollPosField(NdhsCMenuCon* menuCon,
											NdhsCPlugin::NdhsCPluginMenu* menu,
											LcTScalar minVal,
											LcTScalar maxVal,
											bool wrap,
											LcTScalar rebound,
											int span)
	: NdhsCField(menuCon, "", false, menu, -1, IFXI_FIELDDATA_FLOAT, IFXI_FIELD_SCOPE_MENU, true, false, "")
{
	updateBounds(minVal,maxVal,wrap,rebound,span);
	m_velocityProfile = ENdhsVelocityProfileLinear;
	m_dragValueUpdate=false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::updateBounds(LcTScalar min,
											LcTScalar max,
											bool wrap,
											LcTScalar rebound,
											int span)
{
	LcTDScalar localMax = (LcTDScalar) max;
	// For wrapping menus, we want to allow the scrollpos field to vary from max to
	// min (and vice versa). We do this by increasing the 'max' value by a little
	// less than 1, so the field varies from [min:max + 1)
	if (wrap)
	{
		localMax = (localMax + 1) - FIELD_ALMOST_ZERO;
	}

	m_origMinValue = m_minAllowableValue = m_minValue = (LcTDScalar)min;
	m_origMaxValue = m_maxAllowableValue = m_maxValue = localMax;
	m_wrap = wrap;
	m_scrollSpan = span;
	m_reboundValue = rebound;
	m_dragValueUpdate = false;
	m_wrapCount = 0;
	m_wrapStride = (int)(m_maxValue - m_minValue) + 1;
}


#ifdef IFX_SERIALIZATION
NdhsCScrollPosField* NdhsCScrollPosField::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCScrollPosField*obj=new NdhsCScrollPosField();
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCScrollPosField::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCScrollPosField) - sizeof(NdhsCField)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCField::serialize(serializeMaster,true);
	ENdhsCFieldType typ = ENdhsCFieldTypeScrollPos;
	SERIALIZE(typ,serializeMaster,cPtr)
	SERIALIZE(parentHandle,serializeMaster,cPtr)
	SERIALIZE(m_snapshotValue,serializeMaster,cPtr)
	SERIALIZE(m_currentValue,serializeMaster,cPtr)
	SERIALIZE(m_mode,serializeMaster,cPtr)
	SERIALIZE(m_lastSettledValue,serializeMaster,cPtr)
	SERIALIZE(m_scrollSpan,serializeMaster,cPtr)
	SERIALIZE(m_itemsShown,serializeMaster,cPtr)
	SERIALIZE(m_normalLoop,serializeMaster,cPtr)
	SERIALIZE(m_wrap,serializeMaster,cPtr)
	SERIALIZE(m_origMaxValue,serializeMaster,cPtr)
	SERIALIZE(m_origMinValue,serializeMaster,cPtr)
	SERIALIZE(m_maxAllowableValue,serializeMaster,cPtr)
	SERIALIZE(m_minAllowableValue,serializeMaster,cPtr)
	SERIALIZE(m_maxValue,serializeMaster,cPtr)
	SERIALIZE(m_minValue,serializeMaster,cPtr)
	SERIALIZE(m_duration,serializeMaster,cPtr)
	SERIALIZE(m_originalValue,serializeMaster,cPtr)
	SERIALIZE(m_targetValue,serializeMaster,cPtr)
	SERIALIZE(m_originalTimestamp,serializeMaster,cPtr)
	SERIALIZE(m_distance,serializeMaster,cPtr)
	SERIALIZE(m_velocityProfile,serializeMaster,cPtr)
	SERIALIZE(m_reboundValue,serializeMaster,cPtr)
	SERIALIZE(m_wrapCount,serializeMaster,cPtr)
	SERIALIZE(m_wrapStride,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCScrollPosField::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummySize=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;
	m_dragValueUpdate=false;
	DESERIALIZE(dummySize,serializeMaster,cPtr)
	DESERIALIZE(parentHandle,serializeMaster,cPtr)
	NdhsCField::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE(m_snapshotValue,serializeMaster,cPtr)
	DESERIALIZE(m_currentValue,serializeMaster,cPtr)
	DESERIALIZE(m_mode,serializeMaster,cPtr)
	DESERIALIZE(m_lastSettledValue,serializeMaster,cPtr)
	DESERIALIZE(m_scrollSpan,serializeMaster,cPtr)
	DESERIALIZE(m_itemsShown,serializeMaster,cPtr)
	DESERIALIZE(m_normalLoop,serializeMaster,cPtr)
	DESERIALIZE(m_wrap,serializeMaster,cPtr)
	DESERIALIZE(m_origMaxValue,serializeMaster,cPtr)
	DESERIALIZE(m_origMinValue,serializeMaster,cPtr)
	DESERIALIZE(m_maxAllowableValue,serializeMaster,cPtr)
	DESERIALIZE(m_minAllowableValue,serializeMaster,cPtr)
	DESERIALIZE(m_maxValue,serializeMaster,cPtr)
	DESERIALIZE(m_minValue,serializeMaster,cPtr)
	DESERIALIZE(m_duration,serializeMaster,cPtr)
	DESERIALIZE(m_originalValue,serializeMaster,cPtr)
	DESERIALIZE(m_targetValue,serializeMaster,cPtr)
	DESERIALIZE(m_originalTimestamp,serializeMaster,cPtr)
	DESERIALIZE(m_distance,serializeMaster,cPtr)
	DESERIALIZE(m_velocityProfile,serializeMaster,cPtr)
	DESERIALIZE(m_reboundValue,serializeMaster,cPtr)
	DESERIALIZE(m_wrapCount,serializeMaster,cPtr)
	DESERIALIZE(m_wrapStride,serializeMaster,cPtr)
}
#endif /* IFX_SERIALIZATION */

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::construct()
{
}

/*-------------------------------------------------------------------------*//**
	Note that in the wrapping case, we assume that val is unwrapped, thus
	we don't need to adjust by wrapCount,wrapStride before comparing to
	m_lastSettledValue (which is also unwrapped)
*/
int NdhsCScrollPosField::nearestAllowableInt(LcTDScalar val)
{
	int retVal;

	// find the valid int value
	if (m_wrap)
		retVal = (int)NEAREST_INT(val);
	else
		retVal = (int)(min(max((LcTDScalar)NEAREST_INT(val), m_minAllowableValue), m_maxAllowableValue));

	// If the scroll span is one, we're done.
	if (m_scrollSpan == 1)
		return retVal;

	// Otherwise, we've a bunch more work to do to identify the integer value for scrollpos
	// that will leave the items in their slot, not between slots
	LcTDScalar scalarVal = val;

	// for non-wrapping menus, cap the value
	if (!m_wrap)
		scalarVal = (min(max(val, m_minAllowableValue), m_maxAllowableValue));

	scalarVal = isSmallValue((LcTScalar)scalarVal)? 0 : scalarVal;

	// Ok - we know the 'last settled value' was 'safe'...work out how far we
	// are away from that
	LcTDScalar diff = scalarVal - m_lastSettledValue;

	// Adjust diff so that it is in the range 0 - scrollSpan
	diff = diff - ((int)(diff / m_scrollSpan)) * m_scrollSpan;

	if (diff < 0)
		diff = m_scrollSpan + diff;

	diff = isSmallValue((LcTScalar)diff)? 0 : diff;

	if (diff != 0)
	{
		bool forceGoForward = false;
		bool forceGoBack = false;

		if (!m_wrap)
		{
			// Still have to be careful about staying within the legal bounds
			// for non-wrapping menus...with rebound it's possible to go out of
			// the legal range temporarily, but we mustn't settle there
			forceGoForward = (scalarVal - diff) < m_minAllowableValue;
			forceGoBack = (scalarVal + m_scrollSpan - diff) > m_maxAllowableValue;
		}

		// Finally, we can select our safe value - note that the calculations
		// will be near integer values, so we add on .5 before taking the
		// integer parts to guard against rounding errors giving us
		// x.99999999, and retVal being one smaller than it should be
		LcTScalar modifier = scalarVal > 0 ? 0.5f : -0.5f;
		if ((diff <= m_scrollSpan/2.0 && !forceGoForward) || forceGoBack)
		{
			// Go to a lower safe value
			retVal = (int)(scalarVal - diff + modifier);
		}
		else
		{
			// Go to the higher safe value
			retVal = (int)(scalarVal  + m_scrollSpan - diff + modifier);
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTScalar NdhsCScrollPosField::getRawFieldData(NdhsCElement* element)
{
	LC_UNUSED(element); // Ignore any element context on input fields
	return (LcTScalar)m_currentValue;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::valueUpdated()
{
	broadcastFieldValueUpdate();
}

/*-------------------------------------------------------------------------*//**
	Special version of setTargetValue used by PageModel, Components and PageManager
	Note that we expect 'target' to be wrapped to the valid range on the way
	in, although we'll store it 'unwrapped' in m_targetValue
*/
bool NdhsCScrollPosField::setTargetValue(LcTScalar target,
											ENdhsFieldDirection dir,
											bool wrap,
											unsigned int duration,
											ENdhsScrollFieldMode mode,
											ENdhsVelocityProfile velocityProfile)
{
	bool retVal = false;
	LcTDScalar localTarget = (LcTScalar) target;

	if (!isUserLocked())
	{
		// Set mode to default
		setMode(mode, velocityProfile);

		if (!m_wrap &&
			  (    (ENdhsFieldDirectionIncreasing == dir && localTarget < m_currentValue)
				|| (ENdhsFieldDirectionDecreasing == dir && localTarget > m_currentValue)))
		{
			// Can't reach target
			return false;
		}

		// Check limits
		if (localTarget > m_maxValue)
			localTarget = m_maxValue;
		if (localTarget < m_minValue)
			localTarget = m_minValue;


		// Forget about previous updates
		m_lastUpdatedValid = false;

		// Calc distance & direction needed
		LcTDScalar distance = 0;
		bool bTargetOK = false;

		while (bTargetOK == false)
		{
			// Will only take one pass unless we're kick scrolling with a max velocity in
			// place
			bTargetOK = true;

			if (m_wrap && dir == ENdhsFieldDirectionIncreasing && localTarget < m_currentValue)
			{
				// Wraparound case
				distance = localTarget - m_currentValue + m_wrapStride;
			}
			else if (m_wrap && dir == ENdhsFieldDirectionDecreasing && localTarget > m_currentValue)
			{
				// Wraparound case
				distance = localTarget - m_currentValue - m_wrapStride;
			}
			else
			{
				// General case
				distance = localTarget - m_currentValue;
			}

			if ((m_mode == ENdhsScrollFieldModeNormal) && (isMaxSpeedInfinite() == false))
			{
				// For kick scrolls, we need to obey any max speed applied.  This is done by adjusting the
				// target by whole integers until the starting velocity is below the threshold
				if ( lc_fabs(distance) > .0005 * m_currentMaxSpeed * duration )
				{
					// if distance is less than 1, we can't reach the target at all
					// and obey the max velocity rule.
					if (lc_fabs(distance) < 1.0 + FIELD_ALMOST_ZERO)
						return false;

					localTarget += (dir == ENdhsFieldDirectionIncreasing) ? -1 : 1;
					bTargetOK = false;
				}
			}
		}

		m_targetValue = getWrapAdjustedVal() + distance;

		if (m_mode == ENdhsScrollFieldModeFlick)
		{
			if (isDecelerationInfinite())
			{
				// Temporarily unwrapped...
				m_currentValue = m_targetValue;
				m_wrapCount = 0;

				m_wrapCount = wrapToRange(m_currentValue);
				m_duration = 0;
			}
			else
			{
				if (distance < -FIELD_ALMOST_ZERO)
				{
					setVelocity((LcTScalar)(-sqrt(2.0 * -distance * m_currentDeceleration)));
				}
				else if (distance > FIELD_ALMOST_ZERO)
				{
					setVelocity((LcTScalar)sqrt(2.0 * distance * m_currentDeceleration));
				}
			}
		}
		else
		{
			// Record orig value, target value, duration and velocity profile for normal mode scrolling
			// given a start time, current time, duration we can work out the current value
			// using the appropriate value-time equation matching the chosen velocity profile
			setDeceleration(0);

			m_duration = duration;
			m_originalValue = getWrapAdjustedVal();
			m_distance = distance;
			m_velocityProfile = velocityProfile;

			// Don't use scrollpos version, otherwise it'll change the mode to flick scroll
			NdhsCField::setVelocity(getInitialVelocity(m_distance, (LcTScalar)m_duration, m_velocityProfile));
		}

		retVal = true;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	setTargetValue for drag regions, etc.  Always changes mode to 'flick'
*/
bool NdhsCScrollPosField::setTargetValue(LcTScalar target,
											unsigned int duration)
{
	return setTargetValue(target, ENdhsFieldDirectionAny, false, duration, ENdhsScrollFieldModeFlick, ENdhsVelocityProfileUnknown);
}

/*-------------------------------------------------------------------------*//**
	Changes scrollpos mode from flick to normal or vice-versa
*/
void NdhsCScrollPosField::setMode(ENdhsScrollFieldMode mode, ENdhsVelocityProfile velocityProfile)
{
	if (m_mode != mode)
	{
		m_mode = mode;
		m_velocityProfile = velocityProfile;

		m_duration = 0;
		m_distance = 0;
		NdhsCField::setVelocity(0);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCScrollPosField::updateValue(LcTTime timestamp, bool& finalUpdate)
{
	finalUpdate = false;
	bool notify = false;

	// First, check for scrolls that have finished
	if (m_mode == ENdhsScrollFieldModeFlick)
	{
		if (isDecelerationInfinite() || atRest()
			|| (!m_wrap && m_currentValue >= m_maxValue && m_currentVelocity > 0)
			|| (!m_wrap && m_currentValue <= m_minValue && m_currentVelocity < 0))
		{
			updateTimestamp(timestamp);
			finalUpdate = true;
			return false;
		}
	}
	else if (m_duration == 0)
	{
		updateTimestamp(timestamp);
		finalUpdate = true;

		if (getWrapAdjustedVal() != m_targetValue)
		{
			// m_currentValue temporarily unwrapped
			m_currentValue = m_targetValue;
			m_wrapCount = 0;

			m_wrapCount = wrapToRange(m_currentValue);
			setVelocity(0);

			// Notify watchers
			valueUpdated();
		}

		m_velocityProfile = ENdhsVelocityProfileLinear;

		return true;
	}

	if (!m_lastUpdatedValid)
	{
		// Take initial timestamp
		updateTimestamp(timestamp);
		m_originalTimestamp = timestamp;
		return true;
	}

	// Update the current value
	if (m_mode == ENdhsScrollFieldModeFlick)
	{
		unsigned int dTime = updateTimestamp(timestamp);
		LcTScalar oldVelocity = m_currentVelocity;

		// unwrap m_currentValue temporarily
		m_currentValue += m_wrapCount * m_wrapStride;
		m_wrapCount = 0;

		// Give the deceleration the correct direction
		m_currentValue += calcChangeInValue(dTime);
		notify = true;

		// Re-wrap m_currentValue
		m_wrapCount = wrapToRange(m_currentValue);

		// If velocity changes sign, reset to 0
		if ((oldVelocity > 0 && m_currentVelocity < 0) || (oldVelocity < 0 && m_currentVelocity > 0))
		{
			setVelocity(0);
		}

		if (atRest())
		{
			// Unwrap m_currentValue
			m_currentValue += m_wrapCount * m_wrapStride;
			m_wrapCount = 0;

			int nearestInt = nearestAllowableInt(m_currentValue);
			LcTDScalar distanceFromInteger = m_currentValue - (LcTDScalar)(nearestInt);

			if (distanceFromInteger < 0.01 && distanceFromInteger > -0.01)
			{
				m_currentValue = (LcTDScalar)nearestInt;
				m_lastSettledValue = nearestInt;

				finalUpdate = true;
			}
			else
			{
				// wrap nearestInt before sending into setTargetValue...
				LcTDScalar nearestIntScalar = (LcTDScalar)nearestInt;
				(void)wrapToRange(nearestIntScalar);
				nearestInt = (int)nearestIntScalar;

				setTargetValue((LcTScalar)nearestInt, ENdhsFieldDirectionAny, false, 0, m_mode, m_velocityProfile);
			}

			// wrap m_currentValue
			m_wrapCount = wrapToRange(m_currentValue);
		}
	}
	else
	{
		// Normal Mode
		// Work out normalized time point
		unsigned int diff = timestamp - m_originalTimestamp;
		LcTScalar timepoint = (LcTScalar)(diff) / (LcTScalar)m_duration;
		unsigned int dTime = updateTimestamp(timestamp);
		LcTDScalar newValue;

		if (timepoint < 1)
		{
			// Calculate current (unwrapped) value
			newValue = m_originalValue + m_distance * calculateDistanceFraction(timepoint);

			// Calculate the velocity
			if (dTime)
				NdhsCField::setVelocity((LcTScalar)((newValue - m_currentValue) / dTime));
		}
		else
		{
			newValue = m_targetValue;
			// Reset the clock
			m_originalTimestamp = timestamp;

			NdhsCField::setVelocity(getInitialVelocity(m_distance, (LcTScalar)m_duration, m_velocityProfile));

			if (m_normalLoop > 0)
				m_normalLoop--;

			if (m_normalLoop == 0)
			{
				// Job done.
				finalUpdate = true;

				m_velocityProfile = ENdhsVelocityProfileLinear;

				if (getWrapAdjustedVal() != newValue)
					notify = true;

				// Note this sets current value to newValue implicitly!
				setVelocity(0);
			}
		}

		if (getWrapAdjustedVal() != newValue)
		{
			// m_currentValue is now unwrapped
			m_currentValue = newValue;
			m_wrapCount = 0;

			// Ensure m_currentValue is within valid range.
			m_wrapCount = wrapToRange(m_currentValue);

			notify = true;
		}

	}

	// Notify watchers
	if (notify)
		valueUpdated();

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::setValue(LcTScalar newValue, bool finalUpdate)
{
	if ((newValue != m_currentValue) || finalUpdate)
	{
		m_lastUpdatedValid = false;
		m_currentValue = newValue;
		m_wrapCount = wrapToRange(m_currentValue);
		setVelocity(0);
		ENdhsFieldDirection dir = ENdhsFieldDirectionAny;

		if (finalUpdate)
		{
			LcTScalar target = (LcTScalar)nearestAllowableInt(newValue);
			dir = (target < (LcTScalar)m_currentValue) ? ENdhsFieldDirectionDecreasing : ENdhsFieldDirectionIncreasing;
			setTargetValue(target, dir, false, 0, m_mode, m_velocityProfile);
		}
		else
			setTargetValue(newValue, ENdhsFieldDirectionAny, false, 0, m_mode, m_velocityProfile);

		valueUpdated();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::setNormalizedValue(LcTScalar normValue, bool finalUpdate)
{
	LcTDScalar newVal = normValue * (m_maxValue - m_minValue + 1) + m_minValue;
	newVal = min(max(newVal, m_minValue), m_maxValue);

	if ((newVal != m_currentValue) || finalUpdate)
	{
		m_lastUpdatedValid = false;
		setVelocity(0);
		m_currentValue = newVal;
		ENdhsFieldDirection dir = ENdhsFieldDirectionAny;

		if (finalUpdate)
		{
			LcTScalar target = (LcTScalar)nearestAllowableInt(m_currentValue);
			dir = (target < (LcTScalar)m_currentValue) ? ENdhsFieldDirectionDecreasing : ENdhsFieldDirectionIncreasing;
			setTargetValue(target, dir, false, 0, ENdhsScrollFieldModeFlick, m_velocityProfile);
		}

		valueUpdated();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::jumpValueTo(int value)
{
	if ((LcTDScalar)value != getWrapAdjustedVal())
	{
		m_lastUpdatedValid = false;
		m_currentValue = (LcTDScalar)value;
		m_wrapCount = 0;
		m_targetValue = m_currentValue;
		m_lastSettledValue = value;
		setVelocity(0);

		valueUpdated();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::takeSnapshot()
{
	m_snapshotValue = getWrapAdjustedVal();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::addToSnapshot(LcTDScalar dValue, LcTDScalar minValue, LcTDScalar maxValue, bool finalUpdate)
{
	LcTDScalar oldVal = m_currentValue;

	if (dValue != 0)
	{
		// m_currentValue is currently unwrapped
		m_currentValue = m_snapshotValue + dValue;
		m_wrapCount = 0;

		if (!m_wrap)
		{
			m_currentValue = min(max(m_currentValue, m_minValue), m_maxValue);
		}
		else
		{
			m_wrapCount = wrapToRange(m_currentValue);
		}

		// m_currentValue wrapped again
		if (m_currentValue != oldVal)
		{
			valueUpdated();
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::setVelocity(LcTScalar velocity)
{
	if (!isUserLocked())
	{
		bool broadcastUpdate = false;

		// We want to be in flick scrolling mode if we're using a velocity
		setMode(ENdhsScrollFieldModeFlick, ENdhsVelocityProfileUnknown);

		if (!isDecelerationInfinite())
		{
			LcTScalar newVelocity;

			m_lastUpdatedValid = false;

			if (!isMaxSpeedInfinite())
			{
				newVelocity = min(max(velocity, -m_currentMaxSpeed), m_currentMaxSpeed);
			}
			else
			{
				newVelocity = velocity;
			}

			if (m_currentDeceleration > 0)
			{
				LcTScalar directionalDeceleration;

				// Give the deceleration the correct direction
				if (newVelocity < 0)
				{
					directionalDeceleration = -m_currentDeceleration;
				}
				else
				{
					directionalDeceleration = m_currentDeceleration;
				}

				LcTScalar timeToStop = newVelocity / directionalDeceleration;
				LcTDScalar projectedEndPosition = getWrapAdjustedVal() + newVelocity * timeToStop - (LcTDScalar)(directionalDeceleration * timeToStop * timeToStop) / 2.0f;
				int targetEndPosition = 0;

#ifdef IFX_GENERATE_SCRIPTS
				if (NdhsCScriptGenerator::getInstance())
					NdhsCScriptGenerator::getInstance()->setScrollTime(timeToStop, true);
#endif

				targetEndPosition = nearestAllowableInt(projectedEndPosition);

				LcTDScalar distance = (LcTDScalar)targetEndPosition - getWrapAdjustedVal();
				m_targetValue = (LcTDScalar)targetEndPosition;

				// NB Don't use NdhsCInputField::setVelocity here, as that limits to the max velocity
				// Sometimes we need to go everso slightly over the max velocity to reach the target
				if (fabs(distance) < FIELD_ALMOST_ZERO)
				{
					m_currentVelocity = 0;
				}
				else if (distance < 0)
				{
					m_currentVelocity = (LcTScalar)(-sqrt(2.0 * -distance * (LcTDScalar)m_currentDeceleration));
				}
				else
				{
					m_currentVelocity = (LcTScalar)(sqrt(2.0 * distance * (LcTDScalar)m_currentDeceleration));
				}
			}
			else
			{
				NdhsCField::setVelocity(newVelocity);
			}

			if (isSmallValue(m_currentVelocity))
			{
				// Unwrap m_currentValue temporarily
				m_currentValue += m_wrapCount * m_wrapStride;
				m_wrapCount = 0;

				// Snap to nearest int
				int newVal = nearestAllowableInt(m_currentValue);
				if (!isSmallValue((LcTScalar)((LcTDScalar)newVal - m_currentValue)))
					broadcastUpdate = true;
				m_currentValue = (LcTDScalar)newVal;

				m_lastSettledValue = newVal;

				// Now re-wrap m_currentValue
				m_wrapCount = wrapToRange(m_currentValue);
				NdhsCField::setVelocity(0);
			}
		}
		else
		{
			// Unwrap m_currentValue temporarily
			m_currentValue += m_wrapCount * m_wrapStride;
			m_wrapCount = 0;

			// Snap to nearest int
			int newVal = nearestAllowableInt(m_currentValue);
			if (!isSmallValue((LcTScalar)((LcTDScalar)newVal - m_currentValue)))
				broadcastUpdate = true;
			m_currentValue = (LcTDScalar)newVal;
			m_lastSettledValue = newVal;

			// Now re-wrap m_currentValue
			m_wrapCount = wrapToRange(m_currentValue);
			NdhsCField::setVelocity(0);
		}

		// There may be observers to notify in the case of a 'snap to int' - otherwise
		// observers will be notified as part of the updateValue call.
		if (broadcastUpdate)
			valueUpdated();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::setMaxSpeed(LcTScalar maxSpeed)
{
	// We want max speed to take into account span, so that
	// the speed refers to the traversal of rows
	maxSpeed *= m_scrollSpan;

	NdhsCField::setMaxSpeed(maxSpeed);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::setDeceleration(LcTScalar deceleration)
{
	// We want deceleration to take into account span, so that
	// the speed refers to the traversal of rows
	deceleration /= m_scrollSpan;

	NdhsCField::setDeceleration(deceleration);
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCScrollPosField::wrapToRange(LcTDScalar& unwrappedVal)
{
	int wrapCount = 0;
	LcTDScalar adjustedMax = m_maxValue;
	LcTDScalar adjustedMin = m_minValue;

	if (m_wrap)
	{
		if (unwrappedVal > m_maxValue)
		{
			// Wrap value back into range
			while (unwrappedVal > m_maxValue)
			{
				unwrappedVal -= m_wrapStride;
				++wrapCount;
			}
		}
		else if (unwrappedVal < m_minValue)
		{
			// Wrap value back into range
			while (unwrappedVal < m_minValue)
			{
				unwrappedVal += m_wrapStride;
				--wrapCount;
			}
		}
	}
	else
	{
		switch (m_velocityProfile)
		{
		case ENdhsVelocityProfileBounce:
			adjustedMax += .15f;
			adjustedMin -= .15f;
			break;
		case ENdhsVelocityProfileCatapult:
			adjustedMax += .3f;
			adjustedMin -= .3f;
			break;
		default:
			break;
		}

		if (unwrappedVal > adjustedMax)
		{
			// Clip value to range
			unwrappedVal = adjustedMax;
		}
		else if (unwrappedVal < adjustedMin)
		{
			// Clip value to range
			unwrappedVal = adjustedMin;
		}
	}

	return wrapCount;
}

/*-------------------------------------------------------------------------*//**
	Function for calculating the initial velocity for the given velocity profile.
*/
LcTScalar NdhsCScrollPosField::getInitialVelocity(LcTDScalar distance,
													LcTScalar duration,
													ENdhsVelocityProfile velocityProfile)
{
	LcTScalar retVal = 0;

	switch(velocityProfile)
	{
	case ENdhsVelocityProfileLinear:
	case ENdhsVelocityProfilePauseStartFinish:
		// Linear is always the average velocity
		if (duration > 0)
			retVal = (LcTScalar)(distance / duration);
		break;

	case ENdhsVelocityProfileDecelerate:
		// Decelerate starts off at twice the average velocity
		if (duration > 0)
			retVal = 2.0f * (LcTScalar)(distance / duration);
		break;

	case ENdhsVelocityProfileBounce:
		// Bounce sets off to reach 1.15xdistance in .75xduration decelerating linearly
		if (duration > 0)
			retVal = 3.066666f * (LcTScalar)(distance / duration);
		break;

	case ENdhsVelocityProfileCatapult:
		// Catapult sets off to reach -.3xdistance in .5xduration decelerating linearly
		if (duration > 0)
			retVal = -1.2f * (LcTScalar)(distance / duration);
		break;

	default:
		// All other velocity profiles start out stationary
		retVal = 0;
		break;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Function for calculating the fraction of total distance to travel
	given a normalized time figure in the range [0,1] and a velocity profile.
*/
LcTScalar NdhsCScrollPosField::calculateDistanceFraction(LcTUnitScalar normalizedTime)
{
	// Assumes linear profile
	LcTScalar retVal = normalizedTime;

	/* This code makes a lot of use of quadratic graph curves. The general way to
	 * calculate these is:
	 *
	 * retVal = turnY - (normalizedTime - turnX) * (normalizedTime - turnX) * (turnY - sampleY) / ((sampleX-turnX) * (sampleX-turnX));
	 *
	 * where (turnX, turnY) is the turning point of the parabola, and
	 * (sampleX, sampleY) is a point you know to be on the curve.
	 *
	 * The X-axis here is the input normalizedTime value, and the Y-axis is the distance output.
	 */

	switch(m_velocityProfile)
	{
	case ENdhsVelocityProfileHalfsine:
		// Use quadratic approximation to half-sin

		if (normalizedTime < 0.5)
		{
			// turn: (0, 0)
			// sample: (0.5, 0.5)

			retVal = normalizedTime * normalizedTime * 2;
		}
		else
		{
			// turn: (1, 1)
			// sample: (0.5, 0.5)

			LcTScalar oneMinusNormalizedTime = 1 - normalizedTime; // precalc
			retVal = 1 - oneMinusNormalizedTime * oneMinusNormalizedTime * 2;
		}
		break;


	case ENdhsVelocityProfileAccelerate:
		{
			// turn: (0, 0)
			// sample: (1, 1)

			retVal = normalizedTime * normalizedTime;
		}
		break;

	case ENdhsVelocityProfileDecelerate:
		{
			// turn: (1, 1)
			// sample: (0, 0)

			LcTScalar oneMinusNormalizedTime = normalizedTime - 1; // precalc
			retVal = 1 - oneMinusNormalizedTime * oneMinusNormalizedTime;
		}
		break;

	case ENdhsVelocityProfileBounce:
		{
			// config:
			const LcTScalar maxVal		= (LcTScalar)NDHS_VELOCITY_PROFILE_BOUNCE_MAX_EXTENSION; 		// Maximum return value, the peak of the bounce
			const LcTScalar maxValPos	= (LcTScalar)NDHS_VELOCITY_PROFILE_BOUNCE_MAX_EXTENSION_POS; 	// The base tween position when the maxVal is reached

			if (normalizedTime < maxValPos)
			{
				// Decelerate until the maximum value is reached

				// turn: (maxVal, maxValPos)
				// sample: (0, 0)

				// precalc:
				LcTScalar distanceUntilMaxValPos = maxValPos - normalizedTime;

				retVal = maxVal - distanceUntilMaxValPos * distanceUntilMaxValPos * maxVal / (maxValPos * maxValPos);
			}
			else
			{
				// Accerate to end position after the maximum value is reached

				// turn: (maxVal, maxValPos)
				// sample: (1, 1)

				// precalc:
				LcTScalar distanceAfterMaxValPos = normalizedTime - maxValPos;
				const LcTScalar distanceUntilEnd = 1 - maxValPos;

				retVal = maxVal - distanceAfterMaxValPos * distanceAfterMaxValPos * (maxVal - 1) / (distanceUntilEnd * distanceUntilEnd);
			}
		}
		break;

	case ENdhsVelocityProfilePauseStartFinish:
		{
			LcTScalar pauseFraction = 1000.0f / m_duration;

			retVal = normalizedTime * (1.0f + pauseFraction) - (pauseFraction / 2);

			if (retVal < 0)
			{
				retVal = 0;
			}

			if(retVal >= 1)
			{
				retVal = 1;
			}
		}
		break;

	case ENdhsVelocityProfileCatapult:
		{
			// catapult

			// config:
			const LcTScalar minVal		= (LcTScalar)NDHS_VELOCITY_PROFILE_CATAPULT_MAX_DRAWBACK; 		// Minimum return value, the catapult fully pulled back
			const LcTScalar minValPos	= (LcTScalar)NDHS_VELOCITY_PROFILE_CATAPULT_MAX_DRAWBACK_POS; 	// The base tween position when the minVal is reached

			if (normalizedTime < minValPos)
			{
				// Decelerate until fully pulled back

				// turn: (minValPos, minVal)
				// sample: (0, 0)

				// precalc:
				LcTScalar distanceUntilMinVal = normalizedTime - minValPos;

				retVal = minVal - distanceUntilMinVal * distanceUntilMinVal * minVal / (minValPos * minValPos);
			}
			else
			{
				// Decelerate until end

				// turn: (1, 1)
				// sample: (minValPos, minVal)

				// precalc:
				LcTScalar normalizedTimeMinusOne = normalizedTime - 1;
				const LcTScalar minValPosMinusOne = minValPos - 1;

				retVal = 1 - normalizedTimeMinusOne * normalizedTimeMinusOne * (1 - minVal) / (minValPosMinusOne * minValPosMinusOne);
			}
		}
		break;

	default:
		break;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::setAbsolute()
{
	if (!m_wrap)
	{
		m_minValue = m_minAllowableValue;
		m_maxValue = m_maxAllowableValue;
		m_currentDeceleration = (LcTScalar)(m_maxAllowableValue / 1000000.0);
		m_wrapStride = 1;
	}
	else
	{
		// Should never get here
		LC_ASSERT(false);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::setRelative()
{
	if (!m_wrap)
	{
		m_minValue = m_minAllowableValue - m_reboundValue;
		m_maxValue = m_maxAllowableValue + m_reboundValue;
		m_wrapStride = 1;
	}
	else
	{
		m_minValue = m_origMinValue;
		m_maxValue = m_origMaxValue;
		m_wrapStride = (int)(m_maxValue - m_minValue) + 1;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCScrollPosField::setVelocity(LcTScalar velocity, LcTScalar minValue, LcTScalar maxValue)
{
	setVelocity(velocity);
}

/*-------------------------------------------------------------------------*//**
*/
LcTScalar NdhsCScrollPosField::getNormalizedValue(NdhsCElement* element)
{
	LC_UNUSED(element);
	return (LcTScalar)(m_maxValue <= m_minValue ? m_minValue : (m_currentValue - m_minValue) / (m_maxValue - m_minValue));
}

/*-------------------------------------------------------------------------*//**
*/
LcTScalar NdhsCScrollPosField::getCurrentAllowableValue()
{
	// Given the current scrollpos value, what's the value representing a valid
	// active item, considering span, rounding towards 0
	int nearestActiveItem = nearestAllowableInt(m_currentValue);

	if (m_currentValue > 0 && (m_currentValue < nearestActiveItem))
		nearestActiveItem -= m_scrollSpan;

	if (m_currentValue < 0 && (m_currentValue > nearestActiveItem))
		nearestActiveItem += m_scrollSpan;

	return (LcTScalar) nearestActiveItem;
}

/*-------------------------------------------------------------------------*//**
	Note that m_targetValue is unwrapped internally, so wrap it for external consumption
*/
LcTScalar NdhsCScrollPosField::getTargetValue()
{
	LcTDScalar wrappedTargetValue = m_targetValue;

	(void)wrapToRange(wrappedTargetValue);

	// We can assume that the lost precision from the cast down to a float will
	// not matter, as the target value will will be integral
	return (LcTScalar)wrappedTargetValue;
}
