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
#ifndef	LcOglCSLTypeVectorH
#define	LcOglCSLTypeVectorH

#include "inflexionui/engine/inc/LcAll.h"
#include "inflexionui/engine/inc/LcOglCSLType.h"

template <class	T>
class LcOglCSLTypeVector : public LcOglCSLType
{
	LcTmAlloc<T>	m_pValues;
	LcTmAlloc<T>	m_pDefaultValues;
	LcTmAlloc<T>	m_pMinValues;
	LcTmAlloc<T>	m_pMaxValues;
	
protected:
	LC_IMPORT	LcOglCSLTypeVector()	
	{
		m_locationIndex = -1;
		m_slType = ELcOglSLTypeVector;
	}

	LC_IMPORT void construct(int dimension)
	{
		m_pValues.alloc(dimension);
		m_pDefaultValues.alloc(dimension);
		m_pMinValues.alloc(dimension);
		m_pMaxValues.alloc(dimension);
		
		// Set the dimension of	this vector.
		this->setDimension(dimension);
	}

public:
	LC_IMPORT static LcTaOwner<	LcOglCSLTypeVector<T> >	create(int dimension)
	{
		LcTaOwner< LcOglCSLTypeVector<T> > ref;
		ref.set(new	LcOglCSLTypeVector<T>());
		ref->construct(dimension);
		return ref;
	}

	LC_EXPORT_VIRTUAL ~LcOglCSLTypeVector<T>()
	{
		// clear resources,	unload everything related to this effect
		m_pValues.free();
		m_pDefaultValues.free();
		m_pMinValues.free();
		m_pMaxValues.free();
	}

	inline int getDimension()	{	return m_dimension;	}

	LC_IMPORT void setDimension(int	dim)
	{
		m_dimension	= dim;
	}

	LC_IMPORT T& operator [](int index)
	{
		if(index>=0	&& index< m_dimension)
		{
			return m_pValues[index];
		}
		else
		{
			return m_pValues[m_dimension - 1];
		}
	}

	// Retrieve	current	value(s)
	LC_IMPORT void getValue(T* vector)
	{
		for(int	i=0; i<m_dimension;	i++)
		{
			vector[i] =	m_pValues[i];
		}
	}

	// Retrieve	Default	value(s)
	LC_IMPORT void getDefaultValue(T* vector)
	{
		for(int	i=0; i<m_dimension;	i++)
		{
			vector[i] =	m_pDefaultValues[i];
		}
	}

	// Retrieve	Minimum	allowable value(s)
	LC_IMPORT void getMinValue(T* vector)
	{
		for(int	i=0; i<m_dimension;	i++)
		{
			vector[i] =	m_pMinValues[i];
		}
	}

	// Retrieve	Maximum	allowable value(s)
	LC_IMPORT void getMaxValue(T* vector)
	{
		for(int	i=0; i<m_dimension;	i++)
		{
			vector[i] =	m_pMaxValues[i];
		}
	}

	// Set current value(s)
	LC_IMPORT void setValue(T* vector)
	{
		for(int	i=0; i<m_dimension;	i++)
		{
			m_pValues[i] = vector[i];
		}
	}

	// Set Default value(s)
	LC_IMPORT void setDefaultValue(T* vector)
	{
		for(int	i=0; i<m_dimension;	i++)
		{
			m_pDefaultValues[i]	= vector[i];
		}
	}

	// Set Minimum allowable value(s)
	LC_IMPORT void setMinValue(T* vector)
	{
		for(int	i=0; i<m_dimension;	i++)
		{
			m_pMinValues[i]	= vector[i];
		}
	}

	// Set Maximum allowable value(s)
	LC_IMPORT void setMaxValue(T* vector)
	{
		for(int	i=0; i<m_dimension;	i++)
		{
			m_pMaxValues[i]	= vector[i];
		}
	}

	LC_IMPORT void setSLTypeIdentifier(LcTmString& identifier)
	{
		m_identifier = identifier;
	}

	LC_IMPORT void setSLType(ELcOglSLType slType)	{	m_slType = slType;			}
	LC_IMPORT void setSLTypeName(LcTmString	name)	{	m_name = name;				}
	LC_IMPORT void setLocationIndex(int	loc)		{	m_locationIndex	= loc;		}

	inline LcTmString	getSLTypeIdentifier(void)	{	return m_identifier;		}
	inline ELcOglSLType	getSLType()					{	return ELcOglSLTypeVector;	}
	inline LcTmString	getSLTypeName()				{	return m_name;				}
	inline int			getLocationIndex()			{	return m_locationIndex;		}
	
	inline bool isInterpolatable(void)
	{
		// Only float vectors are interpolatable in Inflexion
		return (m_identifier.compareNoCase("vec2")  == 0) ||
			   (m_identifier.compareNoCase("vec3")  == 0) ||
			   (m_identifier.compareNoCase("vec4")  == 0) ;
	}
};

#endif /* LcOglCSLTypeVectorH */
