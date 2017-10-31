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
#include "inflexionui/engine/ifxui_engine_porting.h"
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


#if !defined(UPDATE_SOFTKEYS_CMD)
	#define UPDATE_SOFTKEYS_CMD 101
#endif

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<NdhsCAppUi> NdhsCAppUi::create(	const LcTmString& language,
													const LcTmString& displayMode,
													const LcTmString& themeName,
													bool restoreSavedState)

{
	LcTaOwner<NdhsCAppUi> ref;
	ref.set(new NdhsCAppUi);
	ref->construct(language, displayMode, themeName,restoreSavedState);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCAppUi::~NdhsCAppUi()
{
	// Clear environment space.
	LcCEnvironment* env = LcCEnvironment::get();
	env->setActiveSpace(NULL);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCAppUi::construct(	const LcTmString& language,
							const LcTmString& displayMode,
							const LcTmString& themeName,
							bool restoreSavedState)
{
	m_suspended = false;

	// Create appropriate kind of space
	LcTaOwner<LcCSpace> newSpace = LcCSpace::create();

	// Set environment space
	LcCEnvironment::get()->setActiveSpace(newSpace.ptr());

	// Configure it
	newSpace->setFrameInterval(IFX_FRAME_INTERVAL);
	setSpace(newSpace);

	// Create the UI mutex
	m_uiLock = LcCMutex::create("IFX_APP");

	// Set ini file save location
	m_dataPath = NDHS_INI_FILE_SAVE_LOCATION;

	// Convert all the back slashes to forward slashes.
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		m_dataPath.replace(NDHS_PLAT_DIR_SEP_CHAR, NDHS_DIR_SEP_CHAR);
	#endif

	// Finally call base construct() with the correct size.
	NdhsCMenuCon::construct(language, displayMode, themeName, restoreSavedState);
}

/*-------------------------------------------------------------------------*//**
	Create and return a page manager object.
*/
LcTaOwner<NdhsCPageManager> NdhsCAppUi::createPageManager(const LcTmString& language,
														  const LcTmString& displayMode,
														  const LcTmString& themeName)
{
	// Create the page manager with specified configuration.
	LcTaOwner<NdhsCPageManager> ref = NdhsCPageManager::create(	getAppName(),
																this,
																language,
																displayMode,
																themeName);

	getSpace()->setFocus(ref.ptr());
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Will be called when theme is applied, which must happen before ANY
	NDE content is displayed on screen - so configure space here
*/
void NdhsCAppUi::configureScreenCanvas(bool statusPane)
{
	int	scrnMidHeight		= 0;
	int	scrnMidWidth		= 0;
	int configCanvasWidth	= 0;
	int configCanvasHeight	= 0;
	int originY				= 0;
	int designWidth			= 0;

	LC_UNUSED(statusPane)

	// Get theme size metrics
	getPageManager()->getCurrentDesignSize(designWidth, originY);
	getPageManager()->getCurrentScreenSize(configCanvasWidth, configCanvasHeight);

	scrnMidWidth = configCanvasWidth / 2;

	// Check if the screen size matches the theme size if it doesn't then scale
	// the theme accordingly.
	if ((designWidth != configCanvasWidth))
		scrnMidHeight += ((originY * configCanvasWidth) / designWidth);
	else
		scrnMidHeight += originY;

	// Configure the space using the input theme width.
	getSpace()->configureCanvas(
		LcTPixelRect(0, 0, configCanvasWidth, configCanvasHeight),
		LcTPixelPoint(scrnMidWidth, scrnMidHeight),
		0,
		(LcTScalar)designWidth
		);
}

/*-------------------------------------------------------------------------*//**
	Set the current engine configuration
*/
IFX_RETURN_STATUS NdhsCAppUi::setConfiguration(	const char* language,
									const char* displayMode,
									const char* themeName,
									const char* entryPoint_Id)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		if (getPageManager())
		{
			if (getPageManager()->applyConfiguration(language, displayMode, themeName, entryPoint_Id))
				status = IFX_SUCCESS;
		}
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
	Get the current engine configuration
*/
IFX_RETURN_STATUS NdhsCAppUi::getConfiguration(	char* language,
									char* displayMode,
									char* themeName,
									char* entrypointId,
									unsigned int maxStringLen)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	// Protect this section against OOM events - Note that we've made it as
	// unlikely as possible for these to occur, but it is still possible
	// that LcTString::bufUtf8 might allocate, thus we need cleanup stack
	// protection.
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		if (getPageManager())
		{
			strncpy(language, getPageManager()->getCurrentLanguageString(), maxStringLen);
			strncpy(displayMode, getPageManager()->getCurrentScreenModeString(), maxStringLen);
			strncpy(themeName, getPageManager()->getCurrentThemeString(), maxStringLen);
			strncpy(entrypointId, getPageManager()->getCurrentEntryPointId(), maxStringLen);
			status = IFX_SUCCESS;
		}
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
	Process application events based on the time supplied.
*/
IFX_RETURN_STATUS NdhsCAppUi::processTimers(IFX_UINT32 timeUpper, IFX_UINT32 timeLower)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

#ifdef IFX_MEMORYTEST_STARTUP
	// This is different path so we need a different varaible
	// to hold the allocation count
	g_startupTest_allocsUntilFail = g_currentTestProcessTimer++;
	g_testCount++;
#endif

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		// Lock space UI against other threads
		m_uiLock->lock(LcCMutex::EInfinite);

		if (getSpace())
		{
			getSpace()->processTimers(timeUpper, timeLower);
			status = IFX_SUCCESS;
		}

		// Now allow other threads into UI
		m_uiLock->unlock();
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
	Put the application into an idle (not animating) state.
*/
IFX_RETURN_STATUS NdhsCAppUi::setIdle()
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		// Lock space UI against other threads
		m_uiLock->lock(LcCMutex::EInfinite);

		if (getPageManager())
		{
			getPageManager()->setIdle();
			status = IFX_SUCCESS;
		}

		// Now allow other threads into UI
		m_uiLock->unlock();
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
	Resume static animation
*/
IFX_RETURN_STATUS NdhsCAppUi::resumeStaticAnimations()
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		// Lock space UI against other threads
		m_uiLock->lock(LcCMutex::EInfinite);

		if (getPageManager())
		{
			getPageManager()->resumeStaticAnimations();
			status = IFX_SUCCESS;
		}

		// Now allow other threads into UI
		m_uiLock->unlock();
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
	Refresh the display / fields and display.
*/
IFX_RETURN_STATUS NdhsCAppUi::refresh(bool refreshAll)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		// Lock space UI against other threads
		m_uiLock->lock(LcCMutex::EInfinite);

		if (refreshAll)
		{
			// Force all fields to be refreshed.
			getPageManager()->setFieldCacheDirty();
		}

		// Repaint the frame.
		if (getSpace())
		{
			getSpace()->revalidateAll();
			status = IFX_SUCCESS;
		}

		// Now allow other threads into UI
		m_uiLock->unlock();
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
	Signal to the engine that the asynchronous link has completed.
*/
IFX_RETURN_STATUS NdhsCAppUi::asyncLinkComplete()
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		// Lock space UI against other threads
		m_uiLock->lock(LcCMutex::EInfinite);

		// Reset Async blocking
		getPageManager()->getCon()->resetAsyncLinkBlocking();

		// Schedule message now
		getPageManager()->scheduleAsynchronousLaunchComplete();
		status = IFX_SUCCESS;

		// Now allow other threads into UI
		m_uiLock->unlock();
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
	Stop any active animations and delete the canvasses.
*/
IFX_RETURN_STATUS NdhsCAppUi::suspend()
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (!m_suspended)
	{
		volatile int memoryException = 0;

		// Protect this section against OOM events
		LC_CLEANUP_PUSH_FRAME(memoryException);

		if (!memoryException)
		{
			// Lock space UI against other threads
			m_uiLock->lock(LcCMutex::EInfinite);

			getPageManager()->onSuspend();

			if (getSpace())
			{
				if (getSpace()->onSuspend())
					status = IFX_SUCCESS;
			}

			m_suspended = true;

			// Now allow other threads into UI
			m_uiLock->unlock();
		}
		else
		{
			status = memoryException;
			memoryException = 0;
		}

		// Get rid of cleanup frame
		LC_CLEANUP_POP_FRAME(memoryException);
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	Restore the canvasses.
*/
IFX_RETURN_STATUS NdhsCAppUi::resume()
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (m_suspended)
	{
		volatile int memoryException = 0;

		// Protect this section against OOM events
		LC_CLEANUP_PUSH_FRAME(memoryException);

		if (!memoryException)
		{
			// Lock space UI against other threads
			m_uiLock->lock(LcCMutex::EInfinite);

			if (getSpace())
			{
				if (getSpace()->onResume())
					status = IFX_SUCCESS;
			}

			getPageManager()->onResume();

			m_suspended = false;

			// Now allow other threads into UI
			m_uiLock->unlock();
		}
		else
		{
			status = memoryException;
			memoryException = 0;
		}

		// Get rid of cleanup frame
		LC_CLEANUP_POP_FRAME(memoryException);
	}

	return status;
}


/*-------------------------------------------------------------------------*//**
	Handle a key down event
*/
IFX_RETURN_STATUS NdhsCAppUi::keyDown(int c)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	if(m_suspended)
		return IFX_ERROR;

#ifdef IFX_USE_SCRIPTS
	// In case of script execution not in demo mode
	// extenal events are not handled
#ifdef IFX_EXECUTE_DEMOMODE_SCRIPT
	if (getSpace())
	{
		getSpace()->suspendScriptEvents();

		if(getSpace()->scriptEventsActive())
		{
			return IFX_SUCCESS;
		}
	}
#else
	status = IFX_SUCCESS;
	return status;
#endif
#endif

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		// Lock space UI against other threads
		m_uiLock->lock(LcCMutex::EInfinite);

		if (getSpace() && getSpace()->getFocus())
		{
			if (getSpace()->getFocus()->onKeyDown(c))
				status = IFX_SUCCESS;
		}

		// Now allow other threads into UI
		m_uiLock->unlock();
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
	Handle a key up event
*/
IFX_RETURN_STATUS NdhsCAppUi::keyUp(int c)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	if( m_suspended)
		return IFX_ERROR;

#ifdef IFX_USE_SCRIPTS
	// In case of script execution not in demo mode
	// extenal events are not handled
#ifdef IFX_EXECUTE_DEMOMODE_SCRIPT
	if (getSpace())
	{
		getSpace()->suspendScriptEvents();

		if(getSpace()->scriptEventsActive())
		{
			return IFX_SUCCESS;
		}
	}
#else
	status = IFX_SUCCESS;
	return status;
#endif
#endif

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		// Lock space UI against other threads
		m_uiLock->lock(LcCMutex::EInfinite);

		if (getSpace() && getSpace()->getFocus())
		{
			if (getSpace()->getFocus()->onKeyUp(c))
				status = IFX_SUCCESS;
			// NB: -1 is a special code meaning "last key depressed"
		}

		// Now allow other threads into UI
		m_uiLock->unlock();
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
	Handle a mouse down event
*/
IFX_RETURN_STATUS NdhsCAppUi::mouseDown(const LcTPixelPoint& pt)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if( m_suspended)
		return IFX_ERROR;

#ifndef LC_USE_STYLUS
	LC_UNUSED(pt)
#else

#ifdef IFX_USE_SCRIPTS
	// In case of script execution not in demo mode
	// extenal events are not handled
#ifdef IFX_EXECUTE_DEMOMODE_SCRIPT
	if (getSpace())
	{
		getSpace()->suspendScriptEvents();

		if(getSpace()->scriptEventsActive())
		{
			return IFX_SUCCESS;
		}
	}
#else
	status = IFX_SUCCESS;
	return status;
#endif
#endif

	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		// Lock space UI against other threads
		m_uiLock->lock(LcCMutex::EInfinite);

		if (getSpace() && getSpace()->getFocus())
		{
			if (getSpace()->getFocus()->onMouseDown(pt))
				status = IFX_SUCCESS;
		}

		// Now allow other threads into UI
		m_uiLock->unlock();
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

#endif //#ifdef LC_USE_STYLUS

	return status;
}

/*-------------------------------------------------------------------------*//**
	Handle a mouse up event
*/
IFX_RETURN_STATUS NdhsCAppUi::mouseUp(const LcTPixelPoint& pt)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if( m_suspended)
		return IFX_ERROR;

#ifndef LC_USE_STYLUS
	LC_UNUSED(pt)
#else

#ifdef IFX_USE_SCRIPTS
	// In case of script execution not in demo mode
	// extenal events are not handled
#ifdef IFX_EXECUTE_DEMOMODE_SCRIPT
	if (getSpace())
	{
		getSpace()->suspendScriptEvents();

		if(getSpace()->scriptEventsActive())
		{
			return IFX_SUCCESS;
		}
	}
#else
	status = IFX_SUCCESS;
	return status;
#endif
#endif

	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		// Lock space UI against other threads
		m_uiLock->lock(LcCMutex::EInfinite);

		if (getSpace() && getSpace()->getFocus())
		{
			if (getSpace()->getFocus()->onMouseUp(pt))
				status = IFX_SUCCESS;
		}

		// Now allow other threads into UI
		m_uiLock->unlock();
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

#endif //#ifdef LC_USE_STYLUS

	return status;
}

/*-------------------------------------------------------------------------*//**
	Handle a mouse move event
*/
IFX_RETURN_STATUS NdhsCAppUi::mouseMove(const LcTPixelPoint& pt)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if( m_suspended)
		return IFX_ERROR;

#ifndef LC_USE_STYLUS
	LC_UNUSED(pt)
#else

#ifdef IFX_USE_SCRIPTS
	// In case of script execution not in demo mode
	// extenal events are not handled
#ifdef IFX_EXECUTE_DEMOMODE_SCRIPT
	if (getSpace())
	{
		getSpace()->suspendScriptEvents();

		if(getSpace()->scriptEventsActive())
		{
			return IFX_SUCCESS;
		}
	}
#else
	status = IFX_SUCCESS;
	return status;
#endif
#endif

	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		// Lock space UI against other threads
		m_uiLock->lock(LcCMutex::EInfinite);

		if (getSpace() && getSpace()->getFocus())
		{
			if (getSpace()->getFocus()->onMouseMove(pt))
				status = IFX_SUCCESS;
		}

		// Now allow other threads into UI
		m_uiLock->unlock();
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

#endif //#ifdef LC_USE_STYLUS

	return status;
}

