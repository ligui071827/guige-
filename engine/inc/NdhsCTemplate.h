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
#ifndef NdhsCTemplateH
#define NdhsCTemplateH

#include "inflexionui/engine/inc/LcTPlacement.h"
#include "inflexionui/engine/inc/NdhsCKeyFrameList.h"
#include "inflexionui/engine/inc/ifxui_integration.h"
#include "inflexionui/engine/inc/NdhsCExpression.h"

class NdhsCElement;
class NdhsCTransitionAgent;
class NdhsCElementGroup;
class NdhsCPageManager;
class NdhsCPageModel;
class NdhsCManifest;
class NdhsCMenu;
class NdhsCMenuItem;
class NdhsCMenuComponent;
class NdhsCMenuComponentTemplate;

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	class LcOglCSLType;
#endif	/* IFX_RENDER_DIRECT_OPENGL_20 */

/*-------------------------------------------------------------------------*//**
*/
class NdhsCTemplate : public LcCBase ,public ISerializeable
{
public:
	typedef LcTmOwner<NdhsCExpression::CExprSkeleton> TmMExprSkeleton;

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	LcOglCEffect* 						m_customEffect;
#endif	/* IFX_RENDER_DIRECT_OPENGL_20 */

	//
	// XML Template Data Storage Classes
	//
	class CVariableInfo : public LcCBase
	{
	protected:
		CVariableInfo()		{}

	public:
		LcTmString			name;
		IFXI_FIELDDATA_TYPE	type;

		TmMExprSkeleton		boundExpression;
		TmMExprSkeleton		defaultValue;

		static LcTaOwner<CVariableInfo>		create();
		virtual ~CVariableInfo()			{}
	};

	// Parameter class

	class CParameters : public LcCBase
	{
	protected:
		CParameters()		{}

	public:

		IFXI_FIELDDATA_TYPE	type;
		IFXI_FIELD_MODE		mode;
		IFXI_FIELD_SCOPE	scope;

		LcTmString			name;
		TmMExprSkeleton		defaultValue;

		static LcTaOwner<CParameters>		create();
		virtual ~CParameters()				{}
	};

	// Displacement class

	class CDisplacement : public LcCBase
	{

	public:
		LcTScalar			maxValue;
		LcTScalar			minValue;
		LcTmString			animation;
		LcTmString			value;
		LcTmOwner<NdhsCExpression::CExprSkeleton> exprSkeleton;

		static LcTaOwner<CDisplacement>			create();
		virtual ~CDisplacement()				{}
		virtual LcTScalar						getNormalizedValue(NdhsCExpression* expr);
	};

	class CIElementDisplacementObserver;

	class CElementDisplacement : public LcCBase, public NdhsIExpressionObserver
	{

	public:
		LcTmOwner<NdhsCExpression>	expression;
		CDisplacement * displacement;
		CIElementDisplacementObserver * elementRef;
		LcTPlacement cachedPlacement;
		int cachedMask;
		CElementDisplacement()
		{

		}

	public:
		// forward the expression change to the relevant element
		void expressionDirty(NdhsCExpression* expr)
		{
			if(elementRef!=NULL)
			{
				elementRef->applyDisplacement(this);
			}
		}
		void 	expressionDestroyed(NdhsCExpression* expr){}
		static LcTaOwner<CElementDisplacement>			create();
#ifdef IFX_SERIALIZATION
		ISerializeable*		getSerializeAble(int &type){type=-1; return NULL;};
		bool				isMenuItemChild(){return false;}
#endif /* IFX_SERIALIZATION */
	};

	class CIElementDisplacementObserver
	{
		public:
		virtual void applyDisplacement(CElementDisplacement*displacement)=0;
	};

	//
	// Action class.
	//
	class CAction : public LcCBase
	{
	public:

		class CAttempt : public LcCBase
		{
		protected:
			CAttempt()							{}

		public:
			typedef enum {
				EScrollBy = 0,
				EJumpBy,
				EJumpTo,
				EBack,
				ELink,
				EDeactivate,
				ESuspend,
				EExit,
				ESetFocus,
				EUnsetFocus,
				EMoveFocus,
				EStop,
				ESignal
			} EAttemptType;

			EAttemptType						attemptType;

			static LcTaOwner<CAttempt>			create();
			virtual ~CAttempt()					{}
		};

		class CScrollBy : public CAttempt
		{
		protected:
			CScrollBy()							{}

		public:
			typedef enum {
				EScrollByTypeNormal = 0,
				EScrollByTypeKick
			} EScrollByType;

			LcTScalar							amount;
			LcTmString							action;
			ENdhsVelocityProfile				velocityProfile;
			int									duration;
			LcTmString							field;
			EScrollByType						type;

			TmMExprSkeleton						minExpr;
			TmMExprSkeleton						maxExpr;
			bool								minDefined;
			bool								maxDefined;

			static LcTaOwner<CScrollBy>			create();
			virtual ~CScrollBy()				{}
		};

		class CBack : public CAttempt
		{
		protected:
			CBack() 							{}

		public:
			LcTmString							page;
			LcTmString							action;
			int									backToLevel;

			static LcTaOwner<CBack>				create();
			virtual ~CBack()					{}
		};

		class CLink : public CAttempt
		{
		protected:
			CLink() 							{}

		public:
			TmMExprSkeleton						uri;
			LcTmString							uriString;
			LcTmString							action;

			static LcTaOwner<CLink>				create();
			virtual ~CLink()					{}
		};

		class CSetFocus : public CAttempt
		{
		protected:
			CSetFocus()							{}

		public:
			LcTmString							className;
			LcTmString							action;

			static LcTaOwner<CSetFocus>			create();
			virtual ~CSetFocus()				{}
		};

		class CUnsetFocus : public CAttempt
		{
		protected:
			CUnsetFocus()						{}

