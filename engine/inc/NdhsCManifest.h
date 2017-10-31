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
#ifndef NdhsCManifestH
#define NdhsCManifestH

#include "inflexionui/engine/inc/LcCSerializeMaster.h"

class NdhsCManifest : public LcCBase,public ISerializeable
{
public:

	class CManifestFile
	{
	public:
		LcTmString		path;
		LcTmString		absolutePath;
		int				m_marginTop;
		int				m_marginBottom;
		int				m_marginLeft;
		int				m_marginRight;
		int				m_frameCount;
		unsigned int	m_compiledShaderFormat;
		IFX_SIGNATURE	m_signature;
		
		CManifestFile()
		{
			m_marginTop=0;
			m_marginBottom=0;
			m_marginLeft=0;
			m_marginRight=0;
			m_frameCount=1;
			m_compiledShaderFormat=0;
		}
		bool operator<(CManifestFile &inp)
		{
			return (path.compareNoCase(inp.path) < 0);
		}
	};

	typedef LcTmArray<NdhsCManifest*> TmAPaletteList;

private:

	TmAPaletteList		m_linkedPalettes;

	typedef LcTmArray<CManifestFile> TmAManifestFiles;
	TmAManifestFiles	m_manifestFiles;

	LcTmString			m_signatureFile;
	LcTmString			m_compatibility;

	IFX_SIGNATURE		m_signature;

	LcTmString			m_manifestFile;
	LcTmString			m_manifestPath;

	LcTmString			m_packageName;

						NdhsCManifest(const LcTmString& manifest, const LcTmString& packageName);
	void				construct();

public:

	static LcTaOwner<NdhsCManifest> create(const LcTmString& manifest, const LcTmString& packageName);
#ifdef IFX_SERIALIZATION
	void					deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	static NdhsCManifest*	loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual SerializeHandle serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	bool					isMenuItemChild(){return false;}
#endif /* IFX_SERIALIZATION */
	virtual 			~NdhsCManifest();

	bool				loadManifestHeader();
	bool				loadManifest(bool shaderManifest = false);
#ifdef NDHS_JNI_INTERFACE
	bool				loadTargetManifest();
#endif
	bool				fileExists(const LcTmString& file,
									LcTmString &returnFilename,
									bool wildcard = false,
									LcTaArray<NdhsCManifest::CManifestFile*> *fileData = 0);
	int 				findIndex(int a, int b, LcTmString &file, bool wildcard);

	void				addLinkedPalette(NdhsCManifest* palette);
	TmAPaletteList*		getLinkedPalettes()	{ return &m_linkedPalettes; }

	inline LcTaString	getManifestFile()	{ return m_manifestFile; }
	inline LcTaString	getManifestPath()	{ return m_manifestPath; }
	inline LcTaString	getSignatureFile()	{ return m_signatureFile; }
	inline LcTaString	getCompatibility()	{ return m_compatibility; }
	inline LcTaString	getPackageName()	{ return m_packageName; }

	int					getFileCount()		{ return (int)m_manifestFiles.size(); }
	CManifestFile*		getFile(int index)	{ return &m_manifestFiles[index]; }
};

#endif
