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
#ifndef	LcOglCSLTypeScalarH
#define	LcOglCSLTypeScalarH

#include "inflexionui/engine/inc/LcAll.h"

template <typename T>
class LcOglCSLTypeScalar : public LcOglCSLType
{
	T 				m_scalar;			// Current value
	T 				m_scalarDefault;	// Default value
	T 				m_scalarMin;		// Minimum allowable value
	T 				m_scalarMax;		// Maximum allowable value

protected:
	LC_IMPORT   LcOglCSLTypeScalar()    
	{
		m_dimension = 1;
		m_locationIndex = -1;
		m_slType = ELcOglSLTypeScalar;
	}

public:

	LC_IMPORT static LcTaOwner<LcOglCSLTypeScalar<T> > create()
	{
		LcTaOwner< LcOglCSLTypeScalar<T> > ref;
		ref.set(new	LcOglCSLTypeScalar<T>()	);
		return ref;
	}

	LC_EXPORT_VIRTUAL ~LcOglCSLTypeScalar<T>()
	{
		// clear resources,	unload everything related to this effect
	}

	LC_IMPORT LcOglCSLTypeScalar<T>	operator =(const LcOglCSLTypeScalar<T> object)
	{
		if(this	!= &object)
			this->m_scalar = object.scalar;

		return *this;
	}

	inline T getValue()			const	{	return (m_scalar);			}
	inline T getDefaultValue()	const	{	return (m_scalarDefault);	}
	inline T getMinValue()		const	{	return (m_scalarMin);		}
	inline T getMaxValue()		const	{	return (m_scalarMax);		}

	LC_IMPORT void setValue(T* value)
	{
		m_scalar = *value;
	}

	LC_IMPORT void setDefaultValue(T* value)
	{
		m_scalarDefault =	*value;
	}

	LC_IMPORT void setMinValue(T* value)
	{
		m_scalarMin =	*value;
	}

	LC_IMPORT void setMaxValue(T* value)
	{
		m_scalarMax =	*value;
	}


	inline ELcOglSLType	getSLType()	{	return ELcOglSLTypeScalar; }

	inline	int	getDimension()					{	return (m_dimension);	}

	inline LcTmString getSLTypeName()			{	return m_name;	}

	inline int getLocationIndex()			   {	return m_locationIndex;	}

	inline LcTmString  getSLTypeIdentifier(void)  {	   return m_identifier;	 }
	
	inline bool isInterpolatable(void)
	{
		// Only scalar float SL types are interpolatable in Inflexion
		return (m_identifier.compareNoCase("float") == 0);
	}

	LC_IMPORT void setSLType(ELcOglSLType slType)
	{
		m_slType = slType;
	}

	LC_IMPORT void setDimension(int	dimension)
	{
		m_dimension	= dimension;
	}

	LC_IMPORT void setSLTypeName(LcTmString	name)
	{
		m_name = name;
	}

	LC_IMPORT void setLocationIndex(int	loc	= -1)
	{
		m_locationIndex =	loc;
	}

	LC_IMPORT void setSLTypeIdentifier(LcTmString& identifier)
	{
		m_identifier = identifier;
	}
};

#endif /* LcOglCSLTypeScalarH */