		public:
			LcTmString							action;

			static LcTaOwner<CUnsetFocus>		create();
			virtual ~CUnsetFocus()				{}
		};

		class CTabFocus : public CAttempt
		{
		protected:
			CTabFocus()							{}

		public:
			int									amount;
			bool								wrap;
			LcTmString							action;

			static LcTaOwner<CTabFocus>			create();
			virtual ~CTabFocus()				{}
		};

	protected:
		CAction()								{}

	public:
		LcTmOwnerArray<CAttempt>				attempts;

		static LcTaOwner<CAction>				create();
		virtual ~CAction()								{}
	};

	class CConditionBlock : public LcCBase
	{
	protected:
		CConditionBlock() {}

	public:
		TmMExprSkeleton												guardExpr;
		LcTmOwnerMap<LcTmString, NdhsCExpression::CExprSkeleton>	assignments;
		CAction*													action;

		static LcTaOwner<CConditionBlock>	create();
		virtual ~CConditionBlock()			{}
	};

	class CEventInfo : public LcCBase
	{
	protected:
		CEventInfo() {}

	public:
		LcTmOwnerArray<CConditionBlock>		conditions;

		static LcTaOwner<CEventInfo>		create();
		virtual ~CEventInfo()				{}
	};

	class CLayout;

	class CSlotClassTrigger : public LcCBase
	{	
	protected:
		CSlotClassTrigger () {}

	public:
		int			slot;
		LcTmString	elementClass;
		CEventInfo* eventInfo;

		static LcTaOwner<CSlotClassTrigger>		create();
		virtual ~CSlotClassTrigger()			{}
	};
	typedef LcTmOwnerArray<CSlotClassTrigger>	TmASlotClassTriggerList;

	class CState : public LcCBase
	{
	protected:
		CState() {}

	public:
		LcTmMap<int, CEventInfo*>			pressTriggers;
		LcTmMap<int, CEventInfo*>			tapSlotTriggers;
		LcTmMap<LcTmString, CEventInfo*>	tapClassTriggers;
		LcTmMap<int, CEventInfo*>			signalSlotTriggers;
		LcTmMap<LcTmString, CEventInfo*>	signalClassTriggers;

		TmASlotClassTriggerList				signalSlotClassTriggers;

		CEventInfo*							catchAllKeyPress;
		CEventInfo*							catchAllStylusTap;

		TmMExprSkeleton						condition;
		CLayout*							layout;

		static LcTaOwner<CState>			create();

		virtual ~CState()
		{
			pressTriggers.clear();
			tapSlotTriggers.clear();
			tapClassTriggers.clear();
			signalSlotTriggers.clear();
			signalClassTriggers.clear();
			signalSlotClassTriggers.clear();
		}
	};

	//
	// Layout class.
	//
	class CLayout : public LcCBase
	{
	public:
		typedef LcTmMap<int, NdhsCKeyFrameList*>			TmMSlotCurves;
		typedef LcTmMap<LcTmString, TmMSlotCurves>			TmMItemCurves;

	protected:
		CLayout(bool masterLayout, NdhsCTemplate* owner)
		: pageLayout(owner), menuLayout(owner), primaryLightLayout(owner)
		{
			m_isMaster = masterLayout;
			m_owner = owner;

			furnitureLayouts.clear();
		}

		void								construct();

		typedef LcTmOwnerMap<int, NdhsCKeyFrameList>			TmMOwnedSlotCurves;
		typedef LcTmMap<LcTmString, TmMOwnedSlotCurves>			TmMOwnedItemCurves;

		TmMItemCurves                       m_itemCurves;
		TmMSlotCurves                       m_slotCurves;
		TmMOwnedItemCurves                  m_ownedItemCurves;
		TmMOwnedSlotCurves                  m_ownedSlotCurves;

		bool                                m_isMaster;
		NdhsCTemplate*						m_owner;

		bool								overlay;

	public:

		class TmElementLayout
		{
		public:
			bool								hide;
			bool								unload;
			NdhsCTemplate*						owningTemplate;

			int									mask;
			LcTPlacement						layout;
			LcTmMap<LcTmString, LcTmString>		displacementArray;
			LcTmArray<CDisplacement*>			displacementObjectArray;

			TmElementLayout()
			{
				hide = false;
				unload = false;
				owningTemplate = NULL;
				mask = 0;
			}

			TmElementLayout(NdhsCTemplate* templ)
			{
				hide = false;
				unload = false;
				owningTemplate = templ;
				mask = 0;
			}

			virtual ~TmElementLayout()
			{
				displacementObjectArray.clear();
				displacementArray.clear();
			}

			void configureDisplacementsFromXml(LcCXmlElem* elem);
			void mergeDisplacements(TmElementLayout& layout);
			bool inheritDisplacement(CDisplacement* disp);
			TmElementLayout operator=(TmElementLayout& elem2);
		};

		class TaElementLayout : public LcTaAuto<TmElementLayout>
		{
			public:
			TaElementLayout(NdhsCTemplate* templ) : LcTaAuto<TmElementLayout>() { owningTemplate = templ; }
			TaElementLayout operator=(TmElementLayout& elem2);
		};

		class CItemLayout : public LcCBase
		{
		protected:
			CItemLayout(NdhsCTemplate* owner)
			: slotLayout(owner)
			{
				elementLayouts.clear();
			}

		public:
			TmElementLayout						slotLayout;
			LcTmMap<LcTmString, TmElementLayout> elementLayouts;

			static LcTaOwner<CItemLayout>		create(NdhsCTemplate* owner);

			virtual ~CItemLayout()
			{
				elementLayouts.clear();
			}
		};

		LcTmString							inherits;
		LcTmString							name;
		TmElementLayout						pageLayout;
		TmElementLayout						menuLayout;
		LcTmMap<LcTmString, TmElementLayout> furnitureLayouts;
		LcTmOwnerArray<CItemLayout>			itemLayouts;
		TmElementLayout						primaryLightLayout;
		LcTmOwner<CState>					stateInfo;

		TmMSlotCurves&						slotCurves()		{ return m_isMaster ? (TmMSlotCurves&)m_ownedSlotCurves : m_slotCurves; }
		TmMItemCurves&						itemCurves()		{ return m_isMaster ? (TmMItemCurves&)m_ownedItemCurves : m_itemCurves; }

		bool								addCurve(int curveId, const LcTmString& className, LcTmOwner<NdhsCKeyFrameList>& keyframeList);

		bool								isMaster()			{ return m_isMaster; }
		NdhsCTemplate*						owner()				{ return m_owner; }

		LcTPlacement*						getPlacementForClass(ENdhsObjectType		elementUse,
																	int					slot,
																	const LcTmString&	className,
																	int&				mask,
																	bool&               ishide,
																	bool&				unLoadElement);

		NdhsCKeyFrameList*					getSlotCurve(int fromSlot, int toSlot);
		NdhsCKeyFrameList*					getItemCurve(const LcTmString& className, int fromSlot, int toSlot);
		const LcTmArray<NdhsCTemplate::CDisplacement*>* getDisplacements(ENdhsObjectType		elementUse,
																	int					slot,
																	const LcTmString&	className);

		static inline int					makeCurveId(int fromSlot, int toSlot) { return ((fromSlot << 16) | toSlot); }

		static LcTaOwner<CLayout>			create(bool masterLayout, NdhsCTemplate* owner);

		virtual ~CLayout();
	};


	//
	// Aggregate class type
	//
	class CGroup : public LcCBase
	{
	protected:
		CGroup()
		{
			groupType			= ENdhsObjectTypeFurniture;

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
			openGLRenderQuality = "normal";
#endif

		}

	public:

		LcTmString						parentGroup;
		ENdhsObjectType					groupType;
#ifdef LC_USE_LIGHTS
		ENdhsLightModel					lightModel;
#endif

		int								m_drawLayerIndex;
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
		LcTmString 						openGLRenderQuality;
#endif

		static LcTaOwner<CGroup>		create();
		virtual ~CGroup()				{}
	};

	//
	// Base element class
	//
	class CElement : public LcCBase
	{
	protected:
		CElement()
		{
			elementType		= ENdhsElementTypeText;
			isDetail 		= false;
		}

	public:
		ENdhsElementType					elementType;
		LcTmString							elementParent;
		TmMExprSkeleton						resourceData;
		bool								isDetail;
		int									m_drawLayerIndex;

		static LcTaOwner<CElement>			create();
		virtual ~CElement()					{}
	};

	//
	//	Component element class
	//
	class CComponentElement : public CElement
	{
		protected:
			CComponentElement ()
			{
				elementType = ENdhsElementTypeComponent;
				clipArea = LcTVector(1,1,1);
				layoutTeleport = false;
				scrollTeleport = false;
#ifdef LC_USE_LIGHTS
				lightModel = ENdhsLightModelNormal;
#endif

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
				openGLRenderQuality = "normal";
#endif
			}

		public:

			ETemplateType					templateType;
			LcTVector						clipArea;
			bool							layoutTeleport;
			bool							scrollTeleport;

			NdhsCTemplate*			        componentFile;
			LcTmString						path;
			LcTmOwnerMap<LcTmString, NdhsCExpression::CExprSkeleton> bindingParameters;

			LcTmString						className;
#ifdef LC_USE_LIGHTS
			ENdhsLightModel					lightModel;
#endif

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)

			LcTmString 						openGLRenderQuality;

#endif	/*#if defined(IFX_RENDER_DIRECT_OPENGL_20) */

		static LcTaOwner<CComponentElement>
											create();
		void							releaseTemplate();
		virtual ~CComponentElement()	{ bindingParameters.clear(); releaseTemplate();}
	};

	//
	// Graphic element class
	//
	class CGraphicElement : public CElement
	{
	protected:
		CGraphicElement()
		{
			// The extentHint should be 0,0,0 so that the
			// actual image size is used if not set
			extentHint = LcTVector(0, 0, 0);
			layoutTeleport = false;
			scrollTeleport = false;
			frameCount = 1;
			tappable = EPartial;
			tapTolerance = 0.0f;
			sketchyMode = LcCWidget::ESketchyAllowed;
			antiAlias = false;
#ifdef LC_USE_LIGHTS
			lightModel = ENdhsLightModelNormal;
#endif

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
			openGLRenderQuality = "normal";
#endif
		}

	public:

		// Material properties are equally applicable to both versions of OpenGL ES
		LcTColor										materialSpecularColor;
		LcTColor										materialEmissiveColor;
		LcTScalar                                       materialSpecularShininess;
#ifdef LC_USE_LIGHTS
		ENdhsLightModel									lightModel;
#endif

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)

		typedef	LcTmOwnerMap<LcTmString, LcOglCSLType>	TmEffectUniMap;
		TmEffectUniMap									effectUniMap;
		LcTmString										visualEffect;
		LcTmString 										openGLRenderQuality;
		TmMExprSkeleton									useCustomEffect;

#endif	/*#if	defined(IFX_RENDER_DIRECT_OPENGL_20) */

#ifdef IFX_USE_PLUGIN_ELEMENTS
		TmMExprSkeleton						eventHandler;
