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
#ifndef LCOGLCSLTYPE_H
#define LCOGLCSLTYPE_H

#include "inflexionui/engine/inc/LcAll.h"

typedef enum
{
	ELcOglSLTypeScalar,
	ELcOglSLTypeVector,
	ELcOglSLTypeMatrix
		
}ELcOglSLType;

class LcOglCSLType : public LcCBase
{	
protected:
	LcTmString		m_name;
	LcTmString		m_identifier;
	ELcOglSLType	m_slType;
	int 			m_dimension;
	int 			m_locationIndex;
	
public:
	LcOglCSLType()			{}
    virtual ~LcOglCSLType()        {}

    virtual ELcOglSLType getSLType() = 0;
    virtual	void setSLType(ELcOglSLType slType) = 0;

    virtual int getDimension() = 0;
    virtual void setDimension(int dimension) = 0;	
    
    virtual	LcTmString getSLTypeName() = 0;
    virtual	void setSLTypeName(LcTmString name) = 0;
    
    virtual	int getLocationIndex() = 0;
    virtual	void setLocationIndex(int loc) = 0;

    virtual void setSLTypeIdentifier(LcTmString& identifier) = 0;
    virtual LcTmString  getSLTypeIdentifier(void) = 0; 
	
	virtual bool isInterpolatable(void) = 0; 
};

#endif	/* LCOGLCSLTYPE_H */
