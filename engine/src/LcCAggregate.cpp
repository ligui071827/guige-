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

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCAggregate> LcCAggregate::create()
{
	LcTaOwner<LcCAggregate> ref;
	ref.set(new LcCAggregate());
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Default constructor
*/
LC_EXPORT LcCAggregate::LcCAggregate()
{
//	m_bBGColor				= false;
//	m_iAnimatingChildren	= 0;
//	m_bRootOfTween			= false;

	// By default, aggregates don't prevent their children being sketched
	setSketchyMode(LcCWidget::ESketchyAllowed);
}

/*-------------------------------------------------------------------------*//**
	Class destructor. Deletes any widgets that have been realized on the
	aggregate.
*/
LC_EXPORT_VIRTUAL LcCAggregate::~LcCAggregate()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCAggregate::setBackgroundColor(LcTColor c)
{
	m_bBGColor	= true;
	m_cBGColor	= c;
	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
void LcCAggregate::addWidget(LcCWidget* w)
{
	m_widgets.push_back(w);
}

/*-------------------------------------------------------------------------*//**
*/
void LcCAggregate::removeWidget(LcCWidget* w)
{
	// Find and erase the widget entry
	TmAWidget::iterator it;
	it = find(m_widgets.begin(), m_widgets.end(), w);
	if (it != m_widgets.end())
		m_widgets.erase(it);

	// Will need to redraw space without it (no need to revalidate agg)
	LcCSpace* sp = getSpace();
	if (sp)
		sp->revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
const LcTTransform& LcCAggregate::getXfmToGlobal(bool bScaled)
{
	// Calc transforms only when needed
	if (m_bXfmsDirty)
		combineTransforms();

	// Return requested transform
	return bScaled? m_xfmToGlobal : m_xfmToUnscaled;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCAggregate::internalScaleChanged(LcTVector vScaleInternal)
{
	// Get accumulated internal/external scalings from parents
	m_xfmToUnscaled = m_agg? m_agg->getXfmToGlobal(false) :
		LcTTransform::identity();

	// Combine scale due to aggregate's differing internal/external extent
	m_xfmToUnscaled.scale(vScaleInternal);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTColor LcCAggregate::getBackgroundColor()
{
	// Return local background color, or that inherited from parent
	return m_bBGColor? m_cBGColor : LcCWidget::getBackgroundColor();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCAggregate::setXfmsDirty()
{
	m_bXfmsDirty = true;

	// Must recalculate transforms for sub-widgets too
	for (unsigned i = 0; i < m_widgets.size(); i++)
	{
		m_widgets[i]->setXfmsDirty();
		m_widgets[i]->onParentPlacementChange();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCAggregate::doOnRealize()
{
	// Call show handlers for sub-widgets (ignore errors)
	for (unsigned i = 0; i < m_widgets.size(); i++)
		m_widgets[i]->doOnRealize();

	// Local handler - this may create and realize sub-widgets resulting
	// in them being shown, which is why we do it AFTER showing any pre-added
	// widgets above... otherwise the new ones will get shown twice
	onRealize();

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCAggregate::doOnRetire()
{
	// Local handler
	onRetire();

	// Call hide handlers for sub-widgets
	for (unsigned i = 0; i < m_widgets.size(); i++)
		m_widgets[i]->doOnRetire();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCAggregate::onFontUpdated()
{
	// Children now search for font settings through parent aggs so tell them
	// their font may have changed
	for (unsigned i = 0; i < m_widgets.size(); i++)
		m_widgets[i]->onFontUpdated();
}

LC_EXPORT_VIRTUAL void LcCAggregate::releaseResources()
{
	for (unsigned i = 0; i < m_widgets.size(); i++)
		m_widgets[i]->releaseResources();
}

LC_EXPORT_VIRTUAL void LcCAggregate::reloadResources()
{
	for (unsigned i = 0; i < m_widgets.size(); i++)
		m_widgets[i]->reloadResources();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCAggregate::setDirty()
{
	// Note that dirty flag on aggregate itself is ignored
	for (unsigned i = 0; i < m_widgets.size(); i++)
		m_widgets[i]->setDirty();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCAggregate::setVisibilityDirty()
{
	m_bIsVisibilityCached = false;

	// all children need to know too
	for (unsigned i = 0; i < m_widgets.size(); i++)
		m_widgets[i]->setVisibilityDirty();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCAggregate::doPrepareForPaint()
{
	// Local handler
	onPrepareForPaint();

	// Call handlers for sub-widgets
	for (unsigned i = 0; i < m_widgets.size(); i++)
		m_widgets[i]->doPrepareForPaint();
}

/*-------------------------------------------------------------------------*//**
	Handles events fired from widgets.

	This method can be overridden to handle the messages within the application
	code. If this method is overridden, and the application code does not handle
	a particular event, the event should be passed on to this method. The default
	behavior of this method is to forward the message on to its parent aggregate.

	@param e The widget event to be handled
*/
LC_EXPORT_VIRTUAL void LcCAggregate::onWidgetEvent(LcTWidgetEvent* e)
{
	// Bubble event to parent unless event pertains to the aggregate itself
	// in which case the base class handling will have done this
	if (m_agg && e->getWidget() != this)
		m_agg->onWidgetEvent(e);
}

/*-------------------------------------------------------------------------*//**
	Causes the internal x,y coordinates of the widget to correspond to screen pixels.
	This assumes that neither the widget nor any of its parent aggregates are
	scaled or rotated.  Actual size and internal extent are mutually exclusive.
	If getInternalExtent() is called on a widget with actual size enabled, the x,y
	components will report the pixel area corresponding to the external extent
	set with setExtent().
*/
LC_EXPORT void LcCAggregate::useCanvasUnits()
{
	m_eInternalType = IS_PIXELS;
	setXfmsDirty();
}

/*-------------------------------------------------------------------------*//**
	The aggregate is rotated if there is rotation and the axis of rotation is not
	parallel to the z axis, or if the parent aggregate is rotated.
*/
LC_EXPORT bool LcCAggregate::aggregateRotated(LcTVector& location)
{
	LcTQuaternion ori = getOrientation();
	bool retVal = !ori.isZero() && !ori.isInXY();

	if (retVal == true)
		location = mapLocalToGlobal(LcTVector(0, 0, 0));
	else if (getAggregate() != NULL)
		retVal = getAggregate()->aggregateRotated(location);

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCAggregate::resetAnimationMask()
{
	m_animationMask = 0;

	// Call hide handlers for sub-widgets
	for (unsigned i = 0; i < m_widgets.size(); i++)
		m_widgets[i]->resetAnimationMask();
}

