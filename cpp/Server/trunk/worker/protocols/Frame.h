
#ifndef __FRAME_HH__
#define __FRAME_HH__
#include "ProtocolsBase.h"
#include <vector>

class Frame:public ProtocolsBase
{
public:
	Frame(void);
	virtual ~Frame(void);

//	virtual void input(char* recv_buffer, int &len);
	virtual int decode(char* recv_buffer, int &len,clienthandler_ptr conn);
	virtual void encode(char* data, int &len,SL_ByteBuffer& message);
};

#endif //__FRAME_HH__

