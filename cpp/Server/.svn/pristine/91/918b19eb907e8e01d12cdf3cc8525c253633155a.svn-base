/*
 * tscon.hpp
 *
 *  Created on: 2016年6月23日
 *      Author: shuisheng
 */

#ifndef TSCON_HPP_
#define TSCON_HPP_

#include <map>
#include <vector>
using namespace std;
#include <boost/thread/mutex.hpp>

template<typename T>
class tsvector
{
public:
	tsvector<T>(){};
	~tsvector<T>(){};

	void push(const T &t) {
		boost::mutex::scoped_lock lock(mutex_);
		con.push_back(t);
	}

	void remove(const T &t) {
		boost::mutex::scoped_lock lock(mutex_);
		typename vector<T>::iterator it = std::find(con.begin(), con.end(), t);
		if (it != con.end())
			con.erase(it);
	}

	template<typename _InputIterator, typename _Function>
	_Function
	for_each(_InputIterator __first, _InputIterator __last, _Function __f) {
		boost::mutex::scoped_lock lock(mutex_);
		return for_each(__first, __last, __f);
	}

private:
	vector<T> con;
	boost::mutex mutex_;
};

template<typename key, typename value>
class tsmap
{
public:
	tsmap<key, value>(){};
	~tsmap<key, value>(){};

	typedef typename map<key, value>::iterator iterator;

	void push(const key &k, const value &val) {
		boost::mutex::scoped_lock lock(mutex_);
		con[k] = val;
	}

	bool remove(const key &k) {
		boost::mutex::scoped_lock lock(mutex_);
		iterator it = con.find(k);
		if (it != con.end()){
			con.erase(it);
			return true;
		}
		return false;
	}

	iterator find(const key &k){
		boost::mutex::scoped_lock lock(mutex_);
		iterator it = con.find(k);
		return it;
	}

	void erase(iterator it) {
		boost::mutex::scoped_lock lock(mutex_);
		con.erase(it);
	}

	map<key, value>& getcon() { return con; }
	boost::mutex& getmutex() { return mutex_; }
private:
	map<key, value> con;
	boost::mutex mutex_;
};

#endif /* TSCON_HPP_ */
