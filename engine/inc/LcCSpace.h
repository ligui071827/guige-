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
#ifndef LcCSpaceH
#define LcCSpaceH

#include "inflexionui/engine/inc/LcCApplet.h"
#include "inflexionui/engine/inc/LcITimer.h"
#include "inflexionui/engine/inc/LcTMessage.h"
#include "inflexionui/engine/inc/LcCWidget.h"
#include "inflexionui/engine/inc/LcTPixelRect.h"
#include "inflexionui/engine/inc/LcTVector.h"
#include "inflexionui/engine/inc/LcCFont.h"
#include "inflexionui/engine/inc/LcTTransform4.h"
#include "inflexionui/engine/inc/LcTPlaneRect.h"
#include "inflexionui/engine/inc/LcCAnimator.h"
#include "inflexionui/engine/inc/LcCMutex.h"

#if !defined(LC_OGL_DIRECT)
	#include "inflexionui/engine/inc/LcCNdiGraphics.h"
#endif

#if defined(LC_PLAT_OGL)
	// OpenGL
	extern "C"
	{
		#include "inflexionui/engine/ifxui_porting.h"
	}

	// Use 'clamp to edge' instead of 'clamp' EXCEPT for OGL1 UIX previewer case
	#if defined(LC_PLAT_OGL_20) || !defined(NDHS_JNI_INTERFACE)
		#ifdef GL_CLAMP
			#undef GL_CLAMP
		#endif

		// In case the OGL header doesn't define GL_CLAMP_TO_EDGE...
		#ifndef GL_CLAMP_TO_EDGE
			#define GL_CLAMP_TO_EDGE   0x812F
		#endif

		#define GL_CLAMP GL_CLAMP_TO_EDGE
	#endif //defined(LC_PLAT_OGL_20) || !defined(NDHS_JNI_INTERFACE)
#endif


// Debug logging, if disabled - The enabled version is in platform specific space.
#ifndef LC_DEBUG_LOGGING
	#define LC_DEBUG_LOG(m)
#endif



class LcITimer;
class LcCBitmap;
class LcCSkin;
class LcTCanvasRegion;
class LcTCapture;
class LcCMutex;

#ifdef LC_USE_MESHES
	class LcCMesh;
#endif

#ifdef LC_USE_LIGHTS
	class LcCLight;
	class LcwCLight;
#endif

#if defined(IFX_USE_PLUGIN_ELEMENTS) && !defined(LC_PLAT_OGL)
	class LcCNdiGraphics;
#endif

#if defined(LC_PLAT_OGL)
	class LcOglCBitmap;
#endif

// The canvas is required for all modes except blitting to the screen.
// NOTE: This is also not used by the Toolkits which have their own
// canvas.
#if !defined(LC_PLAT_OGL)
	#define _LC_PLAT_WANT_CANVAS
#endif

/*-------------------------------------------------------------------------*//**
	Note that this class has several different compilation modes
	(LC_OGL_DIRECT and not direct).  These might
	logically be separated into different classes, but practically
	are best kept in one class to make it easier to build the same
	project for multiple modes (e.g. for devices with/without OpenGL
	hardware acceleration)
*/

class LcIBitmapLoader
{
public:
	virtual LcCBitmap*	getBitmap(const LcTmString &file) = 0;
};

class LcCSpace : public LcCBase, public LcITimer, private LcTMessage::IHandler
{
public:

	// Cursor values
	enum ECursor
	{
		CURSOR_DEFAULT,
		CURSOR_HAND
	};

	// Key codes
	enum EKeyCode
	{
		// start at HIGHWORD so not to conflict with any device scancodes
		KEY_HASH = 0x0000FFFF,
		KEY_ASTERISK,
		KEY_UP,
		KEY_DOWN,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_SELECT,
		KEY_BACK,
		KEY_PLATFORM_START,
		KEY_APPLICATION_START
	};

protected:
	// Cache map defines.
	typedef LcTmOwnerMap<LcTmString, LcCBitmap>	TmMBitmapMap;
	typedef LcTmOwnerMap<LcTmString, LcCFont>	TmMFontMap;

#ifdef LC_USE_MESHES
	typedef LcTmOwnerMap<LcTmString, LcCMesh>	TmMMeshMap;
#endif

private:

#if defined(_LC_PLAT_WANT_CANVAS)
	// This class allows separate surfaces to be used for
	class CCanvas : public LcCBase, public LcCNdiGraphics::ISurface
	{
		friend class LcCSpace;

