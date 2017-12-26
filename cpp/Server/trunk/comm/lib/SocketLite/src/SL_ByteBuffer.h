#ifndef SOCKETLITE_BYTEBUFFER_H
#define SOCKETLITE_BYTEBUFFER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif
#include <stdlib.h>
#include <string.h>
#include "SL_DataType.h"

class SL_ByteBuffer_Interface
{
public:
    SL_ByteBuffer_Interface() 
    {
    }

    virtual ~SL_ByteBuffer_Interface() 
    {
    }

    virtual int clear() 
    { 
        return 0; 
    }

    virtual void reset() 
    {
    }

    virtual char* buffer() const 
    { 
        return NULL; 
    }

    virtual size_t buffer_size() const 
    { 
        return 0; 
    }

    virtual bool data(const char *data, size_t size, bool owner) 
    { 
        return false; 
    }

    virtual char* data() const 
    { 
        return NULL; 
    }

    virtual size_t data_size() const 
    { 
        return 0; 
    }

    virtual size_t capacity() const 
    { 
        return 0; 
    }

    virtual size_t data_begin() const 
    { 
        return 0; 
    }

    virtual void data_begin(size_t new_begin) 
    {
    }

    virtual size_t data_end() const 
    { 
        return 0; 
    }

    virtual void data_end(size_t new_end) 
    {
    }

    virtual bool empty() const 
    { 
        return true; 
    }

    virtual bool owner() const 
    { 
        return false; 
    }

    virtual void owner(bool owner) 
    {
    }

    virtual bool reserve(size_t new_size) = 0;
    virtual bool resize(int new_size) 
    { 
        return reserve(new_size); 
    }

    virtual bool write(const char *data, size_t size, int multiple) = 0;
    virtual bool write(const char *data)
    {
        return write(data, strlen(data), 100);
    }

    virtual bool write(char c)
    {
        return write((const char*)&c, sizeof(char), 100);
    }

    virtual bool write(short n)
    {
        return write((const char*)&n, sizeof(short), 100);
    }

    virtual bool write(int n)
    {
        return write((const char*)&n, sizeof(int), 100);
    }

    virtual bool write(int64 n)
    {
        return write((const char*)&n, sizeof(int64), 100);
    }

    virtual bool write(float f)
    {
        return write((const char*)&f, sizeof(float), 100);
    }

    virtual bool write(double d)
    {
        return write((const char*)&d, sizeof(double), 100);
    }
};

class SL_ByteBuffer
{
public:
	inline SL_ByteBuffer()
		: buffer_size_(0)
		, data_begin_(0)
		, data_end_(0)
		, owner_(true)
		, buffer_(NULL)
	{
	}

	inline SL_ByteBuffer(size_t len)
		: buffer_size_(0)
		, data_begin_(0)
		, data_end_(0)
		, owner_(true)
		, buffer_(NULL)
	{
        reserve(len);
	}

    inline SL_ByteBuffer(const SL_ByteBuffer &buf)
    {
        if (buf.owner_)
        {
            buf.owner_      = false;
            owner_          = true;
            buffer_         = buf.buffer_;
            buffer_size_    = buf.buffer_size_;
            data_begin_     = buf.data_begin_;
            data_end_       = buf.data_end_;
        }
        else
        {
            owner_          = false;
            buffer_         = NULL;
            buffer_size_    = 0;
            data_begin_     = 0;
            data_end_       = 0;
        }
    }

    inline SL_ByteBuffer& operator=(const SL_ByteBuffer &buf)
    {
        if (this != &buf)
        {
		    if ((owner_) && (NULL != buffer_) && (buffer_ != buf.buffer_))
		    {
			    free(buffer_);
		    }

            if (buf.owner_)
            {
                buf.owner_      = false;
                owner_          = true;
                buffer_         = buf.buffer_;
                buffer_size_    = buf.buffer_size_;
                data_begin_     = buf.data_begin_;
                data_end_       = buf.data_end_;
            }
            else
            {
                owner_          = false;
                buffer_         = NULL;
                buffer_size_    = 0;
                data_begin_     = 0;
                data_end_       = 0;
            }
        }
        return *this;
    }

	inline ~SL_ByteBuffer()
	{
		clear();
	}

	inline int clear()
	{
		if ((owner_) && (NULL != buffer_))
		{
			free(buffer_);
		}
		buffer_		    = NULL;
        owner_          = true;
		buffer_size_    = 0;
		data_begin_	    = 0;
		data_end_	    = 0;
        return 0;
	}

	inline void reset()
	{
        data_begin_     = 0;
        data_end_       = 0;
	}

	inline char* buffer() const
	{
		return buffer_;
	}

	inline size_t buffer_size() const
	{
		return buffer_size_;
	}

