#ifndef __MACRO_DEFINE_H__
#define __MACRO_DEFINE_H__

#include "message_comm.h"
#include "message_vchat.h"
#define MAX_MESSAGE_SIZE (1024*8)
#define BROADCAST_ROOM_BEGIN_NUM	(1000000000)
/************************************************************************/
/*             从请求报文中获取相关结构指针  宏定义  BEGIN              */
/************************************************************************/
/*comment：                                                             */
/*    报头结构体 COM_MSG_HEADER                                         */
/*    路由结构体 ClientGateMask_t                                       */
/*    信令结构体 all in file "cmd_vchat.h"                              */
/************************************************************************/

#define INCLUED_RPC_NAMESPACE	using namespace std;	\
using namespace apache::thrift;	\
using namespace apache::thrift::protocol;	\
using namespace apache::thrift::transport;	\
using namespace TUsermgrIf;

#define SERIALIZETOARRAY_BUF(respInfo,respbuf,len)  char respbuf[8196] = {0};	\
				int len = respInfo.ByteSize();	\
				respInfo.SerializeToArray(respbuf,len);

#define SERIALIZETOARRAY_GATE(PROBUFCLASS,POINTER)	\
	char probuf[MAX_MESSAGE_SIZE];	\
	memset(probuf, 0, MAX_MESSAGE_SIZE); \
	int buflen = PROBUFCLASS.ByteSize();	\
	POINTER->length=SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + buflen;	\
	PROBUFCLASS.SerializeToArray(probuf,buflen);	\
	memcpy(POINTER->content + SIZE_IVM_CLIENTGATE,probuf,buflen);

#define SERIALIZETOARRAY_NO_GATE(PROBUFCLASS,POINTER)	\
	char probuf[MAX_MESSAGE_SIZE];	\
	memset(probuf, 0, MAX_MESSAGE_SIZE); \
	int buflen = PROBUFCLASS.ByteSize();	\
	POINTER->length=SIZE_IVM_HEADER + buflen;	\
	PROBUFCLASS.SerializeToArray(probuf,buflen);	\
	memcpy(POINTER+SIZE_IVM_HEADER,probuf,buflen);
//声明并获取“报头指针”、“路由指针”和“信令结构指针”
#define GEN_MSGINFO(data, pHead, pGateMask, pReq, ReqStruct) \
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER*)(data); \
	ClientGateMask_t* pGateMask = (ClientGateMask_t*)(pHead->content); \
	ReqStruct* pReq = (ReqStruct *)(pHead->content + SIZE_IVM_CLIENTGATE);

#define GEN_MSGINFO_PF(data, pHead, pGateMask, pReq, ReqStruct) \
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER*)(data); \
	ClientGateMask_t* pGateMask = (ClientGateMask_t*)(pHead->content); \
	ReqStruct pReq; \
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE,pHead->length - SIZE_IVM_REQUEST(pHead));
//（不声明）仅获取“报头指针”、“路由指针”和“信令结构指针”
#define GEN_MSGINFO_EX(data, pHead, pGateMask, pReq, ReqStruct) \
	pHead = (COM_MSG_HEADER*)(data); \
	pGateMask = (ClientGateMask_t*)(pHead->content); \
	pReq = (ReqStruct *)(pHead->content + SIZE_IVM_CLIENTGATE);

//声明并获取“报头指针”和“信令结构指针”
#define GEN_MSGINFO_HEAD_REQ(data, pHead, pReq, ReqStruct) \
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER*)(data); \
	ReqStruct* pReq = (ReqStruct *)(pHead->content + SIZE_IVM_CLIENTGATE);

//（不声明）仅获取“报头指针”和“信令结构指针”
#define GEN_MSGINFO_HEAD_REQ_EX(data, pHead, pReq, ReqStruct) \
	pHead = (COM_MSG_HEADER*)(data); \
	pReq = (ReqStruct *)(pHead->content + SIZE_IVM_CLIENTGATE);

