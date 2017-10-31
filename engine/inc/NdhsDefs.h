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
#ifndef NdhsDefsH
#define NdhsDefsH

enum ENdhsUserAction
{
	ENdhsUserActionIdle
};

typedef enum
{
	ENdhsDecoratorPreFrame = 0,
	ENdhsDecoratorFirstFrame,
	ENdhsDecoratorMidFrame,
	ENdhsDecoratorLastFrame,
	ENdhsDecoratorPostFrame
} ENdhsDecoratorFrame;

typedef enum
{
	ENdhsDecoratorTypeIgnore = 0,
	ENdhsDecoratorTypeLinear,
	ENdhsDecoratorTypeSticky
} ENdhsDecoratorType;

typedef enum
{
	ETemplateTypePage,
	ETemplateTypeComponent,
	ETemplateTypeMenuComponent,
	ETemplateTypeInvalid
} ETemplateType;

typedef enum
{
	ENdhsVelocityProfileUnknown,
	ENdhsVelocityProfileLinear,
	ENdhsVelocityProfileAccelerate,
	ENdhsVelocityProfileDecelerate,
	ENdhsVelocityProfileBounce,
	ENdhsVelocityProfileHalfsine,
	ENdhsVelocityProfileCatapult,
	ENdhsVelocityProfilePauseStartFinish
} ENdhsVelocityProfile;

#define NDHS_VELOCITY_PROFILE_BOUNCE_MAX_EXTENSION 		1.15f
#define NDHS_VELOCITY_PROFILE_BOUNCE_MAX_EXTENSION_POS 	0.75f
#define NDHS_VELOCITY_PROFILE_CATAPULT_MAX_DRAWBACK 	-0.3f
#define NDHS_VELOCITY_PROFILE_CATAPULT_MAX_DRAWBACK_POS	0.5f

typedef enum {
	ENdhsParentVisibilityHideAllParents = 0,
	ENdhsParentVisibilityHideAllShowRoot,
	ENdhsParentVisibilityHideDirectParent,
	ENdhsParentVisibilityShowDirectParent
} ENdhsParentVisibility;

typedef enum
{
	ENdhsNavigationKeyHash = 0x0000FFFF, // start at HIGHWORD so not to conflict with any device scancodes
	ENdhsNavigationKeyAsterisk,
	ENdhsNavigationKeyUp,
	ENdhsNavigationKeyDown,
	ENdhsNavigationKeyLeft,
	ENdhsNavigationKeyRight,
	ENdhsNavigationKeySelect,
	ENdhsNavigationSoftKeyBack,
	ENdhsNavigationOptionsMenu,
	ENdhsNavigationStylusTap,
	ENdhsNavigationOpenLink,
	ENdhsNavigationOnSignal,
	ENdhsNavigationChainAction
} ENdhsNavigation;

typedef enum {
	ENdhsPageStatePreOpen = 0,
	ENdhsPageStateOpen,
	ENdhsPageStateClose,
	ENdhsPageStateInteractive,
	ENdhsPageStateHide,
	ENdhsPageStateSelected,
	ENdhsPageStateShow,
	ENdhsPageStateLaunch,
	ENdhsPageStateNone
} ENdhsPageState;

typedef enum {
	ENdhsGroupTypeItem,
	ENdhsGroupTypeFurniture
} ENdhsGroupType;

typedef enum {
	ENdhsAnimationTypeTerminalState,
	ENdhsAnimationTypeInteractiveState,
	ENdhsAnimationTypeScroll,
	ENdhsAnimationTypeLayoutChange,
	ENdhsAnimationTypeStatic,
	ENdhsAnimationTypeDrag,
	ENdhsAnimationTypeScrollKick,
	ENdhsAnimationTypeNone
} ENdhsAnimationType;

typedef enum {
	ENdhsObjectTypeFurniture,
	ENdhsObjectTypeItemComponent,
	ENdhsObjectTypeFurnitureComponent,
	ENdhsObjectTypeItem
} ENdhsObjectType;

