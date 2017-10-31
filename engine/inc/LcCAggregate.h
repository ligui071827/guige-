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
#ifndef LcCAggregateH
#define LcCAggregateH

#include "inflexionui/engine/inc/LcCWidget.h"

/*-------------------------------------------------------------------------*//**
	Encapsulates the aggregation of multiple child widgets into one widget that
	can be animated or used as one unit.
*/
class LcCAggregate : public LcCWidget
{
	LC_DECLARE_RTTI(LcCAggregate, LcCWidget)

private:

	typedef LcTmArray<LcCWidget*>	TmAWidget;
	TmAWidget						m_widgets;

	// Ignores all transformations except internal scale; used in calculation
	// of internal scales (canvas units etc) for child widgets
	LcTTransform					m_xfmToUnscaled;

	// Local background color (for fading)
	bool							m_bBGColor;
	LcTColor						m_cBGColor;

	// LcCWidget methods
	LC_VIRTUAL		void			internalScaleChanged(LcTVector vInternalScale);
	LC_VIRTUAL		void			setDirty();
	LC_VIRTUAL		void			setVisibilityDirty();
	LC_VIRTUAL		void			doPrepareForPaint();
	LC_VIRTUAL		void			onFontUpdated();

LC_PRIVATE_INTERNAL_PROTECTED:

	// LcCWidget methods
	LC_VIRTUAL		bool			doOnRealize();
	LC_VIRTUAL		void			doOnRetire();
	LC_VIRTUAL		void			setXfmsDirty();

LC_PRIVATE_INTERNAL_PUBLIC:

	// Widget list
					void			addWidget(LcCWidget* w);
					void			removeWidget(LcCWidget* w);

	// Helpers
				const LcTTransform&	getXfmToGlobal(bool bScaled);

LC_PROTECTED_INTERNAL_PUBLIC:

	// Child widget event handlers - defaults forward to parent
	LC_VIRTUAL		void			onWidgetEvent(LcTWidgetEvent* e);

protected:

	// Event firing
	inline			void			fireAggregateEvent(LcCWidget* w, LcCWidget::TAggregateEvent* e)
										{ w->onAggregateEvent(e); }

	// Allow only 2-phase construction
	LC_IMPORT						LcCAggregate();

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcCAggregate> create();
	LC_VIRTUAL						~LcCAggregate();

	// Others
	LC_VIRTUAL		LcTColor		getBackgroundColor();
	LC_IMPORT		void			setBackgroundColor(LcTColor c);

	// Set internal coord space equivalent to pixels
	LC_IMPORT		void			useCanvasUnits();

	// Check if this aggregate or an owning aggregate is rotated out of the Z plane
	LC_VIRTUAL		bool			aggregateRotated(LcTVector& location);

	LC_VIRTUAL		void			releaseResources();
	LC_VIRTUAL		void			reloadResources();

	LC_VIRTUAL		void			resetAnimationMask();
};

#endif // LcCAggregateH