//声明并获取“信令结构指针”
#define GEN_MSGINFO_REQ(data, pReq, ReqStruct) \
	ReqStruct* pReq = (ReqStruct *)(((COM_MSG_HEADER*)(data))->content + SIZE_IVM_CLIENTGATE);

//（不声明）仅获取“信令结构指针”
#define GEN_MSGINFO_REQ_EX(data, pReq, ReqStruct) \
	pReq = (ReqStruct *)(((COM_MSG_HEADER*)(data))->content + SIZE_IVM_CLIENTGATE);

//声明并获取“报头指针”、“路由指针”和“信令结构指针”，并校验数据长度是否合法（定长）
#define GEN_MSGINFO_AND_CHECK_LENGTH(data, pHead, pGateMask, pReq, ReqStruct) \
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER*)(data); \
	if (pHead->length < SIZE_IVM_TOTAL(ReqStruct)) \
	{ \
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_TOTAL(ReqStruct), pHead->length); \
		return -1; \
	} \
	ClientGateMask_t* pGateMask=(ClientGateMask_t*)(pHead->content); \
	ReqStruct* pReq = (ReqStruct *)(pHead->content + SIZE_IVM_CLIENTGATE);

//声明并获取“信令结构指针”，并校验数据长度是否合法（定长）
#define GEN_MSGREQ_AND_CHECK_LENGTH(data, pReq, ReqStruct) \
	if (((COM_MSG_HEADER*)(data))->length < SIZE_IVM_TOTAL(ReqStruct)) \
	{ \
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_TOTAL(ReqStruct), ((COM_MSG_HEADER*)(data))->length); \
		return -1; \
	} \
	ReqStruct* pReq = (ReqStruct *)(((char *)(data)) + SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE);


//校验请求报文变长长度的合法性
#define CHECK_VARIABLE_LENGTH(pHead, ReqStruct, lengthVariable) \
	if ((pHead)->length < SIZE_IVM_TOTAL(ReqStruct) + (lengthVariable)) \
	{ \
		LOG_PRINT(log_warning, "parse msginfo variable length error: need %d but recv %d!", SIZE_IVM_TOTAL(ReqStruct) + (lengthVariable), (pHead)->length); \
		return -1; \
	}

//声明并获取“报头指针”，设置报文头信息
#define GEN_MSGHEAD_AND_SET(data, pHead, mainCmd, subCmd) \
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER*)(data); \
	pHead->version = MDM_Version_Value; \
	pHead->checkcode = CHECKCODE; \
	pHead->maincmd = mainCmd; \
	pHead->subcmd = subCmd;

//声明并获取“报头指针”、“路由指针”和“信令结构指针”，设置报文头信息（仅适合定长结构体, 否则length需重新设置）
#define GEN_MSGINFO_AND_SETHEAD(data, pHead, pGateMask, pReq, ReqStruct, mainCmd, subCmd) \
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER*)(data); \
	pHead->version = MDM_Version_Value; \
	pHead->checkcode = CHECKCODE; \
	pHead->maincmd = mainCmd; \
	pHead->subcmd = subCmd; \
	pHead->length = SIZE_IVM_TOTAL(ReqStruct); \
	ClientGateMask_t *pGateMask=(ClientGateMask_t*)(pHead->content); \
	ReqStruct *pReq = (ReqStruct *)(pHead->content + SIZE_IVM_CLIENTGATE);

//声明并获取“报头指针”和“信令结构指针”，设置报文头信息（仅适合定长结构体, 否则length需重新设置）
#define GEN_MSGINFO_HEAD_REQ_SETHEAD(data, pHead, pReq, ReqStruct, mainCmd, subCmd) \
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER*)(data); \
	pHead->version = MDM_Version_Value; \
	pHead->checkcode = CHECKCODE; \
	pHead->maincmd = mainCmd; \
	pHead->subcmd = subCmd; \
	pHead->length = SIZE_IVM_TOTAL(ReqStruct); \
	ReqStruct *pReq = (ReqStruct *)(pHead->content + SIZE_IVM_CLIENTGATE);

