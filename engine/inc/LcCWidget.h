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
#ifndef LcCWidgetH
#define LcCWidgetH

#include "inflexionui/engine/inc/LcTTransform.h"
#include "inflexionui/engine/inc/LcStl.h"
#include "inflexionui/engine/inc/LcTPixelDim.h"
#include "inflexionui/engine/inc/LcCFont.h"
#include "inflexionui/engine/inc/LcTAlloc.h"
#include "inflexionui/engine/inc/LcTPlacement.h"
#include "inflexionui/engine/inc/LcCAnimator.h"

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	class LcOglCSLType;
#endif

class LcCAggregate;
class LcCSpace;
class LcTPixelPoint;
class LcTWidgetEvent;
class LcCApplet;

#ifdef LC_USE_XML
	class LcCXmlElem;
#endif

/*-------------------------------------------------------------------------*//**
*/
class LcCWidget : public LcCBase, public LcCAnimator::IObserver
{
	LC_DECLARE_RTTI_BASE(LcCWidget)

	// Derived aggregate class is tightly integrated with widget
	friend class LcCAggregate;

public:

	// Reserve bits 0 to 15 for base widget class.
	// bits 16 to 31 can be used by widget sub-class
	// bits 0 to 8 and FFFF - All are defined in the placement class.
	enum EXmlMask
	{
		EFontName		= 0x100,
		EFontStyle		= 0x200,
		EFontColor		= 0x400,
		EAnimateOnShow	= 0x800,
		EAnimateOnHide	= 0x1000,
		EAll			= 0xFFFF
	}; 

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	enum EEffectTranslucencyCache
	{
		EEffectTranslucencyCacheUndefined	= 0x00,
		EEffectTranslucencyCacheOpaque		= 0x01,
		EEffectTranslucencyCacheTranslucent = 0x02		
	};
#endif

	// Sketchy mode defines the three states of images. They are:
	// Sketchy is disable (high quality mode).
	// Sketchy is allowed (normal quality mode).
	// Force sketchy (low quality mode).
	enum ESketchyMode
	{
		ESketchyDisabled,
		ESketchyAllowed,
		ESketchyForced
	};


	// Events that may be fired by aggregates to their child widgets
	class TAggregateEvent
	{
	private:
		LcCAggregate*	m_container;
		int				m_code;

	public:

		TAggregateEvent(int c, LcCAggregate* container)
			{ m_container = container; m_code = c; }

		// Access
		LcCAggregate*	getAggregate()	{ return m_container; }
		int				getCode()		{ return m_code; }
	};

private:

	enum ETweenState
	{
		//IS_STATIC,
		IS_SHOWN,
		IS_SHOWING,
		IS_HIDING,
		IS_HIDDEN
	};

	// Aggregate in which control is hosted
	LcCAggregate*					m_agg;

	// For finding settings in XML
#ifdef LC_USE_XML
	LcTmString						m_sXmlName;
	int								m_iXmlMask;
#endif

	
	// Location, scale, orientation, opacity
	LcTPlacement					m_placement;

	// Coordinate space configuration
	LcTVector						m_vInternalExtent;
	LcTVector						m_vInternalOffset;
	LcTmAlloc<LcTTransform>			m_pxfmOrientation;
	bool							m_bUsingLayoutExtent;
	bool							m_isFullScreen;

	// Combined transforms for efficiency
	bool							m_bXfmsDirty;
	LcTTransform					m_xfmToGlobal;
	LcTVector						m_vScaleInternal;

	// Condition
	bool							m_bEnabled;
	bool							m_bVisible;

	// Cache result of isVisible(), as is expensive to calculate
	bool							m_bCachedVisibility;
	bool							m_bIsVisibilityCached;

	// State
	bool							m_bDirty;
	ETweenState						m_eTweenState;
	int								m_animationMask;

	// z-order index
	int								m_drawLayerIndex;
	// Properties
	LcTmString						m_sFontName;
	LcCFont::EStyle					m_iFontStyle;
	LcTColor						m_iFontColor;
	ESketchyMode					m_iSketchyMode;

	LcTColor						m_materialSpecularColor;
	LcTColor						m_materialEmissiveColor;
	LcTScalar						m_materialSpecularShininess;
	LcTVector						m_currentMousePos;

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	LcOglCEffect*									m_effect;

