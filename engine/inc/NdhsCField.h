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
#ifndef NdhsCFieldH
#define NdhsCFieldH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTString.h"
#include "inflexionui/engine/inc/LcCSerializeMaster.h"
#include "inflexionui/engine/inc/NdhsIExpressionObserver.h"
#include "inflexionui/engine/inc/NdhsCLaundry.h"


// Forward declarations
class NdhsCPluginMenu;
class NdhsCPageManager;
class NdhsCElement;

// Direction in which to change a field
enum ENdhsFieldDirection
{
	ENdhsFieldDirectionAny,
	ENdhsFieldDirectionIncreasing,
	ENdhsFieldDirectionDecreasing
};

enum ENdhsCFieldType
{
	ENdhsCFieldTypeBase,
	ENdhsCFieldTypeInt,
	ENdhsCFieldTypeFloat,
	ENdhsCFieldTypeBool,
	ENdhsCFieldTypeScrollPos,
	ENdhsCFieldTypePlugin,
	ENdhsCFieldTypeMeta,
	ENdhsCFieldTypeTime
};

// Note that this constrains the smallest velocity applied to a scroll field...velocities
// below this will be ignored, and the field will stay at the current value.  For
// example, scrolling will not work with a layout time of greater than 100 seconds
// given the value below
#define FIELD_ALMOST_ZERO	0.00001f
#define FIELD_ALMOST_ZERO_D	0.000000001f
#define OVERSHOOT_MARGIN	0.01f

// Field base class
class NdhsCField : public LcCBase, public NdhsIExpressionObserver, public NdhsILaundryItem, public ISerializeable
{
public:
	// Observer interface
	class IObserver
	{
	public:
		virtual void 			fieldValueUpdated(NdhsCField* field) = 0;
		virtual void 			fieldDirty(NdhsCField* field) = 0;
		virtual void 			fieldDestroyed(NdhsCField* field) = 0;

#ifdef IFX_SERIALIZATION
		virtual ISerializeable* getSerializeAble(int &type) = 0;
		static  IObserver*		loadState(int type,SerializeHandle h,LcCSerializeMaster * serializeMaster);
#endif /* IFX_SERIALIZATION */
	};

private:
	// Field context
	int							m_menuItemIndex;
	LcTmString					m_fieldName;
	NdhsCPlugin::NdhsCPluginMenu* m_menu;

	IFXI_FIELDDATA_TYPE 		m_fieldType;
	IFXI_FIELD_SCOPE			m_basicScope;

	NdhsCMenuCon*				m_menuCon;

	LcTmArray<IObserver*>		m_observers;

	bool						m_isInput;
	bool						m_isOutput;

	// Output fields
	bool						m_needPluginUpdate;

	// Input fields
	int							m_userLockCount;

	bool 						m_pluginField;

	bool 						m_dirty;
	bool						m_ignoreLaundry; // whether it will add itself to laundry

	// Cleaning information
	LcTmOwner<NdhsCExpression>	m_boundExpression;
	NdhsIFieldContext*			m_assignmentContext;
	int							m_assignmentSlotNum;
	NdhsCMenuItem* 				m_assignemntItem;

	bool 						m_isError;

	bool						findObserver(IObserver* obs);

protected:
	// Output fields
	LcTmString					m_currentStringValue;
	NdhsCExpression*			m_assignment;

	// Input fields
	LcTTime 					m_lastUpdated; // timestamp
	bool						m_lastUpdatedValid;

	LcTScalar					m_currentVelocity;
	LcTScalar					m_currentDeceleration; // infinite if -ve
	LcTScalar					m_currentMaxSpeed; // infinite if -ve
	LcTScalar					m_targetValue;

protected:
								NdhsCField(){}
								NdhsCField(	NdhsCMenuCon* menuCon,
											const LcTmString& fieldName,
											bool pluginField,
											NdhsCPlugin::NdhsCPluginMenu* menu,
											int menuItemIndex,
											IFXI_FIELDDATA_TYPE type,
											IFXI_FIELD_SCOPE scope,
											bool isInput,
											bool isOutput,
											const LcTaString defaultValue);


