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


#ifdef LC_USE_LIGHTS

/*-------------------------------------------------------------------------*//**
	Static factory method
*/
LcTaOwner<NdhsCLightElement>
NdhsCLightElement::create(	ENdhsObjectType							use,
								const LcTmString&						className,
								NdhsCMenu*								menu,
								NdhsCMenuItem*							menuItem,
								NdhsCTemplate::CSecondaryLightElement*	pLE,
								NdhsCPageManager*						pPageManager,
								NdhsCElementGroup*						pPage,
								int										stackLevel)
{
	LcTaOwner<NdhsCLightElement> ref;
	ref.set(new NdhsCLightElement());
	ref->construct(use, className, menu, menuItem, pLE, pPageManager, pPage, stackLevel);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Constructor
*/
NdhsCLightElement::NdhsCLightElement()
{
}

/*-------------------------------------------------------------------------*//**
	Construction method - creates the widget and animator
*/
void NdhsCLightElement::construct(	ENdhsObjectType							use,
										const LcTmString&						className,
										NdhsCMenu*								menu,
										NdhsCMenuItem*							menuItem,
										NdhsCTemplate::CSecondaryLightElement*	pLE,
										NdhsCPageManager*						pPageManager,
										NdhsCElementGroup*						pPage,
										int										stackLevel)
{
	if (pLE)
	{
		// Lights are never tappable
		setTappable(EOff);

		// Let the base class initialize
		NdhsCElement::construct(use,
								ENdhsElementTypeLight,
								pLE->elementParent,
								className,
								NULL,
								NULL,
								menu,
								menuItem,
								pPageManager,
								pPage,
								stackLevel,
								pLE->m_drawLayerIndex,
								pLE->layoutTeleport,
								pLE->scrollTeleport,
								pLE->isDetail);

		m_settings = NdhsCMenuWidgetFactory::CSettings::create();

		m_settings->attenuationConstant	= pLE->attenuationConstant;
		m_settings->attenuationLinear		= pLE->attenuationLinear;
		m_settings->attenuationQuadratic	= pLE->attenuationQuadratic;
		m_settings->specularColor			= pLE->specularColor;
		m_settings->lightType				= pLE->lightType;
	}
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
NdhsCLightElement::~NdhsCLightElement()
{
}


void NdhsCLightElement::reloadElement()
{
	if (!getWidget())
	{
		// Recreate the widget
		LcTaOwner<LcCWidget> newWidget =
			NdhsCMenuWidgetFactory::createWidget(m_settings.ptr(),
								ENdhsWidgetTypeLight,
								getPackagePath(),
								"",
								getPageManager(),
								this,
								getMenu(),
								getLocalMenuItem(),
								getPage(),
								getStackLevel());

		setWidget(newWidget);
		getWidget()->setDrawLayerIndex(getDrawLayerIndex());
	}
}


#ifdef IFX_SERIALIZATION
NdhsCLightElement* NdhsCLightElement::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCLightElement *obj=new NdhsCLightElement();
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCLightElement::serialize(LcCSerializeMaster *serializeMaster,bool force)
{
	SerializeHandle handle=-1;
	if(!force)
	{
		handle=serializeMaster->getHandle(this);
		if(handle!=-1 && serializeMaster->isSerialized(handle))
		{
			return handle;
		}
		else if(handle==-1)
		{
			handle=serializeMaster->newHandle(this);
		}
	}
	else
	{
		handle=serializeMaster->newHandle(this);
	}

	int outputSize = sizeof(NdhsCLightElement) - sizeof(NdhsCElement)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCElement::serialize(serializeMaster,true);
	ENdhsElementType dataType=ENdhsElementTypeLight;
	SERIALIZE(dataType,serializeMaster,cPtr);
	SERIALIZE(parentHandle,serializeMaster,cPtr);
	SERIALIZE_Owner(m_settings,serializeMaster,cPtr);

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCLightElement::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{

	int dummy=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;

	DESERIALIZE(dummy,serializeMaster,cPtr);
	DESERIALIZE(parentHandle,serializeMaster,cPtr);
	NdhsCElement::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE_Owner(m_settings,serializeMaster,cPtr,NdhsCMenuWidgetFactory::CSettings)

}
#endif /* IFX_SERIALIZATION */

#endif // LC_USE_LIGHTS
