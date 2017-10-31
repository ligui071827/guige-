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
#ifndef NdhsCPageTemplateH
#define NdhsCPageTemplateH

#include "inflexionui/engine/inc/LcTPlacement.h"
#include "inflexionui/engine/inc/NdhsCKeyFrameList.h"

class NdhsCElement;
class NdhsCTransitionAgent;
class NdhsCElementGroup;
class NdhsCPageManager;
class NdhsCPageModel;
class NdhsCMenu;
class NdhsCMenuItem;
class NdhsCMenuComponent;

bool triggerCompare(const NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTrigger* lhs, const NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTrigger* rhs);

/*-------------------------------------------------------------------------*//**
*/
class NdhsCPageTemplate : public NdhsCTemplate
{
public:

private:

	ENdhsParentVisibility				m_parentVisibility;
	bool								m_showWhenInSub;
	LcTmString							m_cleanupLink;

	LcTmString							m_templateFile;

	void								cleanup();
	ETemplateType						getTemplateType(){ return m_templateType; }

protected:

	// Allow only 2-phase construction
										NdhsCPageTemplate(NdhsCPageManager* pageManager, const LcTmString& templateFile);
										NdhsCPageTemplate(){}
public:
	// Creation/destruction
	static	LcTaOwner<NdhsCPageTemplate> create(NdhsCPageManager* pageManager, const LcTmString& templateFile);
	virtual								~NdhsCPageTemplate();

	inline ENdhsParentVisibility		getParentVisibility()				{ return m_parentVisibility; }
	inline bool							getShowWhenInSub()					{ return m_showWhenInSub; }
	inline LcTaString					getCleanupLink()					{ return m_cleanupLink; }
	inline int							getDrawLayerIndex()					{ return m_drawLayerIndex; }
	// Function to load XML into data structures
	LC_IMPORT bool						configureFromXml(const LcTmString& designSize, int stackLevel, int& nestedComponentLevel);
	bool 								configureSettingsFromXml(LcCXmlElem* eSettings, const LcTmString& designSize);

	NdhsCPageTemplate*					getTemplate() 						{return this; }

	LC_IMPORT bool 						configureDecorationTypesFromXml(LcCXmlElem* eInitialState, CLayoutDecoration* initialState);

};

#endif
