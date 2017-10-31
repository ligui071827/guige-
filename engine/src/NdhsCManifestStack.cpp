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
	Creation
*/
LcTaOwner<NdhsCManifestStack> NdhsCManifestStack::create(NdhsCPageManager* parentPageManager)
{
	LcTaOwner<NdhsCManifestStack> ref;
	ref.set(new NdhsCManifestStack());
	ref->construct(parentPageManager);
	return ref;
}
#ifdef IFX_SERIALIZATION
void NdhsCManifestStack::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	int count=0;
	SerializeHandle h=-1;
	LcTaString manifest="";
	LcTaString package="";
	DESERIALIZE_Reserve(m_parentPageManager,serializeMaster,cPtr,NdhsCPageManager)
	DESERIALIZE(count,serializeMaster,cPtr)
	NdhsCManifest * currentManifest = m_manifestStack[0];
	DESERIALIZE(h,serializeMaster,cPtr);
	currentManifest->deSerialize(h,serializeMaster);

	for(int i=1;i<count;++i)
	{
		DESERIALIZE(h,serializeMaster,cPtr)
		currentManifest=(NdhsCManifest*)serializeMaster->getPointer(h);
		if(currentManifest==NULL)
		{
			currentManifest=NdhsCManifest::loadState(h,serializeMaster);
		}
		if(currentManifest!=NULL)
		{
			LcTaOwner<NdhsCManifest> ref;
			ref.set(currentManifest);
			m_manifestStack.push_back(ref);
		}
	}
}
SerializeHandle NdhsCManifestStack::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCManifestStack) +sizeof(IFX_INT32)*2+sizeof(SerializeHandle)*m_manifestStack.size();
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SERIALIZE_Ptr(m_parentPageManager,serializeMaster,cPtr);

	int count=m_manifestStack.size();
	SERIALIZE(count,serializeMaster,cPtr);
	for(TmAManifestStack::iterator iter=m_manifestStack.begin();iter!=m_manifestStack.end();iter++)
	{
		SERIALIZE_Reserve(*iter,serializeMaster,cPtr);
	}
	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}


#endif /* IFX_SERIALIZATION */
/*-------------------------------------------------------------------------*//**
	Construction
*/
void NdhsCManifestStack::construct(NdhsCPageManager* parentPageManager)
{
	m_parentPageManager = parentPageManager;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCManifestStack::~NdhsCManifestStack()
{
	m_manifestStack.clear();
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCManifestStack::pushManifest(const LcTmString& manifest, const LcTmString& packageName)
{
	LcTaOwner<NdhsCManifest> man = NdhsCManifest::create(manifest, packageName);

	man->loadManifest();

	if (manifestAlreadyExists(man.ptr()) == false)
		m_manifestStack.push_back(man);

	// Doesn't matter if it loads, it must still go on the stack
	// to keep in sync with the token stack
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCManifestStack::manifestAlreadyExists(NdhsCManifest* man)
{
	LcTmOwnerArray<NdhsCManifest>::iterator it = m_manifestStack.begin();
	for(; it != m_manifestStack.end(); it++)
	{
		NdhsCManifest* manifest = *it;
		if (manifest->getManifestFile().compareNoCase(man->getManifestFile()) == 0 && manifest->getManifestPath().compareNoCase(man->getManifestPath()) == 0)
			return true;
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCManifestStack::popManifest(int count)
{
	if (count > (int)m_manifestStack.size())
		count = (int)m_manifestStack.size();

	for (int i = 0; i < count; i++)
	{
		// Iterate the palettes and unload the ones listed.
		NdhsCManifest* oldManifest = m_manifestStack.back();
		if (oldManifest)
			unloadLinkedPalettes(oldManifest);

		// Remove stack.
		m_manifestStack.pop_back();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCManifestStack::unloadLinkedPalettes(NdhsCManifest* manifest)
{
	NdhsCManifest::TmAPaletteList* linkPal = manifest->getLinkedPalettes();
	NdhsCManifest::TmAPaletteList::iterator palIter = linkPal->begin();
	for (; palIter != linkPal->end(); ++palIter)
	{
		NdhsCManifest* palMan = *palIter;
		m_parentPageManager->closePalette(palMan);
	}
}

/*-------------------------------------------------------------------------*//**
	Pops the stack leaving the supplied level at the top of the stack
*/
void NdhsCManifestStack::popManifestToLevel(int level)
{
	if (level >= (int)m_manifestStack.size())
		return;

	// Calculate how many items to pop
	int count = ((int)m_manifestStack.size() - level) - 1;

	popManifest(count);
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCManifest* NdhsCManifestStack::getManifest(int stackLevel)
{
	if (stackLevel >= (int)m_manifestStack.size())
		return NULL;

    if (stackLevel < 0)
		return NULL;

	return m_manifestStack[stackLevel];
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCManifestStack::findFile(const LcTmString&	file,
									LcTmString&		outPath,
									NdhsCManifest*	paletteManifest,
									int				stackLevel,
									int*			outStackLevel,
									bool			wildcard,
									LcTaArray<NdhsCManifest::CManifestFile*> *fileData)
{
	LcTaString outFile;
	LcTaString paletteFile = file;
	bool manifestFound = false;
	bool paletteOnlyManifest = false;

	NdhsCManifest* manifest = paletteManifest;

	if (file.isEmpty())
		return false;

	// Check if the path includes the palettes path.
	if (file.subString(0, NDHS_PALETTE_DIR_LENGTH).compareNoCase(NDHS_PALETTE_DIR) == 0)
	{
		// This path can only be for a palette.
		paletteOnlyManifest = true;

		// Remove the palette name.
		int paletteNamePos = file.find(NDHS_DIR_SEP, NDHS_PALETTE_DIR_LENGTH + 2);
		paletteFile = file.tail(paletteNamePos + 1);
	}

	// First if the there is a palette check that.
	if (manifest)
	{
		// Find the palette path.
		if (manifest->fileExists(paletteFile, outFile,wildcard,fileData))
			manifestFound = true;
	}
	if ((!manifestFound || wildcard) && !paletteOnlyManifest)
	{
		// Make sure we don't overflow
		if (stackLevel >= (int)m_manifestStack.size())
			stackLevel = (int)m_manifestStack.size() - 1;

		// Check the manifest stack.
		for (int i = stackLevel; i >= 0 && (!manifestFound || wildcard); i--)
		{
			manifest = m_manifestStack[i];
			if (manifest && manifest->fileExists(file, outFile,wildcard,fileData))
			{
				if (outStackLevel)
					*outStackLevel = i;

				manifestFound = true;
			}
		}
	}

	if (manifestFound)
	{
		// Use the manifest spelling as the return value and set the
		// directory slash separators to the non default if required.
		outPath = manifest->getManifestPath() + outFile;
		#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
			outPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
		#endif

		return true;
	}

	return false;
}