	private:

		// The space this is owned by.
		LcCSpace*							m_space;

		// The clipping rect
		LcTPixelRect						m_canvasRect;

		// NDE graphics object
		LcTmOwner<LcCNdiGraphics>			m_gfx;

		// Two-phase construction
		LC_IMPORT							CCanvas();
		LC_IMPORT			void			construct(LcCSpace* space);

	protected:

		// The canvas
		IFX_CANVAS*							m_canvas;

		// Canvas creation, destruction routines.
		LC_IMPORT			bool			createCanvas(LcTPixelRect& r, bool isBackground);
		LC_IMPORT			void			destroyCanvas();

		// Graphics accessor
		LC_IMPORT			LcCNdiGraphics*	getGfx()		{ return m_gfx.ptr(); }

	public:

		// Construction/destruction
		LC_IMPORT static LcTaOwner<CCanvas> create(LcCSpace* space);
		LC_VIRTUAL							~CCanvas();

		// LcCNdiGraphics::ISurface methods
		LC_VIRTUAL			int				getLineSpan();
		LC_VIRTUAL			LcTPixelRect	getBounds();
		LC_VIRTUAL			LcTByte*		getDataStart();

	};
#endif // _LC_PLAT_WANT_CANVAS

protected:

	// Timer event queue
	typedef LcTmMap<LcTTime, LcITimer::IEvent*> TmMTimer;
	TmMTimer						m_timerEvents;
	typedef LcTmArray<LcITimer::IEvent*> TmATimerImmediate;
	TmATimerImmediate				m_timerEventsImmediate;
	LcTmOwner<LcCMutex>				m_timerMutex;

#if defined (IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
	typedef LcTmMap<LcTTime, LcITimer::IEvent*> TmMTimerScript;
	TmMTimerScript					m_timerScriptEvents;

	TmMTimerScript					m_timerRealScriptEvents;
	TmMTimer						m_timerRealEvents;
	TmATimerImmediate				m_timerRealEventsImmediate;
#endif

private:

	typedef LcTmArray<LcCApplet*>				TmAApplet;
	typedef LcTmArray<LcCAnimator*>				TmAAnimator;
	typedef LcTmArray<LcTCanvasRegion*>			TmARegion;
	typedef LcTmArray<LcCWidget*>				TmAWidget;
#if defined(LC_USE_LIGHTS)
	typedef LcTmArray<LcwCLight*>				TmALightWidget;
	typedef LcTmOwnerArray<LcCLight>			TmALightPool;
	typedef LcTmMap<LcCLight*, LcwCLight*>		TmMAllocatedLights;
#endif

	// Parameters for mapping 3D x,y at z=0 onto 2D
	LcTPixelRect					m_recBounds;
	LcTPixelPoint					m_ptOrigin2D;
	LcTVector						m_globalExtent;

	// Screen orientation.
	bool							m_portraitMode;

#if defined (IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
	bool							Timer_Canceled;
#endif

	// Widgets contained within space
	TmAApplet						m_applets;

	// Widgets rendered within space (records contained in block m_regionMem)
	TmARegion						m_regions;
	TmAWidget						m_widgets;
	LcTmAlloc<LcTCanvasRegion>		m_regionMem;
	LcTmAlloc<LcTCanvasRegion>		m_newRegionMem;

	// Primary light settings.
	LcTPlacement					m_primaryLightCurrentPlacement;
	LcTPlacement					m_primaryLightPlacement;
	LcTPlacement					m_primaryLightDefaultPlacement;
	int								m_primaryLightMask;
#if !defined(LC_PLAT_OGL)
	LcTColor						m_primaryLightColor;
	LcTColor						m_primaryLightAmbient;
#endif

#if !defined(LC_USE_NO_ROTATION)
	// Primary light orientation cache
	LcTVector						m_primaryLightVector;
	LcTQuaternion					m_lastPrimaryLightOrientation;
#endif

	// Secondary lights
#if defined(LC_USE_LIGHTS)
	TmALightWidget					m_lightWidgets;
	int								m_maxSecondaryLights;
	int								m_numberSecLightsAllocated;
	TmALightPool					m_secOwnedLightPool;
	TmMAllocatedLights				m_secAllocatedLights;
#endif

