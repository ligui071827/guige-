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
#ifndef NdhsCPluginH
#define NdhsCPluginH

#define NDHS_COUNT_NOT_IMPLEMENTED -1
#define NDHS_COUNT_FAILED -2

#include "inflexionui/engine/inc/NdhsDefs.h"
#include "inflexionui/engine/inc/ifxui_integration.h"
#include "inflexionui/engine/inc/NdhsCExclusivity.h"


class NdhsCMenuCon;
class NdhsCField;
#ifdef IFX_USE_PLUGIN_ELEMENTS
class NdhsCPluginElementView;
class NdhsCPluginElement;
#endif

#if defined(IFX_WIN_PLAYER)
	#include "inflexionui/engine/inc/NdhsCModuleIntegration.h"
	extern LcTmOwner<NdhsCModuleIntegration>	m_moduleIntegration;
#endif /* IFX_WIN_PLAYER */

/*-------------------------------------------------------------------------*//**
	Abstract base class that stores data representing a plugin.
*/
class NdhsCPlugin : public NdhsCExclusivity::IExclusivityHandler, public LcCBase,public ISerializeable
{
public:
	class NdhsCPluginMenu;
	class NdhsCPluginHElement;
	friend class NdhsCPluginMenu;
	friend class NdhsCPluginHElement;

private:
	typedef LcTmMap<LcTmString, LcTmOwner<NdhsCField> >	TmMNameCache;

public:

	class NdhsCPluginMenu : public LcCBase ,public ISerializeable
	{
		friend class NdhsCPlugin;

	private:

		NdhsCPlugin*						m_pluginParent;

		// The plugin session pointer.
		IFX_HMENU							m_menuSession;
		LcTmString							m_menuName;

		NdhsCField* 						createField(	const LcTmString& fieldName,
															int menuItemIndex,
															NdhsCElement* element,
															IFXI_FIELD_MODE& mode,
															IFXI_FIELD_SCOPE& scope,
															IFXI_FIELDDATA_TYPE& variant);

		NdhsCField* 						findField(		const LcTmString& fieldName,
															int menuItemIndex);

		typedef LcTmMap<int, TmMNameCache>	TmMIndexCache;

		TmMIndexCache						m_menuFieldCache;

	protected:

		// Two-phase construction
		LC_IMPORT							NdhsCPluginMenu()								{}
		void								construct(
												NdhsCPlugin* pluginParent,
												IFX_HMENU menuSession,
												const LcTmString& defaultMenuName);

	public:

		static LcTaOwner<NdhsCPluginMenu> 	create(
												NdhsCPlugin* pluginParent,
												IFX_HMENU menuSession,
												const LcTmString& defaultMenuName);

		virtual								~NdhsCPluginMenu();

		// Get the menu count
		int									getItemCount();
		int 								getMenuFirstActiveItem();

		// Retrieve the relevant data.
		inline	bool						getMenuElementData(NdhsCElement* element, const LcTmString& elementClass, LcTmString& returnValue)
												{ return m_pluginParent->getElementData(-1, elementClass, IFX_FIELD, element, this, returnValue) ; }
		inline 	bool						getMenuItemElementData(NdhsCElement* element, int menuItemIndex, const LcTmString& elementClass, LcTmString& returnValue)
												{ return m_pluginParent->getElementData(menuItemIndex, elementClass, IFX_FIELD, element, this, returnValue) ; }
		inline 	bool						getMenuItemElementLink(int menuItemIndex, LcTmString& returnValue)
												{ return m_pluginParent->getElementData(menuItemIndex, "", IFX_ITEM_LINK, NULL, this, returnValue) ; }
		bool								getMenuLOD();

		// Accessors
		IFX_HMENU							getMenuSession()								{ return m_menuSession; }
		LcTaString							getMenuName()									{ return m_menuName; }
#if 0 /*ligui added here*/
		LcTaString							getMenuSortClass();
		LcTaString							getMenuSortType();
		LcTaString							getMenuSortDirection();
#endif

		bool								updateFields(LcTTime timestamp);
		void 								refreshField(	const LcTmString& fieldName,
															int menuItemIndex);
		void								setPluginFieldsDirty(bool requiresLaundry);
		void								stopFields();
		void								clearFieldCache();

