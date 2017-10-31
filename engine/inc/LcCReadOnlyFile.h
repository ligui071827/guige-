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
#ifndef LcCReadOnlyFileH
#define LcCReadOnlyFileH

#include "inflexionui/engine/inc/LcCBase.h"
class LcCXmlElem;

/*-------------------------------------------------------------------------*//**
	Simple API for reading data from a file-like source.  This could be a
	real file, a ROMized data buffer, or a stream from HTTP.  Note that
	backwards seek() operations may fail on some sources.  Note also that
	C classes are used for generic handling of files, even though not all
	derived classes will encapsulate resources that require ownership/cleanup
*/
class LcCReadOnlyFile : public LcCBase
{
private:

#ifdef LC_USE_TOOLSET_FILES
	IFXP_FILE							m_file;
#else
	IFXE_FILE							m_file;
#endif

	// Specify from where we need to look for files
	ERomFileSystem						m_romFileSystem;

protected:

	// Abstract
										LcCReadOnlyFile(ERomFileSystem romFileSystem);
	inline								LcCReadOnlyFile() {}
	
public:

	// Type
	enum EFrom { EStart, ECurrent, EEnd };

	// API similar to stdio's fread() etc
	virtual			unsigned			read(void* addr, unsigned size, unsigned count);
	virtual			bool				seek(int offset, EFrom from);
	virtual			int					size();
	virtual			void				close();

	// Open a file given a full pathname
	virtual			bool				open(const LcTmString& name);

	// For simple generic read-only file system access
	LC_IMPORT static LcTaOwner<LcCReadOnlyFile> openFile(const LcTmString& name, ERomFileSystem romFileSystem = ETheme);
	
	
										~LcCReadOnlyFile();
};

#endif //LcCReadOnlyFileH
