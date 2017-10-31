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
#ifndef NdhsCAppUiH
#define NdhsCAppUiH

class LcCMutex;

/*-------------------------------------------------------------------------*//**
	NdhsCAppUi is derived from the platform-independent NdhsCMenuCon class,
	and is an Ndhs-specific menu controller.
*/
class NdhsCAppUi : public NdhsCMenuCon
{
private:

	// Cached info
	LcTmString						m_dataPath;
	LcTmOwner<LcCMutex>				m_uiLock;
	bool							m_suspended;

	// For normal files in App subfolder:-
	virtual LcTaString				getThemePath() 		{ return NDHS_PACKAGE_DIR NDHS_DIR_SEP; }

protected:

	virtual LcTaOwner<NdhsCPageManager> 	createPageManager(	const LcTmString& language,
																const LcTmString& displayMode,
																const LcTmString& themeName);

	virtual LcTaString				getDataPath()		{ return m_dataPath; }
	virtual LcTaString				getAppName()		{ return "NDHS"; }
	virtual LcTaString				getFullAppName()	{ return ""; }
	virtual int						getAppId()			{ return 0; }

	// Construction
	void							construct(	const LcTmString& language,
												const LcTmString& displayMode,
												const LcTmString& themeName,bool restoreSavedState);

public:
	// Construction etc - NDE pattern.
	LC_IMPORT static LcTaOwner<NdhsCAppUi> create(	const LcTmString& language,
													const LcTmString& displayMode,
													const LcTmString& themeName,bool restoreSavedState);
	virtual							~NdhsCAppUi();

	virtual void					configureScreenCanvas(bool statusPane);

	IFX_RETURN_STATUS				setConfiguration(	const char* language,
														const char* displayMode,
														const char* themeName,
														const char* entryPoint_Id);

	IFX_RETURN_STATUS				getConfiguration(	char* language,
														char* displayMode,
														char* themeName,
														char* entrypointId,
														unsigned int maxStringLen);
	IFX_RETURN_STATUS 				destroyPageStack();

	// Drive the application
	IFX_RETURN_STATUS				processTimers(IFX_UINT32 timeUpper, IFX_UINT32 timeLower);

	// This is used to manage asynchronous module links.
	IFX_RETURN_STATUS				asyncLinkComplete();

	// Methods to control the app state
	IFX_RETURN_STATUS				setIdle();
	IFX_RETURN_STATUS				refresh(bool refreshAll);
	IFX_RETURN_STATUS				suspend();
	IFX_RETURN_STATUS				resume();
	IFX_RETURN_STATUS				resumeStaticAnimations();

	// Key events
	IFX_RETURN_STATUS				keyDown		(int c);
	IFX_RETURN_STATUS				keyUp		(int c);

	// Mouse events
	IFX_RETURN_STATUS				mouseDown	(const LcTPixelPoint& pt);
	IFX_RETURN_STATUS				mouseUp		(const LcTPixelPoint& pt);
	IFX_RETURN_STATUS				mouseMove	(const LcTPixelPoint& pt);

	// Open entry point
	IFX_RETURN_STATUS 				openEntryPoint(bool openDefault, const LcTmString& entryPoint);

	// save state
	IFX_RETURN_STATUS				saveState		(int *abortSave);

	virtual void					doAsyncLaunchComplete();

#if defined (IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
	IFX_RETURN_STATUS				setTestTime  (IFX_UINT32 timeUpper, IFX_UINT32 timeLower);
	IFX_RETURN_STATUS				getTestTime  (IFX_UINT32* timeUpper, IFX_UINT32* timeLower);
#endif

#if defined (IFX_GENERATE_SCRIPTS)
	IFX_RETURN_STATUS				updateTestTime		(IFX_UINT32 timeUpper, IFX_UINT32 timeLower);
	IFX_RETURN_STATUS				incrementTestTime	(IFX_UINT32 timeUpper, IFX_UINT32 timeLower);
#endif
};

#endif