/*-------------------------------------------------------------------------*//**
	Delete the existing pages on page stack
*/
IFX_RETURN_STATUS NdhsCAppUi::destroyPageStack()
{
	volatile int memoryException = 0;

	// Set up NDE cleanup stack
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (memoryException == 0)
	{
		getPageManager()->destroyPageStack();
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memoryException);

	if (memoryException != 0)
		return IFX_ERROR;

	return IFX_SUCCESS;
}

/*-------------------------------------------------------------------------*//**
	Save the runtime state (if option supported)
*/
IFX_RETURN_STATUS NdhsCAppUi::saveState(int* abortState)
{
	IFX_RETURN_STATUS status = IFX_SUCCESS;

#ifdef IFX_SERIALIZATION
	volatile int memoryException = 0;

	// Set up NDE cleanup stack
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		if(getPageManager()->saveState(abortState) == false)
			status = IFX_ERROR;
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Clean up NDE cleanup stack
	LC_CLEANUP_POP_FRAME(memoryException);

#endif /* IFX_SERIALIZATION */

	return status;
}

/*-------------------------------------------------------------------------*//**
	Open an entry point
*/
IFX_RETURN_STATUS NdhsCAppUi::openEntryPoint(bool openDefault, const LcTmString& entryPoint)
{
	IFX_RETURN_STATUS status = IFX_SUCCESS;
	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		// Lock space UI against other threads
		m_uiLock->lock(LcCMutex::EInfinite);

		if (getSpace()->isCanvasConstructed() == false)
		{
			getSpace()->constructCanvas();
		}

		// Ask page manager to open entry point
#if !defined(NDHS_JNI_INTERFACE)
		status = getPageManager()->openEntryPoint(openDefault, entryPoint);
		if ((status != IFX_SUCCESS) && (openDefault == true))
		{
#if defined(IFX_USE_SCRIPTS) && !defined(IFX_EXECUTE_DEMOMODE_SCRIPT)
			IFXP_Display_Error_Note((IFX_WCHAR*)L"Invalid theme found while autotesting");
#else
			displayHaltError("No valid themes found", true);
#endif
		}
#endif

		// Now allow other threads into UI
		m_uiLock->unlock();
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCAppUi::doAsyncLaunchComplete()
{
	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		NdhsCMenuCon::doAsyncLaunchComplete();
	}
	else
	{
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);
}

