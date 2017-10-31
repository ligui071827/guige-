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
#ifndef NdhsSharedDefsH
#define NdhsSharedDefsH

// Include XML tag defines.
#include "inflexionui/engine/inc/NdhsXmlDefs.h"
#include "inflexionui/engine/inc/NdhsManifestXmlDefs.h"

/*-------------------------------------------------------------------------*//**
	NDHS version information - used to validate theme project files
*/
	// Name of the project files for signature validation
	#define NDHS_PROJECT_FILENAME					"project.xml"
#if defined(NDHS_PREVIEWER) || defined(NDHS_JNI_INTERFACE)
	#define NDHS_MANIFEST_FILENAME					NDHS_PROJECT_FILENAME
#else
	#define NDHS_MANIFEST_FILENAME					"manifest.xml"
#endif

	#define NDHS_PALETTE_DIR						"Palettes"
	#define NDHS_PALETTE_DIR_LENGTH					8
	#define NDHS_PACKAGE_DIR						"Packages"

	#define NDHS_PACKAGE_FILENAME					"package.xml"
	#define NDHS_APP_SETTINGS_FILENAME  			"settings.xml"
	#define NDHS_GENERAL_TOKENS_FILENAME			"all.tokens"

	#define NDHS_XML_FILE_EXTENSION					".xml"

	#define NDHS_INI_FILE_SAVE_LOCATION				"data/"

	#define NDHS_MANIFEST_ROOT_NAME					"uiManifest"

	#define NDHS_DIR_SEP							"/"
	#define NDHS_DIR_SEP_CHAR						'/'

#if defined (IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
	#define NDHS_SCRIPT_FILENAME					"engine.script"
	#define NDHS_BATCH_FILENAME						"engine.batch"
#endif

	#define NDHS_CUSTOM_P_SCREEN_NAME				"Custom Portrait"
	#define NDHS_CUSTOM_L_SCREEN_NAME				"Custom Landscape"
	#define NDHS_CUSTOM_P_SCREEN_NAME_FULL			"Custom Portrait (%dx%d)"
	#define NDHS_CUSTOM_L_SCREEN_NAME_FULL			"Custom Landscape (%dx%d)"

	// Current app version
	#define NDHS_MAJOR_VERSION						2
	#define NDHS_MINOR_VERSION						6
	#define NDHS_PATCH_VERSION						0

	// This is the oldest theme version which is accepted.
	// Any themes earlier than this, will be rejected
	#define NDHS_EARLIEST_SUPPORTED_MAJOR					2
	#define NDHS_EARLIEST_SUPPORTED_MINOR					4
	#define NDHS_EARLIEST_SUPPORTED_PATCH					7

#if (NDHS_EARLIEST_SUPPORTED_MAJOR > NDHS_MAJOR_VERSION) || \
	((NDHS_EARLIEST_SUPPORTED_MAJOR == NDHS_MAJOR_VERSION) && (NDHS_EARLIEST_SUPPORTED_MINOR > NDHS_MINOR_VERSION)) && \
	((NDHS_EARLIEST_SUPPORTED_MAJOR == NDHS_MAJOR_VERSION) && (NDHS_EARLIEST_SUPPORTED_MINOR == NDHS_MINOR_VERSION) && NDHS_EARLIEST_SUPPORTED_PATCH > NDHS_PATCH_VERSION)
#error NdhsSharedDefs - break version is greater than application version
#endif

#endif //NdhsSharedDefsH
