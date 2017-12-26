
#ifndef __PROTOCOLS_HH__
#define __PROTOCOLS_HH__

#include <vector>
#include "clienthandler.h"
enum {
	TASK_MSGTYPE_UNKOWN =-1,
	TASK_MSGTYPE_NETMSG =1,
	TASK_MSGTYPE_LOCAL =2,
	TASK_MSGTYPE_SERVER_NETMSG =3,
	TASK_MSGTYPE_TIMER = 4,
};
class Application;
class IWorkerEvent;
typedef struct tag_task_proc_data
{
	int msgtye;
	char* pdata;
	int datalen;
	clienthandler_ptr connection;

	void resperrinf(uint16 errcode)
	{
		if (connection && datalen >= SIZE_IVM_HEAD_TOTAL)
			connection->resperrinf((COM_MSG_HEADER *)pdata, (ClientGateMask_t *)(pdata + SIZE_IVM_HEADER), errcode);
	}

	void sendMessage(SL_ByteBuffer &buf, bool bForce = true){
		if (connection)
			connection->write_message(buf, bForce);
	}

	void sendMessage(const char *pData, size_t len, bool bForce = true){
		if (pData && len && connection)
			connection->write_message(pData, len, bForce);
	}

	template<class Type>
	SL_ByteBuffer respProtobuf(Type resp, uint16 subcmd)
	{
		int nProtoLen = resp.ByteSize();
		SL_ByteBuffer buff(SIZE_IVM_HEAD_TOTAL + nProtoLen);
		buff.write(pdata, SIZE_IVM_HEAD_TOTAL);
		DEF_IVM_HEADER(pHead, buff.buffer());
		pHead->length = buff.buffer_size();
		pHead->subcmd = subcmd;
		resp.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
		sendMessage(buff.buffer(), buff.buffer_size());
		return buff;
	}

}task_proc_data;

typedef struct tag_task_proc_param
{
	int   message;
	int64 param1;
	int64 param2;
	char msg[0];
}task_proc_param;

class IWorkerEvent
{
public:
	virtual bool onWorkerStart(Application * worker){return true;}
	virtual bool onWorkerStop(Application * worker){return true;}
	virtual bool onConnect(clienthandler_ptr connection){return true;}
	virtual bool onMessage(task_proc_data * message){return true;}
	virtual bool onWrite(clienthandler_ptr connection, const char *pdata, unsigned int datalen, SL_ByteBuffer &outBuff){return false;}
	virtual bool onClose(clienthandler_ptr connection){return true;}
	virtual bool onError(clienthandler_ptr connection,int code,string msg){return true;}
	virtual bool onWorkerReload(Application * worker){return true;};
};

typedef bool (*parse_msg_func)(const char *, int, SL_ByteBuffer &);

class ProtocolsBase
{
public:
	ProtocolsBase(void);
	virtual ~ProtocolsBase(void);

	static ProtocolsBase* 	app;
	static ProtocolsBase* getInstance();

	virtual int decode(char* recv_buffer, int &len,clienthandler_ptr conn);
	virtual void encode(char* data, int len,SL_ByteBuffer& message);
	virtual void sendAuthMsgIfNeed(clienthandler_ptr conn);
	static ProtocolsBase* getProtocolByName(string protocolname);
	int handle_message(const char* pdata, int msglen,clienthandler_ptr conn);

	void setParseFunc(parse_msg_func func);

private:
	parse_msg_func parse_message;
};

#endif //____PROTOCOLS_HH____

