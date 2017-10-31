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


#if defined( NDHS_PREVIEWER ) || defined( NDHS_JNI_INTERFACE )
#define MANIFEST_ROOT_NAME					"uiProject"
#else
#define MANIFEST_ROOT_NAME					NDHS_MANIFEST_ROOT_NAME
#endif

/*-------------------------------------------------------------------------*//**
	Creation
*/

LcTaOwner<NdhsCManifest> NdhsCManifest::create(const LcTmString& manifest, const LcTmString& packageName)
{
	LcTaOwner<NdhsCManifest> ref;
	ref.set(new NdhsCManifest(manifest, packageName));
	ref->construct();
	return ref;
}

#ifdef IFX_SERIALIZATION
NdhsCManifest* NdhsCManifest::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	ISerializeable *ptr =serializeMaster->getPointer(handle);
	if(ptr==NULL)
	{
		void * cPtr=serializeMaster->getOffset(handle);
		LcTaString manifest="";
		LcTaString package="";
		DESERIALIZE_String(manifest,serializeMaster,cPtr)
		DESERIALIZE_String(package,serializeMaster,cPtr)

		LcTaOwner<NdhsCManifest> ref=NdhsCManifest::create(manifest,package);
		ref->deSerialize(handle,serializeMaster);
		ref->loadManifest();
		return ref.release();
	}
	return (NdhsCManifest*)ptr;
}

void NdhsCManifest::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	LcTaString manifest="";
	LcTaString package="";
	DESERIALIZE_String(manifest,serializeMaster,cPtr)
	DESERIALIZE_String(package,serializeMaster,cPtr)

	int count=0;
	NdhsCManifest *iPtr=NULL;
	DESERIALIZE(count,serializeMaster,cPtr);

	for(int i=0;i<count;++i)
	{
		DESERIALIZE_Ptr(iPtr,serializeMaster,cPtr,NdhsCManifest);
		if(iPtr!=NULL)
		{
			m_linkedPalettes.push_back((NdhsCManifest*)iPtr);
		}
	}
}
SerializeHandle NdhsCManifest::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCManifest) +sizeof(IFX_INT32)*2+sizeof(SerializeHandle)*m_linkedPalettes.size();
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SERIALIZE_String(m_manifestFile,serializeMaster,cPtr);
	SERIALIZE_String(m_packageName,serializeMaster,cPtr);
	int count=m_linkedPalettes.size();
	SERIALIZE(count,serializeMaster,cPtr);

	for(TmAPaletteList::iterator iter=m_linkedPalettes.begin();iter!=m_linkedPalettes.end();iter++)
	{
		SERIALIZE_Ptr(*iter,serializeMaster,cPtr);
	}
	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}

#endif /* IFX_SERIALIZATION */
/*-------------------------------------------------------------------------*//**
*/
NdhsCManifest::NdhsCManifest(const LcTmString& manifest, const LcTmString& packageName)
{
	m_linkedPalettes.clear();

	m_manifestFile = manifest;
	m_packageName = packageName;

	// Store the Project path
	m_manifestPath = manifest;
	int offset = m_manifestPath.findLastChar(NDHS_DIR_SEP_CHAR);
	if (offset == LcTmString::npos)
	{
		// No path
		m_manifestPath = "";
	}
	else
	{
		m_manifestPath = m_manifestPath.subString(0, offset + 1);
	}


}

/*-------------------------------------------------------------------------*//**
	Construction
*/
void NdhsCManifest::construct()
{
}


/*-------------------------------------------------------------------------*//**
*/
NdhsCManifest::~NdhsCManifest()
{
	m_manifestFiles.clear();
}

