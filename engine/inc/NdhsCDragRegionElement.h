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

#ifndef NdhsCDragRegionElementH
#define NdhsCDragRegionElementH


#include "inflexionui/engine/inc/NdhsCElement.h"
class NdhsCField;

/*-------------------------------------------------------------------------*//**
	NdhsCDragRegionElement is the class for drag region elements
*/
class NdhsCDragRegionElement : public NdhsCElement
{

private:
	LcTScalar			m_xSensitivity;
	LcTScalar			m_ySensitivity;
	LcTScalar			m_xThreshold;
	LcTScalar			m_yThreshold;
	//Wrap feature not supported in this release
	bool				m_xWrap;
	bool				m_yWrap;
	bool				m_xInverted;
	bool				m_yInverted;
	bool				m_xDragEnabled;
	bool				m_yDragEnabled;
	bool				m_isAbsolute;
	bool				m_isScrollDrag;

	NdhsCExpression*	m_xMaxSpeedExpr;
	NdhsCExpression*	m_yMaxSpeedExpr;

	NdhsCExpression*	m_xDecelerationExpr;
	NdhsCExpression*	m_yDecelerationExpr;

	LcTScalar			m_xMinValue;
	LcTScalar			m_xMaxValue;
	LcTScalar			m_yMaxValue;
	LcTScalar			m_yMinValue;

	LcTmOwner<NdhsCExpression>	m_xMinExpr;
	LcTmOwner<NdhsCExpression>	m_xMaxExpr;
	LcTmOwner<NdhsCExpression>	m_yMaxExpr;
	LcTmOwner<NdhsCExpression>	m_yMinExpr;

	LcTScalar			m_xCurrentVelocity;
	LcTScalar			m_yCurrentVelocity;

	NdhsCField*			m_xField;
	NdhsCField*			m_yField;

	typedef struct _TMouseSnapshot_Tag
	{
		bool			valid;
		LcTTime			timestamp;
		LcTPixelPoint	screenPos;
		LcTVector		localPos;

		bool 			operator==(const _TMouseSnapshot_Tag &other) const {
							return (valid == other.valid) && (timestamp == other.timestamp) &&
								(screenPos.x == other.screenPos.x) && (screenPos.y == other.screenPos.y)
								&& (localPos.x == other.localPos.x) && (localPos.y == other.localPos.y); }
	}
	TMouseSnapshot;

	TMouseSnapshot		m_firstMouseState;
	TMouseSnapshot		m_currentMouseState;
	TMouseSnapshot		m_lastFrameMouseState;
	TMouseSnapshot		m_penultimateFrameMouseState;

	bool 				m_watchingMouse;
	bool 				m_xThresholdBreached;
	bool 				m_yThresholdBreached;

	// Two-phase construction
						NdhsCDragRegionElement();
	void				construct(	ENdhsObjectType							use,
									const LcTmString&						className,
									NdhsCMenu*								menu,
									NdhsCMenuItem*							menuItem,
									NdhsCTemplate::CDragRegionElement*	pDE,
									NdhsCPageManager*						pPageManager,
									NdhsCElementGroup*							pPage,
									int										stackLevel);

	bool				doAbsoluteUpdate(NdhsCMenuWidgetFactory::CDragRegionItem* widget, const LcTVector& intersection, bool finalUpdate);
	bool				doRelativeUpdate(NdhsCMenuWidgetFactory::CDragRegionItem* widget, const LcTPixelPoint& pt, const LcTVector& intersection, bool finalXUpdate, bool finalYUpdate);

	void				startConsuming();
	void				setupMinMaxValues();
	void 				doLaundryClean();

public:
	static LcTaOwner<NdhsCDragRegionElement>
						create(		ENdhsObjectType							use,
									const LcTmString&						className,
									NdhsCMenu*								menu,
									NdhsCMenuItem*							menuItem,
									NdhsCTemplate::CDragRegionElement*	pDE,
									NdhsCPageManager*						pPageManager,
									NdhsCElementGroup*							pPage,
									int										stackLevel);

	bool				switchRenderingMode(bool renderingEnabled);
	void				reloadElement();

	bool				isTapCatcher()		{ return m_isAbsolute && !m_xField && !m_yField && getTappable() == EFull; }

	virtual void  		onSuspend();
	virtual void  		onResume();
	void				setUnloaded(bool value){LC_UNUSED(value)}

	LcTScalar 			getNormalizedValue(NdhsCField & field,LcTScalar minVal, LcTScalar maxVal);
	void 				setNormalizedValue(NdhsCField & field,LcTScalar minVal, LcTScalar maxVal, LcTScalar val, bool finalValue);

	LcTScalar 			getNormalizedYValue(LcTScalar val)	{ return (val - m_yMinValue) / (m_yMaxValue - m_yMinValue); }

	virtual bool 		onMouseDown (const LcTPixelPoint& pt);
	virtual bool 		onMouseMove (const LcTPixelPoint& pt);
	virtual bool 		onMouseUp	(const LcTPixelPoint& pt);
	virtual void		onMouseCancel();
	virtual bool		dragScrolling()								{ return m_isScrollDrag && getTouchdownStatus(); }
	virtual bool		isDragging();

	virtual	void		populateElementList(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList);

	// NdhsIExpressionObserver support
	virtual void 		expressionDirty(NdhsCExpression* expr);
	virtual void 		expressionDestroyed(NdhsCExpression* expr);

	// NdhsILaundryItem support
	virtual bool		cleanLaundryItem(LcTTime timestamp);

	virtual				~NdhsCDragRegionElement();

#ifdef IFX_SERIALIZATION
	static NdhsCDragRegionElement* loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	SerializeHandle		serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	void				deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */
};
#endif // NdhsCDragRegionElementH
