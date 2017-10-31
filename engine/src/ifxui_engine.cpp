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

/***************************************************************************
*	This file implements the UI Engine Control API exposed to applications.
****************************************************************************/

#include "inflexionui/engine/inc/LcAll.h"

#include "inflexionui/engine/ifxui_engine.h"
#include "inflexionui/engine/ifxui_engine_porting.h"
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif

// Internal implementation of IFXE_FILE handles
typedef struct
{
#ifdef IFX_USE_PLATFORM_FILES
	IFXP_FILE			platformFile;
#endif // def IFX_USE_PLATFORM_FILES

#if !defined(NDHS_PACKAGER) && !defined(NDHS_INTEGRATOR)
	LcTRomFileEntry*	romFile;
	IFX_UINT32			offset;
#endif

} IFXE_INTERNAL_FILE;

// Application pointer
NdhsCAppUi* g_appui;

#if defined (IFX_USE_SCRIPTS)
// Script excutor pointer
NdhsCScriptExecutor* g_scriptExecutor;
IFX_BOOL             at_restart_engine_pending = IFX_FALSE;
IFX_RETURN_STATUS    AT_Restart_Engine (void);
#endif

#if defined (IFX_GENERATE_SCRIPTS)
// Script generator pointer
NdhsCScriptGenerator* g_scriptGenerator;
#endif

// IFX Display Information
IFX_DISPLAY* g_ifxDisplay;

// Runtime state
IFXE_STATE g_currentState = IFXE_STATE_UNINITIALIZED;

// handler for 'restart engine' return
void EL_onRestartEvent(void);
static IFX_RETURN_STATUS EL_getBestFit(IFXE_CONFIGURATION* config);

/*-------------------------------------------------------------------------*//**
	Initialize library and pass in queue to receive notifications.
*/
IFX_RETURN_STATUS IFXE_Initialize(IFXE_CONFIGURATION* config)
{
	IFX_RETURN_STATUS status = IFX_SUCCESS;
	volatile int memoryException = 0;

	bool restoreState=false;

#ifdef IFX_SERIALIZATION
	restoreState=config->restoreState==1;
#endif /* IFX_SERIALIZATION */
	if (g_currentState == IFXE_STATE_UNINITIALIZED)
	{
		// If engine is created already, make sure to free the existing instance of g_appui
		if (g_appui)
		{
			delete g_appui;
			g_appui = NULL;
		}

		// Initialize the porting layer
		status = IFXP_Initialize();

#ifndef NDHS_OMIT_ENVIRONMENT_OWNERSHIP
		// Create a singleton instance of the environment object
		if (status == IFX_SUCCESS)
		{
			LcCEnvironment::create();
			if (LcCEnvironment::get() == NULL)
			{
				status = IFX_ERROR;
			}
		}
#endif // #ifndefNDHS_OMIT_ENVIRONMENT_OWNERSHIP

		if (status == IFX_SUCCESS)
		{
			g_currentState = IFXE_STATE_INITIALIZED;
			(void)IFXP_Runtime_State_Changed(IFXE_STATE_INITIALIZED);
		}
	}

#if defined (IFX_USE_SCRIPTS)

	// Setting up batch execution.
#ifndef IFX_EXECUTE_DEMOMODE_SCRIPT
	bool batchFileFound = false;
#endif
	bool scriptFileFound = false;

	// Set up NDE cleanup stack
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (memoryException == 0)
	{
		LcTaOwner<NdhsCScriptExecutor> scriptExecutor = NdhsCScriptExecutor::create();

		g_scriptExecutor = scriptExecutor.release();

		if (g_scriptExecutor == NULL)
		{
			status = IFX_ERROR;
			IFXP_Display_Error_Note((IFX_WCHAR*)L"Batch/Script executor failed.");
		}

		if(status == IFX_SUCCESS)
		{
#ifndef IFX_EXECUTE_DEMOMODE_SCRIPT
			batchFileFound = g_scriptExecutor->startBatchExecution();

			if(!batchFileFound)
#endif
			scriptFileFound = g_scriptExecutor->startScriptExecution();

#ifndef IFX_EXECUTE_DEMOMODE_SCRIPT
			if (!batchFileFound && !scriptFileFound)
			{
				status = IFX_ERROR;
			}
#endif
		}
	}
	else
	{
		// Some error has occurred.
		status = IFX_ERROR;
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memoryException);

#endif

#if defined (IFX_GENERATE_SCRIPTS)

	// Setting up script generatorion

	// Set up NDE cleanup stack
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if (memoryException == 0)
	{
		LcTaOwner<NdhsCScriptGenerator> scriptGenerator = NdhsCScriptGenerator::create();

		g_scriptGenerator = scriptGenerator.release();

		if (g_scriptGenerator == NULL)
		{
			status = IFX_ERROR;
			IFXP_Display_Error_Note((IFX_WCHAR*)L"Script generator failed.");
		}
	}
	else
	{
		// Some error has occurred.
		status = IFX_ERROR;
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memoryException);

#endif

	// Validate the display configuration and pick a default if necessary.
	if (status == IFX_SUCCESS && config != NULL)
	{
		status = EL_getBestFit(config);

		// The Porting layer must have its display setup before creating the Engine instance.
		// Set the porting layer display mode.
		if (status == IFX_SUCCESS)
		{
			status = IFXP_Display_Set_Mode(config->left, config->top, config->width, config->height, config->display_mode);
		}

		if (g_currentState == IFXE_STATE_INITIALIZED)
		{
			// Create the application.
			if (status == IFX_SUCCESS)
			{
				// Set up NDE cleanup stack
				LC_CLEANUP_PUSH_FRAME(memoryException);

				if (memoryException == 0)
				{
						// Create interactive application UI hosted in space
						LcTaOwner<NdhsCAppUi> appui = NdhsCAppUi::create(config->language,
																		 config->display_mode,
																		 config->package_name, restoreState);
						g_appui = appui.release();
				}
				else
				{
					// Some catastrophic OOM error has occurred.
					status = IFX_ERROR;
				}

				if (g_appui == NULL)
					status = IFX_ERROR;

				// Finished with cleanup stack
				LC_CLEANUP_POP_FRAME(memoryException);
			}

#ifndef NDHS_OMIT_ENVIRONMENT_OWNERSHIP
			// Clean up on error
			if (status != IFX_SUCCESS)
			{
				// Remove the singleton environment object
				LcCEnvironment::free();
			}
#endif // #ifndef NDHS_OMIT_ENVIRONMENT_OWNERSHIP
		}
		else if (status == IFX_SUCCESS)
		{
			// If we are already initialized, then destroy the existing stack.
			if (g_currentState == IFXE_STATE_INACTIVE_ASYNC_LINK)
				return IFX_ERROR_ASYNC_BLOCKING;

			// Delete the exisiting page stack
			g_appui->destroyPageStack();
		}
	}

	if (status == IFX_SUCCESS && config != NULL)
	{
		// Open the theme entry point node.
		if (!restoreState && g_appui)
		{
			status = g_appui->openEntryPoint(true, config->entrypoint_id);
		}
		else
		{
			status = IFX_SUCCESS;
		}

		if (status == IFX_SUCCESS)
		{
			g_currentState = IFXE_STATE_ACTIVE;
			(void)IFXP_Runtime_State_Changed(IFXE_STATE_ACTIVE);
			status = IFXP_Display_Get_Info(&g_ifxDisplay);
		}
#if defined (IFX_USE_SCRIPTS) && !defined(IFX_EXECUTE_DEMOMODE_SCRIPT)
		else
		{
			status = IFX_SUCCESS;
			status = IFXP_Display_Get_Info(&g_ifxDisplay);

			// Set up NDE cleanup stack
			LC_CLEANUP_PUSH_FRAME(memoryException);

			g_scriptExecutor->testCaseLoadFailed();

			// Finished with cleanup stack
			LC_CLEANUP_POP_FRAME(memoryException);

			AT_Set_Engine_Restart();
		}
#endif
	}
	else
	{
		if (g_currentState == IFXE_STATE_ACTIVE)
		{
			// Delete the existing page stack
			g_appui->destroyPageStack();

			// Force redraw to clear screen
			g_appui->refresh(false);

			delete g_appui;
			g_appui = NULL;

			// note that we're inactive
			g_currentState = IFXE_STATE_INITIALIZED;
		}
	}

	return status;
}