#endif

		// MeshGrid settings
		int									meshGridX;
		int									meshGridY;

		LcTVector							extentHint;
		bool								layoutTeleport;
		bool								scrollTeleport;
		int									frameCount;
		ETappable							tappable;
		LcTScalar							tapTolerance;
		LcCWidget::ESketchyMode				sketchyMode;
		TmMExprSkeleton 					enableFocusExprSkel;
		bool								antiAlias;

		static LcTaOwner<CGraphicElement>	create();
		virtual ~CGraphicElement()
		{
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
			effectUniMap.clear();
#endif
		}
	};


	//
	// Text element class
	//
	class CTextElement : public CElement
	{
	public:

		class CFont : public LcCBase
		{
		protected:
			CFont()
			{
				style = LcCFont::DEFAULT;
				height = 0;
			}

		public:
			TmMExprSkeleton						face;
			LcCFont::EStyle						style;
			LcTmString							color;
			int									height;
			LcTmString							hAlign;
			LcTmString							vAlign;
			LcTmString							abbrevSuffix;

			static LcTaOwner<CFont>				create();
		};

	protected:
		CTextElement()
		{
			layoutTeleport	= false;
			scrollTeleport	= false;
			// default tappable for text element
			tappable		= EPartial;
			tapTolerance	= 0.0f;
			sketchyMode		= LcCWidget::ESketchyAllowed;
			antiAlias = false;
#ifdef LC_USE_LIGHTS
			lightModel = ENdhsLightModelNormal;
#endif

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
			openGLRenderQuality = "normal";
#endif
		}

		void								construct();

	public:

		// Material properties are equally applicable to both versions of OpenGL ES
		LcTColor										materialSpecularColor;
		LcTColor										materialEmissiveColor;
		LcTScalar                                       materialSpecularShininess;
#ifdef LC_USE_LIGHTS
		ENdhsLightModel									lightModel;
#endif

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)

		typedef	LcTmOwnerMap<LcTmString, LcOglCSLType>	TmEffectUniMap;
		TmEffectUniMap									effectUniMap;
		LcTmString										visualEffect;
		LcTmString 										openGLRenderQuality;
		TmMExprSkeleton									useCustomEffect;

#endif	/*#if	defined(IFX_RENDER_DIRECT_OPENGL_20) */

#ifdef IFX_USE_PLUGIN_ELEMENTS
		TmMExprSkeleton						eventHandler;
#endif

		// MeshGrid settings
		int									meshGridX;
		int									meshGridY;

		LcTmOwner<CFont>					font;
		bool								layoutTeleport;
		bool								scrollTeleport;
		ETappable							tappable;
		LcTScalar							tapTolerance;
		LcCWidget::ESketchyMode				sketchyMode;
		TmMExprSkeleton 					enableFocusExprSkel;
		bool								antiAlias;
		TmMExprSkeleton						verticalMarquee;
		TmMExprSkeleton						horizontalMarquee;
		int									marqueeSpeed;

		static LcTaOwner<CTextElement>		create();
		virtual ~CTextElement()
		{
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
			effectUniMap.clear();
#endif
		}
	};


	//
	// Plug-in element class.
	//
#ifdef IFX_USE_PLUGIN_ELEMENTS
	class CPluginElement : public CElement
	{
	protected:
		CPluginElement()
		{
			extentHint = LcTVector(0, 0, 0);
			tappable = EPartial;
			tapTolerance = 0.0f;
			layoutTeleport = false;
			scrollTeleport = false;
			antiAlias = false;
#ifdef LC_USE_LIGHTS
			lightModel = ENdhsLightModelNormal;
#endif

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
			openGLRenderQuality = "normal";
#endif
		}

	public:

		// Material properties are equally applicable to both versions of OpenGL ES
		LcTColor										materialSpecularColor;
		LcTColor										materialEmissiveColor;
		LcTScalar                                       materialSpecularShininess;
#ifdef LC_USE_LIGHTS
		ENdhsLightModel									lightModel;
#endif

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)

		typedef	LcTmOwnerMap<LcTmString, LcOglCSLType>	TmEffectUniMap;
		TmEffectUniMap									effectUniMap;
		LcTmString										visualEffect;
		LcTmString 										openGLRenderQuality;
		TmMExprSkeleton									useCustomEffect;

#endif	/*#if	defined(IFX_RENDER_DIRECT_OPENGL_20) */

		LcTVector							extentHint;
		ETappable							tappable;
		LcTScalar							tapTolerance;
		bool								layoutTeleport;
		bool								scrollTeleport;
		TmMExprSkeleton						eventHandler;
		TmMExprSkeleton 					enableFocusExprSkel;
		bool								antiAlias;

		// MeshGrid settings
		int									meshGridX;
		int									meshGridY;

		static LcTaOwner<CPluginElement>	create();
		virtual ~CPluginElement()
		{
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
			effectUniMap.clear();
#endif
		}
	};
#endif

	// Stores the order information for tabbing.
	struct TTabData
	{
		int				tabOrder;
		LcTmString		className;
		ENdhsGroupType	elementType;
	};

#ifdef LC_USE_LIGHTS
	//
	// Secondary light source element
	//
	class CSecondaryLightElement : public CElement
	{
	protected:
		CSecondaryLightElement()
		{
			lightType 				= ENdhsLightTypeBulb;
			elementParent 			= ENdhsElementParentScreen;
			attenuationConstant		= 1;
			attenuationLinear		= 0;
			attenuationQuadratic	= 0;
			layoutTeleport			= false;
			scrollTeleport			= false;
			specularColor			= LcTColor::WHITE;
		}

	public:
		ENdhsLightType								lightType;

		LcTColor									specularColor;

		LcTScalar									attenuationConstant;
		LcTScalar									attenuationLinear;
		LcTScalar									attenuationQuadratic;
		LcTaString									groupName;
		bool										layoutTeleport;
		bool										scrollTeleport;

		static LcTaOwner<CSecondaryLightElement> 	create();
		virtual ~CSecondaryLightElement()			{}
	};
#endif // LC_USE_LIGHTS

