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
#ifndef LcTWidgetEventH
#define LcTWidgetEventH

// Common event types used by widgets in general
const int LC_WE_START_ANIMATE		= 503;
const int LC_WE_STOP_ANIMATE		= 504;
const int LC_WE_ABORT_ANIMATE		= 505;
const int LC_WE_IS_HIDDEN			= 506;
const int LC_WE_IS_SHOWN			= 507;
const int LC_WE_IS_HIDING			= 508;
const int LC_WE_IS_SHOWING			= 509;
const int LC_WE_TWEEN_ABORTED		= 510;
const int LC_WE_DELAY_COMPLETE		= 511;

// Base code values for standard widgets
const int LC_WE_NAVOSPHERE			= 1000;
const int LC_WE_BUTTON				= 1100;
const int LC_WE_MSGBOX				= 1200;
const int LC_WE_MENU				= 1300;
const int LC_WE_EDIT				= 1400;
const int LC_WE_LISTBOX				= 1500;
const int LC_WE_COMBOBOX			= 1600;
const int LC_WE_IMAGEBUTTON			= 1700;
const int LC_WE_LIST				= 1800;
const int LC_WE_LIST_MODEL_WIDGET	= 1900;
const int LC_WE_APPLICATION			= 9000;

class LcCWidget;
/*-------------------------------------------------------------------------*//**
	Encapsulates the set of events that control a widget, or indicate its state
*/
class LcTWidgetEvent
{
	friend class LcCWidget;

private:

	LcCWidget*			m_widget;
	int					m_code;

public:

	// Just for convenience - widget set by fire method
	inline				LcTWidgetEvent(int c)	{ m_code = c; }

	// Access
	inline	LcCWidget*	getWidget()				{ return m_widget; }
	inline	int			getCode()				{ return m_code; }
};

#endif // LcTWidgetEventH