	NdhsCMenuCon*				getMenuCon()					{ return m_menuCon; }

	void						broadcastFieldValueUpdate();
	void 						broadcastFieldDirty();
	void 						broadcastFieldDestroyed();
	void 						addToLaundry();

	// Output fields
	bool						fieldNeedsPluginUpdate()		{ return m_pluginField && m_needPluginUpdate; }
	void						pluginUpdatedField()			{ if (m_isOutput) m_needPluginUpdate = false; }

	// Input fields
	bool						isSmallValue(LcTScalar val)		{ return (val >= -FIELD_ALMOST_ZERO && val <= FIELD_ALMOST_ZERO); }
	bool						isSmallValue(LcTDScalar val)	{ return (val >= -FIELD_ALMOST_ZERO_D && val <= FIELD_ALMOST_ZERO_D); }
	bool						isMaxSpeedInfinite()			{ return (m_currentMaxSpeed < 0); }
	bool						isDecelerationInfinite()		{ return (m_currentDeceleration < 0); }

	unsigned int				updateTimestamp(LcTTime newTime);
	LcTScalar					calcChangeInValue(unsigned int dTime);

	virtual bool				isAnimating()					{ return false; }
	virtual void				setAnimating(bool anim)			{ LC_UNUSED(anim); }

	bool						isLaundryUsed()					{ return m_ignoreLaundry; }
	void 						cleanSelf();

	void						setClean()						{ m_dirty = false; }

	void						setErrorStatus(bool errorStatus)	{ m_isError = errorStatus; }

#ifdef IFX_SERIALIZATION
	bool m_menuItemChild;
#endif /* IFX_SERIALIZATION */

public:

	bool						isPluginField()					{ return m_pluginField; }
	virtual bool				isInput()						{ return m_isInput; }
	virtual bool				isOutput()						{ return m_isOutput; }
			bool				isError()						{ return m_isError; }