//（不声明）仅获取“报头指针”、“路由指针”和“信令结构指针”，设置报文头信息（仅适合定长结构体, 否则length需重新设置）
#define GEN_MSGINFO_AND_SETHEAD_EX(data, pHead, pGateMask, pReq, ReqStruct, mainCmd, subCmd) \
	pHead = (COM_MSG_HEADER*)(data); \
	pHead->version = MDM_Version_Value; \
	pHead->checkcode = CHECKCODE; \
	pHead->maincmd = mainCmd; \
	pHead->subcmd = subCmd; \
	pHead->length = SIZE_IVM_TOTAL(ReqStruct); \
	pGateMask=(ClientGateMask_t*)(pHead->content); \
	pReq = (ReqStruct *)(pHead->content + SIZE_IVM_CLIENTGATE);

#define GEN_RSP_MSGINFO_SET_HEAD_GATE(data, mainCmd, subCmd, reqID, reqGate, pRspHead, pRspGateMask, ProtobufObj) \
	pRspHead = (COM_MSG_HEADER *)(data); \
	pRspHead->version = MDM_Version_Value; \
	pRspHead->checkcode = CHECKCODE; \
	pRspHead->maincmd = mainCmd; \
	pRspHead->subcmd = subCmd; \
	pRspHead->reqid = reqID; \
	pRspHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + ProtobufObj.ByteSize(); \
	pRspGateMask = (ClientGateMask_t *)(pRspHead->content); \
	memcpy(pRspGateMask, reqGate, SIZE_IVM_CLIENTGATE); \
	ProtobufObj.SerializeToArray(pRspHead->content + SIZE_IVM_CLIENTGATE, ProtobufObj.ByteSize());

#define WRITE_LOCK(mutex) boost::unique_lock<boost::shared_mutex> writeLock##__LINE__(mutex);
#define READ_LOCK(mutex) boost::shared_lock<boost::shared_mutex> readLock##__LINE__(mutex);

#define IF_METHOD_FALSE_RETURN(method, errCode) \
	if (!(method)) \
	{ \
		return errCode; \
	}

#define IF_METHOD_FALSE_RETURN_EX(method, errCode, format, args...) \
	if (!(method)) \
	{ \
		LOG_PRINT(log_error, "" format "", ##args); \
		return errCode; \
	}

#define IF_METHOD_FALSE_CONTINUE_EX(method, format, args...) \
	if (!(method)) \
	{ \
		LOG_PRINT(log_error, "" format "", ##args); \
		continue; \
	}

#define IF_METHOD_FALSE_BREAK_EX(method, format, args...) \
	if (!(method)) \
	{ \
		LOG_PRINT(log_error, "" format "", ##args); \
		break; \
	}

#define IF_METHOD_NOTZERO_RETURN(method, errCode) \
	if ((method)) \
	{ \
		return errCode; \
	}

#define IF_METHOD_NOTZERO_RETURN_EX(method, errCode, format, args...) \
	if ((method)) \
	{ \
		LOG_PRINT(log_error, "" format "", ##args); \
		return errCode; \
	}

#define IF_METHOD_MINUS_RETURN_EX(method, errCode, format, args...) \
	if ((method) < 0) \
	{ \
		LOG_PRINT(log_error, "" format "", ##args); \
		return errCode; \
	}

#define DELETE_POINT(pointor) \
	if (pointor) \
	{ \
		delete pointor; \
		pointor = NULL; \
	}

#define BOOL_TO_RESULTSTR(b) ((b) ? "success": "failed")

/************************************************************************/
/*             从请求报文中获取相关结构指针  宏定义  END                */
/************************************************************************/

#endif //__CMD_CHECK_H__
