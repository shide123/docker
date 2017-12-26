#ifndef SOCKETLITE_SINGLETON_H
#define SOCKETLITE_SINGLETON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif

template <typename TYPE>
class SL_Singleton
{
public:
	inline static TYPE* instance()
	{
	    static TYPE object_type;
        return &object_type;
	}
private:
	inline SL_Singleton() 
    {
    }

	inline ~SL_Singleton() 
    {
    }
};

#endif