	typedef	LcTmOwnerMap<LcTmString, LcOglCSLType>	TmEffectUniMap;
	typedef	LcTmMap<LcTmString, GLuint>				TmTextureIdMap;  // [Texture name] vs. [Texture ID]
	typedef LcTmMap<int, bool>						TmTexturePotMap; // [Texture ID]   vs. [Texture isPOT]
	TmEffectUniMap									effectUniMap;
	LcTmString										m_visualEffect;
	TmTextureIdMap									m_textureIdMap;
	TmTexturePotMap									m_texturePotMap;
	LcTmString										m_openGLRenderQuality;
	bool											m_bUseCustomEffect;
#endif
	
	// More state
	bool											m_bBlackout;

	bool											m_isTranslucent;

	// Internal space type
	enum { IS_NONE, IS_SPECIFIED, IS_PIXELS } 		m_eInternalType;

	// Builds combined transforms, and is overridden by aggregate class
					void			combineTransforms();
	virtual			void			internalScaleChanged(LcTVector vScaleInternal) { LC_UNUSED(vScaleInternal) }
	

	// Handler implementing LcCAnimator::IObserver
	LC_VIRTUAL		void			onAnimationEvent(LcCAnimator* a, int code);

LC_PRIVATE_INTERNAL_PROTECTED:

					void			setInternalTypeToPixels() { m_eInternalType = IS_PIXELS; }

	// May be redefined for derived aggregate
	LC_VIRTUAL		void			setDirty()			{ m_bDirty = true; }
	LC_VIRTUAL		void			setVisibilityDirty() { m_bIsVisibilityCached = false; }

	// May be redefined for internal derived widgets
	LC_VIRTUAL		bool			doOnRealize();
	LC_VIRTUAL		void			doOnRetire();

	// Tests parent and its visibility
	LC_VIRTUAL		bool			parentIsVisible();
	LC_VIRTUAL		bool			parentIsHidden();

	// Inform widgets of placement change
	LC_VIRTUAL		void			onPlacementChange(int mask) { LC_UNUSED(mask) }

	// Inform widgets that a parents placement has changed
	LC_VIRTUAL		void			onParentPlacementChange()	{}
	LC_VIRTUAL		void			setXfmsDirty();

LC_PRIVATE_INTERNAL_PUBLIC:

	// State - used to trigger onPrepareToHide()
	bool							m_bPreviousVisible;

	// Helpers private to NDE
					bool			isUnder(LcCWidget* w);
					int				getAnimationMask();
	LC_VIRTUAL		void			resetAnimationMask();
					void			setPlacement(const LcTPlacement& p,
												int mask,
												LcCAnimator::EAnimState animationState);
	inline			bool			isDirty()			{ return m_bDirty; }

	// Check if owning aggregate is rotated out of a Z plane
	LC_VIRTUAL		bool			aggregateRotated(LcTVector& location);

	// May be redefined for derived aggregate
	LC_VIRTUAL		void			doPrepareForPaint();
	LC_VIRTUAL		void			doPrepareForPaintIfDirty();

	// Allow derived aggregate to perform any pre-frame update action - note that this is called
	// only on the top-level aggregate registered with the active space
	LC_VIRTUAL		bool			doPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame) { LC_UNUSED(timestamp); LC_UNUSED(finalFrame); return false; }
	LC_VIRTUAL		void			doPreFrameUpdate() {}
	LC_VIRTUAL		void			doPostFrameUpdate() {}
																		  	
					LcTScalar		getGlobalZ(bool& bRotated, bool& bParentRotated);
					bool			isBlackout() const { return m_bBlackout; }
					void			setBlackout(bool flag) { m_bBlackout = flag; }

	// UI events 
#ifdef LC_USE_STYLUS
	// Mouse events
	virtual 		bool			onMouseDown	(const LcTPixelPoint& pt) { LC_UNUSED(pt) return false; }
	virtual 		bool			onMouseUp	(const LcTPixelPoint& pt) { LC_UNUSED(pt) return false; }
	virtual 		bool			onMouseMove(const LcTPixelPoint& pt) { LC_UNUSED(pt) return false; }
