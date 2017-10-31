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
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


/*-------------------------------------------------------------------------*//**
	Constructor
*/
NdhsCLaundry::NdhsCLaundry(NdhsCMenuCon* menuCon)
:	m_menuCon(menuCon),
	m_clearRemovedItems(true),
	m_cleaning(false)
{
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCLaundry> NdhsCLaundry::create(NdhsCMenuCon* menuCon)
{
	LcTaOwner<NdhsCLaundry> ref;
	ref.set(new NdhsCLaundry(menuCon));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCLaundry::construct()
{
	LcTaOwner<LcCMutex> newMutex = LcCMutex::create("Laundry");
	m_itemsLock = newMutex;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCLaundry::addItem(NdhsILaundryItem* item)
{
	if (m_itemsLock->lock())
	{
		if (m_cleaning)
		{
			removeFromList(item, m_removedItems);
		}

		addToList(item, m_items);

		// Reschedule a frame update in case nothing else does
		if (m_menuCon)
		{
			NdhsCPageManager* pageManager = m_menuCon->getPageManager();

			if (pageManager)
			{
				pageManager->startAnimation();
			}
		}

		m_itemsLock->unlock();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCLaundry::removeItem(NdhsILaundryItem* item)
{
	if (m_itemsLock->lock())
	{
		if (m_cleaning)
		{
			addToList(item, m_removedItems);
		}

		removeFromList(item, m_items);

		m_itemsLock->unlock();
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCLaundry::cleanAll(bool forceClean)
{
	bool reschedule = false;

	if (m_menuCon->getSpace() && (!m_cleaning || forceClean))
	{
		LcTTime timestamp = m_menuCon->getSpace()->getTimestamp();
		bool clearRemovedItems = false;

		// Make a copy of the item list
		LcTaArray<NdhsILaundryItem*> itemListCopy;
		if (m_itemsLock->lock())
		{
			itemListCopy.assign(m_items.begin(), m_items.end());
			m_items.clear();
			m_itemsLock->unlock();
		}

		if(m_clearRemovedItems)
		{
			clearRemovedItems = true;
			m_clearRemovedItems = false;
			m_cleaning = true;
		}

		// Clean each item in list
		LcTmArray<NdhsILaundryItem*>::iterator it = itemListCopy.begin();
		for (;it != itemListCopy.end(); it++)
		{
			NdhsILaundryItem* item = (*it);

			if (item && !isInList(item, m_removedItems))
			{
				reschedule |= item->cleanLaundryItem(timestamp);
			}
		}

		// Give each item a chance to be re-added
		for (it = itemListCopy.begin(); it != itemListCopy.end(); it++)
		{
			NdhsILaundryItem* item = (*it);

			if (item && !isInList(item, m_removedItems))
			{
				if (item->stillDirty())
				{
					if (m_itemsLock->lock())
					{
						addToList(item, m_items);
						m_itemsLock->unlock();
					}

					item->addedToLaundry();
				}
			}
		}

		// All done
		if (m_itemsLock->lock())
		{
			// For recursive calls to cleanAll we don't want to clear 
			// removed item with each call do it only for first one!
			if (clearRemovedItems)
			{
				m_removedItems.clear();
				m_clearRemovedItems = true;
				m_cleaning = false;
			}

			m_itemsLock->unlock();
		}
	}

	return reschedule;
}

bool NdhsCLaundry::isInList(NdhsILaundryItem* item, LcTmArray<NdhsILaundryItem*>& itemList)
{
	bool found = false;

	if (item)
	{
		LcTmArray<NdhsILaundryItem*>::iterator listIt = itemList.begin();
		for (; listIt != itemList.end() && !found; listIt++)
		{
			if ((*listIt) == item)
			{
				found = true;
			}
		}
	}

	return found;
}

void NdhsCLaundry::removeFromList(NdhsILaundryItem* item, LcTmArray<NdhsILaundryItem*>& itemList)
{
	if (item)
	{
		LcTmArray<NdhsILaundryItem*>::iterator it = itemList.begin();

		while (it != itemList.end())
		{
			if ((*it) == item)
			{
				it = itemList.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}

void NdhsCLaundry::addToList(NdhsILaundryItem* item, LcTmArray<NdhsILaundryItem*>& itemList)
{
	if (item && !isInList(item, itemList))
	{
		itemList.push_back(item);
	}
}

