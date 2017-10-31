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

#ifndef NdhsCLightElementH
#define NdhsCLightElementH

#ifdef LC_USE_LIGHTS

#include "inflexionui/engine/inc/NdhsCElement.h"

/*-------------------------------------------------------------------------*//**
	NdhsCLightElement is the class for secondary light elements
*/
class NdhsCLightElement : public NdhsCElement
{
private:

	// Two-phase construction
						NdhsCLightElement();
	void				construct(	ENdhsObjectType							use,
									const LcTmString&						className,
									NdhsCMenu*								menu,
									NdhsCMenuItem*							menuItem,
									NdhsCTemplate::CSecondaryLightElement* pLE,
									NdhsCPageManager*						pPageManager,
									NdhsCElementGroup*						pPage,
									int										stackLevel);

	LcTmOwner<NdhsCMenuWidgetFactory::CSettings>	m_settings;

public:
	virtual void					reloadElement();

	static LcTaOwner<NdhsCLightElement>
						create(		ENdhsObjectType							use,
									const LcTmString&						className,
									NdhsCMenu*								menu,
									NdhsCMenuItem*							menuItem,
									NdhsCTemplate::CSecondaryLightElement*	pLE,
									NdhsCPageManager*						pPageManager,
									NdhsCElementGroup*						pPage,
									int										stackLevel);


	virtual				~NdhsCLightElement();

#ifdef IFX_SERIALIZATION
	static	NdhsCLightElement*	loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual	SerializeHandle		serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual	void				deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */
};

#endif // LC_USE_LIGHTS

#endif // NdhsCDragRegionElementH
