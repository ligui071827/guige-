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
#ifndef	LcOglCSLTypeMatrixH
#define	LcOglCSLTypeMatrixH

#include "inflexionui/engine/inc/LcOglCSLType.h"

template <class	T>
class LcOglCSLTypeMatrix : public LcOglCSLType
{
	LcTmAlloc<T>	m_pValues;

protected:
	LC_IMPORT	LcOglCSLTypeMatrix() 
	{
		m_locationIndex = -1;
		m_slType = ELcOglSLTypeMatrix;
	}

	LC_IMPORT void construct(int dimension)
	{
		int	order =	dimension *	dimension;

		m_pValues.alloc(order);

		// Set the dimension of	this vector.
		this->setDimension(order);
	}

public:
	LC_IMPORT static LcTaOwner<	LcOglCSLTypeMatrix<T> >	create(int dimension)
	{
		LcTaOwner< LcOglCSLTypeMatrix<T> > ref;
		ref.set(new	LcOglCSLTypeMatrix<T>());
		ref->construct(dimension);
		return ref;
	}

	LC_EXPORT_VIRTUAL ~LcOglCSLTypeMatrix<T>()
	{
		// clear resources,	unload everything related to this effect
		if(m_pValues)
		{
			m_pValues.free();
		}
	}

	LC_IMPORT T& operator [](int index)
	{
		if(index>=0	&& index< m_dimension)
		{
			return m_pValues[index];
		}
		else
		{
			return m_pValues[m_dimension-1];
		}
	}

	inline int getDimension()
	{
		return (m_dimension);
	}

	LC_IMPORT void setDimension(int	dim)
	{
		this->m_dimension	= dim;
	}

	LC_IMPORT void getValue(T *matrix)
	{
		for(int	i=0; i<m_dimension;	i++)
		{
			matrix[i] =	m_pValues[i];
		}
	}

	LC_IMPORT void setValue(T *matrix)
	{
		for(int	i=0; i<m_dimension;	i++)
		{
			m_pValues[i] = matrix[i];
		}
	}

	LC_IMPORT void setSLType(ELcOglSLType slType)				{	m_slType = slType;			}
	LC_IMPORT void setSLTypeName(LcTmString	name)				{	m_name = name;				}
	LC_IMPORT void setLocationIndex(int	loc)					{	m_locationIndex	=	loc;	}
	LC_IMPORT void setSLTypeIdentifier(LcTmString& identifier)	{	m_identifier = identifier;	}

	inline ELcOglSLType	getSLType()								{	return ELcOglSLTypeMatrix;	}
	inline LcTmString	getSLTypeName()							{	return m_name;				}
	inline int			getLocationIndex()						{	return m_locationIndex;		}
	inline LcTmString	getSLTypeIdentifier(void)				{	return m_identifier;		}
	
	inline bool isInterpolatable(void)
	{
		// Matrices despite being float, are currently not interplatable in Inflexion
		return false;
	}
};

#endif /* LcOglCSLTypeMatrixH */
