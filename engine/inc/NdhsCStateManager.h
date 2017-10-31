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

#ifndef NdhsCStateManagerH
#define NdhsCStateManagerH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTString.h"

class NdhsCExpression;

class NdhsCStateManager : public LcCBase, public NdhsIExpressionObserver, public NdhsILaundryItem
{
public:
	class IObserver
	{
	public:
		virtual void		layoutChanged(NdhsCTemplate::CLayout* newLayout,bool animate) = 0;
		virtual void		doAction(NdhsCTemplate::CAction* action, int slotNum = -1) = 0;
		virtual void		stateManagerDestroyed(NdhsCStateManager* sm) = 0;
	};

private:
	class CConditionBlock : public LcCBase
	{
	protected:
		CConditionBlock() {}

	public:
		NdhsCExpression*					guardExpr;
		LcTmMap<LcTmString, NdhsCExpression*>	assignments;
		NdhsCPageTemplate::CAction*			action;

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

	class CSlotClassTrigger : public LcCBase
	{	
	protected:
		CSlotClassTrigger () {}

	public:
		int						slot;
		LcTmString				elementClass;
		LcTmOwner<CEventInfo>	eventInfo;

		static LcTaOwner<CSlotClassTrigger>		create();
		virtual ~CSlotClassTrigger()			{}
	};
	typedef LcTmOwnerArray<CSlotClassTrigger>	TmASlotClassTriggerList;


	class CState : public LcCBase
	{
	protected:
		CState() {}

	public:
		LcTmOwnerMap<int, CEventInfo>			pressTriggers;
		LcTmOwnerMap<int, CEventInfo>			tapSlotTriggers;
		LcTmOwnerMap<LcTmString, CEventInfo>	tapClassTriggers;
		LcTmOwnerMap<int, CEventInfo>			signalSlotTriggers;
		LcTmOwnerMap<LcTmString, CEventInfo>	signalClassTriggers;
		LcTmOwner<CEventInfo>					catchAllKeyPress;
		LcTmOwner<CEventInfo>					catchAllStylusTap;
		TmASlotClassTriggerList					signalSlotClassTriggers;


		LcTmOwner<NdhsCExpression>				condition;
		NdhsCTemplate::CLayout*					layout;

		static LcTaOwner<CState>				create();

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

	LcTmOwnerArray<CState>			m_states;
	CState*							m_currentState;

#if defined(NDHS_JNI_INTERFACE)
	LcTmOwner<NdhsCPageManager::CStaticPreviewCache>
									m_snapShot;
	bool							m_allowLayoutChange;
#endif

	NdhsCPageManager*				m_pageManager;

	LcTmArray<IObserver*>			m_observers;

	NdhsCLaundry* 					m_laundry;
	bool							m_dirty;
	bool							m_stillDirty;
	bool							m_testingLayoutConditions;

	NdhsIFieldContext* 				m_fieldContext;

	bool							findObserver(IObserver* obs);

protected:
									NdhsCStateManager(NdhsCLaundry* laundry, NdhsCPageManager* pageManager)
										: m_pageManager(pageManager),
										m_laundry(laundry),
										m_dirty(false),
										m_fieldContext(NULL)
#if defined(NDHS_JNI_INTERFACE)
										,m_allowLayoutChange(true)
#endif
										{}

	void 							construct(NdhsCTemplate* templ, NdhsIFieldContext* context);
	LcTaOwner<CState> 				constructStateInfo(NdhsCPageTemplate::CState* baseStateInfo, NdhsIFieldContext* context);
	LcTaOwner<CEventInfo> 			constructEventInfo(NdhsCPageTemplate::CEventInfo* baseEventInfo, NdhsIFieldContext* context);
	LcTaOwner<CConditionBlock> 		constructConditionBlock(NdhsCPageTemplate::CConditionBlock* baseConditionBlock, NdhsIFieldContext* context);


	bool 						processEventInfo(CEventInfo* eventInfo, int slotNum);


	void						broadcastAction(NdhsCPageTemplate::CAction*	action, int slotNum);
	void						broadcastLayoutChange(NdhsCTemplate::CLayout* newLayout,bool animate);
	void 						broadcastStateManagerDestoyed();

	// NdhsILaundryItem
	virtual bool 				cleanLaundryItem(LcTTime timestamp);
	virtual bool 				stillDirty()			{ return m_stillDirty; }
	virtual void 				addedToLaundry() 		{ m_stillDirty = false; }

public:
	void 						testLayoutConditions(bool force=false);
	static LcTaOwner<NdhsCStateManager>	create(NdhsCLaundry* laundry, NdhsCTemplate* templ, NdhsIFieldContext* context, NdhsCPageManager* pageManager);

	/* Handle events; all return "consumed" */
	bool						processKeypress(int scancode);
	bool 						processSlotTap(int slot,const LcTmString& className);
	bool						processClassTap(const LcTmString& className);
	bool						processClassSignal(const LcTmString& className);
	bool						processSlotSignal(int slot);
	bool						processSlotClassNameSignal(int slot, const LcTmString& className);

	NdhsCTemplate::CLayout* 	getCurrentLayout()			{ return m_currentState ? m_currentState->layout :  NULL; }

	bool						isCatchAll();
	void						stopObservingFields();

	// Observer support
	void						addObserver(IObserver* obs);
	void						removeObserver(IObserver* obs);

	// NdhsIExpressionObserver implementation
	virtual void 				expressionDirty(NdhsCExpression* expr);
	virtual void 				expressionDestroyed(NdhsCExpression* expr);

#if defined(NDHS_JNI_INTERFACE)
	void						forceLayout(LcTmString layout);
	void						setAggregateSnapShotInfo(LcTmOwner<NdhsCPageManager::CStaticPreviewCache> info);
	void						allowLayoutChange(bool layoutChange) { m_allowLayoutChange = layoutChange; }
#endif //defined(NDHS_JNI_INTERFACE)

	ENdhsPageState				getStateInfo();
	virtual 					~NdhsCStateManager();
#ifdef IFX_SERIALIZATION
		ISerializeable*			getSerializeAble(int &type){type=-1; return NULL;};
#endif /* IFX_SERIALIZATION */
};

#endif // NdhsCStateManagerH