/*-------------------------------------------------------------------------*//**
	This function only loads the manifest header for platforms that are
	not interested in the file signing, and only want to do a compatibility
	check.
*/
bool NdhsCManifest::loadManifestHeader()
{
	// Load project from XML
	LcTaString err;
	// Set the directory slash separators to the non default if required.
	LcTaString localPath = m_manifestFile;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif
	LcTaOwner<LcCXmlElem> eRoot = LcCXmlElem::load(localPath, err);

	if (eRoot)
	{
		if (eRoot->getName().compareNoCase(MANIFEST_ROOT_NAME) == 0)
		{
			// Signature File
			m_signatureFile = eRoot->getAttr(NDHS_TP_XML_SIGNATURE);

			LcTaString path = m_manifestPath + m_signatureFile;

#ifndef NDHS_JNI_INTERFACE
			// load the signature
			LcTaOwner<LcCReadOnlyFile> file = LcCReadOnlyFile::openFile(path.bufUtf8());
			if (file)
			{
				for (int i = 0; i < IFX_HASH_SIZE; i++)
				{
					if(file->read(&m_signature.hash[i], sizeof(IFX_UINT32), 1) != 1)
						break;
				}
				file->close();
			}
#endif

			// Compatibility
			m_compatibility = eRoot->getAttr(NDHS_TP_XML_COMPATIBILITY);

			return true;
		}
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCManifest::loadManifest(bool shaderManifest)
{
	bool retVal = false;

	// Load project from XML
	LcTaString err;
	LcTaString prefixPath = getManifestPath();
	// Set the directory slash separators to the non default if required.
	LcTaString localPath = m_manifestFile;
	LcTaString manifestRoot;
	LcTaOwner<LcCXmlElem> eRoot;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif

	m_manifestFiles.clear();

	if (shaderManifest)
	{
		eRoot = LcCXmlElem::load(localPath, err, EShader);
		manifestRoot = NDHS_MANIFEST_ROOT_NAME;
	}
	else
	{
		eRoot = LcCXmlElem::load(localPath, err);
		manifestRoot = MANIFEST_ROOT_NAME;
	}

	if (eRoot)
	{
		if (eRoot->getName().compareNoCase(manifestRoot) == 0)
		{
			// Signature File
			m_signatureFile = eRoot->getAttr(NDHS_TP_XML_SIGNATURE);

			LcTaString path = m_manifestPath + m_signatureFile;

#ifndef NDHS_JNI_INTERFACE
			// load the signature
			LcTaOwner<LcCReadOnlyFile> file = LcCReadOnlyFile::openFile(path.bufUtf8());
			if (file)
			{
				for (int i = 0; i < IFX_HASH_SIZE; i++)
				{
					if(file->read(&m_signature.hash[i], sizeof(IFX_UINT32), 1) != 1)
						break;
				}
				file->close();
			}
#endif			

			// Compatibility
			m_compatibility = eRoot->getAttr(NDHS_TP_XML_COMPATIBILITY);

			LcCXmlElem* eTemp = NULL;

#if defined(NDHS_PREVIEWER) || defined(NDHS_JNI_INTERFACE)
			if (shaderManifest == false)
			{
				LcCXmlElem* eProjectFiles = eRoot->find(NDHS_TP_XML_PROJECT_FILES);
				if (eProjectFiles)
				{
					LcCXmlElem* eFileSet = eProjectFiles->getFirstChild();
					for (; eFileSet != NULL; eFileSet = eFileSet->getNext())
					{
						if (eFileSet->getName().compareNoCase(NDHS_TP_XML_FILE_SET) != 0)
							continue;

						eTemp = eFileSet;
					}
				}
			}
			else
			{
				eTemp = eRoot.ptr();
			}
#else
			eTemp = eRoot.ptr();
#endif
			if (eTemp)
			{
				// Iterate through the files
				LcCXmlElem* eFile = eTemp->getFirstChild();

				for (; eFile != NULL; eFile = eFile->getNext())
				{
#if !defined(IFX_WIN_PLAYER)
					if (eFile->getName().compareNoCase(NDHS_TP_XML_MANIFEST_FILE) == 0)
#endif
					{
						LcTaString value="";
						CManifestFile mf;
						mf.path = eFile->getAttr(NDHS_TP_XML_MANIFEST_PATH);
						value=eFile->getAttr(NDHS_TP_XML_MARGIN_BOTTOM);
						mf.m_marginBottom = value.toInt();
						value=eFile->getAttr(NDHS_TP_XML_MARGIN_LEFT);
						mf.m_marginLeft = value.toInt();
						value=eFile->getAttr(NDHS_TP_XML_MARGIN_RIGHT);
						mf.m_marginRight = value.toInt();
						value=eFile->getAttr(NDHS_TP_XML_MARGIN_TOP);
						mf.m_marginTop = value.toInt();
						value=eFile->getAttr(NDHS_TP_XML_FRAME_COUNT, "1");
						mf.m_frameCount = value.toInt()==0?1:value.toInt();
						value=eFile->getAttr(NDHS_TP_XML_COMPILED_SHADER_FORMAT);
						mf.m_compiledShaderFormat = value.toInt();
						mf.absolutePath = prefixPath + mf.path;
						#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
							mf.absolutePath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
						#endif

						mf.m_signature = m_signature;
						m_manifestFiles.push_back(mf);

					}
				}

				// Record success status
				retVal = true;
			}

			IFX_ShellSort(m_manifestFiles.begin(),m_manifestFiles.end());
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
 * 	Find index of the file in manifest
*/
int NdhsCManifest::findIndex(int a, int b, LcTmString &file, bool wildcard)
{
	int mid = 0;
	int comp = 0;
	int n;

	if (wildcard)
	{
		n = file.findLastChar('.');

		// If file name doesn't contain a ., there's no wildcard so use whole string.
		if (n < 0)
			n = file.length();
		else
			n++; // Include the '.' in the wildcard search to avoid substring matches!
	}
	else
	{
		n = file.length();
	}

	while (a <= b)
	{
		// Binary chop on each iteration
		mid = (a + b) / 2;
		comp = m_manifestFiles[mid].path.compareNoCaseN(file, n);
		if (comp == 0)
		{
			return mid;
		}
		else if (comp < 0)
		{
			// move start index
			a = mid + 1;
		}
		else
		{
			// move end index
			b = mid - 1;
		}
	}
	return -1;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCManifest::fileExists(const LcTmString& file, LcTmString& returnFilename, bool wildcard, LcTaArray<NdhsCManifest::CManifestFile*> *fileData)
{
	int count = m_manifestFiles.size();

	if (count == 0)
		return false;

	bool matchFound = false;
	LcTaString filename = file;
	int mid = findIndex(0, count - 1, filename, wildcard);

	if (mid == -1)
		return false;

	int n;

	if (wildcard)
	{
		// Include the '.' in the wildcard search to avoid substring matches!
		n = filename.findLastChar('.') + 1;
	}
	else
	{
		// We have a match.
		matchFound = true;
		returnFilename = m_manifestFiles[mid].path;
		if(fileData != NULL)
		{
			fileData->push_back(&m_manifestFiles[mid]);
		}
		return matchFound;
	}

	for (int i = mid-1; i >= 0; --i)				// This will work linear mode
	{
		if (m_manifestFiles[i].path.compareNoCaseN(filename, n) == 0)
		{
			matchFound = true;
			returnFilename = m_manifestFiles[i].path;
			if (fileData != NULL)
			{
				fileData->push_back(&m_manifestFiles[i]);
			}

			if (!wildcard)
				return matchFound;
		}
		else
		{
			break;
		}
	}

	for (int i = mid; i < count; ++i)			// This will not run in linear
	{
		if (m_manifestFiles[i].path.compareNoCaseN(filename, n) == 0)
		{
			matchFound = true;
			returnFilename = m_manifestFiles[i].path;
			if (fileData != NULL)
			{
				fileData->push_back(&m_manifestFiles[i]);
			}

			if (!wildcard)
				return matchFound;
		}
		else
		{
			break;
		}
	}
	return matchFound;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCManifest::addLinkedPalette(NdhsCManifest* palette)
{
	m_linkedPalettes.push_back(palette);
}

#ifdef NDHS_JNI_INTERFACE
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCManifest::loadTargetManifest()
{
	m_manifestFiles.clear();

	// Load project from XML
	LcTaString err;
	// Set the directory slash separators to the non default if required.
	LcTaString localPath = m_manifestFile;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif
	LcTaOwner<LcCXmlElem> eRoot = LcCXmlElem::load(localPath, err);

	if (eRoot)
	{
		if (eRoot->getName().compareNoCase(NDHS_TP_XML_UI_MANIFEST) == 0)
		{
			// Signature File
			m_signatureFile = eRoot->getAttr(NDHS_TP_XML_SIGNATURE);

			// Compatibility
			m_compatibility = eRoot->getAttr(NDHS_TP_XML_COMPATIBILITY);

			LcCXmlElem* eTemp = eRoot.ptr();

			// Iterate through the files
			LcCXmlElem* eFile = eTemp->getFirstChild();

			for (; eFile != NULL; eFile = eFile->getNext())
			{
				if (eFile->getName().compareNoCase(NDHS_TP_XML_MANIFEST_FILE) == 0)
				{
					CManifestFile mf;
					mf.path = eFile->getAttr(NDHS_TP_XML_MANIFEST_PATH);

					m_manifestFiles.push_back(mf);
				}
			}
		}

		return true;
	}

	return false;
}
#endif