#ifdef LC_USE_STYLUS
	//
	//	Drag Region element class
	//
	class CDragRegionElement : public CElement
	{
	public:
		class CDragRegionSetting : public LcCBase
		{
		protected:
			CDragRegionSetting ()
			{
				isDragEnabled = false;
				isAbsolute = false;
			}

		public:
			LcTmString						field;
			bool 							inverted;
			bool							isAbsolute;
			bool							isDragEnabled;
			LcTScalar						sensitivity;
			LcTScalar						threshold;
			TmMExprSkeleton					deceleration;
			TmMExprSkeleton					maxVelocity;
			TmMExprSkeleton					minValue;
			TmMExprSkeleton					maxValue;

			static LcTaOwner<CDragRegionSetting>create();
		};

	protected:
		CDragRegionElement ()
		{
		}

		void								construct();

	public:
		LcTmOwner<CDragRegionSetting>		xDrag;
		LcTmOwner<CDragRegionSetting>		yDrag;
		// whether axis are absolute/relative
		bool								isAbsolute;
		ETappable							tappable;
		LcTScalar							tapTolerance;

		static LcTaOwner<CDragRegionElement>create();

		virtual 							~CDragRegionElement(){}
	};
#endif //LC_USE_STYLUS

	//
	// Decoration state class
	//

class CLayoutDecoration : public LcCBase
	{
	public:

		typedef enum {
			EToActive = -5,
			EFromActive = -4,
			EActive = -3,
			ESlotAny = -2,
			EInactive = -1
		} ESlotType;

		class CDecorationInfo : public LcCBase
		{
		public:

			class CAnimationRef : public LcCBase
			{
			protected:
				CAnimationRef()
				{
					duration = 0;
					loopCount = 1;
				}

			public:

				LcTmString							animation;
				LcTmString							showAnimation;
				LcTmString							hideAnimation;

				// These are only used by static animations
				int									duration;
				int									loopCount;
				ENdhsVelocityProfile				velocityProfile;

				static LcTaOwner<CAnimationRef>		create();
			};

			class CDecorationItem : public LcCBase
			{
			protected:
				CDecorationItem() 					{}
				void								construct();

			public:

				LcTmOwner<CAnimationRef>						slotAnimation;
				LcTmOwnerMap<LcTmString, CAnimationRef> 		elementAnimations;
				LcTmOwner<CAnimationRef>						defaultElementAnimation;

				static LcTaOwner<CDecorationItem>	create();
			};

			class CFromSlot : public LcCBase
			{
			protected:
				CFromSlot()							{}

			public:
				LcTmOwnerMap<int, CDecorationItem>				fromSlotItemAnimations;
				LcTmOwnerMap<int, CDecorationItem>				offsetSlotItemAnimations;

				static LcTaOwner<CFromSlot>						create();
			};

			class TTimingData
			{
			public:
				TTimingData()
				{
					delay = -1;
					duration = -1;
					backgroundDelay = -1;
					velocityProfile = ENdhsVelocityProfileUnknown;
				}

				int						delay;
				int						duration;
				int						backgroundDelay;
				int						primaryLightDelay;
				int						primaryLightDuration;
				ENdhsVelocityProfile 	velocityProfile;
			};

			class CTrigger : public LcCBase
			{
			protected:
				CTrigger()					{}

			public:
				int							key;
				LcTScalar					position;

				static LcTaOwner<CTrigger>	create();
			};

			class CTriggersList : public LcCBase
			{
			protected:
				CTriggersList()					{}

			public:
				int								cleanupTrigger;
				bool							hasCleanupTrigger;
				LcTmOwnerArray<CTrigger>		triggerList;
				// These are only used by static animations
				int								transitionTime;
				int								loopCount;

				static LcTaOwner<CTriggersList>	create();

				void addTrigger(int, const LcTScalar position);
			};

		protected:
			CDecorationInfo()
			{
				//slotActive = ESlotAny;
			}

			void								construct();

		public:

			// Aggregate, furniture and detail animations
			LcTmOwner<CAnimationRef>						pageAnimation;
			LcTmOwner<CAnimationRef>						menuAnimation;
			LcTmOwnerMap<LcTmString, CAnimationRef>			furnitureAnimations;
			LcTmOwner<CAnimationRef>						outerGroupAnimations;
			LcTmOwner<CAnimationRef>						defaultFurnitureAnimation;

			// Default item animation
			LcTmOwner<CDecorationItem>						defaultItemAnimations;

			// Item animations for static and state change
			LcTmOwnerMap<int, CDecorationItem>				slotItemAnimations;

			// Item animations for item slot reassign
			LcTmOwnerMap<int, CFromSlot>					toSlotItemAnimations;

			TTimingData										timingData;
			bool											timingDataSet;

			// trigger list
			LcTmOwner<CTriggersList>						triggers;
			bool											triggersSet;

			void cleanupDecorationItemMap(LcTmOwnerMap<int, CDecorationItem>& itemMap);

			static LcTaOwner<CDecorationInfo>	create();
			virtual							~CDecorationInfo();
		};

	protected:
		CLayoutDecoration()
		{
		}

	public:
		LcTmString							layout;
		LcTmString							fromLayout;
		typedef LcTaArray<CDecorationInfo*> TmADecInfo;

		LcTmOwnerArray<CDecorationInfo>			stateDecorations;
		LcTmOwner<CDecorationInfo>				scrollDecoration;
		LcTmOwner<CDecorationInfo>				staticDecoration;

		static LcTaOwner<CLayoutDecoration> create();
		virtual								~CLayoutDecoration()		{}
	};

	typedef LcTmArray<CLayout*> TmMLayoutList;

	typedef LcTmOwnerArray<CVariableInfo>  			TmMVariableInfoList;
	typedef LcTmOwnerMap<LcTmString, CParameters>	TmMParameterMap;

