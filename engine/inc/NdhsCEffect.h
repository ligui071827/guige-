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
#ifndef NdhsCEffectH
#define NdhsCEffectH

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

/*-------------------------------------------------------------------------*//**
	NdhsCEffect contains the common functions required by Default and custom
	shaders in the Inflexion engine.
*/

class LcOglCSLType;

class NdhsCEffect : public LcCBase	
{
private:
	LcOglCEffect* 						m_defaultEffect;
	LcCSpace*							m_space;
	LcTmOwner<NdhsCManifest>			m_shaderManifest;

	// Two-phase construction
						NdhsCEffect();
	void				construct(LcCSpace* space);

public:
	static	LcTaOwner<NdhsCEffect> 		create(LcCSpace* space);
	virtual								~NdhsCEffect();

	bool						configureDefaultEffectFromXml(const LcTmString& effectFileName, LcTmString& effectName, NdhsCManifest* paletteMan, int stackLevel, bool isHQ, IFX_SIGNATURE signature);
	void						configureDefaultEffectsFromXml(int stackLevel = 0);

	bool						configureShaderFromXml(LcCXmlElem* shader, LcTmString* fileName);
	bool						configureAttributesFromXml(LcCXmlElem* attributes, LcOglCEffect* effect);
	bool						configureUniformsFromXml(LcCXmlElem* uniforms, LcOglCEffect* effect);
};

#endif	/* defined(IFX_RENDER_DIRECT_OPENGL_20) */

#endif