#endif

	// Key events
	virtual 		bool			onKeyDown	(int c)	{ LC_UNUSED(c) return false; }
	virtual 		bool			onKeyUp		(int c)	{ LC_UNUSED(c) return false; }
	virtual			void			onKeyRepeat	(int c)	{ LC_UNUSED(c) }

LC_PROTECTED_INTERNAL_PUBLIC:

	LC_VIRTUAL		bool			canBeClipped() { return false; }
	LC_VIRTUAL		void			onPaint(const LcTPixelRect& clip) {LC_UNUSED(clip)}
	LC_VIRTUAL		void			onPaintOpaque(const LcTPixelRect& clip) { LC_UNUSED(clip); }
	LC_VIRTUAL		void			onWantBoundingBox() {}
	LC_VIRTUAL		bool			isOpaque() { return false; }

	// Space needs to call this
	virtual			void			onPrepareForHide()	{}

	// Is widget in fullscreen mode
	LC_VIRTUAL		void			setFullScreen(bool fullScreen) { m_isFullScreen=fullScreen;}
	LC_VIRTUAL		bool			isFullScreen() { return m_isFullScreen;}

protected:

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	EEffectTranslucencyCache						m_effectMakesTranslucent;
#endif

	//Tapping and Tolerance - needed by some sub-classes
	ETappable						m_tappable;
	LcTScalar						m_tapTolerance;

#ifdef LC_USE_LIGHTS
	bool							m_lightModelSimple;
#endif

	LcTScalar						m_cachedOpacity;
	bool							m_useCachedOpacity;
	// Internal sizing/extent
	LC_IMPORT		void			setInternalOffset(const LcTVector& vOS);
	LC_IMPORT		void			setInternalExtent(const LcTVector& vIE);
	inline			void			setInternalExtent(LcTScalar dX, LcTScalar dY, LcTScalar dZ)
										{ setInternalExtent(LcTVector(dX, dY, dZ)); }

	// Use pixel units internally
	LC_VIRTUAL		LcTPixelDim		getPreferredExtent();

	// Property updates
	virtual			void			onFontUpdated()		{}

	// Handlers - see also onPrepareForHide() above
	virtual			void			onRealize()			{}
	virtual			void			onRetire()			{}
	virtual			void			onPrepareForPaint()	{}

	// Event firing helper
	LC_IMPORT		void			fireWidgetEvent(LcTWidgetEvent* e);

	// Event handling
	virtual			void			onWidgetEvent(LcTWidgetEvent* e)		{ LC_UNUSED(e) }
	virtual			void			onAggregateEvent(TAggregateEvent* e)	{ LC_UNUSED(e) }

	// Abstract so keep constructor protected
	LC_IMPORT						LcCWidget();

