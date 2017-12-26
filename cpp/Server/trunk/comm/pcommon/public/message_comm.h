
#ifndef __MESSAGE_COMMON_HH__
#define __MESSAGE_COMMON_HH__

#include "yc_datatypes.h"

#define SIZE_IVM_HEADER           sizeof(COM_MSG_HEADER)
#define SIZE_IVM_CLIENTGATE       sizeof(ClientGateMask_t)
#define SIZE_IVM_HEAD_TOTAL 	  (SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE)
#define SIZE_IVM_REQUEST(pHead)   ((pHead)->length - SIZE_IVM_HEADER - SIZE_IVM_CLIENTGATE)
#define SIZE_IVM_NOMASK_REQUEST(pHead)   ((pHead)->length - SIZE_IVM_HEADER)
#define SIZE_IVM_TOTAL(s)         (SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + sizeof(s))

#define DEF_IVM_HEADER(x,y)       COM_MSG_HEADER* x = (COM_MSG_HEADER*)(y)
#define DEF_IVM_CLIENTGATE(x,y)   ClientGateMask_t* x = (ClientGateMask_t*)((y)+SIZE_IVM_HEADER);
#define DEF_IVM_DATA(y)           ((y) + SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE) 

#define SIZE_IVM_INDEX_HEADER     sizeof(COM_MSG_INDEX_HEADER)
//----------------------------------------------------------
#pragma pack(1)

//14 bytes
typedef struct tag_COM_MSG_HEADER
{
	int32  length;       //必须在这个位置
	uint8  version;      //版本号,当前为10
	uint8  checkcode;
	uint16 maincmd;     //主命令
	uint16 subcmd;      //子命令
#if (!defined(LIBNODE)) && (!defined(ALARMNOTIFY))
	uint32 reqid;		//请求id 
#endif
	char   content[0];  //内容
}COM_MSG_HEADER;

typedef struct tag_COM_MSG_AUTH_REQ
{
	char  content[33];
}COM_MSG_AUTH_REQ;

typedef struct tag_COM_MSG_AUTH_RESP
{
	char  ckey[33];
	char  content[33];
}COM_MSG_AUTH_RESP;
enum {
	PACK_ORDINARY = 0,
	PACK_REQ = 1,
	PACK_RESP
};
typedef struct tag_COM_MSG_INDEX_HEADER
{
	int32  length;       //必须在这个位置
	uint8  version;      //版本号,当前为10
	uint8  checkcode;	//是请求或是回应包（PACK_REQ为请求，PACK_RESP为回应）
	uint16 maincmd;     //主命令
	uint16 subcmd;      //子命令
	uint32 index;		//消息序号
	char   content[0];  //内容
}COM_MSG_INDEX_HEADER;
//6*8=48bytes
typedef struct tag_ClientGateMask
{
	uint64 param1;    //gate-obj
	uint64 param2;    //gate-connect-id
	uint64 param3;    
	uint64 param4;    //client-devtype(from gateway to server)
	uint64 param5;    //client-ip(from gateway to server)
	uint64 param6;    //client-port(from gateway to server)
}ClientGateMask_t;

#pragma pack()

//COM_MSG_HEADER + ClientGateMask_t + ...

#endif //__MESSAGE_COMMON_HH__
