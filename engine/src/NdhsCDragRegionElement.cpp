/***************************************************************************
*
*				Copyright 2006 Mentor Graphics Corporation
*						  All Rights Reserved.
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


#ifdef LC_USE_STYLUS

#define FLICK_VELOCITY_THRESHOLD_DECELERATION_FACTOR 400.0f
#define MAXIMUM_INTER_MOUSE_INPUT_INTERVAL_TO_FLICK 150

/*-------------------------------------------------------------------------*//**
	Static factory method
*/
LcTaOwner<NdhsCDragRegionElement>
NdhsCDragRegionElement::create(	ENdhsObjectType							use,
								const LcTmString&						className,
								NdhsCMenu*								menu,
								NdhsCMenuItem*							menuItem,
								NdhsCTemplate::CDragRegionElement*		pDE,
								NdhsCPageManager*						pPageManager,
								NdhsCElementGroup*						pPage,
								int										stackLevel)
{
	LcTaOwner<NdhsCDragRegionElement> ref;
	ref.set(new NdhsCDragRegionElement());
	ref->construct(use, className, menu, menuItem, pDE, pPageManager, pPage, stackLevel);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Constructor
*/
NdhsCDragRegionElement::NdhsCDragRegionElement()
{
}

/*-------------------------------------------------------------------------*//**
	Construction method - creates the widget and animator
*/
void NdhsCDragRegionElement::construct(	ENdhsObjectType							use,
										const LcTmString&						className,
										NdhsCMenu*								menu,
										NdhsCMenuItem*							menuItem,
										NdhsCTemplate::CDragRegionElement*		pDE,
										NdhsCPageManager*						pPageManager,
										NdhsCElementGroup*						pPage,
										int										stackLevel)
{
	setUnloaded(false);
	if (pDE)
	{
		NdhsCTemplate::CDragRegionElement::CDragRegionSetting* xDrag = pDE->xDrag.ptr();
		NdhsCTemplate::CDragRegionElement::CDragRegionSetting* yDrag = pDE->yDrag.ptr();

		LcTaString xFieldName;
		LcTaString yFieldName;

		int menuItemIndex = -1;

		if (menuItem != NULL && menu == menuItem->getOwner())
		{
			menuItemIndex = menuItem->getIndex();
		}

		m_isAbsolute = pDE->isAbsolute;

		// Set tappable status - touch regions are either 'full' or 'off'.
		// 'Partial' should be treated as 'full' - touch regions will not
		// absorb taps unless they're 'dragging' or have a tap trigger
		// defined (i.e. 'normal' drag regions will let taps through to
		// underlying elements)
		if (pDE->tappable == EPartial)
			setTappable(EFull);
		else
			setTappable(pDE->tappable);

		// Store tap tolerance
		setTapTolerance(pDE->tapTolerance);

		if (xDrag)
		{
			xFieldName = xDrag->field;

			if (xDrag->minValue && xDrag->maxValue)
			{
				m_xMinExpr = xDrag->minValue->createExpression(pPage, -1, menuItem);
				m_xMaxExpr = xDrag->maxValue->createExpression(pPage, -1, menuItem);
			}

			m_xDragEnabled = xDrag->isDragEnabled;

			if (m_xDragEnabled && !xFieldName.isEmpty())
			{
				m_xSensitivity = xDrag->sensitivity;
				m_xInverted = xDrag->inverted;

				if (m_isAbsolute)
				{
					m_xMaxSpeedExpr = NULL;
					m_xDecelerationExpr = NULL;
					m_xThreshold = 0;
				}
				else
				{
					NdhsCExpression::CExprSkeleton*	xMaxSpeed = xDrag->maxVelocity.ptr();
					if (xMaxSpeed)
						m_xMaxSpeedExpr = xMaxSpeed->getContextFreeExpression();

					NdhsCExpression::CExprSkeleton* xDeceleration = xDrag->deceleration.ptr();
					if (xDeceleration)
						m_xDecelerationExpr = xDeceleration->getContextFreeExpression();

					m_xThreshold = xDrag->threshold;
				}

				// Find the field to be controlled
				m_xField = pPage->getFieldValue(xFieldName, menu, menuItemIndex, NULL);

				if (m_xField)
				{
					m_isScrollDrag = m_xField->isScrollPosField();

					if (!m_xField->isDraggable() || (m_isScrollDrag && m_isAbsolute && ((NdhsCScrollPosField*)m_xField)->isWrapping()))
					{
						// Field is unusable
						m_xField = NULL;
					}
				}
			}
		}

		if (yDrag)
		{
			yFieldName = yDrag->field;

			if (yDrag->minValue && yDrag->maxValue)
			{
				m_yMinExpr = yDrag->minValue->createExpression(pPage, -1, menuItem);
				m_yMaxExpr = yDrag->maxValue->createExpression(pPage, -1, menuItem);
			}

			m_yDragEnabled = yDrag->isDragEnabled;

			if (m_yDragEnabled && !yFieldName.isEmpty())
			{
				m_ySensitivity = yDrag->sensitivity;
				m_yInverted = yDrag->inverted;

				if (m_isAbsolute)
				{
					m_yMaxSpeedExpr = NULL;
					m_yDecelerationExpr = NULL;
					m_yThreshold = 0;
				}
				else
				{
					NdhsCExpression::CExprSkeleton*	yMaxSpeed = yDrag->maxVelocity.ptr();
					if (yMaxSpeed)
						m_yMaxSpeedExpr = yMaxSpeed->getContextFreeExpression();

					NdhsCExpression::CExprSkeleton*	yDeceleration = yDrag->deceleration.ptr();
					if (yDeceleration)
						m_yDecelerationExpr = yDeceleration->getContextFreeExpression();

					m_yThreshold = yDrag->threshold;
				}

				// Find the field to be controlled
				m_yField = pPage->getFieldValue(yFieldName, menu, menuItemIndex, NULL);

				if (m_yField)
				{
					m_isScrollDrag = m_yField->isScrollPosField();

					if (!m_yField->isDraggable() || (m_isScrollDrag && m_isAbsolute && ((NdhsCScrollPosField*)m_yField)->isWrapping()))
					{
						// Field is unusable
						m_yField = NULL;
					}
				}
			}
		}

		setupMinMaxValues();

		// Let the base class initialize
		NdhsCElement::construct(use,
								ENdhsElementTypeDragRegion,
								pDE->elementParent,
								className,
								NULL,
								NULL,
								menu,
								menuItem,
								pPageManager,
								pPage,
								stackLevel,
								pDE->m_drawLayerIndex,
								false,
								false,
								pDE->isDetail);

		// Now attempt to create widget
		LcTaOwner<LcCWidget> newWidget =
		NdhsCMenuWidgetFactory::createWidget(	NULL,
								ENdhsWidgetTypeDragRegion,
								getPackagePath(),
								"",
								getPageManager(),
								this,
								getMenu(),
								getLocalMenuItem(),
								pPage,
								getStackLevel());

		setWidget(newWidget);

		// Set element's tappable property
		getWidget()->setTappable(getTappable());
		getWidget()->setTapTolerance(getTapTolerance());
	}
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
NdhsCDragRegionElement::~NdhsCDragRegionElement()
{
}

/*-------------------------------------------------------------------------*//**
	Switch rendering on/off
*/
bool NdhsCDragRegionElement::switchRenderingMode(bool renderingEnabled)
{
	if (!getWidget())
		return false;

	NdhsCMenuWidgetFactory::CDragRegionItem *widget = (NdhsCMenuWidgetFactory::CDragRegionItem*) (getWidget());
	widget->switchRenderingMode(renderingEnabled);
	return true;
}

/*-------------------------------------------------------------------------*//**
	Load the widget
*/
void NdhsCDragRegionElement::reloadElement()
{
	if (getWidget())
		getWidget()->setDrawLayerIndex(getDrawLayerIndex());
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCDragRegionElement::onSuspend()
{
	// Do nothing for suspend / resume
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCDragRegionElement::onResume()
{
	// Do nothing for suspend / resume
}

/*-------------------------------------------------------------------------*//**
	is Dragging
*/

bool NdhsCDragRegionElement::isDragging ()
{
	if(m_xField && !m_xField->atRest())
		return true;

	if(m_yField && !m_yField->atRest())
		return true;
	return false;
}


#ifdef IFX_SERIALIZATION
NdhsCDragRegionElement* NdhsCDragRegionElement::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCDragRegionElement *obj=new NdhsCDragRegionElement();
	if(handle==-1)
		return obj;
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}
SerializeHandle	NdhsCDragRegionElement::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCDragRegionElement) - sizeof(NdhsCElement)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;

	SerializeHandle parentHandle = NdhsCElement::serialize(serializeMaster,true);
	ENdhsElementType dataType=ENdhsElementTypeDragRegion;
	SERIALIZE(dataType,serializeMaster,cPtr);
	SERIALIZE(parentHandle,serializeMaster,cPtr);
	SERIALIZE(m_yWrap,serializeMaster,cPtr);
	SERIALIZE(m_xWrap,serializeMaster,cPtr);
	SERIALIZE(m_yThreshold,serializeMaster,cPtr);
	SERIALIZE(m_xThreshold,serializeMaster,cPtr);
	SERIALIZE(m_ySensitivity,serializeMaster,cPtr);
	SERIALIZE(m_xSensitivity,serializeMaster,cPtr);
	SERIALIZE(m_yMinValue,serializeMaster,cPtr);
	SERIALIZE(m_yMaxValue,serializeMaster,cPtr);
	SERIALIZE(m_xMaxValue,serializeMaster,cPtr);
	SERIALIZE(m_xMinValue,serializeMaster,cPtr);
	SERIALIZE_Ptr(m_yDecelerationExpr,serializeMaster,cPtr);
	SERIALIZE_Ptr(m_xDecelerationExpr,serializeMaster,cPtr);
	SERIALIZE_Ptr(m_yMaxSpeedExpr,serializeMaster,cPtr);
	SERIALIZE_Ptr(m_xMaxSpeedExpr,serializeMaster,cPtr);
	SERIALIZE(m_isScrollDrag,serializeMaster,cPtr);
	SERIALIZE(m_isAbsolute,serializeMaster,cPtr);
	SERIALIZE(m_yDragEnabled,serializeMaster,cPtr);
	SERIALIZE(m_xDragEnabled,serializeMaster,cPtr);
	SERIALIZE(m_yInverted,serializeMaster,cPtr);
	SERIALIZE(m_xInverted,serializeMaster,cPtr);
	SERIALIZE(m_yThresholdBreached,serializeMaster,cPtr);
	SERIALIZE_Owner(m_xMinExpr,serializeMaster,cPtr);
	SERIALIZE_Owner(m_xMaxExpr,serializeMaster,cPtr);
	SERIALIZE_Owner(m_yMaxExpr,serializeMaster,cPtr);
	SERIALIZE_Owner(m_yMinExpr,serializeMaster,cPtr);
	SERIALIZE(m_xCurrentVelocity,serializeMaster,cPtr);
	SERIALIZE(m_yCurrentVelocity,serializeMaster,cPtr);
	SERIALIZE_Ptr(m_xField,serializeMaster,cPtr);
	SERIALIZE_Ptr(m_yField,serializeMaster,cPtr);
	SERIALIZE(m_watchingMouse,serializeMaster,cPtr);
	SERIALIZE(m_xThresholdBreached,serializeMaster,cPtr);
	SERIALIZE(m_yThresholdBreached,serializeMaster,cPtr);

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCDragRegionElement::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummy=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;

	DESERIALIZE(dummy,serializeMaster,cPtr);
	DESERIALIZE(parentHandle,serializeMaster,cPtr);
	NdhsCElement::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE(m_yWrap,serializeMaster,cPtr);
	DESERIALIZE(m_xWrap,serializeMaster,cPtr);
	DESERIALIZE(m_yThreshold,serializeMaster,cPtr);
	DESERIALIZE(m_xThreshold,serializeMaster,cPtr);
	DESERIALIZE(m_ySensitivity,serializeMaster,cPtr);
	DESERIALIZE(m_xSensitivity,serializeMaster,cPtr);
	DESERIALIZE(m_yMinValue,serializeMaster,cPtr);
	DESERIALIZE(m_yMaxValue,serializeMaster,cPtr);
	DESERIALIZE(m_xMaxValue,serializeMaster,cPtr);
	DESERIALIZE(m_xMinValue,serializeMaster,cPtr);
	DESERIALIZE_Ptr(m_yDecelerationExpr,serializeMaster,cPtr,NdhsCExpression);
	DESERIALIZE_Ptr(m_xDecelerationExpr,serializeMaster,cPtr,NdhsCExpression);
	DESERIALIZE_Ptr(m_yMaxSpeedExpr,serializeMaster,cPtr,NdhsCExpression);
	DESERIALIZE_Ptr(m_xMaxSpeedExpr,serializeMaster,cPtr,NdhsCExpression);
	DESERIALIZE(m_isScrollDrag,serializeMaster,cPtr);
	DESERIALIZE(m_isAbsolute,serializeMaster,cPtr);
	DESERIALIZE(m_yDragEnabled,serializeMaster,cPtr);
	DESERIALIZE(m_xDragEnabled,serializeMaster,cPtr);
	DESERIALIZE(m_yInverted,serializeMaster,cPtr);
	DESERIALIZE(m_xInverted,serializeMaster,cPtr);
	DESERIALIZE(m_yThresholdBreached,serializeMaster,cPtr);
	DESERIALIZE_Owner(m_xMinExpr,serializeMaster,cPtr,NdhsCExpression);
	DESERIALIZE_Owner(m_xMaxExpr,serializeMaster,cPtr,NdhsCExpression);
	DESERIALIZE_Owner(m_yMaxExpr,serializeMaster,cPtr,NdhsCExpression);
	DESERIALIZE_Owner(m_yMinExpr,serializeMaster,cPtr,NdhsCExpression);
	DESERIALIZE(m_xCurrentVelocity,serializeMaster,cPtr);
	DESERIALIZE(m_yCurrentVelocity,serializeMaster,cPtr);
	DESERIALIZE_Ptr(m_xField,serializeMaster,cPtr,NdhsCField);
	DESERIALIZE_Ptr(m_yField,serializeMaster,cPtr,NdhsCField);
	DESERIALIZE(m_watchingMouse,serializeMaster,cPtr);
	DESERIALIZE(m_xThresholdBreached,serializeMaster,cPtr);
	DESERIALIZE(m_yThresholdBreached,serializeMaster,cPtr);

	// Now attempt to create widget
	LcTaOwner<LcCWidget> newWidget =
	NdhsCMenuWidgetFactory::createWidget(	NULL,
							ENdhsWidgetTypeDragRegion,
							getPackagePath(),
							"",
							getPageManager(),
							this,
							getMenu(),
							getLocalMenuItem(),
							getPage(),
							getStackLevel());

	setWidget(newWidget);

	// Set element's tappable property
	getWidget()->setTappable(getTappable());
	getWidget()->setTapTolerance(getTapTolerance());

	realize();
}
#endif /* IFX_SERIALIZATION */


/*-------------------------------------------------------------------------*//**
	mouse down
*/
bool NdhsCDragRegionElement::onMouseDown (const LcTPixelPoint& pt)
{
	// For this one case, we need to emulate a normal element, with
	// regards to event consumption and _touchElement updating
	if (isTapCatcher())
		return NdhsCElement::onMouseDown(pt);

	NdhsCMenuWidgetFactory::CDragRegionItem* widget = (NdhsCMenuWidgetFactory::CDragRegionItem*) (getWidget());

	if (!widget || (getPageManager()->getMouseState() == NdhsCPageManager::EMouseUp || getPageManager()->getMouseState() == NdhsCPageManager::EMouseOver))
	{
		return false;
	}

	LcTVector intersection;
	bool planeHitTest = widget->getLocalCoords(pt, intersection);
	bool consumed = false;

	if (planeHitTest && widget->isVisible())
	{
		if (widget->contains(intersection, getTapTolerance()))
		{
			// check for absolute/relative mode
			if (m_isAbsolute)
			{
				if (m_xField)
				{
					m_xField->setDragValueUpdate(true);
				}

				if (m_yField)
				{
					m_yField->setDragValueUpdate(true);
				}

				consumed = doAbsoluteUpdate(widget, intersection, false);

#ifdef IFX_GENERATE_SCRIPTS
			if(consumed)
			{
				if (NdhsCScriptGenerator::getInstance())
					NdhsCScriptGenerator::getInstance()->captureAbsoluteDragMove();
			}
#endif //IFX_GENERATE_SCRIPTS

				if (consumed)
				{
					if (m_xField)
					{
						m_xField->setUserControlLock(true);
					}

					if (m_yField)
					{
						m_yField->setUserControlLock(true);
					}
				}
			}
			else
			{
				if (m_xField || m_yField)
				{
					// Correct intersection to be absolute local coords, not
					// widget-relative
					intersection.add(widget->getPlacement().location);

					m_lastFrameMouseState.valid = false;
					m_penultimateFrameMouseState.valid = false;

					m_currentMouseState.valid = true;
					m_currentMouseState.timestamp = widget->getSpace()->getTimestamp();
					m_currentMouseState.screenPos = pt;
					m_currentMouseState.localPos = intersection;

					m_firstMouseState = m_currentMouseState;
					m_xThresholdBreached = (m_xThreshold == 0) && m_xField;
					m_yThresholdBreached = (m_yThreshold == 0) && m_yField;

					if (m_xField)
					{
						if (!m_xField->atRest())
						{
							m_xField->cancelMovement();
							m_xThresholdBreached = true;
						}
					}

					if (m_yField)
					{
						if (!m_yField->atRest())
						{
							m_yField->cancelMovement();
							m_yThresholdBreached = true;
						}
					}

					if (m_xThresholdBreached)
					{
						m_xField->takeSnapshot();
					}

					if (m_yThresholdBreached)
					{
						if (m_xField != m_yField || !m_xThresholdBreached)
						{
							m_yField->takeSnapshot();
						}
					}

					m_firstMouseState = m_currentMouseState;
					m_firstMouseState.valid = true;

					consumed = m_xThresholdBreached || m_yThresholdBreached;

					m_currentMouseState.valid = consumed;

					if (consumed)
					{
						if (m_xThresholdBreached)
						{
							m_xField->setUserControlLock(true);
							m_xField->setDragValueUpdate(true);
						}

						if (m_yThresholdBreached)
						{
							m_yField->setUserControlLock(true);
							m_yField->setDragValueUpdate(true);
						}
					}
				}
			}

			if (m_xField || m_yField)
				m_watchingMouse = true;
		}
	}

	if (consumed)
	{
		startConsuming();
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
	mouse Move
*/
bool NdhsCDragRegionElement::onMouseMove (const LcTPixelPoint& pt)
{
	// For this one case, we need to emulate a normal element, with
	// regards to event consumption and _touchElement updating
	if (isTapCatcher())
		return NdhsCElement::onMouseMove(pt);

	NdhsCMenuWidgetFactory::CDragRegionItem* widget = (NdhsCMenuWidgetFactory::CDragRegionItem*) (getWidget());

	if (!widget || (getPageManager()->getMouseState() == NdhsCPageManager::EMouseUp || getPageManager()->getMouseState() == NdhsCPageManager::EMouseOver))
	{
		return false;
	}

	LcTVector intersection;
	bool planeHitTest = widget->getLocalCoords(pt, intersection);
	bool consumed = false;

	//check for the absolute and relative mode
	if (planeHitTest && m_watchingMouse)
	{
		if (m_isAbsolute)
		{
			consumed = doAbsoluteUpdate(widget, intersection, false);

#ifdef IFX_GENERATE_SCRIPTS
			if (consumed)
			{
				if (NdhsCScriptGenerator::getInstance())
					NdhsCScriptGenerator::getInstance()->captureAbsoluteDragMove();
			}
#else
			// Avoid compiler warning for 'set but not used' local
			consumed = consumed;
#endif //IFX_GENERATE_SCRIPTS
		}
		else
		{
			// Correct intersection to be absolute local coords, not
			// widget-relative
			intersection.add(widget->getPlacement().location);

			doRelativeUpdate(widget, pt, intersection, false, false);

			if(!(m_lastFrameMouseState == m_currentMouseState))
			{
				if (!m_lastFrameMouseState.valid && (m_currentMouseState.timestamp - m_firstMouseState.timestamp) >= IFX_FRAME_INTERVAL)
				{
					m_lastFrameMouseState = m_firstMouseState;
				}
				else if ((m_currentMouseState.timestamp - m_lastFrameMouseState.timestamp) >= IFX_FRAME_INTERVAL)
				{
					m_penultimateFrameMouseState = m_lastFrameMouseState;
					m_lastFrameMouseState = m_currentMouseState;
				}
			}
		}
	}

	return m_watchingMouse && (m_isAbsolute || m_xThresholdBreached || m_yThresholdBreached);
}

/*-------------------------------------------------------------------------*//**
	mouse Up
*/
bool NdhsCDragRegionElement::onMouseUp (const LcTPixelPoint& pt)
{
	// For this one case, we need to emulate a normal element, with
	// regards to event consumption and _touchElement updating
	if (isTapCatcher())
		return NdhsCElement::onMouseUp(pt);

	NdhsCMenuWidgetFactory::CDragRegionItem* widget = (NdhsCMenuWidgetFactory::CDragRegionItem*) (getWidget());

	if (!widget)
	{
		return false;
	}

	LcTVector intersection;
	bool planeHitTest = widget->getLocalCoords(pt, intersection);
	bool wasCapturingMouse = m_watchingMouse;
	bool consumed = false;

	// check for relative mode
	if (planeHitTest && m_watchingMouse)
	{
		if (m_isAbsolute)
		{
			// set the fields values as if mouse pos lie with in configured tolerance or outside of tolerance range accordingly
			consumed = doAbsoluteUpdate(widget, intersection, true);


#ifdef IFX_GENERATE_SCRIPTS
			//stop capture move events and set capture drag
			if(consumed)
			{
				if (NdhsCScriptGenerator::getInstance())
					NdhsCScriptGenerator::getInstance()->captureAbsoluteDragMove();
			}
#endif //IFX_GENERATE_SCRIPTS

			if (m_xField)
			{
				m_xField->setUserControlLock(false);
				m_xField->setDragValueUpdate(false);
			}

			if (m_yField)
			{
				m_yField->setUserControlLock(false);
				m_yField->setDragValueUpdate(false);
			}
		}
		else
		{
			// Correct intersection to be absolute local coords, not
			// widget-relative
			intersection.add(widget->getPlacement().location);

			bool finalXUpdate = true;
			bool finalYUpdate = true;

			LcTScalar xVelocity = 0;
			LcTScalar yVelocity = 0;

			LcTTime time = 0;
			LcTScalar dLocalX = 0;
			LcTScalar dLocalY = 0;

			TMouseSnapshot* mouseSnapshot = NULL;

			LcTScalar xDeceleration = -1;
			LcTScalar yDeceleration = -1;

			// Select the best mouse state to estimate the speed
			if (m_penultimateFrameMouseState.valid)
			{
				mouseSnapshot = &m_penultimateFrameMouseState;
			}
			else if (m_lastFrameMouseState.valid)
			{
				mouseSnapshot = &m_lastFrameMouseState;
			}
			else if (m_firstMouseState.valid)
			{
				mouseSnapshot = &m_firstMouseState;
			}

			if (mouseSnapshot)
			{
				time = widget->getSpace()->getTimestamp() - mouseSnapshot->timestamp;

				if (time >= MAXIMUM_INTER_MOUSE_INPUT_INTERVAL_TO_FLICK)
				{
					// Too long between sample points, assume mouse is still
					dLocalX = 0;
					dLocalY = 0;
				}
				else if (time >= IFX_FRAME_INTERVAL)
				{
					dLocalX = intersection.x - mouseSnapshot->localPos.x;
					dLocalY = intersection.y - mouseSnapshot->localPos.y;
				}
				else
				{
					// not enough time to give a good reading of the speed
					time = 0;
				}
			}

			if (time > 0)
			{
				const LcTPlacement& placement = widget->getPlacement();

				if (m_xField && m_xThresholdBreached)
				{
					if (m_xDecelerationExpr)
					{
						m_xDecelerationExpr->evaluate(getPage(), -1, getMenuItem());

						// To convert from units/s/s to units/ms/ms, need to divide by 1000 * 1000
						xDeceleration = m_xDecelerationExpr->getValueScalar() / 1000000.0f;
					}

					if (xDeceleration >= 0)
					{
						LcTScalar normalizedOffset = dLocalX / placement.extent.x;
						LcTScalar increment = normalizedOffset * m_xSensitivity * m_xField->getSensitivityFactor();

						if (m_xInverted)
						{
							increment = -increment;
						}

						xVelocity = increment/(LcTScalar)time;

						LcTScalar minVelocity = 0;

						// Test threshold for flicking
						if (xDeceleration > 0)
						{
							minVelocity = xDeceleration * FLICK_VELOCITY_THRESHOLD_DECELERATION_FACTOR;

							if (xVelocity < minVelocity && xVelocity > -minVelocity)
							{
								xVelocity = 0.0;
							}
						}

						finalXUpdate = (xVelocity == 0.0);
					}
				}

				if (m_yField && m_yThresholdBreached)
				{
					if (m_yDecelerationExpr)
					{
						m_yDecelerationExpr->evaluate(getPage(), -1, getMenuItem());

						// To convert from units/s/s to units/ms/ms, need to divide by 1000 * 1000
						yDeceleration = m_yDecelerationExpr->getValueScalar() / 1000000.0f;
					}

					if (yDeceleration >= 0)
					{
						LcTScalar normalizedOffset = dLocalY / placement.extent.y;
						LcTScalar increment = normalizedOffset * m_ySensitivity * m_yField->getSensitivityFactor();

						if (m_yInverted)
						{
							increment = -increment;
						}

						yVelocity = increment/(LcTScalar)time;

						LcTScalar minVelocity = 0;

						// Test threshold for flicking
						if (yDeceleration > 0)
						{
							minVelocity = yDeceleration * FLICK_VELOCITY_THRESHOLD_DECELERATION_FACTOR;

							if (yVelocity < minVelocity && yVelocity > -minVelocity)
							{
								yVelocity = 0.0;
							}
						}

						finalYUpdate = (yVelocity == 0.0);
					}
				}
			}

			consumed = doRelativeUpdate(widget, pt, intersection, finalXUpdate, finalYUpdate);

			if (consumed)
			{
				if (m_xField)
				{
					if (m_xThresholdBreached)
					{
						m_xField->setUserControlLock(false);
						m_xField->setDragValueUpdate(false);
					}

					LcTScalar xMaxSpeed = -1;

					if (m_xMaxSpeedExpr)
					{
						m_xMaxSpeedExpr->evaluate(getPage(), -1, getMenuItem());

						// To convert from units/s to units/ms, need to divide by 1000
						xMaxSpeed = m_xMaxSpeedExpr->getValueScalar() / 1000.0f;
					}

					if (xMaxSpeed >= 0)
					{
						m_xField->setMaxSpeed(xMaxSpeed);
					}
					else
					{
						m_xField->setInfiniteMaxSpeed();
					}

					if (xDeceleration >= 0)
					{
						m_xField->setDeceleration(xDeceleration);
					}
					else
					{
						m_xField->setInfiniteDeceleration();
					}

					if (m_xMinValue == 0 && m_xMaxValue == 0)
					{
						m_xField->setVelocity(xVelocity);
					}
					else
					{
						m_xField->setVelocity(xVelocity, m_xMinValue, m_xMaxValue);
					}
				}

				if (m_yField)
				{
					if (m_yThresholdBreached)
					{
						m_yField->setUserControlLock(false);
						m_yField->setDragValueUpdate(false);
					}

					LcTScalar yMaxSpeed = -1;

					if (m_yMaxSpeedExpr)
					{
						m_yMaxSpeedExpr->evaluate(getPage(), -1, getMenuItem());

						// To convert from units/s to units/ms, need to divide by 1000
						yMaxSpeed = m_yMaxSpeedExpr->getValueScalar() / 1000.0f;
					}

					if (yMaxSpeed >= 0)
					{
						m_yField->setMaxSpeed(yMaxSpeed);
					}
					else
					{
						m_yField->setInfiniteMaxSpeed();
					}

					if (yDeceleration >= 0)
					{
						m_yField->setDeceleration(yDeceleration);
					}
					else
					{
						m_yField->setInfiniteDeceleration();
					}

					if (m_yMinValue == 0 && m_yMaxValue == 0)
					{
						m_yField->setVelocity(yVelocity);
					}
					else
					{
						m_yField->setVelocity(yVelocity, m_yMinValue, m_yMaxValue);
					}
				}

				getPageManager()->startAnimation();
			}
		}
	}

	setTouchdownStatus(false, false);

	m_watchingMouse = false;

	return (wasCapturingMouse && (m_isAbsolute || m_xThresholdBreached || m_yThresholdBreached));
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCDragRegionElement::onMouseCancel()
{
	// For this one case, we need to emulate a normal element, with
	// regards to event consumption and _touchElement updating
	if (isTapCatcher())
		return NdhsCElement::onMouseCancel();

	if (m_watchingMouse && (m_isAbsolute || m_xThresholdBreached || m_yThresholdBreached))
	{
		m_watchingMouse = false;
		setTouchdownStatus(false, true);

		if (m_xField)
		{
			setNormalizedValue(*m_xField,m_xMinValue,m_xMaxValue,
						getNormalizedValue(*m_xField,m_xMinValue,m_xMaxValue)
						,true);

			if (m_xThresholdBreached || m_isAbsolute)
			{
				m_xField->setUserControlLock(false);
				m_xField->setDragValueUpdate(false);
			}
		}

		if (m_yField)
		{
			setNormalizedValue(*m_yField,m_yMinValue,m_yMaxValue,
						getNormalizedValue(*m_yField,m_yMinValue,m_yMaxValue)
						,true);

			if (m_yThresholdBreached || m_isAbsolute)
			{
				m_yField->setUserControlLock(false);
				m_yField->setDragValueUpdate(false);
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCDragRegionElement::populateElementList(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList)
{
	NdhsCPageModel* pageModel = getPage()->findParentPageModel();

	if (pageModel)
	{
		if (pageModel->getPageState() == ENdhsPageStateInteractive
			|| ((pageModel->getPageState() == ENdhsPageStateSelected) && (m_isScrollDrag == false)))
		{
			// Drag regions on non-active pages shouldn't be entered in the list
			NdhsCElement::populateElementList(widgets, pageWidgetElemList);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCDragRegionElement::setNormalizedValue(NdhsCField& field, LcTScalar minVal, LcTScalar maxVal, LcTScalar val, bool finalValue)
{
	if (field.isScrollPosField())
	{
		((NdhsCScrollPosField&)field).setNormalizedValue(val,finalValue);
	}
	else
	{
		if (field.getFieldType() == IFXI_FIELDDATA_INT)
		{
			// Special case for ints - so e.g. range 0 to 3 there are 4 sections
			field.setValue((LcTScalar)max(min((int)(val * (maxVal - minVal + 1) + minVal), (int)maxVal), (int)minVal), finalValue);
		}
		else
		{
			field.setValue(val * (maxVal - minVal) + minVal, finalValue);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTScalar NdhsCDragRegionElement::getNormalizedValue(NdhsCField & field, LcTScalar minVal, LcTScalar maxVal)
{
	if (maxVal <= minVal)
	{
		return field.getNormalizedValue(this, minVal, maxVal);
	}
	else
	{
		LcTScalar val = (field.getRawFieldData(this) - minVal) / (maxVal - minVal);
		val = val < 0 ? 0 : (val > 1 ? 1 : val);
		return val;
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCDragRegionElement::doAbsoluteUpdate(
			NdhsCMenuWidgetFactory::CDragRegionItem* widget,
			const LcTVector& intersection,
			bool finalUpdate)
{
	// capture the drag event and set the field value
	bool consumed = false;
	const LcTPlacement& placement = widget->getPlacement();
	LcTVector extent = placement.extent;

	if (m_xField)
	{
		LcTScalar width = extent.x;
		LcTScalar normalizedOffset = min(1.0f, max(0.0f, (intersection.x / width + 0.5f)));

		if (m_xInverted)
		{
			normalizedOffset = 1.0f - normalizedOffset;
		}

		setNormalizedValue(*m_xField,m_xMinValue,m_xMaxValue,normalizedOffset,finalUpdate);
		consumed = true;
	}

	if (m_yField)
	{
		LcTScalar height = extent.y;
		LcTScalar normalizedOffset = min(1.0f, max(0.0f, (intersection.y / height + 0.5f)));

		if (m_yInverted)
		{
			normalizedOffset = 1.0f - normalizedOffset;
		}

		setNormalizedValue(*m_yField,m_yMinValue,m_yMaxValue,normalizedOffset,finalUpdate);
		consumed = true;
	}

	if (consumed)
	{
		doLaundryClean();
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCDragRegionElement::doRelativeUpdate(
			NdhsCMenuWidgetFactory::CDragRegionItem* widget,
			const LcTPixelPoint& pt,
			const LcTVector& intersection,
			bool finalXUpdate,
			bool finalYUpdate)
{
	bool consumed = false;
	bool eitherOrigBreached = m_xThresholdBreached || m_yThresholdBreached;

	if (m_xField && !m_xThresholdBreached)
	{
		int dX = pt.x - m_firstMouseState.screenPos.x;

		if (dX >= m_xThreshold || dX <= -m_xThreshold)
		{
			m_xThresholdBreached = true;
			m_xField->setUserControlLock(true);
			m_xField->setDragValueUpdate(true);

			if (m_xField != m_yField || !m_yThresholdBreached)
			{
				m_xField->takeSnapshot();
			}

			m_firstMouseState.screenPos.x = pt.x;
			m_firstMouseState.localPos.x = intersection.x;
		}
	}

	if (m_yField && !m_yThresholdBreached)
	{
		int dY = pt.y - m_firstMouseState.screenPos.y;

		if (dY >= m_yThreshold || dY <= -m_yThreshold)
		{
			m_yThresholdBreached = true;
			m_yField->setUserControlLock(true);
			m_yField->setDragValueUpdate(true);

			if (m_xField != m_yField || !m_xThresholdBreached)
			{
				m_yField->takeSnapshot();
			}

			m_firstMouseState.screenPos.y = pt.y;
			m_firstMouseState.localPos.y = intersection.y;
		}
	}

	if (m_xThresholdBreached || m_yThresholdBreached)
	{
		if (!eitherOrigBreached)
		{
			startConsuming();
		}

		const LcTPlacement& placement = widget->getPlacement();

#ifdef IFX_GENERATE_SCRIPTS
		LcTTime AT_previousTime = m_currentMouseState.timestamp;
		unsigned int AT_diffTime =  widget->getSpace()->getTimestamp() - AT_previousTime;

		if (NdhsCScriptGenerator::getInstance())
			NdhsCScriptGenerator::getInstance()->setScrollTime(AT_diffTime, false);
#endif

		m_currentMouseState.valid = true;
		m_currentMouseState.timestamp = widget->getSpace()->getTimestamp();

		if (m_xThresholdBreached)
		{
			m_currentMouseState.screenPos.x = pt.x;
			m_currentMouseState.localPos.x = intersection.x;
		}
		else
		{
			m_currentMouseState.screenPos.x = m_firstMouseState.screenPos.x;
			m_currentMouseState.localPos.x = m_firstMouseState.localPos.x;
		}

		if (m_yThresholdBreached)
		{
			m_currentMouseState.screenPos.y = pt.y;
			m_currentMouseState.localPos.y = intersection.y;
		}
		else
		{
			m_currentMouseState.screenPos.y = m_firstMouseState.screenPos.y;
			m_currentMouseState.localPos.y = m_firstMouseState.localPos.y;
		}

		// Calculate changes to fields
		LcTScalar xIncrement = 0;
		LcTScalar yIncrement = 0;

		if (m_xThresholdBreached && m_xField)
		{
			LcTScalar dLocalX = m_currentMouseState.localPos.x - m_firstMouseState.localPos.x;
			LcTScalar dOffset = dLocalX / placement.extent.x;
			xIncrement = dOffset * m_xSensitivity * m_xField->getSensitivityFactor();

			if (m_xInverted)
			{
				xIncrement = -xIncrement;
			}
		}

		if (m_yThresholdBreached && m_yField)
		{
			LcTScalar dLocalY = m_currentMouseState.localPos.y - m_firstMouseState.localPos.y;
			LcTScalar normalizedOffset = dLocalY / placement.extent.y;
			yIncrement = normalizedOffset * m_ySensitivity * m_yField->getSensitivityFactor();

			if (m_yInverted)
			{
				yIncrement = -yIncrement;
			}
		}

		// Perform field updates
		if (m_xField == m_yField && (m_xThresholdBreached || m_yThresholdBreached))
		{
			m_xField->addToSnapshot(xIncrement + yIncrement, m_xMinValue, m_xMaxValue, finalXUpdate && finalYUpdate);
		}
		else
		{
			if (m_xThresholdBreached && m_xField)
			{
				m_xField->addToSnapshot(xIncrement, m_xMinValue, m_xMaxValue, finalXUpdate);
			}

			if (m_yThresholdBreached && m_yField)
			{
				m_yField->addToSnapshot(yIncrement, m_yMinValue, m_yMaxValue, finalYUpdate);
			}
		}

		doLaundryClean();

		consumed = true;
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCDragRegionElement::startConsuming()
{
	if (m_xField)
	{
		if (m_xField->isScrollPosField())
		{
			NdhsCScrollPosField* spfield = (NdhsCScrollPosField*)m_xField;

			if (m_isAbsolute)
			{
				spfield->setAbsolute();
			}
			else
			{
				spfield->setRelative();
			}
		}
	}

	if (m_yField)
	{
		if (m_yField->isScrollPosField())
		{
			NdhsCScrollPosField* spfield = (NdhsCScrollPosField*)m_yField;

			if (m_isAbsolute)
			{
				spfield->setAbsolute();
			}
			else
			{
				spfield->setRelative();
			}
		}
	}

	setTouchdownStatus(true, true);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCDragRegionElement::doLaundryClean()
{
	// Add drag region to the laundry
	NdhsCPageManager* pageManager = getPageManager();

	if (pageManager)
	{
		NdhsCMenuCon* con = pageManager->getCon();

		if (con)
		{
			NdhsCLaundry* laundry = con->getLaundry();

			if (laundry)
			{
				laundry->cleanAll();
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCDragRegionElement::expressionDirty(NdhsCExpression* expr)
{
	if (   expr == m_xMinExpr.ptr() || expr == m_xMaxExpr.ptr()
		|| expr == m_yMinExpr.ptr() || expr == m_yMaxExpr.ptr())
	{
		NdhsCLaundry* laundry = getLaundry();

		if (laundry)
		{
			laundry->addItem(this);
		}
	}
	else
	{
		// Not one of this class's expressions, so pass it to base
		NdhsCElement::expressionDirty(expr);
	}
}


/*-------------------------------------------------------------------------*//**
*/
void NdhsCDragRegionElement::expressionDestroyed(NdhsCExpression* expr)
{
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCDragRegionElement::cleanLaundryItem(LcTTime timestamp)
{
	setupMinMaxValues();
	if (m_watchingMouse && m_currentMouseState.valid)
	{
		onMouseMove(m_currentMouseState.screenPos);
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCDragRegionElement::setupMinMaxValues()
{
	if (m_xDragEnabled)
	{
		if (m_xMinExpr)
		{
			m_xMinExpr->evaluate();

			if (m_xMinExpr->isError() || !m_xMinExpr->isNumeric())
			{
				m_xMinExpr->errorDiagnostics("Drag region minimum x value", true);
			}
			else
			{
				m_xMinValue = m_xMinExpr->getValueScalar();
			}
		}
		else
		{
			// error case
			m_xMinValue = 0;
		}

		if (m_xMaxExpr)
		{
			m_xMaxExpr->evaluate();

			if (m_xMaxExpr->isError() || !m_xMaxExpr->isNumeric())
			{
				m_xMaxExpr->errorDiagnostics("Drag region maximum x value", true);
			}
			else
			{
				m_xMaxValue = m_xMaxExpr->getValueScalar();
			}
		}
		else
		{
			// error case
			m_xMaxValue = 0;
		}

		if (m_xMaxValue <= m_xMinValue)
		{
			// error case
			m_xMaxValue = m_xMinValue;
		}
	}

	if (m_yDragEnabled)
	{
		if (m_yMinExpr)
		{
			m_yMinExpr->evaluate();

			if (m_yMinExpr->isError() || !m_yMinExpr->isNumeric())
			{
				m_yMinExpr->errorDiagnostics("Drag region minimum y value", true);
			}
			else
			{
				m_yMinValue = m_yMinExpr->getValueScalar();
			}
		}
		else
		{
			// error case
			m_yMinValue = 0;
		}

		if (m_yMaxExpr)
		{
			m_yMaxExpr->evaluate();

			if (m_yMaxExpr->isError() || !m_yMaxExpr->isNumeric())
			{
				m_yMaxExpr->errorDiagnostics("Drag region maximum y value", true);
			}
			else
			{
				m_yMaxValue = m_yMaxExpr->getValueScalar();
			}
		}
		else
		{
			// error case
			m_yMaxValue = 0;
		}

		if (m_yMaxValue <= m_yMinValue)
		{
			// error case
			m_yMaxValue = m_yMinValue;
		}
	}
}

#endif // LC_USE_STYLUS