public:

	// Destruction
	LC_VIRTUAL						~LcCWidget();

	// Methods
	LC_IMPORT		bool			realize(LcCAggregate* s);
	LC_IMPORT		void			retire();
	LC_IMPORT		void			revalidate();

	// Query place in hierarchy
	LC_VIRTUAL		LcCSpace*		getSpace();
	LC_VIRTUAL		LcCApplet*		getApplet();
	inline			LcCAggregate*	getAggregate()		{ return m_agg; }

	// Placement includes all positioning info
	inline		const LcTPlacement&	getPlacement()		{ return m_placement; }
	LC_IMPORT		void			setPlacement(const LcTPlacement& p, int mask);

	// Positioning
	inline			LcTVector		getLocation()		{ return m_placement.location; }
	LC_IMPORT		void			setLocation(const LcTVector& vLocation);
	inline			void			setLocation(LcTScalar dX, LcTScalar dY, LcTScalar dZ)
										{ setLocation(LcTVector(dX, dY, dZ)); }

	// Internal coordinate space adjustment
	LC_IMPORT		LcTVector		getInternalExtent();
	LC_IMPORT		LcTVector		getInternalOffset();

	// Sizing/extent
	LC_IMPORT		LcTVector		getExtent();
	LC_IMPORT		void			setExtent(const LcTVector& vExtent, bool bUsingLayoutExtent);
	inline			void			setExtent(LcTScalar dX, LcTScalar dY, LcTScalar dZ, bool bUsingLayoutExtent)
										{ setExtent(LcTVector(dX, dY, dZ), bUsingLayoutExtent); }

	// Returns true if the set extent is a layout extent.
	LC_VIRTUAL		bool			isUsingLayoutExtent()	{ return m_bUsingLayoutExtent; }
	LC_VIRTUAL		void			setUsingLayoutExtent(bool usingLayoutExtent)	{ m_bUsingLayoutExtent = usingLayoutExtent; }

	// Scaling
	inline			LcTVector		getScale()				{ return m_placement.scale; }
	LC_IMPORT		void			setScale(const LcTVector& vScale);
	inline			void			setScale(LcTScalar d)	{ setScale(LcTVector(d, d, d)); }
	inline			void			setScale(LcTScalar dX, LcTScalar dY, LcTScalar dZ)
										{ setScale(LcTVector(dX, dY, dZ)); }

	// Orientation
	inline			LcTQuaternion	getOrientation()		{ return m_placement.orientation; }
	LC_IMPORT		void			setOrientation(const LcTQuaternion& q);

	// Offset
	LC_IMPORT		void			setOffset(const LcTVector& vOffset);
	inline			LcTVector		getOffset()				{ return m_placement.centerOffset; }

	// Returns transform from local space to global
	LC_IMPORT	const LcTTransform&	getXfmToGlobal();

	// For derived classes to determine how to relate to their bounding boxes
	LC_IMPORT		LcTVector		getOverallScale();

	// Widget condition
	LC_VIRTUAL		void			setEnabled(bool b);
	inline			bool			getEnabled()		{ return m_bEnabled; }
	LC_VIRTUAL		bool			isEnabled();

	// Widget condition
	LC_VIRTUAL		void			setVisible(bool b);
	inline			bool			getVisible()		{ return m_bVisible; }
	LC_IMPORT		bool			isVisible();
	LC_IMPORT		bool			isHidden();

	// Set text properties
	LC_IMPORT		void			setFont(const LcTmString& sName, LcCFont::EStyle iStyle);
	LC_IMPORT		void			setFontColor(LcTColor iColor);

	// Get local text properties
	inline			LcTaString		getFontName()		{ return m_sFontName; }
	inline			LcCFont::EStyle	getFontStyle()		{ return m_iFontStyle; }
	inline			LcTColor		getFontColor()		{ return m_placement.color; }

	// Text property search - through parents, applet or LAF
	LC_VIRTUAL		LcTaString		findFontName();
	LC_VIRTUAL		LcCFont::EStyle	findFontStyle();
	LC_VIRTUAL		LcTColor		findFontColor();
	LC_VIRTUAL		LcTColor		findFontColorDisabled();

	// Other properties
	inline			void			setSketchyMode(ESketchyMode mode)	{ m_iSketchyMode = mode; }
	LC_VIRTUAL		bool			canSketch();
	inline			void			setSketchyAnimation(bool sketchy)
														{ if (sketchy)
															m_animationMask |= LcCAnimator::EMaskAnimating;
														  else
															m_animationMask &= ~LcCAnimator::EMaskAnimating;
														}

	// Appearance
	LC_IMPORT		void			setOpacity(LcTScalar d);
	inline			LcTScalar		getOpacity()		{ return m_placement.opacity; }
	LC_VIRTUAL		bool			isTranslucent();
	inline			void			setTranslucent(bool val) { m_isTranslucent = m_isTranslucent || val; }

#ifdef LC_USE_LIGHTS
	inline			bool			isLightModelSimple() { return m_lightModelSimple; }
	inline			void			setLightModelSimple(bool val) { m_lightModelSimple = val; }
#endif

	// Frame
	LC_IMPORT		void			setFrame(int fr);
	inline			int				getFrame()			{ return m_placement.frame; }

	// For derived classes to get details of how much they should be faded by
	// NB: if blending not possible, should fade to solid background color
	LC_IMPORT		LcTScalar		getOverallOpacity();
	LC_VIRTUAL		LcTColor		getBackgroundColor();
	LC_VIRTUAL		bool			obedientToZ()		{ return false;}

	// Map from local space to global space
	LC_IMPORT		LcTVector		mapLocalToGlobal(const LcTVector& vLocal);

	// Reverse mapping is only used with stylus