typedef enum {
#ifdef IFX_USE_PLUGIN_ELEMENTS
	ENdhsElementTypePlugin,
#endif
	ENdhsElementTypeAggregate,
	ENdhsElementTypeText,
#ifdef LC_USE_STYLUS
	ENdhsElementTypeDragRegion,
#endif
#ifdef LC_USE_LIGHTS
	ENdhsElementTypeLight,
#endif
	ENdhsElementTypeGraphic,
	ENdhsElementTypeComponent
} ENdhsElementType;

typedef enum {
	ENdhsElementParentPage,
	ENdhsElementParentMenu,
	ENdhsElementParentScreen
} ENdhsElementParent;

typedef enum {
	ENdhsWidgetTypeUnknown,
	ENdhsWidgetTypeBitmap,
	ENdhsWidgetTypeVideoAnimator,
	ENdhsWidgetTypeColoredMesh,
	ENdhsWidgetTypeText,
#ifdef LC_USE_STYLUS
	ENdhsWidgetTypeDragRegion,
#endif
#ifdef IFX_USE_PLUGIN_ELEMENTS
	ENdhsWidgetTypePluginElement,
#endif
#ifdef LC_USE_LIGHTS
	ENdhsWidgetTypeLight,
#endif
	ENdhsWidgetTypeNonWidget
} ENdhsWidgetType;

typedef enum {
	ENdhsLinkTypeUnknown,
	ENdhsLinkTypeMenu,
	ENdhsLinkTypePage,
	ENdhsLinkTypeTheme,
	ENdhsLinkTypeMenuPlugin,
#ifdef IFX_USE_PLUGIN_ELEMENTS
	ENdhsLinkTypeCreatePluginElement,
	ENdhsLinkTypeCreateEventHandler,
#endif
	ENdhsLinkTypeSyncLinkPlugin,
	ENdhsLinkTypeAsyncLinkPlugin
} ENdhsLinkType;

// From where to load menu
typedef enum {
	ENdhsUnknown,
	ENdhsCurrentNode,
	ENdhsOtherNode,
	ENdhsMenuFile,
	ENdhsModule
} ENdhsMenuToLoadFrom;

#ifdef LC_USE_LIGHTS
typedef enum {
	ENdhsLightTypeBulb,
	ENdhsLightTypeDirectional
} ENdhsLightType;

typedef enum {
	ENdhsLightModelNormal,
	ENdhsLightModelSimple
} ENdhsLightModel;

#endif // def LC_USE_LIGHTS

#if defined(NDHS_PREVIEWER)

	// Tracing for previewer
	#define NDHS_TRACE_ENABLED

	typedef enum {
		ENdhsTraceLevelInfo,
		ENdhsTraceLevelWarning,
		ENdhsTraceLevelError
	} ENdhsTraceLevel;

	typedef enum {
#ifdef IFX_GENERATE_SCRIPTS
		ENdhsTraceModuleScriptGenerator,
#endif
#ifdef IFX_USE_SCRIPTS
		ENdhsTraceModuleScriptExecutor,
#endif
		ENdhsTraceModuleTemplate,
		ENdhsTraceModuleGeneral,
		ENdhsTraceModuleExpressions,
		ENdhsTraceModuleOGL
	} ENdhsTraceModule;

	#include "inflexionui/engine/inc/NdhsCLog.h"

	// Call previewer-specific trace functionality
	#define NDHS_TRACE(a, b, c)				NdhsCLog::Instance()->addEntry(a, c)
	#define NDHS_TRACE_EXT(a, b, c, d, e)	NdhsCLog::Instance()->addEntry(a, c, d, e)

	// Enable blocks of code that are only relevant when tracing
	#define NDHS_TRACE_START		{
	#define NDHS_TRACE_END			}