#if defined (IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
	Set time of testing framework.
*/
IFX_RETURN_STATUS NdhsCAppUi::setTestTime(IFX_UINT32 timeUpper, IFX_UINT32 timeLower)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		if (getSpace())
		{
			getSpace()->setTestTime(timeUpper, timeLower);
			status = IFX_SUCCESS;
		}
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
	Get current time of testing framework.
*/
IFX_RETURN_STATUS NdhsCAppUi::getTestTime(IFX_UINT32* timeUpper, IFX_UINT32* timeLower)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		if (getSpace())
		{
			if(getSpace()->getTestTime(timeUpper, timeLower))
				status = IFX_SUCCESS;
		}
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}
#endif

#if defined (IFX_GENERATE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
	Update current time of testing framework.
*/
IFX_RETURN_STATUS NdhsCAppUi::updateTestTime(IFX_UINT32 timeUpper, IFX_UINT32 timeLower)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		if (getSpace())
		{
			getSpace()->updateTestTime(timeUpper, timeLower);
			status = IFX_SUCCESS;
		}
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
	Increment current time of testing framework.
*/
IFX_RETURN_STATUS NdhsCAppUi::incrementTestTime(IFX_UINT32 timeUpper, IFX_UINT32 timeLower)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	volatile int memoryException = 0;

	// Protect this section against OOM events
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (!memoryException)
	{
		if (getSpace())
		{
			getSpace()->incrementTestTime(timeUpper, timeLower);
			status = IFX_SUCCESS;
		}
	}
	else
	{
		status = memoryException;
		memoryException = 0;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}
#endif
