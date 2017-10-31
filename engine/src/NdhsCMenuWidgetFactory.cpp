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


#include <stdlib.h>
#include <math.h>

#define FACTORY_DEFAULT_FONT_HEIGHT			12
#define FACTORY_DEFAULT_ABBREV_SUFFIX		"..."


/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuWidgetFactory::CSettings>	NdhsCMenuWidgetFactory::CSettings::create()
{
	LcTaOwner<CSettings> ref;
	ref.set(new CSettings());
//	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Create a widget for use by the LcwCList
*/
LcTaOwner<LcCWidget> NdhsCMenuWidgetFactory::createWidget(CSettings* settings,
														ENdhsWidgetType iconType,
														const LcTmString& packagePath,
														const LcTmString& iconData,
														NdhsCPageManager* pPageManager,
														NdhsCElement* element,
														NdhsCMenu* menu,
														NdhsCMenuItem* menuItem,
														NdhsIFieldContext* fieldContext,
														int stackLevel)
{
	LcTaOwner<LcCWidget> retItem;

	// What we create depends upon the item's type
	switch (iconType)
	{
		#ifdef LC_USE_MESHES
			// Mesh
			case ENdhsWidgetTypeColoredMesh:
			{
				LcTaOwner<CMeshItem> newWidget = CMeshItem::create(packagePath, iconData, iconType, settings, pPageManager, element, menu, menuItem, stackLevel);
				retItem = newWidget;
				break;
			}
		#endif

		case ENdhsWidgetTypeText:
		{
			LcTaOwner<CTextItem> newWidget = CTextItem::create(iconData, iconType, settings);
			retItem = newWidget;
			break;
		}

		// Image
		case ENdhsWidgetTypeBitmap:
		{
			LcTaOwner<CImageItem> newWidget = CImageItem::create(packagePath, iconData, iconType, settings, pPageManager, element, menu, menuItem, fieldContext, stackLevel);
			retItem = newWidget;
			break;
		}

#ifdef IFX_USE_PLUGIN_ELEMENTS
		// Embedded plugin element
		case ENdhsWidgetTypePluginElement:
		{
			LcTaOwner<CPluginItem> newWidget = CPluginItem::create(packagePath, iconData, iconType, settings, pPageManager, element, menu, menuItem, stackLevel);
			retItem = newWidget;
			break;
		}
#endif
#ifdef LC_USE_STYLUS
		// Drag Region
		case ENdhsWidgetTypeDragRegion:
		{
			LcTaOwner<CDragRegionItem> newWidget = CDragRegionItem::create();
			retItem = newWidget;
		}
		break;
#endif // LC_USE_STYLUS
#ifdef LC_USE_LIGHTS
		// Light
		case ENdhsWidgetTypeLight:
		{
			LcTaOwner<CLightItem> newWidget = CLightItem::create(settings);
			retItem = newWidget;
		}
		break;
#endif // LC_USE_LIGHTS

		default:
		{
			break;
		}
	}

	return retItem;
}
#ifdef IFX_SERIALIZATION
NdhsCMenuWidgetFactory::CSettings* NdhsCMenuWidgetFactory::CSettings::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	CSettings *obj=new CSettings();
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCMenuWidgetFactory::CSettings::serialize(LcCSerializeMaster *serializeMaster,bool force)
{
	SerializeHandle handle=-1;
	if(!force)
	{
		handle=serializeMaster->getHandle(this);
		if(handle!=-1 && serializeMaster->isSerialized(handle))
		{
			return handle;
		}
		else if(handle==-1)
		{
			handle=serializeMaster->newHandle(this);
		}
	}
	else
	{
		handle=serializeMaster->newHandle(this);
	}

	int outputSize = sizeof(CSettings)+sizeof(IFX_INT32);
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SERIALIZE(frameCount,serializeMaster,cPtr);
	SERIALIZE(forceExtent,serializeMaster,cPtr);
	SERIALIZE(defaultExtent,serializeMaster,cPtr);
	SERIALIZE_String(fontName,serializeMaster,cPtr);
	SERIALIZE(fontStyle,serializeMaster,cPtr);
	SERIALIZE(fontHeight,serializeMaster,cPtr);
	SERIALIZE(fontHAlign,serializeMaster,cPtr);
	SERIALIZE(fontVAlign,serializeMaster,cPtr);
	SERIALIZE_String(fontAbbrevSuffix,serializeMaster,cPtr);
	SERIALIZE(sketchyMode,serializeMaster,cPtr);
	SERIALIZE(antiAlias,serializeMaster,cPtr);
	SERIALIZE(meshGridX,serializeMaster,cPtr);
	SERIALIZE(meshGridY,serializeMaster,cPtr);
#ifdef IFX_USE_PLUGIN_ELEMENTS
	SERIALIZE_String(eventHandlerLink,serializeMaster,cPtr);
	SERIALIZE_String(eventHandler,serializeMaster,cPtr);
#endif
#ifdef LC_USE_LIGHTS
	SERIALIZE(lightType,serializeMaster,cPtr);
	SERIALIZE(attenuationConstant,serializeMaster,cPtr);
	SERIALIZE(attenuationLinear,serializeMaster,cPtr);
	SERIALIZE(attenuationQuadratic,serializeMaster,cPtr);
	SERIALIZE(specularColor,serializeMaster,cPtr);
#endif
	SERIALIZE_Reserve(parentPaletteMan,serializeMaster,cPtr);

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCMenuWidgetFactory::CSettings::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	DESERIALIZE(frameCount,serializeMaster,cPtr);
	DESERIALIZE(forceExtent,serializeMaster,cPtr);
	DESERIALIZE(defaultExtent,serializeMaster,cPtr);
	DESERIALIZE_String(fontName,serializeMaster,cPtr);
	DESERIALIZE(fontStyle,serializeMaster,cPtr);
	DESERIALIZE(fontHeight,serializeMaster,cPtr);
	DESERIALIZE(fontHAlign,serializeMaster,cPtr);
	DESERIALIZE(fontVAlign,serializeMaster,cPtr);
	DESERIALIZE_String(fontAbbrevSuffix,serializeMaster,cPtr);
	DESERIALIZE(sketchyMode,serializeMaster,cPtr);
	DESERIALIZE(antiAlias,serializeMaster,cPtr);
	DESERIALIZE(meshGridX,serializeMaster,cPtr);
	DESERIALIZE(meshGridY,serializeMaster,cPtr);
#ifdef IFX_USE_PLUGIN_ELEMENTS
	DESERIALIZE_String(eventHandlerLink,serializeMaster,cPtr);
	DESERIALIZE_String(eventHandler,serializeMaster,cPtr);
#endif
#ifdef LC_USE_LIGHTS
	DESERIALIZE(lightType,serializeMaster,cPtr);
	DESERIALIZE(attenuationConstant,serializeMaster,cPtr);
	DESERIALIZE(attenuationLinear,serializeMaster,cPtr);
	DESERIALIZE(attenuationQuadratic,serializeMaster,cPtr);
	DESERIALIZE(specularColor,serializeMaster,cPtr);
#endif
	DESERIALIZE_Reserve(parentPaletteMan,serializeMaster,cPtr,NdhsCManifest);
}
#endif /* IFX_SERIALIZATION */
/*-------------------------------------------------------------------------*//**
	Forces the extent of a widgets
*/
LcTVector NdhsCMenuWidgetFactory::forceExtent(LcCWidget* wid, int w, int h, LcTVector& forceBitmapSize, bool keepAspectRatio)
{
	// Force size if required
	if (forceBitmapSize.equals(LcTVector(0,0,0)))
	{
		LcTVector defaultExtent((LcTScalar)w, (LcTScalar)h, 1);
		wid->setExtent(defaultExtent, false);
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
		wid->setImageSize(w, h);
#endif
		return defaultExtent;
	}
	else if (keepAspectRatio)
	{
		LcTScalar x;
		LcTScalar y;

		if (w > h)
		{
			y = LcTScalar(h) * LcTScalar(forceBitmapSize.x) / LcTScalar(w);
			x = forceBitmapSize.x;

			if (y > forceBitmapSize.y)
			{
				x = LcTScalar(x) * LcTScalar(forceBitmapSize.y) / LcTScalar(y);
				y = LcTScalar(forceBitmapSize.y);
			}
		}
		else
		{
			x = LcTScalar(w) * LcTScalar(forceBitmapSize.y) / LcTScalar(h);
			y = forceBitmapSize.y;

			if (x > forceBitmapSize.x)
			{
				y = LcTScalar(y) * LcTScalar(forceBitmapSize.x) / LcTScalar(x);
				x = LcTScalar(forceBitmapSize.x);
			}
		}

		LcTVector defaultExtent(x, y, 1);
		wid->setExtent(defaultExtent, false);

		return defaultExtent;
	}
	else
	{
		LcTVector defaultExtent(forceBitmapSize.x, forceBitmapSize.y, 1);
		wid->setExtent(forceBitmapSize.x, forceBitmapSize.y, 1, false);

		return defaultExtent;
	}
}

/*-------------------------------------------------------------------------*//**
	CTEXTITEM CLASS
*/

/*-------------------------------------------------------------------------*//**
	Create CTextItem
*/
LcTaOwner<NdhsCMenuWidgetFactory::CTextItem>
	NdhsCMenuWidgetFactory::CTextItem::create(const LcTmString& text,
											  ENdhsWidgetType iconType,
											  CSettings* settings)
{
	LcTaOwner<NdhsCMenuWidgetFactory::CTextItem> ref;
	ref.set(new NdhsCMenuWidgetFactory::CTextItem);
	ref->construct(text, iconType, settings);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Construct CTextItem
*/
void NdhsCMenuWidgetFactory::CTextItem::construct(const LcTmString& text,
												  ENdhsWidgetType iconType,
												  CSettings* settings)
{
	LC_UNUSED(iconType)

	setCaption(text);
	setSketchyMode(settings->sketchyMode);
	setAntiAlias(settings->antiAlias);
	setFont(settings->fontName, settings->fontStyle);
	setFontHeight((LcTScalar)settings->fontHeight);
	setAbbrevSuffix(settings->fontAbbrevSuffix);
	setHorizontalAlign(settings->fontHAlign);
	setVerticalAlign(settings->fontVAlign);
	setMeshGrid(settings->meshGridX, settings->meshGridY);

	setTranslucent(true);
}

/*-------------------------------------------------------------------------*//**
	CIMAGEITEM CLASS
*/

/*-------------------------------------------------------------------------*//**
	Create CImageItem
*/
LcTaOwner<NdhsCMenuWidgetFactory::CImageItem>
	NdhsCMenuWidgetFactory::CImageItem::create(const LcTmString& packagePath,
											   const LcTmString& iconData,
											   ENdhsWidgetType iconType,
											   CSettings* settings,
											   NdhsCPageManager* pPageManager,
											   NdhsCElement* element,
											   NdhsCMenu* menu,
											   NdhsCMenuItem* menuItem,
											   NdhsIFieldContext* fieldContext,
											   int stackLevel)
{
	LcTaOwner<NdhsCMenuWidgetFactory::CImageItem> ref;
	ref.set(new NdhsCMenuWidgetFactory::CImageItem);
	ref->construct(packagePath, iconData, iconType, settings, pPageManager, element, menu, menuItem, fieldContext, stackLevel);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Construct CImageItem
*/
void NdhsCMenuWidgetFactory::CImageItem::construct(const LcTmString& packagePath,
												   const LcTmString& iconData,
												   ENdhsWidgetType iconType,
												   CSettings* settings,
												   NdhsCPageManager* pPageManager,
												   NdhsCElement* element,
												   NdhsCMenu* menu,
												   NdhsCMenuItem* menuItem,
												   NdhsIFieldContext* fieldContext,
												   int stackLevel)
{
	LC_UNUSED(iconType)

	m_iconData	= iconData;

	setExtent(LcTVector(1, 1, 1), false);

	setSketchyMode(settings->sketchyMode);
	setAntiAlias(settings->antiAlias);
	setMeshGrid(settings->meshGridX, settings->meshGridY);

	LcTVector forceBitmapSize = settings->forceExtent;

	// Keep the aspect ratio only if all margins are 0

	// Now load the icon with the package path
	LcTaString iconFullPath;
	pPageManager->getTokenStack()->replaceTokens(
												packagePath + m_iconData,
												iconFullPath,
												element,
												menu,
												menuItem,
												fieldContext,
												stackLevel);

	// Check if the item is referenced in an external palette using the full palette path.
	// If it is then ignore its parent palette manifest.
	NdhsCManifest* iconPaletteMan = pPageManager->getPaletteManifest(iconFullPath);
	if (NULL == iconPaletteMan)
		iconPaletteMan = settings->parentPaletteMan;

	LcCBitmap* icon = pPageManager->getBitmap(iconFullPath, stackLevel, iconPaletteMan);
	if (icon == NULL)
	{
		// If not loaded and there was a package path,
		// try loading without it
		if (packagePath.length() > 0)
		{
			LcTaString iconShortPath;
			pPageManager->getTokenStack()->replaceTokens(m_iconData,
														iconShortPath,
														element,
														menu,
														menuItem,
														fieldContext,
														stackLevel);

			icon = pPageManager->getBitmap(iconShortPath, stackLevel, settings->parentPaletteMan);
		}
	}

	if (icon)
	{
		// Only load image if header framecount value matches template one
		if (icon->getFrameCount() == settings->frameCount)
		{
			LcTaOwner<LcCBitmapFrame> iconFrame = LcCBitmapFrame::create();
			m_iconFrame = iconFrame;

			m_iconFrame->setBitmap(icon);

			setTranslucent(icon->isTranslucent());

			// Use the icon if we loaded it successfully
			setImage((LcIImage*)m_iconFrame.ptr());
		}
		else
		{
			// We must acquire and release the bitmap to free it
			icon->acquire();
			icon->release();

			// Display an error to the UIDesigner about this problem.
			NDHS_TRACE_EXT(ENdhsTraceLevelError,
							ENdhsTraceModuleTemplate,
							"The Frame Count for the image does not match the Frame Count in the template.",
							packagePath + m_iconData,
							-1);
		}
	}
	else
	{
		setImage(NULL);
	}

	if (getImage())
	{
		// Keep the aspect ratio only if all margins are 0
		bool keepAspectRatio = true;
		if (icon->getMarginTop() != 0
			|| icon->getMarginBottom() != 0
			|| icon->getMarginLeft() != 0
			|| icon->getMarginRight() != 0)
		{
			keepAspectRatio = false;
		}

		settings->defaultExtent = NdhsCMenuWidgetFactory::forceExtent(this,
																	getImage()->getSize().width,
																	getImage()->getSize().height,
																	forceBitmapSize,
																	keepAspectRatio);
	}
}

/*-------------------------------------------------------------------------*//**
	Destroy CImageItem
*/
NdhsCMenuWidgetFactory::CImageItem::~CImageItem()
{
	freeImage();
}

/*-------------------------------------------------------------------------*//**
	Initializes the image for an item
*/
void NdhsCMenuWidgetFactory::CImageItem::initImage()
{
}

/*-------------------------------------------------------------------------*//**
	Throws away the image for an item
*/
void NdhsCMenuWidgetFactory::CImageItem::freeImage()
{
	// Don't have to free image explicitly as it is reference counted
	setImage(NULL);
}

/*-------------------------------------------------------------------------*//**
	Prepares the item for painting
*/
void NdhsCMenuWidgetFactory::CImageItem::onPrepareForPaint()
{
	LcCBitmapFrame* bmf = (LcCBitmapFrame*)getImage();

	if (bmf)
	{
		bmf->setFrame(getFrame());
		LcwCImage::onPrepareForPaint();
	}
}

#ifdef LC_USE_MESHES
/*-------------------------------------------------------------------------*//**
	CMESHITEM CLASS
*/

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CMeshItem::construct(const LcTmString& packagePath,
												  const LcTmString& iconData,
												  ENdhsWidgetType iconType,
												  CSettings* settings,
												  NdhsCPageManager* pPageManager,
												  NdhsCElement* element,
												  NdhsCMenu* menu,
												  NdhsCMenuItem* menuItem,
												  int stackLevel)
{
	LcwCMesh::construct();

	setSketchyMode(settings->sketchyMode);
	setAntiAlias(settings->antiAlias);

	setExtent(LcTVector(1, 1, 1), false);

	LcTVector forceMeshSize = settings->forceExtent;
	LcTaString iconFullPath = packagePath + iconData;

	// Check if the item is referenced in an external palette using the full palette path.
	// If it is then ignore its parent palette manifest.
	NdhsCManifest* meshPaletteMan = pPageManager->getPaletteManifest(iconFullPath);
	if (NULL == meshPaletteMan)
		meshPaletteMan = settings->parentPaletteMan;

	// Now load the mesh with the package path
	LcCMesh* mesh = pPageManager->getMesh(iconFullPath, element, menu, menuItem, stackLevel, meshPaletteMan);
	if (mesh == NULL)
	{
		// If not loaded and there was a package path,
		// try loading without it
		if (packagePath.length() > 0)
			mesh = pPageManager->getMesh(iconData, element, menu, menuItem, stackLevel, settings->parentPaletteMan);
	}

	if (mesh)
	{
		// The internal extent is set to contain the furthest edge in each plane.
		LcTVector meshBoundary(2 * max(lc_fabs(mesh->getMinX()), lc_fabs(mesh->getMaxX())),
										2 * max(lc_fabs(mesh->getMinY()), lc_fabs(mesh->getMaxY())),
										2 * max(lc_fabs(mesh->getMinZ()), lc_fabs(mesh->getMaxZ())));

		// If the extent hint has not been provided.
		if (forceMeshSize.equals(LcTVector(0, 0, 0)))
		{
			LcTVector defaultMeshBoundary(lc_fabs(mesh->getMaxX()-mesh->getMinX()),
				lc_fabs(mesh->getMaxY()-mesh->getMinY()),
				lc_fabs(mesh->getMaxZ()-mesh->getMinZ()));

			// Setup the default extent as the smallest of the screen width or height scaled by the ScaledBoundingRadius / 10.
			// The ScaledBoundingRadius is the sourceMeshBoundingRadius scaled to the range [1, 10), i.e.
			// ScaledBoundingRadius = sourceMeshBoundingRadius / 10^(floor(log10(max bounding box dimension in mesh units)))
			int scrnWidth;
			int scrnHeight;
			pPageManager->getCurrentScreenSize(scrnWidth, scrnHeight);
			LcTScalar shortestScreenEdge = (LcTScalar)((min(scrnWidth, scrnHeight) ));

			LcTScalar meshConversionScale = shortestScreenEdge * mesh->getScaledBoundingRadius() / 10;
			settings->defaultExtent.x = (defaultMeshBoundary.x*defaultMeshBoundary.x * meshConversionScale)/meshBoundary.x;
			settings->defaultExtent.y = (defaultMeshBoundary.y*defaultMeshBoundary.y * meshConversionScale)/meshBoundary.y;
			settings->defaultExtent.z = (defaultMeshBoundary.z*defaultMeshBoundary.z * meshConversionScale)/meshBoundary.z;
		}
		else
		{
			// Setup the extent hint to fit into the specified extent hint whilst
			// maintaining aspect ratio.
			// This will also use the mesh display scale.
			LcTVector meshExtentScale(forceMeshSize.x / meshBoundary.x,
										forceMeshSize.y / meshBoundary.y,
										forceMeshSize.z / meshBoundary.z);

			LcTScalar meshConversionScale = min(meshExtentScale.x, min(meshExtentScale.y, meshExtentScale.z));
			LcTScalar meshDisplayScale = mesh->getDScale();
			settings->defaultExtent.x = (meshBoundary.x * meshDisplayScale) * meshConversionScale;
			settings->defaultExtent.y = (meshBoundary.y * meshDisplayScale) * meshConversionScale;
			settings->defaultExtent.z = (meshBoundary.z * meshDisplayScale) * meshConversionScale;
		}

		setExtent(settings->defaultExtent, false);
	}

	setMesh(mesh);
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuWidgetFactory::CMeshItem>
	NdhsCMenuWidgetFactory::CMeshItem::create(const LcTmString& packagePath,
											  const LcTmString& iconData,
											  ENdhsWidgetType iconType,
											  CSettings* settings,
											  NdhsCPageManager* pPageManager,
											  NdhsCElement* element,
											  NdhsCMenu* menu,
											  NdhsCMenuItem* menuItem,
											  int stackLevel)
{
	LcTaOwner<CMeshItem> ref;
	ref.set(new CMeshItem);
	ref->construct(packagePath, iconData, iconType, settings, pPageManager, element, menu, menuItem, stackLevel);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuWidgetFactory::CMeshItem::~CMeshItem()
{
	// Don't have to free mesh explicitly as it is reference counted
	setMesh(NULL);
}

#endif

#ifdef IFX_USE_PLUGIN_ELEMENTS
/*-------------------------------------------------------------------------*//**
	CPluginItem CLASS
*/

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::construct(const LcTmString& packagePath,
															const LcTmString& iconData,
															ENdhsWidgetType iconType,
															CSettings* settings,
															NdhsCPageManager* pPageManager,
															NdhsCElement* element,
															NdhsCMenu* menu,
															NdhsCMenuItem* menuItem,
															int stackLevel)

{
	LcwCPlugin::construct();
	setAntiAlias(settings->antiAlias);
	setMeshGrid(settings->meshGridX, settings->meshGridY);

	// Extract link prefix and body from the creation link
	LcTaString linkName;
	pPageManager->getTokenStack()->replaceTokens(iconData, linkName, element, menu, menuItem, NULL, stackLevel);

	LcTmString creationLink;
	creationLink = linkName.subString(0, linkName.find(":"));

	// Populate property structure - if no extent hint
	// has been set on the class then height, width will be -1
	m_elemProperty.mode = IFX_MODE_ELEMENT_INVALID;
	m_elemProperty.translucency = IFX_TRANSLUCENCY_ELEMENT_OPAQUE;
	m_elemProperty.hasPreMultipliedAlpha = IFX_FALSE;
	m_elemProperty.requiredBufferWidth = (int)settings->forceExtent.x;
	m_elemProperty.requiredBufferHeight = (int)settings->forceExtent.y;
	m_elemProperty.requiredBufferFormat = IFX_32BPP_RGBA;

	m_meshGridX = settings->meshGridX;
	m_meshGridY = settings->meshGridY;

	// Now acquire the correct integrated module handle for the link prefix
	NdhsCPlugin* pPlugin = pPageManager->getCon()->getTRLinkTypePlugin(creationLink);

	// Acquire the embedded element instance reference
	if(pPlugin != NULL)
	{
		NdhsCPlugin::NdhsCPluginMenu* menuPlugin = NULL;
		if (menu)
			menuPlugin = menu->getMenuPlugin();

		int item = -1;
		if (menuItem)
			item = menuItem->getIndex();

		m_hPluginElement = pPageManager->getPluginElement(menuPlugin,
																item,
																linkName,
																&m_elemProperty);
	}

	if (m_hPluginElement != NULL)
	{
		// add to element observer list
		m_hPluginElement->addObserver(this);

		// Plugins follow similar rules to the graphic widgets...if the extentHint is set, default extent obeys
		// the extent hint whilst keeping aspect ratios preserved.  If there is no extentHint, then default
		// extent is the size of the buffer.
		settings->defaultExtent = NdhsCMenuWidgetFactory::forceExtent(this,
																		m_elemProperty.requiredBufferWidth,
																		m_elemProperty.requiredBufferHeight,
																		settings->forceExtent,
																		true);
		// Set the translucency property on the widget
		setTranslucent(m_elemProperty.translucency == IFX_TRANSLUCENCY_ELEMENT_NONOPAQUE);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuWidgetFactory::CPluginItem>
	NdhsCMenuWidgetFactory::CPluginItem::create(const LcTmString& packagePath,
															const LcTmString& iconData,
															ENdhsWidgetType iconType,
															CSettings* settings,
															NdhsCPageManager* pPageManager,
															NdhsCElement* element,
															NdhsCMenu* menu,
															NdhsCMenuItem* menuItem,
															int stackLevel)
{
	LcTaOwner<CPluginItem> ref;
	ref.set(new CPluginItem(pPageManager));
	ref->construct(packagePath, iconData, iconType, settings, pPageManager, element, menu, menuItem, stackLevel);

	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuWidgetFactory::CPluginItem::~CPluginItem()
{
	cleanup();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::releaseResources()
{
	if (m_hPluginElement)
		m_hPluginElement->releaseResources();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::reloadResources()
{
	if (m_hPluginElement)
		m_hPluginElement->reloadResources();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::onRealize()
{
	getSpace()->addWidget(this);

	if (m_hPluginElement)
	{
		m_hPluginElement->realize(getSpace());
		m_placementChanged = true;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::onRetire()
{
	if (m_hPluginElement)
		m_hPluginElement->retire();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::onPlacementChange(int mask)
{
	// We defer the actual reposition until as late as possible to avoid
	// the direct repaints getting ahead of the UI repaints
	m_placementChanged = true;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::onPrepareForPaint()
{
	if (m_hPluginElement == NULL)
		return;

	// Normally we only issue reposition calls if the element is
	// active, which is why this deferred event is done here.
	// Note that we may get here for an inactive element, but only
	// if we are about to activate (see below)
	if (m_placementChanged)
	{
		// Let the plugin element decide if the rendering window has changed
		if (m_hPluginElement->onWidgetPlacementChanged(this) == false)
		{
			// Problem updating the view - we destroy the element in this case
			cleanup();
		}

		m_placementChanged = false;
	}

	// Check plugin element again, in case cleaned up above!
	if (m_hPluginElement && (m_activated == false))
	{
		m_activated = true;

		// This is the only point at which we activate the element, because
		// this is called immediately before it is to be painted for real
		if (m_hPluginElement->activateElement())
		{
			// prepare the view for painting (via the plugin element)
			m_hPluginElement->prepareForPaint(this);
		}
		else
		{
			// Problem activating the element - we destroy the widget in this case.
			cleanup();
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::doPrepareForPaint()
{
	if (m_hPluginElement == NULL)
		return;

	// Do the base class actions - this will reset the dirty flag on the widget
	LcCWidget::doPrepareForPaint();

	if (m_hPluginElement && m_hPluginElement->checkPaintElement() == false)
	{
		// If paint element fails then destroy this element
		cleanup();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::onPrepareForHide()
{
	deactivate();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::deactivate()
{
	if ((m_hPluginElement == NULL) || (m_activated == false))
		return;

	m_activated = false;

	if (m_hPluginElement->deactivateElement() == false)
	{
		// Problem deactivating the element - we destroy the widget in this case
		cleanup();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::cleanup()
{
	// Ensure the blackout layer flag is unset
	setBlackoutStatus(false);

	if (m_hPluginElement == NULL)
		return;

	// remove from element observer list
	m_hPluginElement->removeObserver(this);

	// release reference
	m_pPageManager->releasePluginElement(m_hPluginElement);

	m_hPluginElement = NULL;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CPluginItem::setVisible(bool b)
{
	// Let base class react
	LcCWidget::setVisible(b);

	if (m_hPluginElement == NULL)
		return;

	// Now update element and view if necessary
	if (!b)
		onPrepareForHide();
	else
		revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuWidgetFactory::CPluginItem::setElementFocus(bool enableFocus)
{
	// cannot put this in the header file cos it would need a include of NdhsCPluginElementView 
	// causing a circular dependancy
	return m_hPluginElement ? m_hPluginElement->setElementFocus(enableFocus) : false;  
}

/*-------------------------------------------------------------------------*//**
	Plugin element view has changed, update as necessary
*/
void NdhsCMenuWidgetFactory::CPluginItem::switchView(NdhsCPluginElementView* v)
{
	// Check for canvas units
	if (v && v->usesCanvasUnits())
	{
		/*	Causes the internal x,y coordinates of the widget to correspond to screen pixels.
			This assumes that neither the widget nor any of its parent aggregates are
			scaled or rotated.  Actual size and internal extent are mutually exclusive.
			If getInternalExtent() is called on a widget with actual size enabled, the x,y
			components will report the pixel area corresponding to the external extent
			set with setExtent().
		*/
		setInternalTypeToPixels();
		setXfmsDirty();
	}
}

/*-------------------------------------------------------------------------*//**
	Plugin element would like to engage/disengage 'full screen' mode
*/
bool NdhsCMenuWidgetFactory::CPluginItem::makeFullScreen(bool bStartFullScreen)
{
	// Tell base class
	setFullScreen(bStartFullScreen);

	return true;
}
#endif // IFX_USE_PLUGIN_ELEMENTS

#ifdef LC_USE_STYLUS

/*-------------------------------------------------------------------------*//**
	CDragRegionItem
*/
NdhsCMenuWidgetFactory::CDragRegionItem::CDragRegionItem()
	: LcCWidget()
{
#if defined(NDHS_JNI_INTERFACE)
	m_renderingEnabled = false;
#elif defined(NDHS_PAINT_DRAG_REGIONS)
	m_renderingEnabled = true;
#endif
}

/*-------------------------------------------------------------------------*//**
	Create CDragRegionItem
*/
LcTaOwner<NdhsCMenuWidgetFactory::CDragRegionItem>
	NdhsCMenuWidgetFactory::CDragRegionItem::create()
{
	LcTaOwner<NdhsCMenuWidgetFactory::CDragRegionItem> ref;
	ref.set(new NdhsCMenuWidgetFactory::CDragRegionItem);
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Construct CDragRegionItem
*/
void NdhsCMenuWidgetFactory::CDragRegionItem::construct()
{
	m_rect = LcTPlaneRect(0,0,0,0,0);
}

/*-------------------------------------------------------------------------*//**
	onRealize
*/
void NdhsCMenuWidgetFactory::CDragRegionItem::onRealize()
{
#if defined(NDHS_PAINT_DRAG_REGIONS)
	prepareForViewing();
#endif // NDHS_PAINT_DRAG_REGIONS
	getSpace()->addWidget(this);
	LcCWidget::onRealize();
}

/*-------------------------------------------------------------------------*//**
	onRetire
*/
void NdhsCMenuWidgetFactory::CDragRegionItem::onRetire()
{
#if defined(NDHS_PAINT_DRAG_REGIONS)
	cleanupViewing();
#endif // NDHS_PAINT_DRAG_REGIONS
	LcCWidget::onRetire();
}

/*-------------------------------------------------------------------------*//**
	getLocalCoordinate
*/
bool NdhsCMenuWidgetFactory::CDragRegionItem::getLocalCoords(const LcTPixelPoint& pt, LcTVector& intersection)
{
	return getSpace()->mapCanvasToLocal(pt, this, LcTVector(0.0, 0.0, 0.0), intersection);
}

/*-------------------------------------------------------------------------*//**
	set rendring mode on/off
*/
void NdhsCMenuWidgetFactory::CDragRegionItem::switchRenderingMode(bool renderingEnabled)
{
#if defined(NDHS_JNI_INTERFACE)
	m_renderingEnabled = renderingEnabled;
#else
	LC_UNUSED(renderingEnabled);
#endif
}

/*-------------------------------------------------------------------------*//**
	Point lies with in extent of element or not
*/
bool NdhsCMenuWidgetFactory::CDragRegionItem::contains(const LcTVector& pt, LcTScalar expandRectEdge)
{
	if (getTappable()==EFull)
	{
		return m_rect.contains(pt, expandRectEdge);
	}
	return m_rect.contains(pt);
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
NdhsCMenuWidgetFactory::CDragRegionItem::~CDragRegionItem()
{
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CDragRegionItem::onWantBoundingBox()
{
	LcTPlacement placement(getPlacement());

	LcTScalar xDiv2 = placement.extent.x / 2;
	LcTScalar yDiv2 = placement.extent.y / 2;

	m_rect  = LcTPlaneRect (placement.centerOffset.x - xDiv2, placement.centerOffset.y + yDiv2,
							placement.centerOffset.z,
							placement.centerOffset.x + xDiv2, placement.centerOffset.y - yDiv2);

	getSpace()->extendBoundingBox(m_rect);
}

#if defined(NDHS_PAINT_DRAG_REGIONS)
/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CDragRegionItem::prepareForViewing()
{
	LcTPlacement placement(getPlacement());

#if defined(NDHS_JNI_INTERFACE)
	LcTaOwner<NdhsCMemoryImage> newMemImage = NdhsCMemoryImage::create(getSpace(),
																		(int)placement.extent.x,
																		(int)placement.extent.y,
																		IFX_32BPP_RGBA);
	m_image = newMemImage;
	LcTByte* data = m_image->data();

	unsigned int length = (unsigned int)(placement.extent.x * placement.extent.y * 4);

	for (unsigned int i = 0; i < length; i+=4)
	{
		data[i+0] = placement.color.rgba.r;
		data[i+1] = placement.color.rgba.g;
		data[i+2] = placement.color.rgba.b;
		data[i+3] = (unsigned char)(placement.opacity * 255);
	}

#elif defined(NDHS_JNI_INTERFACE)

	LcTaOwner<NdhsCMemoryImage> newMemImage = NdhsCMemoryImage::create(getSpace(), 1, 1, IFX_32BPP_RGBA);
	m_image = newMemImage;
	LcTByte* data = m_image->data();

	data[0] = NDHS_PAINT_DRAG_REGIONS_R;
	data[1] = NDHS_PAINT_DRAG_REGIONS_G;
	data[2] = NDHS_PAINT_DRAG_REGIONS_B;
	data[3] = 0x4C;

#endif
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CDragRegionItem::cleanupViewing()
{
	m_image.destroy();
}
#endif // NDHS_PAINT_DRAG_REGIONS

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuWidgetFactory::CDragRegionItem::onPaint(const LcTPixelRect& clip)
{
// onPaint is a pure virtual function so not enclosing, whole function inside macro
#if defined(NDHS_PAINT_DRAG_REGIONS)
	if (!m_image.ptr() || !m_renderingEnabled)
		return;

	// we are using the cached rect, that we cached in onWantBoundingBox
	m_image->draw(m_rect, clip, LcTColor::WHITE, 1.0, false, 1, 1);

#endif // NDHS_PAINT_DRAG_REGIONS
}
#endif // LC_USE_STYLUS



#ifdef LC_USE_LIGHTS

/*-------------------------------------------------------------------------*//**
	Constructor
*/
NdhsCMenuWidgetFactory::CLightItem::CLightItem()
: 	LcwCLight()
{
}

/*-------------------------------------------------------------------------*//**
	Create CLightItem
*/
LcTaOwner<NdhsCMenuWidgetFactory::CLightItem>
	NdhsCMenuWidgetFactory::CLightItem::create(CSettings* settings)
{
	LcTaOwner<NdhsCMenuWidgetFactory::CLightItem> ref;
	ref.set(new NdhsCMenuWidgetFactory::CLightItem);
	ref->construct(settings);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Construct CLightItem
*/
void NdhsCMenuWidgetFactory::CLightItem::construct(CSettings* settings)
{
	if (settings)
	{
		setAttenuation(settings->attenuationConstant, settings->attenuationLinear, settings->attenuationQuadratic);
		setSpecularColor(settings->specularColor);

		switch (settings->lightType)
		{
		case ENdhsLightTypeBulb:
			setLightType(EBulb);
			break;

		case ENdhsLightTypeDirectional:
			setLightType(EDirectional);
			break;

		default:
			// Do nothing
			break;
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
NdhsCMenuWidgetFactory::CLightItem::~CLightItem()
{
}

#endif //LC_USE_LIGHTS