	// Animators currently active
	TmAAnimator						m_animators;

	// Other parameters
	int								m_iFrameInterval;
	int								m_iRepeatDelay;

	// Used to fix timestamp during 'processTimers' call
	bool 							m_useFixedTime;
	LcTTime 						m_fixedTime;

#if defined(LC_USE_LIGHTS)
	// Light are switched to simple mode?
	bool							m_isLightModelSimple;
#endif

	// Internal state
	bool							m_bDirty;
	bool							m_depthMask;
	bool							m_bRepainted;
	bool							m_bPaintScheduled;
	LcCWidget*						m_paintWidget;
	LcCWidget*						m_focus;
	LcTPixelRect					m_builtBox;
	LcTScalarQuad					m_builtQuad;
#if defined(IFX_RENDER_INTERNAL) || defined (IFX_RENDER_DIRECT_OPENGL)\
								 || defined (IFX_RENDER_DIRECT_OPENGL_20)\
								|| defined (IFX_RENDER_BUFFERED_OPENGL)

#if !defined (LC_PAINT_FULL)
	LcTmArray<int>					m_regionOverlapArraySize;
	LcTmArray<int*>					m_regionsOverlap;
#endif

#endif
	LcTTransform4					m_xfmCanvas;
#ifdef IFX_USE_PLUGIN_ELEMENTS
	bool							m_bFullScreenMode;
#endif

#if defined(_LC_PLAT_WANT_CANVAS)
	// Graphics and background canvas.
	LcTmOwner<CCanvas>				m_gfxCanvas;
	LcTmOwner<CCanvas>				m_bgCanvas;

	// This is used as a temporary variable during the Non-palette mode
	// and as a permanent image when we are using palettes.
	LcTmOwner<LcCNdiBitmapFile>		m_backgroundImageNdi;

	LcTmOwner<LcCCustomBitmapFile>	m_backgroundImageCustom;
#endif

	// Store the background image file name to be used during
	// suspend and resume.
	LcTmString						m_bgFile;
	bool							m_suspended;

#if defined(LC_PLAT_OGL)

	// The Ogl Context.
	LcTmOwner<LcOglCContext>		m_oglContext;

#if defined(LC_USE_EGL)

	// Assume standard OpenGL ES EGL
	EGLDisplay						m_eglDisplay;
	LcTmAlloc<EGLConfig>			m_eglConfig;
	EGLContext						m_eglContext;
	EGLSurface						m_eglSurface;
#endif	// #if defined(LC_USE_EGL)

#if defined(LC_OGL_DIRECT)
	// Background for drawing direct to screen
	LcOglCBitmap*					m_bgBitmap;
#endif	// #if defined(LC_OGL_DIRECT)
#endif	// #if defined(LC_PLAT_OGL)

	// Message objects for frequent but once-only event types
	LcTMessage						m_msgFrame;
	LcTMessage						m_msgRepeat;

	// Default Text properties
	LcTmString						m_defaultFontName;
	LcCFont::EStyle					m_defaultFontStyle;
	LcTColor						m_defaultFontColor;
	LcTColor						m_defaultFontColorDisabled;

	// Resource management
	LcTmString						m_sBasePath;
	LcTmString						m_sImagePath;
	LcTmString						m_sXMLPath;

protected:
	// Cached images and fonts for efficiency
	TmMBitmapMap					m_bitmapMap;
	TmMFontMap						m_fontMap;

#ifdef LC_USE_MESHES
	TmMMeshMap						m_meshMap;
#endif

#if defined(NDHS_JNI_INTERFACE)
	bool							m_forceRetainResource;
#endif

private:
	// Mutex used to serialize input to the engine.
	LcTmOwner<LcCMutex>				m_mutex;

	// Stylus-specific helpers
#ifdef LC_USE_STYLUS
public:

					LcCWidget*		findWidgetAt(const LcTPixelPoint& pt);
					bool			findWidgetsAt(const LcTPixelPoint& pt, TmAWidget& widgetList);

private:
#endif

