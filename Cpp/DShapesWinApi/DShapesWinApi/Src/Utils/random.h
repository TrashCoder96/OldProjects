#pragma once

/****************************************************************************
**
** Copyright (C) 2010 Daniel Yurlov
**
****************************************************************************/

namespace Utils {

	template <class T>
	class Random {
	public:
		explicit Random(T seed) : m_noize(seed), m_current(seed) {}

		T random();

	private:
		T                 m_current;
		unsigned long int m_noize;
	};


	template <class T>
	T Random<T>::random()
	{
		m_noize  += 7345132513;
		m_noize   = (8253729 * m_noize + 2396403);
		m_current = m_noize % 7345132567;
		return m_current;
	}

}