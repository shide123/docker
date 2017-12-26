
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

	// ++������ 
	LockRef& operator ++ () {
		this->_lock.lock();
		this->_ref ++;
		this->_lock.unlock();
		return *this;
	}

	// --������
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
	//���� 
	explicit CountedPtr(T* p = NULL) 
		:ptr(p),counter(new long(1))
	{} 
	//���� 
	~CountedPtr() {Release();} 
	//�������� 
	CountedPtr(const CountedPtr<T>& p) 
		:ptr(p.ptr),counter(p.counter) 
	{++*counter;} 
	//=������ 
	CountedPtr<T>& operator= (const CountedPtr<T>& p) { 
		if(this!=&p) { 
			Release();  
			ptr = p.ptr; 
			counter = p.counter;
			++*counter; 
		} 
		return *this; 
	} 
	//������...
private: 
	void Release() { 
		if(--*counter == 0) { 
			delete counter; 
			delete ptr; 
		}  
	} 
};

//һ��Ҫʵ��Ĭ��copy����,��ֹϵͳ�Զ�����Ĭ��copy�����͸�ֵ����,ȱ��������������������(���������ָ��Ļ�)
template<class T, class TTLOCK>
class SafeCountedPtr
{
private:
	T* ptr;
	LockRef<TTLOCK> * counter;
public:
	//���� 
	explicit SafeCountedPtr(T* p = NULL) 
		:ptr(p),counter(new LockRef<TTLOCK> (1))
	{} 
	//���� 
	~SafeCountedPtr() {Release();} 

	//!!Ĭ�Ͽ������� 
	SafeCountedPtr(const SafeCountedPtr<T, TTLOCK>& p) 
		:ptr(p.ptr),counter(p.counter) 
	{ 
		++*counter; 
	} 
	//=������ 
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

	// ->������
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

