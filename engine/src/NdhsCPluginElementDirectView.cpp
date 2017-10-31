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

#if defined(IFX_USE_PLUGIN_ELEMENTS) && !defined(LC_OGL_DIRECT)


/*-------------------------------------------------------------------------*//**
	ImageWrapper nested class
*/
LcTaOwner<NdhsCPluginElementDirectView::CImageWrapper> NdhsCPluginElementDirectView::CImageWrapper::create(NdhsCPluginElementDirectView* pOwner)
{
	LcTaOwner<NdhsCPluginElementDirectView::CImageWrapper> ref;
	ref.set(new CImageWrapper(pOwner));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPluginElementDirectView::CImageWrapper::CImageWrapper(NdhsCPluginElementDirectView* owner)
{
	m_pOwner = owner;
	m_space = m_pOwner->getPluginHandle()->getSpace();
}

/*-------------------------------------------------------------------------*//**
	Opaque if embedded element is active
*/
bool NdhsCPluginElementDirectView::CImageWrapper::isOpaque()
{
	return m_pOwner->viewActive();
}

/*-------------------------------------------------------------------------*//**
	Transparent if embedded element is inactive
*/
bool NdhsCPluginElementDirectView::CImageWrapper::isTransparent(
											const LcTPlaneRect&	dest,
											const LcTVector&	scale,
											const LcTPlaneRect*	clip,
											const LcTVector&	hitPos)
{
	return (m_pOwner->viewActive() == false);
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPluginElementDirectView> NdhsCPluginElementDirectView::create(
														NdhsCPlugin::NdhsCPluginHElement* w)
{
	LcTaOwner<NdhsCPluginElementDirectView> ref;
	ref.set(new NdhsCPluginElementDirectView(w));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPluginElementDirectView::NdhsCPluginElementDirectView(
														NdhsCPlugin::NdhsCPluginHElement* w)
							 : NdhsCPluginElementView(w)
{
	m_attachedWidget = NULL;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementDirectView::construct()
{
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementDirectView::attachWidget(LcwCPlugin* pWidget)
{
	if (!pWidget || m_attachedWidget)
		return;

	// Get the element placement in canvas coords
	LcTPixelRect rect;
	calcBoundingBoxCanvas(pWidget, rect);

	// Populate context
	m_context.iClipWidth = rect.getWidth();
	m_context.iClipHeight = rect.getHeight();

	// Port is void* to cater for different graphics subsystems
	m_context.pDisplay = pWidget->getSpace()->getDirectDrawPort();

	LcTaOwner<NdhsCPluginElementDirectView::CImageWrapper> newImage =
									NdhsCPluginElementDirectView::CImageWrapper::create(this);
	m_image = newImage;

	pWidget->setImage((LcIImage*)m_image.ptr());

	m_attachedWidget = pWidget;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementDirectView::detachWidget(LcwCPlugin* pWidget)
{
	if (!pWidget)
		return;

	if (m_attachedWidget == pWidget)
	{
		// Release the direct draw port
		if(m_context.pDisplay)
			pWidget->getSpace()->releaseDirectDrawPort(m_context.pDisplay);

		// Unset the view
		if (pWidget->getImage() == (LcIImage*)m_image.ptr())
		{
			pWidget->setImage(NULL);
		}
		m_attachedWidget = NULL;
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPluginElementDirectView::~NdhsCPluginElementDirectView()
{
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPluginElementDirectView::onWidgetPlacementChanged(LcwCPlugin* pWidget)
{
	bool retVal = true;

	if (getPluginHandle() == NULL || pWidget == NULL)
		return false;

	// According to the spec, we do not reposition when going full-screen
	if (getPluginHandle()->isFullScreen())
		return true;

	// Work out new canvas coords
	LcTPixelRect bbox;
	calcBoundingBoxCanvas(pWidget, bbox);

	// If the position of the direct-draw rectangle has changed...
	if (   m_recOutput.getLeft()	!= bbox.getLeft()
		|| m_recOutput.getTop()		!= bbox.getTop()
		|| m_recOutput.getRight()	!= bbox.getRight()
		|| m_recOutput.getBottom()	!= bbox.getBottom())
	{
		// ...save the new position
		m_recOutput = bbox;

		// ...and inform the plug-in so that it can re-draw
		if (getPluginHandle()->positionElement(	m_recOutput.getLeft(),
											m_recOutput.getTop(),
											m_recOutput.getWidth(),
											m_recOutput.getHeight()) == false)
		{
			retVal = false;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Updates image layer paint rect.
*/
void NdhsCPluginElementDirectView::prepareForPaint(LcwCPlugin* pWidget)
{
	if (pWidget == NULL)
		return;

	// Inform the layer where to paint itself
	pWidget->setPaintRect(calcBoundingBox3D(pWidget));

	return;
}

/*-------------------------------------------------------------------------*//**
	Return the  bounding box in local coords centered around 0,0,0
*/
LcTPlaneRect NdhsCPluginElementDirectView::calcBoundingBox3D(LcCWidget* pWidget)
{
	LcTPlaneRect rect;

	if (pWidget == NULL)
		return rect;

	LcTVector extent = pWidget->getExtent();
	LcTScalar newWidth = (LcTScalar)((int)extent.x);
	LcTScalar newHeight = (LcTScalar)((int)extent.y);

	// If full screen, expand the paint rectangle to size of canvas
	if (getPluginHandle()->isFullScreen())
	{
		LcTPixelRect rect	= pWidget->getSpace()->getCanvasBounds();
		newWidth = (LcTScalar)rect.getWidth();
		newHeight = (LcTScalar)rect.getHeight();
	}

	// Calculate position around center
	rect.setLeft(-(newWidth / 2));
	rect.setTop(newHeight / 2);
	rect.setZDepth(0);
	rect.setRight(newWidth / 2);
	rect.setBottom(-(newHeight / 2));
	return rect;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElementDirectView::calcBoundingBoxCanvas(LcCWidget* pWidget, LcTPixelRect& rect)
{
	if (pWidget == NULL || pWidget->getSpace() == NULL)
		return;

	// Get corners in 3D space
	LcTVector extent = pWidget->getExtent();
	LcTVector location = LcTVector();

	LcTVector tl(location.x - extent.x/2,location.y + extent.y/2, location.z);
	LcTVector br(location.x + extent.x/2, location.y - extent.y/2, location.z);

	// Map to canvas coords, since plug-in implement uses canvas coords
	LcTPixelPoint tlc = pWidget->getSpace()->mapLocalToCanvas(pWidget, tl);
	LcTPixelPoint brc = pWidget->getSpace()->mapLocalToCanvas(pWidget, br);

	// Output values
	rect.setLeft(tlc.x);
	rect.setTop(tlc.y);
	rect.setRight(brc.x);
	rect.setBottom(brc.y);
}

#endif //IFX_USE_PLUGIN_ELEMENTS