	// Private helpers
					void			calcProjection();
					void			processAnimators();
					void			paintCanvas();
					void			updateBounds();
					void 			buildRegionRectangleList(
										LcTmArray<LcTPixelRect>&		paintRects,
										LcTmArray<LcTCanvasRegion*>&	regionsDownward,
										bool							bIncludeWidget);
					void 			mergeRectangles(
										LcTmArray<LcTPixelRect>&		paintRects,
										LcTmArray<LcTCanvasRegion*>&	regionsDownward,
										bool							bAvoidDirtying);
					void			makeIntersectionsIntoStripes(
										LcTmArray<LcTPixelRect>&		rects);
					void			blitToBufferWithHoles(
										const LcTPixelRect&				rect);
					void			cutHolesInRect(
										const LcTPixelRect&				rect,
										unsigned						startAt,
										LcTaArray<LcTPixelRect>&		rectArray);

					void			updatePrimaryLight();

#if defined(LC_USE_LIGHTS)
					// Check if a light resource is available.
					LcCLight*		getSecondaryLight(LcwCLight* lightWidget);
#endif

	// LcTMessage::IHandler interface
	LC_VIRTUAL		void			onMessage(int iID, int iParam);

	// Resource loaders
	LC_VIRTUAL LcTaOwner<LcCBitmap>	loadBitmap(const LcTmString& file);

	LC_VIRTUAL LcTaOwner<LcCBitmap> loadCustomBitmap(const LcTmString& file,
														int marginLeft=0,
														int marginRight=0,
														int marginTop=0,
														int marginBottom=0,
														int frameCount=0);

	LC_VIRTUAL LcTaOwner<LcCFont>	loadFont(const LcTmString& fontName, LcCFont::EStyle style);

#ifdef LC_USE_MESHES
	LC_VIRTUAL LcTaOwner<LcCMesh>	loadMesh(const LcTmString& file);
#endif

#if defined(LC_PLAT_OGL) && defined(LC_USE_EGL)
					void 			destroyEgl();
#endif

LC_PRIVATE_INTERNAL_PUBLIC:

	// Helpers internal to NDE
					void			addWidget(LcCWidget* w);
					void			removeWidget(LcCWidget* w);
	inline			LcCWidget*		getPaintWidget()						{ return m_paintWidget; }
	inline			bool			getDepthMask()							{ return m_depthMask; }
	virtual			void			unloadBitmap(LcCBitmap* obj);
					bool			imagePresent(LcCBitmap* obj);
					void			unloadFont(LcCFont* obj);
#ifdef LC_USE_LIGHTS
					void			addLightWidget(LcwCLight* lightWid);
					void			removeLightWidget(LcwCLight* lightWid);

	// Release the light from the pool.
	LC_IMPORT		bool			releaseSecondaryLight(LcCLight* light);
#endif

	// Primary light settings API
	inline		const LcTPlacement&	getPrimaryLightPlacement()				{ return m_primaryLightPlacement; }
	LC_IMPORT		void			setPrimaryLightPlacement(const LcTPlacement& p, int mask, bool updateCurrent = true);

#if !defined(LC_PLAT_OGL)
	inline			LcTColor		getPrimaryLightColor()					{ return m_primaryLightColor; }
	inline			LcTColor		getAmbientLightColor()					{ return m_primaryLightAmbient; }

#if !defined(LC_USE_NO_ROTATION)
					LcTScalar		getPaintLighting();
#endif // !LC_USE_NO_ROTATION
					LcTColor 		getLitObjectColor(LcTColor baseTint);
#endif // !defined(LC_PLAT_OGL)

					bool			isEyeFacing();

#ifdef LC_USE_MESHES
	virtual			void			unloadMesh(LcCMesh* m);
#endif

	// Hooks for derived class
	virtual			LcTaString		mapTypeIDToClassName(const LcTmString& s)	{ return s; }
	virtual			void			processTimers(IFX_UINT32 timeUpper, IFX_UINT32 timeLower);

	// For realizing applets
					void			addApplet(LcCApplet* a);
					void			removeApplet(LcCApplet* a);

	// Repaint the background.
	inline			void			repaintBackground() { revalidateAll(); }

#ifdef IFX_USE_PLUGIN_ELEMENTS
	inline			void			setFullScreenMode(bool bFullScreen)	{ m_bFullScreenMode = bFullScreen;}
#endif

protected:

	// Hooks for derived class - note that all rectangles passed in here are
	// relative to the origin of the target control, i.e. the same origin
	// to which the specified canvas bounds are relative
	LC_VIRTUAL		bool			startPaint();
	LC_VIRTUAL		bool			startPaintBackground();
	LC_VIRTUAL		bool			paintBackground(const LcTPixelRect& r);
	LC_VIRTUAL		bool			endPaintBackground();
	LC_VIRTUAL 		bool 			paintDebugRect(const LcTPixelRect& r, LcTColor c);
	LC_VIRTUAL		bool			startPaintWidgets();
	LC_VIRTUAL		bool			endPaintWidgets();
	LC_VIRTUAL		void			blitToBuffer(const LcTPixelRect& r)		{ blitToScreen(r); }
	LC_VIRTUAL		bool			startBlitToScreen();
	LC_VIRTUAL		bool			blitToScreen(const LcTPixelRect& r);
	LC_VIRTUAL		bool			endBlitToScreen();
	LC_VIRTUAL		bool			endPaint();
	LC_VIRTUAL		void 			createCanvases();
	LC_VIRTUAL		void			boundsUpdated();

	virtual			void			onDeactivate()							{}

	// Methods for derived class access only
	LC_IMPORT		bool			getRepainted();

	// Abstract so keep constructor protected
	LC_IMPORT						LcCSpace();
	LC_IMPORT 		void			construct();

	// Iterators for the cache objects.
	inline	TmMBitmapMap::iterator	getBitmapMapStart()	{ return m_bitmapMap.begin(); }
	inline	TmMBitmapMap::iterator	getBitmapMapEnd()	{ return m_bitmapMap.end(); }
	inline	TmMFontMap::iterator	getFontMapStart()	{ return m_fontMap.begin(); }
	inline	TmMFontMap::iterator	getFontMapEnd()		{ return m_fontMap.end(); }

	LC_IMPORT 		void 			setUseFixedTime(bool useFixedTime, LcTTime fixedTime)
														{m_useFixedTime = useFixedTime; m_fixedTime = fixedTime; }

LC_PROTECTED_INTERNAL_PUBLIC:

	virtual			void			onActivate();
	virtual			bool			onSuspend();
	virtual			bool			onResume();

public:

	// Construction / Destruction
	LC_IMPORT static LcTaOwner<LcCSpace>	create();
	LC_VIRTUAL							 	~LcCSpace();

	LC_EXPORT		void 			constructCanvas();
	// Mapping to screen
	LC_IMPORT		void			configureCanvas(
										const LcTPixelRect& rec,
										const LcTPixelPoint& ptOrigin2D,
										int zDepth,
										LcTScalar globalExtentWidth);
	inline			LcTPixelRect	getCanvasBounds()				{ return m_recBounds; }
	inline			LcTPixelPoint	getCanvasOrigin2D()				{ return m_ptOrigin2D; }
	inline			LcTVector		getGlobalExtent()				{ return m_globalExtent; }
	LC_IMPORT		bool			contains(const LcTPixelPoint& pt);

#if defined(LC_PLAT_OGL)
	// OglContext accessor
	inline			LcOglCContext*	getOglContext()					{ return m_oglContext.ptr(); }
#endif // defined(LC_PLAT_OGL)

#if defined(LC_PLAT_OGL) && defined(LC_USE_EGL)
					EGLDisplay      getEglDisplay()                 { return m_eglDisplay; }
					EGLSurface      getEglSurface()                 { return m_eglSurface; }
					EGLContext      getEglContext()                 { return m_eglContext; }
#endif //defined(LC_PLAT_OGL) && defined(LC_USE_EGL)

	// Accessor/mutator for the portrait mode.
	inline			bool			isPortraitMode()				{ return m_portraitMode; }
	inline			void			setPortraitMode(bool portrait)	{ m_portraitMode = portrait; }

	// Configuration
	inline			void			setFrameInterval(int i)			{ m_iFrameInterval = i; }
	inline			int				getFrameInterval()				{ return m_iFrameInterval; }
	inline			void			setRepeatDelay(int i)			{ m_iRepeatDelay = i; }
	LC_VIRTUAL		bool			setBackgroundImage(const LcTmString& sFile);
#if defined(LC_USE_LIGHTS)
	// Set the light value to the lowest value specified.
	inline			void			setMaxSecondaryLights(int maxLights) { if (maxLights < m_maxSecondaryLights) m_maxSecondaryLights = maxLights; }
#endif