		void								activeItemUpdated(int itemIndex);
#ifdef IFX_SERIALIZATION
	virtual SerializeHandle					serialize(LcCSerializeMaster *serializeMaster,bool force=false){return -1;}
			bool							isMenuItemChild(){return false;}
#endif /* IFX_SERIALIZATION */
	};

#ifdef IFX_USE_PLUGIN_ELEMENTS
	class NdhsCPluginHElement : public LcCBase
	{
	public:
		// Observer interface to allow module-side (and other) events to be broadcast
		class IObserver
		{
		public:
			// whenever the view changes, for whatever reason
			virtual void					switchView(NdhsCPluginElementView* v)	= 0;
			// notify that the contents of the element image have changed
			virtual void					contentsUpdated()						= 0;
			// Element would like the observer to go 'full screen'
			virtual bool					makeFullScreen(bool bStartFullScreen)	= 0;
			// whenever the view changes, for whatever reason
			virtual LcwCPlugin*				getWidget()								= 0;
		};

		friend class NdhsCPlugin;

	private:

		NdhsCPlugin*						m_pluginParent;
		NdhsCPageManager*					m_pageManager;
		LcCSpace*							m_space;

		// The plugin session pointer.
		IFX_HELEMENT						m_pluginElement;

		int									m_refCount;
		int									m_realizeCount;
		int									m_activateCount;
		bool								m_resourcesHeld;
		bool								m_isFullScreen;

		bool								m_paintedThisFrame;

		IFX_ELEMENT_PROPERTY				m_cachedProperties;		
		IFX_ELEMENT_MODE					m_currentMode;
		LcTmOwner<NdhsCPluginElementView>	m_view;
		LcTmArray<IObserver*>				m_observers;

	private:
		void								destroyView();

		void								switchMode(IFX_ELEMENT_MODE newMode);

	protected:

		// Two-phase construction
		LC_IMPORT							NdhsCPluginHElement()								{}
		void								construct(
												NdhsCPlugin*		pluginParent,
												NdhsCPageManager*	pageManager,
												IFX_HELEMENT		pluginElement,
												IFX_ELEMENT_PROPERTY *pProperty);

	public:

		static LcTaOwner<NdhsCPluginHElement> create(
												NdhsCPlugin*		pluginParent,
												NdhsCPageManager*	pageManager,
												IFX_HELEMENT		pluginElement,
												IFX_ELEMENT_PROPERTY *pProperty);

		virtual								~NdhsCPluginHElement();

		IFX_HELEMENT						getPluginElement() { return m_pluginElement; }
		bool								isPluginElement(IFX_HELEMENT hElement) { return hElement == m_pluginElement; }

		// plugin element methods.
		bool								activateElement();
		bool								deactivateElement();
		bool								setElementFocus(bool enableFocus);
		bool								isActive()				{ return m_activateCount != 0; }

		// Wrapper methods
		bool								processElementKeyDownEvent(int key);
		bool								processElementKeyUpEvent(int key);
		int									getElementCaretPosition();
		bool								positionElement(int x, int y, int w, int h);
		bool								paintElement();
		bool								processElementStylusDownEvent(int x, int y);
		bool								processElementStylusUpEvent(int x, int y);
		bool								processElementStylusDragEvent(int x, int y);
		void								processElementStylusCancelEvent();

		void								acquire() { ++m_refCount; }
		void								release() { --m_refCount; }
		int									getRefCount() { return m_refCount; }
		IFX_ELEMENT_PROPERTY*				getCachedProperties() { return &m_cachedProperties; }
		void								realize(LcCSpace* pSpace);
		void								retire();

		void								releaseResources();
		void								reloadResources();

		void								refreshBufferedElement();
		bool								resizeBufferedElement(int w, int h);

		void								addObserver(IObserver* obs);
		void								removeObserver(IObserver* obs);

		IFX_ELEMENT_MODE					getMode() { return m_currentMode; }
		void								modeChangeOnAnimation(bool atAnimationStart); 

		bool								makeFullScreen(bool bStartFullScreen);
		bool								isFullScreen() { return m_isFullScreen; }
		LcCSpace*							getSpace() { return m_space; }
		bool								onWidgetPlacementChanged(LcwCPlugin* pWidget);
		void								prepareForPaint(LcwCPlugin* pCurrentWidget);
		bool								checkPaintElement();
		void								frameStart()			{ m_paintedThisFrame = false; }
		void								frameEnd();
	};
#endif

private:

	// The plugin session pointer.
	IFX_HIL									m_pluginSession;

	NdhsCMenuCon*							m_con;
	LcTmOwner<NdhsCExclusivity> 			m_exclusivity;

	TmMNameCache 							m_globalFieldCache;

	inline NdhsCExclusivity*				getExclusivityPtr() 	{ return m_exclusivity.ptr(); }
	int										getExclusivityCount();

