
#ifndef __SHARE_BOUND_PTR_H_20080822__
#define __SHARE_BOUND_PTR_H_20080822__


//
template<class TTLOCK>
class LockRef
{
private:
	long   _ref;
	TTLOCK _lock;
public:
	LockRef(long l)
	{
		//_lock.init();
		_ref = l;
	}
	~LockRef(void)
	{
		//_lock.destroy();
	}

	// ++操作符 
	LockRef& operator ++ () {
		this->_lock.lock();
		this->_ref ++;
		this->_lock.unlock();
		return *this;
	}

	// --操作符
	long operator -- () {
		long ret;
		this->_lock.lock();
		this->_ref --;
		ret = this->_ref;
		this->_lock.unlock();
		return ret;
	}
};

//
template<class T>
class CountedPtr 
{ 
private:
	T * ptr; 
	long * counter; 
public: 
	//构造 
	explicit CountedPtr(T* p = NULL) 
		:ptr(p),counter(new long(1))
	{} 
	//析构 
	~CountedPtr() {Release();} 
	//拷贝构造 
	CountedPtr(const CountedPtr<T>& p) 
		:ptr(p.ptr),counter(p.counter) 
	{++*counter;} 
	//=操作符 
	CountedPtr<T>& operator= (const CountedPtr<T>& p) { 
		if(this!=&p) { 
			Release();  
			ptr = p.ptr; 
			counter = p.counter;
			++*counter; 
		} 
		return *this; 
	} 
	//其它略...
private: 
	void Release() { 
		if(--*counter == 0) { 
			delete counter; 
			delete ptr; 
		}  
	} 
};

//一定要实现默认copy函数,防止系统自动调用默认copy函数和赋值函数,缺显著调用两次析构函数(如果类中有指针的话)
template<class T, class TTLOCK>
class SafeCountedPtr
{
private:
	T* ptr;
	LockRef<TTLOCK> * counter;
public:
	//构造 
	explicit SafeCountedPtr(T* p = NULL) 
		:ptr(p),counter(new LockRef<TTLOCK> (1))
	{} 
	//析构 
	~SafeCountedPtr() {Release();} 

	//!!默认拷贝构造 
	SafeCountedPtr(const SafeCountedPtr<T, TTLOCK>& p) 
		:ptr(p.ptr),counter(p.counter) 
	{ 
		++*counter; 
	} 
	//=操作符 
	SafeCountedPtr<T,TTLOCK>& operator= (const SafeCountedPtr<T,TTLOCK>& p) { 
		if(this!=&p) { 
			Release();  
			ptr = p.ptr; 
			counter = p.counter;
			++*counter; 
		} 
		return *this; 
	} 

	//
	T* get() const
	{	// return wrapped pointer
		return this->ptr;
	};

	//
	void reset(T* p=0) {
		if( this->get() == p)
			return;
		this->Release();
		this->counter= new LockRef<TTLOCK> (1);
		this->ptr = p.ptr; 
	};

	// ->操作符
	T *operator-> (void) const
	{
		return this->ptr;
	};
    

private: 
	void Release() { 
		if(--*counter == 0) { 
			delete counter; 
			if(ptr != 0)
			    delete ptr; 
		}  
	} 
};




#endif // __SHARE_BOUND_PTR_H_20080822__

