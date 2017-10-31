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
#ifndef NdhsCMenuComponentTemplateH
#define NdhsCMenuComponentTemplateH

#include "inflexionui/engine/inc/LcTPlacement.h"
#include "inflexionui/engine/inc/NdhsCKeyFrameList.h"
#include "inflexionui/engine/inc/NdhsCTemplate.h"

class NdhsCElement;
class NdhsCTransitionAgent;
class NdhsCElementGroup;
class NdhsCPageManager;
class NdhsCPageModel;
class NdhsCMenu;
class NdhsCMenuItem;
class NdhsCMenuComponent;
class NdhsCComponent;

/*-------------------------------------------------------------------------*//**
*/
class NdhsCMenuComponentTemplate : public NdhsCTemplate
{
public:

	//
	// XML Template Data Storage Classes
	//

	//
	// Action class.
	//
	class CMenuAction : public CAction
	{
	public:

		class CJumpBy : public CAttempt
		{
		protected:
			CJumpBy()							{}

		public:
			int									amount;
			bool								wrap;
			LcTmString							action;
			ENdhsVelocityProfile				velocityProfile;
			int									duration;

			static LcTaOwner<CJumpBy>			create();
			virtual ~CJumpBy()					{}
		};

		class CJumpTo : public CAttempt
		{
		protected:
			CJumpTo()							{}

		public:
			int									slot;
			LcTmString							action;
			ENdhsVelocityProfile				velocityProfile;
			int									duration;

			static LcTaOwner<CJumpTo>			create();
			virtual ~CJumpTo()					{}
		};

		class CSelect : public CAttempt
		{
		protected:
			CSelect()							{}

		public:
			int									slot;

			static LcTaOwner<CSelect>			create();
			virtual ~CSelect()					{}
		};

		class CDeactivate : public CAttempt
		{
		protected:
			CDeactivate()						{}

		public:
			LcTmString							action;
			ENdhsVelocityProfile				velocityProfile;
			int									duration;

			static LcTaOwner<CDeactivate>		create();
			virtual ~CDeactivate()				{}
		};

	protected:
		CMenuAction()							{}

	public:
		static LcTaOwner<CMenuAction>			create();
		virtual ~CMenuAction()					{}
	};

private:

	TmMExprSkeleton						m_dataSource;

	//
	// XML Template Data
	//

	LcTmOwnerArray<CAction::CAttempt>	m_defaultSelectAttempt;
	LcTmOwnerArray<CAction::CAttempt>	m_defaultBackAttempt;

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

	void								cleanup();

	// Functions to load XML into data structures
	bool								configureSettingsFromXml(LcCXmlElem* eSettings);
	bool								configureActionFromXml(LcCXmlElem* eAction);
	virtual void						configureAttemptFromXml(LcCXmlElem* eAttempt, CAction* action);
	bool								configureDefaultActions();

	void								setDefaultValues(CLayout& layout);

protected:

	LC_EXPORT bool configureStaticDecorationFromXml(LcCXmlElem* decorationRoot);
	LC_EXPORT bool configureStaticDecorationTypesFromXml(LcCXmlElem* eInitialState, CLayoutDecoration* initialState);

	// Allow only 2-phase construction
										NdhsCMenuComponentTemplate(NdhsCPageManager* pageManager, LcTmOwner<LcCXmlElem>& root);

public:
	// Creation/destruction
	static	LcTaOwner<NdhsCMenuComponentTemplate> create(NdhsCPageManager* pageManager, LcTmOwner<LcCXmlElem>& root);
	virtual								~NdhsCMenuComponentTemplate();
	virtual bool 						isOuterGroup(const LcTaString& elementName);

	inline int							getDefaultTerminalTime()			{ return m_defaultTerminalTime; }
	inline ENdhsVelocityProfile			getDefaultTerminalVelocityProfile()	{ return m_defaultTerminalVelocityProfile; }

	inline int							getSlotCount()						{ return m_slotCount; }
	inline int							getFirstActiveSlot()				{ return m_firstActive; }
	inline int							getFirstSelectableSlot()			{ return m_firstSelectable; }
	inline int							getLastSelectableSlot()				{ return m_lastSelectable; }
	inline int							getFirstPopulateSlot()				{ return m_populateFrom; }
	inline bool							fullMode()							{ return m_fullMode; }
	inline bool							scrollWrap()						{ return m_scrollWrap; }
	inline int							scrollSpan()						{ return m_scrollSpan; }
	inline LcTScalar                    scrollRebound()						{ return m_scrollRebound; }
	inline LcTScalar					scrollKickDeceleration()			{ return m_scrollKickDeceleration; }
	inline LcTScalar					scrollKickMaxVelocity()				{ return m_scrollKickMaxVelocity; }

	// Function to load XML into data structures
	LC_IMPORT bool						configureFromXml(const LcTmString& designSize, int stackLevel, int& nestedComponentLevel );

	int			 						getSlotElementCount()				{ return (int)(m_itemClasses.size()); }
	NdhsCMenuComponentTemplate*			getTemplate() { return this; }
	ETemplateType						getTemplateType(){ return m_templateType; }
	NdhsCExpression::CExprSkeleton*		getDataSource() { return m_dataSource.ptr(); }
};

#endif