	// OK to call revalidate from another thread, but not repaint
	LC_IMPORT		void			revalidate(bool dirtyCanvas = true);
	LC_VIRTUAL		void			repaint(bool finalFrame);

	// For widgets to register points in bounding box
	LC_IMPORT		void			extendBoundingBox(const LcTVector& v);
	LC_IMPORT		void			extendBoundingBox(const LcTPlaneRect& r);

#ifdef LC_USE_MESHES
	// Derives a maximal bounding box that includes this mesh
	LC_IMPORT		void			extendBoundingBox(const LcTVector& pos, LcCMesh* mesh);

	// For the given mesh, centered on the given vector, calculates the bounding box
	LC_IMPORT       void            calcBoundingBox(const LcTVector& pos,
													LcCMesh* mesh,
													int &xLeft,
													int &yTop,
													int &xRight,
													int &yBottom,
													LcTScalar &z,
													LcTScalarQuad& sq);
#endif

	// Parallels widget methods but not through same interface
	LC_IMPORT		LcTVector		mapGlobalToCanvasSubpixel(const LcTVector& vPos);
	LC_IMPORT		LcTVector		mapLocalToCanvasSubpixel(const LcTVector& vPos);
	LC_IMPORT		LcTScalarQuad	mapLocalToCanvasSubpixel(const LcTPlaneRect& rect);

	// Parallels widget methods but not through same interface
	LC_IMPORT		LcTPixelPoint	mapGlobalToCanvas(const LcTVector& vPos);
	LC_IMPORT		LcTPixelPoint	mapLocalToCanvas(LcCWidget* pWidget, const LcTVector& vPos);

#ifdef LC_USE_STYLUS
	// Mapping back from canvas only used by stylus code, for now
	LC_IMPORT		LcTVector		mapCanvasToGlobal(const LcTPixelPoint& pt, LcTScalar targetZ);
	LC_IMPORT		LcTVector		mapCanvasToLocal(const LcTPixelPoint& pt, LcCWidget& widget);
	LC_EXPORT 		bool 			mapCanvasToLocal(const LcTPixelPoint& pt, LcCWidget* widget,
													LcTVector location, LcTVector& intersection);
#endif

	// Drawing classes, such as images, use this as a hint
	LC_IMPORT		bool			canSketch();

	// Keyboard focus
	inline			void			setFocus(LcCWidget* w)			{ m_focus = w; }
	inline			LcCWidget*		getFocus()						{ return m_focus; }

	// API methods to be implemented according to platform
	virtual			void			lock()							{ m_mutex->lock(); }
	virtual			void			unlock()						{ m_mutex->unlock(); }
	virtual			void			setCursor(ECursor c)			{ LC_UNUSED(c) }
	virtual			LcITimer*		getTimer() 						{ return this; }
	virtual			LcTTime			getTimestamp();

	// LcITimer methods
	LC_VIRTUAL		bool			start()							{ return true; }
	LC_VIRTUAL		bool			stop()							{ return true; }
	LC_VIRTUAL		bool			schedule(LcITimer::IEvent* e, int iTime);
	LC_VIRTUAL		bool			scheduleImmediate(LcITimer::IEvent* e);
	LC_VIRTUAL		void			cancel(LcITimer::IEvent* e);

#if defined (IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
					void			setTestTime(IFX_UINT32 timeUpper, IFX_UINT32 timeLower);
					bool			getTestTime(IFX_UINT32* timeUpper, IFX_UINT32* timeLower);
	LC_VIRTUAL		bool			scheduleScriptTimer(LcITimer::IEvent* e, int iTime);
					LcTTime			getRealClockTime();
	LC_VIRTUAL		void			cancelScript(LcITimer::IEvent* e);
					bool			eventQueueEmpty();
#endif

#if defined (IFX_GENERATE_SCRIPTS)
					void			updateTestTime(IFX_UINT32 timeUpper, IFX_UINT32 timeLower);
					void			incrementTestTime(IFX_UINT32 timeUpper, IFX_UINT32 timeLower);
					bool			getMainEventsQueueHeadTime(LcTTime* queueHeadTime);
					bool			getScriptEventsQueueHeadTime(LcTTime* queueHeadTime);
	virtual			void			processScriptGenerateTimers(IFX_UINT32 timeUpper, IFX_UINT32 timeLower);
#endif

#if defined (IFX_USE_SCRIPTS)
	virtual			void			processScriptExecuteTimers(IFX_UINT32 timeUpper, IFX_UINT32 timeLower);
#if !defined(LC_PLAT_OGL)
					IFX_CANVAS*		getCanvas()		{ return m_gfxCanvas->m_canvas; }
#endif

#ifdef IFX_EXECUTE_DEMOMODE_SCRIPT
					void			clearTimers();
					void			suspendScriptEvents();
					bool			scriptEventsActive();
#endif

#endif

