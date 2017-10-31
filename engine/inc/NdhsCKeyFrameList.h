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
#ifndef NdhsCKeyFrameListH
#define NdhsCKeyFrameListH

class NdhsIPath;

/*-------------------------------------------------------------------------*//**
*/
class NdhsCKeyFrameList : public LcCBase
{

public:

	class TKeyFrame
	{
	public:
		// public, so no m_
		LcTScalar           position;
		LcTPlacement        placement;
		int                 mask;

		// default constructor
		TKeyFrame() { mask = LcTPlacement::ENone; }

		bool operator <  (const TKeyFrame& rhs) const { return (position <  rhs.position); }
	};

private:

	typedef LcTmArray<TKeyFrame>    TmAKeyFrames;
	TmAKeyFrames                    m_KeyFrames;

	// this mask stores the maximum possible set of attributes specified in XML.
	// if some attribute is missing from all keyframes, the corresponding bit will not be set.
	// A bit-wise OR of all original masks of all keyframes.

	int                             m_mask;
	bool							m_0Added;
	bool							m_1Added;

protected:

	// Constructor
									NdhsCKeyFrameList();
	void                            construct();

	void                            interpolateMissingElements(int mask);

public:

	// Destruction
	virtual							~NdhsCKeyFrameList();

	// Construction
	LC_IMPORT static LcTaOwner<NdhsCKeyFrameList> create();

	// Returns an int (handle) that identifies the key frame,
	// later used to set individual attributes.
	LC_IMPORT       void            addKeyFrame(const LcTScalar position,
												const int mask,
												const LcTPlacement& placement);
	inline          int             numberOfKeyFrames()			{ return (int)m_KeyFrames.size(); }

	// Processes the added key frames to interpolate values.
	// This function must be called once, only.
	LC_IMPORT       void            processKeyFrames();

	LC_IMPORT       int             getPlacement(	const LcTScalar position,
													LcTPlacement& newPlacement,
													LcTColor startColor,
													LcTColor endColor,
													LcTColor startColor2,
													LcTColor endColor2) const;
	inline 			int 			getMask() const			    {return m_mask;}

};

#endif // NdhsCKeyFrameListH
