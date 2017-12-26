#include <assert.h>
#include <string>
#include <sys/wait.h>
#include "Websocket.h"
#include "csha1.h"
#include "base64.h"
using namespace std;

Websocket::Websocket(void)
{
}


Websocket::~Websocket(void)
{

}

int Websocket::fetch_fin(char * msg, int & pos)
{
	fin_ = (unsigned char)msg[pos] >> 7;
	return 0;
}

int Websocket::fetch_opcode(char * msg, int & pos)
{
	opcode_ = msg[pos] & 0x0f;
	++pos;
	return 0;
}

int Websocket::fetch_mask(char * msg, int & pos)
{
	mask_ = (unsigned char)msg[pos] >> 7;
	return 0;
}

int Websocket::fetch_payload_length(char * msg, int & pos, unsigned long & payload_length_)
{
	payload_length_ = msg[pos] & 0x7f;
	++pos;
	if(payload_length_ == 126)
	{
		uint16_t length = 0;
		memcpy(&length, msg + pos, 2);
		pos += 2;
		payload_length_ = ntohs(length);
	}
	else if(payload_length_ == 127)
	{
		uint64_t length = 0;
		memcpy(&length, msg + pos, 8);
		pos += 8;
		payload_length_ = ntohl(length);
	}
	return 0;
}

int Websocket::fetch_masking_key(char * msg, int & pos)
{
	if(mask_ != 1)
		return 0;
	for(int i = 0; i < 4; i++)
		masking_key_[i] = msg[pos + i];
	pos += 4;
	return 0;
}

int Websocket::fetch_payload(char * msg, int & pos, char * payload_, unsigned int payload_length_, unsigned int max_length)
{
	if (payload_length_ > max_length)
	{
		LOG_PRINT(log_error, "payload_length_:%u is larger than max length:%u.", payload_length_, max_length);
		return -1;
	}

	if(mask_ != 1)
	{
		memcpy(payload_, msg + pos, payload_length_);
	}
	else
	{
		for(unsigned int i = 0; i < payload_length_; ++i)
		{
			int j = i % 4;
			payload_[i] = msg[pos + i] ^ masking_key_[j];
		}
	}
	pos += payload_length_;
	return 0;
}

int Websocket::decode(char* recv_buffer_, int &recv_buffer_remainlen_,clienthandler_ptr conn)
{
	if(en_session_status_connected != conn->session_status_)
	{
		if (handshark(recv_buffer_,recv_buffer_remainlen_,conn) >= 0)
		{
			ProtocolsBase::sendAuthMsgIfNeed(conn);
		}
		return 0;
	}
	char * msg = recv_buffer_;

	while(recv_buffer_remainlen_ > 2)
	{
		int pos = 0;
		int pre_pos = pos;
		//first byte
		fetch_fin(msg, pos);
		fetch_opcode(msg, pos);

		//second byte
		fetch_mask(msg, pos);

		if (opcode_ == 0x8) //websocket close
		{
			LOG_PRINT(log_error, "[clientobject:%x,client conn:%u,remote:%s:%u]Websocket receive close signal.", conn.get(), conn->getconnid(), conn->getremote_ip(), conn->getremote_port());
			return -2;
		}

		//get payload length
		unsigned long payload_length_ = 0;
		fetch_payload_length(msg, pos, payload_length_);
		if (payload_length_ <= 0 || payload_length_ >= en_msgbuffersize)
		{
			LOG_PRINT(log_error, "payload_length_:%u is wrong.", payload_length_);
			recv_buffer_remainlen_ = 0;
			return -1;
		}
		else if (payload_length_ > recv_buffer_remainlen_)
		{
			LOG_PRINT(log_error, "payload_length_:%u is larger than recv_buffer_remainlen_:%u !", payload_length_, recv_buffer_remainlen_);
			break;
		}
		else
		{
			fetch_masking_key(msg, pos);

			char payload_[en_msgbuffersize] = {0};
			if (fetch_payload(msg, pos, payload_, payload_length_, en_msgbuffersize))
			{
				recv_buffer_remainlen_ = 0;
				return -1;
			}

			handle_message(payload_, payload_length_, conn);

			int len = pos - pre_pos;
			pre_pos = pos;
			recv_buffer_remainlen_ -= len;
			msg += len;
		}
	}

	if(msg != recv_buffer_ && recv_buffer_remainlen_ > 0)
	{
		memmove(recv_buffer_, msg, recv_buffer_remainlen_);
	}
	return 0;
}

int Websocket::fetch_http_info(char* recv_buffer_)
{
	std::istringstream s(recv_buffer_);
	std::string request;

	std::getline(s, request);
	if (request[request.size() - 1] == '\r')
	{
		request.erase(request.end() - 1);
	}
	else
	{
		return -1;
	}

	std::string header; 

	std::string::size_type end;

	while (std::getline(s, header) && header != "\r")
	{
		if (header[header.size() - 1] != '\r')
		{
			continue; //end
		}
		else
		{
			header.erase(header.end() - 1);	//remove last char
		}

		end = header.find(": ",0);
		if (end != std::string::npos)
		{
			std::string key = header.substr(0, end);
			std::string value = header.substr(end + 2);
			m_header_map_[key] = value;
		}
	}

	return 0;
}

std::string Websocket::parse_str()
{
	std::string message = "";
	message += "HTTP/1.1 101 Switching Protocols\r\n";
	message += "Connection: upgrade\r\n";
	message += "Sec-WebSocket-Accept: ";

	std::string server_key = m_header_map_["Sec-WebSocket-Key"];
	server_key += MAGIC_KEY;
    
	unsigned int message_digest[5];
    CSHA1 sha;
	sha.Reset();
	sha << server_key.c_str();
	sha.Result(message_digest);
	for (int i = 0; i < 5; ++i)
	{
		message_digest[i] = htonl(message_digest[i]);
	}

	server_key = base64_encode(reinterpret_cast<const unsigned char*>(message_digest), 20);
	server_key += "\r\n";

	message += server_key;
	message += "Upgrade: websocket\r\n\r\n";

	return message;
}

int Websocket::handshark(char* recv_buffer_, int &recv_buffer_remainlen_,clienthandler_ptr conn)
{
	if (fetch_http_info(recv_buffer_))
	{
		LOG_PRINT(log_error, "fetch_http_info failed.");
		return -1;
	}

	//反向代理取源客户端ip
	if (m_header_map_.find("X-Forwarded-For") != m_header_map_.end() && m_header_map_["X-Forwarded-For"].size())
	{
		std::string ip = m_header_map_["X-Forwarded-For"];
		size_t pos = ip.find(',');
		if (pos != std::string::npos)
		{
			ip = ip.substr(0, pos);
		}
		int port = atoi(m_header_map_["X-Real-Port"].c_str());
		LOG_PRINT(log_info, "recv client from Nginx[%s:%d] Real-IP[%s:%d]", 
				conn->getremote_ip(), conn->getremote_port(), ip.c_str(), port);
		strncpy(conn->remote_ip, ip.c_str(), sizeof(conn->remote_ip) - 1);
		conn->remote_port = port;
	}

	std::string message = parse_str();

	memset(recv_buffer_, 0, sizeof(recv_buffer_));
	recv_buffer_remainlen_ = 0;

	boost::system::error_code ignored_error;
	conn->socket_.write_some(boost::asio::buffer(message), ignored_error);
	conn->session_status_ = en_session_status_connected;
	
	LOG_PRINT(log_info, "handshark with client[%s:%d] successfully", conn->getremote_ip(), conn->getremote_port());
	return 0;
}

void Websocket::encode(char* data, int &len,SL_ByteBuffer& message)
{
	message.write(data, len);
}

void Websocket::sendAuthMsgIfNeed(clienthandler_ptr conn)
{
// 	do after handshark
// 	if(Application::getInstance()->m_bAuth)
// 		conn->sendAuthmsg();
}