/*-------------------------------------------------------------------------*//**
	Terminate the Inflexion UI Engine.
*/
IFX_RETURN_STATUS IFXE_Shutdown(void)
{
	IFX_RETURN_STATUS status = IFX_SUCCESS;

	if (g_currentState != IFXE_STATE_UNINITIALIZED)
	{
		volatile int memoryException = 0;

		// Set up NDE cleanup stack
		LC_CLEANUP_PUSH_FRAME(memoryException);

		if (memoryException == 0)
		{
			// Force a 'suspend' notification if we're currently active
			if (g_currentState == IFXE_STATE_ACTIVE)
			{
				g_currentState = IFXE_STATE_INACTIVE_SUSPENDED;
				(void)IFXP_Runtime_State_Changed(IFXE_STATE_INACTIVE_SUSPENDED);
			}

#if defined (IFX_USE_SCRIPTS)
			if(g_scriptExecutor)
			{
				delete g_scriptExecutor;
				g_scriptExecutor = NULL;
			}
#endif

#if defined (IFX_GENERATE_SCRIPTS)
			if(g_scriptGenerator)
			{
				delete g_scriptGenerator;
				g_scriptGenerator = NULL;
			}
#endif

			if (g_appui)
			{
				delete g_appui;
				g_appui = NULL;
			}
		}
		else
		{
			status = IFX_ERROR;
		}

		// Finished with cleanup stack
		LC_CLEANUP_POP_FRAME(memoryException);

#ifndef NDHS_OMIT_ENVIRONMENT_OWNERSHIP
		// Remove the singleton environment object
		LcCEnvironment::free();
#endif // #ifndef NDHS_OMIT_ENVIRONMENT_OWNERSHIP

		// Now go to the shut down state
		g_currentState = IFXE_STATE_UNINITIALIZED;
		(void)IFXP_Runtime_State_Changed(IFXE_STATE_UNINITIALIZED);
	}

	// Porting layer is independent
	status = IFXP_Shutdown();

	if (status == IFX_ERROR)
	{
		// Reset, there is some critical error
		// don't let someone initialize engine
		// again
		g_currentState = IFXE_STATE_INITIALIZED;
	}

	return status;
}


