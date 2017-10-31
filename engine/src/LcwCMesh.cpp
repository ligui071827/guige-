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


#if defined(LC_PLAT_OGL) && defined(LC_USE_MESHES)

#include <math.h>

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcwCMesh> LcwCMesh::create()
{
	LcTaOwner<LcwCMesh> ref;
	ref.set(new LcwCMesh);
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcwCMesh::LcwCMesh()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcwCMesh::~LcwCMesh()
{
	if (m_mesh)
	{
		m_mesh->release();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCMesh::releaseResources()
{
	if(m_mesh)
	{
		m_mesh->releaseMeshResources();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCMesh::reloadResources()
{
	if(m_mesh)
	{
		m_mesh->reloadMeshResources();
	}
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	configEffectUniforms(getSpace(),"");
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCMesh::onRealize()
{
	LcCSpace* sp = getSpace();

	// We now know the space we are using, so check the spaces match
	if (m_mesh && m_mesh->getSpace() != sp)
	{
		m_mesh->release();
		m_mesh = NULL;
		return;
	}

	sp->addWidget(this);
}

/*-------------------------------------------------------------------------*//**
	Return whether the mesh is at the specified point.
*/
LC_EXPORT_VIRTUAL bool LcwCMesh::contains(const LcTVector& loc, LcTScalar expandRectEdge)
{
	if (getTappable() == EOff)
		return false;

	// check vector pos is within the unit bounding box at meshes core.
	if (loc.magnitude() < 1.0)
		return true;

	return false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcwCMesh::setMesh(LcCMesh* m)
{
	// Do nothing if mesh isn't changing
	if (m_mesh == m)
		return;

	// Detach from previous
	if (m_mesh)
	{
		m_mesh->release();
		m_mesh = NULL;
	}

	// If we are setting a mesh rather than clearing
	if (m)
	{
		// Check that mesh belongs to same space as widget
		// NB: validation important to ensure animation is done on same space
		LcCSpace* sp = getSpace();
		if (!sp || m->getSpace() == sp)
		{
			// Yes, it does!
			m_mesh = m;
			m_mesh->acquire();
			m_mesh->attachWidget(this);

			// Force a re-evaluation of internal offset and scale to fit the
			// new mesh into the configured extent, if fixed by the layout
			onPlacementChange(0);
		}
	}

	// Trigger repaint
	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCMesh::onPlacementChange(int mask)
{
	LcTVector internalOffset(0, 0, 0);
	LcTVector internalExtent(getExtent()); // Effectively scale(1,1,1)

	if (!m_mesh)
		return;

	// If the extent was explicitly set by the layout rather than by extent hint,
	// calculate the adjustment position and scale to fit the mesh exactly
	// into the extent specified
	LcTScalar minValue;
	LcTScalar maxValue;

	// Calculate origin offset and internal extent in X
	minValue = m_mesh->getMinX();
	maxValue = m_mesh->getMaxX();
	if ((maxValue != 0) || (minValue != 0))
	{
		internalOffset.x = -1 * (minValue + ((maxValue - minValue) / 2));
		internalExtent.x = maxValue - minValue;
	}

	// Calculate origin offset and internal extent in Y
	minValue = m_mesh->getMinY();
	maxValue = m_mesh->getMaxY();
	if ((maxValue != 0) || (minValue != 0))
	{
		internalOffset.y = -1 * (minValue + ((maxValue - minValue) / 2));
		internalExtent.y = maxValue - minValue;
	}

	// Calculate origin offset and internal extent in Z
	minValue = m_mesh->getMinZ();
	maxValue = m_mesh->getMaxZ();
	if ((maxValue != 0) || (minValue != 0))
	{
		internalOffset.z = -1 * (minValue + ((maxValue - minValue) / 2));
		internalExtent.z = maxValue - minValue;
	}

	// Note that this forces matrices to be recomputed too
	setInternalOffset(internalOffset);
	setInternalExtent(internalExtent);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCMesh::onWantBoundingBox()
{
	if (m_mesh)
		getSpace()->extendBoundingBox(LcTVector(), m_mesh);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCMesh::onPaintOpaque(const LcTPixelRect& clip)
{
	if (m_mesh)
	{
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
		if (getEffectMakesTranslucent() == EEffectTranslucencyCacheUndefined)
		{
			m_effectMakesTranslucent = EEffectTranslucencyCacheOpaque;

			LcOglCContext *context = getSpace()->getOglContext();

			if ((context!=NULL) && context->getCustomEffectTranslucency(LIGHT00_EFFECT_INDEX, this))
			{
				m_effectMakesTranslucent = EEffectTranslucencyCacheTranslucent;
			}
		}
#endif		
		LcTScalar opacity = getOverallOpacity();
		m_cachedOpacity = opacity;
		m_useCachedOpacity = true;

		// Values between < 1.0 and LC_OPAQUE_THRESHOLD will be drawn with blending enabled.
		if (opacity < 1.0f || isTranslucent())
			return;

		// Pass in font color in case we want to use color
		m_mesh->draw(LcTVector(), findFontColor(), opacity, LcCMesh::ESubMeshDrawOpaqueOnly, m_antiAlias);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCMesh::onPaint(const LcTPixelRect& clip)
{
	if (m_mesh)
	{
		LcTScalar opacity = getOverallOpacity();
		m_useCachedOpacity = false;
		if (opacity < LC_TRANSPARENT_THRESHOLD)
			return;

		getSpace()->getOglContext()->setDepthMask(GL_TRUE);

		// If opacity is less than 1.0 then we must draw our opaque meshes with alpha blending enabled.
		if (opacity < 1.0f)
			m_mesh->draw(LcTVector(), findFontColor(), opacity, LcCMesh::ESubMeshDrawOpaqueOnly, m_antiAlias);

		if (opacity > LC_OPAQUE_THRESHOLD)
			opacity = 1;

		// Pass in font color in case we want to use color
		m_mesh->draw(LcTVector(), findFontColor(), opacity, LcCMesh::ESubMeshDrawTranslucentOnly, m_antiAlias);
		
		getSpace()->getOglContext()->setDepthMask(GL_FALSE);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCMesh::onPrepareForPaint()
{
}

#endif // #if defined(LC_PLAT_OGL) && defined(LC_USE_MESHES)
