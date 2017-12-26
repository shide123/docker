/*
 * Header.h
 *
 *  Created on: Sep 20, 2016
 *      Author: boa
 */

#ifndef __HTTP_HH__
#define __HTTP_HH__

#include "ProtocolsBase.h"

class Http: public virtual ProtocolsBase {
public:
	virtual int decode(char* recv_buffer, int &len,clienthandler_ptr conn);
	virtual void encode(char* data, int& len,SL_ByteBuffer& message);
};

#endif /* __HTTP_HH__ */