/*-------------------------------------------------------------------------*//**
  Change the current display mode, theme package name and/or language to be
  used by the Engine.

  Any combination of display_mode, package_name or language may
  be specified, using NULL where a feature is not required to be modified.
*/
IFX_RETURN_STATUS IFXE_Set_Configuration(IFXE_CONFIGURATION* config)
{
	IFX_RETURN_STATUS status = IFX_SUCCESS;

	if (config == NULL)
		return IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING
		|| g_currentState == IFXE_STATE_INACTIVE_SUSPENDED)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_currentState == IFXE_STATE_INACTIVE_ASYNC_LINK)
		return IFX_ERROR_ASYNC_BLOCKING;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	// Apply the configuration
	if (g_appui)
	{
		IFXE_CONFIGURATION currentConfig;
		status = IFXE_Get_Configuration(&currentConfig);

		// If the display mode has been changed, then apply it
		if ((status == IFX_SUCCESS)
				&& (lc_strlen(config->display_mode) > 0)
				&& (lc_strcmpi(config->display_mode, currentConfig.display_mode) != 0))
		{
			status = EL_getBestFit(config);

			if (status == IFX_SUCCESS)
			{
				status = IFXP_Display_Set_Mode(config->left, config->top, config->width, config->height, config->display_mode);
			}

			// Retrieve the display information.
			if (status == IFX_SUCCESS)
			{
				status = IFXP_Display_Get_Info(&g_ifxDisplay);
			}
		}

		if (status == IFX_SUCCESS)
		{
			status = g_appui->setConfiguration(config->language, config->display_mode, config->package_name,config->entrypoint_id);

			if (status == IFX_ERROR_RESTART)
			{
				EL_onRestartEvent();
				status = IFX_ERROR;
			}
		}
	}
	else
	{
		status = IFX_ERROR;
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	Retrieve the current configuration.
*/
IFX_RETURN_STATUS IFXE_Get_Configuration(IFXE_CONFIGURATION* config)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (config == NULL)
		return IFX_ERROR;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (config && g_appui)
	{
		memset(config, 0, sizeof(IFXE_CONFIGURATION));

		config->left = g_ifxDisplay->offsetX;
		config->top  = g_ifxDisplay->offsetY;

		status = g_appui->getConfiguration(config->language,
										config->display_mode,
										config->package_name,
										config->entrypoint_id,
										IFX_MAX_CONFIG_STRING_LENGTH);
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	Call when it is required to save state of the engine and resume in future.
*/
IFX_RETURN_STATUS IFXE_Save_State (int *abortSave)
{
	IFX_RETURN_STATUS status= IFX_ERROR;

	if(g_appui)
	{
		if (g_appui->saveState(abortSave) == IFX_SUCCESS)
			status = IFX_SUCCESS;
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	Call when a key is depressed.
	The code should be an IFX_KEY_CODE (see ifxui_defs.h), a portable key code or
	a device-specific scan code.
*/
IFX_RETURN_STATUS IFXE_Key_Down(IFX_INT32 code)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING
		|| g_currentState == IFXE_STATE_INACTIVE_SUSPENDED)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_currentState == IFXE_STATE_INACTIVE_ASYNC_LINK)
		return IFX_ERROR_ASYNC_BLOCKING;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (g_appui)
	{
		// If async blocking message is scheduled do the transition
		if (g_appui->isAsyncBlockingMessageScheduled())
		{
			g_appui->doAsyncLaunchComplete();
		}
		else
		{
			// If there is a blocking module call then return an error.
			if (g_appui->isAsyncLinkBlocking())
				return IFX_ERROR_ASYNC_BLOCKING;
		}

		status = g_appui->keyDown(code);

		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
		else if (g_appui->isAsyncLinkBlocking() && (g_currentState != IFXE_STATE_INACTIVE_ASYNC_LINK))
		{
			g_currentState = IFXE_STATE_INACTIVE_ASYNC_LINK;
			(void)IFXP_Runtime_State_Changed(IFXE_STATE_INACTIVE_ASYNC_LINK);
		}
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	Call when a key is released.
	The code should be an IFX_KEY_CODE (see ifxui_defs.h), a portable key code or
	a device-specific scan code.
*/
IFX_RETURN_STATUS IFXE_Key_Up(IFX_INT32 code)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING
		|| g_currentState == IFXE_STATE_INACTIVE_SUSPENDED)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_currentState == IFXE_STATE_INACTIVE_ASYNC_LINK)
		return IFX_ERROR_ASYNC_BLOCKING;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (g_appui)
	{
		// If async blocking message is scheduled do the transition
		if (g_appui->isAsyncBlockingMessageScheduled())
		{
			g_appui->doAsyncLaunchComplete();
		}
		else
		{
			// If there is a blocking module call then return an error.
			if (g_appui->isAsyncLinkBlocking())
				return IFX_ERROR_ASYNC_BLOCKING;
		}

		status = g_appui->keyUp(code);

		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
		else if (g_appui->isAsyncLinkBlocking() && (g_currentState != IFXE_STATE_INACTIVE_ASYNC_LINK))
		{
			g_currentState = IFXE_STATE_INACTIVE_ASYNC_LINK;
			(void)IFXP_Runtime_State_Changed(IFXE_STATE_INACTIVE_ASYNC_LINK);
		}
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS IFXE_Touch_Down(IFX_UINT32 touchId, IFX_INT32 x, IFX_INT32 y)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

#ifndef IFX_USE_STYLUS
	LC_UNUSED(x)
	LC_UNUSED(y)
#endif

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING
		|| g_currentState == IFXE_STATE_INACTIVE_SUSPENDED)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_currentState == IFXE_STATE_INACTIVE_ASYNC_LINK)
		return IFX_ERROR_ASYNC_BLOCKING;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (touchId != 0)
		return IFX_ERROR;

#ifdef IFX_USE_STYLUS
	if (g_appui)
	{
		// If async blocking message is scheduled do the transition
		if (g_appui->isAsyncBlockingMessageScheduled())
		{
			g_appui->doAsyncLaunchComplete();
		}
		else
		{
			// If there is a blocking module call then return an error.
			if (g_appui->isAsyncLinkBlocking())
				return IFX_ERROR_ASYNC_BLOCKING;
		}

		x -= g_ifxDisplay->offsetX;
		y -= g_ifxDisplay->offsetY;

		status = g_appui->mouseDown(LcTPixelPoint(x, y));

		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
		else if (g_appui->isAsyncLinkBlocking() && (g_currentState != IFXE_STATE_INACTIVE_ASYNC_LINK))
		{
			g_currentState = IFXE_STATE_INACTIVE_ASYNC_LINK;
			(void)IFXP_Runtime_State_Changed(IFXE_STATE_INACTIVE_ASYNC_LINK);
		}
	}
#endif

	return status;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS IFXE_Touch_Move(IFX_UINT32 touchId, IFX_INT32 x, IFX_INT32 y)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

#ifndef IFX_USE_STYLUS
	LC_UNUSED(x)
	LC_UNUSED(y)
#endif

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING
		|| g_currentState == IFXE_STATE_INACTIVE_SUSPENDED)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_currentState == IFXE_STATE_INACTIVE_ASYNC_LINK)
		return IFX_ERROR_ASYNC_BLOCKING;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (touchId != 0)
		return IFX_ERROR;

