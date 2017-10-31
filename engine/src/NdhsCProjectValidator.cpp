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



//number of unsigned longs in hash
#define NDHS_HASH_SIZE			5

// Chunk size value is
// best as a multiple of 64 since the hashing algorithm
// works on 64 byte chunks.
#define NDHS_HASH_CHUNK_SIZE	1024

/*-------------------------------------------------------------------------*//**
	Static function to check that the provided major and minor version
	fall into the allowed range
*/
bool NdhsCProjectValidator::checkVersion(int major, int minor)
{
	bool versionOk = false;

	// If the theme expects a newer version of the application
	// than we currently have running, we cannot load it
	// NB: ignore the 'release' value
	if (major < NDHS_MAJOR_VERSION ||
		(major == NDHS_MAJOR_VERSION && minor <= NDHS_MINOR_VERSION))
			versionOk = true;

	// HOWEVER!  Since we might no longer accept old format themes,
	// we reject if the theme is < the most recent accepted version
	if (versionOk)
	{
		if(major < NDHS_EARLIEST_SUPPORTED_MAJOR ||
			(major == NDHS_EARLIEST_SUPPORTED_MAJOR && minor < NDHS_EARLIEST_SUPPORTED_MINOR))
				versionOk = false;
	}

	return versionOk;
}

/*-------------------------------------------------------------------------*//**
	Loads the project and generates the signature
*/
int NdhsCProjectValidator::loadProject(NdhsCManifest* manifest)
{
	int loaded         = EError;
	bool projectFileOk = true;

	// Store the Project path
	m_projectPath = manifest->getManifestPath();

	if (projectFileOk)
	{
		LcTaString versionNumber = manifest->getCompatibility();
		if (versionNumber.length() > 0)
		{
			int major	= versionNumber.getWord(1, ',').toInt();
			int minor	= versionNumber.getWord(2, ',').toInt();
//			int release = versionNumber.getWord(3, ',').toInt();

			bool versionOk = checkVersion(major, minor);

			if (versionOk == false)
			{
				loaded = EErrorVersion;
				projectFileOk = false; //don't hash the project
			}
		}
	}

// We don't check signatures in debug builds, as that would prevent
// us from easily editing test themes
#if defined(IFX_VALIDATE_PROJECTS) || defined(NDHS_JNI_INTERFACE)
	if (projectFileOk)
	{
		// Initialize the hashing object
		m_signature.init();

		// Disguise the signature
		LcTUInt32 hash[NDHS_HASH_SIZE];

		// Initialize hash to NDHS key
		SETB32_BE(&hash[0], 0x8BE5D6A6);
		SETB32_BE(&hash[1], 0xE780EB74);
		SETB32_BE(&hash[2], 0x6D4A1395);
		SETB32_BE(&hash[3], 0x9324F17E);
		SETB32_BE(&hash[4], 0x08E288C1);

		// Seed hash with NDHS key
		m_signature.update((unsigned char*)hash, NDHS_HASH_SIZE * sizeof(LcTUInt32));

		// Store the signature file
		m_signatureFile = manifest->getSignatureFile();

		// Hash the files
		if (validateFiles(manifest))
		{

			// Hash the project file
			LcTUInt32 sig[NDHS_HASH_SIZE];

			// Get a signature for the file
			if (getFileSignature(manifest->getManifestFile(), 0, sig))
			{
				// Add the signature to the running signature
				m_signature.update((unsigned char*)sig, NDHS_HASH_SIZE * sizeof(LcTUInt32));
			}

			m_signature.complete();
		}
		else
		{
			projectFileOk = false;
		}
	}

#endif //defined(IFX_VALIDATE_PROJECTS) || defined(NDHS_JNI_INTERFACE)

	if (projectFileOk)
		loaded = ESuccess;

	return loaded;
}

