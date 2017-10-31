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
#ifndef LcTTimeH
#define LcTTimeH

/*-------------------------------------------------------------------------*//**
	Encapsulates a time value to cope with wrapping of unsigned 32 bit integers.
	Use wherever you require a timestamp value.
*/
class LcTTime
{
public:

	unsigned int						m_time;
	int									m_wrapCount;

public:

	// Construction
	inline								LcTTime()
											{
												m_wrapCount = 0;
												m_time = 0;
											}
	inline								LcTTime(const LcTTime& T)
											{
												m_wrapCount = T.m_wrapCount;
												m_time = T.m_time;
											}
template<typename T>
	inline								LcTTime(T timeVal)
											{
												if (timeVal < 0)
												{
													m_wrapCount = -1 + (int)(((-timeVal) / UINT_MAX));
													m_time = UINT_MAX - (-((int)timeVal)) % UINT_MAX;
												}
												else
												{
													m_wrapCount = (int)(timeVal / UINT_MAX);
													m_time = ((int)timeVal) % UINT_MAX;
												}
											}

	// Comparison of LcTTime objects
	inline				bool			operator==(const LcTTime T) const
											{
												if (T.m_wrapCount > m_wrapCount)
													return false;
												else if (T.m_wrapCount < m_wrapCount)
													return false;
												else
													return T.m_time == m_time;
											}
	inline				bool			operator<=(const LcTTime T) const
											{
												if (T.m_wrapCount > m_wrapCount)
													return true;
												else if (T.m_wrapCount < m_wrapCount)
													return false;
												else
													return T.m_time >= m_time;
											}
	inline				bool			operator>=(const LcTTime T) const
											{
												if (T.m_wrapCount < m_wrapCount)
													return true;
												else if (T.m_wrapCount > m_wrapCount)
													return false;
												else
													return T.m_time <= m_time;
											}
	inline				bool			operator<(const LcTTime T) const
											{ return !(T <= *this); }
	inline				bool			operator>(const LcTTime T) const
											{ return !(T >= *this); }

	// Comparison functions for other types
template<typename T>
	inline				bool			operator==(T timeVal) const
											{ return (LcTTime(timeVal) == *this); }
template<typename T>
	inline				bool			operator>=(T timeVal) const
											{ return (*this >= LcTTime(timeVal)); }
template<typename T>
	inline				bool			operator<=(T timeVal) const
											{ return (*this <= LcTTime(timeVal)); }
template<typename T>
	inline				bool			operator>(T timeVal) const
											{ return (*this > LcTTime(timeVal)); }
template<typename T>
	inline				bool			operator<(T timeVal) const
											{ return (*this < LcTTime(timeVal)); }

	// Arithmetic functions
	inline				LcTTime			operator+(const LcTTime T) const
											{
												LcTTime temp;
												temp.m_wrapCount = this->m_wrapCount + T.m_wrapCount;

												// Check for unsigned int wrap
												if (this->m_time + T.m_time < this->m_time)
												{
													unsigned int remainder = UINT_MAX - T.m_time;
													temp.m_time = this->m_time - remainder;
													temp.m_wrapCount++;
												}
												else
												{
													temp.m_time = this->m_time + T.m_time;
												}

												return temp;
											}
template<typename T>
	inline				LcTTime			operator+(T timeVal) const
											{
												LcTTime temp = LcTTime(timeVal);
												temp = *this + temp;
												return temp;
											}

	inline				LcTTime			operator-(const LcTTime T) const
											{
												LcTTime temp;
												temp.m_wrapCount = this->m_wrapCount - T.m_wrapCount;

												if (this->m_time < T.m_time)
												{
													unsigned int remainder = T.m_time - this->m_time;
													temp.m_time = UINT_MAX - remainder;
													temp.m_wrapCount--;
												}
												else
												{
													temp.m_time = this->m_time - T.m_time;
												}

												return temp;
											}
template<typename T>
	inline				LcTTime			operator-(T timeVal) const
											{
												LcTTime temp = LcTTime(timeVal);
												temp =  *this - temp;
												return temp;
											}

template<typename T>
	inline				LcTTime			operator=(T timeVal)
											{
												*this = LcTTime(timeVal);
												return *this;
											}

template<typename T>
	inline				LcTTime			operator+=(T timeVal)
											{
												*this = *this + LcTTime(timeVal);
												return *this;
											}

template<typename T>
	inline				LcTTime			operator-=(T timeVal)
											{
												*this = *this - LcTTime(timeVal);
												return *this;
											}

	// Casting to other types - Note that casting to 32 bit integers may wrap
	inline								 operator void() const {}
template<typename T>
	inline								 operator T() const
											{
												return (T)UINT_MAX * m_wrapCount + (T)m_time;
											}
};

#endif // LcTTimeH
