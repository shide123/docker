/*
 * Header.h
 *
 *  Created on: Sep 20, 2016
 *      Author: boa
 */

#ifndef PROTOCOLS_HEADER_H_
#define PROTOCOLS_HEADER_H_

#include "ProtocolsBase.h"
#include "stock_comm.h"

class DCL: public virtual ProtocolsBase {
public:
	DCL();
	virtual ~DCL();
	virtual int decode(char* recv_buffer, int &len,clienthandler_ptr conn);
	virtual void encode(char* data, int &len,SL_ByteBuffer& message);
};

#endif /* PROTOCOLS_HEADER_H_ */
