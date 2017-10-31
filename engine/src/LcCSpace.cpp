/***************************************************************************
*
*			   Copyright 2006 Mentor Graphics Corporation
*						 All Rights Reserved.
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


#include <math.h>

const int	MSG_FRAME			= 0;
const int	MSG_REPEAT			= 1;

// To what extent background regions are combined for painting
#define BG_COMBINE_THRESHOLD	400

// If two numbers differ by less that this tolerance figure, we consider them
// the same.
#define ZERO_TOLERANCE .001
#if defined(IFX_RENDER_INTERNAL) || defined (LC_PLAT_OGL)
	#define ORDER_3D_ZERO_TOLERANCE .00001
#endif

/*-------------------------------------------------------------------------*//**
*/
class LcTCanvasRegion
{
public:

	// Widget with which this region is associated
	LcCWidget*					m_widget;

	// Region type - widget or background
	// NB: if widget dirtied by overlap, both will be set
	bool						m_bPaintWidget;
	bool						m_bPaintBack;

	// TRUE if this widget/background should be wholly redrawn
	bool						m_bDirty;

	// NON-EMPTY if this widget should be partially redrawn;
	// the array will contain clipping rectangles to apply to widget paints
	LcTmArray<LcTPixelRect>		m_clipRects;

	// Cached information about widget
	LcTPixelRect				m_rect;
	LcTScalar					m_dZ;
#if defined(IFX_RENDER_INTERNAL) || defined (LC_PLAT_OGL)
	LcTScalarQuad				m_quad;
	LcTScalar					m_dZ_max;
	LcTScalar					m_dZ_min;
#endif
	int							m_drawLayerIndex;
	bool						m_bParentAggRotated;
	bool						m_bRotated;
	bool						m_bOpaque;
	bool						m_bBlackout;
	bool						m_bCanBeClipped;
};

/*-------------------------------------------------------------------------*//**
*/
class LcTCapture
{
public:

	LcCWidget*		m_widget;
	bool			m_bLocked;
	LcTCapture*		m_previous;
};

