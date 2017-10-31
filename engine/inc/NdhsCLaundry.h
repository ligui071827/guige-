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

#ifndef NdhsCLaundryH
#define NdhsCLaundryH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTString.h"

class NdhsILaundryItem
{
public:
	virtual bool	cleanLaundryItem(LcTTime timestamp) = 0;

	// Override these if the item may want to be re-added after cleaning
	virtual bool 	stillDirty()			{ return false; }
	virtual void 	addedToLaundry() 		{ }
};

class NdhsCLaundry : public LcCBase
{
private:
	NdhsCMenuCon*						m_menuCon;

	LcTmOwner<LcCMutex> 				m_itemsLock;

	LcTmArray<NdhsILaundryItem*>		m_items;
	LcTmArray<NdhsILaundryItem*>		m_removedItems;
	bool								m_clearRemovedItems;
	bool								m_cleaning;

	bool								isInList(NdhsILaundryItem* item, LcTmArray<NdhsILaundryItem*>& itemList);
	void								removeFromList(NdhsILaundryItem* item, LcTmArray<NdhsILaundryItem*>& itemList);
	void								addToList(NdhsILaundryItem* item, LcTmArray<NdhsILaundryItem*>& itemList);

protected:
			NdhsCLaundry(NdhsCMenuCon* menuCon);

	void	construct();

public:
	static LcTaOwner<NdhsCLaundry>		create(NdhsCMenuCon* menuCon);

	void	addItem(NdhsILaundryItem* item);
	void	removeItem(NdhsILaundryItem* item);

	bool	cleanAll(bool forceClean = false);

	bool	isEmpty()					{ return m_items.empty(); }
};

#endif // NdhsCLaundryH
