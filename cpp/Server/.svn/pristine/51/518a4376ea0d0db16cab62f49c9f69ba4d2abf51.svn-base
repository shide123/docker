/*
 * DCL.cpp
 *
 *  Created on: Sep 20, 2016
 *      Author: boa
 */
#include <assert.h>
#include <string>
#include <sys/wait.h>
#include "DCL.h"
using namespace std;
DCL::DCL() {
	// TODO Auto-generated constructor stub

}

DCL::~DCL() {
	// TODO Auto-generated destructor stub
}

int DCL::decode(char* recv_buffer, int &len,clienthandler_ptr conn)
{
	char* p = recv_buffer;

	while(len >= sizeof(CMDHEAD)) {
//		int msgpacklen = *((int*)p);
		CMDHEAD* DCL = (CMDHEAD*)p;
		int msgpacklen = DCL->m_nLen + sizeof(CMDHEAD);

		if(msgpacklen <= 0 || msgpacklen > en_msgbuffersize) {
			len = 0;
			return -1;
		}
		else if(len < msgpacklen) {
			break;
		}
		else {
//			if(handle_message(p, msgpacklen) == -1) {
//				recv_buf_remainlen_ = 0;
//				return -1;
//			}
			handle_message(p, msgpacklen,conn);
			len -= msgpacklen;
			p += msgpacklen;
		}
	}

	if(len >= en_msgmaxsize) {
		len = 0;
		return -1;
	}

	if(p != recv_buffer && len > 0){
		memmove(recv_buffer, p, len);
	}
	return 0;
}

void DCL::encode(char* data, int &len,SL_ByteBuffer& message)
{
	message.write(data, len);
}
