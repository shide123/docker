#pragma once
#include "../src/SL_ByteBuffer.h"
#include "../src/SL_Task.h"

class CTestTask : public SL_Task<SL_ByteBuffer>
{
public:
    CTestTask(void);
    virtual int svc(SL_ByteBuffer *buff)
    {
        printf("%s\n",buff->data());
        return 0;
    }
public:
    virtual ~CTestTask(void);
};