	static LcTaOwner<NdhsCField>	create(	NdhsCMenuCon* menuCon,
											const LcTmString& fieldName,
											bool pluginField,
											NdhsCPlugin::NdhsCPluginMenu* menu,
											int menuItemIndex,
											NdhsCElement* element,
											IFXI_FIELD_MODE mode,
											IFXI_FIELD_SCOPE scope,
											IFXI_FIELDDATA_TYPE variant,
											const LcTaString defaultValue = "");

#ifdef IFX_SERIALIZATION
	static NdhsCField*	loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	static NdhsCField*	loadStatePluginField(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual		SerializeHandle	serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual		SerializeHandle	serializePluginField(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual		void	deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual		void	deSerializePluginField(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	ISerializeable*		getSerializeAble(int &type){type=3; return this;}
	bool isMenuItemChild(){return m_menuItemIndex!=-1 || m_menuItemChild;}
	void					deflate(){addToLaundry();}
	void setMenuItemChild(bool val){m_menuItemChild=val;}
#endif /* IFX_SERIALIZATION */

	/* Observer support */
			void				addObserver(IObserver* obs);
			void				removeObserver(IObserver* obs);

	/* Field context */
	virtual int					getMenuItemIndex()							{ return m_menuItemIndex; }
	virtual NdhsCPlugin::NdhsCPluginMenu* getMenu()									{ return m_menu; }
	virtual LcTaString			getFieldName() 								{ return m_fieldName; }
	virtual IFXI_FIELD_SCOPE	getBasicScope()								{ return m_basicScope; }

	/* Data access */
	virtual LcTaString			getFieldData(NdhsCElement* element)			{ return getFieldData(element, false); }
	virtual	LcTaString			getFieldData(NdhsCElement* element, bool forceBroadcast);
	virtual LcTScalar			getRawFieldData(NdhsCElement* element) 		{ LC_UNUSED(element); LC_ASSERT(false); return 0; }
	virtual bool 				getRawFieldDataBool(NdhsCElement* element)	{ LC_UNUSED(element); LC_ASSERT(false); return false; }
	virtual LcTScalar			getNormalizedValue(NdhsCElement* element, LcTScalar minValue, LcTScalar maxValue)
																			{ LC_UNUSED(element); LC_UNUSED(minValue); LC_UNUSED(maxValue); return 0; }
	virtual LcTScalar			getTargetValue()							{ LC_ASSERT(false); return 0; }
	virtual LcTScalar			getVelocity()								{ return m_isInput ? m_currentVelocity : 0; }
	virtual IFXI_FIELDDATA_TYPE	getFieldType() 								{ return m_fieldType; }
	virtual LcTScalar			getSensitivityFactor()						{ return 1.0; }
	virtual	void				refresh();
	virtual void 				setDirtyByPlugin(bool requiresLaundry);
	virtual void 				setDirtyByAssignment(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item);
	virtual bool				isScrollPosField()							{ return false; }
	virtual bool				isDraggable()								{ return m_isInput; }
	virtual void				setUserControlLock(bool lock)				{ lock ? m_userLockCount++ : m_userLockCount--; }
			bool				isUserLocked()								{ return (m_userLockCount != 0); }
	virtual void				setDragValueUpdate(bool val)				{ LC_UNUSED(val); }
			bool				isDragValueUpdate()							{ return false; }

	/* Field value scrolling */
	virtual void				setVelocity(LcTScalar velocity);
	virtual void				setVelocity(LcTScalar velocity, LcTScalar minValue, LcTScalar maxValue)
																			{ LC_UNUSED(velocity); LC_UNUSED(minValue); LC_UNUSED(maxValue); }
	virtual void				setMaxSpeed(LcTScalar maxSpeed);
	virtual void				setDeceleration(LcTScalar deceleration);
	virtual void				setInfiniteDeceleration();
	virtual void				setInfiniteMaxSpeed();
	virtual bool				setTargetValue(LcTScalar target, unsigned int duration)
																			{ LC_UNUSED(target); LC_UNUSED(duration); return false; }
	virtual bool				setTargetValue(LcTScalar target, LcTScalar minValue, LcTScalar maxValue, unsigned int duration)
																			{ LC_UNUSED(target); LC_UNUSED(minValue); LC_UNUSED(maxValue); LC_UNUSED(duration); return false; }
	virtual bool				updateValue(LcTTime timestamp, bool& finalUpdate)
																			{ LC_UNUSED(timestamp); finalUpdate = true; return false; }
	virtual void				setValue(LcTScalar newValue, bool finalUpdate)
																			{ LC_UNUSED(newValue); LC_UNUSED(finalUpdate); }
	virtual void				setValue(LcTmString newValue, bool finalUpdate)
																			{ m_currentStringValue = newValue; if (finalUpdate) broadcastFieldValueUpdate(); }
	virtual void				setValueBool(bool newValue, bool finalUpdate)
																			{ LC_UNUSED(newValue); LC_UNUSED(finalUpdate); }
	virtual void				takeSnapshot()								{}
	virtual void				addToSnapshot(LcTScalar dValue, LcTScalar minValue, LcTScalar maxValue, bool finalUpdate)
																			{ LC_UNUSED(dValue); LC_UNUSED(finalUpdate); }
	virtual void				setValueFromExpression(NdhsCExpression* expr, NdhsIFieldContext* context, int slot, NdhsCMenuItem* item);

	virtual void				cancelMovement();
	virtual bool				atRest()									{ return m_isInput ? isSmallValue(m_currentVelocity) : true; }

	virtual void				bindExpression(LcTmOwner<NdhsCExpression>& expr);

	virtual void				setIgnoreLaundry(bool ignoreLaundry)		{ m_ignoreLaundry = ignoreLaundry; }

	NdhsCExpression*			getBoundExpression()						{ return m_boundExpression.ptr(); }

	/* Expression observer */
	virtual void 				expressionDirty(NdhsCExpression* expr);
	virtual void 				expressionDestroyed(NdhsCExpression* expr);

	/* NdhsILaundryItem */
	virtual bool 				cleanLaundryItem(LcTTime timestamp);
	virtual bool 				stillDirty()								{ return (!m_ignoreLaundry) && isAnimating(); }
	virtual void 				addedToLaundry()							{ broadcastFieldDirty(); }

	virtual 					~NdhsCField();
};

#endif /* NdhsCFieldH */