	// Resources
	LC_IMPORT		LcCBitmap*		getBitmap(const LcTmString& file,
													int marginLeft=0,
													int marginRight=0,
													int marginTop=0,
													int marginBottom=0,
													int frameCount=1);
	// Resources
	LC_IMPORT		bool			isBitmapFile(const LcTmString& file);

	LC_IMPORT		LcCFont*		getFont(const LcTmString& fontName, LcCFont::EStyle style);

	// Meshes are optional
#ifdef LC_USE_MESHES
	LC_IMPORT		LcCMesh*		getMesh(const LcTmString& file);
#endif

	// For activating/deactivating animators
	LC_IMPORT		void			addAnimator(LcCAnimator* a);
	LC_IMPORT		void			removeAnimator(LcCAnimator* a);

	LC_IMPORT		void			revalidateAll(bool excludeBackground = false);

	LC_VIRTUAL		IFX_DISPLAY*	getDisplay();

	LC_VIRTUAL		void			transformsChanged();
	LC_VIRTUAL		void			transformsChanged(LcTTransform& xfm);
	LC_VIRTUAL		void			setLightModel();
	LC_VIRTUAL		void			unsetLightModel(bool force);

#if defined(IFX_USE_PLUGIN_ELEMENTS) && !defined(LC_OGL_DIRECT)
	LC_VIRTUAL		IFX_DISPLAY*	getDirectDrawPort()								{ return getDisplay(); }
	LC_VIRTUAL		void			releaseDirectDrawPort(IFX_DISPLAY* pDisplay)	{}

	#if !defined(LC_PLAT_OGL)
		// Expose platform-specific gfx context to caller
		LC_VIRTUAL		LcCNdiGraphics*	getCanvasGfx()		{ return m_gfxCanvas->getGfx(); }
	#endif
#endif

	// Mutex support
	LC_VIRTUAL		LcTaOwner<LcCMutex>	createMutex(const LcTmString& name)		{ return LcCMutex::create(name); };

#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
	// Benchmark signal emitter - overload this on platforms that should support the
	// benchmarking application
	LC_VIRTUAL 		void 			emitBenchmarkSignal(unsigned int signal, int data = 0);
#endif

#if defined(IFX_WIN_PLAYER)
	LC_VIRTUAL 		void 			emitIfxPlayerSignal(unsigned int signal);
#endif

	LC_VIRTUAL		void			takeScreenshot(const char* imgName);

	int								getWidgetCount()						{ return (int)(m_widgets.size()); }
	bool							isCanvasConstructed();

#if defined(NDHS_JNI_INTERFACE)
	inline void						retainResource(bool retainResource) 	{ m_forceRetainResource = retainResource; }
#endif

	// Text property accessors
	inline		void				setDefaultFontName(const LcTmString& name)	{ m_defaultFontName = name; }
	inline		LcTaString 			getDefaultFontName() 						{ return m_defaultFontName; }
	inline		void				setDefaultFontStyle(LcCFont::EStyle style)	{ m_defaultFontStyle = style; }
	inline		LcCFont::EStyle		getDefaultFontStyle()						{ return m_defaultFontStyle; }
	inline		void				setDefaultFontColor(LcTColor color)			{ m_defaultFontColor = color; }
	inline		LcTColor			getDefaultFontColor()						{ return m_defaultFontColor; }
	inline		void				setDefaultFontColorDisabled(LcTColor color)	{ m_defaultFontColorDisabled = color; }
	inline		LcTColor			getDefaultFontColorDisabled()				{ return m_defaultFontColorDisabled; }

	inline		void				setBasePath(const LcTmString& path) 		{ m_sBasePath = path; }
	inline		LcTaString			getImagePath()			    				{ return m_sBasePath + m_sImagePath; }
	inline		LcTaString			getXMLPath()								{ return m_sBasePath + m_sXMLPath; }
};

#endif // LcCSpaceH