#ifdef IFX_USE_STYLUS
	if (g_appui)
	{
		// If async blocking message is scheduled do the transition
		if (g_appui->isAsyncBlockingMessageScheduled())
		{
			g_appui->doAsyncLaunchComplete();
		}
		else
		{
			// If there is a blocking module call then return an error.
			if (g_appui->isAsyncLinkBlocking())
				return IFX_ERROR_ASYNC_BLOCKING;
		}

		x -= g_ifxDisplay->offsetX;
		y -= g_ifxDisplay->offsetY;

		status = g_appui->mouseMove(LcTPixelPoint(x, y));

		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
		else if (g_appui->isAsyncLinkBlocking() && (g_currentState != IFXE_STATE_INACTIVE_ASYNC_LINK))
		{
			g_currentState = IFXE_STATE_INACTIVE_ASYNC_LINK;
			(void)IFXP_Runtime_State_Changed(IFXE_STATE_INACTIVE_ASYNC_LINK);
		}
	}
#endif

	return status;
}


/*-------------------------------------------------------------------------*//**
	Call when mouse/stylus events occur.
*/
IFX_RETURN_STATUS IFXE_Touch_Up(IFX_UINT32 touchId, IFX_INT32 x, IFX_INT32 y)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

#ifndef IFX_USE_STYLUS
	LC_UNUSED(x)
	LC_UNUSED(y)
#endif

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING
		|| g_currentState == IFXE_STATE_INACTIVE_SUSPENDED)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_currentState == IFXE_STATE_INACTIVE_ASYNC_LINK)
		return IFX_ERROR_ASYNC_BLOCKING;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (touchId != 0)
		return IFX_ERROR;

#ifdef IFX_USE_STYLUS
	if (g_appui)
	{
		// If async blocking message is scheduled do the transition
		if (g_appui->isAsyncBlockingMessageScheduled())
		{
			g_appui->doAsyncLaunchComplete();
		}
		else
		{
			// If there is a blocking module call then return an error.
			if (g_appui->isAsyncLinkBlocking())
				return IFX_ERROR_ASYNC_BLOCKING;
		}

		x -= g_ifxDisplay->offsetX;
		y -= g_ifxDisplay->offsetY;

		status = g_appui->mouseUp(LcTPixelPoint(x, y));

		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
		else if (g_appui->isAsyncLinkBlocking() && (g_currentState != IFXE_STATE_INACTIVE_ASYNC_LINK))
		{
			g_currentState = IFXE_STATE_INACTIVE_ASYNC_LINK;
			(void)IFXP_Runtime_State_Changed(IFXE_STATE_INACTIVE_ASYNC_LINK);
		}
	}
#endif

	return status;
}

/*-------------------------------------------------------------------------*//**
	Call to drive the engine. All scheduled events up to time will be processed.
*/
IFX_RETURN_STATUS IFXE_Process_Timers(IFX_UINT32 time_upper, IFX_UINT32 time_lower)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

#if defined (IFX_USE_SCRIPTS)
	if (at_restart_engine_pending == IFX_TRUE)
	{
		at_restart_engine_pending = IFX_FALSE;
		status = AT_Restart_Engine();

		if (status == IFX_ERROR)
		{
			//if could not restart engine then stop testing
			IFXE_Shutdown();
			return IFX_ERROR;
		}
	}
#endif

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING)
	{
		// OK - tear down the engine, and start it up again

		// We'll need to cache the current configuration
		IFXE_CONFIGURATION cachedConfig;
		IFXE_Get_Configuration(&cachedConfig);

		// Now destroy the engine...
		IFXE_Shutdown();

		// ...and regenerate it
		status = IFXE_Initialize(&cachedConfig);
	}
	else
	{
		if (g_appui)
		{
			status = g_appui->processTimers(time_upper, time_lower);

			if (status == IFX_ERROR_RESTART
#ifdef IFX_MEMORYTEST_DYNAMIC
				|| status == -1
#endif
				)
			{
				EL_onRestartEvent();
				status = IFX_ERROR;
			}
			else if (g_appui->isAsyncLinkBlocking() && (g_currentState != IFXE_STATE_INACTIVE_ASYNC_LINK))
			{
				g_currentState = IFXE_STATE_INACTIVE_ASYNC_LINK;
				(void)IFXP_Runtime_State_Changed(IFXE_STATE_INACTIVE_ASYNC_LINK);
			}
			else if ((g_appui->isAsyncLinkBlocking() == false) && (g_currentState == IFXE_STATE_INACTIVE_ASYNC_LINK))
			{
				// An async link launch failed - return us to active duty
				g_currentState = IFXE_STATE_ACTIVE;
				(void)IFXP_Runtime_State_Changed(IFXE_STATE_ACTIVE);
			}
		}
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	Instruct the engine to go stop all running animations and go to a static
	state.
*/
IFX_RETURN_STATUS IFXE_Set_Idle(void)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING
		|| g_currentState == IFXE_STATE_INACTIVE_SUSPENDED)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_currentState == IFXE_STATE_INACTIVE_ASYNC_LINK)
		return IFX_ERROR_ASYNC_BLOCKING;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (g_appui)
	{
		status = g_appui->setIdle();

		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	Instruct the engine to resume static animation
*/
IFX_RETURN_STATUS IFXE_Resume_Static_Animations(void)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING
		|| g_currentState == IFXE_STATE_INACTIVE_SUSPENDED)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_currentState == IFXE_STATE_INACTIVE_ASYNC_LINK)
		return IFX_ERROR_ASYNC_BLOCKING;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (g_appui)
	{
		status = g_appui->resumeStaticAnimations();

		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	Instruct the engine to repaint the whole display.
	This shall not refresh field data.
*/
IFX_RETURN_STATUS IFXE_Refresh_Display(void)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING)
		return IFX_ERROR_ENGINE_NOT_READY;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (g_appui)
	{
		status = g_appui->refresh(false);

		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
	}

	return status;
}