private:

	LcTmString							m_templateFile;
	//
	// XML Template Data
	//

	LcTmOwnerArray<CAction::CAttempt>	m_defaultSelectAttempt;
	LcTmOwnerArray<CAction::CAttempt>	m_defaultBackAttempt;

	// Trigger
	CLayoutDecoration::CDecorationInfo::CTriggersList* m_pAnimTriggerList;

	//
	// Anything after this point is non-XML internal class data
	// Only data read directly from the template XML file should
	// go above this point
	//

	enum EMemberType
	{
		EElement,
		EGroup
	};

	typedef struct tagGroupMember
	{
		EMemberType type;
		LcTmString name;
	} TGroupMember;

	LcTmMap<LcTmString, LcTmArray<TGroupMember> > m_groupList;

	LcTmOwnerArray<CDisplacement> 		m_displacementObjectStore;

protected:

	NdhsCPageManager*					m_pageManager;
	ETemplateType						m_templateType;
#ifdef LC_USE_LIGHTS
	ENdhsLightModel						m_lightModel;
#endif
	NdhsCManifest*						m_paletteManifest;
	LcTVector							m_boundingBox;

	LcTmOwner<LcCXmlElem>				m_root;

	// Layouts
	typedef LcTmOwnerMap<LcTmString, CLayout> TmMLayouts;
	TmMLayouts							m_layouts;
	// Actions
	typedef LcTmOwnerMap<LcTmString, CAction> TmMActions;
	TmMActions							m_actions;

	// Animations
	typedef LcTmOwnerMap<LcTmString, NdhsCKeyFrameList> TmMAnimations;
	TmMAnimations						m_animations;

	// Tab order table.
	typedef LcTmArray<TTabData>	TmATabOrderList;
	TmATabOrderList						m_tabOrderList;

#ifdef NDHS_JNI_INTERFACE
	bool								m_skipFirstActiveCheck;
#endif

	// General template settings
	int									m_firstSelectable;
	int									m_lastSelectable;
	int									m_slotCount;
	int									m_firstActive;
	int									m_populateFrom;
	int									m_defaultLayoutTime;
	int									m_defaultScrollTime;
	int 								m_defaultTerminalTime;
	int									m_drawLayerIndex; //Draw Layer Index of the page
	ENdhsVelocityProfile				m_defaultLayoutVelocityProfile;
	ENdhsVelocityProfile				m_defaultScrollVelocityProfile;
	ENdhsVelocityProfile				m_defaultTerminalVelocityProfile;
	bool								m_fullMode;
	bool								m_scrollWrap;
	int									m_scrollSpan;
	LcTScalar  							m_scrollRebound;
	LcTScalar							m_scrollKickDeceleration;
	LcTScalar							m_scrollKickMaxVelocity;

	bool 								m_focusStop;
	LcTmString							m_defaultFocus;

	TmMVariableInfoList					m_variableInfoList;
	TmMParameterMap						m_paramterMap;

	// Element Classes

	typedef LcTmOwnerMap<LcTmString, CElement> TmMElements;
	TmMElements							m_itemClasses;

	typedef LcTmArray<LcTmString>		TmAClasses;
	TmAClasses							m_itemClassNames;

	TmMElements							m_furnitureClasses;

	typedef LcTmOwnerMap<LcTmString, CGroup> TmMGroups;
	TmMGroups							m_groupClasses;

	TmAClasses							m_furnitureClassNames;
	TmAClasses							m_groupClassNames;

	// Decorations
	typedef LcTmOwnerArray<CLayoutDecoration> TmADecorations;
	TmADecorations						m_decorations;
	typedef LcTmOwnerArray<CLayoutDecoration> TmMLayoutDecorations;

	TmMLayoutDecorations 				m_layoutDecorationsMap;

	TmMLayoutList						m_layoutList;

	LcTmOwnerArray<CEventInfo>			m_eventDump;

private:

	void								cleanup();

	TmMElements*						getItemClasses() { return &m_itemClasses; }
	TmMElements*						getFurnitureClasses() { return &m_furnitureClasses; }

	// Functions to load XML into data structures

#ifdef LC_USE_STYLUS
	bool 								configureDragRegionFromXml(LcCXmlElem* eDrag, CDragRegionElement::CDragRegionSetting* drag);
#endif

protected:

	bool 								configureVariablesFromXML(LcCXmlElem* eVars);
	bool 								configureParameterFromXML(LcCXmlElem* eParams);
	bool								checkNameUniqueness(LcTmString& name);
	bool								configureTimingFromXML(LcCXmlElem* eTiming);
	bool 								configureFocusSettingsFromXML(LcCXmlElem* eFocus);
	bool 								configureGroupsFromXml(LcCXmlElem* eAggregates);
	bool								configureAnimationFromXml(LcCXmlElem* eAnimation, int stackLevel);
	bool								configureItemFromXml(LcCXmlElem* eItem, CLayout::CItemLayout* itemLayout, int stackLevel);
	bool 								configureStateInfoFromXml(LcCXmlElem* eState, CState* state, CLayout* layout);
	bool 								configureEventInfoFromXML(LcCXmlElem* eEvent, CEventInfo* eventInfo);
	bool								configureBoundingBox(LcCXmlElem* eBoundingBox);

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

	LC_EXPORT bool						configureCustomEffectFromXml(const LcTmString& effectFileName, LcTmString& effectName, NdhsCManifest* paletteMan, int stackLevel);

