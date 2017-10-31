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
#ifndef NdhsCManifestStackH
#define NdhsCManifestStackH

class NdhsCManifest;

#include "inflexionui/engine/inc/LcCSerializeMaster.h"

class NdhsCManifestStack : public LcCBase,public ISerializeable
{
private:

	typedef LcTmOwnerArray<NdhsCManifest> TmAManifestStack;
	TmAManifestStack			m_manifestStack;

	NdhsCPageManager*			m_parentPageManager;

	// Two-phase construction
							NdhsCManifestStack() {}
	void					construct(NdhsCPageManager* parentPageManager);

	void					unloadLinkedPalettes(NdhsCManifest* manifest);

public:

#ifdef IFX_SERIALIZATION
	void					deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual SerializeHandle serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	bool					isMenuItemChild(){return false;}
#endif /* IFX_SERIALIZATION */
	// Create/destruct
	static LcTaOwner<NdhsCManifestStack> create(NdhsCPageManager* parentPageManager);

	virtual 				~NdhsCManifestStack();

	bool					pushManifest(const LcTmString& manifest, const LcTmString& packageName);
	void					popManifest(int count = 1);
	void					popManifestToLevel(int level);
	bool					manifestAlreadyExists(NdhsCManifest* man);

	NdhsCManifest*			getManifest(int stackLevel);

	int						getStackHeight()	{ return (int)m_manifestStack.size(); }

	bool					findFile(const LcTmString& file, LcTmString& outPath, NdhsCManifest* paletteManifest, int stackLevel,int* outStackLevel = NULL,bool wildcard=false,LcTaArray<NdhsCManifest::CManifestFile*> *fileData=0);
};

#endif //NdhsCManifestStackH