/*-------------------------------------------------------------------------*//**
	Instruct the engine to repaint the whole display.
	This shall refresh all field data.
*/
IFX_RETURN_STATUS IFXE_Refresh_All(void)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING
		|| g_currentState == IFXE_STATE_INACTIVE_SUSPENDED)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_currentState == IFXE_STATE_INACTIVE_ASYNC_LINK)
		return IFX_ERROR_ASYNC_BLOCKING;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (g_appui)
	{
		status = g_appui->refresh(true);

		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
	}

	return status;
}


/*-------------------------------------------------------------------------*//**
	Inform the engine that an asynchronous link has completed.
	This shall repaint the whole screen.
*/
IFX_RETURN_STATUS IFXE_Async_Link_Complete(void)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING)
		return IFX_ERROR_ENGINE_NOT_READY;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (g_appui)
	{
		status = g_appui->asyncLinkComplete();

		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
		else if (g_currentState != IFXE_STATE_INACTIVE_SUSPENDED)
		{
			g_currentState = IFXE_STATE_ACTIVE;
			(void)IFXP_Runtime_State_Changed(IFXE_STATE_ACTIVE);
		}
	}

	return status;
}


/*-------------------------------------------------------------------------*//**
	This shall stop any animations and free the foreground and background
	canvasses.
*/
IFX_RETURN_STATUS IFXE_Suspend(void)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING
		|| g_currentState == IFXE_STATE_INACTIVE_SUSPENDED)
		return IFX_ERROR_ENGINE_NOT_READY;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (g_appui)
	{
		status = g_appui->suspend();

		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
		else
		{
			g_currentState = IFXE_STATE_INACTIVE_SUSPENDED;
			(void)IFXP_Runtime_State_Changed(IFXE_STATE_INACTIVE_SUSPENDED);
		}
	}

	return status;
}


