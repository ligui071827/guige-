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
#include "inflexionui/engine/inc/NdhsCPageTemplate.h"

#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageTemplate> NdhsCPageTemplate::create(NdhsCPageManager* pageManager, const LcTmString& templateFile)
{
	LcTaOwner<NdhsCPageTemplate> ref;
	ref.set(new NdhsCPageTemplate(pageManager, templateFile));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageTemplate::NdhsCPageTemplate(NdhsCPageManager* pageManager, const LcTmString& templateFile)
{
	m_pageManager = pageManager;
	m_templateFile = templateFile;

	m_parentVisibility = ENdhsParentVisibilityHideAllParents;
	m_showWhenInSub = false;

#ifdef NDHS_JNI_INTERFACE
	m_skipFirstActiveCheck = false;
#endif //NDHS_JNI_INTERFACE

}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageTemplate::~NdhsCPageTemplate()
{
	cleanup();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageTemplate::cleanup()
{
	m_parentVisibility = ENdhsParentVisibilityHideAllParents;
	m_showWhenInSub = false;
	m_cleanupLink = "";
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCPageTemplate::configureFromXml(const LcTmString& designSize, int stackLevel, int& nestedComponentLevel)
{
	LcTaString err;
	bool result = false;
	bool success = true;

	cleanup();

	// Set the directory slash separators to the non default if required.
	LcTaString localPath = m_templateFile;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif
	LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(localPath, err);

	if (root)
	{
		// Loop simply allow us to break out if an error occurs
		do
		{
			//
			// Settings Element
			//

			if (false == (success = configureSettingsFromXml(root->find(NDHS_TP_XML_SETTINGS), designSize)))
			{
				break;
			}


			//
			// Actions Element
			//

			LcCXmlElem* eActions = root->find(NDHS_TP_XML_ACTIONS);
			if (eActions == NULL)
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Actions section missing");
			}

			LcCXmlElem* eAction = eActions ? eActions->getFirstChild() : NULL;
			for (; eAction && success; eAction = eAction->getNext())
			{
				if (eAction->getName().compareNoCase(NDHS_TP_XML_ACTION) != 0)
					continue;

				success = configureActionFromXml(eAction);
			}


			if (!success)
				break;


			// Configure any default actions
			success = configureDefaultActions();


			if (!success)
				break;

			//
			// Classes Element
			//

			if (false == (success = configureClassesFromXml(root->find(NDHS_TP_XML_CLASSES), stackLevel, nestedComponentLevel)))
			{
				break;
			}


			//
			// Layouts Element
			//

			LcCXmlElem* eLayouts = root->find(NDHS_TP_XML_LAYOUTS);
			LcCXmlElem* eLayout = eLayouts ? eLayouts->getFirstChild() : NULL;

			success = true;
			for (; eLayout && success; eLayout = eLayout->getNext())
			{
				if (eLayout->getName().compareNoCase(NDHS_TP_XML_LAYOUT) != 0)
					continue;

				success = configureLayoutFromXml(eLayout, stackLevel);
			}



			if (!success || !eLayouts)
			{
				NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Layouts section missing");
				success = false;
				break;
			}

			//
			// Animations Element
			//

			LcCXmlElem* eAnimations = root->find(NDHS_TP_XML_ANIMATIONS);
			LcCXmlElem* eAnimation = eAnimations ? eAnimations->getFirstChild() : NULL;
			for (; eAnimation && success; eAnimation = eAnimation->getNext())
			{
				if (eAnimation->getName().compareNoCase(NDHS_TP_XML_ANIMATION) != 0)
					continue;

				success = configureAnimationFromXml(eAnimation, stackLevel);
			}

			if (!success)
				break;

			//
			// Decorations Element
			//

			LcCXmlElem* eDecorations = root->find(NDHS_TP_XML_DECORATIONS);
			LcCXmlElem* rootNode = eDecorations ? eDecorations->getFirstChild() : NULL;
			for (; rootNode && success; rootNode = rootNode->getNext())
			{
				configureDecorationFromXml(rootNode);
				// Not critical if this fails, just go onto the next one
			}


			if (!success)
				break;
		} while (false);

		result = success;

		root.destroy();
	}


	if (result == false)
		cleanup();

	return result;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageTemplate::configureSettingsFromXml(LcCXmlElem* eSettings, const LcTmString& designSize)
{
	if (!eSettings)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Settings section missing");
		return false;
	}

	// Design Width
	LcTaString templateDesignSize = eSettings->getAttr(NDHS_TP_XML_DESIGN_SIZE);

	LcTaString templateType = eSettings->getAttr(NDHS_TP_XML_TEMPLATE_TYPE);
	m_templateType = determineTemplateType(templateType);

#ifdef LC_USE_LIGHTS
	m_lightModel = ENdhsLightModelNormal;
	LcTaString lightModel = eSettings->getAttr(NDHS_TP_XML_LIGHT_MODEL, "normal");
	if (lightModel.compareNoCase("simple") == 0)
	{
		m_lightModel = ENdhsLightModelSimple;
	}
#endif

	// The design size must be the same as the one specified in settings.xml
	if (templateDesignSize.compareNoCase(designSize) != 0)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Incompatible design size used for template");
		return false;
	}

	// Get drawLayerIndex
	m_drawLayerIndex = eSettings->getAttr(NDHS_TP_XML_DRAW_PLANE, NDHS_LOWER_INVALID_DRAW_LAYER_INDEX_STRING).toInt();

	m_cleanupLink = eSettings->getAttr(NDHS_TP_XML_CLEANUP_LINK);

	// Visibility element
	LcCXmlElem* eVisibility = eSettings->find(NDHS_TP_XML_VISIBILITY);
	if (eVisibility)
	{
		LcTaString parents = eVisibility->getAttr(NDHS_TP_XML_PARENTS).toLower();

		if (parents.compare(NDHS_TP_XML_HIDE_ALL_SHOW_ROOT) == 0)
		{
			m_parentVisibility = ENdhsParentVisibilityHideAllShowRoot;
		}
		else if (parents.compare(NDHS_TP_XML_HIDE_DIRECT_PARENT) == 0)
		{
			m_parentVisibility = ENdhsParentVisibilityHideDirectParent;
		}
		else if (parents.compare(NDHS_TP_XML_SHOW_DIRECT_PARENT) == 0)
		{
			m_parentVisibility = ENdhsParentVisibilityShowDirectParent;
		}
		else
		{
			// Default hideAllParents
			m_parentVisibility = ENdhsParentVisibilityHideAllParents;
		}

		m_showWhenInSub = LcCXmlAttr::strToBool(eVisibility->getAttr(NDHS_TP_XML_SHOW_WHEN_IN_SUB, "false"));
	}

	// Configure timing information
	configureTimingFromXML(eSettings->find(NDHS_TP_XML_TIMING));

	// Configure focus information
	configureFocusSettingsFromXML(eSettings->find(NDHS_TP_XML_FOCUS));

	// Configure variables now
	configureVariablesFromXML(eSettings->find(NDHS_TP_XML_VARIABLES));

	return true;
}
