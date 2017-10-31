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
#ifndef NdhsCPageManagerH
#define NdhsCPageManagerH

#include "inflexionui/engine/inc/LcTMessage.h"
#include "inflexionui/engine/inc/NdhsCMenuComponentTemplate.h"
#include "inflexionui/engine/inc/NdhsCPageTemplate.h"
#include "inflexionui/engine/inc/NdhsCManifest.h"
#include "inflexionui/engine/inc/NdhsCPlugin.h"


class NdhsCPageModel;
class NdhsCMenuCon;
class NdhsCManifestStack;
class NdhsCManifest;
class NdhsCTokenStack;
class NdhsCScrollPosField;
class NdhsCPath;
class NdhsCMenuComponent;
class NdhsCEntryPointMapStack;
class NdhsCPage;

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	#include "inflexionui/engine/inc/NdhsCEffect.h"
	class NdhsCEffect;
#endif	/* defineed(IFX_RENDER_DIRECT_OPENGL_20) */

#define NESTED_COMPONENT_LEVEL 25

/*------------------------------------------------------------------------*//*
*/
class NdhsCPageManager : public LcCApplet, public LcTMessage::IHandler , public ISerializeable
{
public:
	typedef enum
	{
#ifdef IFX_USE_PLUGIN_ELEMENTS
		EEventRefreshBufferedElement,
#endif
		EEventRefreshPage,
		EEventRefreshPageCompletion,
		EEventRefreshField,
		EEventTrigger,
		EEventLink,
		EEventSetActiveItem
	} EEventType;

#ifdef LC_USE_STYLUS
	// Mouse state
	enum EMouse
	{
		EMouseUp,
		EMouseDragging,
		EMouseDown,
		EMouseOver
	};
#endif

LC_PRIVATE_INTERNAL_PUBLIC:
	class TDesignSize
	{
	public:
		int									width;
		int									origin;

		TDesignSize() {}
	};

#if defined(NDHS_JNI_INTERFACE)
	class CStaticPreviewCache : public LcCBase
	{
	protected:
		CStaticPreviewCache ()
		{
			slotNumber = -1;
			identifier = 0;
		}

	public:

		int							slotNumber;
		LcTmString					componentClassName;
		LcTmString					componentPath;
		LcTmString					layoutName;
		unsigned int				identifier;

		static LcTaOwner<CStaticPreviewCache>
									create();
		virtual						~CStaticPreviewCache() {}

	};
	typedef LcTmOwnerMap<unsigned int, CStaticPreviewCache>  TmAPreviewCache;
#endif

public:
	class CDisplay : public LcCBase
	{
	public:

		class CDisplayMode : public LcCBase
		{
		protected:
			CDisplayMode() 						{}

		public:
			int									width;
			int									height;
			LcTmString							designSize;
			LcTmString							defaultMenu;
			LcTmString							defaultLink;

			static LcTaOwner<CDisplayMode>		create();
		};

	protected:
		CDisplay()								{}

	public:
		LcTmOwnerMap<LcTmString, CDisplayMode> displayModes;

		static LcTaOwner<CDisplay>			create();
	};

	class TPageWidgetElem
	{
	public:
		NdhsCElement*						element;
		NdhsCElementGroup*					eventManager;
		bool								mouseDownSent;
		bool								ignoreEntry;

		TPageWidgetElem()
		{
			element = NULL;
			eventManager = NULL;
			mouseDownSent = false;
			ignoreEntry = false;
		}

		TPageWidgetElem(NdhsCElement* elem, NdhsCElementGroup* eventManagerAggr,
							bool mouseDownEventSent, bool entry)
		{
			element = elem;
			eventManager = eventManagerAggr;
			mouseDownSent = mouseDownEventSent;
			ignoreEntry = entry;
		}

		~TPageWidgetElem()
		{
			element = NULL;
			eventManager = NULL;
			mouseDownSent = false;
			ignoreEntry = false;
		}
	};

	typedef LcTmArray<TPageWidgetElem>		TmAPageWidgetElem;
	typedef LcTmArray<LcCWidget*>			TmAWidgets;

private:
	typedef LcTmMap<LcTmString, TDesignSize> TmMDesignSizes;
	TmMDesignSizes							m_designSizes;

	typedef LcTmOwnerMap<LcTmString, CDisplay> TmMDisplays;
	TmMDisplays								m_displays;

