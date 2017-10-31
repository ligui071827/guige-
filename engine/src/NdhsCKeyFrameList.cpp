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


#include <math.h>

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<NdhsCKeyFrameList> NdhsCKeyFrameList::create()
{
	LcTaOwner<NdhsCKeyFrameList> ref;
	ref.set(new NdhsCKeyFrameList());
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCKeyFrameList::NdhsCKeyFrameList()
{
	m_mask = LcTPlacement::ENone;
	m_0Added = false;
	m_1Added = false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCKeyFrameList::construct()
{
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCKeyFrameList::~NdhsCKeyFrameList()
{
	m_KeyFrames.clear();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCKeyFrameList::addKeyFrame(const LcTScalar position, const int mask, const LcTPlacement& placement)
{
	TKeyFrame keyFrame;

	if (position == 0)
		m_0Added = true;

	if (position == 1)
		m_1Added = true;

	keyFrame.position   = position;
	keyFrame.mask       = mask;
	keyFrame.placement  = placement;

	// Add to path array
	m_KeyFrames.push_back(keyFrame);

	m_mask |= mask;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCKeyFrameList::processKeyFrames()
{
	TKeyFrame keyFrame;
	keyFrame.placement.extent = LcTVector(0, 0, 0);
	keyFrame.mask = m_mask & ~(LcTPlacement::EColor | LcTPlacement::EColor2);

	if (!m_0Added)
	{
		// Add initial key frame if not specified
		keyFrame.position = 0.0;
		m_KeyFrames.push_back(keyFrame);
	}

	if (!m_1Added)
	{
		// Add final key frame if not specified
		keyFrame.position = 1.0;
		m_KeyFrames.push_back(keyFrame);
	}

	// sort the array using our sort function
	IFX_ShellSort(m_KeyFrames.begin(), m_KeyFrames.end());

	// Set mask for the key frames @ position 0.0 and 1.0, based on m_mask.
	// Don't add 'color' for end points, as they'll be queried from base tween if not specified
	m_KeyFrames[0].mask |= m_mask & ~(LcTPlacement::EColor | LcTPlacement::EColor2);
	m_KeyFrames[m_KeyFrames.size()-1].mask |= m_mask & ~(LcTPlacement::EColor | LcTPlacement::EColor2);

	// interpolate missing attributes, only if its mask bit is set in m_mask
	if (m_mask & LcTPlacement::ELocation)
	{
		interpolateMissingElements(LcTPlacement::ELocation);
	}

	if (m_mask & LcTPlacement::EExtent)
	{
		interpolateMissingElements(LcTPlacement::EExtent);
	}

	if (m_mask & LcTPlacement::EScale)
	{
		interpolateMissingElements(LcTPlacement::EScale);
	}

	if (m_mask & LcTPlacement::EOrientation)
	{
		interpolateMissingElements(LcTPlacement::EOrientation);
	}

	if (m_mask & LcTPlacement::EOpacity)
	{
		interpolateMissingElements(LcTPlacement::EOpacity);
	}

	// Note: colors are not interpolated.

	if (m_mask & LcTPlacement::EFrame)
	{
		interpolateMissingElements(LcTPlacement::EFrame);
	}

	if (m_mask & LcTPlacement::EOffset)
	{
		interpolateMissingElements(LcTPlacement::EOffset);
	}

	if (m_mask & LcTPlacement::EIntensity)
	{
		interpolateMissingElements(LcTPlacement::EIntensity);
	}

	if (m_mask & LcTPlacement::EColor2)
	{
		interpolateMissingElements(LcTPlacement::EColor2);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCKeyFrameList::interpolateMissingElements(int mask)
{
	int xy0Index   = -1;
	int xyIndex    = -1;
	int xy1Index   = -1;
	int lpIndx;
	int lpIndx2;
	int size = (int)m_KeyFrames.size();

	for (lpIndx = 0; lpIndx < size; lpIndx++)
	{
		if ((m_KeyFrames[lpIndx].mask & mask) == LcTPlacement::ENone)
		{
			xyIndex = lpIndx;
			xy0Index = lpIndx - 1;

			// find xy1 (as xy0 & xy are already found)

			for (lpIndx2 = lpIndx + 1; lpIndx2 < size; lpIndx2++)
			{
				if ((m_KeyFrames[lpIndx2].mask & mask) != LcTPlacement::ENone)
				{
					xy1Index = lpIndx2;
					break;
				}
			}

			if ((xy0Index < 0) || (xyIndex < 0) || (xy1Index < 0))
			{
				// can't interpolate / interpolation not required
				return;
			}

			// interpolate the values

			LcTScalar d = (m_KeyFrames[xyIndex].position - m_KeyFrames[xy0Index].position) /
						  (m_KeyFrames[xy1Index].position - m_KeyFrames[xy0Index].position);

			m_KeyFrames[xyIndex].placement.interpolate(m_KeyFrames[xy0Index].placement,
													   m_KeyFrames[xy1Index].placement,
													   mask,
													   d);

			m_KeyFrames[xyIndex].mask |= mask;
		}

		// reset indices

		xyIndex = -1;
		xy0Index = -1;
		xy1Index = -1;
	}
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCKeyFrameList::getPlacement(const LcTScalar position,
									LcTPlacement& newPlacement,
									LcTColor startColor,
									LcTColor endColor,
									LcTColor startColor2,
									LcTColor endColor2) const
{
	int closestYoungerIndex = -1;
	int closestElderIndex = -1;
	LcTScalar deltaPos;
	LcTPlacement closestYounger = LcTPlacement();
	LcTPlacement closestElder = LcTPlacement();
	LcTPlacement tempPlacement = LcTPlacement();

	// extrapolation case: position  > 1.0
	if (position > 1.0)
	{
		closestYoungerIndex = (int)m_KeyFrames.size() - 2;
		closestElderIndex = (int)m_KeyFrames.size() - 1;
	}
	else if (position < 0.0)
	{
		//  extrapolation case: position  < 0.0
		closestYoungerIndex = 1;
		closestElderIndex = 0;
	}
	else
	{
		// interpolation cases
		// else find neighboring key frames
		unsigned int lpIndx;
		for (lpIndx = 0; lpIndx < m_KeyFrames.size(); lpIndx++)
		{
			if (m_KeyFrames[lpIndx].position == position)
			{
				// found an exact match
				newPlacement = m_KeyFrames[lpIndx].placement;

				if (!(m_KeyFrames[lpIndx].mask & LcTPlacement::EColor)
					&& (m_mask & LcTPlacement::EColor))
				{
					newPlacement.color = LcTColor::mixColors(startColor, endColor, position);
				}

				if (!(m_KeyFrames[lpIndx].mask & LcTPlacement::EColor2)
					&& (m_mask & LcTPlacement::EColor2))
				{
					newPlacement.color2 = LcTColor::mixColors(startColor2, endColor2, position);
				}

				return m_mask;
			}
			else if (m_KeyFrames[lpIndx].position > position)
			{
				closestElderIndex = lpIndx;
				closestYoungerIndex = lpIndx - 1;
				break;
			}
		}

		if ((closestYoungerIndex == -1) || (closestElderIndex == -1))
		{
			// some error in key frame specification...
			return LcTPlacement::ENone;
		}
	}

	// relative position between key frames
	deltaPos = (position - m_KeyFrames[closestYoungerIndex].position) /
			  (m_KeyFrames[closestElderIndex].position - m_KeyFrames[closestYoungerIndex].position);

	closestYounger = m_KeyFrames[closestYoungerIndex].placement;

	if (!(m_KeyFrames[closestYoungerIndex].mask & LcTPlacement::EColor)
		&& (m_mask & LcTPlacement::EColor))
	{
		closestYounger.color = LcTColor::mixColors(startColor, endColor, m_KeyFrames[closestYoungerIndex].position);
	}

	if (!(m_KeyFrames[closestYoungerIndex].mask & LcTPlacement::EColor2)
		&& (m_mask & LcTPlacement::EColor2))
	{
		closestYounger.color2 = LcTColor::mixColors(startColor2, endColor2, m_KeyFrames[closestYoungerIndex].position);
	}

	closestElder = m_KeyFrames[closestElderIndex].placement;

	if (!(m_KeyFrames[closestElderIndex].mask & LcTPlacement::EColor)
		&& (m_mask & LcTPlacement::EColor))
	{
		closestElder.color = LcTColor::mixColors(startColor, endColor, m_KeyFrames[closestElderIndex].position);
	}

	if (!(m_KeyFrames[closestElderIndex].mask & LcTPlacement::EColor2)
		&& (m_mask & LcTPlacement::EColor2))
	{
		closestElder.color2 = LcTColor::mixColors(startColor2, endColor2, m_KeyFrames[closestElderIndex].position);
	}

	tempPlacement.interpolate(closestYounger, closestElder, m_mask, deltaPos);

	newPlacement = tempPlacement;

	return m_mask;
}