	NdhsCField* 							createField(	const LcTmString& fieldName,
															NdhsCElement* element,
															IFXI_FIELD_MODE& mode,
															IFXI_FIELDDATA_TYPE& variant);

	NdhsCField* 							findField(		const LcTmString& fieldName);

protected:

	// Plugin functions, protected because set by derived class
	LPFN_IFXI_INITIALIZE					m_pPluginInitialize;
	LPFN_IFXI_SHUTDOWN						m_pPluginShutDown;
	LPFN_IFXI_VALIDATEGUID					m_pPluginValidateGUID;
	LPFN_IFXI_GETLINKTYPECOUNT				m_pPluginGetLinkTypeCount;
	LPFN_IFXI_GETLINKTYPEDATA				m_pPluginGetLinkTypeData;
	LPFN_IFXI_GETEXCLUSIVITYCOUNT			m_pPluginGetExclusivityCount;
	LPFN_IFXI_OPENMENU						m_pPluginOpenMenu;
	LPFN_IFXI_CLOSEMENU						m_pPluginCloseMenu;
	LPFN_IFXI_GETITEMCOUNT					m_pPluginGetItemCount;
	LPFN_IFXI_GETFIRSTACTIVEITEM			m_pPluginGetFirstActiveItem;
    LPFN_IFXI_SETACTIVEITEM                 m_pPluginSetActiveItem;
	LPFN_IFXI_GETFIELDINFO					m_pPluginGetFieldInfo;
    LPFN_IFXI_GETFIELDSIZE					m_pPluginGetFieldSize;
	LPFN_IFXI_GETFIELDDATA					m_pPluginGetFieldData;
    LPFN_IFXI_GETFIELDRAW					m_pPluginGetFieldRaw;
    LPFN_IFXI_SETFIELDRAW					m_pPluginSetFieldRaw;
    LPFN_IFXI_GETFIELDSIZEFROMRAW			m_pPluginGetFieldSizeFromRaw;
	LPFN_IFXI_EXECUTELINK					m_pPluginExecuteLink;
	LPFN_IFXI_EXCLUSIVITYSTATUSCHANGE		m_pPluginExclusivityStatusChange;
#ifdef IFX_USE_PLUGIN_ELEMENTS
	LPFN_IFXI_CREATEELEMENT					m_pPluginCreateElement;
	LPFN_IFXI_DESTROYELEMENT				m_pPluginDestroyElement;
	LPFN_IFXI_ACTIVATEELEMENT				m_pPluginActivateElement;
	LPFN_IFXI_DEACTIVATEELEMENT				m_pPluginDeactivateElement;
	LPFN_IFXI_SETELEMENTFOCUS				m_pPluginSetElementFocus;
	LPFN_IFXI_UNSETELEMENTFOCUS				m_pPluginUnsetElementFocus;
	LPFN_IFXI_PROCESSELEMENTKEYDOWNEVENT	m_pProcessElementKeyDownEvent;
	LPFN_IFXI_PROCESSELEMENTKEYUPEVENT		m_pProcessElementKeyUpEvent;
	LPFN_IFXI_GETELEMENTCARETPOSITION		m_pGetElementCaretPosition;
	LPFN_IFXI_POSITIONELEMENT				m_pPluginPositionElement;
	LPFN_IFXI_PROCESSELEMENTSTYLUSDOWNEVENT m_pProcessElementStylusDownEvent;
	LPFN_IFXI_PROCESSELEMENTSTYLUSUPEVENT	m_pProcessElementStylusUpEvent;
	LPFN_IFXI_PROCESSELEMENTSTYLUSDRAGEVENT m_pProcessElementStylusDragEvent;
	LPFN_IFXI_PROCESSELEMENTSTYLUSCANCELEVENT m_pProcessElementStylusCancelEvent;
	LPFN_IFXI_PAINTELEMENT					m_pPluginPaintElement;
	LPFN_IFXI_CHANGEELEMENTMODE				m_pPluginChangeElementMode;	
#endif

	// Abstract so keep constructor protected
	LC_IMPORT								NdhsCPlugin(NdhsCMenuCon* con);
	void 									construct();

	// Accessor mutators
	IFX_HIL									getPluginSession()								{ return m_pluginSession; }
	void									setPluginSession(IFX_HMENU pluginSession)		{ m_pluginSession = pluginSession; }

	// For derived class to use to set up entry points
	virtual			bool					loadPlugin();

public:

	static LcTaOwner<NdhsCPlugin>			create(NdhsCMenuCon* con);

	// Plugin initialization method.
	bool									initialize();

