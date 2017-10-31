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
#ifndef NdhsCMenuWidgetFactoryH
#define NdhsCMenuWidgetFactoryH

#if defined(NDHS_PAINT_DRAG_REGIONS)
	class NdhsCMemoryImage;
#endif

#ifdef LC_USE_STYLUS
	class NdhsCDragRegionElement;
#endif
	class NdhsCTextElement;

/*-------------------------------------------------------------------------*//**
	A generic widget factory for the NDE menuing classes,  offering image,
	video, and mesh-based items for widgets derived from LcwCSelector.
*/
class NdhsCMenuWidgetFactory : public LcCBase
{
	// Different child classes for each type of item
	class CImageItem; friend class CImageItem;
	class CTextItem; friend class CTextItem;
#ifdef LC_USE_MESHES
	class CMeshItem ; friend class CMeshItem ;
#endif

public:
#ifdef IFX_USE_PLUGIN_ELEMENTS
	class CPluginItem; friend class CPluginItem;
#endif

	class CSettings : public LcCBase, public ISerializeable
	{
	protected:
									CSettings()			{}

	public:
		int							frameCount;
		LcTVector					forceExtent;
		LcTVector					defaultExtent;
		LcTmString					fontName;
		LcCFont::EStyle				fontStyle;
		int							fontHeight;
		LcwCLabel::EHorizontalAlign	fontHAlign;
		LcwCLabel::EVerticalAlign	fontVAlign;
		LcTmString					fontAbbrevSuffix;
		int							marqueeSpeed;
		LcCWidget::ESketchyMode		sketchyMode;
		bool						antiAlias;
#ifdef IFX_USE_PLUGIN_ELEMENTS
		LcTmString					eventHandlerLink;
		LcTmString					eventHandler;
#endif
#ifdef LC_USE_LIGHTS
		ENdhsLightType				lightType;
		LcTScalar					attenuationConstant;
		LcTScalar					attenuationLinear;
		LcTScalar					attenuationQuadratic;
		LcTColor					specularColor;
#endif
		NdhsCManifest*				parentPaletteMan;

		int							meshGridX;
		int							meshGridY;

		static LcTaOwner<CSettings>	create();

#ifdef IFX_SERIALIZATION
		static	CSettings*			loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
		virtual	SerializeHandle		serialize(LcCSerializeMaster *serializeMaster,bool force=false);
		virtual	void				deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
				bool				isMenuItemChild(){return false;}
#endif /* IFX_SERIALIZATION */
	};

private:

	/*-------------------------------------------------------------------------*//**
		Text items
	*/
	class CTextItem : public LcwCLabel
	{

	protected:

		// Two-phase construction
								CTextItem() : LcwCLabel() {}
		void					construct(
									const LcTmString&	text,
									ENdhsWidgetType		iconType,
									CSettings*			settings);
	public:

		// Construction and destruction
		static	LcTaOwner<CTextItem> create(
									const LcTmString&	text,
									ENdhsWidgetType		iconType,
									CSettings*			settings);
		virtual					~CTextItem() {}
	};

	/*-------------------------------------------------------------------------*//**
		Image items - standard bitmaps
	*/
	class CImageItem : public LcwCImage
	{
		class CVideoAnimatorItem; friend class CVideoAnimatorItem;
	protected:

		LcTmString				m_iconData;
		LcTmOwner<LcCBitmapFrame> m_iconFrame;

		// Two-phase construction
								CImageItem() : LcwCImage() {}
		void					construct(
										const LcTmString&	packagePath,
										const LcTmString&	iconData,
										ENdhsWidgetType		iconType,
										CSettings*			settings,
										NdhsCPageManager*	pPageManager,
										NdhsCElement*		element,
										NdhsCMenu*			menu,
										NdhsCMenuItem*		menuItem,
										NdhsIFieldContext*	fieldContext,
										int					stackLevel);
	public:
		// Construction and destruction
		static	LcTaOwner<CImageItem> create(
										const LcTmString&	packagePath,
										const LcTmString&	iconData,
										ENdhsWidgetType		iconType,
										CSettings*			settings,
										NdhsCPageManager*	pPageManager,
										NdhsCElement*		element,
										NdhsCMenu*			menu,
										NdhsCMenuItem*		menuItem,
										NdhsIFieldContext* 	fieldContext,
										int					stackLevel);
		virtual					~CImageItem();

		// Helpers
		virtual void			initImage();
		virtual void			freeImage();

		// From LcCWidget
		virtual void			onPrepareForPaint();
	};

#ifdef LC_USE_MESHES
	/*-------------------------------------------------------------------------*//**
		Mesh items - 3D objects
	*/
	class CMeshItem : public LcwCMesh
	{
	protected:

		// Two-phase construction
								CMeshItem() : LcwCMesh() {}
		void					construct(
											const LcTmString&	packagePath,
											const LcTmString&	iconData,
											ENdhsWidgetType		iconType,
											CSettings*			settings,
											NdhsCPageManager*	pPageManager,
											NdhsCElement*		element,
											NdhsCMenu*			menu,
											NdhsCMenuItem*		menuItem,
											int					stackLevel);

	public:

		// Construction and destruction
		static	LcTaOwner<CMeshItem> create(
										const LcTmString&		packagePath,
										const LcTmString&		iconData,
										ENdhsWidgetType			iconType,
										CSettings*				settings,
										NdhsCPageManager*		pPageManager,
										NdhsCElement*			element,
										NdhsCMenu*				menu,
										NdhsCMenuItem*			menuItem,
										int						stackLevel);
		virtual					~CMeshItem();
	};
#endif



#ifdef LC_USE_LIGHTS
	/*-------------------------------------------------------------------------*//**
		Light items
	*/
	class CLightItem : public LcwCLight
	{