#endif

	virtual bool						configureDecorationRefFromXml(LcCXmlElem* eComponent, bool bStatic,
																		CLayoutDecoration::CDecorationInfo::CAnimationRef* animationRef);
	int									convertTriggerKey(const LcTmString& key);

	NdhsCKeyFrameList*					getComponentDecorationAnimation(CLayoutDecoration::CDecorationInfo* info, int initialSlotActive,
																		ENdhsAnimationType animType,
																		int finalSlotActive, ENdhsObjectType objectType,
																		const LcTmString& elementClass, int fromSlot, int toSlot,
																		int* transitionTime, int* loopCount, ENdhsVelocityProfile* velocityProfile);
	NdhsCKeyFrameList*					getDecorationFromSlotAnimation(CLayoutDecoration::CDecorationInfo::CFromSlot* from,
																		int initialSlotActive, ENdhsAnimationType animType,
																		ENdhsObjectType objectType, const LcTmString& elementClass,
																		int fromSlot, int toSlot, int* transitionTime, int* loopCount,
																		ENdhsVelocityProfile* velocityProfile);
	NdhsCKeyFrameList*					getDecorationItemAnimation(CLayoutDecoration::CDecorationInfo::CDecorationItem* item,
																		ENdhsAnimationType animType, ENdhsObjectType objectType,
																		const LcTmString& elementClass, int* transitionTime, int* loopCount,
																		ENdhsVelocityProfile* velocityProfile);
	NdhsCKeyFrameList*					getActualDecorationAnimation(CLayoutDecoration::CDecorationInfo::CAnimationRef* ref, const LcTaString& animation,
																		ENdhsAnimationType animType, int* transitionTime, int* loopCount,
																		ENdhsVelocityProfile* velocityProfile);

protected:
	void								getBestDecInfo( ENdhsAnimationType	animationType,
														 				CLayoutDecoration::CDecorationInfo::TTimingData**	pTimingData,
														 				CLayoutDecoration::CDecorationInfo::CTriggersList**	pTriggers,const LcTmString& oLayout,
																		const LcTmString &nLayout);

	bool                                configureKeyFrameListFromXml(LcCXmlElem* eAnimation, NdhsCKeyFrameList* keyFrameList, const int maskToRead, int stackLevel);
	bool								configureActionFromXml(LcCXmlElem* eAction);
	virtual void						configureAttemptFromXml(LcCXmlElem* eAttempt, CAction* action);
	bool								configureDefaultActions();
	bool								configureTriggersFromXml(LcCXmlElem* eState, CState* state);
	bool 								convertKeyToInt(LcTmString& key, int& convertedKey, bool& catchAll);
	bool								configureLayoutFromXml(LcCXmlElem* eLayout, int stackLevel);
	bool								configureElementListFromXml(LcCXmlElem* eElementList, ENdhsGroupType groupType,
																		CLayout* layout, CLayout::CItemLayout* itemLayout, int stackLevel);
	bool								configureClassesFromXml(LcCXmlElem* eClasses, int stackLevel, int& nestedComponentLevel);
	bool								configureFontFromXml(LcCXmlElem* eFont, CTextElement::CFont* font);

	bool								configureElementClassesFromXml(LcCXmlElem* eElementClasses,
																		ENdhsGroupType groupType,
																		CTextElement::CFont* defaultFont,
																		int stackLevel,
																		int& nestedComponentLevel);
	bool                                configureCurveFromXml(LcCXmlElem* eCurve, CLayout* pLayout, int stackLevel);
	bool								getColorFromXML(LcCXmlElem* placementElem, const LcTmString& colorName, int stackLevel, LcTColor& retColor);

	LcTaOwner<NdhsCElement> 			createElement(const LcTmString&		className,
														CElement*			pElement,
														ENdhsObjectType		usage,
														NdhsCElementGroup*	page,
														NdhsCMenu*			menu,
														NdhsCMenuItem*		menuItem,
														int					stackLevel,
														NdhsCElementGroup*	parentGroup);

	LC_IMPORT static void				mergeLayouts(CLayout& layout1, CLayout* layout2);
	virtual bool 						isOuterGroup(const LcTaString& elementName);
	bool  								isValidVariableName(const LcTmString& varName);

private:
	void								setDefaultValues(CLayout& layout);

	NdhsCKeyFrameList* 					getFurnitureDecorationAnimation(CLayoutDecoration::CDecorationInfo* info, int initialSlotActive,
																		ENdhsAnimationType animType,
																		int finalSlotActive, ENdhsObjectType objectType,
																		const LcTmString& elementClass,
																		int fromSlot, int toSlot,
																		int* transitionTime, int* loopCount, ENdhsVelocityProfile* velocityProfile);
	NdhsCKeyFrameList* 					getItemDecorationAnimation(CLayoutDecoration::CDecorationInfo* info, int initialSlotActive,
																		ENdhsAnimationType animType,
																		int finalSlotActive, ENdhsObjectType objectType,
																		const LcTmString& elementClass,
																		int fromSlot, int toSlot,
																		int* transitionTime, int* loopCount, ENdhsVelocityProfile* velocityProfile);

protected:

	// Allow only 2-phase construction
										NdhsCTemplate(NdhsCPageManager* pageManager, LcTmOwner<LcCXmlElem>& root);
										NdhsCTemplate();
public:

	// Creation/destruction
	static	LcTaOwner<NdhsCTemplate> create(NdhsCPageManager* pageManager, LcTmOwner<LcCXmlElem>& root);
	virtual								~NdhsCTemplate();

	NdhsCManifest*						getPaletteManifest()				{ return m_paletteManifest; }
	void								setPaletteManifest(NdhsCManifest* palManifest)	{ m_paletteManifest = palManifest; }

	inline int							getDefaultTerminalTime()			{ return m_defaultTerminalTime; }
	inline ENdhsVelocityProfile			getDefaultTerminalVelocityProfile()	{ return m_defaultTerminalVelocityProfile; }

	inline int							getDefaultLayoutTime()				{ return m_defaultLayoutTime; }
	inline ENdhsVelocityProfile			getDefaultLayoutVelocityProfile()	{ return m_defaultLayoutVelocityProfile; }

	TmMVariableInfoList&				getVariableInfoList()				{ return m_variableInfoList; }
	TmMParameterMap&					getParametersMap()					{ return m_paramterMap; }

	LcTVector							getComponentBoundingBox()			{ return m_boundingBox; }

	ETemplateType						determineTemplateType(LcTmString& templateType);
	ETemplateType						getTemplateType()					{ return m_templateType; }

