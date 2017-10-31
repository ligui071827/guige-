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
#ifndef NdhsCProjectValidatorH
#define NdhsCProjectValidatorH

#if defined(IFX_VALIDATE_PROJECTS) || defined(NDHS_TOOLKIT)
	#include "inflexionui/engine/inc/LcCShaHash.h"
#endif
#include "inflexionui/engine/inc/NdhsCManifest.h"

/*-------------------------------------------------------------------------*//**
*/
class NdhsCProjectValidator
{
private:

	LcTmString			m_projectPath;

#if defined(IFX_VALIDATE_PROJECTS) || defined(NDHS_JNI_INTERFACE)
	LcTmString			m_signatureFile;
	LcCShaHash			m_signature;

	bool				validateFiles(NdhsCManifest* manifest);
	bool				validateFile(NdhsCManifest::CManifestFile* file);

	bool				getFileSignature(const LcTmString& file, const unsigned int offset, LcTUInt32* sig);
#endif // defined(IFX_VALIDATE_PROJECTS) || defined(NDHS_JNI_INTERFACE)

public:

	enum { ESuccess = 0, EError = 1, EErrorVersion = 2 };

	// Construction/Destruction
						NdhsCProjectValidator() {}
						~NdhsCProjectValidator() {}

	// Loads and processes the project
	int					loadProject(NdhsCManifest* manifest);

	// Checks the compatibility version
	static	bool		checkVersion(int major, int minor);

#if defined(IFX_VALIDATE_PROJECTS)
	// Used by the device to validate the project signature
	bool				validateProject();
#endif //IFX_VALIDATE_PROJECTS

#if defined(NDHS_JNI_INTERFACE) || (defined(NDHS_TOOLKIT) && defined(IFX_VALIDATE_PROJECTS))
	// Used by toolkit to return the project signature
	void				getProjectSignature(LcTUInt32* sig);

	// Returns the file used to store the signature
	LcTaString			getSignatureFile() { return m_signatureFile; }

#endif
};

#endif /* NdhsCProjectValidatorH */