	protected:

		// Two-phase construction
								CLightItem();
		void					construct(CSettings* settings);

	public:

		// Construction and destruction
		static	LcTaOwner<CLightItem> create(CSettings*	settings);
		virtual					~CLightItem();
	};
#endif //LC_USE_LIGHTS

	static LcTVector			forceExtent(LcCWidget*	wid,
											int			w,
											int			h,
											LcTVector&	forceBitmapSize,
											bool		keepAspectRatio);

public:
#ifdef IFX_USE_PLUGIN_ELEMENTS
	/*-------------------------------------------------------------------------*//**
		Rectangular regions of screen drawn to by plugin elements (and not by NDHS!)
	*/
	class CPluginItem : public LcwCPlugin, NdhsCPlugin::NdhsCPluginHElement::IObserver
	{
		LC_DECLARE_RTTI(NdhsCMenuWidgetFactory::CPluginItem, LcwCPlugin)

	private:
		NdhsCPlugin::NdhsCPluginHElement				*m_hPluginElement;
		IFX_ELEMENT_PROPERTY 							m_elemProperty;
		bool											m_placementChanged;
		NdhsCPageManager*								m_pPageManager;

		int												m_meshGridX;
		int												m_meshGridY;
		bool											m_activated;

		void					cleanup();

	LC_PRIVATE_INTERNAL_PROTECTED:

		// LcCWidget methods
		virtual void			onRealize();
		virtual void			onRetire();
		virtual void			onPlacementChange(int mask);
		virtual void			onPrepareForPaint();
		virtual void			onPrepareForHide();
		virtual void			doPrepareForPaint();
		virtual void			doPrepareForPaintIfDirty() { doPrepareForPaint(); }

	protected:

		// Two-phase construction
								CPluginItem(NdhsCPageManager*	pPageManager) : LcwCPlugin()
											{ m_pPageManager = pPageManager; }

		void					construct(
										const LcTmString&	packagePath,
										const LcTmString&	iconData,
										ENdhsWidgetType		iconType,
										CSettings*			settings,
										NdhsCPageManager*	pPageManager,
										NdhsCElement*		element,
										NdhsCMenu*			menu,
										NdhsCMenuItem*		menuItem,
										int					stackLevel);

	public:

		// Construction and destruction
		static	LcTaOwner<CPluginItem> create(
										const LcTmString&	packagePath,
										const LcTmString&	iconData,
										ENdhsWidgetType		iconType,
										CSettings*			settings,
										NdhsCPageManager*	pPageManager,
										NdhsCElement*		element,
										NdhsCMenu*			menu,
										NdhsCMenuItem*		menuItem,
										int					stackLevel);
		virtual					~CPluginItem();

		bool					setElementFocus(bool enableFocus);
		void					deactivate();

		NdhsCPlugin::NdhsCPluginHElement* getPluginElement()	{ return m_hPluginElement; }

		// LcCWidget method
		virtual void			setVisible(bool b);
		
		virtual void  			releaseResources();
		virtual void  			reloadResources();

		// NdhsCPlugin::NdhsCPluginHElement::IObserver interface
		void 					switchView(NdhsCPluginElementView* v);
		void					contentsUpdated()				{ revalidate(); }
		bool					makeFullScreen(bool bStartFullScreen);
		LcwCPlugin*				getWidget()						{ return this; }
	};
#endif //IFX_USE_PLUGIN_ELEMENTS

#ifdef LC_USE_STYLUS
	/*-------------------------------------------------------------------------*//**
		Drag Region
	*/
	class CDragRegionItem : public LcCWidget
	{
		LC_DECLARE_RTTI(NdhsCMenuWidgetFactory::CDragRegionItem, LcCWidget)

	private:
#if defined(NDHS_PAINT_DRAG_REGIONS)
		bool					m_renderingEnabled;
		// pointer to memory image that will be rendered to by the drag region element
		LcTmOwner<NdhsCMemoryImage>	m_image;
#endif // NDHS_PAINT_DRAG_REGIONS
		LcTPlaneRect			m_rect;


	protected:

		// Two-phase construction
								CDragRegionItem();
		void					construct();

	public:
		// Construction and destruction
		static	LcTaOwner<CDragRegionItem>
								create();

		void					onRealize();
		void					onRetire();
		bool					getLocalCoords(const LcTPixelPoint& pt, LcTVector& intersection);
		void					switchRenderingMode(bool renderingEnabled);
		bool					contains(const LcTVector& pt, LcTScalar expandRectEdge);

		void				onWantBoundingBox();
		bool				canBeClipped()	{ return true; }
		void				onPaint(const LcTPixelRect& clip);
#if defined(NDHS_PAINT_DRAG_REGIONS)
		void				prepareForViewing();
		void 				cleanupViewing();
#endif // NDHS_PAINT_DRAG_REGIONS

		virtual					~CDragRegionItem();
	};
#endif // LC_USE_STYLUS

	static LcTaOwner<LcCWidget> createWidget(	CSettings*			settings,
												ENdhsWidgetType		iconType,
												const LcTmString&	packagePath,
												const LcTmString&	iconData,
												NdhsCPageManager*	pPageManager,
												NdhsCElement*		element,
												NdhsCMenu*			menu,
												NdhsCMenuItem*		menuItem,
												NdhsIFieldContext*  fieldContext,
												int					stackLevel);
};

#endif // NdhsCMenuWidgetFactoryH

