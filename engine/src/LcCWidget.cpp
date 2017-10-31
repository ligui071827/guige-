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
LC_EXPORT LcCWidget::LcCWidget()
{
//	m_agg					= NULL;
	m_eInternalType			= IS_NONE;
	m_isFullScreen			= false;
	m_bEnabled				= true;
	m_bVisible				= true;
	m_eTweenState			= IS_SHOWN;
	m_bXfmsDirty			= true;
	m_iFontStyle			= LcCFont::DEFAULT; // not defined
	m_iFontColor			= LcTColor::NONE; // not defined
	m_bUsingLayoutExtent	= true;	// Default to using the layout extent.
	m_useCachedOpacity		= false;
	m_isTranslucent			= false;

#ifdef LC_USE_LIGHTS
	m_lightModelSimple		= false;
#endif

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	m_currentMousePos.x = m_currentMousePos.y = m_currentMousePos.z = 0.0;
#endif
//	m_iXmlMask				= 0;
//	m_bAllowSketch			= false;	
//	m_bDirty		= false;
//	m_bNative		= false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCWidget::~LcCWidget()
{
	// First un-realize the gadget
	// NB: this will NOT call the derived class onRetire() as this has been destructed already
	retire();

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	effectUniMap.clear();
	m_textureIdMap.clear();
	m_texturePotMap.clear();
#endif

	if (m_pxfmOrientation)
		m_pxfmOrientation.free();
}

/*-------------------------------------------------------------------------*//**
	Returns the space in which this 3D component is hosted.
	Note that any changes to the component which may interact badly with paint
	or mouse operations, must be synchronized on this <tt>LcCSpace</tt> object.
	If the widget is not yet shown, returns NULL
*/
LC_EXPORT_VIRTUAL LcCSpace* LcCWidget::getSpace()
{
	return m_agg? m_agg->getSpace() : NULL;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCApplet* LcCWidget::getApplet()
{
	return m_agg? m_agg->getApplet() : NULL;
}

/*-------------------------------------------------------------------------*//**
	Add a widget to a parent aggregate
*/
LC_EXPORT bool LcCWidget::realize(LcCAggregate* s)
{
	// Already have  validate container 
	if (m_agg)
		return false;

	// Set up container
	m_agg = s;
	m_agg->addWidget(this);

	// Nothing more to do if not shown yet
	if (getSpace())
	{
		// Enables mappings
		setXfmsDirty();

		// Call widget's onRealize() handler.  Note that if doOnRealize() fails, any cleanup will
		// already have been done, including removing the widget from its parent
		// aggregate. 
		if (!doOnRealize())
			return false;
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
	Remove a widget from its parent and shut it down
*/
LC_EXPORT void LcCWidget::retire()
{
	// May be required to stop animations etc, but only if shown
	LcCSpace* sp = getSpace();
	if (sp)
		doOnRetire();

	// Remove from parent - following this the widget can be deleted
	if (m_agg)
	{
		m_agg->removeWidget(this);
		m_agg = NULL;
	}
}

/*-------------------------------------------------------------------------*//**
	This should be called whenever the z-positions or paint areas of
	the component widgets may have changed.
*/
LC_EXPORT void LcCWidget::revalidate()
{
	// Do nothing unless widget is shown
	LcCSpace* sp = getSpace();
	if (sp)
	{
		bool oldVisible = m_bCachedVisibility;
		bool newVisible = isVisible();

		// Indicate to space that repaint is necessary (if we're visible,
		// or if visibility has just changed)
		if (newVisible || (oldVisible != newVisible))
		{
			// May be overridden, e.g. by aggregate
			setDirty();

			sp->revalidate();
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCWidget::doOnRealize()
{
	m_bXfmsDirty = true;
	setVisibilityDirty();

	// Call widget's handler.  Note that at some point we could change this to
	// return a bool, but we would have to consider what to do if an aggregate's
	// onRealize() returned false... i.e. whether to onRetire() children already shown.
	// Not sure how best to handle this so leave it for now
	onRealize();

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCWidget::doOnRetire()
{
	// Call widget's handler
	onRetire();

	LcCSpace* sp = getSpace();
	sp->removeWidget(this);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCWidget::doPrepareForPaint()
{
	// Reset dirty flag before doing any preparation, so that preparation
	// methods can request revalidation next time round. 
	m_bDirty = false;

	// Call widget's handler
	onPrepareForPaint();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCWidget::doPrepareForPaintIfDirty()
{
	if (m_bDirty)
		doPrepareForPaint();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCWidget::setXfmsDirty()
{
	// If transforms require changing, we should revalidate to cause redraw
	m_bXfmsDirty = true;
	revalidate();
}

/*-------------------------------------------------------------------------*//**
	Helper for building new combined transforms whenever parts change.
	Must be called just before any transform is accessed, if m_bXfmsDirty == true.
*/
void LcCWidget::combineTransforms()
{
	// Can't do anything unless realized
	if (!getSpace())
		return;

	// Cache a orientation matrix if we need one but it's not present.  When the
	// axis is not the X, Y, or Z axis we do this here rather than in the setOrientation()
	// method, as it is more efficient when animating changes in orientation
	if (!m_pxfmOrientation && !m_placement.orientation.isZero())
	{
		m_pxfmOrientation.alloc(1);
		*m_pxfmOrientation = LcTTransform::rotate(m_placement.orientation);
	}

	// Start with parent space if there is a parent
	m_xfmToGlobal = m_agg? m_agg->getXfmToGlobal(true) : LcTTransform::identity();

	// If we have a orientation matrix...
	if (m_pxfmOrientation)
	{
		// Apply translation to left of orientation (more efficient) -> TR
		LcTTransform t(*m_pxfmOrientation);
		t.translatePrefix(m_placement.location);

		// Combine with global transform -> GTR
		m_xfmToGlobal.multiply(t);
	}
	else
	{
		// Apply translation to right of global transform -> GT
		m_xfmToGlobal.translate(m_placement.location);
	}

	// Apply scale -> GTRS or GTS
	// I.e. transforms are applied in the order: scale, rotate, translate, parent.
	m_xfmToGlobal.scale(m_placement.scale);

	// Apply offset translation to right of global transform -> GT
	m_xfmToGlobal.translate(m_placement.centerOffset);

	// If we have an internal scale type specified, we need to work out
	// the internal scale
	if (m_eInternalType == IS_PIXELS || m_eInternalType == IS_SPECIFIED)
	{
		// Calc internal extent to implement actual size
		if (m_eInternalType == IS_PIXELS)
		{
			// Parent space's unscaled mapping to global
			// NB: first time we ask parent, we might calc transforms there too!
			LcTTransform xfmParent = m_agg? m_agg->getXfmToGlobal(false)
				: LcTTransform::identity();

			// Calculate the global 3D area (1,1,0) would map to unscaled
			// NB: we do the mapping at parent's z=0
			LcTVector vtl = xfmParent.transform(LcTVector());
			LcTVector vbr = xfmParent.transform(LcTVector(1, -1, 0));

			// Calculate the global 2D pixel area this would map to
			LcTPixelPoint ptl = getSpace()->mapGlobalToCanvas(vtl);
			LcTPixelPoint pbr = getSpace()->mapGlobalToCanvas(vbr);

			// Build scale transform
			// NB: need to work out a useful Z factor here, when orientation added
			m_vScaleInternal = LcTVector(
				LcTScalar(1) / (pbr.x - ptl.x),
				LcTScalar(1) / (pbr.y - ptl.y),
				1);

			// Get preferred size in canvas units
			LcTPixelDim prefExtent = getPreferredExtent();

			// Calculate internal width
			if (m_placement.extent.x > 0)
				m_vInternalExtent.x	= m_placement.extent.x / m_vScaleInternal.x;
			else
			{
				// If external width zero, calculate from preferred width
				m_vInternalExtent.x	= LcTScalar(prefExtent.width);
			}

			// Calculate internal height
			if (m_placement.extent.y > 0)
				m_vInternalExtent.y	= m_placement.extent.y / m_vScaleInternal.y;
			else
			{
				// If external height zero, calculate from preferred height
				m_vInternalExtent.y	= LcTScalar(prefExtent.height);
			}

			// We don't have a preferred Z so if externally zero it stays zero
			m_vInternalExtent.z	= m_placement.extent.z / m_vScaleInternal.z;
		}

		// If internal extent is set...
		else if (m_eInternalType == IS_SPECIFIED)
		{
			// If either value on any axis is 0, that axis will be unscaled
			// i.e. the widget will use parent coords internally
			m_vScaleInternal = LcTVector(1, 1, 1);

			// Build scale transform from internal extent to external extent
			if (m_placement.extent.x > 0 && m_vInternalExtent.x > 0)
				m_vScaleInternal.x = LcTScalar(m_placement.extent.x) / m_vInternalExtent.x;
			if (m_placement.extent.y > 0 && m_vInternalExtent.y > 0)
				m_vScaleInternal.y = LcTScalar(m_placement.extent.y) / m_vInternalExtent.y;
			if (m_placement.extent.z > 0 && m_vInternalExtent.z > 0)
				m_vScaleInternal.z = LcTScalar(m_placement.extent.z) / m_vInternalExtent.z;
		}

		// Apply internal scale -> GTRSI
		m_xfmToGlobal.scale(m_vScaleInternal);
	}
	else
	{
		// Even with no scaling, we need to ensure unscaled transform is set
		m_vScaleInternal = LcTVector(1, 1, 1);
	}

	// Apply internal offset
	m_xfmToGlobal.translate(m_vInternalOffset);

	// Need to accumulate internal scales alone, through aggregate chains
	internalScaleChanged(m_vScaleInternal);

	// Don't need to recalc transforms until ready
	m_bXfmsDirty = false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT const LcTTransform& LcCWidget::getXfmToGlobal()
{
	if (m_bXfmsDirty)
		combineTransforms();

	return m_xfmToGlobal;
}

/*-------------------------------------------------------------------------*//**
	Sets selected components of the widget's placement, as specified by
	the given mask
*/
LC_EXPORT void LcCWidget::setPlacement(	const LcTPlacement& p,
										int mask,
										LcCAnimator::EAnimState animationState)
{
	int modifiedMask;

	// Set the current animation mask for GetAnimationMask to use
	m_animationMask  |= (mask & LcTPlacement::EAll);

	if (LcCAnimator::EAnimMidFrames == animationState)
		m_animationMask |= LcCAnimator::EMaskAnimating;

	// Assigns only those parameters specified by the mask
	if (mask != 0)
	{
		if (mask & LcTPlacement::EOpacity)
		{
			// If we're being made visible/invisible, we'll need to update the
			// cached visibility
			if (m_bIsVisibilityCached)
			{
				if (((m_placement.opacity == 0.0) && (p.opacity > 0.0))
					|| ((m_placement.opacity > 0.0) && (p.opacity == 0.0)))
					setVisibilityDirty();
			}
		}

		modifiedMask = m_placement.assign(p, mask);

		// Only refresh the widget if assign modified
		// any parameters.
		if (modifiedMask != 0)
		{
			// Orientation parameters may have changed
			// NB: the new matrix will be calculated and cached on combineTransforms()
			// as this is more efficient when animating through orientation
			if (modifiedMask & LcTPlacement::EOrientation)
				m_pxfmOrientation.free();

			if (modifiedMask & LcTPlacement::EExtent)
			{
				// Make sure we don't apply any negative extents
				m_placement.extent.capAtZero();

				// Assume the extent is the layout extent.
				m_bUsingLayoutExtent = true;

			}

			// Force refresh of combined transforms
			setXfmsDirty();

			onPlacementChange(modifiedMask);
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Sets selected components of the widget's placement, as specified by
	the given mask
*/
LC_EXPORT void LcCWidget::setPlacement(const LcTPlacement& p, int mask)
{
	// Call the internal overload always passing true (finished animation... i.e. not animating)
	setPlacement(p, mask, LcCAnimator::EAnimStopped);
}

/*-------------------------------------------------------------------------*//**
	Sets the location of the component in global 3D space.
*/
LC_EXPORT void LcCWidget::setLocation(const LcTVector& vLocation)
{
	m_placement.location = vLocation;
	setXfmsDirty();
}

/*-------------------------------------------------------------------------*//**
	Note that no origin is specified because this is defined by the internal space.
	I.e. the internal space is mapped to a cuboid of this size such that the
	internal space's origin is placed over the widget's location.
*/
LC_EXPORT void LcCWidget::setExtent(const LcTVector& vExtent, bool bUsingLayoutExtent)
{
	m_placement.extent = vExtent;
	m_bUsingLayoutExtent = bUsingLayoutExtent;
	setXfmsDirty();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTVector LcCWidget::getExtent()
{
	// Calc internal extent if dependent on transforms
	// NB: we have to do this here because zero X,Y might need to be filled in
	if (m_eInternalType == IS_PIXELS && m_bXfmsDirty)
		combineTransforms();

	return m_placement.extent;
}

/*-------------------------------------------------------------------------*//**
	Sets independent scaling factors to apply when painting this component.
	This is the distance in global space equivalent to distance 1 in local space.
*/
LC_EXPORT void LcCWidget::setScale(const LcTVector& vScale)
{
	m_placement.scale = vScale;
	setXfmsDirty();
}

/*-------------------------------------------------------------------------*//**
	Sets the orientation of the widget.
	Use this only when vAxis is not parallel with X, Y or Z.
	The orientation is clockwise looking down the vector given
*/
LC_EXPORT void LcCWidget::setOrientation(const LcTQuaternion& q)
{
	m_placement.orientation = q;

	// Free previous cached matrix
	// NB: the new matrix will be calculated and cached on combineTransforms()
	m_pxfmOrientation.free();

	// Force refresh of orientation matrix and combined transforms
	setXfmsDirty();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCWidget::setOffset(const LcTVector& vOffset)
{
	m_placement.centerOffset		= vOffset;
	setXfmsDirty();
}

/*-------------------------------------------------------------------------*//**
	Configures the bounds of the internal coordinate space of a widget.
	This is the cuboid volume in the widget's internal space which will
	be mapped to the external extent of the widget as set by setExtent().
	For example, the internal extent could be set to (1,1,1) for a widget
	which is internally implemented to occupy a unit cube, and subsequently
	stretched to fill its extent.  If not set, the widget coordinate space
	will map 1:1 on to the parent coordinate space, translated to the widget
	location.  Internal extent and actual size are mutually exclusive.
*/
LC_EXPORT void LcCWidget::setInternalExtent(const LcTVector& vIE)
{
	m_vInternalExtent	= vIE;
	m_eInternalType		= IS_SPECIFIED;
	setXfmsDirty();
}

/*-------------------------------------------------------------------------*//**
	Reports the volume to be occupied by a widget, in its internal coordinate space.
*/
LC_EXPORT LcTVector LcCWidget::getInternalExtent()
{
	// Calc internal extent if dependent on transforms
	if (m_eInternalType == IS_PIXELS && m_bXfmsDirty)
		combineTransforms();

	// Return outer extent unless internal configured
	return m_eInternalType == IS_NONE ? m_placement.extent : m_vInternalExtent;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCWidget::setInternalOffset(const LcTVector& vOS)
{
	m_vInternalOffset	= vOS;
	setXfmsDirty();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTVector LcCWidget::getInternalOffset()
{
	return m_vInternalOffset;
}

/*-------------------------------------------------------------------------*//**
	Will be called to get the preferred extent of this widget in canvas units.
	Will be called only if useCanvasUnits() has been specified and either X
	or Y of the extent is zero.  In or after onRealize(), getExtent() will then
	return X or Y values calculated such that they scale correctly to the
	preferred width/height.  This could be called from an enclosing aggregate's
	onRealize() handler to adjust the layout accordingly.
*/
LC_EXPORT_VIRTUAL LcTPixelDim LcCWidget::getPreferredExtent()
{
	return LcTPixelDim(0, 0);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCWidget::setEnabled(bool b)
{
	// Change in enabled state can only require revalidation if parent enabled
	bool bChanged = (b != m_bEnabled && (!m_agg || m_agg->isEnabled()));

	m_bEnabled = b;

	if (bChanged)
		revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCWidget::isEnabled()
{
	return m_bEnabled && (!m_agg || m_agg->isEnabled());
}

/*-------------------------------------------------------------------------*//**
	Sets the desired visibility of this widget.  The widget may still not
	actually appear yet, if it is not realized or its parent is not visible
*/
LC_EXPORT void LcCWidget::setVisible(bool b)
{
	// Do nothing unless changing state
	if (b == m_bVisible)
		return;

	// Note that m_bVisible is the desired status of the widget local to
	// its parent - the actual visibility at any point in time depends on
	// the tween state (e.g. if m_bVisible == false and we're doing a hide
	// animation, the widget is still visible)
	m_bVisible = b;

	// isVisible() may return incorrect value if we don't update the cache...
	setVisibilityDirty();

	// Set tween state to be "already done" ... note that no events are fired
	if (m_bVisible)
		m_eTweenState = IS_SHOWN;
	else
		m_eTweenState = IS_HIDDEN;

	// Need to repaint with state change
	revalidate();
}

/*-------------------------------------------------------------------------*//**
	Returns true if the widget is actually visible.  Actual visibility
	may not correspond to the visibility flag set by setVisible() and
	returned by getVisible(), because isVisible() also takes into account
		(a) opacity
		(b) the visibility of the parent
		(c) temporary visibility whilst show/hide animations complete
*/
LC_EXPORT bool LcCWidget::isVisible()
{
	// Cache result, as call to parentIsVisible() potentially expensive
	// Result is recalculated when widget is dirtied
	if (!m_bIsVisibilityCached)
	{
		m_bCachedVisibility = m_eTweenState != IS_HIDDEN
			&& m_placement.opacity > 0
			&& parentIsVisible();
		m_bIsVisibilityCached = true;
	}

	return m_bCachedVisibility;
}

/*-------------------------------------------------------------------------*//**
	Returns true if the widget is realized and its parent is visible
*/
LC_EXPORT_VIRTUAL bool LcCWidget::parentIsVisible()
{
	return m_agg && m_agg->isVisible();
}

/*-------------------------------------------------------------------------*//**
	Returns true if the widget is hidden. Takes into account whether the
	parent is hidden or not.
*/
LC_EXPORT bool LcCWidget::isHidden()
{
	return	m_eTweenState == IS_HIDDEN
			|| parentIsHidden();
}

/*-------------------------------------------------------------------------*//**
	Returns true if the widget is realized and its parent is hidden
*/
LC_EXPORT_VIRTUAL bool LcCWidget::parentIsHidden()
{
	return m_agg && m_agg->isHidden();
}

/*-------------------------------------------------------------------------*//**
	For no override, pass name "" or style -1
*/
LC_EXPORT void LcCWidget::setFont(const LcTmString& sName, LcCFont::EStyle iStyle)
{
	if (m_sFontName != sName || m_iFontStyle != iStyle)
	{
		m_sFontName		= sName;
		m_iFontStyle	= iStyle;

		// Notify derived class so that it can re-load font object
		onFontUpdated();
		revalidate();
	}
}

/*-------------------------------------------------------------------------*//**
	For no override, pass color NONE
*/
LC_EXPORT void LcCWidget::setFontColor(LcTColor iColor)
{
/*	if (m_iFontColor != iColor)
	{
		m_iFontColor = iColor;
		revalidate();
	}*///font color now stored in placement

	if(m_placement.color != iColor)
	{
		m_placement.color = iColor;
		revalidate();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTaString LcCWidget::findFontName()
{
	return (m_sFontName.length() > 1 || !m_agg)?
		(LcTaString)m_sFontName : m_agg->findFontName();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCFont::EStyle LcCWidget::findFontStyle()
{
	return (m_iFontStyle != -1 || !m_agg)?
		m_iFontStyle : m_agg->findFontStyle();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTColor LcCWidget::findFontColor()
{
	return (m_placement.color != LcTColor::NONE || !m_agg)?
		m_placement.color : m_agg->findFontColor();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTColor LcCWidget::findFontColorDisabled()
{
	if (!m_agg)
		return LcTColor::NONE;
	else
		return m_agg->findFontColorDisabled();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCWidget::canSketch()
{
#if defined (LC_PLAT_OGL)
	// Ignore sketchy mode for OpenGL
	return false;
#else
	#ifdef LC_PAINT_DEBUG
		// In debug rectangle mode, draw all quality=low items sketchy
		// even when not animating (for debugging paint positions etc)
		if (m_iSketchyMode == ESketchyForced)
			return true;
	#endif

		// Get the animation mask for the widget currently being drawn
		int mask = getAnimationMask();

		// The image can be sketched (drawn inaccurately) as long as it is part
		// of a widget that is being animated in position or orientation
		// - unless the disallow bit is set, in which case, don't sketch
		// - or if the force bit is set in which case sketch when animating.
		return (0 == (mask & LcCAnimator::EMaskDisallowSketch))
				&& ((0 != (mask & LcCAnimator::EMaskAnimating))
					&& ((0 != (mask & LcCAnimator::EMaskForceSketch))
							|| ((0 != (mask & (LcTPlacement::ELocation
									| LcTPlacement::EOrientation))))));
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCWidget::setOpacity(LcTScalar d)
{
	// If we've been made visible/invisible, we'll need to update the
	// cached visibility
	if (m_bIsVisibilityCached)
	{
		if (((m_placement.opacity == 0.0) && (d > 0.0))
			|| ((m_placement.opacity > 0.0) && (d == 0.0)))
			setVisibilityDirty();
	}

	m_placement.opacity = d;

	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCWidget::setFrame(int fr)
{
	m_placement.frame = fr;
	revalidate();
}

/*-------------------------------------------------------------------------*//**
	Translate a point from local 3D space to global 3D space.
*/
LC_EXPORT LcTVector LcCWidget::mapLocalToGlobal(const LcTVector& vLocal)
{
	// Calc transforms only when needed
	if (m_bXfmsDirty)
		combineTransforms();

	// Apply the combined transform
	return m_xfmToGlobal.transform(vLocal);
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
	Translate a point from global 3D space to local 3D space.
*/
LC_EXPORT LcTVector LcCWidget::mapGlobalToLocal(const LcTVector& vGlobal)
{
	// Calc transforms only when needed
	if (m_bXfmsDirty)
		combineTransforms();

	// Apply the inverse transform
	LcTTransform xfmFromGlobal = m_xfmToGlobal;
	xfmFromGlobal.invert();
	return xfmFromGlobal.transform(vGlobal);
}
#endif



/*-------------------------------------------------------------------------*//**
	Fire a widget event to the parent aggregate
*/
LC_EXPORT void LcCWidget::fireWidgetEvent(LcTWidgetEvent* e)
{
	// Ensure originator is set
	e->m_widget = this;

	if (m_agg)
		m_agg->onWidgetEvent(e);

	onWidgetEvent(e);
}

/*-------------------------------------------------------------------------*//**
	Called by derived widget to determine overall fade to apply to this widget
*/
LC_EXPORT_VIRTUAL void LcCWidget::onAnimationEvent(LcCAnimator* a, int code)
{
	// Default case if not tweening - pass event to observers
	LcCAnimator::TEvent e(code, a);
	fireWidgetEvent(&e);
}

/*-------------------------------------------------------------------------*//**
	Returns a bit pattern indicating which widget parameters are currently
	being animated - bits defined in LcTPlacement and LcCAnimator
*/
int LcCWidget::getAnimationMask()
{
	int mask = m_animationMask;

	// If widget does not allow sketching, include a disallow bit
	if (m_iSketchyMode == ESketchyDisabled)
	{
		mask |= LcCAnimator::EMaskDisallowSketch;
	}
	// If the widget is in low quality mode, force sketchy.
	else if (m_iSketchyMode == ESketchyForced)
	{
		mask |= LcCAnimator::EMaskForceSketch;
	}

	// Combine with animations applied to widget's parents
	if (m_agg)
		mask |= m_agg->getAnimationMask();

	return mask;
}

/*-------------------------------------------------------------------------*//**
*/
void LcCWidget::resetAnimationMask()
{	
	m_animationMask = 0;
}

/*-------------------------------------------------------------------------*//**
	Called by derived widget to determine overall fade to apply to this widget
*/
LC_EXPORT LcTScalar LcCWidget::getOverallOpacity()
{
	if(m_useCachedOpacity)
	{
		return m_cachedOpacity;
	}
	// Combine with parent opacity
	return m_agg?
		  m_agg->getOverallOpacity() * m_placement.opacity
		: m_placement.opacity;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTVector LcCWidget::getOverallScale()
{
	if (m_bXfmsDirty)
		combineTransforms();

	// Combine with parent scale
	LcTVector overallScale;
	if(m_agg != NULL)
		overallScale = m_agg->getOverallScale();
	else
		overallScale = LcTVector(1,1,1);

	// Apply both explicit scale and internal scale due to extent matching
	overallScale.scale(m_placement.scale).scale(m_vScaleInternal);
	return overallScale;
}

/*-------------------------------------------------------------------------*//**
	Called by derived widget to determine overall fade to apply to this widget
*/
LC_EXPORT_VIRTUAL LcTColor LcCWidget::getBackgroundColor()
{
	if (m_agg)
		return m_agg->getBackgroundColor();
	else
		return LcTColor::NONE;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCWidget::isTranslucent()
{	
	return (m_isTranslucent
#if defined(IFX_RENDER_DIRECT_OPENGL_20)	
		|| m_effectMakesTranslucent == EEffectTranslucencyCacheTranslucent
#endif 
	);
}

/*-------------------------------------------------------------------------*//**
*/
bool LcCWidget::isUnder(LcCWidget* w)
{
	return w == this || (m_agg && m_agg->isUnder(w));
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCWidget::aggregateRotated(LcTVector& location)
{
	return m_agg ? m_agg->aggregateRotated(location) : false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTVector LcCWidget::getGlobalNormal()
{
	LcTVector vUnitZ(0.0f, 0.0f, 1.0f);
	LcTVector localOrigin = mapLocalToGlobal(LcTVector(0, 0, 0));

	// Translate a unit Z vector from widget space to get a normal to
	// the widget's x,y plane in global space
	LcTVector vNormal = mapLocalToGlobal(vUnitZ).subtract(localOrigin);

	// Cope with the degenerate case where e.g. the z-scale is 0
	if (vNormal.isZero())
	{
		LcTVector vUnitX(1.0f, 0.0f, 0.0f);
		LcTVector vUnitY(0.0f, 1.0f, 0.0f);

		LcTVector vX = mapLocalToGlobal(vUnitX).subtract(localOrigin);
		LcTVector vY = mapLocalToGlobal(vUnitY).subtract(localOrigin);

		vNormal = vX.cross(vY);
	}

	// Normalize to a unit vector
	vNormal.normalise();

	return vNormal;
}

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT void LcCWidget::addEffectUniform(LcTmString name, LcOglCSLType* slType, LcCSpace *space)
{
	LcOglCEffect *effect = NULL;
	LcOglCContext* context = NULL;

	if(space)
		context = space->getOglContext();

	if(context)
		effect = context->getEffectByName(m_visualEffect);

	if(!effect)
	{
		return;
	}

	// Add the uniform to this widget only if it is valid according to
	// .effect file of the specifie effect
	if (effect->isValidConfigUniform(slType))
	{
		effect->cloneSLType (slType, name, &effectUniMap);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT void LcCWidget::configEffectUniforms(LcCSpace *space, LcTmString path)
{
	LcOglCEffect *effect = NULL;
	LcOglCContext* context = NULL;

	if(space)
		context = space->getOglContext();

	if(context)
		effect = context->getEffectByName(m_visualEffect);

	if(!effect)
	{
		return;
	}

	LcTmOwnerMap<LcTmString, LcOglCSLType>::iterator it;
	LcTaString resourceName;
	LcTaString name;
	LcOglCSLType *slType = NULL;
	CTextureInfo* textureInfo = NULL;
	LcOglCBitmap *image = NULL;

	setEffect(effect);

	m_textureIdMap.clear();
	m_texturePotMap.clear();

	for(it=effectUniMap.begin(); it != effectUniMap.end(); it++)
	{
		name = (*it).first;
		slType = (*it).second;

		if (!slType) continue;

		if (0 == slType->getSLTypeIdentifier().compareNoCase("sampler2D"))
		{
			resourceName = ((LcOglCSLTypeScalar<LcTmString> *)slType)->getValue();

			// Getting texture information.
			textureInfo = effect->getTextureInfoByName(name);

			if(textureInfo != NULL)
			{
				context->setTextureInfo (textureInfo);

				image = (LcOglCBitmap *)space->getBitmap(resourceName);

				if(image)
				{
					LcTScalarRect rect;

					LcOglCTexture* texture = image->getTexture(rect);

					((LcOglCSLTypeScalar<LcTmString> *)slType)->setLocationIndex(effect->getUniformLocByName(name));

					effect->setTextureInfo(name, texture->getTexture(), texture->isPOT());

					// Store the texture information into corresponding maps for later use
					m_textureIdMap[name] = texture->getTexture();
					m_texturePotMap[texture->getTexture()] = texture->isPOT();

					// Update the wrapping mode for texture Info
					textureInfo->setWrapMode (texture->getWrapMode());

					// Update the texture info with this newly read bitmap for later referral
					textureInfo->setBitmap (image);

					context->setTextureInfo (NULL);
				}
				else
				{
					effect->setTextureInfo(name, 0, false);
					textureInfo->setBitmap (image);
					m_textureIdMap[name] = 0;
					m_texturePotMap [0] = false;
				}
			}
		}
		else if (0 == slType->getSLTypeIdentifier().compareNoCase("samplerCube"))
		{
			LcTaString names[6];

			((LcOglCSLTypeVector<LcTmString> *)slType)->getValue(names);

			// Getting texture information from effect.
			textureInfo = effect->getTextureInfoByName(name);

			if(textureInfo != NULL)
			{
				context->setTextureInfo (textureInfo);

				GLuint id = context->getCubeBitmaps (names[0],
												  names[1],
												  names[2],
												  names[3],
												  names[4],
												  names[5]);
				m_textureIdMap[name] = id;
				m_texturePotMap[id] = false;

				textureInfo->setWrapMode (GL_CLAMP);

				context->setTextureInfo (NULL);
			}
		}
		else
		{
			// Get the uniform location for non-sampler class-level effect uniforms
			slType->setLocationIndex(effect->getUniformLocByName(name));
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT int LcCWidget::passWidgetInfoToOGL()
{
	int status = 0;
	int id = 0;
	bool isPOT = false;
	LcTaString name;
	LcTmMap<LcTmString, GLuint>::iterator it;

	// If the widget has no effect, makes no sense to proceed further
	if (!m_effect)
	{
		status = -1;
		return status;
	}

	for(it=m_textureIdMap.begin(); it != m_textureIdMap.end(); it++)
	{
		name = (*it).first;
		id = (*it).second;

		isPOT = m_texturePotMap[id];

		m_effect->setTextureInfo(name, id, isPOT);
	}

	// Traverse through the rest of effect uniforms (other than samplers)
	// and pass on their values to OGL2
	LcTmOwnerMap<LcTmString, LcOglCSLType>::iterator ait;

	for(ait=effectUniMap.begin(); ait != effectUniMap.end(); ait++)
	{
		LcTaString name = (*ait).first;
		LcOglCSLType *slType = (*ait).second;

		if (!slType) continue;

		if ( (slType->getSLTypeIdentifier().compareNoCase("sampler2D")	 == 0) ||
			 (slType->getSLTypeIdentifier().compareNoCase("samplerCube") == 0) )
		{
			continue;
		}
		else // All non-sampler uniforms
		{
			status = m_effect->passUniformInfoToOGL (slType);
		}
	}

	// Traverse the layout related parameters with tweened values
	for(ait=m_placement.layoutUniMap.begin(); ait != m_placement.layoutUniMap.end(); ait++)
	{
		LcTaString name = (*ait).first;
		LcOglCSLType *slType = (*ait).second;

		if (!slType) continue;

		// Only Interpolatable uniforms!
		if (slType->isInterpolatable())
		{
			slType->setLocationIndex(m_effect->getUniformLocByName(name));
			status = m_effect->passUniformInfoToOGL (slType);
		}
	}

	return (status);
}

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT int LcCWidget::passWidgetThemeInfoToOGL()
{
	int status = 0;
	LcOglCSLType *slType = NULL;

	// If the widget has no effect, makes no sense to proceed further
	if (!m_effect)
	{
		status = -1;
		return status;
	}

	//----------------------------------
	// 1 - "IFX_TAP_X"
	//----------------------------------
	slType = m_effect->getSLTypeByMapping(IFX_TAP_X);
	if (slType)
	{
		float value = m_currentMousePos.x;

		((LcOglCSLTypeScalar<float> *)slType)->setValue(&value);

		status = m_effect->passUniformInfoToOGL (slType);
	}

	//----------------------------------
	// 2 - "IFX_TAP_Y"
	//----------------------------------
	slType = m_effect->getSLTypeByMapping(IFX_TAP_Y);
	if (slType)
	{
		float value = m_currentMousePos.y;

		((LcOglCSLTypeScalar<float> *)slType)->setValue(&value);

		status = m_effect->passUniformInfoToOGL (slType);
	}

	//----------------------------------
	// 3 - "IFX_TAP_Z"
	//----------------------------------
	slType = m_effect->getSLTypeByMapping(IFX_TAP_Z);
	if (slType)
	{
		float value = 0.0f;

		((LcOglCSLTypeScalar<float> *)slType)->setValue(&value);

		status = m_effect->passUniformInfoToOGL (slType);
	}

	//----------------------------------
	// 4 - "IFX_TAP_XY"
	//----------------------------------
	slType = m_effect->getSLTypeByMapping(IFX_TAP_XY);
	if(slType)
	{
		float values [2];

		values[0] = m_currentMousePos.x;
		values[1] = m_currentMousePos.y;

		((LcOglCSLTypeVector<float> *)slType)->setValue(values);

		status = m_effect->passUniformInfoToOGL (slType);
	}

	//----------------------------------
	// 5 - "IFX_TAP_XYZ"
	//----------------------------------
	slType = m_effect->getSLTypeByMapping(IFX_TAP_XYZ);
	if(slType)
	{
		float values [3];

		values[0] = m_currentMousePos.x;
		values[1] = m_currentMousePos.y;
		values[2] = 0.0f;

		((LcOglCSLTypeVector<float> *)slType)->setValue(values);

		status = m_effect->passUniformInfoToOGL (slType);
	}

	//----------------------------------
	// 6 - "IFX_ELEMENT_LOCATION"
	//----------------------------------
	slType = m_effect->getSLTypeByMapping(IFX_ELEMENT_LOCATION);
	if(slType)
	{
		float values [3];

		values[0] = m_placement.location.x;
		values[1] = m_placement.location.y;
		values[2] = m_placement.location.z;

		((LcOglCSLTypeVector<float> *)slType)->setValue(values);

		status = m_effect->passUniformInfoToOGL (slType);
	}

	//----------------------------------
	// 7 - "IFX_ELEMENT_OPACITY"
	//----------------------------------
	slType = m_effect->getSLTypeByMapping(IFX_ELEMENT_OPACITY);
	if(slType)
	{
		float value = m_placement.opacity;

		((LcOglCSLTypeScalar<float> *)slType)->setValue(&value);

		status = m_effect->passUniformInfoToOGL (slType);
	}

	//----------------------------------
	// 8 - "IFX_SCREEN_WIDTH"
	//----------------------------------
	slType = m_effect->getSLTypeByMapping(IFX_SCREEN_WIDTH);
	if(slType)
	{
		int value = 0;
		LcCSpace *space = getSpace();

		if (space)
		{
			value = (int)space->getGlobalExtent().x;
		}

		((LcOglCSLTypeScalar<int> *)slType)->setValue(&value);

		status = m_effect->passUniformInfoToOGL (slType);
	}

	//----------------------------------
	// 9 - "IFX_SCREEN_HEIGHT"
	//----------------------------------
	slType = m_effect->getSLTypeByMapping(IFX_SCREEN_HEIGHT);
	if(slType)
	{
		int value = 0;
		LcCSpace *space = getSpace();

		if (space)
		{
			value = (int)space->getGlobalExtent().y;
		}

		((LcOglCSLTypeScalar<int> *)slType)->setValue(&value);

		status = m_effect->passUniformInfoToOGL (slType);
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT void LcCWidget::calcTapPosition(const LcTPixelPoint& pt)
{
	LcTVector extent = getExtent();
	LcTScalar width;
	LcTScalar height;
	int reqWidth	= 32;
	int reqHeight	= 32;

	while (reqWidth < this->width)
		reqWidth <<= 1;
	while (reqHeight < this->height)
		reqHeight <<= 1;

	LcCSpace* space = getSpace();

	if (space)
	{
		width = space->getGlobalExtent().x;
		height = space->getGlobalExtent().y;

		LcTVector c = mapLocalToGlobal(m_placement.location);
		float xc = c.x + (width / 2);
		float yc = (height / 2) - (c.y);
		float xres = 1 / extent.x;
		float yres = 1 / extent.y;

		m_currentMousePos.x = ((pt.x - xc) * xres) + 0.5f;
		m_currentMousePos.y = ((pt.y - yc) * yres) + 0.5f;
		m_currentMousePos.z = 0.0f;

		// Adjust x coordinate if needed
		if (reqWidth != this->width)
		{
			if (this->width > 0)
				m_currentMousePos.x = m_currentMousePos.x * this->width / reqWidth;
		}

		// Adjust y coordinate if needed
		if (reqHeight != this->height)
		{
			if (this->height > 0)
				m_currentMousePos.y = m_currentMousePos.y * this->height / reqHeight;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT void LcCWidget::populateEffectUniformsFromEffectFile (LcCSpace *space)
{
	LcOglCEffect *effect = NULL;
	LcOglCContext* context = NULL;

	if(space)
		context = space->getOglContext();

	if(context)
		effect = context->getEffectByName(m_visualEffect);

	if(!effect)
	{
		return;
	}
	// First of all, fill the effect uniform map with config uniforms as specified in
	// the .effect file. This is to ensure that these uniforms exist in the map
	// even if user forgets/opts not to specify any of them

	// Populate the (class-level) effect uniforms map with configurable uniforms
	// known for this effect in question
	effect->populateMapWithConfigUniforms (&effectUniMap, false); // reset interpolatable flag
}

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT void LcCWidget::prepareStaticDisplacementFromEffect (LcTPlacement& layoutPlacement, LcTPlacement& staticPlacement)
{
	LcOglCEffect *effect = NULL;

	effect = getEffect();

	if(!effect)
	{
		return;
	}

	effect->populateMapWithMissingConfigUniforms (&staticPlacement.layoutUniMap, true); // set interpolatable flag
}

#endif /* IFX_RENDER_DIRECT_OPENGL_20 */

/*-------------------------------------------------------------------------*//**
*/
LcTScalar LcCWidget::getGlobalZ(bool& bRotated, bool& bParentRotated)
{
	LcTVector location;

	// Check to see if one of the owning aggregates is rotated - if it is,
	// then the location in global space of the aggregate is returned.
	bParentRotated = aggregateRotated(location);
	bRotated = !(getOrientation().isZero()) && !(getOrientation().isInXY());

	// Transform location to global coords if no aggregate is rotated
	if(bParentRotated == false)
		location = mapLocalToGlobal(LcTVector(0.0F, 0.0F, 0.0F));

	return location.z;
}