/*-------------------------------------------------------------------------*//**
	This shall restore the foreground and background canvasses and repaint
	the screen.
*/
IFX_RETURN_STATUS IFXE_Resume(void)
{
	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING)
		return IFX_ERROR_ENGINE_NOT_READY;

	if ((g_currentState == IFXE_STATE_UNINITIALIZED)
		|| (g_currentState == IFXE_STATE_INITIALIZED))
		return IFX_ERROR;

	if (g_appui)
	{
		status = g_appui->resume();
		if (status == IFX_ERROR_RESTART)
		{
			EL_onRestartEvent();
			status = IFX_ERROR;
		}
		else
		{
			g_currentState = IFXE_STATE_ACTIVE;
			(void)IFXP_Runtime_State_Changed(IFXE_STATE_ACTIVE);
		}
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	Prepare Engine for restarting on a catastrophic error.
*/
void EL_onRestartEvent(void)
{
	// Prevent modules from calling in to the engine via the IFXI api
	g_appui->setCallbacksEnabled(false);

	// Note that the IFXE api is also unavailable for now
	g_currentState = IFXE_STATE_INACTIVE_RESTARTING;
	(void)IFXP_Runtime_State_Changed(IFXE_STATE_INACTIVE_RESTARTING);

	// Schedule a timeout event - we'll tear down and reload the engine
	// on the next process timers event
	IFX_UINT32 currentTimeLower;
	IFX_UINT32 currentTimeUpper;
	IFXP_Timer_Get_Current_Time(&currentTimeUpper, &currentTimeLower);
	IFXP_Timer_Schedule(currentTimeUpper, currentTimeLower);
}

/*-------------------------------------------------------------------------*//**
	Take the engine configuration parameter and verify the presence of
	the requested display mode. If it is not present then apply a best
	fit based on the widht and height supplied. If this fails then use
	the defaultMode flag in the display list. Finally if this is not
	present then fall back to the first display mode in the list.
	The configuration parameter will be updated with the chosen
	displayMode string and the widht and height integer values.
*/
static IFX_RETURN_STATUS EL_getBestFit(IFXE_CONFIGURATION* config)
{
	IFX_RETURN_STATUS status = IFX_SUCCESS;
	volatile int memoryException = 0;

	// Set up NDE cleanup stack
	LC_CLEANUP_PUSH_FRAME(memoryException);

	if(memoryException == 0)
	{
		LcTaString err;

		// Store the best fit settings
		LcTaString bestName = "";
		int bestWidth       = -1;
		int bestHeight      = -1;

		// Store the requested settings
		LcTaString reqName  = config->display_mode;
		int reqWidth        = config->width;
		int reqHeight       = config->height;

		// Store the default settings if found
		LcTaString defName  = "";
		int defWidth        = -1;
		int defHeight       = -1;

		// Set the directory slash separators to the non default if required.
		LcTaString localPath = NdhsCMenuCon::findAppPath() + NDHS_APP_SETTINGS_FILENAME;
		#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
			localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
		#endif

		LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(localPath, err);
		if (root)
		{
			LcTaString guid = "";

			// Get the displays section

			LcCXmlElem* displays = root->find("displays");
			if (displays)
			{
				LcCXmlElem* display = displays->getFirstChild();

				for (; display; display = display->getNext())
				{
					if (display->getName().compareNoCase("display") == 0)
					{
						LcTaString displayName = display->getAttr("name").toLower();
						LcTaString defaultMode = display->getAttr("defaultMode").toLower();

						if (displayName.length() != 0)
						{
							LcCXmlElem* displayMode = display->getFirstChild();

							for (; displayMode; displayMode = displayMode->getNext())
							{
								if (displayMode->getName().compareNoCase("mode") == 0)
								{
									LcTaString name = displayMode->getAttr("name").toLower();
									int width		= displayMode->getAttr("width", "-1").toInt();
									int height		= displayMode->getAttr("height", "-1").toInt();

									if (!name.isEmpty())
									{
										if (name.compareNoCase(reqName) == 0)
										{
											// The requested name is present, use it.
											bestName   = name;
											bestWidth  = width;
											bestHeight = height;

											break;
										}

										if (defaultMode.compareNoCase(name) == 0 || (defName.length() == 0))
										{
											// Save the default setting
											// (first in list, or defaultMode if it exists).
											defName   = name;
											defWidth  = width;
											defHeight = height;
										}

										if ((reqWidth > 0) && (reqHeight > 0))
										{
											if (width <= reqWidth && height <= reqHeight)
											{
												if ((bestWidth * bestHeight) < (width * height))
												{
													bestName   = name;
													bestWidth  = width;
													bestHeight = height;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// Work out what display mode we should be using
		if (bestName.length() != 0 && bestWidth > 0 && bestHeight > 0)
		{
			config->width  = bestWidth;
			config->height = bestHeight;
			lc_strcpy(config->display_mode, bestName.bufUtf8());
		}
		else if (defName.length() != 0 && defWidth > 0 && defHeight > 0)
		{
			config->width  = defWidth;
			config->height = defHeight;
			lc_strcpy(config->display_mode, defName.bufUtf8());
		}
		else if (!root)
		{
			IFXP_Display_Error_Note((IFX_WCHAR*)L"Unable to find settings.xml - please verify that a theme is present.");
			status = IFX_ERROR;
		}
		else
		{
			IFXP_Display_Error_Note((IFX_WCHAR*)L"Cannot find a suitable display mode.");
			status = IFX_ERROR;
		}

		// If the configuration requested it, then center horizontally
		if (config->left == IFX_CENTER_DISPLAY)
		{
			if (reqWidth > 0)
			{
				config->left = (reqWidth - config->width) / 2;
			}
			else
			{
				config->left = 0;
			}
		}

		// If the configuration requested it, then center vertically
		if (config->top == IFX_CENTER_DISPLAY)
		{
			if (reqHeight > 0)
			{
				config->top = (reqHeight - config->height) / 2;
			}
			else
			{
				config->top = 0;
			}
		}
	} // if(memoryException == 0)
	else
	{
		// Some catastrophic OOM error has occurred.
		status = IFX_ERROR;
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memoryException);

	return status;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS IFXE_File_Open(IFXE_FILE *handle, const char *file_name)
{
	IFX_RETURN_STATUS retVal = IFX_ERROR;

	if (file_name)
	{
		IFXE_INTERNAL_FILE* fileInt = NULL;

		{
			unsigned int fileNameLen = lc_strlen(file_name);

			if (fileNameLen <= IFX_MAX_FILE_NAME_LEN)
			{
				bool isRomFile = false;

				if (fileNameLen > (IFX_PLAT_DRIVE_SEP_POS + 1))
				{
					isRomFile = file_name[IFX_PLAT_DRIVE_SEP_POS] == IFX_PLAT_DRIVE_SEP_CHAR && lc_toupper(file_name[0]) == IFX_ROM_FILE_DRIVE_LETTER;
				}

				if (isRomFile)
				{
#ifdef IFX_USE_ROM_FILES

					fileInt = (IFXE_INTERNAL_FILE*)LcGetRomFile(g_romFileCount, g_romFileTable, file_name);

					if (fileInt && fileInt->romFile)
					{
						fileInt->offset = 0;
						retVal = IFX_SUCCESS;
					}
#endif // IFX_USE_ROM_FILES
				}
				else
				{
#ifdef IFX_USE_PLATFORM_FILES
					fileInt = (IFXE_INTERNAL_FILE*)IFXE_AllocateMemoryUnsafe(sizeof(IFXE_INTERNAL_FILE));

					if (fileInt)
					{
						fileInt->platformFile = NULL;

	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
						char validatedFileName[IFX_MAX_FILE_NAME_LEN];

						const char* inputPtr = file_name;
						char* outputPtr = validatedFileName;
						bool complete = false;

						while (!complete)
						{
							char inputChar = *inputPtr++;

							if (NDHS_DIR_SEP_CHAR == inputChar)
								*outputPtr++ = NDHS_PLAT_DIR_SEP_CHAR;
							else
								*outputPtr++ = inputChar;

								complete = inputChar == '\0';
							}
	#else
							const char* validatedFileName = file_name;
	#endif
							retVal = IFXP_File_Open(&(fileInt->platformFile), validatedFileName);
						}
#endif // def IFX_USE_PLATFORM_FILES
				}
			}
		}

		if (IFX_SUCCESS == retVal)
		{
			*handle = fileInt;
		}
		else
		{
			IFXE_FreeMemoryUnsafe(fileInt);
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS IFXE_File_Read(IFXE_FILE handle, void *addr, IFX_UINT32 size, IFX_UINT32 *bytes_read)
{
	IFX_RETURN_STATUS retVal = IFX_ERROR;

	IFXE_INTERNAL_FILE* fileInt = (IFXE_INTERNAL_FILE*)handle;

	if (fileInt)
	{
#ifdef IFX_USE_PLATFORM_FILES
		if (fileInt->platformFile)
		{
			retVal = IFXP_File_Read(fileInt->platformFile, addr, size, bytes_read);
		}
		else
#endif // def IFX_USE_PLATFORM_FILES

#if !defined(NDHS_PACKAGER) && !defined(NDHS_INTEGRATOR)
		if (fileInt->romFile)
		{
			IFX_UINT32 fileSizeRemaining = fileInt->romFile->length > fileInt->offset ? fileInt->romFile->length - fileInt->offset : 0;
			IFX_UINT32 bytesToRead = min(size, fileSizeRemaining);

			if (bytesToRead > 0)
			{
				memcpy(addr, (fileInt->romFile->data + fileInt->offset), bytesToRead);
				fileInt->offset += bytesToRead;
				retVal = IFX_SUCCESS;
			}

			if (bytes_read)
			{
				*bytes_read = bytesToRead;
			}
		}
		else
#endif
		{
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS IFXE_File_Seek(IFXE_FILE handle, IFX_INT32 offset, IFX_SEEK seek_mode)
{
	IFX_RETURN_STATUS retVal = IFX_ERROR;

	IFXE_INTERNAL_FILE* fileInt = (IFXE_INTERNAL_FILE*)handle;

	if (fileInt)
	{
#ifdef IFX_USE_PLATFORM_FILES
		if (fileInt->platformFile)
		{
			retVal = IFXP_File_Seek(fileInt->platformFile, offset, seek_mode);
		}
		else
#endif // def IFX_USE_PLATFORM_FILES

#if !defined(NDHS_PACKAGER) && !defined(NDHS_INTEGRATOR)
		if (fileInt->romFile)
		{
			switch (seek_mode)
			{
				case IFX_SEEK_SET:
				{
					if (offset >= 0 && (IFX_UINT32)offset < fileInt->romFile->length)
					{
						 fileInt->offset = offset;
						 retVal = IFX_SUCCESS;
					}
				}
				break;

				case IFX_SEEK_CUR:
				{
					if (offset + fileInt->offset < fileInt->romFile->length && (IFX_INT32)(offset + fileInt->offset) >= 0)
					{
						 fileInt->offset += offset;
						 retVal = IFX_SUCCESS;
					}
				}
				break;

				case IFX_SEEK_END:
				{
					if (offset + fileInt->romFile->length > 0 && offset <= 0)
					{
						 fileInt->offset = fileInt->romFile->length - offset;
						 retVal = IFX_SUCCESS;
					}
				}
				break;

				default:
					// No action
					break;
			}
		}
		else
#endif
		{
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS IFXE_File_Size(IFXE_FILE handle, IFX_UINT32 *file_size)
{
	IFX_RETURN_STATUS retVal = IFX_ERROR;

	IFXE_INTERNAL_FILE* fileInt = (IFXE_INTERNAL_FILE*)handle;

	if (fileInt)
	{
#ifdef IFX_USE_PLATFORM_FILES
		if (fileInt->platformFile)
		{
			retVal = IFXP_File_Size(fileInt->platformFile, file_size);
		}
		else
#endif // def IFX_USE_PLATFORM_FILES

#if !defined(NDHS_PACKAGER) && !defined(NDHS_INTEGRATOR)
		if (fileInt->romFile && file_size)
		{
			*file_size = fileInt->romFile->length;
			retVal = IFX_SUCCESS;
		}
		else
#endif
		{
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS IFXE_File_Close(IFXE_FILE handle)
{
	IFX_RETURN_STATUS retVal = IFX_ERROR;

	IFXE_INTERNAL_FILE* fileInt = (IFXE_INTERNAL_FILE*)handle;

	if (fileInt)
	{
#ifdef IFX_USE_PLATFORM_FILES
		if (fileInt->platformFile)
		{
			retVal = IFXP_File_Close(fileInt->platformFile);
		}
#endif // def IFX_USE_PLATFORM_FILES

		IFXE_FreeMemoryUnsafe(fileInt);
	}

	return retVal;
}

#if defined (IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
	This function calls the engine to set time of testing framework.
*/
IFX_RETURN_STATUS IFXE_Set_Test_Time(IFX_UINT32 timeUpper, IFX_UINT32 timeLower){

	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_appui)
	{
		status = g_appui->setTestTime(timeUpper, timeLower);
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	This function calls the engine to get time of testing framework.
*/
IFX_RETURN_STATUS IFXE_Get_Test_Time(IFX_UINT32* timeUpper, IFX_UINT32* timeLower){

	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_appui)
	{
		status = g_appui->getTestTime(timeUpper, timeLower);
	}

	return status;
}
#endif

#if defined (IFX_GENERATE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
	This function calls the engine to update time of testing framework.
*/
IFX_RETURN_STATUS IFXE_Update_Test_Time(IFX_UINT32 timeUpper, IFX_UINT32 timeLower){

	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_appui)
	{
		status = g_appui->updateTestTime(timeUpper, timeLower);
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	This function calls the engine to increment time of testing framework.
*/
IFX_RETURN_STATUS IFXE_Increment_Test_Time(IFX_UINT32 timeUpper, IFX_UINT32 timeLower){

	IFX_RETURN_STATUS status = IFX_ERROR;

	if (g_currentState == IFXE_STATE_INACTIVE_RESTARTING)
		return IFX_ERROR_ENGINE_NOT_READY;

	if (g_appui)
	{
		status = g_appui->incrementTestTime(timeUpper, timeLower);
	}

	return status;
}
#endif

#if defined (IFX_USE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
	Set Restart event for Engine.
*/
void AT_Set_Engine_Restart(void)
{
	// Prevent modules from calling in to the engine via the IFXI api
	g_appui->setCallbacksEnabled(false);

	at_restart_engine_pending = IFX_TRUE;

	// Schedule a timeout event - Engine will be tear down and recreated
	// on the next process timers event
	IFX_UINT32 currentTimeLower;
	IFX_UINT32 currentTimeUpper;
	IFXP_Timer_Get_Current_Time(&currentTimeUpper, &currentTimeLower);
	IFXP_Timer_Schedule(currentTimeUpper, currentTimeLower);
}

/*-------------------------------------------------------------------------*//**
	Restart the Inflexion UI Engine.
*/
IFX_RETURN_STATUS AT_Restart_Engine(void)
{
	IFX_RETURN_STATUS status = IFX_SUCCESS;
	bool restoreState = false;
	bool testAvailable = false;
	volatile int memoryException = 0;

	// Get current configuration
	IFXE_CONFIGURATION config;
	IFXE_Get_Configuration(&config);

	// Setting to zero for using theme values
	config.width = 0;
	config.height = 0;
	config.left = 0;
	config.top = 0;
	lc_strcpy(config.display_mode,"");

	// Set up NDE cleanup stack
	LC_CLEANUP_PUSH_FRAME(memoryException);

	testAvailable = g_scriptExecutor->loadNextTestCase();
	if(testAvailable == true)
	{
		// Configuration may vary with new theme so determine new best fit
		status = EL_getBestFit(&config);
	}
	else
	{
		status = IFX_ERROR;
	}

	if(testAvailable)
	{
		while(status == IFX_ERROR)
		{
			g_scriptExecutor->testCaseLoadFailed();
			testAvailable = g_scriptExecutor->loadNextTestCase();

			if(testAvailable == false)
				break;

			else
				status = EL_getBestFit(&config);

		}
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memoryException);

	// The Porting layer must have its display setup before creating the Engine instance.
	// Set the porting layer display mode.
	if (status == IFX_SUCCESS)
	{
		status = IFXP_Display_Set_Mode(config.left, config.top, config.width, config.height, config.display_mode);
	}

	if(status == IFX_SUCCESS)
	{
		// Set up NDE cleanup stack
		LC_CLEANUP_PUSH_FRAME(memoryException);

		// delete previous engine instant
		if (g_appui)
		{
			delete g_appui;
			g_appui = NULL;
		}

		g_currentState = IFXE_STATE_INITIALIZED;
		(void)IFXP_Runtime_State_Changed(IFXE_STATE_INITIALIZED);

		if (memoryException == 0)
		{
			// Create interactive application UI hosted in space
			LcTaOwner<NdhsCAppUi> appui = NdhsCAppUi::create(config.language,
															 config.display_mode,
															 config.package_name, restoreState);
			g_appui = appui.release();
		}
		else
		{
			// Some catastrophic OOM error has occurred.
			status = IFX_ERROR;
		}

		if (g_appui == NULL)
			status = IFX_ERROR;

		// Finished with cleanup stack
		LC_CLEANUP_POP_FRAME(memoryException);
	}

	if (status == IFX_SUCCESS)
	{
		// Open the theme entry point node.
		status = g_appui->openEntryPoint(true, config.entrypoint_id);

		if (status == IFX_SUCCESS)
		{
			g_currentState = IFXE_STATE_ACTIVE;
			(void)IFXP_Runtime_State_Changed(IFXE_STATE_ACTIVE);
			status = IFXP_Display_Get_Info(&g_ifxDisplay);
		}
		else
		{
			// Set up NDE cleanup stack
			LC_CLEANUP_PUSH_FRAME(memoryException);

			testAvailable = g_scriptExecutor->testCaseLoadFailed();

			// Finished with cleanup stack
			LC_CLEANUP_POP_FRAME(memoryException);

			if(testAvailable == true)
			{
				AT_Set_Engine_Restart();
				status = IFX_SUCCESS;
			}
		}
	}

	if(status == IFX_ERROR)
	{
		IFXP_Command(IFXP_CMD_EXIT);
	}

	return IFX_SUCCESS;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS IFXE_Callback(
		IFX_HUI					hIfx,
		IFX_CALLBACK_CODE		nCode,
		IFX_HELEMENT			hElement,
		IFX_HMENU				hMenu,
		IFX_INT32				item,
		const void*				pInput,
		void*					pOutput)
{
	return NdhsCPlugin::pluginCallback(hIfx, nCode, hElement, hMenu, item, pInput, pOutput);
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS IFXE_ExclusivityPermitted(IFX_HUI hIfx, IFX_HEXCLUSIVITY hExclusivity, IFX_INT32 *pResult)
{
	return NdhsCPlugin::exclusivityPermitted(hIfx, hExclusivity, pResult);
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS IFXE_RequestExclusivity(IFX_HUI hIfx, IFX_HEXCLUSIVITY hExclusivity,
										  IFX_UINT32 priority,
										  IFX_UINT32 timeout)
{
	return NdhsCPlugin::requestExclusivity(hIfx, hExclusivity, priority, timeout);
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS IFXE_ReleaseExclusivity(IFX_HUI hIfx, IFX_HEXCLUSIVITY hExclusivity)
{
	return NdhsCPlugin::releaseExclusivity(hIfx, hExclusivity);
}

#if !defined(NDHS_PACKAGER) && !defined(NDHS_INTEGRATOR)
/*-------------------------------------------------------------------------*//**
*/
IFXE_FILE LcGetRomFile(IFX_UINT32 romFileCount, LcTRomFileEntry* pRomFileTable, const char* file_name)
{
	if (romFileCount == 0 || pRomFileTable == NULL || file_name == NULL || lc_strlen(file_name) > IFX_MAX_FILE_NAME_LEN)
		return NULL;

	IFXE_INTERNAL_FILE* fileInt = (IFXE_INTERNAL_FILE*)IFXE_AllocateMemoryUnsafe(sizeof(IFXE_INTERNAL_FILE));

	if (fileInt)
	{
		fileInt->romFile = NULL;
		fileInt->offset = 0;

#ifdef IFX_USE_PLATFORM_FILES
		fileInt->platformFile = NULL;
#endif // def IFX_USE_PLATFORM_FILES

#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		char validatedFileName[IFX_MAX_FILE_NAME_LEN];

		const char* inputPtr = file_name + (IFX_PLAT_DRIVE_SEP_POS + 2);
		char* outputPtr = validatedFileName;
		bool complete = false;

		while (!complete)
		{
			char inputChar = *inputPtr++;

			if (NDHS_PLAT_DIR_SEP_CHAR == inputChar)
			{
				inputChar = NDHS_DIR_SEP_CHAR;
			}

			*outputPtr++ = inputChar;

			complete = inputChar == '\0';
		}
#else
	const char* validatedFileName = NULL;
#if defined(NDHS_JNI_INTERFACE)
		validatedFileName = file_name + (IFX_PLAT_DRIVE_SEP_POS + 3);
#else
		validatedFileName = file_name + (IFX_PLAT_DRIVE_SEP_POS + 2);
#endif

#endif

		// Search rom table
		for (unsigned i = 0; i < romFileCount && !fileInt->romFile; i++)
		{
			if (lc_strcmpi(validatedFileName, pRomFileTable[i].name) == 0)
			{
				fileInt->romFile = &pRomFileTable[i];
			}
		}

		if (fileInt->romFile == NULL)
		{
			IFXE_FreeMemoryUnsafe(fileInt);
			fileInt = NULL;
		}
	}



	return fileInt;
}

#endif
