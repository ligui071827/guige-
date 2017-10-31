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

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

#include "inflexionui/engine/inc/NdhsCEffect.h"

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCEffect> NdhsCEffect::create(LcCSpace* space)
{
	LcTaOwner<NdhsCEffect> ref;
	ref.set(new NdhsCEffect());
	ref->construct(space);
	return ref;
}

void NdhsCEffect::construct(LcCSpace* space)
{
	m_space = space;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCEffect::NdhsCEffect()
{
	// Default consturctor
}

NdhsCEffect::~NdhsCEffect()
{
	// Destructor
}

/*-------------------------------------------------------------------------*//**
																			 */
void NdhsCEffect::configureDefaultEffectsFromXml(int stackLevel)
{
	// Normal default effects
	LcTaString light0	  	= "Z:/Effects/normal_light_0.effect";

	LcTaString light1	  	= "Z:/Effects/normal_light_1.effect";
	LcTaString light2	  	= "Z:/Effects/normal_light_2.effect";

	LcTaString tex 		  	= "Z:/Effects/normal_background.effect";

	LcTaString texLight0   	= "Z:/Effects/normal_texlight_0.effect";
	LcTaString texLight1   	= "Z:/Effects/normal_texlight_1.effect";
	LcTaString texLight2   	= "Z:/Effects/normal_texlight_2.effect";

	LcTaString alphaLight0 	= "Z:/Effects/normal_alphalight_0.effect";
	LcTaString alphaLight1 	= "Z:/Effects/normal_alphalight_1.effect";
	LcTaString alphaLight2 	= "Z:/Effects/normal_alphalight_2.effect";

	// HQ default effects
	LcTaString highLight0	  	= "Z:/Effects/high_light_0.effect";
	LcTaString highLight1	  	= "Z:/Effects/high_light_1.effect";
	LcTaString highLight2	  	= "Z:/Effects/high_light_2.effect";

	LcTaString highTexLight0   	= "Z:/Effects/high_texlight_0.effect";
	LcTaString highTexLight1   	= "Z:/Effects/high_texlight_1.effect";
	LcTaString highTexLight2   	= "Z:/Effects/high_texlight_2.effect";

	LcTaString highAlphaLight0 	= "Z:/Effects/high_alphalight_0.effect";
	LcTaString highAlphaLight1 	= "Z:/Effects/high_alphalight_1.effect";
	LcTaString highAlphaLight2 	= "Z:/Effects/high_alphalight_2.effect";

	LcTaString signatureFile    = "Z:/manifest.sig";

	LcTaString effectPath;
	LcTaString temp;

	NdhsCManifest* paletteMan = NULL;

	IFX_SIGNATURE signature = {{0}};

	LcTaOwner<LcCReadOnlyFile> file = LcCReadOnlyFile::openFile(signatureFile.bufUtf8());

	if(file)
	{
		for (int i = 0; i < IFX_HASH_SIZE; i++)
		{
			if(file->read(&signature.hash[i], sizeof(IFX_UINT32), 1) != 1)
				break;
		}
		file->close();
	}

	m_shaderManifest = NdhsCManifest::create("Z:/manifest.xml", "");
	if (!m_shaderManifest)
		return;
	m_shaderManifest->loadManifest(true);
	paletteMan = m_shaderManifest.ptr();

	//------------------------------------------------------------
	// NORMAL Mode
	//------------------------------------------------------------

	// 1- Light Only : Normal
	configureDefaultEffectFromXml(light0, temp, paletteMan, stackLevel, false, signature); // Do not set "HQ" flag
	configureDefaultEffectFromXml(light1, temp, paletteMan, stackLevel, false, signature); // Do not set "HQ" flag
	configureDefaultEffectFromXml(light2, temp, paletteMan, stackLevel, false, signature); // Do not set "HQ" flag

	// 2- Tex Only : Normal
	configureDefaultEffectFromXml(tex, temp, paletteMan, stackLevel, false, signature); // Do not set "HQ" flag

	// 3- TexLight : Normal
	configureDefaultEffectFromXml(texLight0, temp, paletteMan, stackLevel, false, signature); // Do not set "HQ" flag
	configureDefaultEffectFromXml(texLight1, temp, paletteMan, stackLevel, false, signature); // Do not set "HQ" flag
	configureDefaultEffectFromXml(texLight2, temp, paletteMan, stackLevel, false, signature); // Do not set "HQ" flag

	// 4- AlphaLight : Normal
	configureDefaultEffectFromXml(alphaLight0, temp, paletteMan, stackLevel, false, signature); // Do not set "HQ" flag
	configureDefaultEffectFromXml(alphaLight1, temp, paletteMan, stackLevel, false, signature); // Do not set "HQ" flag
	configureDefaultEffectFromXml(alphaLight2, temp, paletteMan, stackLevel, false, signature); // Do not set "HQ" flag

	//------------------------------------------------------------
	// HQ Mode
	//------------------------------------------------------------

	// 5- Light Only : Enhanced/HQ
	configureDefaultEffectFromXml(highLight0, temp, paletteMan, stackLevel, true, signature); // Set "HQ" flag
	configureDefaultEffectFromXml(highLight1, temp, paletteMan, stackLevel, true, signature); // Set "HQ" flag
	configureDefaultEffectFromXml(highLight2, temp, paletteMan, stackLevel, true, signature); // Set "HQ" flag

	// 7- TexLight : Enhanced/HQ
	configureDefaultEffectFromXml(highTexLight0, temp, paletteMan, stackLevel, true, signature); // Set "HQ" flag
	configureDefaultEffectFromXml(highTexLight1, temp, paletteMan, stackLevel, true, signature); // Set "HQ" flag
	configureDefaultEffectFromXml(highTexLight2, temp, paletteMan, stackLevel, true, signature); // Set "HQ" flag

	// 8- AlphaLight : Enhanced/HQ
	configureDefaultEffectFromXml(highAlphaLight0, temp, paletteMan, stackLevel, true, signature); // Set "HQ" flag
	configureDefaultEffectFromXml(highAlphaLight1, temp, paletteMan, stackLevel, true, signature); // Set "HQ" flag
	configureDefaultEffectFromXml(highAlphaLight2, temp, paletteMan, stackLevel, true, signature); // Set "HQ" flag
}

/*-------------------------------------------------------------------------*//**
																			 */
bool NdhsCEffect::configureDefaultEffectFromXml(const LcTmString& effectFileName, LcTmString& effectName,
                                                                            NdhsCManifest* paletteMan, int stackLevel, bool isHQ, IFX_SIGNATURE signature)
{
	bool status = false;
	LcTaString err;
	LcTaString name, usage;
	LcTaString zDrive = "Z:/";

	LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(effectFileName, err, EShader);

	if (root)
	{
		LcTaString usage = root->getAttr(NDHS_TP_XML_USAGE);
		LcTaString name = root->getAttr(NDHS_TP_XML_NAME);

		effectName = name;
		m_defaultEffect = m_space->getOglContext()->addDefaultEffect(name);

		// Ensure that this default effect exists but does not exist in the cache
		if (m_defaultEffect && !m_defaultEffect->isCached())
		{
			LcTaString effectNameFull = effectFileName;

			m_defaultEffect->setEffectName(effectNameFull);
			m_defaultEffect->setUsage(usage);

			m_defaultEffect->setHighQualityLightingStatus(isHQ);
		}
		else
		{
			return false;
		}

#ifndef NDHS_JNI_INTERFACE

		// Make shader file name of extension .vfbin
		int fLen = effectFileName.getWord(-1, '/').length();

		LcTaString unifiedBinaryFileName = effectFileName.subString(3,effectFileName.length()- fLen -3) + name + ".vfbin";

		unsigned int binaryFormat = 0;

		LcTaArray<NdhsCManifest::CManifestFile*> fileData;

		// First lookup unified binary shader file exist, if it fails then lookup for binary shader or source file
		if (paletteMan->fileExists (unifiedBinaryFileName, unifiedBinaryFileName, false, &fileData))
		{
			// Set it as unified binary
			m_defaultEffect->setUnifiedBinary(true);
			m_defaultEffect->setBinary(true);

			unifiedBinaryFileName = zDrive + unifiedBinaryFileName;

			// Assigning vertex and fragment shader file.
			m_defaultEffect->setVShaderFile(unifiedBinaryFileName);

			binaryFormat = fileData[0]->m_compiledShaderFormat;
			m_defaultEffect->setBinaryFormat(binaryFormat);

			status = true;
		}
		else

#endif /* NDHS_JNI_INTERFACE */

		{
			LcTaString vertexSourceFileName, fragmentSourceFileName;

			LcCXmlElem* vertexShader = root->find(NDHS_TP_XML_VERTEX_SHADER);
			LcCXmlElem* fragmentShader = root->find(NDHS_TP_XML_FRAGMENT_SHADER);

			if(vertexShader && fragmentShader)
			{
				// Vertex shader configuration
				status = configureShaderFromXml(vertexShader, &vertexSourceFileName);

				if(status)
				{
					// Fragment shader configuration
					status = configureShaderFromXml(fragmentShader, &fragmentSourceFileName);
				}
			}

			if(!status)
			{
				return false;
			}

			status = false;

			if (vertexSourceFileName.isEmpty() == 0 && fragmentSourceFileName.isEmpty() == 0)
			{

#ifndef NDHS_JNI_INTERFACE

				LcTaString extVert = vertexSourceFileName.getWord(-1, '.');
				LcTaString extFrag = fragmentSourceFileName.getWord(-1, '.');

				// Make shader file name with extension .vbin and .fbin
				LcTaString vertexBinaryFileName = vertexSourceFileName.subString(0, vertexSourceFileName.length() - extVert.length() - 1) + ".vbin";
				LcTaString fragmentBinaryFileName = fragmentSourceFileName.subString(0, fragmentSourceFileName.length() - extFrag.length() - 1) + ".fbin";

				// Lookup binary shader file, if it fails then lookup source file
				if (paletteMan->fileExists (vertexBinaryFileName, vertexBinaryFileName, false, &fileData) &&
				   paletteMan->fileExists (fragmentBinaryFileName, fragmentBinaryFileName, false) )
				{
					// Set it as pre-compiled binary
					m_defaultEffect->setBinary(true);

					vertexBinaryFileName = zDrive + vertexBinaryFileName;
					fragmentBinaryFileName = zDrive + fragmentBinaryFileName;

					// Assigning vertex and fragment shader file.
					m_defaultEffect->setVShaderFile(vertexBinaryFileName);
					m_defaultEffect->setFShaderFile(fragmentBinaryFileName);

					binaryFormat = fileData[0]->m_compiledShaderFormat;
					m_defaultEffect->setBinaryFormat(binaryFormat);

					status = true;
				}
				else

#endif /* NDHS_JNI_INTERFACE */
				{
					if (paletteMan->fileExists (vertexSourceFileName, vertexSourceFileName, false) &&
					   paletteMan->fileExists (fragmentSourceFileName, fragmentSourceFileName, false) )
					{
						// Set as source shader
						m_defaultEffect->setBinary(false);

						vertexSourceFileName = zDrive + vertexSourceFileName;
						fragmentSourceFileName = zDrive + fragmentSourceFileName;

						// Assigning vertex and fragment shader file.
						m_defaultEffect->setVShaderFile(vertexSourceFileName);
						m_defaultEffect->setFShaderFile(fragmentSourceFileName);

						status = true;
					}
				}
			}
		}

		if(status)
		{
			m_defaultEffect->setEffectSignature(signature);

			LcCXmlElem* attributes = root->find(NDHS_TP_XML_ATTRIBUTES);
			status = configureAttributesFromXml(attributes, m_defaultEffect);

			if(status)
			{
				LcCXmlElem* uniforms = root->find(NDHS_TP_XML_UNIFORMS);
				status = configureUniformsFromXml(uniforms, m_defaultEffect);
			}
		}
	}
	else
	{
		m_defaultEffect = NULL;
		status = false;
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCEffect::configureAttributesFromXml(LcCXmlElem* attributes, LcOglCEffect *effect)
{
	bool status = true;
	LcCXmlElem* attribute;
	LcTaString name, type, mapping;

	if(attributes)
	{
		attribute = attributes->getFirstChild();

		for(; attribute; attribute = attribute->getNext())
		{
			if(attribute->getName().compareNoCase(NDHS_TP_XML_ATTRIBUTE) ==  0)
			{
				name = attribute->getAttr(NDHS_TP_XML_NAME);
				type = attribute->getAttr(NDHS_TP_XML_TYPE);
				mapping = attribute->getAttr(NDHS_TP_XML_ENGINE_MAPPING);

				effect->addAttribute(name, type, mapping);
			}
		}
	}

	return (status);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCEffect::configureUniformsFromXml(LcCXmlElem* uniforms, LcOglCEffect* effect)
{
	bool				status = true;
	LcCXmlElem* 		uniform;
	LcCXmlElem* 		child;
	LcTaString 			name, type, mapping;
	bool 				mipmap = false;

	if(uniforms)
	{
		uniform = uniforms->getFirstChild();

		for(; uniform; uniform = uniform->getNext())
		{
			if(uniform->getName().compareNoCase(NDHS_TP_XML_UNIFORM) ==  0)
			{
				name = uniform->getAttr(NDHS_TP_XML_NAME);
				type = uniform->getAttr(NDHS_TP_XML_TYPE);
				mapping = uniform->getAttr(NDHS_TP_XML_ENGINE_MAPPING);
				mipmap = LcCXmlAttr::strToBool(uniform->getAttr(NDHS_TP_XML_MIP_MAP, "false"));

				LcTaString data[6], minData[6], maxData[6];

				// Default Values
				child = uniform->find(NDHS_TP_XML_DEFAULT_VALUE);

				if(child)
				{
					LcTaString value = "value#";

					for(int i = 0; i < 6; i++)
					{
						value.replace('#', ('0' + (i+1)));
						data[i] = child->getAttr(value);
						value = "value#";
					}
				}

				// Min Values
				child = uniform->find(NDHS_TP_XML_MIN_VALUE);

				if(child)
				{
					LcTaString value = "value#";

					for(int i = 0; i < 6; i++)
					{
						value.replace('#', ('0' + (i+1)));
						minData[i] = child->getAttr(value);
						value = "value#";
					}
				}

				// Max Values
				child = uniform->find(NDHS_TP_XML_MAX_VALUE);

				if(child)
				{
					LcTaString value = "value#";

					for(int i = 0; i < 6; i++)
					{
						value.replace('#', ('0' + (i+1)));
						maxData[i] = child->getAttr(value);
						value = "value#";
					}
				}

				effect->addUniformFromEffect(name, type, mapping, data, minData, maxData, mipmap);
			}
		}
	}
	else
	{
		status = false;
	}

	return (status);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCEffect::configureShaderFromXml(LcCXmlElem* shader, LcTmString* fileName)
{
	LcTaString attributeName;
	bool status = true;

	if(shader != NULL)
	{
		attributeName = shader->getAttr(NDHS_TP_XML_NAME);

		if(attributeName.isEmpty() == 0)
		{
			shader = shader->find(NDHS_TP_XML_SHADER_SOURCE);
		}
		else
		{
			status = false;
		}
	}

	if(shader != NULL && status)
	{
		attributeName = shader->getAttr(NDHS_TP_XML_FILE);
		*fileName = attributeName;
	}
	else
	{
		status = false;
	}

	return (status);
}

#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */
