
#ifndef __WEBSOCKET_HH__
#define __WEBSOCKET_HH__
#include "ProtocolsBase.h"
#include <vector>

typedef std::map<std::string, std::string> HEADER_MAP;
#define MAGIC_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

class Websocket:public ProtocolsBase
{
public:
	Websocket(void);
	virtual ~Websocket(void);

	virtual int decode(char* recv_buffer, int &len,clienthandler_ptr conn);
	virtual void encode(char* data, int &len,SL_ByteBuffer& message);
	virtual void sendAuthMsgIfNeed(clienthandler_ptr conn);
	string parse_str();
	int fetch_http_info(char* recv_buf_);
	int handshark(char* recv_buffer, int &len,clienthandler_ptr conn);
	int fetch_fin(char * msg, int & pos);
	int fetch_opcode(char * msg, int & pos);
	int fetch_mask(char * msg, int & pos);
	int fetch_payload_length(char * msg, int & pos, unsigned long & payload_length_);
	int fetch_masking_key(char * msg, int & pos);
	int fetch_payload(char * msg, int & pos, char * payload_, unsigned int payload_length_, unsigned int max_length);

	HEADER_MAP m_header_map_;      //websocket protocol
	unsigned char fin_;            //websocket protocol
	unsigned char opcode_;         //websocket protocol
	unsigned char mask_;           //websocket protocol
	unsigned char masking_key_[4]; //websocket protocol
};

#endif //__WEBSOCKET_HH__