#ifdef IFX_USE_SCRIPTS
	// Wrapper
	bool								convertKeyToIntWrapper(LcTmString& key, int& convertedKey, bool& catchAll);
#endif

	// Finds the tab order data for a particular index.	Returns true if successful.
	// The index begins at 1 and goes up to getTabOrderCount().
		   bool							getTabOrderData(int index, TTabData& tabData);
	inline int							getTabOrderCount()					{ return (int)m_tabOrderList.size(); }
		   int							getTabIndex(const LcTmString& className, ENdhsGroupType elementType);

	bool								isFocusStop()						{ return m_focusStop; }
	LcTaString							getDefaultFocus()					{ return m_defaultFocus; }


	LC_IMPORT LcTaOwner<CLayout>		getMergedLayout(CLayout* unmergedLayout);

	ENdhsVelocityProfile				strToVelocityProfile(const LcTmString& profile);

	void								storeDisplacement(LcTaOwner<CDisplacement> dispObject);

	virtual LC_IMPORT NdhsCKeyFrameList*
										getDecorationAnimation(	int							initialSlotActive,
																		ENdhsAnimationType			animType,
																		int							finalSlotActive,
																		ENdhsObjectType				objectType,
																		const LcTmString&			elementClass,
																		int							fromSlot,
																		int							toSlot,
																		int*						transitionTime,
																		int*						loopCount,
																		ENdhsVelocityProfile* 		velocityProfile,
																		const LcTmString& 			oLayout,
																		const LcTmString&			nLayout
																		);

	LC_EXPORT void 						getLayoutDecorationInfo(int&								delay,
																		int&						duration,
																		ENdhsVelocityProfile&		velocityProfile,
																		int&						backgroundDelay,
																		CLayoutDecoration::CDecorationInfo::CTriggersList**
																									pTriggerList,
																		int&						primaryLightDelay,
																		int&						primaryLightDuration,const LcTmString& oLayout,
																		const LcTmString&			nLayout);

	LC_IMPORT void						getTerminalDecorationInfo(int&						delay,
																		 int&						duration,
																		 ENdhsVelocityProfile&		velocityProfile,
																		 int&						backgroundDelay,
																		 CLayoutDecoration::CDecorationInfo::CTriggersList **
																		  							pTriggerList,
																		 int&						primaryLightDelay,
																		 int&						primaryLightDuration,const LcTmString& oLayout,
																		 const LcTmString&			nLayout);

	LC_IMPORT void						getStateDecorationInfo(	CLayoutDecoration::CDecorationInfo::CTriggersList
																									**pTriggerList, const LcTmString& oLayout,
																		const LcTmString&			nLayout);

	LC_IMPORT void						getStaticDecorationInfo(int							slotActive,
																		CLayoutDecoration::CDecorationInfo::CTriggersList **pTriggerList,const LcTmString& oLayout,
																		const LcTmString&			nLayout);

	LC_IMPORT void						getScrollDecorationInfo(int							slotActive,
																		CLayoutDecoration::CDecorationInfo::CTriggersList	**pTriggerList,const LcTmString& oLayout,
																		const LcTmString&			nLayout);

	LC_IMPORT LcTmArray<CAction::CAttempt*>* getAttemptsForAction(const LcTmString& action);
	LC_EXPORT NdhsCTemplate::CAction*		 getAction(const LcTmString& action);
	TmMLayoutList& 							 getLayoutList()						{ return m_layoutList; }

	// Function to load XML into data structures
	virtual LC_IMPORT bool				configureFromXml(const LcTmString& designSize, int stackLevel, int& nestedComponentLevel);
	LC_IMPORT bool						configureSettingsFromXml(LcCXmlElem* eSettings);
	LC_IMPORT bool 						configureDecorationFromXml(LcCXmlElem* decorationRoot);

	LC_IMPORT bool						configureDecorationTypesFromXml(LcCXmlElem* eInitialState, CLayoutDecoration* initialState);

#ifdef NDHS_JNI_INTERFACE
	LC_IMPORT void						setSelectable(int first, int last, int active);
	LC_IMPORT void						skipFirstActiveCheck()				{ m_skipFirstActiveCheck = true; }
	inline LcCSpace*					getSpace();
#endif //NDHS_JNI_INTERFACE

	LcTaOwner<NdhsCElementGroup> 		createElementGroup(NdhsCElementGroup* page,
															NdhsCMenu* menu,
															NdhsCMenuItem* menuItem,
															const LcTmString& groupName,
															int stackLevel,
															int drawLayerIndex,
															NdhsCElementGroup*	parentGroup);

#ifdef LC_USE_LIGHTS	
	inline ENdhsLightModel				getLightModel() {return m_lightModel;}
#endif

	int 								getFurnitureElementCount()			{ return (int)(m_furnitureClasses.size()); }

	virtual NdhsCTemplate::CElement* 					getElementClass(LcTaString className, ENdhsObjectType elementType);
	const NdhsCKeyFrameList*			getAnimation(const LcTmString& animName) { TmMAnimations::iterator it = m_animations.find(animName);
																					return (it == m_animations.end()) ? NULL : (it->second); }
#ifdef IFX_SERIALIZATION
			bool						isMenuItemChild(){return false;}
	virtual	SerializeHandle				serialize(LcCSerializeMaster *serializeMaster,bool force=false){return -1;}
#endif /* IFX_SERIALIZATION */
};

#endif