	typedef LcTmArray<int>						TmAKeyCodes;
	typedef LcTmMap<LcTmString, TmAKeyCodes>	TmMKeys;
	TmMKeys										m_keys;

#if		defined (IFX_GENERATE_SCRIPTS)
	NdhsCScriptGenerator*					m_scriptGenerator;
#endif

#if		defined (IFX_USE_SCRIPTS)
	NdhsCScriptExecutor*					m_scriptExecutor;
#endif

	// Pointer to the theme's menu controller
	NdhsCMenuCon*					m_con;

	LcTmOwner<NdhsCEntryPointMapStack>
									m_entryPointStack;

	LcTmString						m_currentBackground;
	typedef LcTmOwner<NdhsCExpression::CExprSkeleton> TmMExprSkeleton;
	TmMExprSkeleton					m_defaultFontFace;
	LcTmOwner<NdhsCExpression>		m_fontExpr;
	LcCFont::EStyle					m_defaultFontStyle;
	LcTmString						m_defaultFontColor;

	int								m_nestedComponentLevel;
	bool							m_isCyclic;

	// Internal helpers
	enum EAction
	{
		ENoAction,
		EBackDestroyChildren,
		ELaunchLink,
		EBackFromLink,
		EOpenMenuHideDirectParent,
		EOpenMenuShowDirectParent,
		EOpenMenuHideAllParents,
		EOpenMenuHideAllShowRoot,
		EHideToShow,
		ESkipToClose,
		EResetToTop,
		EAnimateActivePage,
		EShowStaticPreview
	};

	class CTemplateItem : public LcCBase
	{
	protected:
		CTemplateItem()				{ refCount = 0; }

	public:
		int								refCount;
		int								stackLevel;
		LcTmString						key;
		LcTmOwner<NdhsCTemplate>		templateFile;

		static LcTaOwner<CTemplateItem>		create();
	};

	class CMenuItem : public LcCBase
	{
		LcTmArray<NdhsCMenuComponent*>	refBy;
	protected:
		CMenuItem()					{ }

	public:
		int							stackLevel;
		LcTmOwner<NdhsCMenu>		menu;
		NdhsCManifest*				paletteManifest;

		static LcTaOwner<CMenuItem>	create();
		void						addReference(NdhsCMenuComponent* newRef);
		void						removeReference(NdhsCMenuComponent* ref);
		bool						isUnreferenced()				{ return refBy.empty(); }
		void						notifyMenuReload(bool firstPhase);
		void						notifyActiveItemChange(int newActiveItem);
	};

	class CNodeItem : public LcCBase
	{
	protected:
		CNodeItem()					{ refCount = 0; }

	public:
		int							refCount;
		int							stackLevel;
		LcTmOwner<NdhsCPage>		uiNode;

		static LcTaOwner<CNodeItem>	create();
	};

#ifdef IFX_USE_PLUGIN_ELEMENTS
	class CPluginElementItem : public LcCBase
	{
	protected:
		CPluginElementItem()		{ refCount = 0; }

	public:
		int							refCount;
		int							stackLevel;
		LcTmOwner<NdhsCPlugin::NdhsCPluginHElement>	pluginElement;

		static LcTaOwner<CPluginElementItem>	create();
	};
#endif
	class CPaletteItem : public LcCBase
	{
	protected:
		CPaletteItem()					{}

	public:
		int							refCount;
		LcTmOwner<NdhsCManifest>	paletteManifest;

		static LcTaOwner<CPaletteItem>	create();
	};

	private:

