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
#ifndef NdhsCEntryPointMapStackH
#define NdhsCEntryPointMapStackH

class NdhsCEntryPointMapStack : public LcCBase
{
private:

	typedef LcTmMap<LcTmString, LcTmString>	TmMPairs;

	// List of key-value pairs
	TmMPairs				m_pairs;

	// Two-phase construction
							NdhsCEntryPointMapStack() {}
	void					construct();

public:

	// Create/destruct
	static LcTaOwner<NdhsCEntryPointMapStack> create();
	virtual 				~NdhsCEntryPointMapStack();

	bool					pushMap(const LcTmString& id, const LcTmString& nodeUri);
	void 					clear();

	LcTaString				getEntryPoint(const LcTmString& id);
	LcTaString				getTokenValue(const LcTmString& id) { return getEntryPoint(id); }
};

#endif //NdhsCEntryPointMapStackH
