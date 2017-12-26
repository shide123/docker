#ifndef MY_TASK_H
#define MY_TASK_H

#include "SL_ByteBuffer.h"
#include "SL_Task.h"

class My_Task : public SL_Task<SL_ByteBuffer>
{
public:
    My_Task(void);
    ~My_Task(void);

    int svc(SL_ByteBuffer &buf);

};

#endif