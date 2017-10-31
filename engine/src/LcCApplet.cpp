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
	@param appName A unique name for this application. It is recommended that
				 this takes the format "com.companyName.applicationName" where
				 it contains your registered company domain name.
*/
LC_EXPORT void LcCApplet::construct(const LcTmString& appName)
{
	LcCAggregate::construct();

	// Cannot copy string values in constructor due to allocation
	m_appName = appName;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCApplet::~LcCApplet()
{
	if (m_space)
		m_space->removeApplet(this);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCApplet::realize(LcCSpace* s)
{
	// Set up container
	m_space = s;
	m_space->addApplet(this);

	// Enables mappings
	setXfmsDirty();

	// Call applet's onRealize() handler.  Note that if doOnRealize() fails, any cleanup will
	// already have been done, including removing the widget from its parent
	// aggregate.
	if (!doOnRealize())
		return false;

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCApplet::doOnRetire()
{
	LcCAggregate::doOnRetire();

	// Detach applet from space
	m_space->removeApplet(this);
	m_space = NULL;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTaString LcCApplet::findFontName()
{
	// Check for applet override
	LcTaString value = getFontName();
	if (value.length() > 1)
		return value;

	// Get default value from LAF if attached
	if (m_space)
		return m_space->getDefaultFontName();
	else
		return "Arial";
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCFont::EStyle LcCApplet::findFontStyle()
{
	// Check for applet override
	LcCFont::EStyle value = getFontStyle();
	if (value > -1)
		return value;

	if (m_space)
		return m_space->getDefaultFontStyle();
	else
		return LcCFont::REGULAR;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTColor LcCApplet::findFontColor()
{
	// Check for applet override
	LcTColor value = getFontColor();
	if (value != LcTColor(LcTColor::NONE))
		return value;

	// Get default value from LAF if attached
	if (m_space)
		return m_space->getDefaultFontColor();
	else
		return LcTColor::BLACK;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTColor LcCApplet::findFontColorDisabled()
{
	// Get default value from LAF if attached
	if (m_space)
		return m_space->getDefaultFontColorDisabled();
	else
		return LcTColor::BLACK;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCBitmap* LcCApplet::getBitmap(const LcTmString& file)
{
	LcCBitmap* pBitmap = NULL;
	LcTaString path = file;

	if (m_space)
	{
		// Check if an absolute path has been supplied (must have drive specified)
		// We will just use the supplied path if so
		if (path[1] == ':')
		{
			// Then try the look and feel path
			path = m_space->getImagePath() + file;
		}

		pBitmap = m_space->getBitmap(path);

		return pBitmap;
	}

	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCFont* LcCApplet::getFont(const LcTmString& name, LcCFont::EStyle style)
{
	LcCFont* pFont = NULL;
	LcTaString path = name;

	if (m_space)
	{
		// Check if an absolute path has been supplied
		// We will just use the supplied path if so
		// If * its a system font
		if ((path[1] == ':') && (path[0] != '*'))
		{
			// Then try the look and feel path
			path = m_space->getImagePath() + name;
		}

		pFont = m_space->getFont(path, style);

		return pFont;
	}

	return NULL;
}

#ifdef LC_USE_MESHES
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCMesh* LcCApplet::getMesh(const LcTmString& file)
{
	LcCMesh* pMesh = NULL;
	LcTaString path = file;

	if (m_space)
	{
		// Check if an absolute path has been supplied (must have drive specified)
		// We will just use the supplied path if so
		if (path[1] == ':')
		{
			// Then try the look and feel path
			path = m_space->getImagePath() + file;
		}

		pMesh = m_space->getMesh(path);
		if (pMesh)
		{
			pMesh->initializeData(NULL);
			return pMesh;
		}
	}

	return NULL;
}

#endif

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCApplet::checkFileExists(const LcTmString& file)
{
	bool fileExists = false;

	// Check all read-only file types
	LcTaOwner<LcCReadOnlyFile> f = LcCReadOnlyFile::openFile(file);
	if (f)
		fileExists = true;

	return fileExists;
}