#elif defined(NDHS_PACKAGER)

	// Tracing for packager
	#define NDHS_TRACE_ENABLED

	typedef enum {
		ENdhsTraceLevelInfo,
		ENdhsTraceLevelWarning,
		ENdhsTraceLevelError
	} ENdhsTraceLevel;

	// Call previewer-specific trace functionality
	#define NDHS_TRACE(a, b, c)				switch (a) { case ENdhsTraceLevelInfo	: fprintf(stdout, "%s\n", LcTmString(c).bufUtf8()); break; \
														 case ENdhsTraceLevelWarning: fprintf(stdout, "? %s\n", LcTmString(c).bufUtf8()); break; \
														 case ENdhsTraceLevelError	: fprintf(stderr, "! %s\n", LcTmString(c).bufUtf8()); break; }

	#define NDHS_TRACE_EXT(a, b, c, d, e)	switch (a) { case ENdhsTraceLevelInfo	: fprintf(stdout, "%s\n", LcTmString(c).bufUtf8()); break; \
														 case ENdhsTraceLevelWarning: fprintf(stdout, "? %s\n", LcTmString(c).bufUtf8()); break; \
														 case ENdhsTraceLevelError	: fprintf(stderr, "! %s\n", LcTmString(c).bufUtf8()); break; }

	// Enable blocks of code that are only relevant when tracing
	#define NDHS_TRACE_START		{
	#define NDHS_TRACE_END			}

#elif defined(NDHS_JNI_INTERFACE)

	// Tracing for JNI Interface
	#define NDHS_TRACE_ENABLED

	typedef enum {
		ENdhsTraceLevelInfo,
		ENdhsTraceLevelWarning,
		ENdhsTraceLevelError
	} ENdhsTraceLevel;

	typedef enum {
#ifdef IFX_GENERATE_SCRIPTS
		ENdhsTraceModuleScriptGenerator,
#endif
#ifdef IFX_USE_SCRIPTS
		ENdhsTraceModuleScriptExecutor,
#endif
		ENdhsTraceModuleTemplate,
		ENdhsTraceModuleGeneral,
		ENdhsTraceModuleExpressions,
		ENdhsTraceModuleOGL
	} ENdhsTraceModule;

	#include "inflexionui/engine/inc/NdhsCLog.h"

	// Call previewer-specific trace functionality
	#define NDHS_TRACE(a, b, c)				NdhsCLog::Instance()->addEntry(NULL, a, c)
	#define NDHS_TRACE_EXT(a, b, c, d, e)	NdhsCLog::Instance()->addEntry(NULL, a, c, d, e)

	// Enable blocks of code that are only relevant when tracing
	#define NDHS_TRACE_START		{
	#define NDHS_TRACE_END			}

#elif defined(NDHS_THE_ORIGINAL_TRACE_BEHAVIOUR)

	// Tracing for other platforms
	#define NDHS_TRACE_ENABLED

	typedef enum {
		ENdhsTraceLevelInfo,
		ENdhsTraceLevelWarning,
		ENdhsTraceLevelError
	} ENdhsTraceLevel;

	typedef enum {
		ENdhsTraceModuleTemplate,
		ENdhsTraceModuleGeneral,
		ENdhsTraceModuleExpressions,
		ENdhsTraceModuleOGL
	} ENdhsTraceModule;

	#define NDHS_TRACE_LEVEL			3

	// Calls platform's own TRACE function
	#define NDHS_TRACE(a, b, c)			switch (a) { case ENdhsTraceLevelInfo	: if (NDHS_TRACE_LEVEL > 2) TRACE("Info: " c "\n"	); break; \
													 case ENdhsTraceLevelWarning: if (NDHS_TRACE_LEVEL > 1) TRACE("Warning: " c "\n"); break; \
													 case ENdhsTraceLevelError	: if (NDHS_TRACE_LEVEL > 0) TRACE("Error: " c "\n"	); break; }
	#define NDHS_TRACE_EXT(a, b, c, d, e)

	// Enable blocks of code that are only relevant when tracing
	#define NDHS_TRACE_START		{
	#define NDHS_TRACE_END			}

#elif defined(NU_SIMULATION)

	// Tracing for simtest
	#define NDHS_TRACE_ENABLED

	typedef enum {
		ENdhsTraceLevelInfo,
		ENdhsTraceLevelWarning,
		ENdhsTraceLevelError
	} ENdhsTraceLevel;

	typedef enum {
#ifdef IFX_GENERATE_SCRIPTS
		ENdhsTraceModuleScriptGenerator,
#endif
#ifdef IFX_USE_SCRIPTS
		ENdhsTraceModuleScriptExecutor,
#endif
		ENdhsTraceModuleTemplate,
		ENdhsTraceModuleGeneral,
		ENdhsTraceModuleExpressions,
		ENdhsTraceModuleOGL
	} ENdhsTraceModule;

	// Call previewer-specific trace functionality
	#define NDHS_TRACE(a, b, c)			switch (a) { case ENdhsTraceLevelInfo	: printf("Info: %s\n", LcTmString(c).bufUtf8()); break; \
													 case ENdhsTraceLevelWarning: printf("Warning: %s\n", LcTmString(c).bufUtf8()); break; \
													 case ENdhsTraceLevelError	: printf("ERROR: %s\n", LcTmString(c).bufUtf8()); break; }
	#define NDHS_TRACE_EXT(a, b, c, d, e)	NDHS_TRACE(a, b, c)

	// Enable blocks of code that are only relevant when tracing
	#define NDHS_TRACE_START		{
	#define NDHS_TRACE_END			}

#elif (defined(IFX_LOG_VERBOSE) || defined (IFX_LOG_MAIN))

	// Tracing for other platforms
	#define NDHS_TRACE_ENABLED

	typedef enum {
		ENdhsTraceLevelInfo,
		ENdhsTraceLevelWarning,
		ENdhsTraceLevelError
	} ENdhsTraceLevel;

	typedef enum {
		ENdhsTraceModuleTemplate,
		ENdhsTraceModuleGeneral,
		ENdhsTraceModuleExpressions,
		ENdhsTraceModuleOGL
	} ENdhsTraceModule;

	#define NDHS_TRACE_LEVEL			3

	// NDHS_TRACE does nothing
	#define NDHS_TRACE(a, b, c)         switch (a) { case ENdhsTraceLevelInfo	: if (NDHS_TRACE_LEVEL > 2) IFXP_Error_Print((LcTmString("Info: ") += LcTmString(c)).bufWChar()); break; \
													 case ENdhsTraceLevelWarning: if (NDHS_TRACE_LEVEL > 1) IFXP_Error_Print((LcTmString("Warning: ") += LcTmString(c)).bufWChar()); break; \
													 case ENdhsTraceLevelError	: if (NDHS_TRACE_LEVEL > 0) IFXP_Error_Print((LcTmString("ERROR: ") += LcTmString(c)).bufWChar()); break; }

	#define NDHS_TRACE_EXT(a, b, c, d, e)	NDHS_TRACE(a, b, c)

	// Enable blocks of code that are only relevant when tracing
	#define NDHS_TRACE_START		{
	#define NDHS_TRACE_END			}

#else

	// No tracing (i.e. release builds)

	// NDHS_TRACE does nothing
	#define NDHS_TRACE(a, b, c)
	#define NDHS_TRACE_EXT(a, b, c, d, e)

	// Enable blocks of code that are only relevant when tracing
	#define NDHS_TRACE_START		if (FALSE) {
	#define NDHS_TRACE_END			}

#endif

	/* If Log level is set to verbose */
#if defined(IFX_LOG_VERBOSE)
	#define NDHS_TRACE_VERBOSE(a, b, c) NDHS_TRACE(a, b, c)
#else
	#define NDHS_TRACE_VERBOSE(a, b, c)
#endif

#define NDHS_LOWER_INVALID_DRAW_LAYER_INDEX				-101
#define NDHS_UPPER_INVALID_DRAW_LAYER_INDEX				101
#define NDHS_LOWER_INVALID_DRAW_LAYER_INDEX_STRING		"-101"

#endif
