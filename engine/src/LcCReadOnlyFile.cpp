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
#include "inflexionui/engine/ifxui_engine.h"

#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif

#ifdef LC_USE_TOOLSET_FILES
	#define IFX_FILE_READ		IFXP_File_Read
	#define IFX_FILE_SEEK		IFXP_File_Seek
	#define IFX_FILE_SIZE		IFXP_File_Size
	#define IFX_FILE_CLOSE		IFXP_File_Close
	#define IFX_FILE_OPEN		IFXP_File_Open
#else
	#define IFX_FILE_READ		IFXE_File_Read
	#define IFX_FILE_SEEK		IFXE_File_Seek
	#define IFX_FILE_SIZE		IFXE_File_Size
	#define IFX_FILE_CLOSE		IFXE_File_Close
	#define IFX_FILE_OPEN		IFXE_File_Open
#endif

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCReadOnlyFile> LcCReadOnlyFile::openFile(const LcTmString& name, ERomFileSystem romFileSystem)
{
	LcTaOwner<LcCReadOnlyFile> ref;
	ref.set(new LcCReadOnlyFile(romFileSystem));

	if (!ref->open(name))
		ref.destroy();

	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcCReadOnlyFile::LcCReadOnlyFile(ERomFileSystem romFileSystem)
{
#if !defined(NDHS_PACKAGER) && !defined(NDHS_INTEGRATOR) && defined(IFX_RENDER_DIRECT_OPENGL_20)
	m_romFileSystem = romFileSystem;
#else
	m_romFileSystem = ETheme;
#endif
}

/*-------------------------------------------------------------------------*//**
*/
unsigned LcCReadOnlyFile::read(void* addr, unsigned size, unsigned count)
{
	unsigned objectsRead = 0;

	if (m_file)
	{
		IFX_UINT32 bytesRead = 0;

		IFX_RETURN_STATUS status = IFX_ERROR;

		#if defined(NDHS_JNI_INTERFACE)
			if (m_romFileSystem ==  EShader)
				status = IFXE_File_Read(m_file, addr, size * count, &bytesRead);
			else
		 #endif
			status = IFX_FILE_READ(m_file, addr, size * count, &bytesRead);

		if (IFX_SUCCESS == status)
		{
			objectsRead = bytesRead / size;

			// Roll file offset back if not all objects could be read
			if (objectsRead < count)
			{
				seek((objectsRead * size) - bytesRead, ECurrent);
			}
		}
	}

	return objectsRead;
}

/*-------------------------------------------------------------------------*//**
*/
bool LcCReadOnlyFile::seek(int offset, EFrom from)
{
	bool retVal = false;

	if (m_file)
	{
		IFX_SEEK seek = IFX_SEEK_SET;

		switch (from)
		{
			case EStart:	seek = IFX_SEEK_SET; 	break;
			case ECurrent:	seek = IFX_SEEK_CUR;	break;
			case EEnd:		seek = IFX_SEEK_END;	break;
			default:		LC_ASSERT(false);		break;
		}

		IFX_RETURN_STATUS status = IFX_ERROR;

#if defined(NDHS_JNI_INTERFACE)
		if (m_romFileSystem ==  EShader)
			status = IFXE_File_Seek(m_file, offset, seek);
		else
#endif
			status = IFX_FILE_SEEK(m_file, offset, seek);

		retVal = (IFX_SUCCESS == status);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
int LcCReadOnlyFile::size()
{
	int retVal = -1;

	if (m_file)
	{
		IFX_UINT32 fileSize = 0;

		IFX_RETURN_STATUS status = IFX_ERROR;

#if defined(NDHS_JNI_INTERFACE)
		if (m_romFileSystem ==  EShader)
			status = IFXE_File_Size(m_file, &fileSize);
		else
#endif
			status = IFX_FILE_SIZE(m_file, &fileSize);

		if (IFX_SUCCESS == status)
		{
			retVal = fileSize;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void LcCReadOnlyFile::close()
{
	if (m_file)
	{
#if defined(NDHS_JNI_INTERFACE)
		if (m_romFileSystem ==  EShader)
			IFXE_File_Close(m_file);
		else
#endif
		IFX_FILE_CLOSE(m_file);
		m_file = NULL;
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool LcCReadOnlyFile::open(const LcTmString& name)
{
	if (m_file || name.isEmpty())
	{
		close();
	}

	LcTaString localFile = name;

#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR) && defined(LC_USE_TOOLSET_FILES)
	// Deal with different folder separator chars (usually done in IFXE layer)
	localFile.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
#endif

	IFX_RETURN_STATUS retVal = IFX_ERROR;

	if (m_romFileSystem == ETheme)
	{
		retVal = IFX_FILE_OPEN(&m_file, localFile.bufUtf8());
	}
#if !defined(NDHS_PACKAGER) && !defined(NDHS_INTEGRATOR) && defined(IFX_RENDER_DIRECT_OPENGL_20)
	else if (m_romFileSystem == EShader)
	{
		m_file = LcGetRomFile(g_shaderRomFileCount, g_shaderRomFileTable, localFile.bufUtf8());

		if (m_file)
		{
			retVal = IFX_SUCCESS;
		}
	}
#endif

	if (retVal != IFX_SUCCESS)
	{
		m_file = NULL;
	}

	return retVal == IFX_SUCCESS;
}

/*-------------------------------------------------------------------------*//**
*/
LcCReadOnlyFile::~LcCReadOnlyFile()
{
	close();
}