#if defined(_LC_PLAT_WANT_CANVAS)
/*-------------------------------------------------------------------------*//**

	CCanvas Implementation

*/

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCSpace::CCanvas>
LcCSpace::CCanvas::create(LcCSpace* space)
{
	LcTaOwner<CCanvas> ref;
	ref.set(new CCanvas);
	ref->construct(space);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Canvas Constructor.
*/
LC_EXPORT LcCSpace::CCanvas::CCanvas()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCSpace::CCanvas::construct(LcCSpace* space)
{
	m_space		= space;
	m_canvas	= NULL;

	// Create the graphics context for the canvas.
	LcTaOwner<LcCNdiGraphics> newGfx = LcCNdiGraphics::create(this);
	m_gfx = newGfx;
}

/*-------------------------------------------------------------------------*//**
	Canvas Destructor.
*/
LC_EXPORT LcCSpace::CCanvas::~CCanvas()
{
	// Delete graphics and canvas objects.
	m_gfx.destroy();

	destroyCanvas();
}

/*-------------------------------------------------------------------------*//**
	Create a new graphics port canvas.
*/
LC_EXPORT bool LcCSpace::CCanvas::createCanvas(LcTPixelRect& rect, bool isBackground)
{
	bool status = false;
	if (IFX_SUCCESS == IFXP_Canvas_Create(&m_canvas,
										  rect.getWidth(),
										  rect.getHeight(),
										  (isBackground)?IFXP_BACKGROUND:IFXP_FOREGROUND))
	{
		if ( (m_canvas)							 &&
			 ((unsigned int)rect.getWidth() == m_canvas->width)   &&
			 ((unsigned int)rect.getHeight() == m_canvas->height)  )
		{
			status = true;
			m_canvasRect = rect;
		}
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
	Destroy the canvas.
*/
LC_EXPORT void LcCSpace::CCanvas::destroyCanvas()
{
	IFX_RETURN_STATUS status;
	if (m_canvas)
	{
		status = IFXP_Canvas_Destroy(m_canvas);
		m_canvas = NULL;

		if (status != IFX_SUCCESS)
		{
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "Failed to destroy the canvas");
			LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL int LcCSpace::CCanvas::getLineSpan()
{
	int stride = 0;

	// Return number of bytes per line on target canvas
	if (m_canvas)
	{
		stride = m_canvas->stride;
	}

	return stride;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTPixelRect LcCSpace::CCanvas::getBounds()
{
	// Want LcCNdiGraphics to clip to main canvas
	return m_canvasRect;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTByte* LcCSpace::CCanvas::getDataStart()
{
	// First scanline of target canvas
	if (m_canvas)
		return (LcTByte*)m_canvas->colorBuffer;
	else
		return NULL;
}
#endif  // #if defined(_LC_PLAT_WANT_CANVAS)




/*-------------------------------------------------------------------------*//**

	LcCSpace Implementation

*/

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCSpace> LcCSpace::create()
{
	LcTaOwner<LcCSpace> ref;
	ref.set(new LcCSpace);
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCSpace::LcCSpace()
:
//	m_ptOrigin2D(0, 0),
//	m_ptMousePos(0, 0),
	m_msgFrame(this, MSG_FRAME),
	m_msgRepeat(this, MSG_REPEAT)
{
	// NB - string members must be allowed to default to empty!

	m_portraitMode		= true;

	// Other
	m_iFrameInterval	= 30;
//	m_iAnimateInterval	= 0;
	m_iRepeatDelay		= 600;

	// State
//	m_bDirty			= false;
//	m_bPaintScheduled	= false;
//	m_paintWidget		= NULL;
//	m_regionMem			= NULL;
//	m_newRegionMem		= NULL;
//	m_focus				= NULL;

#if defined(NDHS_JNI_INTERFACE)
	m_forceRetainResource = false;
#endif

	// Setup the primary light.
	m_primaryLightMask							= LcTPlacement::EAll;
	m_primaryLightDefaultPlacement.color		= LcTColor::WHITE;
	m_primaryLightDefaultPlacement.color2		= LcTColor::GRAY20;
	m_primaryLightDefaultPlacement.location		= LcTVector(0, 0, 0);
	m_primaryLightDefaultPlacement.orientation	= LcTQuaternion::rotateEuler(0, 0, 0);

	m_primaryLightCurrentPlacement = m_primaryLightDefaultPlacement;
	m_primaryLightPlacement = m_primaryLightCurrentPlacement;

#if !defined(LC_USE_NO_ROTATION)
	m_primaryLightVector.x = 0;
	m_primaryLightVector.y = 0;
	m_primaryLightVector.z = 1;
#endif

	m_defaultFontName		= "Arial";
	m_defaultFontStyle		= LcCFont::REGULAR;
	m_defaultFontColor		= LcTColor::BLACK;
	m_defaultFontColorDisabled	= LcTColor::BLACK;
	m_sBasePath		= "";
	m_sImagePath		= "";
	m_sXMLPath		= "";
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCSpace::construct()
{
#if defined(LC_PLAT_OGL)
	// Create the context.
	m_oglContext = LcOglCContext::create(this);

#endif // defined(LC_PLAT_OGL)

	m_mutex = LcCMutex::create("IFX_SPC");
	m_timerMutex = LcCMutex::create("IFX_TimerQueue");

}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCSpace::constructCanvas()
{
#if defined(_LC_PLAT_WANT_CANVAS)
	// Create two ISurfaces for the graphics to use.
	m_gfxCanvas	= CCanvas::create(this);

	#if !defined(IFX_RENDER_INTERNAL_COMPRESSED)
		m_bgCanvas	= CCanvas::create(this);
	#endif
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCSpace::~LcCSpace()
{
#if defined(_LC_PLAT_WANT_CANVAS)
	if (m_backgroundImageNdi)
		m_backgroundImageNdi.destroy();

	if (m_backgroundImageCustom)
			m_backgroundImageCustom.destroy();

	// Free old pixmaps (might be resizing etc)
	if(m_gfxCanvas)
		m_gfxCanvas->destroyCanvas();

	// Bin old background
	if (m_bgCanvas)
		m_bgCanvas->destroyCanvas();
#endif

#if defined(LC_PLAT_OGL) && defined(LC_USE_EGL)
	destroyEgl();
#endif

#if defined(LC_USE_EGL)
	// Destroy it explicitely because it uses space maps
	// to clear it resources i.e. m_bitmapMap
	m_oglContext.destroy();

	if (m_eglConfig)
		m_eglConfig.free();
#endif

	// Get rid of background region list
	m_regions.clear();
	m_widgets.clear();
	m_regionMem.free();
	m_newRegionMem.free();

	// Clear out owned Bitmaps
	m_bitmapMap.clear();

	// Clear out owned fonts
	m_fontMap.clear();

#ifdef LC_USE_MESHES
	// Clear out owned meshes
	m_meshMap.clear();
#endif
#if defined(LC_USE_LIGHTS)
	m_lightWidgets.clear();

	// Clear out the owned lights.
	m_secOwnedLightPool.clear();
	m_secAllocatedLights.clear();
#endif

#if defined(NDHS_JNI_INTERFACE)
	m_forceRetainResource = false;
#endif
}

#if defined(LC_PLAT_OGL) && defined(LC_USE_EGL)
void LcCSpace::destroyEgl()
{
	eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	if (m_eglSurface != EGL_NO_SURFACE)
	{
		eglDestroySurface(m_eglDisplay, m_eglSurface);
		m_eglSurface = EGL_NO_SURFACE;
	}

	if (m_eglContext != EGL_NO_CONTEXT)
	{
		eglDestroyContext(m_eglDisplay, m_eglContext);
		m_eglContext = EGL_NO_CONTEXT;
	}

	eglTerminate(m_eglDisplay);
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::addApplet(LcCApplet* c)
{
	m_applets.push_back(c);

	// Will need to redraw space with it
	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::removeApplet(LcCApplet* w)
{
	// Find and erase the widget entry
	TmAApplet::iterator it;
	it = find(m_applets.begin(), m_applets.end(), w);
	if (it != m_applets.end())
		m_applets.erase(it);

	// Will need to redraw space without it
	revalidate();
}

/*-------------------------------------------------------------------------*//**
	Adds an active animator, called only by animator class
*/
LC_EXPORT void LcCSpace::addAnimator(LcCAnimator* a)
{
	m_animators.push_back(a);

	// Need to give the animator a chance to animate!
	revalidate();
}

/*-------------------------------------------------------------------------*//**
	Called by an animator when it aborts, to remove from active list
*/
LC_EXPORT void LcCSpace::removeAnimator(LcCAnimator* a)
{
	// Find and zero the animator entry
	// NB: do not erase, as that will mess up iteration and prevent
	// a widget event from animator A from aborting animator B
	TmAAnimator::iterator it;
	it = find(m_animators.begin(), m_animators.end(), a);
	if (it != m_animators.end())
		*it = NULL;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCSpace::canSketch()
{
	// Can be tested for background draws too - disallow sketch on these
	if (!m_paintWidget)
		return false;

	return m_paintWidget->canSketch();
}

/*-------------------------------------------------------------------------*//**
	Configures the 2D canvas reserved for NDE.

	rec is the canvas rectangle relative to the origin of the target control,
	into which the NDE space will be drawn.  This need not occupy the entire
	target control.

	ptOrigin2D is the point within the 2D canvas to which 3D point (0,0,0) will
	be mapped. This is also relative to the origin of the target control, not
	to the origin of the canvas.

	globalExtentWidth sets the bounding cuboid in global 3D space that
	the scene will be laid out in.

	Only a width is specified - the height is calculated such that the extent
	fits exactly into the specified canvas bounds.

	You should ensure that your widgets sit within this cuboid. Any widgets
	outside of the cuboid are liable to be clipped.

	If you set the global extent width to 0,
	then global space will map directly onto canvas space at Z=0.
*/
LC_EXPORT void LcCSpace::configureCanvas(
		const LcTPixelRect& rec,
		const LcTPixelPoint& ptOrigin2D,
		int zDepthPixels,
		LcTScalar globalExtentWidth)
{
	// Store away the requested bounds for future reference.
	m_recBounds = rec;
	m_ptOrigin2D = ptOrigin2D;

	// Defaults
	if (globalExtentWidth == 0)
		globalExtentWidth = LcTScalar(rec.getWidth());
	if (zDepthPixels == 0)
		zDepthPixels = rec.getWidth();

	// Work out height and depth from width ratio
	// NB: LcTScalars are not good enough for this ratio, nor for an intermediate multiply
	LcTScalar height = globalExtentWidth * (float(rec.getHeight()) / rec.getWidth());
	LcTScalar depth = globalExtentWidth * (float(zDepthPixels) / rec.getWidth());
	m_globalExtent = LcTVector(globalExtentWidth, height, depth);

	// Apply settings
	calcProjection();
	boundsUpdated();
	revalidateAll();
}

/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::calcProjection()
{
	// The eye distance is twice the distance from center plane to front,
	// which is half the total z depth of the viewing volume
	LcTScalar dEyeGlobal = m_globalExtent.z;
	LcTScalar dEyeCanvas = dEyeGlobal * m_recBounds.getWidth() / m_globalExtent.x;

	// Fast projection for transforming global coords direct to screen space
	m_xfmCanvas = LcTTransform4::canvas(
				LcTScalar(m_ptOrigin2D.x),
				LcTScalar(m_ptOrigin2D.y),
				dEyeCanvas,
				dEyeGlobal);

	// Must move back by the eye distance so that the eye is at the origin.
	LcTTransform xfmEye = LcTTransform::identity().translatePrefix(0, 0, -dEyeGlobal);
	m_xfmCanvas.multiply(xfmEye);
}

/*-------------------------------------------------------------------------*//**
	Map a point in the global 3D space onto the 2D canvas.
	The point returned is the 2D position at which the point should be drawn
	so that it appears as if it is at the 3D position specified, taking into
	account scaling and perspective.
*/
LC_EXPORT LcTPixelPoint LcCSpace::mapGlobalToCanvas(const LcTVector& vPos)
{
	// Do projection to canvas - Z will be optimized away
	LcTVector v1 = m_xfmCanvas.transformIgnoreZ(vPos);
	return LcTPixelPoint((int)v1.x, (int)v1.y);
}

/*-------------------------------------------------------------------------*//**
	Map a point from the local space of the currently painting widget
	on to the 2D canvas
*/
LC_EXPORT LcTPixelPoint LcCSpace::mapLocalToCanvas(LcCWidget* pWidget, const LcTVector& vPos)
{
	// Still a 2-step mapping ... we could combine the projection matrix
	// into the one stored on the widget, but only for non-OpenGL platforms,
	// and it would become a 4-transform
	return mapGlobalToCanvas(pWidget->mapLocalToGlobal(vPos));
}

/*-------------------------------------------------------------------------*//**
	Map a point in the global 3D space onto the 2D canvas, returning
	subpixel-accurate X and Y in the vector.
	The point returned is the 2D position at which the point should be drawn
	so that it appears as if it is at the 3D position specified, taking into
	account scaling and perspective.
*/
LC_EXPORT LcTVector LcCSpace::mapGlobalToCanvasSubpixel(const LcTVector& vPos)
{
#if defined(IFX_RENDER_INTERNAL) || defined (LC_PLAT_OGL)
	// If we are using 3D NDI bitmaps then pass the Z co-ordinate.
	return m_xfmCanvas.transformWithZ(vPos);
#else
	// Do projection to canvas - Z will be optimized away
	return m_xfmCanvas.transformIgnoreZ(vPos);
#endif
}

/*-------------------------------------------------------------------------*//**
	Map a point from the local space of the currently painting widget
	on to the 2D canvas, returning subpixel-accurate X and Y in the vector
*/
LC_EXPORT LcTVector LcCSpace::mapLocalToCanvasSubpixel(const LcTVector& vPos)
{
	// Still a 2-step mapping ... we could combine the projection matrix
	// into the one stored on the widget, but only for non-OpenGL platforms,
	// and it would become a 4-transform
	return mapGlobalToCanvasSubpixel(m_paintWidget->mapLocalToGlobal(vPos));
}

/*-------------------------------------------------------------------------*//**
	Map a rectangle from the local space of the currently painting widget
	on to the 2D canvas, returning subpixel-accurate X and Y in the vectors
*/
LC_EXPORT LcTScalarQuad LcCSpace::mapLocalToCanvasSubpixel(const LcTPlaneRect& rect)
{
	LcTScalarQuad result;

	// Map each corner in turn.
	result.setTopLeft(mapLocalToCanvasSubpixel(
		LcTVector(rect.getLeft(), rect.getTop(), rect.getZDepth())));
	result.setTopRight(mapLocalToCanvasSubpixel(
		LcTVector(rect.getRight(), rect.getTop(), rect.getZDepth())));
	result.setBottomLeft(mapLocalToCanvasSubpixel(
		LcTVector(rect.getLeft(), rect.getBottom(), rect.getZDepth())));
	result.setBottomRight(mapLocalToCanvasSubpixel(
		LcTVector(rect.getRight(), rect.getBottom(), rect.getZDepth())));

	return result;
}

#ifndef LC_PLAT_OGL

#if !defined(LC_USE_NO_ROTATION)
/*-------------------------------------------------------------------------*//**
*/
LcTScalar LcCSpace::getPaintLighting()
{
	LcTScalar retVal = 1.0f;

	if (m_paintWidget)
	{
		LcTVector vUnitZ(0.0f, 0.0f, 1.0f);

		// Create a rotation matrix.
		if (!m_lastPrimaryLightOrientation.equals(m_primaryLightPlacement.orientation))
		{
			m_lastPrimaryLightOrientation = m_primaryLightPlacement.orientation;
			LcTTransform primaryLightTransform = LcTTransform::rotate(m_lastPrimaryLightOrientation);
			m_primaryLightVector = primaryLightTransform.transform(vUnitZ);
			m_primaryLightVector.normalise();
		}

		// Get the global normal
		LcTVector vNormal = m_paintWidget->getGlobalNormal();

		// In our simple lighting model where we have one infinite light source
		// behind the viewer, lighting on a face is proportional to the cosine
		// of the angle between a normal to the face and the z-axis - i.e., the
		// dot product of the unit normal and the unit z-vector
		LcTScalar dotProd = vNormal.dot(m_primaryLightVector);

		bool isPrimaryLightFacing = (dotProd > 0);

		if (isEyeFacing() == isPrimaryLightFacing)
		{
			retVal = (LcTScalar)lc_fabs(dotProd);
		}
		else
		{
			retVal = 0;
		}
	}

	return retVal;
}
#endif // !LC_USE_NO_ROTATION

/*-------------------------------------------------------------------------*//**
*/
LcTColor LcCSpace::getLitObjectColor(LcTColor baseTint)
{
	LcTColor retVal = baseTint;
	LcTScalar lightLevel = 1.0f;

#if !defined(LC_USE_NO_ROTATION)
	// Determine the lighting level to be applied to this object based on its
	// orientation.  If lighting is not full, reduce light via a color tint using
	// the current Ambient color.
	lightLevel = getPaintLighting();
#endif

	// Combine all the lights.
	// This is designed to simulate opengl.
	if (baseTint.isWhite())
	{
		// If tinted white, just take the light color
		retVal.rgba.r = min(255, (int)(lightLevel * m_primaryLightPlacement.color.rgba.r) + m_primaryLightPlacement.color2.rgba.r);
		retVal.rgba.g = min(255, (int)(lightLevel * m_primaryLightPlacement.color.rgba.g) + m_primaryLightPlacement.color2.rgba.g);
		retVal.rgba.b = min(255, (int)(lightLevel * m_primaryLightPlacement.color.rgba.b) + m_primaryLightPlacement.color2.rgba.b);
	}
	else
	{
		// Otherwise mix the colors via multiplying
		retVal.rgba.r = min(255,
							((int)(lightLevel * baseTint.rgba.r * m_primaryLightPlacement.color.rgba.r) >> 8)
							+ ((baseTint.rgba.r * m_primaryLightPlacement.color2.rgba.r) >> 8) );
		retVal.rgba.g = min(255,
							((int)(lightLevel * baseTint.rgba.g * m_primaryLightPlacement.color.rgba.g) >> 8)
							+ ((baseTint.rgba.g * m_primaryLightPlacement.color2.rgba.g) >> 8) );
		retVal.rgba.b = min(255,
							((int)(lightLevel * baseTint.rgba.b * m_primaryLightPlacement.color.rgba.b) >> 8)
							+ ((baseTint.rgba.b * m_primaryLightPlacement.color2.rgba.b) >> 8) );
	}

	return retVal;
}

#endif //!LC_PLAT_OGL

/*-------------------------------------------------------------------------*//**
*/
bool LcCSpace::isEyeFacing()
{
	if (m_paintWidget)
	{
		// Calculate whether m_paintWidget is facing the eye or not
		LcTVector vOrigS = mapLocalToCanvasSubpixel(LcTVector(0,0,0));
		LcTVector vUnitX(1.0f, 0.0f, 0.0f);
		LcTVector vUnitY(0.0f, 1.0f, 0.0f);
		LcTVector vXS = mapLocalToCanvasSubpixel(vUnitX).subtract(vOrigS);
		LcTVector vYS = mapLocalToCanvasSubpixel(vUnitY).subtract(vOrigS);
		LcTVector vCanvasNormal = vXS.cross(vYS);

		return vCanvasNormal.z < 0;
	}
	else
	{
		// If no m_paintWidget, we're probably painting the background
		return true;
	}
}

/*-------------------------------------------------------------------------*//**
	Sets selected components of the widget's placement, as specified by
	the given mask
*/
LC_EXPORT void LcCSpace::setPrimaryLightPlacement(	const LcTPlacement& placement,
													int mask, bool updateCurrent)
{
	mask &= (LcTPlacement::EColor | LcTPlacement::EColor2 | LcTPlacement::EOrientation);
	if (mask != LcTPlacement::ENone)
	{
		m_primaryLightMask |= m_primaryLightPlacement.assign(placement, mask);

		if(updateCurrent)
			m_primaryLightCurrentPlacement = m_primaryLightPlacement;

		if (updateCurrent && m_primaryLightMask != LcTPlacement::ENone)
			revalidateAll();
	}
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
	Map a point on the 2D canvas to a point in global 3D space at z=targetZ
*/
LC_EXPORT LcTVector LcCSpace::mapCanvasToGlobal(const LcTPixelPoint& pt, LcTScalar targetZ)
{
	LcTScalar dScaleTo2D = m_recBounds.getWidth() / m_globalExtent.x;

	// The location of the point at Z=0
	LcTVector globalPointPos(	 (pt.x - m_ptOrigin2D.x) / dScaleTo2D,
								-((pt.y - m_ptOrigin2D.y) / dScaleTo2D),
								  0);

	// Find out the location of the eye in global space.
	LcTScalar globalEyeZ = (m_globalExtent.z);

	// Find out the distance from the eye to the origin (where Z=0)
	LcTScalar lenEyeOrigin = -globalEyeZ;

	// Find out the distance from the eye to the target Z plane
	LcTScalar lenEyeTarget = targetZ - globalEyeZ;

	// Find out the ratio between these two distances
	LcTScalar ratio = lenEyeTarget / lenEyeOrigin;

	// We can now apply this ratio to the point we are hit testing, and
	// it will map it to the Z plane where the widget is, with correct
	// scaling
	LcTVector returnVal(globalPointPos.x * ratio,
						globalPointPos.y * ratio,
						targetZ);

	return returnVal;
}
#endif

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
	Map a point on the canvas to a local coordinate aligned with a widget. Because we
	are doing this in 3D space, we will perform a ray/plane intersection algorithm.

	For an explanation of the algorithm, see
	 http://www.cl.cam.ac.uk/Teaching/2000/AGraphHCI/SMEG/node2.html
*/
// Given a widget and a point on the screen, this will calculate the point on the widget
// hit by the mouse event in local coordinates.  Note that on 'error' or if there was
// no intersection point, this returns '(0, 0, 0)', so there's no difference between
// tapping the center of the widget and not tapping the widget at all :(
LC_EXPORT LcTVector LcCSpace::mapCanvasToLocal(const LcTPixelPoint& pt, LcCWidget& widget)
{
	LcTVector vec;
	// Note that the original usage of mapCanvasToLocal didn't care about the failure
	// cases, to this is as it was...if the following returns 'false' we still
	// return a default vector
	mapCanvasToLocal(pt, &widget, LcTVector(0.0F, 0.0F, 0.0F), vec);
	return vec;
}

// Given a widget pointer, a location of the plane and a point on the screen, this will
// calculate where on the plane the mouse event happened in local co-ordinates.  The
// return value will be 'true' if 'intersection' contains a point of intersection, and
// false if there was an error or if there is no intersection point (in which case
// 'intersection' is untouched.
LC_EXPORT bool LcCSpace::mapCanvasToLocal(const LcTPixelPoint& pt, LcCWidget* widget,
												LcTVector location, LcTVector& intersection)
{
	if (!widget)
		return false;

// Ray/plane intersection only required when we ARE able to render rotated widgets
#if defined(IFX_RENDER_INTERNAL) || defined (LC_PLAT_OGL)

	// Determine the plane that we are drawing in. This is the location of the widget,
	// transformed into global space.
	LcTVector Pp = LcTVector(0, 0, location.z);
			  Pp = widget->mapLocalToGlobal(Pp);

	// Calculate the normal of the plane. This is the normal pointing towards the eye (0, 0, 1)
	// transformed by the inverse transpose of the matrix attached to the widget. Naturally.
	LcTTransform M = widget->getXfmToGlobal();
				 M.invert();
				 M.transpose(); // Note this only has an effect if using a non-uniform scale.

	LcTVector Np = LcTVector(0, 0, 1);
			  Np = M.transform(Np);
			  Np.normalise();

	// Work out the location of the eye
	LcTVector Pe	= LcTVector(0, 0, getGlobalExtent().z);

	// Work out the location of the hit point on the near plane. This is where the mouse was clicked.
	LcTScalar nearZ = (getGlobalExtent().z) / 2;
	LcTVector Ph = mapCanvasToGlobal(pt, nearZ);

	// If the normal of the plane is at right angles to the ray, there can be no intersection
	double denom = Np.dot(LcTVector::subtract(Ph, Pe));

	if (lc_fabs(denom) < ORDER_3D_ZERO_TOLERANCE)
		return false;

	// Otherwise, work out 't'.
	LcTScalar t = Np.dot(LcTVector::subtract(Pp, Pe)) /
				  (LcTScalar)denom;

	// If 't' < 0 then there is no intersection as the plane is 'behind' the eye
	if (t <= 0)
		return false;

	// Work out the intersection point (in global coordinates)
	LcTVector Pig = LcTVector::add(Pe, LcTVector::subtract(Ph, Pe).scale(t));

	// Map the global intersection point into local coords.
	intersection = widget->mapGlobalToLocal(Pig);

	// Done
	return true;

// Optimized implementation when no rotation of widget is possible
#else

	// Map the location of the widget origin (given in widget space), onto global space.
	LcTVector globalWidgetPos = widget->mapLocalToGlobal(location);

	// The point 'pt' passed in is in canvas space. When Z=0 in global space, canvas
	// units map onto global units (when canvas scale is taken into account). We need
	// to adjust 'pt' to be in global units at the Z of our widget.
	LcTVector globalPointPos = mapCanvasToGlobal(pt, globalWidgetPos.z);

	// Map from global coords to local coords for the widget
	intersection = widget->mapGlobalToLocal(globalPointPos);

	return true;
#endif
}
#endif

/*-------------------------------------------------------------------------*//**
	Indicate that space needs to be repainted
*/
LC_EXPORT void LcCSpace::revalidate(bool dirtyCanvas)
{
	// Set flag to force redraw of canvas (if required)
	m_bDirty |= dirtyCanvas;
	m_bRepainted = false;

	// Schedule paint action, but only if we haven't already
	// NB: use zero interval for maximum responsiveness, but we post so that other
	// revalidate()s called from the same event result in the same repaint().  If a
	// frame happened less than the frame interval ago, we will already have another
	// scheduled via the onExecute() method
	if (!m_bPaintScheduled)
	{
		m_msgFrame.schedule(getTimer(), 0, 0);
		m_bPaintScheduled = true;
	}
}

/*-------------------------------------------------------------------------*//**
	Indicate that everything in the space needs to be repainted.
	May happen as a result of changing the canvas parameters (e.g. resize)
	or the derived class changing its background image, etc.  Note that this
	is expensive and should only be done when absolutely necessary
*/
LC_EXPORT void LcCSpace::revalidateAll(bool excludeBackground)
{
	// Get rid of background region list, as we will start again with
	// full repaint of background
	m_regions.clear();
	m_regionMem.free();
	m_newRegionMem.free();

	// Force all to redraw
	for (unsigned i = 0; i < m_applets.size(); i++)
		m_applets[i]->revalidate();

	// Paint entire background once to start
#ifndef LC_PAINT_FULL
	if (!excludeBackground)
	{
		if (!startPaintBackground()
		||	!paintBackground(getCanvasBounds())
		||	!endPaintBackground())
		{
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "Failed to paint the background");
			LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
		}
	}
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCSpace::onMessage(int iID, int iParam)
{
	switch (iID)
	{
		// Handle frame paint event
		case MSG_FRAME:
		{
#ifdef IFX_MEMORYTEST_STARTUP
			// We're about to draw the first UI, so the startup phase is complete
			g_startupTest_testComplete = 1;

			// Throw a 'restart' error to terminate the test
			LC_CLEANUP_THROW(IFX_ERROR_RESTART);
#endif /* IFX_MEMORYTEST_STARTUP */

#ifdef IFX_MEMORYTEST_DYNAMIC
			if (EIfxMemoryTestStateStartup == g_dynamicTest_state)
			{
				// The first menu is showing for the first time
				g_dynamicTest_state = EIfxMemoryTestStateNormal;
			}
#endif /* IFX_MEMORYTEST_DYNAMIC */

			// We have serviced the schedule request (don't call onExecute() any other way!)
			m_bPaintScheduled = false;

			// Allow aggregate to perform any required actions before we draw the frame
			LcTTime timestamp = getTimestamp();
			bool finalFrame;
			bool reschedule = getFocus()->doPrepareForFrameUpdate(timestamp, finalFrame);

			// Perform any animations - if any animation is not complete, the space will have
			// been marked as dirty anyway, and a further paint will be scheduled
			processAnimators();

			// Schedule another frame now.  This ensures we generally get an even
			// frame rate regardless of the time taken to redraw each frame
			if (reschedule || (m_bDirty && (m_iFrameInterval > 0)))
			{
				m_msgFrame.schedule(getTimer(), 0, m_iFrameInterval);
				m_bPaintScheduled = true;
			}

#if defined (IFX_GENERATE_SCRIPTS)
			if(finalFrame && NdhsCScriptGenerator::getInstance())
			{
				NdhsCScriptGenerator::getInstance()->saveIntermediateScript();

				if(!m_bDirty && !reschedule)
				{
					NdhsCScriptGenerator::getInstance()->setFinalFrameCaptureEvent();
				}
			}
#endif

#if defined (IFX_USE_SCRIPTS) && !defined(NDHS_JNI_INTERFACE)
			/* Check if theme is loaded. If theme is loaded then call repaint. */
			bool themePresent = false;
			if (NdhsCScriptExecutor::getInstance())
				themePresent = NdhsCScriptExecutor::getInstance()->isThemeLoaded();

			if(themePresent)
			{
#endif
			// Do nothing if not dirty.  This is here to trap the case where we reschedule
			// to keep an even frame rate, but then no updates are made before that frame
			if (m_bDirty)
			{
				// Let the app do any pre-frame actions needed
				getFocus()->doPreFrameUpdate();

				// Now rebuild the canvas and blit it to the screen
				repaint(finalFrame);

				// Let the app do any post-frame actions needed
				getFocus()->doPostFrameUpdate();
			}

#if defined (IFX_USE_SCRIPTS) && !defined(NDHS_JNI_INTERFACE)
			}
#endif

			break;
		}

		// Handle key repeat event
		case MSG_REPEAT:
		{
			// The given key has been down for the repeat delay
			if (m_focus)
				m_focus->onKeyRepeat(iParam);

			break;
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Performs immediate screen repaint.  Should only be called by OS paint
	requests e.g. when screen area re-exposed.
*/
LC_EXPORT_VIRTUAL void LcCSpace::repaint(bool finalFrame)
{
	// The "active" space means the space we are currently drawing to,
	// which is not necessarily the one at the front.  The activate handler
	// may be used to set up drawing context, e.g. as required by OpenGL
	LcCEnvironment* env = LcCEnvironment::get();
	if (this != env->getActiveSpace())
	{
		// Deactivate the current active space
		if (env->getActiveSpace())
			env->getActiveSpace()->onDeactivate();

		// Activate this one
		env->setActiveSpace(this);
		onActivate();
	}

	// Only need to rebuild canvas if widgets have changed
	if (m_bDirty)
	{
		// Set flag first so that paint has the option of setting dirty again!
		m_bDirty = false;

		volatile int e = 0;

		// Platform-specific preparation to paint canvas
		if (startPaint())
		{
			// Paint canvas, inside trap handler so that exceptions will
			// still allow endPaint to be called
			LC_CLEANUP_TRAP(e, paintCanvas());

			// Platform-specific finishing up
			if (!endPaint())
			{
				NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "endPaint() failed");
				LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
			}
		}
		else
		{
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "startPaint() failed");
			LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
		}

		if (e)
		{
			// If an OOM exception occurred AND this is the paint following
			// 'transition complete' try to restart the application 'from the top'
			if (finalFrame)
			{
				if (e == IFX_ERROR_MEMORY)
				{
					IFXP_Display_Error_Note((IFX_WCHAR*)L"Out Of Memory - Could not allocate resources");
				}

				LC_CLEANUP_THROW(IFX_ERROR_RESTART);
			}

			// Pass on a restart error
			if (e == IFX_ERROR_RESTART)
				LC_CLEANUP_THROW(IFX_ERROR_RESTART);

			// On an exception within the paint canvas routine, we've no idea about the state of the
			// canvas or the region list etc.  Best do a full revalidate...note if this causes an OOM
			// error, we'll try to restart the application.
			LC_CLEANUP_TRAP(e, revalidateAll());
			if (e != 0)
				LC_CLEANUP_THROW(IFX_ERROR_RESTART);
		}
	}
	else
	{
		// Blit entire buffer to screen (if implementation uses buffer)
		// NB: this cuts out any blackout widgets, according to m_regions list
		// NB: strictly we should call blitToScreen() instead, but for current
		// implementations this does not matter
		if (startBlitToScreen())
		{
			blitToBufferWithHoles(getCanvasBounds());
		}
		if (!endBlitToScreen())
		{
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "endBlitToScreen() failed");
			LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
		}
	}

	// Done
	m_bRepainted = true;
}

/*-------------------------------------------------------------------------*//**
	Returns true from the first call following a repaint - note that
	the space may have been marked dirty again by this repaint.
	Used by platform classes to decide whether to yield between timer events
*/
LC_EXPORT bool LcCSpace::getRepainted()
{
	bool b = m_bRepainted;
	m_bRepainted = false;
	return b;
}

/*-------------------------------------------------------------------------*//**
*/
bool orderRegionY(LcTCanvasRegion* o1, LcTCanvasRegion* o2)
{
	return o2->m_rect.getTop() > o1->m_rect.getTop();
}

/*-------------------------------------------------------------------------*//**
*/
bool orderRectY(const LcTPixelRect& o1, const LcTPixelRect& o2)
{
	return o2.getTop() > o1.getTop();
}

/*-------------------------------------------------------------------------*//**
*/
bool orderZ(LcTCanvasRegion* o1, LcTCanvasRegion* o2)
{
	// In z-ordered widgets: Lower numbers are 'on top' of higher numbers when compositing the scene
	return (o2->m_drawLayerIndex > o1->m_drawLayerIndex) ||
			((o2->m_drawLayerIndex == o1->m_drawLayerIndex) && (o2->m_dZ > o1->m_dZ));
}

/*-------------------------------------------------------------------------*//**
	3D NDI draw order supporting functions start here
*/
#if defined(IFX_RENDER_INTERNAL) || defined (LC_PLAT_OGL)

#define DRAW_SECOND 1
#define DRAW_FIRST 2
#define DRAW_ORDER_UNDEFINED 0
int drawOrder_3d(const LcTCanvasRegion* o1, const LcTCanvasRegion* o2);

/*-------------------------------------------------------------------------*//**
	Given two vectors (a->b) 'v1', (a->c) 'v2', calculate the barycentric
	coordinates of a point f with respect to these vectors.
*/
void calcBarycentricCoords(const LcTVector& f, const LcTVector& a,
						   const LcTVector& b, const LcTVector& c,
						   LcTScalar& u, LcTScalar& v)
{
	LcTVector vu = b;
	LcTVector vv = c;
	LcTScalar v1x, v1y, v2x, v2y;

	vu.subtract(a);
	vv.subtract(a);

	// Store the vector components for use below - save a few
	// subtraction operations
	v1x = vu.x;
	v1y = vu.y;
	v2x = vv.x;
	v2y = vv.y;

	// test that the barycentric vectors aren't colinear
	vv.cross(vu);
	if (vv.isZero() == true)
	{
		// degenerate coordinate system - we can return -1, -1 for our
		// collision testing, as this will guarantee a 'miss'
		u = -1;
		v = -1;
		return;
	}

	// OK - now we know that both (c.x - a.x) and (c.y - a.y) cannot be zero
	// at the same time, likewise (b.x - a.x) and (b.y - a.y)

	// Look for degeneracy first
	if (lc_fabs(v1x) < ORDER_3D_ZERO_TOLERANCE)
	{
		// NB b.y - a.y cannot be zero, nor can c.x - a.x
		v = (f.x - a.x) / v2x;
		u = ((f.y - a.y) - v * v2y) / v1y;
		return;
	}

	if (lc_fabs(v1y) < ORDER_3D_ZERO_TOLERANCE)
	{
		// NB b.x - a.x cannot be zero, nor can c.y - a.y
		v = (f.y - a.y) / v2y;
		u = ((f.x - a.x) - v * v2x) / v1x;
		return;
	}

	if (lc_fabs(v2x) < ORDER_3D_ZERO_TOLERANCE)
	{
		// NB c.y - a.y cannot be zero, nor can b.x - a.x
		u = (f.x - a.x) / v1x;
		v = ((f.y - a.y) - u * v1y) / v2y;
		return;
	}

	if (lc_fabs(v2y) < ORDER_3D_ZERO_TOLERANCE)
	{
		// NB c.x - a.x cannot be zero, nor can b.y - a.y
		u = (f.y - a.y) / v1y;
		v = ((f.x - a.x) - u * v1x) / v2x;
		return;
	}

	// General case
	// Calculate denominator part first - if too close to zero, assume a 'miss'
	// and return -1
	v = v2y * v1x - v1y * v2x;

	if (lc_fabs(v) < ORDER_3D_ZERO_TOLERANCE)
	{
		u = -1;
		v = -1;
		return;
	}

	// Now we can calculate u and v
	v = ((f.y - a.y) * v1x - v1y * (f.x - a.x)) / v;
	u = (f.x - a.x - v2x * v) / v1x;

	return;
}

/*-------------------------------------------------------------------------*//**
	Given a vector (pt1->pt2) calculate the intersection point with the line
	from (1,0) to (0,1), where all coordinates are barycentric to some pair of
	vectors.  Check that the point lies on both pt1->pt2 and on (1,0)->(0,1),
	and return true if so.
*/
bool calcBarymetricIntersectionPoint1( const LcTVector& pt1, const LcTVector& pt2,
									   LcTVector& intersection)
{
	LcTScalar diagX = pt2.x - pt1.x;
	LcTScalar diagY = pt2.y - pt1.y;
	LcTScalar denom = diagX + diagY;

	if (lc_fabs(denom) < ORDER_3D_ZERO_TOLERANCE)
	{
		return false;
	}
	else
	{
		// Calculate how far along the line (pt1) -> (pt2)
		// we are
		intersection.z = (1 - pt1.x - pt1.y) / denom;

		// Calculate the intersection point
		intersection.x = pt1.x + intersection.z * diagX;
		intersection.y = pt1.y + intersection.z * diagY;
	}

	// intersection.z must be between 0 and 1 for a successful hit, and both x,y
	// parameters of the intersection must be > 0
	if ((intersection.x  >= 0)
		&& (intersection.y >= 0)
		&& (intersection.z >= 0)
		&& (intersection.z <= 1))
		return true;
	else
		return false;
}

/*-------------------------------------------------------------------------*//**
	Given a vector (pt1->pt2) calculate the intersection point with the line
	from (0,0) to (vertu,vertv), where all coordinates are barycentric to some
	pair of vectors.  Check that the point lies on both pt1->pt2 and on
	(0,0)->(vertu,vertv), and return true if so.
*/
bool calcBarymetricIntersectionPoint2( const LcTVector& pt1, const LcTVector& pt2,
									   const LcTScalar& vertu, const LcTScalar& vertv,
									   LcTVector& intersection)
{
	LcTScalar diagX = pt2.x - pt1.x;
	LcTScalar diagY = pt2.y - pt1.y;
	LcTScalar denom = vertv * diagX - vertu * diagY;
	LcTScalar ptBeta;

	if (lc_fabs(denom) < ORDER_3D_ZERO_TOLERANCE)
	{
		return false;
	}
	else
	{
		// Calculate how far along the line (pt1) -> (pt2)
		// we are
		intersection.z = (vertu * pt1.y - vertv * pt1.x) / denom;

		// Calculate the intersection point
		intersection.x = pt1.x + intersection.z * diagX;
		intersection.y = pt1.y + intersection.z * diagY;

		// Calculate how far along the line (0, 0) -> (vertu, vertv)
		// we are
		if (lc_fabs(vertu) > ORDER_3D_ZERO_TOLERANCE)
			ptBeta = intersection.x / vertu;
		else
			ptBeta = intersection.y / vertv;
	}

	// both intersection.z and ptBeta must be between 0 and 1 for a successful
	// hit
	if ((ptBeta >= 0)
		&& (ptBeta <= 1)
		&& (intersection.z >= 0)
		&& (intersection.z <= 1))
		return true;
	else
		return false;
}

/*-------------------------------------------------------------------------*//**
	Function that determines whether o1 should be drawn first or second.  We use
	a min z ordering mechanism, with an additional 'tie breaker'
	using the value of the region pointers to ensure that if this function
	returns 'DRAW_FIRST' for regions A, B, then it will return DRAW_SECOND for
	regions B, A.  Note that we can't use m_dZ, as this may be the z value of
	a rotated aggregate rather than of the widget!
*/
int orderRegionByZ(const LcTCanvasRegion* o1, const LcTCanvasRegion* o2)
{
	if (o2->m_drawLayerIndex < o1->m_drawLayerIndex)
		return DRAW_SECOND;
	else if (o1->m_drawLayerIndex < o2->m_drawLayerIndex)
		return DRAW_FIRST;
	else if (o2->m_dZ_min < o1->m_dZ_min)
		return DRAW_SECOND;
	else if (o1->m_dZ_min < o2->m_dZ_min)
		return DRAW_FIRST;
	else if (o2->m_dZ_max < o1->m_dZ_max)
		return DRAW_SECOND;
	else if (o1->m_dZ_max < o2->m_dZ_max)
		return DRAW_FIRST;
	else if (o2 < o1)
		return DRAW_SECOND;
	else
		return DRAW_FIRST;
}

/*-------------------------------------------------------------------------*//**
	Utility macros for dealing with the overlap region array
*/
#define REGION_OVERLAPPED(array, n, i, j)	array[i*(n + 2) + j + 1]
#define REGION_COVER_COUNT(array, n, i) 	array[i*(n + 2) + n + 1]
#define REGION_ADDRESS(array, n, i) 		array[i*(n + 2)]

/*-------------------------------------------------------------------------*//**
	Sort helper function for the 3d NDI case - we use the cached overlap
	array p2dArray where possible, otherwise use the 'old style' orderRegionByZ
	method.
*/
bool orderZ_3d(const LcTCanvasRegion* o1, const LcTCanvasRegion* o2,
			   LcTmArray<int*>& arrayVec, LcTmArray<int>& arraySizeVec)
{
	int i;
	int o1Index = -1, o2Index = -1;
	int order = DRAW_ORDER_UNDEFINED;
	int numRegions;
	LcTaAlloc<int> overLapArray;

	if (o1->m_drawLayerIndex < o2->m_drawLayerIndex)
	{
		return false;
	}
	else if (o1->m_drawLayerIndex > o2->m_drawLayerIndex)
	{
		return true;
	}

	for (i = 0; i < (int)arrayVec.size(); i++)
	{
		overLapArray.attach(arrayVec[i]);

		numRegions = arraySizeVec[i];

		for (int k = 0; k < numRegions; k++)
		{
			if (REGION_ADDRESS(overLapArray, numRegions, k) == (int)o1)
				o1Index = k;

			if (REGION_ADDRESS(overLapArray, numRegions, k) == (int)o2)
				o2Index = k;
		}

		if ((o1Index != -1) && (o2Index != -1))
		{
			order = REGION_OVERLAPPED(overLapArray, numRegions, o1Index, o2Index);
			break;
		}

		overLapArray.release();
	}

	if ((o1Index == -1) && (o2Index == -1))
	{
		order = orderRegionByZ(o1, o2);
	}

	if (order == DRAW_FIRST)
		return true;
	else
		return false;
}

/*-------------------------------------------------------------------------*//**
	Function checks to see if the x-y plane bounding rect containing the
	triangle with vertices a, b, c contains the projection of the point f or not
*/
bool triangleEnclosureCheck(const LcTVector& f, const LcTVector& a,
							const LcTVector& b, const LcTVector& c)
{
	LcTScalar bbLeft, bbTop, bbRight, bbBottom;

	LcTScalar v1 = a.y;
	LcTScalar v2 = b.y;
	LcTScalar v3 = c.y;
	bbTop = max(max(v1, v2), v3);
	bbBottom = min(min(v1, v2), v3);
	v1 = a.x;
	v2 = b.x;
	v3 = c.x;
	bbRight = max(max(v1, v2), v3);
	bbLeft = min(min(v1, v2), v3);

	if ( (f.y > bbTop) || (f.y < bbBottom) || (f.x < bbLeft) || (f.x > bbRight))
		return false;
	else
		return true;
}

/*-------------------------------------------------------------------------*//**
	Function that determines whether one region is overlapped by another, and
	determines whether o1 should be drawn first or second.  If no overlap is
	found, we return 'undefined'.  If the regions are touching, we use the
	orderRegionByZ function to order the regions.
*/
int drawOrder_3d(const LcTCanvasRegion* o1, const LcTCanvasRegion* o2)
{
	int r, i, j;
	LcTScalar zdelta;
	LcTVector tl;
	LcTVector tr;
	LcTVector bl;
	LcTVector br;

	// This function will only order overlapping regions!
	if (o1->m_rect.intersects(o2->m_rect) == false)
		return DRAW_ORDER_UNDEFINED;

	// If one or both of the regions are zero size, the draw order is unimportant!
	if (o1->m_rect.getHeight() == 0 || o1->m_rect.getWidth() == 0 || o2->m_rect.getHeight() == 0 || o2->m_rect.getWidth() == 0)
		return DRAW_ORDER_UNDEFINED;

	// First step - compare max/min canvas z values and layer index order
	if (o2->m_drawLayerIndex > o1->m_drawLayerIndex)
		return DRAW_FIRST;

	if (o1->m_drawLayerIndex > o2->m_drawLayerIndex)
		return DRAW_SECOND;

	if (o2->m_dZ_min > o1->m_dZ_max)
		return DRAW_FIRST;

	if (o1->m_dZ_min > o2->m_dZ_max)
		return DRAW_SECOND;

	// Cater for the case where both regions are in the same z plane
	LcTScalar v1max = o1->m_dZ_max;
	LcTScalar v2max = o2->m_dZ_max;
	LcTScalar v1min = o1->m_dZ_min;
	LcTScalar v2min = o2->m_dZ_min;
	if ((max(v1max, v2max) - min(v1min, v2min)) < ORDER_3D_ZERO_TOLERANCE)
	{
		return orderRegionByZ(o1, o2);
	}

	// Cater for the cases where one of the regions is in the z plane, unrotated
	// We can quickly check the vertices of the rotated region.
	for (r=0; r<2; r++)
	{
		const LcTCanvasRegion* region1 = (r == 0) ? o1 : o2;
		const LcTCanvasRegion* region2 = (r == 0) ? o2 : o1;

		// Is the region in the Z plane?
		if (lc_fabs(region1->m_dZ_max - region1->m_dZ_min) < ORDER_3D_ZERO_TOLERANCE)
		{
			tl = region1->m_quad.getTopLeft();
			tr = region1->m_quad.getTopRight();
			bl = region1->m_quad.getBottomLeft();
			br = region1->m_quad.getBottomRight();

			// Are the region sides parallel to the x and y axis?
			if (((tl.x == bl.x) && (tr.x == br.x) && (tl.y == tr.y) && (bl.y == br.y))
				|| ((tl.y == bl.y) && (tr.y == br.y) && (tl.x == tr.x) && (bl.x == br.x)))
			{
				// Test the four vertices of the other region
				LcTPixelPoint pixelVertex;
				tl = region2->m_quad.getTopLeft();
				tr = region2->m_quad.getTopRight();
				bl = region2->m_quad.getBottomLeft();
				br = region2->m_quad.getBottomRight();

				pixelVertex.x = (int)tl.x;
				pixelVertex.y = (int)tl.y;
				if (region1->m_rect.contains(pixelVertex) == true)
				{
					if (tl.z < region1->m_dZ_max)
						return (r == 1) ? DRAW_FIRST : DRAW_SECOND;
					else
						return (r == 1) ? DRAW_SECOND : DRAW_FIRST;
				}

				pixelVertex.x = (int)tr.x;
				pixelVertex.y = (int)tr.y;
				if (region1->m_rect.contains(pixelVertex) == true)
				{
					if (tr.z < region1->m_dZ_max)
						return (r == 1) ? DRAW_FIRST : DRAW_SECOND;
					else
						return (r == 1) ? DRAW_SECOND : DRAW_FIRST;
				}

				pixelVertex.x = (int)bl.x;
				pixelVertex.y = (int)bl.y;
				if (region1->m_rect.contains(pixelVertex) == true)
				{
					if (bl.z < region1->m_dZ_max)
						return (r == 1) ? DRAW_FIRST : DRAW_SECOND;
					else
						return (r == 1) ? DRAW_SECOND : DRAW_FIRST;
				}

				pixelVertex.x = (int)br.x;
				pixelVertex.y = (int)br.y;
				if (region1->m_rect.contains(pixelVertex) == true)
				{
					if (br.z < region1->m_dZ_max)
						return (r == 1) ? DRAW_FIRST : DRAW_SECOND;
					else
						return (r == 1) ? DRAW_SECOND : DRAW_FIRST;
				}
			}
		}
	}

	// OK - no quick win, so we must go on to the full-blown
	// z ordering test.  For all vertices of the o1 scalar quad perform a
	// ray intersection test on the o2 scalar quad.
	// If we find one intersection, we can order the regions, otherwise we
	// will need to repeat the process for the vertices of o2 and the o1
	// scalar quad.  if there are no intersecting vertices, we must finally
	// check that the diagonals of the two regions don't overlap.  If we
	// still can't find a point incident on the other rect, the regions
	// don't overlap so the draw order is undefined.
	LcTVector diagTestTL, diagTestTR, diagTestBL, diagTestBR;
	LcTVector* pDiagTestVert = &diagTestTL;
	LcTaAlloc<LcTVector> vertices(4);
	LcTaArray<int> vertexOrder;
	vertexOrder.reserve(4);

	for (r = 0; r < 2; r++)
	{
		const LcTCanvasRegion* region1 = (r == 0) ? o1 : o2;
		const LcTCanvasRegion* region2 = (r == 0) ? o2 : o1;

		tl = region2->m_quad.getTopLeft();
		tr = region2->m_quad.getTopRight();
		bl = region2->m_quad.getBottomLeft();
		br = region2->m_quad.getBottomRight();

		// Sort the four vertices of region 1 into z order -
		// by always starting with the vertex with smallest
		// canvas z (i.e. closest to the eye) we can avoid some
		// pop through with rotating widgets intersecting other
		// widgets.
		vertices[0] = region1->m_quad.getTopLeft();
		vertices[1] = region1->m_quad.getBottomRight();
		vertices[2] = region1->m_quad.getTopRight();
		vertices[3] = region1->m_quad.getBottomLeft();

		vertexOrder.push_back(0);

		LcTaArray<int>::iterator it;
		for (i = 1; i < 4; i++)
		{
			for (j = 0; j < i; j++)
			{
				if(vertices[vertexOrder[j]].z > vertices[i].z)
				{
					it = vertexOrder.begin();
					vertexOrder.insert((it + j), i);
					break;
				}
			}

			if (j == i)
				vertexOrder.push_back(i);
		}

		// The four corners of region 1
		for (i = 0; i < 4; i++)
		{
			LcTVector vertex;

			// Get next vertex to check
			vertex = vertices[vertexOrder[i]];
			switch(vertexOrder[i])
			{
			case 0:
				pDiagTestVert = &diagTestTL;
				break;
			case 1:
				pDiagTestVert = &diagTestBR;
				break;
			case 2:
				pDiagTestVert = &diagTestTR;
				break;
			case 3:
				pDiagTestVert = &diagTestBL;
				break;
			}

			// Calculate the barycentric coords
			// of the intersection point in the plane defined by {tl,tr,bl}
			LcTScalar v1, u1;

			if (r == 1)
			{
				// Quick test to see if the vertex being tested lies within
				// the rect that contains the triangle {tl,tr,bl} - note that
				// we only test this for the second iteration as we have to
				// calculate the barycentric coords of one region vertices
				// for the diagonal test.
				if (triangleEnclosureCheck(vertex, tl, tr, bl) == false)
				{
					// Not in triangle {tl,tr,bl} but we still have to
					// check triangle {br,tr,bl}
					u1 = 1;
					v1 = 1;
				}
				else
				{
					calcBarycentricCoords(vertex, tl, tr, bl, u1, v1);
				}
			}
			else
			{
				calcBarycentricCoords(vertex, tl, tr, bl, u1, v1);

				// Store the barycentric co-ordinates of this
				// vertex for the final 'crossing diagonals' test
				(*pDiagTestVert).x = u1;
				(*pDiagTestVert).y = v1;
				(*pDiagTestVert).z = vertex.z;
			}

			// Assume that region 2 is a convex quad - in this case,
			// the whole of the {br, tr, bl} triangle must be between the
			// u1=0 and v1=0 lines, thus if u1 or v1 is < 0 then the point
			// cannot be in the region at all
			if ((u1 >= 0 ) && (v1 >= 0))
			{
				// If u1 >= 0, v1 >= 0, u1 + v1 <= 1 then the intersection lies
				// within the {tl,tr,bl} triangle.  Calculate the delta needed
				// to move from one region to the other along the ray.
				if ((u1 + v1 <= 1.0))
				{
					zdelta = (tl.z - vertex.z)
						+ (tr.z - tl.z) * u1
						+ (bl.z - tl.z) * v1;

					if (lc_fabs(zdelta) < ORDER_3D_ZERO_TOLERANCE)
					{
						return orderRegionByZ(o1, o2);
					}

					if (zdelta > 0)
						return (r == 1) ? DRAW_SECOND : DRAW_FIRST;
					else
						return (r == 1) ? DRAW_FIRST : DRAW_SECOND;
				}

				// We can apply the quick test to see if the vertex being
				// tested lies within the rect that contains the triangle
				// {bl,br,tr} in all cases, as it's only u1 and v1 we need
				// for the diagonal tests.
				if (triangleEnclosureCheck(vertex, br, bl, tr) == false)
					continue;

				// Otherwise the point may lie in the {br, tr, bl} triangle
				// Calculate the barycentric coords of the intersection point in
				// the plane defined by {br, tr, bl}
				LcTScalar v2, u2;
				calcBarycentricCoords(vertex, br, bl, tr, u2, v2);

				// If u2 >= 0, v2 >= 0, u2 + v2 <= 1 we have a hit
				if ((u2 >= 0 ) && (v2 >= 0) && (u2 + v2 <= 1.0))
				{
					zdelta = (br.z - vertex.z)
					  + (bl.z - br.z) * u2
					  + (tr.z - br.z) * v2;

					if (lc_fabs(zdelta) < ORDER_3D_ZERO_TOLERANCE)
					{
						return orderRegionByZ(o1, o2);
					}

					if (zdelta > 0)
						return (r == 1) ? DRAW_SECOND : DRAW_FIRST;
					else
						return (r == 1) ? DRAW_FIRST : DRAW_SECOND;
				}
			}
		}

		// If we haven't found a vertex that lies within the target region,
		// then we have one last case to check for: do the diagonals of the
		// four points in the target region plane intersect with the diagonals
		// of the target quad? If so, we can find out zdelta from that point.
		// If not, then we definitely don't have an intersection and we can
		// carry on.

		// Note that we only want to do this once, as if the diagonals
		// do not intersect the first time round they won't the second time
		// round!
		if (r == 1)
			continue;

		// Compare both region 1 diagonals against bl->tr of region 2.  Note
		// that we're using the barycentric coordinates for (tl,tr, bl) of
		// region 2, so this diagonal is described by u1 + v1 = 1, with
		// u1>=0 and v1>=0.
		LcTVector intersectionPt;
		bool intFound = false;
		LcTScalar zpt = 0;

		if ((intFound = calcBarymetricIntersectionPoint1(diagTestBL,
												diagTestTR,
												intersectionPt)) == true)
		{
			// intersection point lies within the target rect
			zpt = diagTestBL.z + (diagTestTR.z
									- diagTestBL.z)*intersectionPt.z;
		}
		else if ((intFound = calcBarymetricIntersectionPoint1(diagTestBR,
												diagTestTL,
												intersectionPt)) == true)
		{
			// intersection point lies within the target rect
			zpt = diagTestBR.z + (diagTestTL.z
									- diagTestBR.z) * intersectionPt.z;
		}
		else
		{
			// we need the tr,tl,bl barycentric coordinates of the br corner
			LcTScalar vPrime, uPrime;
			calcBarycentricCoords(br, tl, tr, bl, uPrime, vPrime);

			// compare both region 1 diagonals against tl->br of region 2.  In this
			// case, the diagonal is from (0, 0) to (uPrime, vPrime) in (tl, tr, bl)
			// barycentric coordinates, where (uPrime, vPrime) is the br vertex of
			// region 2, calculated above.
			if ((intFound = calcBarymetricIntersectionPoint2(diagTestTL,
												diagTestBR,
												uPrime, vPrime,
												intersectionPt)) == true)
			{
				// intersection point lies within the target rect
				zpt = diagTestTL.z + (diagTestBR.z
										- diagTestTL.z) * intersectionPt.z;
			}
			else if ((intFound = calcBarymetricIntersectionPoint2(diagTestBL,
													diagTestTR,
													uPrime, vPrime,
													intersectionPt)) == true)
			{
				// intersection point lies within the target rect
				zpt = diagTestBL.z + (diagTestTR.z
										- diagTestBL.z) * intersectionPt.z;
			}
		}

		if (intFound)
		{
			// Calculate delta z at intersection point
			zdelta = (tl.z - zpt)
					+ (tr.z - tl.z) * intersectionPt.x
					+ (bl.z - tl.z) * intersectionPt.y;

			if (lc_fabs(zdelta) < ORDER_3D_ZERO_TOLERANCE)
			{
				return orderRegionByZ(o1, o2);
			}

			if (zdelta > 0)
				return (r == 1) ? DRAW_SECOND : DRAW_FIRST;
			else
				return (r == 1) ? DRAW_FIRST : DRAW_SECOND;
		}
	}

	// If we get this far, there was no intersection after all
	return DRAW_ORDER_UNDEFINED;
}

/*-------------------------------------------------------------------------*//**
	Special draw order sort routine for the 3d NDI case

	Three Phase scheme
 1. Go through the region array passed in - we look for a widget coming from
	a rotated aggregate set - widgets up to this point are to be rendered first,
	irrespective of 3d considerations.
	Continue until we have found the last widget from a rotated aggregate set
	- widgets after this point are to be rendered after the rotated aggregate
	set, irrespective of 3d ordering.
	Each set of widgets are sorted according	to the rest of the sorting scheme.
	Note that it's only necessary to sort the first and/or the last set if
	there is at least one widget in it rotated out of the x-y plane, otherwise
	the widgets are already sorted by midpoint z.
 2. For each widget set: for each region, check each region pair to determine
	ordering between the pairs, storing this information in a 2-d array.
 3. For each widget set: sequentially iterate over the array stuffing regions
	that are not overlapped into the draw sequence, updating the overlap array.
	Break any cyclic dependency using the widget with the minimum z.  Finally,
	update the regions array with the correct draw order for the set.
*/
void drawSort_3d(LcTmArray<LcTCanvasRegion*>& regions, const LcTmAlloc<int>& p2dArray)
{
	LcTaArray<LcTCanvasRegion*> orderedRegions;
	LcTaArray<LcTCanvasRegion*>::iterator it;
	int i, j, firstRotAggWidget = -1, lastRotAggWidget = -1;
	bool bRegionSet1Sort = false, bRegionSet3Sort = false;
	unsigned numRegions = (unsigned)regions.size();

	// Zero the overlap array
	memset(p2dArray, 0, sizeof(int)*(numRegions * (numRegions + 2)));

	// Phase 1 - identify the range of widgets that need sorting with the
	// 3d algorithm.  We shall have at most:
	// Region set 1 - furniture 'behind' the rotated agg widgets
	// Region set 2 - widgets on a rotated aggregate
	// Region set 3 - furniture 'in front of' the rotated agg widgets
	// Each region set may need sorting using the 3d ordering algorithm, but
	// must remain ordered with respect to the regions sets
	for (it = regions.begin(), i = 0; it != regions.end(); it++, i++)
	{
		// Store address of region
		REGION_ADDRESS(p2dArray, numRegions, i) = (int)&(*it);

		if ((*it)->m_bParentAggRotated)
		{
			if (firstRotAggWidget == -1)
			{
				firstRotAggWidget = lastRotAggWidget = i;
			}
			else
			{
				lastRotAggWidget = i;
				// reset the region set 3 sort flag, as region set 3 doesn't
				// exist yet!
				if (bRegionSet3Sort) bRegionSet3Sort = false;
			}
		}
		else
		{
			// If there is a rotated widget in region set 1, that will have
			// to be sorted using the 3d sort algorithm
			if ((firstRotAggWidget == -1) && (*it)->m_bRotated)
				bRegionSet1Sort = true;

			// If there is a rotated widget in region set 3, that will have
			// to be sorted using the 3d sort algorithm
			if ((lastRotAggWidget != -1) && (*it)->m_bRotated)
				bRegionSet3Sort = true;
		}
	}

	// Quick test to see if we can stop 3d sorting here - if there
	// are no rotated aggregates, and none of the widgets are rotated
	// out of the x-y plane, the widgets are already ordered
	if ((firstRotAggWidget == -1) && (bRegionSet1Sort == false))
		return;

	int firstRegion = -1, lastRegion = -1;
	bool bDone;
	int minIndex = 0;
	for (int regionSet = 0; regionSet < 3; regionSet++)
	{
		switch(regionSet)
		{
			case 0:
				{
					if (bRegionSet1Sort)
					{
						firstRegion = 0;
						lastRegion = (firstRotAggWidget == -1)? numRegions - 1 : firstRotAggWidget - 1;

						// Region is already sorted (trivially!)
						if (firstRegion == lastRegion)
							continue;
					}
					else
					{
						// Region is already sorted
						continue;
					}
				}
				break;
			case 1:
				{
					firstRegion = firstRotAggWidget ;
					lastRegion = lastRotAggWidget ;

					// Region is already sorted (trivially!)
					if (firstRegion == lastRegion)
						continue;
				}
				break;
			case 2:
				{
					if (bRegionSet3Sort)
					{
						firstRegion = lastRotAggWidget + 1;
						lastRegion = numRegions - 1;

						// Region is already sorted (trivially!)
						if (firstRegion == lastRegion)
							continue;
					}
					else
					{
						// Region is already sorted
						continue;
					}
				}
				break;
		}

		// Pre-size ordered vector
		orderedRegions.reserve(lastRegion - firstRegion + 1);

		// Phase 2 - analyze each pair of regions in the 3d widget range using
		// the 3d draw order mechanism.
		for (i = firstRegion; i < lastRegion + 1; i++)
		{
			// Test against all remaining regions in range
			for (j = i+1; j < lastRegion + 1; j++)
			{
				int orderRes = drawOrder_3d(regions[i], regions[j]);

				if (orderRes == DRAW_SECOND)
				{
					REGION_OVERLAPPED(p2dArray, numRegions, i, j) = DRAW_SECOND;
					REGION_OVERLAPPED(p2dArray, numRegions, j, i) = DRAW_FIRST;
					REGION_COVER_COUNT(p2dArray, numRegions, i) += 1;
				}
				else if (orderRes == DRAW_FIRST)
				{
					REGION_OVERLAPPED(p2dArray, numRegions, i, j) = DRAW_FIRST;
					REGION_OVERLAPPED(p2dArray, numRegions, j, i) = DRAW_SECOND;
					REGION_COVER_COUNT(p2dArray, numRegions, j) += 1;
				}
				else
				{
					REGION_OVERLAPPED(p2dArray, numRegions, i, j) = DRAW_ORDER_UNDEFINED;
					REGION_OVERLAPPED(p2dArray, numRegions, j, i) = DRAW_ORDER_UNDEFINED;
				}
			}

			// We can deal with any uncovered regions early
			if (REGION_COVER_COUNT(p2dArray, numRegions, i) == 0)
			{
				orderedRegions.insert( orderedRegions.begin(), regions[i] );
				REGION_COVER_COUNT(p2dArray, numRegions, i) = -1;

				for (j = firstRegion; j < lastRegion + 1; j++)
				{
					if (i == j)
						continue;

					if (REGION_OVERLAPPED(p2dArray, numRegions, j, i) == DRAW_SECOND)
					{
						REGION_COVER_COUNT(p2dArray, numRegions, j) -= 1;
					}
				}
			}
		}

		// Phase 3: repeatedly go through the overlap array, removing uncovered regions
		// until all regions have been dealt with.
		bDone = false;
		while (bDone == false)
		{
			bDone = true;

			for (i = firstRegion; i < lastRegion + 1; i++)
			{
				// If the total number of regions overlapping this region is 0, add it
				// to the draw order vector and remove this region from the array
				if (REGION_COVER_COUNT(p2dArray, numRegions, i) == 0)
				{
					orderedRegions.insert( orderedRegions.begin(), regions[i] );
					REGION_COVER_COUNT(p2dArray, numRegions, i) = -1;

					for (j = firstRegion; j < lastRegion + 1; j++)
					{
						if (i == j)
							continue;

						if (REGION_OVERLAPPED(p2dArray, numRegions, j, i) == DRAW_SECOND)
						{
							REGION_COVER_COUNT(p2dArray, numRegions, j) -= 1;
						}
					}

					bDone = false;
				}
			}

			if (bDone == true && ((lastRegion - firstRegion + 1) != (int)orderedRegions.size()))
			{
				// Cyclic region widget layering detected! Ordering regions by minimizing max z.
				minIndex = -1;

				for (i = firstRegion; i < lastRegion + 1; i++)
				{
					if (REGION_COVER_COUNT(p2dArray, numRegions, i) > 0)
					{
						if(minIndex == -1)
						{
							minIndex = i;
							continue;
						}

						if (regions[minIndex]->m_dZ_max > regions[i]->m_dZ_max )
						{
							minIndex = i;
						}
					}
				}

				// Now remove this region and carry on.
				orderedRegions.insert( orderedRegions.begin(), regions[minIndex] );
				REGION_COVER_COUNT(p2dArray, numRegions, minIndex) = -1;

				for (i = firstRegion; i < lastRegion + 1; i++)
				{
					if (minIndex == i)
						continue;

					if (REGION_OVERLAPPED(p2dArray, numRegions, i, minIndex) == DRAW_SECOND)
					{
						REGION_COVER_COUNT(p2dArray, numRegions, i) -= 1;
					}
				}

				bDone = false;
			}
		}

		// OK - fill in the '3d sorted' section of the regions vector
		for(it = orderedRegions.begin(), i = firstRegion; it < orderedRegions.end(); it++, i++)
		{
			regions[i] = *it;
		}

		orderedRegions.clear();
	}
}
#endif // IFX_RENDER_INTERNAL || LC_PLAT_OGL
/*-------------------------------------------------------------------------*//**
	3D NDI draw order supporting functions end here
*/

#if !defined (LC_PAINT_FULL)
/*-------------------------------------------------------------------------*//**
	Extracts paint rectangles from the given region list into
	the given paint rect list, according to the filter parameters

	Precondition:  regionsDownward must be sorted downward by Y-coord
	Postcondition:  paintRects sort order is undefined
*/
void LcCSpace::buildRegionRectangleList(
	LcTmArray<LcTPixelRect>&		paintRects,
	LcTmArray<LcTCanvasRegion*>&	regionsDownward,
	bool							bIncludeWidget )
{
	// Clear the output list
	paintRects.clear();

	// Iterate regions in a downward direction.
	unsigned iA;
	for (iA = 0; iA < regionsDownward.size(); iA++)
	{
		LcTCanvasRegion* regionA = regionsDownward[iA];

		// Non-dirty and non-clipped regions are not of interest
		if (!(regionA->m_bDirty || !regionA->m_clipRects.empty()))
			continue;

		// Check that the region is the appropriate paint mode for this extraction
		if (!(regionA->m_bPaintBack || (bIncludeWidget && regionA->m_bPaintWidget)))
			continue;

		// If we have no clip regions
		if (regionA->m_clipRects.empty())
		{
			// We add the entire region rect to the paint list
			paintRects.push_back(regionA->m_rect);
		}
		else if (!bIncludeWidget )
		{
			// Note that if we are including widgets there is no need to include
			// clipped regions because they will already be covered by otherwidgets
			// that clipped them
			for (unsigned iB = 0; iB < regionA->m_clipRects.size(); iB++)
				paintRects.push_back(regionA->m_clipRects[iB]);
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Merges paint rectangles in the given list where the union is
	only a little larger than the combined area.  If indicated by the
	parameters, a merge will be avoided where it would result in the union
	overlapping a new region that is not dirty

	Precondition:  paintRects and regionsDownward must be sorted downward by Y-coord
	Postcondition:  the sort order is preserved
*/
void LcCSpace::mergeRectangles(
	LcTmArray<LcTPixelRect>&		paintRects,
	LcTmArray<LcTCanvasRegion*>&	regionsDownward,
	bool							bAvoidDirtying)
{
	bool bScanAgain = true;

	// Combine background regions as much as possible.  Once again this is a multi-pass
	// algorithm which is repeated until no more changes can be made
	while (bScanAgain)
	{
		// Drop out at end of pass by default
		bScanAgain = false;

		// Iterate regions in order of increasing Y
		unsigned iA, iB;
		for (iA = 0; iA < paintRects.size(); iA++)
		{
			LcTPixelRect& rectA = paintRects[iA];

			// Zero width indicates that the rectangle is to be ignored
			if (rectA.getWidth() == 0)
				continue;

			// Check against widgets further down
			for (iB = iA + 1; iB < paintRects.size(); iB++)
			{
				LcTPixelRect& rectB = paintRects[iB];

				// Zero width indicates that the rectangle is to be ignored
				if (rectB.getWidth() == 0)
					continue;

				// Calculate areas of regions
				int areaA				= rectA.getWidth() * rectA.getHeight();
				int areaB				= rectB.getWidth() * rectB.getHeight();

				// Calculate proposed union and its area
				LcTPixelRect unionAB	= rectA.unionWith(rectB);
				int areaUnion			= unionAB.getWidth() * unionAB.getHeight();

				// Calculate intersection
				LcTPixelRect interAB	= rectA.intersection(rectB);
				int areaInter			= interAB.getWidth() * interAB.getHeight();

				// Don't combine if union area is much larger than individual
				if (areaUnion > (areaA + areaB - areaInter + BG_COMBINE_THRESHOLD))
					continue;

				// If we must avoid making unions that would dirty other regions,
				// search now for any such intersections
				bool bAbort = false;
				if (bAvoidDirtying)
				{
					for (unsigned iC = 0; iC < regionsDownward.size(); iC++)
					{
						LcTCanvasRegion* regionC = regionsDownward[iC];

						// If we have gone past union, we are OK
						if (regionC->m_rect.getTop() > unionAB.getBottom())
							break;

						// Ignore hidden areas as it doesn't matter if we "dirty" these
						if (!regionC->m_bPaintWidget && !regionC->m_bPaintBack)
							continue;

						// Ignore widget if it's wholly dirty
						// NB: if there are clip rectangles we must treat the widget as non-dirty
						if (regionC->m_bDirty)
							continue;

						// If widget intersects with union
						// NB: we must do this test using the WIDGET bounding box!
						if (regionC->m_rect.intersects(unionAB))
						{
							// ...we should not combine as the union will create more paints
							// NB: later we will split the rect pair into stripes anyway
							bAbort = true;
							break;
						}
					}
				}

				// Cannot combine
				if (bAbort)
					continue;

				// We now wish to merge the two rectangles - set region A to be the union,
				// and disable region B.  Note that Y of region A is unchanged, so sort
				// order correctness is preserved, and we continue with our union checks
				rectA = unionAB;
				rectB.setLeft(0);
				rectB.setTop(0);
				rectB.setRight(0);
				rectB.setBottom(0);

				// Indicate that we should scan all again later, to try to
				// merge with regions above this one
				bScanAgain = true;
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Scans a list of rectangles, and replaces any intersecting pairs
	with 1-3 rectangles with equivalent coverage but no intersections.  Proceeds
	iteratively until no more intersections are present.

	Pre-condition:  rects must be sorted downwards, by Y-coord
*/
void LcCSpace::makeIntersectionsIntoStripes(LcTmArray<LcTPixelRect>& rects)
{
	// Starting condition
	int nRects = 1;
	bool rescan = true;

	// Keep iterating until we have no further rectangles added
	while (rescan)
	{
		// Assume this is the last pass
		rescan = false;

		// Sort rectangles into order by Y coord of top edge
		// NB: doesn't happen on first iteration
		if (nRects > 1)
			IFX_ShellSort(rects.begin(), rects.end(), orderRectY);

		// We may add rects to the array while processing, but want to
		// exclude them from the current scan
		nRects = (int)rects.size();

		// Iterate in order of increasing Y
		unsigned iA, iB;
		for (iA = 0; iA < (unsigned)nRects; iA++)
		{
			// Zero width indicates that the rectangle is to be ignored
			if (rects[iA].getWidth() == 0)
				continue;

			// Check against regions further down
			for (iB = iA + 1; iB < (unsigned)nRects; iB++)
			{
				// Re-fetch rectA on inner loop, as addition of new rects can
				// cause array memory to be re-allocated
				LcTPixelRect& rectA = rects[iA];
				LcTPixelRect& rectB = rects[iB];

				// Zero width indicates that the rectangle is to be ignored
				if (rectB.getWidth() == 0)
					continue;

				// Calc rect coords
				int rectAright	= rectA.getRight();
				int rectAbottom	= rectA.getBottom();
				int rectBright	= rectB.getRight();
				int rectBbottom = rectB.getBottom();

				// We can stop checking against A once we pass its bottom
				// NB: this confirms vertical intersection, but note that we
				// also detect adjacent (touching) rects because we can merge
				// them if their width is the same
				if (rectB.getTop() >= rectAbottom)
					break;

				// Skip if there's no horizontal intersection
				if (max(rectA.getLeft(), rectB.getLeft()) > min(rectAright, rectBright))
					continue;

				// Quick encapsulation check
				if (rectA.contains(rectB))
				{
					// zero out B, and move on to the next rect...note that in this
					// case we do not need to rescan the rects
					rectB.setLeft(0);
					rectB.setTop(0);
					rectB.setRight(0);
					rectB.setBottom(0);
					continue;
				}

				// Calc coordinates of the middle horizontal stripe
				int midStripeL	= min(rectA.getLeft(), rectB.getLeft());
				int midStripeR	= max(rectAright, rectBright);
				int midStripeT	= rectB.getTop();	// max(rectA.getTop(), rectB.getTop()) is always rectB.getTop()
				int midStripeB;				// not known yet

				// Bottom rect, if we need to create one
				LcTPixelRect rectC;
				bool bAddBottom = false;

				// If there's no bottom stripe
				if (rectAbottom == rectBbottom)
					midStripeB = rectAbottom;
				else
				{
					// We need a bottom stripe
					int botStripeL;
					int botStripeR;
					int botStripeB;

					// Check whether it's part of rect A or B
					if (rectAbottom > rectBbottom)
					{
						// Top of bottom stripe
						midStripeB	= rectBbottom;

						// Form bottom stripe from rect A
						botStripeL	= rectA.getLeft();
						botStripeR	= rectAright;
						botStripeB	= rectAbottom;
					}
					else
					{
						// Top of bottom stripe
						midStripeB	= rectAbottom;

						// Form bottom stripe from rect B
						botStripeL	= rectB.getLeft();
						botStripeR	= rectBright;
						botStripeB	= rectBbottom;
					}

					// If bottom stripe has same X-coords as middle stripe...
					if (botStripeL == midStripeL && botStripeR == midStripeR)
					{
						// ...merge by extending midStripe downwards
						midStripeB	= botStripeB;
					}
					else
					{
						// Otherwise push bottom stripe as a new rectangle
						rectC.setLeft(botStripeL);
						rectC.setRight(botStripeR);
						rectC.setTop(midStripeB);
						rectC.setBottom(botStripeB);

						// Indicate that we want to add this rect
						bAddBottom		= true;
					}
				}

				// If the midstripe is the same x-coords as rect A, or
				// shares the same top edge, we merge them
				if ((rectA.getLeft() == midStripeL && rectAright == midStripeR)
				||	 rectA.getTop() == midStripeT)
				{
					// Merge midstripe into A (top Y coord unchanged)
					rectA.setLeft(midStripeL);
					rectA.setRight(midStripeR);
					rectA.setBottom(midStripeB);

					// If there are rects between A and B then we need to re-check the
					// array, as those rects might now intersect the all-new rectA
					if (iB - iA > 1)
						rescan = true;

					// Null rect B
					rectB.setLeft(0);
					rectB.setTop(0);
					rectB.setRight(0);
					rectB.setBottom(0);
				}
				else
				{
					// Shorten rect A to sit above midstripe (X coords and top Y unchanged)
					rectA.setBottom(midStripeT);

					// Set up rect B (top Y coord unchanged)
					rectB.setLeft(midStripeL);
					rectB.setRight(midStripeR);
					rectB.setBottom(midStripeB);
				}

				// Add bottom rect if required - note that this is down here because
				// we must add it AFTER we have finished with the rectA & rectB
				// references, since it may cause the vector memory to be realloc'ed
				// NB: rectC will not be included in this scan loop, as a re-sort
				// will be required first
				if (bAddBottom)
				{
					rects.push_back(rectC);
					rescan = true;
				}
			} // for iB
		} // for iA
	} // while
}
#endif //!defined (LC_PAINT_FULL)

/*-------------------------------------------------------------------------*//**
	Makes sure that point v is included in the current widget 's bounding box.
	Should be called by each widget 's onWantBoundingBox() handler
*/
LC_EXPORT void LcCSpace::extendBoundingBox(const LcTVector& v)
{
	// Map the point to 2D canvas
	LcTVector gv		= mapLocalToCanvasSubpixel(v);

	int x0				= int(gv.x);
	int x1				= x0;
	int y0				= int(gv.y);
	int y1				= y0;

	// If the point is not integral, our bounding box is actually a 1x1 rect
	// but check that the values returned don't suffer from minor rounding errors that
	// will result in a larger bounding box than necessary
	if(lc_fabs(gv.x - x0) > ZERO_TOLERANCE)
		x1++;
	if(lc_fabs(gv.y - y0) > ZERO_TOLERANCE)
		y1++;

	// Ensure that the box contains the given point
	m_builtBox.setLeft	(min(m_builtBox.getLeft(), x0));
	m_builtBox.setTop	(min(m_builtBox.getTop(), y0));
	m_builtBox.setRight	(max(m_builtBox.getRight(), x1));
	m_builtBox.setBottom(max(m_builtBox.getBottom(), y1));
}

/*-------------------------------------------------------------------------*//**
	Makes sure that 3D rectangle r is included in the current widget 's bounding box.
	Should be called by each widget 's onWantBoundingBox() handler.
	Note that the rectangle is transformed from local 3D space so this is not a
	straightforward rectangle union!
*/
LC_EXPORT void LcCSpace::extendBoundingBox(const LcTPlaneRect& r)
{
	// Find other corner of 3D rect
	LcTScalar right3	= r.getRight();
	LcTScalar top3		= r.getBottom();

	// Quick check for zero-sized rects (for example, empty text elements)
	if (r.getWidth() == 0 || r.getHeight() == 0)
	{
		m_builtBox.setTop(0);
		m_builtBox.setLeft(0);
		m_builtBox.setBottom(0);
		m_builtBox.setRight(0);
		m_builtQuad.setTopLeft(LcTVector(0,0,0));
		m_builtQuad.setTopRight(LcTVector(0,0,0));
		m_builtQuad.setBottomLeft(LcTVector(0,0,0));
		m_builtQuad.setBottomRight(LcTVector(0,0,0));
		return;
	}

	// Iterate corners of 3D rect
	for (int i = 0; i < 4; i++)
	{
		// Map each to 2D
		LcTVector gv = mapLocalToCanvasSubpixel(LcTVector(
			(i & 1)? right3 : r.getLeft(),
			(i & 2)? top3 : r.getTop(),
			r.getZDepth()));

		int x0				= int(gv.x);
		int x1				= x0;
		int y0				= int(gv.y);
		int y1				= y0;

		// If the point is not integral, our bounding box is actually a 1x1 rect
		// but check that the values returned don't suffer from minor rounding errors that
		// will result in a larger bounding box than necessary
		if(lc_fabs(gv.x - x0) > ZERO_TOLERANCE)
			x1++;
		if(lc_fabs(gv.y - y0) > ZERO_TOLERANCE)
			y1++;

		// Extend current bounding box edges (allow margin for rounding)
		m_builtBox.setLeft(min(m_builtBox.getLeft(), x0));
		m_builtBox.setTop(min(m_builtBox.getTop(), y0));
		m_builtBox.setRight(max(m_builtBox.getRight(), x1));
		m_builtBox.setBottom(max(m_builtBox.getBottom(), y1));

		// Cache the canvas coordinates of the rect in a scalar quad,
		switch(i)
		{
		case 0:
			m_builtQuad.setTopLeft(gv);
			break;
		case 1:
			m_builtQuad.setTopRight(gv);
			break;
		case 2:
			m_builtQuad.setBottomLeft(gv);
			break;
		case 3:
			m_builtQuad.setBottomRight(gv);
			break;
		}
	}
}

#ifdef LC_USE_MESHES

/*-------------------------------------------------------------------------*//**
	Calculates the BB for the provided mesh
*/
LC_EXPORT void LcCSpace::calcBoundingBox(const LcTVector& pos, LcCMesh* mesh,
										 int &xLeftFinal, int &yTopFinal,
										 int &xRightFinal, int &yBottomFinal,
										 LcTScalar &z, LcTScalarQuad& sq)
{
	// Initialize the variables.
	xLeftFinal = 99999;
	yTopFinal = 99999;
	xRightFinal = -200000;
	yBottomFinal = -200000;

	LcTScalar right		= mesh->getMaxX();
	LcTScalar left		= mesh->getMinX();
	LcTScalar top		= mesh->getMaxY();
	LcTScalar bottom	= mesh->getMinY();
	LcTScalar front		= mesh->getMaxZ();
	LcTScalar back		= mesh->getMinZ();

	LcTScalar zBackFinal	= 9999;
	LcTScalar zFrontFinal	= -9999;

	z = 0;
	// Iterate corners of 3D rect
	for (int i = 0; i < 8; i++)
	{
		// Map each to 2D
		LcTVector gv = mapLocalToCanvasSubpixel(LcTVector(
			(i & 1) ? right : left,
			(i & 2) ? bottom : top,
			(i > 3) ? back : front));

		int x0 = int(gv.x);
		int x1 = x0;
		int y0 = int(gv.y);
		int y1 = y0;
		LcTScalar z0 = gv.z;

		// If the point is not integral, our bounding box is actually a 1x1 rect
		// but check that the values returned don't suffer from minor rounding errors that
		// will result in a larger bounding box than necessary
		if (lc_fabs(gv.x - x0) > ZERO_TOLERANCE)
			x1++;
		if (lc_fabs(gv.y - y0) > ZERO_TOLERANCE)
			y1++;

		// Extend current bounding box edges (allow margin for rounding)
		xLeftFinal		= (min(xLeftFinal, x0));
		yTopFinal		= (min(yTopFinal, y0));
		xRightFinal		= (max(xRightFinal, x1));
		yBottomFinal	= (max(yBottomFinal, y1));
		zBackFinal		= (min(zBackFinal, z0));
		zFrontFinal		= (max(zFrontFinal, z0));
		z				+= gv.z;
	}

	// Create a scalar quad representing a 2-D slice through the mesh for
	// use with the internal mode Z ordering algorithm.

	sq.setTopLeft(LcTVector((LcTScalar)xLeftFinal
							,(LcTScalar)yTopFinal
							,zFrontFinal));

	sq.setTopRight(LcTVector((LcTScalar)xRightFinal
							,(LcTScalar)yTopFinal
							,(LcTScalar)zFrontFinal));

	sq.setBottomLeft(LcTVector((LcTScalar)xLeftFinal
							,(LcTScalar)yBottomFinal
							,(LcTScalar)zFrontFinal));

	sq.setBottomRight(LcTVector((LcTScalar)xRightFinal
							,(LcTScalar)yBottomFinal
							,zFrontFinal));

	z *= .125;
}

/*-------------------------------------------------------------------------*//**
	Makes sure that 3D mesh m is included in the current widget 's bounding box.
	Should be called by each widget 's onWantBoundingBox() handler.
*/
LC_EXPORT void LcCSpace::extendBoundingBox(const LcTVector& pos, LcCMesh* mesh)
{
	int xLeft, yTop, xRight, yBottom;
	LcTScalar z;

	calcBoundingBox(pos, mesh, xLeft, yTop, xRight, yBottom, z, m_builtQuad);

	// Get right/bottom of bounding box
	int bboxRight	= m_builtBox.getRight();
	int bboxBottom	= m_builtBox.getBottom();

	// Extend current bounding box edges (allow margin for rounding)
	m_builtBox.setLeft(min(m_builtBox.getLeft(), xLeft));
	m_builtBox.setTop(min(m_builtBox.getTop(), yTop));
	m_builtBox.setRight(max(bboxRight, xRight));
	m_builtBox.setBottom(max(bboxBottom, yBottom));
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::processAnimators()
{
	// Get current timestamp - used to draw all animators
	LcTTime timestamp = getTimestamp();

	// Iterate the animator list stored on the space - note that we iterate
	// by index in case any event fired by advanceToTime() causes the
	// vector's allocated storage to be realloc'd and thus maybe moved,
	// which would invalidate any iterator (that's 3 hours of life wasted today)
	for (int i = 0; i < (int)m_animators.size(); i++)
	{
		LcCAnimator* anim = m_animators[i];

		// If the animator entry is NULL, this animator has been aborted
		// during this animation run, so skip it
		if (!anim)
			continue;

		// Process each animator
		if (anim->advanceToTime(timestamp))
		{
			// If advanceToTime() returns true, the animator is stopping so
			// NULL the list entry so that it will be compacted away later
			m_animators[i] = NULL;
		}
		else
		{
			// Animation in progress.  Must revalidate to force another frame
			// in order to process next animation step.  Most animation actions
			// will cause a revalidate anyway, but we call this here in case
			// all current animators are in a delay state, which would not
			revalidate(false);
		}
	}

	// Now all animators have run, compact the animator list
	TmAAnimator::iterator itAnim = m_animators.begin();
	while (itAnim != m_animators.end())
	{
		// Erase NULL items but skip over non-NULL ones
		if (!*itAnim)
			itAnim = m_animators.erase(itAnim);
		else
			itAnim++;
	}
}

/*-------------------------------------------------------------------------*//**
	Returns the porting widget display structure pointer.
*/
IFX_DISPLAY* LcCSpace::getDisplay()
{
	IFX_DISPLAY *display;

	if (IFX_SUCCESS == IFXP_Display_Get_Info(&display))
	{
		return display;
	}

	LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);

	return NULL;
}

/*-------------------------------------------------------------------------*//**
	This loads the background and scales it to fit into the background, whilst
	maintaining it's aspect ratio.
	If an empty string is passed it allows us to clear the background
*/
LC_EXPORT_VIRTUAL bool LcCSpace::setBackgroundImage(const LcTmString& sFile)
{


	bool returnValue = false;

	// Take a copy of the file name for use when suspending and resuming
	m_bgFile = sFile;

#if defined(LC_OGL_DIRECT)
	if (m_bgBitmap)
	{
		m_bgBitmap->release();
		m_bgBitmap = NULL;
	}

	// Clear background and return so not to waste time attempting to load a non existent ndi
	if (sFile.isEmpty())
		return false;

	// Load OGL Bitmap
	m_bgBitmap = (LcOglCBitmap*)getBitmap(sFile);
	if (m_bgBitmap)
	{
		m_bgBitmap->acquire();
		returnValue = true;
	}
	else
	{
		return false;
	}
#endif

#if defined(_LC_PLAT_WANT_CANVAS)
	// Bin old background
	if (m_bgCanvas)
		m_bgCanvas->destroyCanvas();

	// Delete the Background.
	if (m_backgroundImageNdi)
		m_backgroundImageNdi.destroy();

	if (m_backgroundImageCustom)
		m_backgroundImageCustom.destroy();

	// Clear background and return so not to waste time attempting to load a non existent ndi
	if (sFile.isEmpty())
	{
		return false;
	}

	LcTaString ext = sFile.getWord(-1, '.');
	bool isNdi=false;
	if(ext.compareNoCase("ndi")==0)
	{
		// Open image and fail if it's missing or unsuitable
		LcTaOwner<LcCNdiBitmapFile> tempBackgroundImage = LcCNdiBitmapFile::create();
		if (!tempBackgroundImage->open(sFile, LcCNdiBitmapFile::KFormatAny))
		{
			return false;
		}

		// Background has been checked so set the permanent background.
		m_backgroundImageNdi = tempBackgroundImage;
		isNdi=true;
	}
	else
	{
		LcTaOwner<LcCCustomBitmapFile> tempBackgroundImage = LcCCustomBitmapFile::create();
		if (!tempBackgroundImage->open(sFile, LcCCustomBitmapFile::KFormatAny))
		{
			return false;
		}
		void *image_data=NULL;
		IFX_UINT32 length=0;

		if(!tempBackgroundImage->readData(&image_data,0,&length,true))
			return false;

		tempBackgroundImage->releaseData(image_data);
		tempBackgroundImage->close();

		// Background has been checked so set the permanent background.
		m_backgroundImageCustom= tempBackgroundImage;
	}




	// If we are not going to draw directly from the compressed NDI, expand the
	// background into a bitmap buffer instead, so that we can draw from this (faster)
	#if defined(IFX_RENDER_INTERNAL_COMPRESSED)
		returnValue = isNdi;		// Can render only ndi in compressed mode
	#else
		// We have 3 channels, 24bpp
		LcTPixelDim imgSize		= isNdi?m_backgroundImageNdi->getSize():m_backgroundImageCustom->getSize();

		// Create a bitmap to hold the image
		LcTPixelRect rect(0, 0, getCanvasBounds().getWidth(), getCanvasBounds().getHeight());
		if (m_bgCanvas->createCanvas(rect, true))
		{

			// Calculate the size of the image to blit.
			// It is drawn so that it lines up with the center top.
			int	scrnWidth	= getCanvasBounds().getWidth();
			int	scrnHeight	= getCanvasBounds().getHeight();
			int xPos		= 0;
			int yPos		= 0;

			if ((imgSize.width != scrnWidth) || (imgSize.height != scrnHeight))
			{
				xPos = (scrnWidth - imgSize.width) / 2;
				scrnHeight	= imgSize.height;
				scrnWidth	= imgSize.width;
			}

			// Blit the background to the background canvas.
			LcTScalarRect destRectInter((LcTScalar)xPos,
										(LcTScalar)yPos,
										(LcTScalar)0,
										(LcTScalar)xPos + scrnWidth,
										(LcTScalar)yPos + scrnHeight);
			LcTScalarQuad destQuad;
			destRectInter.convertToQuad(destQuad);

			if(isNdi)
			{
				m_bgCanvas->getGfx()->blitNdiBitmap(
					m_backgroundImageNdi.ptr(),
					0,
					LcTPixelDim(0, 0),
					destQuad,
					getCanvasBounds(), // dest clip
					LcTColor(0xFFFFFF),
					LcTScalar(1.0),
					false);
			}
			else
			{
				m_bgCanvas->getGfx()->blitCustomBitmap(
					m_backgroundImageCustom.ptr(),
					0,
					LcTPixelDim(0, 0),
					destQuad,
					getCanvasBounds(), // dest clip
					LcTColor(0xFFFFFF),
					LcTScalar(1.0),
					false);
			}

			returnValue = true;
		}

		// Delete the Background.
		if (m_backgroundImageNdi)
			m_backgroundImageNdi.destroy();

		if (m_backgroundImageCustom)
			m_backgroundImageCustom.destroy();

	#endif
#endif //_LC_PLAT_WANT_CANVAS

	// Cause refresh with full background repaint
	revalidateAll();


	return returnValue;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCSpace::createCanvases()
{
#if defined(_LC_PLAT_WANT_CANVAS)
	// Free old pixmaps (might be resizing etc)
	m_gfxCanvas->destroyCanvas();

	// Bin old background
	if (m_bgCanvas)
		m_bgCanvas->destroyCanvas();

	// Determine dimensions
	LcTPixelRect r = getCanvasBounds();
	if (r.getWidth() == 0)
		return;

	// Create canvas bitmap for drawing to
	if (m_gfxCanvas->createCanvas(r, false) == false)
	{
		// Without a canvas we cannot do anything.
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
		return;
	}

#endif	// defined(_LC_PLAT_WANT_CANVAS)
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCSpace::boundsUpdated()
{
	// Create any canvasses required
	createCanvases();

#if defined(LC_PLAT_OGL)

#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
	emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_RESET, 0);
#endif

#ifndef LC_OGL_DIRECT
	#error Inflexion UI Engine currently only supports direct mode Open GL.
#endif

	IFX_DISPLAY* display = getDisplay();

#if defined(LC_USE_EGL)

	EGLint err;

	// Create EGL surface.
	EGLint surfAttribs[] =
	{
		EGL_NONE, EGL_NONE
	};

	// If we already have a surface...
	if (m_eglSurface != EGL_NO_SURFACE)
	{
		// ...destroy it before re-creating
		eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		if (m_eglSurface != EGL_NO_SURFACE)
		{
			eglDestroySurface(m_eglDisplay, m_eglSurface);
			m_eglSurface = EGL_NO_SURFACE;
		}

		// Delete the context.
		if (m_eglContext != EGL_NO_CONTEXT)
		{
			eglDestroyContext(m_eglDisplay, m_eglContext);
			m_eglContext = EGL_NO_CONTEXT;
		}

		err = eglGetError();

		eglTerminate(m_eglDisplay);
	}

	// Create EGL surface and context.

	int sample_buffers = 0;
	int samples = 0;
	int depthBufferSize = 24;
	int swapInterval = 0;

	if (display->msaa_samples > 1)
	{
		sample_buffers = 1;
		samples = display->msaa_samples;
	}

	if(display->depth_buffer_size != 0)
	{
		depthBufferSize = display->depth_buffer_size;
	}

	if(display->egl_swap_interval != 0)
	{
		swapInterval = display->egl_swap_interval;
	}

	// Properties required for EGL config.
	EGLint attribList[] =
	{
		// Bit depths will have been set earlier
		// Draw to screen
		EGL_SURFACE_TYPE,	EGL_WINDOW_BIT,

		// Define the MSAA configuration.
		EGL_SAMPLE_BUFFERS, sample_buffers,
		EGL_SAMPLES, samples,

		// Use 24-bit depth buffer to prevent depth testing artifacts on certain platforms!
		EGL_DEPTH_SIZE,	depthBufferSize,

		#if defined(LC_PLAT_OGL_20)
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
		#endif

		EGL_NONE
	};

	EGLint numConfigs = 1;
	EGLint majorVersion = 0;
	EGLint minorVersion = 0;

	// Get the default display
	m_eglDisplay = eglGetDisplay(IFXP_Egl_Get_Default_Display());
	if (m_eglDisplay == EGL_NO_DISPLAY)
	{
		err = eglGetError();
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "eglGetDisplay Failed");
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

	// Initialize the EGL layer
	if (!eglInitialize(m_eglDisplay, &majorVersion, &minorVersion))
	{
		err = eglGetError();
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "eglInitialize Failed");
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

	// Find a suitable config
	if ((!eglGetConfigs(m_eglDisplay, NULL, 0, &numConfigs)) || (numConfigs < 1))
	{
		err = eglGetError();
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "eglGetConfigs Failed");
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

	// Allocate space for the configs and choose one based on the attribList
	if (m_eglConfig)
		m_eglConfig.free();

	m_eglConfig.alloc(sizeof(EGLConfig) * numConfigs);

#if !defined(NDHS_JNI_INTERFACE)
	// nVidia platforms use CSAA in place of MSAA.
	char const * extensions = eglQueryString(m_eglDisplay, EGL_EXTENSIONS);
	EGLint errorCode = eglGetError();
	LcTaString eglExtensions = (extensions == NULL) ? "" : extensions;
	if ((errorCode == EGL_SUCCESS) && (eglExtensions.find("EGL_NV_coverage_sample") > 0))
	{
		// Exchange the MSAA IDs for CSAA IDs
		attribList[2] = EGL_COVERAGE_BUFFERS_NV;
		attribList[4] = EGL_COVERAGE_SAMPLES_NV;
	}
#endif

	if (!eglChooseConfig(m_eglDisplay, attribList, m_eglConfig, numConfigs, &numConfigs))
	{
		err = eglGetError();
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "eglChooseConfig Failed");
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

	// Check that at least one configuration matches
	if (numConfigs < 1)
	{
		err = eglGetError();
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "No Configs Found");
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

	// Choose the best configuration out of the list provided.
	// If there is no exact match, default to the first returned.
	EGLint i;
	EGLint r = 0;
	EGLint g = 0;
	EGLint b = 0;
	EGLint a = 0;
	EGLint s = 0;
	EGLint d = 0;
	EGLint config = 0;
	EGLint bestFitMsaa = -1;
	EGLint targetSamples = display->msaa_samples;

	if (targetSamples == 1)
		targetSamples = 0;

#if defined(IFX_DEBUG_MODE)
	LcTaString eglConfigLog = "";
	for (i = 0; i < numConfigs; i++)
	{
		eglGetConfigAttrib(m_eglDisplay, m_eglConfig[i], EGL_RED_SIZE, &r);
		eglGetConfigAttrib(m_eglDisplay, m_eglConfig[i], EGL_GREEN_SIZE, &g);
		eglGetConfigAttrib(m_eglDisplay, m_eglConfig[i], EGL_BLUE_SIZE, &b);
		eglGetConfigAttrib(m_eglDisplay, m_eglConfig[i], EGL_ALPHA_SIZE, &a);
		eglGetConfigAttrib(m_eglDisplay, m_eglConfig[i], EGL_DEPTH_SIZE, &d);
		eglGetConfigAttrib(m_eglDisplay, m_eglConfig[i], EGL_SAMPLES, &s);
		LcTaString eglConfigLog = "EGL Config[" + LcTaString().fromInt(i) + "] Attribute sizes: red = " + LcTaString().fromInt(r) + ", green = " + LcTaString().fromInt(g) + ", blue = " + LcTaString().fromInt(b) + ", alpha = " + LcTaString().fromInt(a) + ", depth = " + LcTaString().fromInt(d) + ", samples = " + LcTaString().fromInt(s);
		NDHS_TRACE(ENdhsTraceLevelInfo, ENdhsTraceModuleOGL, eglConfigLog);
	}
#endif

	for (i = 0; i < numConfigs; i++)
	{
		bool configMatch = false;

		eglGetConfigAttrib(m_eglDisplay, m_eglConfig[i], EGL_RED_SIZE, &r);
		eglGetConfigAttrib(m_eglDisplay, m_eglConfig[i], EGL_GREEN_SIZE, &g);
		eglGetConfigAttrib(m_eglDisplay, m_eglConfig[i], EGL_BLUE_SIZE, &b);
		eglGetConfigAttrib(m_eglDisplay, m_eglConfig[i], EGL_ALPHA_SIZE, &a);
		eglGetConfigAttrib(m_eglDisplay, m_eglConfig[i], EGL_SAMPLES, &s);

		switch (display->bpp)
		{
			case 16:
				if (r == 5 && g == 6 && b == 5)
					configMatch = true;
			break;
			case 24:
				if (r == 8 && g == 8 && b == 8 && a == 0)
					configMatch = true;
			break;
			case 32:
				if (r == 8 && g == 8 && b == 8 && a == 8)
					configMatch = true;
			break;
			default:
				if (r == IFXP_CANVAS_BITS_RED && g == IFXP_CANVAS_BITS_GREEN && b == IFXP_CANVAS_BITS_BLUE)
					configMatch = true;
			break;
		}

		if (configMatch)
		{
			if (s >= targetSamples)
			{
				if ((s < bestFitMsaa) || (bestFitMsaa < targetSamples))
				{
					// Better match than our current best
					config = i;
					bestFitMsaa = s;
				}
			}
			else
			{
				if (s > bestFitMsaa)
				{
					// Better match than our current best
					config = i;
					bestFitMsaa = s;
				}
			}
		}
	}

	// Attach rendering surface to config (OGL Direct)
	m_eglSurface = eglCreateWindowSurface(m_eglDisplay, m_eglConfig[config], IFXP_Egl_Get_Default_Window(), surfAttribs);

	if (m_eglSurface == EGL_NO_SURFACE)
	{
		err = eglGetError();
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "eglSurface has not been created");
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

	// Create context using chosen config
#if !defined(LC_PLAT_OGL_20)
	m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig[config], m_eglContext, NULL);
#else
	EGLint ai32ContextAttribs[3] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig[config], m_eglContext, ai32ContextAttribs);
#endif

	if (m_eglContext == EGL_NO_CONTEXT)
	{
		err = eglGetError();
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "eglContext has not been created");
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

	// Switch to this space's context (ignore error)
	eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
	err = eglGetError();
	if (err != EGL_SUCCESS)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "eglMakeCurrent Failed");
	}

	eglSwapInterval(m_eglDisplay, swapInterval);

	err = eglGetError();
	if (err != EGL_SUCCESS)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "eglSwapInterval Failed");
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

#if defined (NDHS_TRACE_ENABLED) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
	// Retrieve the actual configuration values.
	EGLint st;
	eglGetConfigAttrib(m_eglDisplay, m_eglConfig[config], EGL_RED_SIZE, &r);
	eglGetConfigAttrib(m_eglDisplay, m_eglConfig[config], EGL_GREEN_SIZE, &g);
	eglGetConfigAttrib(m_eglDisplay, m_eglConfig[config], EGL_BLUE_SIZE, &b);
	eglGetConfigAttrib(m_eglDisplay, m_eglConfig[config], EGL_ALPHA_SIZE, &a);
	eglGetConfigAttrib(m_eglDisplay, m_eglConfig[config], EGL_DEPTH_SIZE, &d);
	eglGetConfigAttrib(m_eglDisplay, m_eglConfig[config], EGL_STENCIL_SIZE, &st);
	eglGetConfigAttrib(m_eglDisplay, m_eglConfig[config], EGL_SAMPLES, &s);

#ifdef NDHS_TRACE_ENABLED
	LcTaString eglConfig = "EGL Selected Config[" + LcTaString().fromInt(config) + "] Attribute sizes: red = " + LcTaString().fromInt(r) + ", green = " + LcTaString().fromInt(g) + ", blue = " + LcTaString().fromInt(b) + ", alpha = " + LcTaString().fromInt(a) + ", depth = " + LcTaString().fromInt(d) + ", samples = " + LcTaString().fromInt(s);
	NDHS_TRACE(ENdhsTraceLevelInfo, ENdhsTraceModuleOGL, eglConfig);
#endif	//defined NDHS_TRACE_ENABLED

#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
	if (s == 0) s = 1;
	emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, display->width * display->height * ((r + g + b + a) * s / 8));
	emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, display->width * display->height * (d + st) / 8);
#endif

#endif // defined (NDHS_TRACE_ENABLED) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)

#endif // defined(LC_USE_EGL)

	// Do common OpenGL configuration
	LC_ASSERT(m_oglContext);

	if (display->bpp < 24)
	{
		m_oglContext->setEnableDithering(true);
	}
	else
	{
		m_oglContext->setEnableDithering(false);
	}

	m_oglContext->setupOpenGL();

#if defined(LC_PLAT_OGL_20)

	// Unload Effects ....

	// Un-load default effects (normal + enhanced)
	m_oglContext->unloadDefaultEffects ();

	// Un-load Custom effects
	m_oglContext->unloadCustomEffects ();

	// Load Effects ...

	// Load Custom effects
	m_oglContext->loadCustomEffects();

	// Load default effects (normal + enhanced)
	m_oglContext->loadDefaultEffects();

#endif
	m_oglContext->boundsUpdated();

#endif // defined(LC_PLAT_OGL)

	updatePrimaryLight();

#if defined(LC_PLAT_OGL)
#if defined(LC_USE_LIGHTS)
	// Find out how many secondary lights exist.
	// NOTE: Remember GL_LIGHT0 is reserved for the primary light source.
	#if defined(LC_PLAT_OGL_20)
		m_maxSecondaryLights = IFX_OGL_NUM_LIGHTS;
	#else
		glGetIntegerv(GL_MAX_LIGHTS, &m_maxSecondaryLights);
	#endif

	m_maxSecondaryLights = max(0, m_maxSecondaryLights - 1);

	// Clear any existing lights.
	m_secOwnedLightPool.clear();
	m_secAllocatedLights.clear();

	// Create the lights
	int index = 1;
	for (index = 1; index <= m_maxSecondaryLights; index++)
	{
		// Create light
		LcTaOwner<LcCLight> secondaryLight = LcOglCLight::create(this, index);
		m_secAllocatedLights.insert(TmMAllocatedLights::value_type(secondaryLight.ptr(), NULL));
		m_secOwnedLightPool.push_back(secondaryLight);
	}
#endif // defined(LC_USE_LIGHTS)
#endif // defined(LC_PLAT_OGL)

}

/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::onActivate()
{
#if defined(LC_PLAT_OGL) && defined(LC_USE_EGL)
	// Switch to this space's context (ignore error)
	eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
#endif
}

/*-------------------------------------------------------------------------*//**
	Event handler; do not call directly.
*/
void LcCSpace::paintCanvas()
{

	unsigned							i;
	int									nVisibleWidgets = 0;
	typedef LcTaArray<LcCWidget*>		TaAWidget;
	typedef LcTaArray<LcTCanvasRegion*>	TaARegion;
	TaAWidget::iterator					itWidget;
	TaARegion::iterator					itRegion;
	LcTaArray<LcTPixelRect>				paintRects;
	LcTaAlloc<int>						regionOverlapArray;

#if defined (LC_PLAT_OGL)
	int numDrawLayers = 1;
	int depthSegment = 0;
	int prevDrawLayerIndex = 0;
#endif


	// ******* Prepare widgets to be hidden, where required *******


	// Call LcCWidget::onPrepareForHide on widgets that need it
	// Also need to count the number of visible widgets
	for (itWidget = m_widgets.begin();
		 itWidget != m_widgets.end();
		 itWidget++)
	{
		bool bIsVisible = (*itWidget)->isVisible();

		if (bIsVisible
#ifdef IFX_USE_PLUGIN_ELEMENTS
			&& (!m_bFullScreenMode || (m_bFullScreenMode && (*itWidget)->isFullScreen()))
#endif
		)
		{
			// Need the count of visible widgets to allocate the
			// list of new regions later
			nVisibleWidgets++;
			m_paintWidget = *itWidget;
		}
		else if ((*itWidget)->m_bPreviousVisible)
		{
			// If the widget was visible and is now being hidden
			// must call onPrepareForHide
			(*itWidget)->onPrepareForHide();
		}

		// State will be used to trigger m_pPreviousVisible next time.
		// Note that it also prevents multiple calls to onPrepareForHide()
		// if the widget has multiple widgets
		(*itWidget)->m_bPreviousVisible = bIsVisible;
	}



	// ******* Draw the lights *******

	// Setup the Primary light.
	updatePrimaryLight();

#ifdef LC_USE_LIGHTS
#ifndef LC_PAINT_FULL
	bool lightPlacementChanged = false;
#endif
	TmALightWidget secLightsToDraw;

	// Draw the first n secondary lights.
	TmALightWidget::reverse_iterator revItWidget;
	for (revItWidget = m_lightWidgets.rbegin();
		 (revItWidget != m_lightWidgets.rend()) && (m_numberSecLightsAllocated <= m_maxSecondaryLights);
		 revItWidget++)
	{
		LcwCLight* pWidget = *revItWidget;
		bool bIsHidden = pWidget->isHidden();

		if (!bIsHidden)
		{
			if (pWidget->isDirty())
			{
#ifndef LC_PAINT_FULL
				lightPlacementChanged = true;
#endif
				// Add to the drawing list.
				secLightsToDraw.push_back(pWidget);
			}
		}
		else if (pWidget->m_bPreviousVisible)
		{
#ifndef LC_PAINT_FULL
			lightPlacementChanged = true;
#endif
			// If the widget was visible and is now being hidden
			// must call onPrepareForHide
			pWidget->onPrepareForHide();

			// Free up the light from the pool.
			pWidget->setLight(NULL);
		}

		// State will be used to trigger m_pPreviousVisible next time.
		// Note that it also prevents multiple calls to onPrepareForHide()
		// if the widget has multiple widgets
		pWidget->m_bPreviousVisible = !bIsHidden;
	}

#ifndef LC_PAINT_FULL
	// If this is not full screen mode and the lights have changed revalidate
	// the scene to ensure all the items affected by the light are redrawn.
	// NOTE: This must be done before the lights are drawn.
	if (lightPlacementChanged)
	{
		// If the lights have changed revalidate the scene to ensure all the items
		// affected by the light are redrawn.
		// NOTE: This must be done before the lights are drawn.
		revalidateAll(true);
	}
#endif

	// Now draw the lights.
	if (!secLightsToDraw.empty())
	{
		TmALightWidget::iterator drawLightIter = secLightsToDraw.begin();
		while (drawLightIter != secLightsToDraw.end())
		{
			// Shut down all the lights, by identifying them in the tables.
			LcwCLight* currWidget = *drawLightIter;

			// If the widget needs a light then try to get one.
			if (!currWidget->getLight())
			{
				LcCLight* newLight = getSecondaryLight(currWidget);
				if (newLight)
				{
					currWidget->setLight(newLight);
				}
			}

			// If a light is available then draw it.
			if (currWidget->getLight())
			{
				// Draw the light - record the light widget, as it may be needed
				// for a call to transformsChanged() during 'onPaint'.
				m_paintWidget = currWidget;
				currWidget->doPrepareForPaint();
				currWidget->onPaint(getCanvasBounds());
			}

			++drawLightIter;
		}

		// Clean up.
		secLightsToDraw.clear();
	}
#endif // defined(LC_USE_LIGHTS)

#if !defined(LC_PAINT_FULL)
	TaARegion regionsDownward;
#endif //!defined(LC_PAINT_FULL)

#ifdef IFX_USE_PLUGIN_ELEMENTS
	if (!m_bFullScreenMode)
#endif
	{
		// ******* Build widget and region lists *******


#if !defined(LC_PAINT_FULL)
		// List of regions to be painted.  For widgets that have been hidden or
		// destroyed, we will have background regions only.  For widgets that are
		// not dirty, or which are new, we will have widget regions only.  And for
		// widgets that are dirty, we will have separate background and widget
		// regions (in case the widget has moved)
		regionsDownward.reserve(m_regions.size() + nVisibleWidgets);

		// Iterate last widget list to give background regions
		for (itRegion = m_regions.begin();
			 itRegion != m_regions.end();
			 itRegion++)
		{
			LcTCanvasRegion* region = *itRegion;

			// Background region is dirty if the widget has been destroyed or the
			// widget has become invisible or the widget is dirty (may have moved)
			region->m_bDirty = !region->m_widget || !region->m_widget ->isVisible()
				|| region->m_widget ->isDirty();

	#ifdef IFX_USE_PLUGIN_ELEMENTS
			 region->m_bDirty =region->m_bDirty || (m_bFullScreenMode && !region->m_widget->isFullScreen());
	#endif
			// Don't bother creating a background entry where a widget is not
			// dirty, as there will be a corresponding widget region entry for it.
			if (region->m_bDirty)
			{
				// Update record to paint background at widget 's previous position
				region->m_widget = NULL;
				region->m_bPaintBack	= true;
				region->m_bPaintWidget	= false;
				region->m_bOpaque		= false;
				region->m_bBlackout		= false;
				region->m_bCanBeClipped	= false;

				// Add to ordering
				// NB: clip rects will be empty from last frame
				regionsDownward.push_back(region);
			}
		}
#endif //LC_PAINT_FULL

		// Finished with old widget list - note records will survive
		m_regions.clear();

		// Just in case last block was left over due to some kind of failure
		m_newRegionMem.free();

		// Create new block for region info (not background regions)
		// NB: cleanup safe: already attached to space (events fired below may leave)
		// NB2: we attach to space immediately because m_regions points into this
		m_newRegionMem.alloc(nVisibleWidgets);
		m_regions.reserve(nVisibleWidgets);

		// Add region records for new widgets to both lists
		for (itWidget = m_widgets.begin(), i = 0;
			 itWidget != m_widgets.end();
			 itWidget ++)
		{
			// Set up region records for visible widgets .  We create these
			// entries for both dirty and non-dirty widgets (non-new dirty widgets
			// will also have a separate background region, see above).
			// Note that we don't know the rects for these until they have
			// been prepared
			if ((*itWidget)->isVisible()
	#ifdef IFX_USE_PLUGIN_ELEMENTS
			 && (!m_bFullScreenMode || (m_bFullScreenMode && (*itWidget)->isFullScreen()))
	#endif
			)
			{
				// Initialize paint region for this widget
				m_newRegionMem[i].m_widget			= *itWidget;
				m_newRegionMem[i].m_bPaintWidget		= true;
	//			m_newRegionMem[i].m_bPaintBack		= false;
				m_newRegionMem[i].m_bDirty			= (*itWidget)->isDirty();
				m_newRegionMem[i].m_clipRects		= LcTmArray<LcTPixelRect>();
				m_newRegionMem[i].m_bBlackout		= (*itWidget)->isBlackout();
				m_newRegionMem[i].m_bOpaque			= m_newRegionMem[i].m_bBlackout || (*itWidget)->isOpaque();
				m_newRegionMem[i].m_bCanBeClipped	= (*itWidget)->canBeClipped();
				m_newRegionMem[i].m_drawLayerIndex  = (*itWidget)->getDrawLayerIndex();

				// NB: this is not maximally efficient, but has been reverted from the
				// "false" setting above to temporarily work around a painting glitch
				m_newRegionMem[i].m_bPaintBack		= true;

				// Add to both lists
				m_regions.push_back(&m_newRegionMem[i]);
#if !defined(LC_PAINT_FULL)
				regionsDownward.push_back(&m_newRegionMem[i]);
#endif //!defined(LC_PAINT_FULL)

				i++;
			}
		}


		// ******* Prepare widgets for painting *******


		// Reset, for mapping to bounding boxes
		m_paintWidget = NULL;

		// List of regions to delete from the region list.
		TaARegion eraseList;

		// Prepare widgets and update bounding boxes
		// NB: cannot do in above loop, as preparing one widget may prepare others in
		// same widget, and this resets dirty flags before we have read them
#if defined (LC_PLAT_OGL)
		prevDrawLayerIndex = NDHS_LOWER_INVALID_DRAW_LAYER_INDEX;
#endif
		for (itRegion = m_regions.begin();
			 itRegion != m_regions.end();
			 itRegion++)
		{
			LcTCanvasRegion* region = *itRegion;

			// Keep track of widget being painted
			m_paintWidget = region->m_widget;

			// Prepare widget
			m_paintWidget->doPrepareForPaint();

			// Call getBoundingBox() to collect points
			m_builtBox = LcTPixelRect(99999, 99999, -200000, -200000);
			m_builtQuad = LcTScalarQuad();

			m_paintWidget->onWantBoundingBox();

			// Other region bits
			region->m_rect		= m_recBounds.intersection(m_builtBox);

			// Check whether the widget is outside the space
			if (region->m_rect.getWidth() == 0 || region->m_rect.getHeight() == 0)
			{
				// If so, there's no need to paint it
				region->m_bPaintWidget	= false;
				region->m_bPaintBack	= false;

				// Remove from the region list
				eraseList.push_back(region);
				continue;
			}

			region->m_dZ		= region->m_widget->getGlobalZ(region->m_bRotated,
																region->m_bParentAggRotated);
#if defined(IFX_RENDER_INTERNAL) || defined (LC_PLAT_OGL)
			region->m_quad		= m_builtQuad;
			region->m_dZ_max	= max(max( m_builtQuad.getTopLeft().z, m_builtQuad.getTopRight().z),
									  max( m_builtQuad.getBottomLeft().z, m_builtQuad.getBottomRight().z));
			region->m_dZ_min	= min(min( m_builtQuad.getTopLeft().z, m_builtQuad.getTopRight().z),
									  min( m_builtQuad.getBottomLeft().z, m_builtQuad.getBottomRight().z));
#endif
		}

		// Remove any invalid regions from the region list
		for (TaARegion::iterator itErase = eraseList.begin();
			 itErase != eraseList.end();
			 itErase++)
		{
			for (itRegion = m_regions.begin();
				 itRegion != m_regions.end();
				 itRegion++)
			{



				if (*itErase == *itRegion)
				{
					m_regions.erase(itRegion);
					break;
				}
			}
		}

		// Sort paint regions now we have bounding boxes
		IFX_ShellSort(m_regions.begin(), m_regions.end(), orderZ);
	#if defined(IFX_RENDER_INTERNAL) || defined (LC_PLAT_OGL)
		// Table indicating whether regions overlap - this has m_regions.size() rows
		// (one for each region) and m_regions.size() + 2 columns. The first column
		// stores the pointer to the relevant region in int form, so that we can
		// use the table even after the m_regions array has been sorted, and the final
		// column indicates the number of regions that are on top of the current region,
		// used only within the drawSort_3d function.
#if !defined (LC_PAINT_FULL)
		m_regionsOverlap.clear();
		m_regionOverlapArraySize.clear();
#endif

		if (m_regions.size() > 0)
		{
			//Now the regions with same draw Layer indices will be sort by the drawSort_3d
			LcTaArray<LcTCanvasRegion*> regionsSubSort;
			LcTaArray<LcTCanvasRegion*>::reverse_iterator it;
			int drawLayerId = m_regions[0]->m_drawLayerIndex;
			unsigned index = 0,count = 0;

			while (index < m_regions.size())
			{
				regionsSubSort.clear();

				for( ;index < m_regions.size(); index++)
				{
					if (drawLayerId != m_regions[index]->m_drawLayerIndex)
						break;

					regionsSubSort.push_back(m_regions[index]);
				}

				if(index < m_regions.size())
				{
					drawLayerId = m_regions[index]->m_drawLayerIndex;
#if defined (LC_PLAT_OGL)
					numDrawLayers++;
#endif
				}

				// Allocate the overlap array for regions with same drawLayerIndex
				regionOverlapArray.alloc((int)(regionsSubSort.size() * (regionsSubSort.size() + 2)));

				drawSort_3d(regionsSubSort, regionOverlapArray);

#if !defined (LC_PAINT_FULL)
				//Store the overlap array
				m_regionsOverlap.push_back(regionOverlapArray.release());

				//Store the number of regions used for overlap array
				m_regionOverlapArraySize.push_back(regionsSubSort.size());
#else
				// The array isn't needed in paint full mode.
				regionOverlapArray.free();
#endif // !defined (LC_PAINT_FULL)

				for (it = regionsSubSort.rbegin(), count = (index -1); it !=  regionsSubSort.rend(); it++, count--)
				{
					m_regions[count]= *it;
				}
			}
		}
	#endif

#if !defined(LC_PAINT_FULL)
		IFX_ShellSort(regionsDownward.begin(), regionsDownward.end(), orderRegionY);



		// ******* Check for widget overlaps that require additional paints *******


		// Scan state
		bool bScanAgain	= true;
		bool bFirstPass	= true;

		// Check for widgets that should be marked dirty due to intersections.
		// This algorithm performs repeated passes until no more changes result.
		while (bScanAgain)
		{
			// After second pass, drop out by default unless bScanAgain gets set
			bScanAgain = bFirstPass;

			// Iterate regions in order of increasing Y
			unsigned iA, iB;
			for (iA = 0; iA < regionsDownward.size(); iA++)
			{
				LcTCanvasRegion* regionA = regionsDownward[iA];

				// Ignore regions that are not to be painted
				if (!(regionA->m_bPaintWidget || regionA->m_bPaintBack))
					continue;

				// Check against regions further down
				for (iB = iA + 1; iB < regionsDownward.size(); iB++)
				{
					LcTCanvasRegion* regionB = regionsDownward[iB];

					// We can stop checking against A once we pass its bottom
					if (regionB->m_rect.getTop() > regionA->m_rect.getBottom())
						break;

					// Ignore regions that are not to be painted
					if (!(regionB->m_bPaintWidget || regionB->m_bPaintBack))
						continue;

					// Not interested in pairs of backgrounds
					if (!(regionA->m_bPaintWidget || regionB->m_bPaintWidget))
						continue;

					// We are only interested in intersections between widgets
					LcTPixelRect intersection = regionA->m_rect.intersection(regionB->m_rect);
					if (intersection.getWidth() == 0)
						continue;

					// Front and back
					LcTCanvasRegion* regionFront;
					LcTCanvasRegion* regionBehind;

					// Determine which is in front
					if (!regionB->m_bPaintWidget
	#if defined(IFX_RENDER_INTERNAL) || defined (LC_PLAT_OGL)
						|| (regionA->m_bPaintWidget && (orderZ_3d(regionB, regionA,
														m_regionsOverlap, m_regionOverlapArraySize) == false)))
	#else
						|| (regionA->m_bPaintWidget && regionA->m_dZ > regionB->m_dZ))
	#endif
					{
						regionFront		= regionA;
						regionBehind	= regionB;
					}
					else
					{
						regionFront		= regionB;
						regionBehind	= regionA;
					}

					// On first pass we only do enclosure check...
					if (bFirstPass)
					{
						// If the front widget is opaque and fully encloses the one behind,
						// mark the back widget as hidden as there will be no need to paint it.
						// Note that we must leave obscured background paints alone (these erase)
						if (regionFront->m_bOpaque && regionBehind->m_bPaintWidget
						&&	regionFront->m_rect.contains(regionBehind->m_rect))
						{
							regionBehind->m_bPaintWidget	= false;
							regionBehind->m_bPaintBack	= false;
						}

						// If the behind widget is a 'blackout' widget and fully encloses
						// the one in front, mark the front widget as hidden
						if (regionBehind->m_bBlackout
						&&	regionBehind->m_rect.contains(regionFront->m_rect))
						{
							regionFront->m_bPaintWidget = false;
							regionFront->m_bPaintBack  = false;
						}
					}
					else // Second and subsequent passes
					{
						// If widget behind is dirty and NOT a blackout widget, widget in front
						// will need to be redrawn, and the background too (since otherwise
						// we would be compositing the behind widget over the front one)
						if (regionBehind->m_bDirty && !regionBehind->m_bBlackout
						&&	!regionFront->m_bDirty)
						{
							// Must paint background - note that the front region is a widget,
							// otherwise it couldn't be in front
							regionFront->m_bPaintBack = true;

							// If the dirtied widget can be clipped, we can draw it more efficiently
							if (regionFront->m_bCanBeClipped)
							{
								// Painting only the clipped area can't render any other
								// area dirty so there's no need to repeat the scan
								regionFront->m_clipRects.push_back(intersection);
							}
							else
							{
								// Can't clip overlapped object so we need to repaint the whole
								// thing, which might result in further overlaps so scan again
								regionFront->m_bDirty	= true;
								bScanAgain				= true;
							}
						}

						// If front widget is dirty AND is painting its background, the widget
						// behind will need to be redrawn too (if the front widget is not painting
						// its background, we are OK to draw it over the top)
						else if (regionFront->m_bDirty && regionFront->m_bPaintBack
							&&	!regionBehind->m_bDirty && !regionBehind->m_bBlackout)
						{
							// Must paint background - note that the behind region must be
							// a widget, otherwise it wouldn't be non-dirty
							regionBehind->m_bPaintBack = true;

							// If the dirtied widget can be clipped, we can draw it more efficiently
							if (regionBehind->m_bCanBeClipped)
							{
								// Painting only the clipped area can't render any other
								// area dirty so there's no need to repeat the scan
								regionBehind->m_clipRects.push_back(intersection);
							}
							else
							{
								// Can't clip overlapped object so we need to repaint the whole
								// thing, which might result in further overlaps so scan again
								regionBehind->m_bDirty	= true;
								bScanAgain				= true;
							}
						}
					}
				}
			}

			// No need to state the obvious here
			bFirstPass = false;
		}

	#if defined(IFX_RENDER_INTERNAL) || defined (LC_PLAT_OGL)
		for (unsigned k = 0; k < m_regionsOverlap.size(); k++)
		{
			regionOverlapArray.attach(m_regionsOverlap[k]);
			regionOverlapArray.free();
		}

		m_regionsOverlap.clear();
		m_regionOverlapArraySize.clear();
	#endif

		// Ensure that clipped regions are optimal
		unsigned iA;
		for (iA = 0; iA < regionsDownward.size(); iA++)
		{
			LcTCanvasRegion* regionA = regionsDownward[iA];
			if (!regionA->m_clipRects.empty())
			{
				// Clip rectangles must be minimal and non-overlapping
				IFX_ShellSort(regionA->m_clipRects.begin(), regionA->m_clipRects.end(), orderRectY);
				makeIntersectionsIntoStripes(regionA->m_clipRects);
			}
		}
#endif //!defined(LC_PAINT_FULL)

	}
#ifdef IFX_USE_PLUGIN_ELEMENTS
	else
	{
		// Finished with old widget list - note records will survive
		m_regions.clear();

		// Just in case last block was left over due to some kind of failure
		m_newRegionMem.free();

		// proceed only if fullscreen widget is found
		if (m_paintWidget!=NULL)
		{
			// nVisibleWidgets will be 1 here because only one plugin element can have
			// fullscreen mode at a time
			m_newRegionMem.alloc(nVisibleWidgets);
			m_regions.reserve(nVisibleWidgets);

			// Initialize paint region for this widget
			m_newRegionMem[0].m_widget			= m_paintWidget;
			m_newRegionMem[0].m_bPaintWidget		= true;
			m_newRegionMem[0].m_bDirty			= m_paintWidget->isDirty();
			m_newRegionMem[0].m_clipRects		= LcTmArray<LcTPixelRect>();
			m_newRegionMem[0].m_bBlackout		= m_paintWidget->isBlackout();
			m_newRegionMem[0].m_bOpaque			= m_newRegionMem[0].m_bBlackout || m_paintWidget->isOpaque();
			m_newRegionMem[0].m_bCanBeClipped	= m_paintWidget->canBeClipped();
			m_newRegionMem[0].m_drawLayerIndex	= m_paintWidget->getDrawLayerIndex();
			m_newRegionMem[0].m_bPaintBack		= true;

			m_regions  	   .push_back(&m_newRegionMem[0]);
#if !defined(LC_PAINT_FULL)
			regionsDownward	   .push_back(&m_newRegionMem[0]);
#endif //!defined(LC_PAINT_FULL)

			LcTCanvasRegion* region = &m_newRegionMem[0];

			// Prepare widget
			// NB: note that widget dirty flag is observed within this, but we must
			// call this anyway to give the widget the option to prepare all its widgets
			m_paintWidget->doPrepareForPaintIfDirty();

			// Call getBoundingBox() to collect points
			m_builtBox = LcTPixelRect(99999, 99999, -200000, -200000);
			m_builtQuad = LcTScalarQuad();

			m_paintWidget->onWantBoundingBox();

			// Other region bits
			region->m_rect		= m_recBounds.intersection(m_builtBox);
			region->m_dZ		= region->m_widget->getGlobalZ(region->m_bRotated,
																region->m_bParentAggRotated);
		}
	}
#endif

	// ******* Paint opaque layers to canvas *******
#ifdef LC_PLAT_OGL
	// Disable blending and allow depth buffer updates.
	// Note that depth buffer testing is always enabled, only writing is controlled here.
	getOglContext()->setBlend(GL_FALSE);
	m_depthMask = true;
	getOglContext()->setDepthMask(GL_TRUE);

	glDepthFunc(GL_LEQUAL);

	// Reset the current widget.
	m_paintWidget = NULL;

	// Paint opaque layers front to back
	TmARegion::reverse_iterator itRevRegion;
	for (itRevRegion = m_regions.rbegin(), prevDrawLayerIndex = NDHS_LOWER_INVALID_DRAW_LAYER_INDEX;
		 itRevRegion != m_regions.rend();
		 itRevRegion++)
	{
		LcTCanvasRegion* region = *itRevRegion;

		if (prevDrawLayerIndex != region->m_drawLayerIndex)
		{
			float segmentSize = 1.0f / numDrawLayers;
			float segment = depthSegment * segmentSize;
			LC_OGL_DEPTH_RANGE(LC_OGL_FROM_FLOAT(segment), LC_OGL_FROM_FLOAT(segment + segmentSize));
			prevDrawLayerIndex = region->m_drawLayerIndex;
			depthSegment++;
		}

		// Note that in debug mode we paint non-dirty layers too!
		if (region->m_bPaintWidget && !region->m_bBlackout)
		{
			// If widget is different to last widget painted...
			m_paintWidget = region->m_widget;

			setLightModel();
			m_paintWidget->onPaintOpaque(m_recBounds);
		}
	}

#endif //def LC_PLAT_OGL


	// ******* Paint background to canvas *******


	if (!startPaintBackground())
	{
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

	// Reset paint state
	// NB: do before background paints, in case they test canSketch
	m_paintWidget = NULL;

	// In full mode paint entire background first
#if defined(LC_PAINT_FULL)
	if (!paintBackground(m_recBounds))
	{
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}
#else
	#ifdef IFX_USE_PLUGIN_ELEMENTS
	if (!m_bFullScreenMode)
	#endif
	{
		// Build list of background rects and sort them
		buildRegionRectangleList(paintRects, regionsDownward, false);
		IFX_ShellSort(paintRects.begin(), paintRects.end(), orderRectY);

		// Merge any major overlaps but avoid overlapping any non-dirty widgets
		mergeRectangles(paintRects, regionsDownward, true);

		// Optimize rects so that each pixel is covered a maximum of once
		makeIntersectionsIntoStripes(paintRects);

		unsigned iA;
		// Otherwise paint individual background regions
		for (iA = 0; iA < paintRects.size(); iA++)
		{
			// Skip zeroed rects
			const LcTPixelRect& rect = paintRects[iA];
			if (rect.getWidth() > 0)
			{
				if (!paintBackground(rect))
				{
					LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
				}
			}
		}
	}
	#ifdef IFX_USE_PLUGIN_ELEMENTS
	else if (!paintBackground(m_recBounds))
	{
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}
	#endif
#endif

	if (!endPaintBackground())
	{
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}



	// ******* Paint widgets to canvas *******


	if (!startPaintWidgets())
	{
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

#if defined (LC_PLAT_OGL)
	// Enable blending and disable depth buffer writes.
	// Note that depth buffer testing is always enabled, only writing is controlled here.
	getOglContext()->setBlend(GL_TRUE);
	m_depthMask = false;
	getOglContext()->setDepthMask(GL_FALSE);

	// Set the depth buffer function back to the engine default
	glDepthFunc(GL_LESS);

	prevDrawLayerIndex = NDHS_LOWER_INVALID_DRAW_LAYER_INDEX;
	depthSegment = numDrawLayers;
#endif

	// Call widget-level paint events
	for (itRegion = m_regions.begin();
		 itRegion != m_regions.end();
		 itRegion++)
	{

		LcTCanvasRegion* region = *itRegion;

#if defined (LC_PLAT_OGL)
		if (prevDrawLayerIndex != region->m_drawLayerIndex)
		{
			float segmentSize = 1.0f / numDrawLayers;
			float segment = depthSegment * segmentSize;
			LC_OGL_DEPTH_RANGE(LC_OGL_FROM_FLOAT(segment - segmentSize), LC_OGL_FROM_FLOAT(segment));
			prevDrawLayerIndex = region->m_drawLayerIndex;
			depthSegment--;
		}
#endif	// defined (LC_PLAT_OGL)

		// Note that in debug mode we paint non-dirty widgets too!
		if (region->m_bPaintWidget && !region->m_bBlackout)
		{
			// If optimizing paints, do the next section only if dirty
			#if !defined(LC_PAINT_FULL)
				if (!region->m_bDirty && region->m_clipRects.empty())
					continue;
			#endif

			// If widget is different to last widget painted...
			m_paintWidget = region->m_widget;

			// In full paint mode, must draw entire widget
			#if defined(LC_PAINT_FULL)
				setLightModel();
				m_paintWidget->onPaint(m_recBounds);

			// In OpenGL direct, if not painting the full screen, we are
			// stenciling rather than clipping so we only need to draw the
			// object once overall, instead of once per cliprect
			#elif defined(LC_OGL_DIRECT)
				if (region->m_bDirty || !region->m_clipRects.empty())
				{
					setLightModel();
					m_paintWidget->onPaint(m_recBounds);
				}

			#else

				// Else draw entire widget if it's flagged wholly dirty
				if (region->m_bDirty)
				{
					setLightModel();
					m_paintWidget->onPaint(m_recBounds);
				}
				else
				{
					// If we have a clip list, paint only the clipped areas
					for (int iP = 0; iP < (int)region->m_clipRects.size(); iP++)
					{
						// Skip zeroed rects
						const LcTPixelRect& clip = region->m_clipRects[iP];
						if (clip.getWidth() > 0)
						{
							setLightModel();
							m_paintWidget->onPaint(clip);
						}
					}
				}
			#endif
		}

	}

	if (!endPaintWidgets())
	{
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

#if !defined (LC_PLAT_OGL)
	// Must now reset the animation mask in all of the widgets.
	// This is used to determine what quality (high/low) to paint each widget based on how it
	// has been animated since the last frame; hence must be reset every frame.
	// NB We ignore sketchy mode in OpenGL mode
	m_focus->resetAnimationMask();
#endif



	// ******* Blit from canvas to buffer/screen *******


	if (startBlitToScreen())
	{
		// If LC_PAINT_FULL and not debugging, we simply blit entire buffer
		#if defined(LC_PAINT_FULL) && !defined(LC_PAINT_DEBUG)
			blitToBufferWithHoles(m_recBounds);

		#else
			// Following revalidateAll(), we must blit entire buffer (no debug rects)
			if (!m_regionMem)
				blitToBufferWithHoles(m_recBounds);
			else
			{
				// Build a list of blit rectangles, including both backgrounds and widgets
				buildRegionRectangleList(paintRects, regionsDownward, true);
				IFX_ShellSort(paintRects.begin(), paintRects.end(), orderRectY);

				// Merge any major overlaps even if these overlap non-dirty widgets
				mergeRectangles(paintRects, regionsDownward, false);

				// Optimize rects so that each pixel is covered a maximum of once
				makeIntersectionsIntoStripes(paintRects);

				// Blit each rectangle (in debug mode, just draw a rectangle)
				unsigned iA;
				for (iA = 0; iA < paintRects.size(); iA++)
				{
					// Skip zeroed rects
					const LcTPixelRect& rect = paintRects[iA];
					if (rect.getWidth() > 0)
					{
						#ifdef LC_PAINT_DEBUG
							paintDebugRect(rect, LcTColor(LcTColor::RED));
						#else
							blitToBufferWithHoles(rect);
						#endif
					}
				}

				// In debug mode we blit the entire screen so as to erase debug rectangles
				#ifdef LC_PAINT_DEBUG
					blitToBufferWithHoles(m_recBounds);
				#endif
			}
		#endif
	}

	if (!endBlitToScreen())
	{
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}



	// ******* Clean up *******

#if !defined(LC_PAINT_FULL)
	// Clear clip lists as we no longer need this info
	unsigned iA;
	for (iA = 0; iA < regionsDownward.size(); iA++)
	{
		LcTCanvasRegion* regionA = regionsDownward[iA];
		if (!regionA->m_clipRects.empty())
			regionA->m_clipRects.clear();
	}
#endif //!defined(LC_PAINT_FULL)

	// Replace memory block - all saved info records are in new block
	m_regionMem.attach(m_newRegionMem.release());

	// Set the paint widget to null because it is only a temporary variable
	// for use during paintCanvas.
	m_paintWidget = NULL;

	unsetLightModel(true);

}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCSpace::startPaint()
{
	bool retVal = false;

	if (IFXP_Render_Start() != IFX_SUCCESS)
		return false;

#if defined(LC_PLAT_OGL)
	// Note that depth buffer testing is always enabled, only writing is controlled here.
	m_depthMask = true;
	getOglContext()->setDepthMask(GL_TRUE);
	#if defined(LC_PAINT_FULL)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	#endif
#endif

#ifdef IFX_ENABLE_BENCHMARKING_FRAME_RATE
	emitBenchmarkSignal(IFXP_BENCHMARKING_FRAME_UPDATE_STARTED);
#endif

#if defined(_LC_PLAT_WANT_CANVAS)
	#if defined(LC_PAINT_FULL)
		// Clear screen to black (for debugging cutout regions)
		LcTPixelRect cb = getCanvasBounds();

		IFXP_RECT rect;
		rect.x1 = cb.getLeft();
		rect.y1 = cb.getTop();
		rect.x2 = cb.getRight();
		rect.y2 = cb.getBottom();

		// Clear the the canvas.
		if (m_gfxCanvas->m_canvas)
		{
			if (IFX_SUCCESS == IFXP_Canvas_Draw_Rect_Fill(m_gfxCanvas->m_canvas, &rect, 0))
			{
				retVal = true;
			}
		}
	#else
		// When there is no canvas, default to success.
		retVal = true;
	#endif
#else
	// When there is no canvas, default to success.
	retVal = true;
#endif
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCSpace::startPaintBackground()
{
	bool retVal = false;
#if defined(_LC_PLAT_WANT_CANVAS)
	// Let the IFXP know where we are in the pipeline
	if (m_gfxCanvas->m_canvas)
	{
		if (IFX_SUCCESS == IFXP_Render_Background_Start(m_gfxCanvas->m_canvas))
		{
			retVal = true;
		}
	}

#elif defined(LC_OGL_DIRECT)

	#if !defined(LC_PAINT_FULL)
		// In direct OpenGL mode with partial screen updates, we use
		// the depth buffer as a stencil to control partial repaints

		// Clear the depth buffer to the FRONT value, which effectively
		// occludes everything.  Later on we will use scissoring to clear
		// just those regions we want to permit to be painted
		m_depthMask = true;
		getOglContext()->setDepthMask(GL_TRUE);
		LC_OGL_CLEAR_DEPTH(0);
		glClear(GL_DEPTH_BUFFER_BIT);
		LC_OGL_CLEAR_DEPTH(1);
		glEnable(GL_SCISSOR_TEST);
	#endif

	// Use projection matrix only for background paints
	if (m_oglContext)
		m_oglContext->transformsChanged(NULL);

	retVal = true;

#else
	// When there is no canvas, default to success.
	retVal = true;
#endif
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCSpace::paintBackground(const LcTPixelRect& r)
{
	bool retVal = false;

#if defined(LC_OGL_DIRECT)
	// In OpenGL direct mode when not painting full, we create
	// stencil regions in the depth buffer instead of drawing the
	// background rects immediately
	#if !defined(LC_PAINT_FULL)
		getOglContext()->scissor(
			r.getLeft(),
			m_recBounds.getBottom() - r.getBottom(),
			r.getWidth(),
			r.getHeight());
		glClear(GL_DEPTH_BUFFER_BIT);
	#endif

	retVal = true;

#elif defined(_LC_PLAT_WANT_CANVAS)
	LcTPixelRect rcb = getCanvasBounds();

	IFXP_RECT rectDest;

	// Copy background to canvas using same rect for source and dest
	if (m_gfxCanvas->m_canvas)
	{

#if defined(IFX_RENDER_INTERNAL_COMPRESSED)
		// If using NDI palettes, draw direct from NDI to canvas
		// custom bitmap not supported
		if (m_backgroundImageNdi)
		{
			// Blit the background to the background canvas.
			int rectLeft = r.getLeft() - rcb.getLeft();
			int rectTop = r.getTop() - rcb.getTop();
			LcTPixelRect srcRect(
				rectLeft,
				rectTop,
				rectLeft + r.getWidth(),
				rectTop + r.getHeight());

			m_gfxCanvas->getGfx()->blitNativeRLE(
				m_backgroundImageNdi->getRleIndex(),
				m_backgroundImageNdi->getPalette(),
				m_backgroundImageNdi->getSize(),
				srcRect);

			retVal = true;
		}

#else
		// Otherwise draw from bitmap buffer to canvas
		if (m_bgCanvas->m_canvas)
		{
			rectDest.x1 = r.getLeft() - rcb.getLeft();
			rectDest.y1 = r.getTop() - rcb.getTop();
			rectDest.x2 = r.getLeft() - rcb.getLeft() + r.getWidth();
			rectDest.y2 = r.getTop() - rcb.getTop() + r.getHeight();

			if (IFX_SUCCESS == IFXP_Render_Background(
														m_gfxCanvas->m_canvas,
														rectDest.x1,
														rectDest.y1,
														m_bgCanvas->m_canvas,
														&rectDest))
			{
				retVal = true;
			}
		}
#endif // #if defined(IFX_RENDER_INTERNAL_COMPRESSED)
		else
		{
			rectDest.x1 = r.getLeft() - rcb.getLeft();
			rectDest.y1 = r.getTop() - rcb.getTop();
			rectDest.x2 = r.getLeft() - rcb.getLeft() + r.getWidth();
			rectDest.y2 = r.getTop() - rcb.getTop() + r.getHeight();

			if (IFX_SUCCESS == IFXP_Canvas_Draw_Rect_Fill(m_gfxCanvas->m_canvas, &rectDest, 0))
			{
				retVal = true;
			}
		}
	}
#else// #if defined(LC_OGL_DIRECT)  #elif defined(_LC_PLAT_WANT_CANVAS)
	retVal = true;
#endif

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCSpace::endPaintBackground()
{
	bool retVal = false;
#if defined(_LC_PLAT_WANT_CANVAS)
	// Let the IFXP know where we are in the pipeline
	if (m_gfxCanvas->m_canvas)
	{
		if (IFX_SUCCESS == IFXP_Render_Background_End(m_gfxCanvas->m_canvas))
		{
			retVal = true;
		}
	}

#elif defined(LC_OGL_DIRECT)

	// Stop background paints from changing the depth buffer
	// Note that depth buffer testing is always enabled, only writing is controlled here.
	m_depthMask = false;
	getOglContext()->setDepthMask(GL_FALSE);

	#if defined(LC_PLAT_OGL_20)
		// Lighting disable functionality
		getOglContext()->getGlobalState()->disableGlobalLighting();
	#else
		glDisable(GL_LIGHTING);
	#endif


	// Set the depth range so that the background paint does not affect any other objects.
	LC_OGL_DEPTH_RANGE(LC_OGL_FROM_FLOAT(1.0f), LC_OGL_FROM_FLOAT(1.0f));

#if !defined(LC_PAINT_FULL)
	// Draw the background once, and it will be stenciled
	// by the depth buffer as set up by paintBackground() above
	glDisable(GL_SCISSOR_TEST);
#endif

	// If we have a background texture...
	if (m_bgBitmap)
	{
		// Source rect must be in coordinates that correspond to BG size not canvas size
		LcTPixelDim bgSize = m_bgBitmap->getSize();
		float bgScale = float(bgSize.width) / m_recBounds.getWidth();
		int bottom = min(m_recBounds.getBottom(), int(bgSize.height / bgScale));

		// Scale clip coords to BG space
		LcTScalarRect srcRect(	0, 0, 0.0,
								bgScale * m_recBounds.getRight(),
								bgScale * bottom);

		// Work out scale factor from canvas to GL coords
		LcTVector globalExtent = getGlobalExtent();
		float destScale = float(globalExtent.x) / m_recBounds.getWidth();

		// Scale destination
		LcTPlaneRect rDest(destScale * (m_recBounds.getLeft() - getCanvasOrigin2D().x),
							destScale * (getCanvasOrigin2D().y - m_recBounds.getTop()),
							0,
							destScale * (m_recBounds.getRight() - getCanvasOrigin2D().x),
							destScale * (getCanvasOrigin2D().y - bottom));

		if (m_bgBitmap->isTranslucent())
			getOglContext()->setBlend(GL_TRUE);

		// Draw background bitmap...
		m_bgBitmap->drawRegion(
			srcRect,
			rDest,
			m_recBounds,
			LcTColor::WHITE,
			1,
			false,
			1,
			1);

		retVal = true;
	}
	// There is no background texture.
	else
	{
		// There is no need to clear, as we are in paint full mode.
		retVal = true;
	}

	#if defined(LC_PLAT_OGL_20)
		// Lighting enable functionality
		getOglContext()->getGlobalState()->enableGlobalLighting();
	#else
		glEnable(GL_LIGHTING);
	#endif

	retVal = true;

#else
	// When there is no canvas, default to success.
	retVal = true;
#endif

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCSpace::startPaintWidgets()
{
	bool retVal = false;
#if defined(_LC_PLAT_WANT_CANVAS)
	// Let the IFXP know where we are in the pipeline
	if (m_gfxCanvas->m_canvas)
	{
		if (IFX_SUCCESS == IFXP_Render_Foreground_Start(m_gfxCanvas->m_canvas))
		{
			retVal = true;
		}
	}
#else
	// When there is no canvas, default to success.
	retVal = true;
#endif
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCSpace::endPaintWidgets()
{
	bool retVal = false;
#if defined(_LC_PLAT_WANT_CANVAS)
	// Let the IFXP know where we are in the pipeline
	if (m_gfxCanvas->m_canvas)
	{
		if (IFX_SUCCESS == IFXP_Render_Foreground_End(m_gfxCanvas->m_canvas))
		{
			retVal = true;
		}
	}
#else
	// When there is no canvas, default to success.
	retVal = true;
#endif
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCSpace::paintDebugRect(const LcTPixelRect& r, LcTColor c)
{
	bool retVal = false;
#if defined(_LC_PLAT_WANT_CANVAS)
	// Copy background to canvas using same rect for source and dest
	if (m_gfxCanvas->m_canvas)
	{
		LcTPixelRect rcb = getCanvasBounds();
		rcb = r.intersection(rcb);


		IFXP_RECT rectDest;

		rectDest.x1 = rcb.getLeft();
		rectDest.y1 = rcb.getTop();
		rectDest.x2 = rcb.getRight() - 1;
		rectDest.y2 = rcb.getBottom() - 1;

		if (IFX_SUCCESS == IFXP_Canvas_Draw_Rect_Debug(m_gfxCanvas->m_canvas, &rectDest, c.rgb8()))
		{
			retVal = true;
		}
	}
#endif	// #if defined(_LC_PLAT_WANT_CANVAS)
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCSpace::transformsChanged()
{
#if defined(LC_PLAT_OGL)
	m_oglContext->transformsChanged(m_paintWidget);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCSpace::transformsChanged(LcTTransform& xfm)
{
#if defined(LC_PLAT_OGL)
	m_oglContext->transformsChanged(m_paintWidget, xfm);
#endif
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL void LcCSpace::setLightModel()
{
#if defined(LC_PLAT_OGL) && defined(LC_USE_LIGHTS)

	if(m_paintWidget != NULL)
	{
		if(!(m_paintWidget->isLightModelSimple())
			&& m_isLightModelSimple)
		{
			unsetLightModel(true);
			return;
		}
		else if(((m_paintWidget->isLightModelSimple())
			&& m_isLightModelSimple) || !(m_paintWidget->isLightModelSimple()))
		{
			// light model is already simple.
			return;
		}

		TmALightWidget::iterator iter;
		LcwCLight *lightWidget = NULL;
		for (iter = m_lightWidgets.begin();(iter != m_lightWidgets.end());iter++)
		{
			lightWidget = (*iter);
			if(lightWidget
				&& lightWidget->getLight() != NULL
				&& (lightWidget->getLight()->isEnabled()))
			{
				lightWidget->getLight()->temporaryDisable();
			}
		}
		setPrimaryLightPlacement(m_primaryLightDefaultPlacement, LcTPlacement::EAll, false);
		updatePrimaryLight();
		m_isLightModelSimple = true;
	}

#endif
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL void LcCSpace::unsetLightModel(bool force)
{
#if defined(LC_PLAT_OGL) && defined(LC_USE_LIGHTS)
	if(!m_isLightModelSimple)
		return;

	TmALightWidget::iterator iter;
	LcwCLight *lightWidget = NULL;
	for (iter = m_lightWidgets.begin(); (iter != m_lightWidgets.end()); iter++)
	{
		lightWidget = (*iter);
		if(lightWidget
			&& lightWidget->getLight() != NULL
			&& (lightWidget->getLight()->isTemporaryDisabled()))
		{
			lightWidget->getLight()->enableLight();
		}
	}
	m_isLightModelSimple = false;
	setPrimaryLightPlacement(m_primaryLightCurrentPlacement, LcTPlacement::EAll, false);
	updatePrimaryLight();
#endif
}

/*-------------------------------------------------------------------------*//**
	This function will return true if the blit to screen is suitably prepared.
	It returns false if this has failed or if the blit to screen was done in
	one go, without requiring numerous rectangle blits.
*/
LC_EXPORT_VIRTUAL bool LcCSpace::startBlitToScreen()
{
	bool retValue = false;

#if defined(_LC_PLAT_WANT_CANVAS)
	if (!m_gfxCanvas->m_canvas)
	{
		return false;
	}

	IFX_DISPLAY* display;
	if (IFX_SUCCESS != IFXP_Display_Get_Info(&display))
	{
		return false;
	}

	// Let the IFXP know where we are in the pipeline
	// Let CSpace know if the porting layer has just blitted
	// the whole canvas to screen.
	if (IFX_SUCCESS == IFXP_Render_Display_Start(m_gfxCanvas->m_canvas))
	{
		retValue = true;
	}
	else
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "Failed to start render cycle");
		LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
	}

	if (display && display->renderFull)
	{
		retValue = false;
	}

#else
	// When there is no canvas, we want to assume that the
	// whole frame is rendered on startblittoscreen.
	retValue = false;
#endif	// #if defined(_LC_PLAT_WANT_CANVAS)

	return retValue;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCSpace::blitToScreen(const LcTPixelRect& r)
{
	bool retVal = false;

#if defined(_LC_PLAT_WANT_CANVAS)
	LcTPixelRect rcb = getCanvasBounds();

	// Source is canvas, so adjust by subtracting recbounds
	if (!m_gfxCanvas->m_canvas)
	{
		return retVal;
	}

	IFXP_RECT rectSrc;

	rectSrc.x1 = r.getLeft() - rcb.getLeft();
	rectSrc.y1 = r.getTop() - rcb.getTop();
	rectSrc.x2 = r.getLeft() - rcb.getLeft() + r.getWidth();
	rectSrc.y2 = r.getTop() - rcb.getTop() + r.getHeight();

	// Blit canvas region to screen port - note that we disable pre-emption around this
	// call to prevent the possibility of partially drawn screens
	if (IFX_SUCCESS == IFXP_Render_Display (r.getLeft(),
											r.getTop(),
											m_gfxCanvas->m_canvas,
											&rectSrc))
	{
		retVal = true;
	}
#else
	// When there is no canvas, default to success.
	retVal = true;
#endif	// #if defined(_LC_PLAT_WANT_CANVAS)

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCSpace::endBlitToScreen()
{
	bool retVal = false;

#if defined(_LC_PLAT_WANT_CANVAS)
	if (!m_gfxCanvas->m_canvas)
	{
		return false;
	}

	// Let the IFXP know where we are in the pipeline
	if (IFX_SUCCESS == IFXP_Render_Display_End(m_gfxCanvas->m_canvas))
	{
		retVal = true;
	}

	#ifdef IFX_MEMORYTEST_DYNAMIC
		if (EIfxMemoryTestStateWaitForCanvasRepaint == g_dynamicTest_state)
		{
			g_dynamicTest_state = EIfxMemoryTestStateAfterSnapshot;

			char imagename[256];
			lc_itoa(g_dynamicTest_snapshotId, imagename, 10);
			lc_strcat(imagename, "_After");
			takeScreenshot(imagename);

			g_dynamicTest_state = EIfxMemoryTestStateNormal;
		}
	#endif /* IFX_MEMORYTEST_DYNAMIC */
#else
	// When there is no canvas, default to success.
	retVal = true;
#endif	//#if defined(_LC_PLAT_WANT_CANVAS)

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCSpace::endPaint()
{
	bool retVal = true;

#if defined (IFX_USE_SCRIPTS)
	if (NdhsCScriptExecutor::getInstance())
		NdhsCScriptExecutor::getInstance()->copyScreenBuffer();
#endif

#if defined(LC_OGL_DIRECT) && defined(LC_USE_EGL)
	// Ensure all OpenGL stuff finished
	eglSwapBuffers(m_eglDisplay, m_eglSurface);
#endif

	if (IFXP_Render_End() != IFX_SUCCESS)
		return false;

#ifdef IFX_ENABLE_BENCHMARKING_FRAME_RATE
	emitBenchmarkSignal(IFXP_BENCHMARKING_FRAME_UPDATE_COMPLETE);
#endif

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Helper - splits the rect into sub-rects such that none of them
	intersect any of the blackout widgets in m_regions, then blitsToBuffer
*/
void LcCSpace::blitToBufferWithHoles(const LcTPixelRect& rect)
{
	typedef LcTaArray<LcTPixelRect> TaRect;
	TaRect				paintRectList;
	TaRect::iterator	itRects;

	// Split apart paint rectangle so as not to draw on any blackout region
	cutHolesInRect(rect, 0, paintRectList);

	for(itRects = paintRectList.begin(); itRects != paintRectList.end(); itRects++)
		blitToBuffer(*itRects);
}

/*-------------------------------------------------------------------------*//**
	Helper - split the rect passed in into an
	array of smaller rects that don't intersect any widgets in the m_regions list
	that are marked as "blackout" widgets.  This method
	is designed to be recursive, returning an array of subrects that make up the part of the
	original rect that is not obscured by a blackout widget
*/
void LcCSpace::cutHolesInRect(
	const LcTPixelRect&		rect,
	unsigned				index,
	LcTaArray<LcTPixelRect>& outRects)
{
#ifndef IFX_USE_PLUGIN_ELEMENTS

	LC_UNUSED(index)

	// If we don't have cutouts, nothing to do
	outRects.push_back(rect);

#else

	// Find the next region that is a blackout
	for (; index < m_regions.size(); index++)
		if (m_regions[index]->m_widget->isBlackout())
			break;

	// If the hole index is out of range, there are no further holes to cut
	if (index >= m_regions.size())
	{
		// ...so our rect is to be passed uncut (terminate recursion)
		outRects.push_back(rect);
		return;
	}

	// Get hole rect
	LcTCanvasRegion* region	= m_regions[index];
	LcTPixelRect hole		= region->m_rect;

	// Other edges
	int rectR				= rect.getRight();
	int rectB				= rect.getBottom();
	int holeR				= hole.getRight();
	int holeB				= hole.getBottom();

	// Get intersection coords
	int intersectX			= max(rect.getLeft(), hole.getLeft());
	int intersectR			= min(rectR, holeR);
	int intersectY			= max(rect.getTop(), hole.getTop());
	int intersectB			= min(rectB, holeB);

	// If there is no intersection...
	if (!(intersectX <= intersectR && intersectY <= intersectB))
	{
		// ...move on with the same rect, to the next hole
		cutHolesInRect(rect, index + 1, outRects);
		return;
	}

	// NB: we prefer wide rects to tall rects for blit efficiency, so we
	// split along the horizontal edges first
	LcTPixelRect newRect = rect;

	// If we have a top part...
	if (rect.getTop() < intersectY)
	{
		newRect.setBottom(intersectY);

		// Check this part against any holes we haven't yet cut
		cutHolesInRect(newRect, index + 1, outRects);
	}

	// If we have a bottom part...
	if (rectB > intersectB)
	{
		newRect.setTop(intersectB);
		newRect.setBottom(rectB);

		// Check this part against any holes we haven't yet cut
		cutHolesInRect(newRect, index + 1, outRects);
	}

	// Center part is iT to iB; split along vertical edges
	newRect.setTop(intersectY);
	newRect.setBottom(intersectB);

	// If we have a left part...
	if (rect.getLeft() < intersectX)
	{
		newRect.setRight(intersectX);

		// Check this part against any holes we haven't yet cut
		cutHolesInRect(newRect, index + 1, outRects);
	}

	// If we have a right part...
	if (rectR > intersectR)
	{
		newRect.setLeft(intersectR);
		newRect.setRight(rectR);

		// Check this part against any holes we haven't yet cut
		cutHolesInRect(newRect, index + 1, outRects);
	}
#endif // IFX_USE_PLUGIN_ELEMENTS
}

/*-------------------------------------------------------------------------*//**
	Helper - update the primary light.
*/
void LcCSpace::updatePrimaryLight()
{
	if (m_primaryLightMask == LcTPlacement::ENone)
		return;

#if defined(LC_PLAT_OGL)
	// Update the OGL Light.
	m_oglContext->updatePrimaryLight(m_primaryLightPlacement, m_primaryLightMask);
#else

	// Update the built in light.
	if (m_primaryLightMask & LcTPlacement::EColor)
	{
		m_primaryLightColor = m_primaryLightPlacement.color;
	}

	if (m_primaryLightMask & LcTPlacement::EColor2)
	{
		m_primaryLightAmbient = m_primaryLightPlacement.color2;
	}
#endif

	m_primaryLightMask = LcTPlacement::ENone;
}

#ifdef LC_USE_LIGHTS
/*-------------------------------------------------------------------------*//**
	Helper - Check if the light pool has any available lights.
*/
LcCLight* LcCSpace::getSecondaryLight(LcwCLight* lightWidget)
{
	LcCLight* retVal = NULL;

	// If there are no lights left then stop.
	if (m_numberSecLightsAllocated < m_maxSecondaryLights)
	{
		// Check the cache for a spare light;
		TmMAllocatedLights::iterator it = m_secAllocatedLights.begin();
		while (it != m_secAllocatedLights.end())
		{
			LcOglCLight *light = (LcOglCLight *) (it->first);
			int lightIndex = light->getLightIndex();
			int targetIndex = m_numberSecLightsAllocated + 1;

			// Choose the light source that has a matching light index
			if ( (it->second == NULL) && ((lightIndex == targetIndex) || lightIndex<=m_numberSecLightsAllocated) )
			{
				it->second = lightWidget;
				retVal = it->first;

				// Keep track of the lights.
				++m_numberSecLightsAllocated;
				return retVal;
			}
			++it;
		}
	}

	// Failure
	return NULL;
}

/*-------------------------------------------------------------------------*//**
	Helper - Release a light from the pool.
*/
bool LcCSpace::releaseSecondaryLight(LcCLight* light)
{
	bool retVal = false;

	TmMAllocatedLights::iterator clearLight = m_secAllocatedLights.find(light);
	if (clearLight != m_secAllocatedLights.end())
	{
		clearLight->second = NULL;
		--m_numberSecLightsAllocated;

		retVal = true;
	}

	return retVal;
}
#endif

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
	Helper - find the widget at a specific point in the 2D rendered space
*/
LcCWidget* LcCSpace::findWidgetAt(const LcTPixelPoint& pt)
{
	// Lock widgets whilst we scan them
	LcCWidget* ret = NULL;

	// Iterate in reverse order to get items on top first
	for (int i = (int)m_regions.size() - 1; i >= 0; i--)
	{
		LcCWidget* widget = m_regions[i]->m_widget;

		// Skip non-blackout widgets if we already have a non-blackout hit
		if (!widget || (ret && !widget->isBlackout()))
			continue;

		// Find out if we're within the bounding box
		if (m_regions[i]->m_rect.contains(pt))
		{
			// Defer opacity test to widget itself
			if (widget->isVisible())
			{
				// Map the 2D point into 3D space where the widget is located
				LcTVector vPoint = mapCanvasToLocal(pt, *widget);
				if (widget->contains(vPoint, 0.0f))
				{
					ret = widget;

					// If it's a blackout, no point searching further;
					// otherwise, we continue searching but due to logic above
					// we won't ever test non-blackout widgets beyond here
					if (widget->isBlackout())
						break;
				}
			}
		}
	}

	return ret;
}
#endif

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
	Helper - Populate a list of widgets at a specific point in the 2D rendered space
*/
bool LcCSpace::findWidgetsAt(const LcTPixelPoint& pt, TmAWidget& widgetList)
{
	bool widgetFound = false;

	// Clear the widget list
	widgetList.clear();

	// Iterate in reverse order to get items on top first
	for (int i = (int)m_regions.size() - 1; i >= 0; i--)
	{
		LcCWidget* widget = m_regions[i]->m_widget;

		if (!widget)
			continue;

		// Find out if widget is not outside of clipping plane and we're within the bounding box
		if ((m_regions[i]->m_dZ > (-2 * m_globalExtent.z))
			&& (m_regions[i]->m_dZ < (.5 * m_globalExtent.z))
			&& m_regions[i]->m_rect.contains(pt))
		{
			// Defer opacity test to widget itself
			if (widget->isVisible())
			{
				// Map the 2D point into 3D space where the widget is located
				LcTVector vPoint = mapCanvasToLocal(pt, *widget);
				if (widget->contains(vPoint, 0.0f))
				{
					widgetFound = true;

					widgetList.push_back(widget);

					// If it's a blackout, no point searching further;
					if (widget->isBlackout())
						break;
				}
			}
		}
	}

	return widgetFound;
}
#endif

#ifdef LC_USE_LIGHTS
/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::addLightWidget(LcwCLight* lightWid)
{
	// Add the widget.
	m_lightWidgets.push_back(lightWid);
}

/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::removeLightWidget(LcwCLight* lightWid)
{
	TmALightWidget::iterator itWidget;

	// Remove the widget from the light widget list
	itWidget = find(m_lightWidgets.begin(), m_lightWidgets.end(), lightWid);
	if (itWidget != m_lightWidgets.end())
		m_lightWidgets.erase(itWidget);
}
#endif // #ifdef LC_USE_LIGHTS

/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::addWidget(LcCWidget* y)
{
	// Add the widget
	m_widgets.push_back(y);
}

/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::removeWidget(LcCWidget* y)
{
	TmAWidget::iterator itWidget;

	// Iterate last widget list to give background regions
	TmARegion::iterator itRegion;
	for (itRegion = m_regions.begin();
		 itRegion != m_regions.end();
		 itRegion++)
	{
		LcTCanvasRegion* region = *itRegion;

		// Reset removed widget if found; bounding box is retained for clearing
		if (region->m_widget == y)
			region->m_widget = NULL;
	}

	// Now remove the widget from the widget list
	itWidget = find(m_widgets.begin(), m_widgets.end(), y);
	if (itWidget != m_widgets.end())
		m_widgets.erase(itWidget);
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCSpace::contains(const LcTPixelPoint& pt)
{
	return m_recBounds.contains(pt);
}
#endif

/*-------------------------------------------------------------------------*//**
	This is the main NDE UI event handler, which should only be called on the
	main NDE UI thread. The current time should be passed to this function.
*/
LC_EXPORT_VIRTUAL void LcCSpace::processTimers(IFX_UINT32 timeUpper, IFX_UINT32 timeLower)
{
	LcTTime timeOut = 0;
	bool scheduleTimer = false;
	LcITimer::IEvent* event = NULL;
	LcTTime internalTime;

#if defined (IFX_GENERATE_SCRIPTS) && !defined(IFX_GENERATE_DEMOMODE_SCRIPT)
	return(processScriptGenerateTimers(timeUpper, timeLower));
#endif

#if defined (IFX_USE_SCRIPTS)
	return(processScriptExecuteTimers(timeUpper, timeLower));
#endif

	internalTime.m_time = timeLower;
	internalTime.m_wrapCount = timeUpper;

	// Force getTimestamp to return the timestamp passed to us - this stops
	// some issues with baseline timestamps being greater than 'current' timestamp
	// when initializing transitions on a frame update
	m_useFixedTime = true;
	// Store timestamp passed to us
	m_fixedTime = internalTime;

	if (!m_suspended)
	{

		// Stop any modules from messing with the timer event array until we're finished
		// with it
		m_timerMutex->lock(LcCMutex::EInfinite);

		// Identify the event that's about to be executed
		TmATimerImmediate::iterator itImmediateEvent = m_timerEventsImmediate.begin();
		TmMTimer::iterator itEvent = m_timerEvents.begin();

		// Check the immediate execution list first, then the normal event list
		if (itImmediateEvent != m_timerEventsImmediate.end())
		{
			// Something on the 'priority' event queue
			event = *itImmediateEvent;
			m_timerEventsImmediate.erase(itImmediateEvent);
		}
		else if(itEvent != m_timerEvents.end())
		{
			// 'Normal' event
			event = itEvent->second;

			// Confirm that the time is right!
			if (internalTime >= itEvent->first)
				m_timerEvents.erase(itEvent);
			else
				event = NULL;
		}

		// Release the lock on the timer array mutex, otherwise any attempt to schedule
		// another event from onExecute would cause a deadlock!
		m_timerMutex->unlock();

		// If we have an event to execute, do it...
		if (event)
		{
			event->onExecute();
		}

		// Reacquire timer mutex while we look to see if there are any
		// remaining events.
		m_timerMutex->lock(LcCMutex::EInfinite);

		// Get scheduled time of next NDE event
		itEvent = m_timerEvents.begin();
		itImmediateEvent = m_timerEventsImmediate.begin();

		// Check the immediate execution list first, then the normal event list
		if (itImmediateEvent != m_timerEventsImmediate.end())
		{
			// Something on the 'priority' event queue
			timeOut = internalTime;
			scheduleTimer = true;
		}
		else if(itEvent != m_timerEvents.end())
		{
			// If the desired time is in the past, schedule immediately
			if (internalTime >= itEvent->first)
				timeOut = internalTime;
			else
				timeOut = itEvent->first;

			scheduleTimer = true;
		}

		m_timerMutex->unlock();

	}
	else
	{
		// If there was a pending repaint, we need to reset the following flag.
		// A new repaint will be scheduled when the engine is resumed.
		if (m_bPaintScheduled)
			m_bPaintScheduled = false;
	}

	// Schedule the timer to call us on expiry.
	if (scheduleTimer)
	{
		IFX_UINT32 timeUpper = timeOut.m_wrapCount;
		IFX_UINT32 timeLower = timeOut.m_time;
		IFXP_Timer_Schedule(timeUpper, timeLower);
	}

	// return getTimestamp to normal
	m_useFixedTime = false;
}

/*-------------------------------------------------------------------------*//**
	This is the event handler function that process Timers when testing framework
	is running.
*/
#if defined (IFX_USE_SCRIPTS)
LC_EXPORT_VIRTUAL void LcCSpace::processScriptExecuteTimers(IFX_UINT32 timeUpper, IFX_UINT32 timeLower)
{
	LcTTime timeOut = 0;
	bool scheduleTimer = false;
	LcITimer::IEvent* event = NULL;
	LcTTime internalTime;
	LcTTime testTime;
	IFX_UINT32 upperTime, lowerTime;
	LcTTime currentTime;
	LcITimer::IEvent* eventS = NULL;
	LcTTime scriptEventTime;
	LcTTime mainEventTime;

	internalTime.m_time = timeLower;
	internalTime.m_wrapCount = timeUpper;

#ifdef IFX_EXECUTE_DEMOMODE_SCRIPT
	m_useFixedTime = true;
	// Store timestamp passed to us
	m_fixedTime = internalTime;
#endif

	if (!m_suspended)
	{

		// Stop any modules from messing with the timer event array until we're finished
		// with it
		m_timerMutex->lock(LcCMutex::EInfinite);

		// Identify the event that's about to be executed
		TmATimerImmediate::iterator itImmediateEvent = m_timerEventsImmediate.begin();
		TmMTimer::iterator itEvent = m_timerEvents.begin();
		TmMTimerScript::iterator scriptEvent = m_timerScriptEvents.begin();

		// Identify the corresponding real time events
		TmATimerImmediate::iterator itRealImmediateEvent = m_timerRealEventsImmediate.begin();
		TmMTimer::iterator itRealEvent = m_timerRealEvents.begin();
		TmMTimerScript::iterator scriptRealEvent = m_timerRealScriptEvents.begin();

		// Check the immediate execution list first, then the normal event list
		// and then the script events list
		if (itImmediateEvent != m_timerEventsImmediate.end())
		{
			// Something on the 'priority' event queue
			event = *itImmediateEvent;
			m_timerEventsImmediate.erase(itImmediateEvent);
			m_timerRealEventsImmediate.erase(itRealImmediateEvent);

		}
		else
		{
			// Here we get the time of testing framework
			getTestTime(&upperTime, &lowerTime);

			currentTime.m_wrapCount = upperTime;
			currentTime.m_time = lowerTime;

			//travers main events list
			if(itEvent != m_timerEvents.end())
			{
				// 'Normal' event
				event = itEvent->second;
				mainEventTime = itEvent->first;
			}

			//travers script event list
			if(scriptEvent != m_timerScriptEvents.end())
			{
				// 'Script' event
				eventS = scriptEvent->second;
				scriptEventTime = scriptEvent->first;
			}

			//both events present
			if(event && eventS)
			{
				if(mainEventTime == scriptEventTime)
				{
					eventS = NULL;
				}

				else if (mainEventTime < scriptEventTime)
				{
					eventS = NULL;
				}

				else if (mainEventTime > scriptEventTime)
				{
					event = NULL;
				}
			}

			//main event
			if(event)
			{
#if !defined (IFX_FAST_TEST_MODE)
				// Confirm that the time is right!
				if (internalTime >= itRealEvent->first)
				{
#endif
					if(itEvent->first > currentTime)
						testTime = itEvent->first;
					else
						testTime = currentTime;

					setTestTime(testTime.m_wrapCount, testTime.m_time);
					m_timerEvents.erase(itEvent);
					m_timerRealEvents.erase(itRealEvent);

#if !defined (IFX_FAST_TEST_MODE)
				}
				else
				{
					event = NULL;
				}
#endif
				Timer_Canceled = false;
			}

			//script event
			if(eventS)
			{
#if !defined (IFX_FAST_TEST_MODE)
				// Confirm that the time is right!
				if (internalTime >= scriptRealEvent->first)
				{
#endif
					if(scriptEvent->first > currentTime)
						testTime = scriptEvent->first;
					else
						testTime = currentTime;

					setTestTime(testTime.m_wrapCount, testTime.m_time);
					m_timerScriptEvents.erase(scriptEvent);
					m_timerRealScriptEvents.erase(scriptRealEvent);

#if !defined (IFX_FAST_TEST_MODE)
				}
				else
				{
					eventS = NULL;
				}
#endif
				Timer_Canceled = false;
			}

		}
		// Release the lock on the timer array mutex, otherwise any attempt to schedule
		// another event from onExecute would cause a deadlock!
		m_timerMutex->unlock();

		// If we have an event to execute, do it...
		if (event)
		{
			event->onExecute();
		}

		if(eventS)
		{
			eventS->onExecute();
		}

		// Reacquire timer mutex while we look to see if there are any
		// remaining events.
		m_timerMutex->lock(LcCMutex::EInfinite);

		// Get scheduled time of next NDE event
		itEvent = m_timerEvents.begin();
		itImmediateEvent = m_timerEventsImmediate.begin();
		scriptEvent = m_timerScriptEvents.begin();

		itRealEvent = m_timerRealEvents.begin();
		itRealImmediateEvent = m_timerRealEventsImmediate.begin();
		scriptRealEvent = m_timerRealScriptEvents.begin();

		// Check the immediate execution list first, then the normal event list
		if (itImmediateEvent != m_timerEventsImmediate.end())
		{
			// Something on the 'priority' event queue
			timeOut = internalTime;
			scheduleTimer = true;
		}
		// If both queues have events compare time and shedule earliest
		else if(itEvent != m_timerEvents.end() && scriptEvent != m_timerScriptEvents.end())
		{
			mainEventTime = itEvent->first;
			scriptEventTime = scriptEvent->first;

			if(mainEventTime <= scriptEventTime)
			{
				timeOut = itRealEvent->first;
			}
			else
			{
				timeOut = scriptRealEvent->first;
			}

			// If the desired time is in the past, schedule immediately
			if (internalTime >= timeOut)
			{
				timeOut = internalTime;
			}

			scheduleTimer = true;
		}

		else if(itEvent != m_timerEvents.end())
		{
			// If the desired time is in the past, schedule immediately
			if (internalTime > itRealEvent->first)
				timeOut = internalTime;
			else
				timeOut = itRealEvent->first;

			scheduleTimer = true;
		}

		else if(scriptEvent != m_timerScriptEvents.end())
		{
			// If the desired time is in the past, schedule immediately
			if (internalTime > scriptRealEvent->first)
				timeOut = internalTime;
			else
				timeOut = scriptRealEvent->first;

			scheduleTimer = true;
		}

		m_timerMutex->unlock();

	}
	else
	{
		// If there was a pending repaint, we need to reset the following flag.
		// A new repaint will be scheduled when the engine is resumed.
		if (m_bPaintScheduled)
			m_bPaintScheduled = false;
	}

	// Schedule the timer to call us on expiry.
	if (scheduleTimer)
	{
		IFX_UINT32 timeUpper = timeOut.m_wrapCount;
		IFX_UINT32 timeLower = timeOut.m_time;
		IFXP_Timer_Schedule(timeUpper, timeLower);
	}

#ifdef IFX_EXECUTE_DEMOMODE_SCRIPT
	// return getTimestamp to normal
	m_useFixedTime = false;
#endif
}
#endif

/*-------------------------------------------------------------------------*//**
	This is the event handler function that process Timers when testing framework
	is running.
*/
#if defined (IFX_GENERATE_SCRIPTS)
LC_EXPORT_VIRTUAL void LcCSpace::processScriptGenerateTimers(IFX_UINT32 timeUpper, IFX_UINT32 timeLower)
{
	LcTTime timeOut = 0;
	bool scheduleTimer = false;
	LcITimer::IEvent* event = NULL;
	LcTTime internalTime;
	LcTTime testTime;

	IFX_UINT32 upperTime, lowerTime;
	LcTTime currentTime;
	LcITimer::IEvent* eventS = NULL;
	LcTTime mainEventTime;
	LcTTime scriptEventTime;

	internalTime.m_time = timeLower;
	internalTime.m_wrapCount = timeUpper;

	// Get the time of testing framework
	getTestTime(&upperTime, &lowerTime);

	// Set current time and test time to the time of testing framework
	currentTime.m_wrapCount = upperTime;
	currentTime.m_time = lowerTime;
	testTime = currentTime;

#ifdef IFX_GENERATE_DEMOMODE_SCRIPT
	m_useFixedTime = true;
	// Store timestamp passed to us
	m_fixedTime = internalTime;
#endif

	if (!m_suspended)
	{

		// Stop any modules from messing with the timer event array until we're finished
		// with it
		m_timerMutex->lock(LcCMutex::EInfinite);

		// Identify the event that's about to be executed
		TmATimerImmediate::iterator itImmediateEvent = m_timerEventsImmediate.begin();
		TmMTimer::iterator itEvent = m_timerEvents.begin();
		TmMTimerScript::iterator scriptEvent = m_timerScriptEvents.begin();

		// Check the immediate execution list first, then the normal event list
		// and then the script events list
		if (itImmediateEvent != m_timerEventsImmediate.end())
		{
			// Something on the 'priority' event queue
			event = *itImmediateEvent;
			m_timerEventsImmediate.erase(itImmediateEvent);

		}
		else
		{
			//travers main events list
			if(itEvent != m_timerEvents.end())
			{
				// 'Normal' event
				event = itEvent->second;
				mainEventTime = itEvent->first;
			}

			//travers script event list
			if(scriptEvent != m_timerScriptEvents.end())
			{
				// 'Script' event
				eventS = scriptEvent->second;
				scriptEventTime = scriptEvent->first;
			}

			//both events present
			if(event && eventS)
			{
				if(mainEventTime == scriptEventTime)
				{
					eventS = NULL;
				}

				else if (mainEventTime < scriptEventTime)
				{
					eventS = NULL;
				}

				else if (mainEventTime > scriptEventTime)
				{
					event = NULL;
				}
			}

			//main event
			if(event)
			{
				// Set the time of testing framework
				if(itEvent->first > currentTime && !Timer_Canceled)
					testTime = itEvent->first;
				else
					testTime = currentTime;

				if (!Timer_Canceled)
				{
					setTestTime(testTime.m_wrapCount, testTime.m_time);
					m_timerEvents.erase(itEvent);
				}
				else
				{
					event = NULL;
				}

				Timer_Canceled = false;
			}

			//script event
			if(eventS)
			{
				if(scriptEvent->first > currentTime)
					testTime = scriptEvent->first;
				else
					testTime = currentTime;

				setTestTime(testTime.m_wrapCount, testTime.m_time);
				m_timerScriptEvents.erase(scriptEvent);

			}
		}

		// Release the lock on the timer array mutex, otherwise any attempt to schedule
		// another event from onExecute would cause a deadlock!
		m_timerMutex->unlock();

		// If we have an event to execute, do it...
		if (event)
		{
			event->onExecute();
		}
		// Execute script event
		if(eventS)
		{
			eventS->onExecute();
		}

		// Reacquire timer mutex while we look to see if there are any
		// remaining events.
		m_timerMutex->lock(LcCMutex::EInfinite);

		// Get scheduled time of next NDE event
		itEvent = m_timerEvents.begin();
		itImmediateEvent = m_timerEventsImmediate.begin();
		scriptEvent = m_timerScriptEvents.begin();

		// Check the immediate execution list first, then the normal event list
		if (itImmediateEvent != m_timerEventsImmediate.end())
		{
			// Something on the 'priority' event queue
			timeOut = testTime;
			scheduleTimer = true;
		}
		// If both queues have events compare time and shedule earliest
		else if(itEvent != m_timerEvents.end() && scriptEvent != m_timerScriptEvents.end())
		{
			mainEventTime = itEvent->first;
			scriptEventTime = scriptEvent->first;

			if(mainEventTime <= scriptEventTime)
			{
				timeOut = mainEventTime;
			}
			else
			{
				timeOut = scriptEventTime;
			}

			// If the desired time is in the past, schedule immediately
			if (testTime >= timeOut)
			{
				timeOut = testTime;
			}

			scheduleTimer = true;
		}

		else if(itEvent != m_timerEvents.end())
		{
			// If the desired time is in the past, schedule immediately
			if (testTime > itEvent->first)
				timeOut = testTime;
			else
				timeOut = itEvent->first;

			scheduleTimer = true;
		}

		else if(scriptEvent != m_timerScriptEvents.end())
		{
			// If the desired time is in the past, schedule immediately
			if (testTime > scriptEvent->first)
				timeOut = testTime;
			else
				timeOut = scriptEvent->first;

			scheduleTimer = true;
		}

		m_timerMutex->unlock();

	}
	else
	{
		// If there was a pending repaint, we need to reset the following flag.
		// A new repaint will be scheduled when the engine is resumed.
		if (m_bPaintScheduled)
			m_bPaintScheduled = false;
	}

	// Schedule the timer to call us on expiry.
	if (scheduleTimer)
	{
		IFX_UINT32 timeUpper = timeOut.m_wrapCount;
		IFX_UINT32 timeLower = timeOut.m_time;
		IFXP_Timer_Schedule(timeUpper, timeLower);
	}

#ifdef IFX_GENERATE_DEMOMODE_SCRIPT
	// return getTimestamp to normal
	m_useFixedTime = false;
#endif
}
#endif

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT bool LcCSpace::isBitmapFile(const LcTmString& file)
{
	LcTaString ext = file.getWord(-1, '.');

	if(ext.compareNoCase("ndi")==0)
	{
		return true;
	}
	else
	{
		LcTaOwner<LcCCustomBitmapFile> bmf = LcCCustomBitmapFile::create();
		if(bmf->checkExtension(file))
		{
			return true;
		}
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCBitmap* LcCSpace::getBitmap(const LcTmString& file,
												int marginLeft,
												int marginRight,
												int marginTop,
												int marginBottom,
												int frameCount)
{

	LcTaString cacheName = file.toLower();

	// First see if the Bitmap is in the cache
	TmMBitmapMap::iterator it = m_bitmapMap.find(cacheName);
	if (it != m_bitmapMap.end())
	{
		return (*it).second;
	}
	else
	{
		LcTaOwner<LcCBitmap> pBitmap;
		LcTaString ext = file.getWord(-1, '.');

		if(ext.compareNoCase("ndi")==0)
		{
			pBitmap=loadBitmap(file);
		}
		else
		{
			pBitmap=loadCustomBitmap(file,
									marginLeft,
									marginRight,
									marginTop,
									marginBottom,
									frameCount);
		}
		// Bitmap not cached, so load it

		if (pBitmap)
		{
			m_bitmapMap.add_element(cacheName, pBitmap);
			return m_bitmapMap[cacheName];
		}
	}


	// Failure
	return NULL;
}

/*-------------------------------------------------------------------------*//**
	Default Implementation of loadBitmap.
*/
LC_EXPORT_VIRTUAL LcTaOwner<LcCBitmap> LcCSpace::loadBitmap(const LcTmString& file)
{
#if defined(LC_OMIT_NDI_OPEN)
	LcTaOwner<LcCBitmap> retError;
	return retError;
#else
	#if defined(LC_PLAT_OGL)
		LcTaOwner<LcOglCNdiBitmap> im = LcOglCNdiBitmap::create(this);
	#else
		LcTaOwner<LcCNdiBitmap> im = LcCNdiBitmap::create(this, m_gfxCanvas->getGfx());
	#endif

	#if defined(LC_PLAT_OGL_20)
		CTextureInfo *textureInfo = m_oglContext->getTextureInfo();

		if (textureInfo)
		{
			im->setTextureUnit (textureInfo->getUnit());
			im->setTextureMipmap (textureInfo->getMipmap());
		}
		else
		{
			im->setTextureUnit (GL_TEXTURE0);
			im->setTextureMipmap (false);
		}
	#endif

	// If open fails, destroy() NULLs before returning value
	if (!im->open(file))
		im.destroy();

	return im;
#endif
}

/*-------------------------------------------------------------------------*//**
	Implementation of loadCustomBitmap.
*/
LC_EXPORT_VIRTUAL LcTaOwner<LcCBitmap> LcCSpace::loadCustomBitmap(const LcTmString& file,
																		int marginLeft,
																		int marginRight,
																		int marginTop,
																		int marginBottom,
																		int frameCount)
{
	#if defined(LC_PLAT_OGL)
		LcTaOwner<LcOglCCustomBitmap> im = LcOglCCustomBitmap::create(this);
	#else
		LcTaOwner<LcCCustomBitmap> im = LcCCustomBitmap::create(this, m_gfxCanvas->getGfx());
	#endif

	#if defined(LC_PLAT_OGL_20)
		CTextureInfo *textureInfo = m_oglContext->getTextureInfo();

		if (textureInfo)
		{
			im->setTextureUnit (textureInfo->getUnit());
			im->setTextureMipmap (textureInfo->getMipmap());
		}
		else
		{
			im->setTextureUnit (GL_TEXTURE0);
			im->setTextureMipmap (false);
		}
	#endif

		// If open fails, destroy() NULLs before returning value
	if (!im->open(file,
					marginLeft,
					marginRight,
					marginTop,
					marginBottom,
					frameCount))
		im.destroy();

	return im;
}

/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::unloadBitmap(LcCBitmap* obj)
{
	// Delete the Bitmap from the cache
	TmMBitmapMap::iterator it = m_bitmapMap.begin();

	for (; it != m_bitmapMap.end(); it++)
	{
		if (it->second == obj)
		{
			m_bitmapMap.erase(it);
			break;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool LcCSpace::imagePresent(LcCBitmap* obj)
{
	// Check the if Bitmap present in our cache or not
	TmMBitmapMap::iterator it = m_bitmapMap.begin();
	for (; it != m_bitmapMap.end(); it++)
	{
		if (it->second == obj)
		{
			return true;
		}
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCFont* LcCSpace::getFont(const LcTmString& fontName, LcCFont::EStyle style)
{
	LcTaString fileLower = fontName.toLower();

	// Add a character corresponding to the enum value to make
	// the map below different depending on style.
	char st = (char)style + 'A';
	fileLower += st;

	TmMFontMap::iterator it = m_fontMap.find(fileLower);
	if (it != m_fontMap.end())
	{
		return (*it).second;
	}
	else
	{
		LcTaOwner<LcCFont> pFont = loadFont(fontName, style);
		if (pFont)
		{
			m_fontMap.add_element(fileLower, pFont);
			return m_fontMap[fileLower];
		}
	}

	// Failure
	return NULL;
}

/*-------------------------------------------------------------------------*//**
	Default Implementation of loadFont.
*/
LC_EXPORT_VIRTUAL LcTaOwner<LcCFont> LcCSpace::loadFont(	const LcTmString& fontName,
															LcCFont::EStyle style)
{
#if defined(LC_OMIT_NDI_OPEN)
	LcTaOwner<LcCFont> retError;
	return retError;
#else
	// For stripping asterisk
	LcTaString rawFontName = fontName;

	// If both font types are allowed, '*' is used to identify native font names
	if (fontName.length()<3 || fontName.subString(fontName.length()-3,3).compareNoCase("ndi")!=0)
	{
		// Strip the asterisk
		rawFontName = fontName;

#if defined(IFX_USE_NATIVE_FONTS)
		// Create native font and open
		#if defined (LC_PLAT_OGL)
			LcTaOwner<LcOglCNativeFont> pFont = LcOglCNativeFont::create(this);
		#else
			LcTaOwner<LcCNdiNativeFont> pFont = LcCNdiNativeFont::create(this,  m_gfxCanvas->getGfx());
		#endif

		if (!pFont->open(rawFontName, style))
			pFont.destroy();

		// Null'ed on error
		return pFont;
#else
		LC_UNUSED(style)
#endif
	}
#if defined(IFX_USE_BITMAPPED_FONTS)
	else
	{
		// Create bitmapped font and open
	#if defined(LC_PLAT_OGL)
		LcTaOwner<LcOglCNdiFont> pFont = LcOglCNdiFont::create(this);
	#else
		LcTaOwner<LcCNdiFont> pFont = LcCNdiFont::create(this, m_gfxCanvas->getGfx());
	#endif
		if (!pFont->open(fontName))
			pFont.destroy();

		// Null'ed on error
		return pFont;
	}
#endif //IFX_USE_BITMAPPED_FONTS

#if !((defined(IFX_USE_NATIVE_FONTS) && !defined(NDHS_TOOLKIT)) && defined(IFX_USE_BITMAPPED_FONTS))
	LcTaOwner<LcCNdiFont> retError;
	return retError;
#endif	// #if !(defined(IFX_USE_NATIVE_FONTS) && defined(IFX_USE_BITMAPPED_FONTS))
#endif	// #if defined(LC_OMIT_NDI_OPEN)
}

/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::unloadFont(LcCFont* obj)
{
	// Delete the font from the cache
	TmMFontMap::iterator it = m_fontMap.begin();
	for (; it != m_fontMap.end(); it++)
	{
		if (it->second == obj)
		{
			m_fontMap.erase(it);
			break;
		}
	}
}

#ifdef LC_USE_MESHES
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCMesh* LcCSpace::getMesh(const LcTmString& file)
{
	LcTaString fileLower = file.toLower();

	// First see if the Mesh is in the cache
	TmMMeshMap::iterator it = m_meshMap.find(fileLower);
	if (it != m_meshMap.end())
	{
		return (*it).second;
	}
	else
	{
		// Mesh not cached, so load it
		LcTaOwner<LcCMesh> pMesh = loadMesh(file);
		if (pMesh)
		{
			pMesh->useRefCount(true);
			m_meshMap.add_element(fileLower, pMesh);
			return m_meshMap[fileLower];
		}
	}

	// Failure
	return NULL;
}
#endif

#ifdef LC_USE_MESHES
/*-------------------------------------------------------------------------*//**
	Default Implementation of loadMesh.
*/
LC_EXPORT_VIRTUAL LcTaOwner<LcCMesh> LcCSpace::loadMesh(const LcTmString& file)
{
#if defined(LC_OMIT_NDI_OPEN)
	LcTaOwner<LcCMesh> retError;
	return retError;
#else
	// Create mesh and load from file
	LcTaOwner<LcCMesh> mesh = LcOglCMesh::create(this);
	if (!mesh->loadDataND3(file))
		mesh.destroy();

	// Null'ed on error
	return mesh;
#endif	// #if defined(LC_OMIT_NDI_OPEN)
}
#endif

#ifdef LC_USE_MESHES
/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::unloadMesh(LcCMesh* obj)
{
	// Delete the mesh from the cache
	TmMMeshMap::iterator it = m_meshMap.begin();
	for (; it != m_meshMap.end(); it++)
	{
		if (it->second == obj)
		{
			m_meshMap.erase(it);
			break;
		}
	}
}
#endif

#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
/*-------------------------------------------------------------------------*//**
	Send the benchmarking event to the porting layer
*/
LC_EXPORT_VIRTUAL void LcCSpace::emitBenchmarkSignal(unsigned int signal, int data)
{
	IFXP_Benchmarking_Signal(signal, data);
}
#endif

#if defined(IFX_WIN_PLAYER)
/*-------------------------------------------------------------------------*//**
	Send the Inflexion player specific signals to porting layer
*/
LC_EXPORT_VIRTUAL void LcCSpace::emitIfxPlayerSignal(unsigned int signal)
{
	IFXP_IfxPlayer_Signal(signal);
}

#endif

/*-------------------------------------------------------------------------*//**
	Return the current system time in ms, unless we're in the middle of
	an 'processTimers' call, in which case it always returns the time
	passed into that.
*/
LC_EXPORT_VIRTUAL LcTTime LcCSpace::getTimestamp()
{
	LcTTime localTime;

	if(m_useFixedTime)
	{
		localTime = m_fixedTime;
	}
	else
	{
		IFX_UINT32 currentTimeUpper;
		IFX_UINT32 currentTimeLower;
		IFXP_Timer_Get_Current_Time(&currentTimeUpper, &currentTimeLower);

#if defined(IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
		// In script generation and execution real time clock is replaced
		// by the testing framework clock.
#if !defined(IFX_EXECUTE_DEMOMODE_SCRIPT) && !defined(IFX_GENERATE_DEMOMODE_SCRIPT)
		getTestTime(&currentTimeUpper, &currentTimeLower);
#endif
#endif

		localTime.m_time = currentTimeLower;
		localTime.m_wrapCount = currentTimeUpper;
	}

	return localTime;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCSpace::schedule(LcITimer::IEvent* e, int iTime)
{
	// Get scheduled time, in ms
	LcTTime schedTime = getTimestamp() + iTime;
#if defined (IFX_USE_SCRIPTS)
	LcTTime realSchedTime = getRealClockTime() + iTime;
#endif

	bool retVal = true;
	bool forceSchedule = false;

	// Lock access to the timer array while we update it
	m_timerMutex->lock(LcCMutex::EInfinite);

	// This code IS required as we do get duplicate time values back!
	// It is here to ensure that we don't schedule multiple events at
	// the same time, since LcTmMap can't cope with duplicate keys
	TmMTimer::iterator it = m_timerEvents.begin();

#if defined (IFX_USE_SCRIPTS)
	TmMTimer::iterator itReal = m_timerRealEvents.begin();
#endif

	for (; it != m_timerEvents.end(); it++)
	{
		// No need to restart check as higher time values will come later
		if (it->first == schedTime)
		{
			schedTime += 1;
		}

		// Check that the existing entry is not after the requested time.
		// If it is, then force a timer re-schedule.
		if (it->first > schedTime)
			forceSchedule = true;

#if defined (IFX_USE_SCRIPTS)
		if(itReal->first == realSchedTime)
		{
			realSchedTime += 1;
		}
		itReal++;
#endif
	}

	// Add new entry to event queue (will be ordered by time)
	m_timerEvents.insert(TmMTimer::value_type(schedTime, e));

#if defined (IFX_USE_SCRIPTS)
	m_timerRealEvents.insert(TmMTimer::value_type(realSchedTime, e));
#endif

#if defined(IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
	TmMTimerScript::iterator itScript = m_timerScriptEvents.begin();
	it = m_timerEvents.begin();

	if(itScript != m_timerScriptEvents.end())
	{
		if(itScript->first > schedTime)
		{
			if(it->first >= schedTime)
				forceSchedule = true;
			else
				forceSchedule = false;
		}
		else
		{
			forceSchedule = false;
		}
	}
#endif

	// Only schedule the timer if there is only one entry in
	// the events array, and the immediate event array is
	// empty, or we need to force a schedule.
	// In all other cases, there'll be a timeout
	// pending.
	if (forceSchedule || ((m_timerEvents.size() == 1) && (m_timerEventsImmediate.size() == 0)))
	{
		IFX_UINT32 timeUpper = schedTime.m_wrapCount;
		IFX_UINT32 timeLower = schedTime.m_time;

#if defined (IFX_USE_SCRIPTS)
		timeUpper = realSchedTime.m_wrapCount;
		timeLower = realSchedTime.m_time;
#endif

		if (IFX_SUCCESS != IFXP_Timer_Schedule(timeUpper, timeLower))
			retVal = false;
	}

	m_timerMutex->unlock();

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Add the event to the immediate execute vector.
*/
LC_EXPORT_VIRTUAL bool LcCSpace::scheduleImmediate(LcITimer::IEvent* e)
{
	// Lock access to the timer array while we update it
	m_timerMutex->lock(LcCMutex::EInfinite);

	// add it to the array - FIFO queue
	m_timerEventsImmediate.push_back(e);

#if defined (IFX_USE_SCRIPTS)
	m_timerRealEventsImmediate.push_back(e);
#endif

	m_timerMutex->unlock();

	// Schedule the timer to call us on expiry.
	LcTTime schedTime = getTimestamp();
	IFX_UINT32 timeUpper = schedTime.m_wrapCount;
	IFX_UINT32 timeLower = schedTime.m_time;

#if defined (IFX_USE_SCRIPTS)
	schedTime = getRealClockTime();
	timeUpper = schedTime.m_wrapCount;
	timeLower = schedTime.m_time;
#endif

	if (IFX_SUCCESS == IFXP_Timer_Schedule(timeUpper, timeLower))
		return true;

	return false;
}

#if defined(IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
	Add the event to the script execute vector. Schedule timer only if normal
	and immediate queues are empty.
*/
LC_EXPORT_VIRTUAL bool LcCSpace::scheduleScriptTimer(LcITimer::IEvent* e, int iTime)
{
	// Get scheduled time, in ms
	LcTTime schedTime = getTimestamp() + iTime;
#if defined (IFX_USE_SCRIPTS)
	LcTTime realSchedTime = getRealClockTime() + iTime;
#endif
	bool retVal = true;
	bool forceSchedule = false;

	// Lock access to the timer array while we update it
	m_timerMutex->lock(LcCMutex::EInfinite);

	// This code IS required as we do get duplicate time values back!
	// It is here to ensure that we don't schedule multiple events at
	// the same time, since LcTmMap can't cope with duplicate keys
	TmMTimerScript::iterator it = m_timerScriptEvents.begin();

#if defined (IFX_USE_SCRIPTS)
	TmMTimer::iterator itReal = m_timerRealScriptEvents.begin();
#endif

	for (; it != m_timerScriptEvents.end(); it++)
	{
		// No need to restart check as higher time values will come later
		if (it->first == schedTime)
		{
			schedTime += 1;
		}

#if defined (IFX_USE_SCRIPTS)
		if(itReal->first == realSchedTime)
		{
			realSchedTime += 1;
		}
		itReal++;
#endif

	}

	// Add new entry to event queue (will be ordered by time)
	m_timerScriptEvents.insert(TmMTimerScript::value_type(schedTime, e));
#if defined (IFX_USE_SCRIPTS)
	m_timerRealScriptEvents.insert(TmMTimerScript::value_type(realSchedTime, e));
#endif

	TmMTimer::iterator itNormal = m_timerEvents.begin();
	it = m_timerScriptEvents.begin();

	if(itNormal != m_timerEvents.end())
	{
		if(itNormal->first > schedTime)
		{
			if(it->first >= schedTime)
				forceSchedule = true;
			else
				forceSchedule = false;
		}
	}
	else
	{
		if(it->first >= schedTime)
			forceSchedule = true;
	}

	// Only schedule the timer if there is only one entry in
	// the events array, and the normal and immediate event arrays are
	// empty, or we need to force a schedule.
	// In all other cases, there'll be a timeout
	// pending.
	if (forceSchedule || ((m_timerScriptEvents.size() == 1) && (m_timerEvents.size() == 0) && (m_timerEventsImmediate.size() == 0)))
	{
		IFX_UINT32 timeUpper = schedTime.m_wrapCount;
		IFX_UINT32 timeLower = schedTime.m_time;

#if defined (IFX_USE_SCRIPTS)
		timeUpper = realSchedTime.m_wrapCount;
		timeLower = realSchedTime.m_time;
#endif

		if (IFX_SUCCESS != IFXP_Timer_Schedule(timeUpper, timeLower))
			retVal = false;
	}

	m_timerMutex->unlock();

	return retVal;

}
#endif

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCSpace::cancel(LcITimer::IEvent* e)
{
	// Lock access to the timer array while we update it
	m_timerMutex->lock(LcCMutex::EInfinite);

	// Scan immediate execution queue
	TmATimerImmediate::iterator it2 = m_timerEventsImmediate.begin();
#if defined (IFX_USE_SCRIPTS)
	TmATimerImmediate::iterator it2Real = m_timerRealEventsImmediate.begin();
#endif
	for(; it2 != m_timerEventsImmediate.end(); it2++)
	{
		if(*it2 == e)
		{
			// Remove event
			m_timerEventsImmediate.erase(it2);
#if defined (IFX_USE_SCRIPTS)
			m_timerRealEventsImmediate.erase(it2Real);
#endif
			break;
		}

#if defined (IFX_USE_SCRIPTS)
		it2Real++;
#endif
	}

	// Scan event queue
	TmMTimer::iterator it = m_timerEvents.begin();
#if defined (IFX_USE_SCRIPTS)
	TmMTimer::iterator itReal = m_timerRealEvents.begin();
#endif

	for (; it != m_timerEvents.end(); it++)
	{
		// If event is found, remove it from queue
		if (it->second == e)
		{
			m_timerEvents.erase(it);

#if defined (IFX_USE_SCRIPTS)
			m_timerRealEvents.erase(itReal);
#endif

#if defined(IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
			Timer_Canceled = true;
#endif
			break;
		}
#if defined(IFX_USE_SCRIPTS)
		itReal++;
#endif

	}

	m_timerMutex->unlock();
}

#if defined(IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
	Cancel script events from script queue.
*/
LC_EXPORT_VIRTUAL void LcCSpace::cancelScript(LcITimer::IEvent* e)
{
	// Lock access to the timer array while we update it
	m_timerMutex->lock(LcCMutex::EInfinite);

	// Scan script event queue
	TmMTimerScript::iterator it = m_timerScriptEvents.begin();
#if defined (IFX_USE_SCRIPTS)
	TmMTimerScript::iterator itReal = m_timerRealScriptEvents.begin();
#endif

	for (; it != m_timerScriptEvents.end(); it++)
	{
		// If event is found, remove it from queue
		if (it->second == e)
		{
			m_timerScriptEvents.erase(it);
#if defined (IFX_USE_SCRIPTS)
			m_timerRealScriptEvents.erase(itReal);
#endif
			break;
		}
#if defined (IFX_USE_SCRIPTS)
		itReal++;
#endif
	}

	m_timerMutex->unlock();
}
#endif

/*-------------------------------------------------------------------------*//**
*/
bool LcCSpace::onSuspend()
{
	m_suspended = true;

	// Any scheduled paint event would be lost
	m_bPaintScheduled = false;
	m_timerEvents.clear();
	m_timerEventsImmediate.clear();

#if defined(IFX_EXECUTE_DEMOMODE_SCRIPT)
	if (NdhsCScriptExecutor::getInstance())
		NdhsCScriptExecutor::getInstance()->deactivateEvents();

	m_timerScriptEvents.clear();
	m_timerRealScriptEvents.clear();

#endif

	m_primaryLightMask = LcTPlacement::EAll;

#if defined(_LC_PLAT_WANT_CANVAS)
	// Free the foreground canvas
	m_gfxCanvas->destroyCanvas();

	// Free the background canvas
	if (m_bgCanvas)
		m_bgCanvas->destroyCanvas();
#endif	// defined(_LC_PLAT_WANT_CANVAS)


#if	defined(LC_PLAT_OGL)
	if(m_oglContext)
		m_oglContext->cleanup();
#endif

	TmMBitmapMap::iterator it = m_bitmapMap.begin();
	for(;it!=m_bitmapMap.end();it++)
	{
		it->second->releaseResources();
	}

	TmMFontMap::iterator it2= m_fontMap.begin();
	for(;it2!=m_fontMap.end();it2++)
	{
		it2->second->releaseResources();
	}

#if defined(LC_PLAT_OGL) && defined(LC_USE_EGL)
	// Tear down EGL
	destroyEgl();
#endif

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool LcCSpace::onResume()
{
	// Make sure the foreground and background canvasses
	// are re-drawn.
	boundsUpdated();

	m_suspended = false;

#if defined(IFX_EXECUTE_DEMOMODE_SCRIPT)
	if (NdhsCScriptExecutor::getInstance())
		NdhsCScriptExecutor::getInstance()->activateEvents();
#endif


	TmMBitmapMap::iterator it = m_bitmapMap.begin();
	for(;it!=m_bitmapMap.end();it++)
	{
		it->second->reloadResources();
	}

	TmMFontMap::iterator it2= m_fontMap.begin();
	for(;it2!=m_fontMap.end();it2++)
	{
		it2->second->reloadResources();
	}

	if(!m_bgFile.isEmpty())
		return setBackgroundImage(m_bgFile+"");
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool LcCSpace::isCanvasConstructed()
{
	bool val = true;

#if defined(LC_USE_EGL)
	if(m_eglContext
#else
	#if defined(_LC_PLAT_WANT_CANVAS)
		if (m_gfxCanvas
		#if !defined(IFX_RENDER_INTERNAL_COMPRESSED)
			  && m_bgCanvas
		#endif
	#else
		if(true
	#endif
#endif
	)
	{
		val = true;
	}
	else
	{
		val = false;

	}


	return val;
}
#if defined (IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
	Set time of the testing framework.
*/
void LcCSpace::setTestTime(IFX_UINT32 timeUpper, IFX_UINT32 timeLower)
{
#if defined (IFX_USE_SCRIPTS)
	if (NdhsCScriptExecutor::getInstance())
		NdhsCScriptExecutor::getInstance()->setTestTime(timeUpper, timeLower);

#endif

#if defined (IFX_GENERATE_SCRIPTS)
	if (NdhsCScriptGenerator::getInstance())
		NdhsCScriptGenerator::getInstance()->setTestTime(timeUpper, timeLower);
#endif
}

/*-------------------------------------------------------------------------*//**
	Get time of the testing framework.
*/
bool LcCSpace::getTestTime(IFX_UINT32* timeUpper, IFX_UINT32* timeLower)
{

	if(!timeUpper || !timeLower)
		return false;

#if defined (IFX_USE_SCRIPTS)
	if (NdhsCScriptExecutor::getInstance())
		NdhsCScriptExecutor::getInstance()->getTestTime(timeUpper, timeLower);
#endif

#if defined (IFX_GENERATE_SCRIPTS)
	if (NdhsCScriptGenerator::getInstance())
		NdhsCScriptGenerator::getInstance()->getTestTime(timeUpper, timeLower);
#endif

	return true;
}

/*-------------------------------------------------------------------------*//**
	Get real clock time.
*/
LcTTime LcCSpace::getRealClockTime()
{
	IFX_UINT32 currentTimeUpper;
	IFX_UINT32 currentTimeLower;
	LcTTime localTime;

	if(m_useFixedTime)
	{
		localTime = m_fixedTime;
	}
	else
	{
		IFXP_Timer_Get_Current_Time(&currentTimeUpper, &currentTimeLower);

		localTime.m_time = currentTimeLower;
		localTime.m_wrapCount = currentTimeUpper;
	}

	return localTime;
}

/*-------------------------------------------------------------------------*//**
	 Event queue empty status.
*/
bool LcCSpace::eventQueueEmpty()
{
	bool retVal = false;

	TmMTimer::iterator itEvent = m_timerEvents.begin();

	if(itEvent == m_timerEvents.end())
	{
		retVal = true;
	}

	return retVal;
}
#endif

#if defined (IFX_GENERATE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
	Update time of the testing framework.
*/
void LcCSpace::updateTestTime(IFX_UINT32 timeUpper, IFX_UINT32 timeLower)
{
	if (NdhsCScriptGenerator::getInstance())
		NdhsCScriptGenerator::getInstance()->updateTestTime(timeUpper, timeLower);

}
/*-------------------------------------------------------------------------*//**
	Increment time of the testing framework.
*/
void LcCSpace::incrementTestTime(IFX_UINT32 timeUpper, IFX_UINT32 timeLower)
{
	if (NdhsCScriptGenerator::getInstance())
		NdhsCScriptGenerator::getInstance()->incrementTestTime(timeUpper, timeLower);

}

/*-------------------------------------------------------------------------*//**
	Get time of event at the head of event queue.
*/
bool LcCSpace::getMainEventsQueueHeadTime(LcTTime* queueHeadTime)
{
	bool retVal = false;
	LcTTime eventQueueTime = 0;
	TmMTimer::iterator itEvent = m_timerEvents.begin();

	if(itEvent != m_timerEvents.end())
	{
		eventQueueTime = itEvent->first;
		queueHeadTime->m_time = eventQueueTime.m_time;
		queueHeadTime->m_wrapCount = eventQueueTime.m_wrapCount;

		retVal = true;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Get time of event at the head of script event queue.
*/
bool LcCSpace::getScriptEventsQueueHeadTime(LcTTime* queueHeadTime)
{
	bool retVal = false;
	LcTTime eventQueueTime = 0;
	TmMTimerScript::iterator scriptEvent = m_timerScriptEvents.begin();

	if(scriptEvent != m_timerScriptEvents.end())
	{
		eventQueueTime = scriptEvent->first;
		queueHeadTime->m_time = eventQueueTime.m_time;
		queueHeadTime->m_wrapCount = eventQueueTime.m_wrapCount;

		retVal = true;
	}

	return retVal;
}
#endif

#ifdef IFX_EXECUTE_DEMOMODE_SCRIPT
/*-------------------------------------------------------------------------*//**
	Clear Timers.
*/
void LcCSpace::clearTimers()
{
	// Clear timers and schedule paint event
	m_bPaintScheduled = false;
	m_timerEvents.clear();
	m_timerEventsImmediate.clear();
	m_timerRealEvents.clear();
}

/*-------------------------------------------------------------------------*//**
	Suspend script events when external events detected.
*/
void LcCSpace::suspendScriptEvents()
{
	if (NdhsCScriptExecutor::getInstance())
		NdhsCScriptExecutor::getInstance()->suspendEvents();
}

/*-------------------------------------------------------------------------*//**
	Status of script events.
*/
bool LcCSpace::scriptEventsActive()
{
	if (NdhsCScriptExecutor::getInstance())
		return (NdhsCScriptExecutor::getInstance()->scriptEventsActive());

	return false;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void LcCSpace::takeScreenshot(const char* imgName)
{
#ifdef IFX_MEMORYTEST_DYNAMIC
	IFX_DISPLAY *display;

	if (IFX_SUCCESS != IFXP_Display_Get_Info(&display))
		return;

	if (display)
	{
		/* Create bitmap file data */

		// Basic width in bytes is width * 3 (one byte each for R, G, B)
		// Then we round up to nearest 4-byte boundary by adding (4-1), then dividing
		// and multiplying by 4, relying on integer floor-style rounding in the divide
		int stride = (((display->width * 3) + 3) / 4) * 4;

		int sizeOfImageData = stride * display->height;
		int sizeOfFile = 14 /*sizeof(BITMAPFILEHEADER)*/ + 40 /*sizeof(BITMAPINFO)*/ + sizeOfImageData;

		LcTaAlloc<LcTByte> startBitmapData(sizeOfFile);

		if (startBitmapData)
		{
			memset(startBitmapData, 0xaa, sizeOfFile);

			LcTByte* bitmapData = startBitmapData;

			/* Write out header info */

			// BITMAPFILEHEADER, bfType
			SETB16_LE_INC(bitmapData, 0x4D42);

			// BITMAPFILEHEADER, bfSize
			SETB32_LE_INC(bitmapData, sizeOfFile);

			// BITMAPFILEHEADER, bfReserved1
			SETB16_LE_INC(bitmapData, 0);

			// BITMAPFILEHEADER, bfReserved2
			SETB16_LE_INC(bitmapData, 0);

			// BITMAPFILEHEADER, bfOffBits
			SETB32_LE_INC(bitmapData, 0x36);

			// BITMAPINFO, biSize
			SETB32_LE_INC(bitmapData, 0x28);

			// BITMAPINFO, biWidth
			SETB32_LE_INC(bitmapData, display->width);

			// BITMAPINFO, biHeight
			SETB32_LE_INC(bitmapData, -display->height); // NB -ve, otherwise data has to be written upside down

			// BITMAPINFO, biPlanes
			SETB16_LE_INC(bitmapData, 1);

			// BITMAPINFO, biBitCount
			SETB16_LE_INC(bitmapData, 24);

			// BITMAPINFO, biCompression
			SETB32_LE_INC(bitmapData, 0); // BI_RGB

			// BITMAPINFO, biSizeImage
			SETB32_LE_INC(bitmapData, sizeOfImageData);

			// BITMAPINFO, biXPelsPerMeter
			SETB32_LE_INC(bitmapData, 4000);

			// BITMAPINFO, biYPelsPerMeter
			SETB32_LE_INC(bitmapData, 4000);

			// BITMAPINFO, biClrUsed
			SETB32_LE_INC(bitmapData, 0);

			// BITMAPINFO, biClrImportant
			SETB32_LE_INC(bitmapData, 0);

			/* Write out image data */
			LcTByte* dstImageDataStart = bitmapData;
			LcTByte* srcImageDataStart = (LcTByte*)display->frameBuffer;

			for (unsigned int row = 0; row < display->height; row++)
			{
				// Find the beginning of each row
				LcTByte* dstImageData = dstImageDataStart + row * stride;
				LcTByte* srcImageData = srcImageDataStart + row * display->stride;

				for (unsigned int col = 0; col < display->width; col++)
				{
					LcTByte red = 0;
					LcTByte green = 0;
					LcTByte blue = 0;

					#ifdef IFX_CANVAS_MODE_8888
						red 	= *srcImageData++;
						green	= *srcImageData++;
						blue	= *srcImageData++;
						srcImageData++;
					#endif

					#ifdef IFX_CANVAS_MODE_888
						red 	= *srcImageData++;
						green	= *srcImageData++;
						blue	= *srcImageData++;
					#endif

					#ifdef IFX_CANVAS_MODE_565
						register unsigned samp = (unsigned)(*(LcTUInt16*)srcImageData);
						red = (samp >> 8) & 0x0F8;
						green = (samp >> 3) & 0x0FC;
						blue = (samp << 3) & 0x0F8;

						srcImageData += sizeof(LcTUInt16);
					#endif

					#if defined (IFX_CANVAS_MODE_1555) || defined(IFX_CANVAS_MODE_444)
						// Unsupported mode
						continue;
					#endif

					*dstImageData++ = blue;
					*dstImageData++ = green;
					*dstImageData++ = red;
				}
			}
		}

		/* Write out the file */
#ifdef NU_SIMULATION
		{
			char filename[1024];

			lc_strcpy(filename, IFX_MEMORYTEST_SCREENSHOT_PATH);
			lc_strcat(filename, imgName);
			lc_strcat(filename, ".bmp");

			FILE* file = fopen(filename, "wb");
			if (file)
			{
				fwrite( startBitmapData, 1, sizeOfFile, file );
				fclose(file);
			}
		}
#endif // def NU_SIMULATION
	}
#endif  /* IFX_MEMORYTEST_DYNAMIC */
}