#ifdef LC_USE_STYLUS
	LC_IMPORT		LcTVector		mapGlobalToLocal(const LcTVector& vGlobal);
#endif

	// Get the widget's global normal
	LC_IMPORT		LcTVector		getGlobalNormal();

					int				getDrawLayerIndex()				{ return m_drawLayerIndex; }
					void			setDrawLayerIndex(int index)	{ m_drawLayerIndex = index; }

	LC_VIRTUAL		void			setTappable(ETappable tappable)	{ m_tappable=tappable; }
	LC_VIRTUAL		ETappable		getTappable() { return m_tappable; }
	LC_VIRTUAL		void			setTapTolerance(LcTScalar tapTolerance)	{ m_tapTolerance=tapTolerance; }
	LC_VIRTUAL		LcTScalar		getTapTolerance()				{ return m_tapTolerance; }

	// Material properties are available in both OpenGL rendering modes
	inline			LcTColor		getMaterialSpecularColor()		{ return m_materialSpecularColor; }
	LC_IMPORT		void			setMaterialSpecularColor (const LcTColor color)	{ m_materialSpecularColor = color; }

	inline			LcTColor		getMaterialEmissiveColor()		{ return m_materialEmissiveColor; }
	LC_IMPORT		void			setMaterialEmissiveColor (const LcTColor color)	{ m_materialEmissiveColor = color; }

	inline			LcTScalar		getMaterialSpecularShininess()	{ return m_materialSpecularShininess; }
	LC_IMPORT		void			setMaterialSpecularShininess (const LcTScalar shininess)	{ m_materialSpecularShininess = shininess; }

	LC_VIRTUAL		void			releaseResources() {}
	LC_VIRTUAL		void			reloadResources()  { LcCWidget::setDirty();}

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

	int width, height;

	LC_IMPORT	 	void			setImageSize(int w, int h) { width = w; height = h; }

	inline			EEffectTranslucencyCache getEffectMakesTranslucent() { return m_effectMakesTranslucent;}
	inline			void			resetEffectMakesTranslucent() { m_effectMakesTranslucent = EEffectTranslucencyCacheUndefined; }

	inline			int				getImageWidth()		{	return width;	}
	inline			int				getImageHeight()	{	return height;	}

	inline			LcTmString		getVisualEffectName()							{ return m_visualEffect; }
	LC_IMPORT		void			setVisualEffectName(const LcTmString& effect)	{ m_visualEffect = effect; }

	LC_IMPORT		void			addEffectUniform(LcTmString name, LcOglCSLType* slType, LcCSpace *space);
	LC_IMPORT		void			configEffectUniforms(LcCSpace *space, LcTmString path);
	inline 			int				getUniformCount()				{ return effectUniMap.size(); }
	LC_IMPORT		int				passWidgetInfoToOGL();
	LC_IMPORT		int				passWidgetThemeInfoToOGL();
	LC_IMPORT		LcOglCEffect*	getEffect()						{ return m_effect; }
	LC_IMPORT		void			setEffect(LcOglCEffect* effect)	{ m_effect = effect; }

	inline			LcTmString		getOpenGLRenderQualitySetting()								{ return m_openGLRenderQuality; }
	LC_IMPORT		void			setOpenGLRenderQualitySetting (const LcTmString& quality)	{ m_openGLRenderQuality = quality; }
	LC_IMPORT		void			calcTapPosition(const LcTPixelPoint& pt);

	LC_IMPORT		void			populateEffectUniformsFromEffectFile (LcCSpace *space);
	LC_IMPORT		void			prepareStaticDisplacementFromEffect (LcTPlacement& layoutPlacement, LcTPlacement& staticPlacement);
	inline 			void			setUseCustomEffect(bool useCustomEffect) 				{ m_bUseCustomEffect = useCustomEffect; }
	inline 			bool 			getUseCustomEffect() 									{ return m_bUseCustomEffect; }
#endif
	LC_VIRTUAL		bool			contains(const LcTVector& pt, LcTScalar expandRectEdge)	{ LC_ASSERT(0); return false; }
};

#endif // LcCWidgetH