    bool data(const char *data, size_t size, bool owner = true)
    {
        if (size <= 0)
        {
            return false;
        }
        if (NULL == data)
        {
            return false;
        }
        if ( (data>=buffer_) && (data<(buffer_+buffer_size_)) )
        {
            return false;
        }

        if (owner)
        {
            char *newBuffer = NULL;
            if (owner_)
            {
                newBuffer = (char*)realloc(buffer_, size);
            }
            else
            {   
                newBuffer = (char*)malloc(size);
            }
            if (NULL == newBuffer)
            {
                return false;
            }
            memcpy(newBuffer, data, size);
            buffer_ = newBuffer;
        }
        else
        {
            if (owner_ && (buffer_ != NULL))
            {
                free(buffer_);
            }
            buffer_  = (char*)data;
        }
        buffer_size_ = size;
        owner_       = owner;
        data_begin_  = 0;
        data_end_    = size;
        return true;
    }

    inline char* data() const
    {
        return buffer_+data_begin_;
    }

	inline size_t data_size() const
	{
		return data_end_-data_begin_;
	}

    inline size_t capacity() const
    {
        return buffer_size_-data_end_;
    }

	inline size_t data_begin() const
	{ 
		return data_begin_;
	}

	void data_begin(size_t new_begin)
	{
		if (new_begin > buffer_size_)
        {
			new_begin = buffer_size_;
        }
		if (new_begin > data_end_)
        {
			data_end_ = new_begin;
        }
		data_begin_ = new_begin;
	}

	inline size_t data_end() const
	{
		return data_end_;
	}

	inline void data_end(size_t new_end)
	{
		if (new_end > buffer_size_)
        {
			new_end = buffer_size_;
        }
		if (new_end < data_begin_)
        {
			data_begin_ = new_end;
        }
		data_end_ = new_end;
	}

	inline bool empty() const
	{
		return ((data_end_-data_begin_)<1);
	}

	inline bool owner() const
	{
		return owner_;
	}

	inline void owner(bool owner)
	{
		owner_ = owner;
	}

	bool reserve(size_t new_size)
	{
        if ((owner_) && (buffer_size_<new_size))
        {
            char *new_buffer = NULL;
            if (NULL != buffer_)
            {
                new_buffer = (char*)realloc(buffer_, new_size);
            }
            else
            {
                new_buffer = (char*)malloc(new_size);
				if (new_buffer)
				{
					memset(new_buffer, 0, new_size);
				}
            }
            if (NULL == new_buffer)
            {
                return false;
            }
            buffer_      = new_buffer;
            buffer_size_ = new_size;
            data_begin_  = 0;
            data_end_    = 0;
        }
        return true;
	}

    inline bool resize(int new_size)
    {
        return reserve(new_size);
    }

	//multiple: 缓冲区扩大倍数
	bool write(const char *data, size_t size, int multiple = 100)
	{
        if (!owner_)
        {
            return false;
        }

        if ((buffer_size_-data_end_) >= size)
        {
            memcpy(buffer_+data_end_, data, size);
        }
        else
        {   
            size_t new_size;
            if (buffer_size_ > 0)
            {
                size_t i = buffer_size_*multiple/100;
                new_size = buffer_size_ + ((i>size)?i:size);
            }
            else
            {
                //新长度为size的2倍(size*2)
                new_size = (size<<1);
            }
            char *new_buffer = NULL;
            if (NULL != buffer_)
            {
                new_buffer =(char*)realloc(buffer_, new_size);
            }
            else
            {
                new_buffer =(char*)malloc(new_size);
            }
            if (NULL == new_buffer)
            {
                return false;
            }
            memcpy(new_buffer+data_end_, data, size);
            buffer_	        = new_buffer;
            buffer_size_    = new_size;
        }
        data_end_ += size;
        return true;
	}

    inline bool write(const char *data)
	{
		return write(data, strlen(data));
	}

	inline bool write(char c)
	{
		return write((const char*)&c, sizeof(char));
	}

	inline bool write(short n)
	{
		return write((const char*)&n, sizeof(short));
	}

	inline bool write(int n)
	{
		return write((const char*)&n, sizeof(int));
	}

    inline bool write(int64 n)
	{
        return write((const char*)&n, sizeof(int64));
	}

	inline bool write(float f)
	{
		return write((const char*)&f, sizeof(float));
	}

	inline bool write(double d)
	{
	    return write((const char*)&d, sizeof(double));
	}

private:	
    typedef char        value_type;
    typedef char*       pointer;
    typedef const char* const_pointer;
    typedef size_t      size_type;

private:
	size_t	        buffer_size_;	//缓冲区大小
	size_t	        data_begin_;    //数据开始位
	size_t	        data_end_;		//数据结束位
	mutable bool    owner_;			//表示是否拥有所有权
	char            *buffer_;		//缓冲区
};

#endif