#if defined(IFX_VALIDATE_PROJECTS)
/*-------------------------------------------------------------------------*//**
	Checks the generated signature against the supplied signature file
*/
bool NdhsCProjectValidator::validateProject()
{
	bool valid = false;
	LcTUInt32 sig1[NDHS_HASH_SIZE];
	LcTUInt32 sig2[NDHS_HASH_SIZE];
	m_signature.getRawDigest(sig1);

	LcTaString path = m_projectPath + m_signatureFile;

	// Set the directory slash separators to the non default if required.
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		path.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif

	// Signature files are non-ROM as only non-ROM filesets get signed
	LcTaOwner<LcCReadOnlyFile> file = LcCReadOnlyFile::openFile(path.bufUtf8());
	if (file.ptr() != NULL)
	{
		file->seek(0, LcCReadOnlyFile::EStart);

		if (file->read(sig2, sizeof(LcTUInt32), NDHS_HASH_SIZE) == NDHS_HASH_SIZE)
		{
			if(lc_memcmp(sig1, sig2, NDHS_HASH_SIZE * sizeof(LcTUInt32)) == 0)
			{
				valid = true;
			}
		}

		file->close();

	}

	return valid;
}
#endif //IFX_VALIDATE_PROJECTS

#if defined(NDHS_JNI_INTERFACE) || (defined(NDHS_TOOLKIT) && defined(IFX_VALIDATE_PROJECTS))
/*-------------------------------------------------------------------------*//**
	Returns the generated signature for the toolkit
*/
void NdhsCProjectValidator::getProjectSignature(LcTUInt32* sig)
{
	if (sig != NULL)
		m_signature.getRawDigest(sig);
}
#endif	// NDHS_TOOLKIT && IFX_VALIDATE_PROJECTS

#if defined(IFX_VALIDATE_PROJECTS) || defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
	Processes the Files sections of the project file
*/
bool NdhsCProjectValidator::validateFiles(NdhsCManifest* manifest)
{
	bool noErrorDetected = true;

	int fileCount = manifest->getFileCount();

	for (int i = 0; i < fileCount && noErrorDetected; i++)
	{
		noErrorDetected = validateFile(manifest->getFile(i));
	}

	return noErrorDetected;
}
#endif // defined(IFX_VALIDATE_PROJECTS) || defined(NDHS_JNI_INTERFACE)

#if defined(IFX_VALIDATE_PROJECTS) || defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
	Processes the file elements of the project file
*/
bool NdhsCProjectValidator::validateFile(NdhsCManifest::CManifestFile* file)
{
	if (file == NULL)
		return false;

	bool fileValidated = false;

	LcTaString path = file->path;
	if (path.length() > 0)
	{
		LcTUInt32 sig[NDHS_HASH_SIZE];

		LcTaString fullPath = m_projectPath + path;

		// Get a signature for the file
		fileValidated = getFileSignature(fullPath, 0, sig);

		if (fileValidated == true)
		{
			// Add the signature to the running signature
			m_signature.update((unsigned char*)sig, NDHS_HASH_SIZE * sizeof(LcTUInt32));
		}
	}

	return fileValidated;
}
#endif // defined(IFX_VALIDATE_PROJECTS) || defined(NDHS_JNI_INTERFACE)

#if defined(IFX_VALIDATE_PROJECTS) || defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
	Generates a signature for an individual file - can start at an offset
	into the file if required
*/
bool NdhsCProjectValidator::getFileSignature(const LcTmString& file, const unsigned int offset, LcTUInt32* sig)
{
	bool result = false;

	if (sig == NULL)
		return result;

	// Set the directory slash separators to the non default if required.
	LcTaString localFile = file;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localFile.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif

	// Only local non-ROM files are signed
	LcTaOwner<LcCReadOnlyFile> sigFile = LcCReadOnlyFile::openFile(localFile.bufUtf8());
	if (sigFile.ptr() != NULL)
	{
		int count;
		unsigned char buf[NDHS_HASH_CHUNK_SIZE];
		LcCShaHash signature;
		signature.init();

		sigFile->seek(offset, LcCReadOnlyFile::EStart);

		while ((count = (int)sigFile->read(buf, 1, NDHS_HASH_CHUNK_SIZE)) > 0)
		{
			signature.update((unsigned char*)buf, count);
		}

		signature.complete();

		signature.getRawDigest(sig);
		result = true;

		sigFile->close();
	}

	return result;
}
#endif // defined(IFX_VALIDATE_PROJECTS) || defined(NDHS_JNI_INTERFACE)