	// Plug-in shutdown method.
	// This should be called just before the plug-in is unloaded.
	void									shutdown();

	// GUID validation
	bool									validateGUID(const LcTmString& guid);


	// Access to the menu con for the callback function
	inline NdhsCMenuCon*					getCon() { return m_con; }

	// Plugin API wrapper methods.
	int										getLinkTypeCount();
	ENdhsLinkType							getLinkTypeData(int linkTypeIndex, LcTmString& linkTypePrefix);

	bool									getElementData(	int					menuItemIndex,
															const LcTmString&	elementClass,
															IFX_FIELD_TYPE		dataType,
															NdhsCElement*		element,
															NdhsCPluginMenu*	menu,
															LcTmString&			returnValue);

	bool									requestLiveElementData(
	    									                int					menuItemIndex,
															const LcTmString&	elementClass,
															IFX_FIELD_TYPE		dataType,
															NdhsCElement*		element,
															NdhsCPluginMenu*	menu,
															LcTmString&			returnValue,
															IFXI_FIELD_SCOPE*   scope);

	bool									getRawElementData(
	    									                int					menuItemIndex,
															const LcTmString&	elementClass,
															NdhsCElement*		element,
															NdhsCPluginMenu*	menu,
															IFXI_VARIANT_TYPE&  returnValue,
															IFXI_FIELD_SCOPE*   pScope);

	bool									translateRawElementData(
															const LcTmString&	elementClass,
															NdhsCPluginMenu*	menu,
															IFXI_VARIANT_TYPE	dataValue,
															LcTmString&			returnValue);

	bool									notifyFieldValueRequest(
	    									                int					menuItemIndex,
															const LcTmString&	elementClass,
															NdhsCPluginMenu*	menu,
															IFXI_VARIANT_TYPE	newValue,
															bool				finalValue);

	// Menu plugin wrappers.
	LcTaOwner<NdhsCPluginMenu>				openMenu(const LcTmString& menuLink);


	NdhsCField*								getField(		const LcTmString& fieldName,
															NdhsCPluginMenu* menu,
															int menuItemIndex,
															NdhsCElement* element);

	bool 									getFieldInfo(const LcTmString& fieldName, NdhsCPlugin::NdhsCPluginMenu* menu, int menuItemIndex,
												IFXI_FIELD_MODE& mode, IFXI_FIELD_SCOPE& scope, IFXI_FIELDDATA_TYPE& variant);

	bool									updateGlobalFields(LcTTime timestamp);
	void									refreshGlobalField(const LcTmString& fieldName);
	void 									setGlobalFieldsDirty(bool requiresLaundry);
	void									stopGlobalFields();

#ifdef IFX_SERIALIZATION
	virtual SerializeHandle					serialize(LcCSerializeMaster *serializeMaster,bool force=false){return -1;}
	bool									isMenuItemChild(){return false;}
#endif /* IFX_SERIALIZATION */

#ifdef IFX_USE_PLUGIN_ELEMENTS
	// Element launcher
	LcTaOwner<NdhsCPluginHElement>			createElement(	NdhsCPluginMenu*	menu,
														    NdhsCPageManager*	pageManager,
															int					item,
															const LcTmString&	link,
															IFX_ELEMENT_PROPERTY *pProperty);
#endif

	// Launch link wrapper.
	bool									launchLink(const LcTmString& link);

	// Implements NdhsCExclusivity::IExclusivityHandler
	virtual IFX_RETURN_STATUS				exclusivityStatusChange(
														IFX_HEXCLUSIVITY 		hExclusivity,
														IFX_EXCLUSIVITY_STATUS 	status);

	virtual									~NdhsCPlugin();

public:
	static IFX_RETURN_STATUS				pluginCallback(
													IFX_HUI				hIfx,
													IFX_CALLBACK_CODE		nCode,
													IFX_HELEMENT			hElement,
													IFX_HMENU				hMenu,
													int						item,
													const void*				pInput,
													void*					pOutput);

	static IFX_RETURN_STATUS				requestExclusivity(
													IFX_HUI				hIfx,
													IFX_HEXCLUSIVITY 	hExclusivity,
													IFX_UINT32			priority,
													IFX_UINT32			timeout);

	static IFX_RETURN_STATUS				releaseExclusivity(
													IFX_HUI				hIfx,
													IFX_HEXCLUSIVITY 	hExclusivity);

	static IFX_RETURN_STATUS				exclusivityPermitted(
													IFX_HUI				hIfx,
													IFX_HEXCLUSIVITY 	hExclusivity,
													IFX_INT32		   *pResult);
};

#endif	//NdhsCPluginH