	 // Warning: Objects of this class are created in non-engine threads,
	 // within callbacks. So do not use any cleanup-stack related types.
	class TEventInfo
	{
	protected:
		inline			void*			operator new(size_t size) THROW_NONE
																	{ return LcTmAlloc<TEventInfo>::allocUnsafe((int)size); }
		inline			void*			operator new[](size_t size) THROW_NONE
																	{ return LcTmAlloc<TEventInfo>::allocUnsafe((int)size); }
#if !defined(LC_STL_DISABLE_NOTHROW)
		inline			void*			operator new(size_t size, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																	{ return LcTmAlloc<TEventInfo>::allocUnsafe((int)size); }
		inline			void*			operator new[](size_t size, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																	{ return LcTmAlloc<TEventInfo>::allocUnsafe((int)size); }
#endif
		inline			void			operator delete(void* p) THROW_NONE
																	{ LcTmAlloc<TEventInfo>::freeUnsafe(p); }
		inline			void			operator delete[](void* p) THROW_NONE
																	{ LcTmAlloc<TEventInfo>::freeUnsafe(p); }
#if !defined(LC_STL_DISABLE_NOTHROW)
		inline			void			operator delete(void* p, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																	{ LcTmAlloc<TEventInfo>::freeUnsafe(p); }
		inline			void			operator delete[](void* p, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																	{ LcTmAlloc<TEventInfo>::freeUnsafe(p); }
#endif

	public:
		EEventType						eventType;
		IFX_HMENU						menu;
		IFX_HELEMENT					element;
		int								item;
		IFX_WCHAR*						fieldOrUri;
		bool							immediate;
		int								key;
		TEventInfo*						next;

		static 			TEventInfo*		createUnsafe()	{ return new TEventInfo(); }
		static 			void			destroy(TEventInfo* pItem) { if (pItem->fieldOrUri) LcTmAlloc<LcTByte>::freeUnsafe(pItem->fieldOrUri); delete pItem; }

		inline			void*			operator new(size_t size, TEventInfo* place) THROW_NONE 	{ LC_UNUSED(size); return place; }
	};

	typedef LcTmOwnerArray<NdhsCPageModel>  TmAPageStack;
	TmAPageStack					m_pageStack;
	typedef LcTmOwnerMap<LcTmString, CTemplateItem> TmMPageTemplateCache;
	TmMPageTemplateCache			m_templates;

	// Placing it in page model will be inefficient, while getting
	// cached menus we will have to traverse all pages cache in worst
	// case
	typedef LcTmOwnerMap<LcTmString, CMenuItem> TmMMenuCache;
	TmMMenuCache					m_menus;
	typedef LcTmOwnerMap<LcTmString, CNodeItem>  TmANodeCache;
	TmANodeCache					m_nodes;

#ifdef IFX_USE_PLUGIN_ELEMENTS
	typedef LcTmOwnerMap<LcTmString, CPluginElementItem> TmMPluginElementCache;
	TmMPluginElementCache			m_pluginElements;
	int								m_eventHandlerElemCount;
#endif

	typedef LcTmOwnerMap<LcTmString, CPaletteItem> TmMPaletteCache;
	TmMPaletteCache					m_paletteMap;
	typedef LcTmOwnerMap<LcTmString, NdhsCManifest> TmMPackageManifestCache;
	TmMPackageManifestCache			m_packageManifestMap;

#if defined(NDHS_JNI_INTERFACE)
	TmAPreviewCache					m_currentStateSnapshot;

	// identifier will be used in 'in-place edit' mode, to
	// set the unique identifier for a component, and when
	// we will go for restoring state we will use this to select
	// layout in state manager, so by using this we can reach to
	// exact component, with out matching each and every attribute
	unsigned int					m_identifier;
	bool							m_inplaceMode;
	bool							m_inStaticPreviewMode;

	LcTmString						m_currentComponentLayoutName;
	unsigned int					m_currentComponentIdentifier;

#endif

	TEventInfo*						m_eventListHead;
	TEventInfo*						m_eventListTail;

	bool							m_animatingStateChange;
	int								m_activePage;
	EAction							m_action;

	bool							m_isLaunchingMenuItem;
	LcTmOwner<NdhsCMenuItem>		m_launchItem;
	NdhsCMenuItem* 					m_itemToLaunch;

	int								m_terminalTime;
	ENdhsVelocityProfile			m_terminalVelocityProfile;

	bool							m_bIsAsynchronousLaunchCompleteMessagePending;

	LcTMessage						m_destroyUnwantedPagesMessage;
	LcTMessage						m_openThemeMessage;
	LcTMessage						m_configureExternalMessage;
	LcTMessage						m_eventMessage;
	LcTMessage						m_launchLinkMessage;
	LcTMessage						m_simulateKeyMessage;
	LcTMessage						m_postTransitionCompleteMessage;
	LcTMessage						m_changeBackgroundMessage;
	LcTMessage						m_asynchronousLaunchCompleteMessage;
#ifdef LC_USE_MOUSEOVER
	LcTMessage						m_doMouseOverHitTestMessage;
#endif


	LcTmString						m_defaultEntryPointID;
	LcTmString						m_currentEntryPointId;
	LcTmString						m_defaultTheme;
	LcTmString						m_mainMenuLink;

	LcTmString						m_dataPath;
	LcTmString						m_iniTokenFile;
	LcTmString						m_usrTokenFile;

	LcTmString						m_language;
	LcTmString						m_screenSize;
	LcTmString						m_designSize;
	LcTmString						m_lastGoodThemeName;

	// Theme name, display mode and language storage for external configuration request.
	LcTmString						m_reqThemeName;
	LcTmString						m_reqDisplayMode;
	LcTmString						m_reqLanguage;
	LcTmString						m_reqEntryPointId;

	// Manifest Stack
	LcTmOwner<NdhsCManifestStack>	m_manifestStack;

	// Token Stack
	LcTmOwner<NdhsCTokenStack>		m_tokenStack;


	NdhsCElement*					m_lastRealizedElement;

	// The Integration Layer
	NdhsCPlugin*					m_plugin;

	// Refresh-queue mutex
	LcTmOwner<LcCMutex>				m_eventQueueMutex;

#ifdef LC_USE_STYLUS
	// Ordered list of widgets elements / pages
	TmAPageWidgetElem				m_pageWidgetElemList;

	// Mouse state and drag element
	EMouse							m_mouseState;
	TPageWidgetElem*				m_dragCapturedEntry;

#endif

	LcTmOwner<NdhsCPath>			m_primaryLightPath;
	bool							m_primaryLightOverride;
	LcTmOwner<NdhsCScrollPosField>	m_primaryLightPos;
	int								m_primaryLightDuration;
	LcTTime							m_primaryLightOverrideTransitonBeginTimestamp;
	bool							m_primaryLightOverrideTransitonStarted;
	bool							m_primaryLightOverrideTransitonEnded;

#ifdef LC_USE_MOUSEOVER
	LcTPixelPoint					m_mouseOverCurrentPt;
	bool							m_doMouseOverHitTesting;
	NdhsCElement*					m_currentMouseOverElement;
	NdhsCElement*					m_currentMouseOverTestElement;
	unsigned int					m_mouseOverDelay;
#endif

	LcTPlacement					m_suspendPrimaryLightPlacement;

	void							onTransitionComplete(bool setIdle);
	void							destroyUnwantedPages(bool force);

	// This will complete the launch item message action. It is used in two parts for asynchronous launch.
	void							postLaunchItemAction();

	bool							openPalette(const LcTmString& palettePath);
	void							loadPalette(LcCXmlElem* root);
	bool							loadTheme(const LcTmString& packagePath, bool animate);

	void							setMatchedBackground(int stackLevel);
	bool							setBackground(const LcTmString& file, int stackLevel);

	bool 							queueEvent(TEventInfo* info);
	void 							destroyEventQueue();

	void							setCurrentDefaultLinkAndMenu();
	CDisplay::CDisplayMode*			getDisplayMode(const LcTmString& displayMode);
	void							setCurrentDesignSize();
	bool							processRefreshEvents(bool refreshTimeout);
	void 							updatePageStates(bool animate, bool menuItem, int stackLevel);
	LcTaString 						getFullPackagePath(const LcTmString& path);
	LcTaString						getFullPalettePath(const LcTmString& path);

	void							setupChangeBackgroundMessage(int subPageIndex);

	NdhsCPlugin::NdhsCPluginMenu* 	getPluginMenu(IFX_HMENU hMenu);
	bool 							onKeyDown(int c, bool fromModule);

	void							setupPrimaryLightTransitionOverrideFlag(int newActivePage, int oldActivePage);
	void							setupPrimaryLightTransitionOverrideTiming(int newActivePage, int oldActivePage);
	void							doPrimaryLightOverride(LcTTime timestamp);

#ifdef LC_USE_MOUSEOVER
	bool 							scheduleMouseOverHitTest(const LcTPixelPoint& pt);
#endif

	// Will be used internally to load menu, in case menu is not cached
	LcTaOwner<NdhsCMenu>			loadMenu(const LcTmString& menuLink, volatile bool& menuLoaded, NdhsCManifest* palManifest, int stackLevel);
	LcTaOwner<NdhsCMenu>			loadMenuFromXML(const LcTmString& menuLink, volatile bool& menuLoaded, NdhsCManifest* palManifest, int stackLevel);
	LcTaOwner<NdhsCMenu>			loadMenuFromPageFile(const LcTmString& menuLink, volatile bool& menuLoaded, bool nodeInfo, int stackLevel);
	NdhsCPage*						getPageByLink(const LcTmString& link, int stackLevel);
	bool							expandLink(const LcTmString& menuLink, LcTmString& expandedLink, int stackLevel);

LC_PRIVATE_INTERNAL_PUBLIC:
	void 							setFieldCacheDirty();
	bool							openPage(NdhsCMenuItem* item, bool animate, bool menuItem, int stackLevel);
	NdhsCPage*						getPage(NdhsCMenuItem* item, int stackLevel);

protected:

	// Helpers for derived classes:

	// Derived classes MAY override these:

	// onRealize event
	virtual void					onRealize();
	virtual	void					onRetire();

	// Message callback method
	enum EMessages {
		EDestroyUnwantedPagesMsg,
		EDestroyUnwantedElementsMsg,
		EOpenThemeMsg,
		EConfigureExternalMsg,
		ELaunchLinkMsg,
		EEventMsg,
		ESimulateKeyMsg,
		EPostTransitionCompleteMsg,
		EChangeBackgroundMsg,
		EAsynchronousLaunchCompleteMsg
#ifdef LC_USE_MOUSEOVER
		,EDoMouseOverHitTest
#endif
	};

	class TBitmapLoader : public LcIBitmapLoader
	{
	public:
							TBitmapLoader()		{}
		virtual				~TBitmapLoader()		{}

		NdhsCPageManager*	pageManager;
		int					stackLevel;
		NdhsCManifest*		paletteManifest;
		LcCBitmap*			getBitmap(const LcTmString &file)
								{ return pageManager->getBitmap(file, stackLevel, paletteManifest); }
	};


	virtual void onMessage(int iID, int iParam);

	// Two-phase construction by derived class
									NdhsCPageManager(NdhsCMenuCon* con);
	void							construct(	const LcTmString& appName,
												const LcTmString& language,
												const LcTmString& screenSize,
												const LcTmString& themeName);

public:

	// Creation/destruction
	static LcTaOwner<NdhsCPageManager> create(	const LcTmString& appName,
												NdhsCMenuCon* con,
												const LcTmString& language,
												const LcTmString& screenSize,
												const LcTmString& themeName);
	virtual							~NdhsCPageManager();

	virtual void					onWidgetEvent(LcTWidgetEvent* e) { LC_UNUSED(e);}

	NdhsCPageTemplate*				getPageTemplate(NdhsCPage* page, int stackLevel, LcTmString& templatePath);
	NdhsCTemplate*					getComponentTemplate(LcTmString& componentName, int stackLevel, NdhsCManifest* paletteManifest, int& nestedComponentLevel);

	void							setScreenSize(const LcTmString& screenSize)	{ setLanguageAndScreenSize(m_language, screenSize); }
	void							setLanguage(const LcTmString& language)		{ setLanguageAndScreenSize(language, m_screenSize); }
	void							setLanguageAndScreenSize(const LcTmString& language, const LcTmString& screenSize);

	bool							getCurrentDesignSize(int& width, int& origin);
	LcTmString						getCurrentScreenMode() 						{ return m_screenSize; }
	LcTmString						getCurrentLanguage() 						{ return m_language; }
	LcTmString						getCurrentTheme()							{ return m_lastGoodThemeName; }
	const char*						getCurrentScreenModeString() 				{ return m_screenSize.bufUtf8(); }
	const char*						getCurrentLanguageString() 					{ return m_language.bufUtf8(); }
	const char*						getCurrentThemeString()						{ return m_lastGoodThemeName.bufUtf8(); }
	const char*						getCurrentEntryPointId()					{ return m_currentEntryPointId.bufUtf8(); }
	bool							getCurrentScreenSize(int& width, int& height);
	bool							getScreenSize(const LcTmString& displayMode, int& width, int& height);
	LcTmArray<int>*					getKeyCodes(const LcTmString& key);

	int								getPageStackLevel(NdhsCPageModel* page);
	void 							destroyPageStack(void);
	bool							launchItem(NdhsCMenuItem* item, bool menuItem, int stackLevel);

	void							decrementAnimatorCount();
	bool							processAttempt(NdhsCTemplate::CAction::CAttempt* attempt, int stackLevel, const LcTmString& additionalInfo = "");
	void							jumpTransitionToEnd();
	void							setIdle();
	void							resumeStaticAnimations();

	inline void						startAnimation()							{ if (getSpace()) getSpace()->revalidate(false); }

	virtual bool					onKeyDown(int c)							{ return onKeyDown(c, false); }
	void							translateKeyCode(int &c);
	virtual bool					onKeyUp(int c);

#if	defined (IFX_GENERATE_SCRIPTS) && defined (NDHS_JNI_INTERFACE)
	NdhsCScriptGenerator*			getScriptGenerator() { return m_scriptGenerator; }
#endif

#if	defined (IFX_USE_SCRIPTS) && defined (NDHS_JNI_INTERFACE)
	NdhsCScriptExecutor*			getScriptExecutor() { return m_scriptExecutor; }
#endif

#ifdef LC_USE_STYLUS
	virtual bool					onMouseDown	(const LcTPixelPoint& pt);
	virtual bool					onMouseMove(const LcTPixelPoint& pt);
	virtual bool					onMouseUp	(const LcTPixelPoint& pt);
	virtual void 					onMouseCancel(NdhsCElementGroup* page, NdhsCElement* element, bool allButCaptured);
	EMouse							getMouseState() { return m_mouseState; }
#endif

#ifdef LC_USE_MOUSEOVER
	void							setMouseOverElement(NdhsCElement* mouseOverElement) { resetMouseOverDisplacement(LcTPixelPoint(), true); m_currentMouseOverTestElement = mouseOverElement; }
	void							displacementApplied();
	void							resetMouseOverDisplacement(const LcTPixelPoint& pt, bool force = false);
	void							resetMouseOverElement(NdhsCElement* mouseOverElement);
#endif

	bool							bubbleTrigger(int code, int slot, LcTmString& elementClass,
													LcTmArray<NdhsCTemplate::CAction::CAttempt*>* optionsAttempts,
													bool fromModule);

	bool							onOptionsMenuTrigger(LcTmArray<NdhsCTemplate::CAction::CAttempt*>* optionsAttempts);
	bool 							onSimulateLinkTrigger(LcTmArray<NdhsCTemplate::CAction::CAttempt*>* linkAttempt);
	void							simulateKeyDown(int c);
	void 							queueKeyDownEvent(int c);
	void 							queueLinkEvent(const IFX_WCHAR* uri);
	void							queueSetActiveItemEvent(IFX_HMENU hMenu, int item);
	void							schedulePostTransitionComplete();

	// This will complete the launch item message action. It is used in two parts for asynchronous launch.
	void							scheduleAsynchronousLaunchComplete();

	inline bool						isAtTop()									{ return m_activePage == 0; }

	inline NdhsCManifestStack*		getManifestStack()							{ return m_manifestStack.ptr(); }
	inline NdhsCTokenStack*			getTokenStack()								{ return m_tokenStack.ptr(); }
	inline NdhsCPageModel*			getActivePage()								{ return m_pageStack[m_activePage]; }

	inline void						setLastRealizedElement(NdhsCElement* lastRealizedElement)	{ m_lastRealizedElement = lastRealizedElement; }

	bool							loadAppConfigFromState(LcTmString& errorString,LcCSerializeMaster *serializeMaster);
	bool							loadAppConfigFromXml(LcTmString& errorString);
	bool							applyFirstTheme();
	IFX_RETURN_STATUS				openEntryPoint(bool openDefault, const LcTmString& entryPoint);
	bool 							getParamterByName(const LcTmString& link, const LcTmString& paramName, LcTmString& value);
	bool							extractLinkPrefixAndLinkBody(const LcTmString& nodeUri,LcTmString& linkPrefix, LcTmString& linkBody);
	bool							updateFields(LcTTime timestamp);
	bool							isValidURIToExecuteFromCallBack(const LcTmString& uri);
	void							savePersistentData();

#ifdef NDHS_PREVIEWER
	void							refreshActivePage();
#endif
	void							refreshPage(IFX_HMENU hMenu, bool immediate);
	void							refreshField(IFX_HMENU hMenu, int item, const IFX_WCHAR* field);
	void							doRefreshField(IFX_HMENU hMenu, int item, const LcTmString& field);

	// This will return the plug-in index of the active item in the specified plug-in menu.
	int								getHMenuActiveItemIndex(IFX_HMENU hMenu);

	// This will return the full path from the manifest stack of a particular menu.
	bool							getFullFilePath(IFX_HMENU hMenu, const IFX_WCHAR* pInput, IFX_WCHAR* returnFilePath);

#if defined(NDHS_PREVIEWER) || defined(NDHS_JNI_INTERFACE)
	inline NdhsCPageModel*			getSafeActivePage()							{ if (m_activePage >= 0 && m_activePage < (int)m_pageStack.size()) return m_pageStack[m_activePage]; else return NULL; }
	bool							applyPackage(LcTmString& package, bool inStaticPreview);
	bool							setEngineState( LcTmString package, LcTmString linkData, LcTmString templ, LcTmString layout, int slot, bool animate,
														LcTmString componentClassName, LcTmString componentPath, LcTmString componentLayoutName, bool editorSwitched);
#endif
	bool 							applyConfiguration(const LcTmString& language, const LcTmString& displayMode, const LcTmString& package, const LcTmString entryPoint_Id);

	void							setPrimaryLightPlacement(const LcTPlacement& pl, int mask);
	inline bool						isPrimaryLightOverrideActive()				{ return m_primaryLightOverride; }

	// Get pointer to the theme's menu controller
	inline NdhsCMenuCon*			getCon()									{ return m_con; }
	inline NdhsCPlugin*				getPlugin()									{ return m_plugin; }

	LcTaString						getDefaultFontFace();
	inline LcCFont::EStyle			getDefaultFontStyle()						{ return m_defaultFontStyle; }
	inline LcTaString				getDefaultFontColor()						{ return m_defaultFontColor; }
	inline int						getDefaultTerminalTime()					{ return m_terminalTime; }
	inline ENdhsVelocityProfile		getDefaultTerminalVelocityProfile()			{ return m_terminalVelocityProfile; }

	// Resource functions
	virtual LcCBitmap*				getBitmap(const LcTmString& file, int stackLevel, NdhsCManifest* paletteManifest);
	virtual bool					findBitmapFile(LcTaArray<NdhsCManifest::CManifestFile*> *fileData, int &index);
	virtual LcCFont*				getFont(const LcTmString& name, LcCFont::EStyle style, int stackLevel);
	virtual LcCFont*				getFont(const LcTmString& name, LcCFont::EStyle style);
#ifdef LC_USE_MESHES
	virtual LcCMesh*				getMesh(const LcTmString&	file,
											NdhsCElement*		element,
											NdhsCMenu*			menu,
											NdhsCMenuItem*		menuItem,
											int					stackLevel,
											NdhsCManifest*		paletteManifest);
#endif

	NdhsCMenu*						getMenu(const LcTmString& menuLink, volatile bool& menuLoaded, NdhsCManifest* palManifest, int stackLevel, NdhsCMenuComponent *caller);

	// Get the palette from the special manifest stack.
	NdhsCManifest*					getPaletteManifest(const LcTmString& palettePath);
	bool							closePalette(NdhsCManifest* paletteManifest);

	// Get Manifest from absolute path (if available)
	NdhsCManifest*					getPackageManifest(const LcTmString& packagePath);
	LcTaString						getFileData(LcTmString& absolutePath,
												LcTaArray<NdhsCManifest::CManifestFile*> *fileData);


	void 							releaseMenu(NdhsCMenu* menu, NdhsCMenuComponent *caller);
	void 							releaseUiNode(NdhsCPage* uiNode);
	ENdhsMenuToLoadFrom				menuToLoadFrom(const LcTmString& menuLink);

	// Make sure the base applet functions don't get used - the new functions above should
	// be used now
	virtual LcCBitmap*				getBitmap(const LcTmString& file) { LC_UNUSED(file) LC_ASSERT(false); return NULL; }
#ifdef LC_USE_MESHES
	virtual LcCMesh*				getMesh(const LcTmString& file) { LC_UNUSED(file) LC_ASSERT(false); return NULL; }
#endif

	LcTaOwner<NdhsCPage>			loadPage(NdhsCMenuItem* item, int stackLevel);

	// Manage idle states.
	void							resetThemeToTop();

#ifdef IFX_USE_PLUGIN_ELEMENTS
	NdhsCPlugin::NdhsCPluginHElement* findOwnerPluginHandler(IFX_HELEMENT hElement);
	// An embedded element wishes to go 'full screen' (or has finished being full screen).
	bool							makeFullScreen(IFX_HELEMENT hElement, bool bStartFullScreen);
	// An embedded element wants to update its contents
	void							refreshBufferedElement(IFX_HELEMENT hElement);
	// An embedded element wants to resize a buffer
	bool							resizeBufferedElement(IFX_HELEMENT hElement, IFX_BUFFER_SIZE* bufferSize);
#endif

	LcTaString						entryPointIDToNodeURI(const LcTmString& entryPoint);

	// Release page/component templates
	void							releaseTemplate(NdhsCTemplate* templateFile);

	// Allow derived aggregate to perform any pre-frame update action - note that this is called
	// only on the top-level aggregate registered with the active space
	virtual bool					doPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame);
	virtual	void					doPreFrameUpdate();
	virtual void					doPostFrameUpdate();

	void 							stopAllFields();

	ETemplateType					getTemplateType(LcCXmlElem* eSettings);

	void 							onSuspend();
	void 							onResume();

	inline int						getNestedComponentLevel() { m_nestedComponentLevel = NESTED_COMPONENT_LEVEL; return m_nestedComponentLevel; }

	inline NdhsCEntryPointMapStack*	getEntryPointMapStack() { return m_entryPointStack.ptr(); }

	bool							openPackage(const LcTmString& packagePath);

#ifdef IFX_USE_PLUGIN_ELEMENTS
	NdhsCPlugin::NdhsCPluginHElement* getPluginElement(NdhsCPlugin::NdhsCPluginMenu *pluginMenu,
																					int itemIndex,
																					LcTaString linkName,
																					IFX_ELEMENT_PROPERTY *elementProperty);
	void							releasePluginElement(NdhsCPlugin::NdhsCPluginHElement* pluginElement);
#endif

#if defined(NDHS_JNI_INTERFACE)
	inline void						incrementIdentifier() { ++m_identifier; }
	inline void						resetIdentifier() { m_identifier = 2; }
	inline unsigned int				getIdentifier() { return m_identifier; }
	CStaticPreviewCache*			getCachedLayoutInfo(CStaticPreviewCache* info);
	inline bool						isInplaceEditMode() { return m_inplaceMode; }
	bool							cacheCurrentStateSnapShot(LcTmOwner<CStaticPreviewCache> info);
	inline TmAPreviewCache*			getCurrentStateSnapShotCache() { return &m_currentStateSnapshot; }
	void							setInPlacePreviewCacheEntry(NdhsCPageManager::CStaticPreviewCache* layoutCacheEntry);
	inline LcTaString				getCurrentComponentLayoutName() { return m_currentComponentLayoutName; }
	inline unsigned int				getCurrentComponentIdentifier() { return m_currentComponentIdentifier; }
	bool							isInStaticPreviewMode() { return m_inStaticPreviewMode; }
	void							resetCache() { m_currentStateSnapshot.clear(); }
	LcTaString						getLayoutToConfigure(CStaticPreviewCache* info);
	CStaticPreviewCache*			getCachedEntry(CStaticPreviewCache* entry);
	CStaticPreviewCache*			getCachedEntry(unsigned int identifier);
#endif

#ifdef IFX_SERIALIZATION
		bool						restoreState(LcCSerializeMaster *serializeMaster);
		SerializeHandle				serialize(LcCSerializeMaster *serializeMaster,bool force=false);
		void						deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
		LcTaOwner<NdhsCMenu>		deSerializeMenu(const LcTmString& menuLink, volatile bool& menuLoaded, NdhsCManifest* palManifest, int stackLevel);
		bool						saveState(int *abortSave);
		bool						isMenuItemChild(){return false;}
		void						incMenuRefCount(NdhsCMenu* menu, NdhsCMenuComponent *caller);
#endif /* IFX_SERIALIZATION */

#ifdef IFX_RENDER_DIRECT_OPENGL_20
	LcTmOwner<NdhsCEffect>			m_effect;
	inline NdhsCEffect* 			getEffect()	{	return m_effect.ptr();	}
#endif /* IFX_RENDER_DIRECT_OPENGL_20 */

#ifdef IFX_USE_STYLUS
	void							ignoreEntry(NdhsCElementGroup* group);
#endif

	bool							isAsyncBlockingMessageScheduled();
	void							doAsyncLaunchComplete();
};

#endif // NdhsCPageManagerH
