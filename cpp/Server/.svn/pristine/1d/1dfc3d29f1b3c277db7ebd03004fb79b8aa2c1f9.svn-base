/*
 * singleton.h
 *
 *  Created on: 2016年7月11日
 *      Author: shuisheng
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_

#include <boost/thread/mutex.hpp>

// class SingleInstance
template <typename T> class SingleInstance
{
public:
	virtual ~SingleInstance(){};

	static T *Instance() {
		if (!m_app){
			boost::mutex::scoped_lock lock(inst_mutex);
			m_app = new T();
		}

		return m_app;
	}
protected:
	SingleInstance(){};
private:
	static T *m_app;
	static boost::mutex inst_mutex;
};

template <typename T> T* SingleInstance<T>::m_app = 0;
template <typename T> boost::mutex SingleInstance<T>::inst_mutex;


#endif /* SINGLETON_H_ */
