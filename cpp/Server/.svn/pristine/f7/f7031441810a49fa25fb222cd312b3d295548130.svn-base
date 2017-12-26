#ifndef __MSGCOMMAPI_H__
#define __MSGCOMMAPI_H__

#include <list>
#include <vector>
#include <map>
#include <string>
#include <stdarg.h>
#include "utils.h"
#include "message_comm.h"
#include "message_vchat.h"
#include "SL_ByteBuffer.h"
#include "CLogThread.h"

using namespace std;

class CMsgComm 
{
public:
	CMsgComm() 
	{
	}

	~CMsgComm() 
	{
	}

	/*
	* @description : 根据长度适配包是否包含ClientGateMask_t，不可用在变长包上
	* @input param : pPkt         -- 包头
	* @input param : full_len     -- 包总长
	* @output param : pGateMask   -- ClientGateMask_t的位置，没有为NULL
	* @output param : pReq        -- 除去COM_MSG_HEADER和ClientGateMask_t后面第一位，没有为NULL
	* @input param : reqlen       -- 除去COM_MSG_HEADER和ClientGateMask_t 剩余的长度
	* @return: true               -- success
			   false              -- false
	*/
	template<typename Struct>
	inline static bool ReadHeader(char * pPkt, int full_len, ClientGateMask_t ** pGateMask, Struct ** pReq, int reqlen)
	{

		bool IsGatemask = true;
		if (full_len == sizeof(COM_MSG_HEADER) + sizeof(ClientGateMask_t) + reqlen)
		{
			*pGateMask = (ClientGateMask_t *)(pPkt);
			*pReq = (Struct *)(pPkt + SIZE_IVM_CLIENTGATE);
		} 
		else if (full_len == sizeof(COM_MSG_HEADER) + reqlen)
		{
			*pGateMask = NULL;
			*pReq = (Struct *)(pPkt);
		}
		else 
		{
			*pGateMask = NULL;
			*pReq = NULL;
			return false;
		}
		return true;
	}

	/*
	* @description : build COM_MSG_HEADER 包头
	* @input param : pPkt          -- 包头
	* @input param : checkcode     -- checkcode
	* @input param : version       -- version
	* @input param : maincmd       -- maincmd
	* @input param : subcmd        -- subcmd
	* @input param : pPktLen       -- 包总长，可以以后再填
	* @return: char*               -- 返回创建完成后COM_MSG_HEADER后面第一个字节
	*/
	static char * Build_COM_MSG_HEADER(char * pPkt, int checkcode, int version, int maincmd, int subcmd, int pPktLen);

	static char * Build_COM_MSG_HEADER(char * pPkt, int maincmd, int subcmd, int pPktLen = 0);

	/*
	* @description : 在pPkt后增加一个Struct
	* @input param : pPkt          -- 包头
	* @input param : pStruct       -- Struct
	* @return: char*               -- 返回后增加一个Struct后面第一个字节
	*/
	template<typename Struct>
	inline static char * Add_Struct(char * pPkt, Struct * pStruct) 
	{	
		if (pPkt == NULL || pStruct == NULL) return pPkt;
		memcpy(pPkt, pStruct, sizeof(Struct));
		return pPkt + sizeof(Struct);
	}

	template<typename Conn>
	inline static void send_data(SL_ByteBuffer & sendbuf, Conn * pToConn)
	{
		if (pToConn)
			pToConn->write_message(sendbuf);
		else
			LOG_PRINT(log_warning, "link error!");
	}

	/*
	* @description : 发送一个无内容包,包括ClientGateMask_t
	* @input param : maincmd            -- maincmd
	* @input param : subcmd             -- subcmd
	* @input param : pToConn            -- 链接
	* @input param : ClientGateMask_t   -- 网关
	*/
	template<typename Conn>
	inline static void SendHead(int maincmd, int subcmd, Conn * pToConn, ClientGateMask_t * pGateMask)
	{
		unsigned int msg_len = sizeof(COM_MSG_HEADER) + sizeof(ClientGateMask_t);
		SL_ByteBuffer sendbuf(msg_len);
		char * pPkt = sendbuf.buffer();
		pPkt = Build_COM_MSG_HEADER(pPkt, CHECKCODE, MDM_Version_Value, maincmd, subcmd, sizeof(COM_MSG_HEADER) + sizeof(ClientGateMask_t));
		Add_Struct(pPkt, pGateMask);
		sendbuf.data_end(msg_len);
		send_data(sendbuf, pToConn);
	}

	/*
	* @description : 发送一个无内容包,不包括ClientGateMask_t
	* @input param : maincmd            -- maincmd
	* @input param : subcmd             -- subcmd
	* @input param : pToConn            -- 链接
	*/
	template<typename Conn>
	inline static void SendHead(int maincmd, int subcmd, Conn * pToConn)
	{
		unsigned int msg_len = sizeof(COM_MSG_HEADER);
		SL_ByteBuffer sendbuf(msg_len);
		char * pPkt = sendbuf.buffer();
		Build_COM_MSG_HEADER(pPkt, CHECKCODE, MDM_Version_Value, maincmd, subcmd, sizeof(COM_MSG_HEADER));
		sendbuf.data_end(msg_len);
		send_data(sendbuf, pToConn);
	}

	/*
	* @description : 对szBuf填充内容:COM_MSG_HEADER + ClientGateMask_t + pData
	* @input param : szBuf				-- 填充内容的内存指针,需事先分配好
	* @input param : nBufLen			-- 事先分配好的内存大小
	* @input param : maincmd			-- maincmd
	* @input param : subcmd				-- subcmd
	* @input/output param : ppGateMask	-- 返回内容里的ClientGateMask_t的指针
	* @input param : pData				-- 指向数据的指针
	* @input param : pDataLen			-- 数据的长度
	* @return: int						-- 返回-1表示操作失败,除此返回COM_MSG_HEADER + ClientGateMask_t + pData的值
	*/
	static int Build_NetMsg(char * szBuf, unsigned int nBufLen, unsigned int mainCmdId, unsigned int subCmdId, ClientGateMask_t ** ppGateMask, void * pData, unsigned int pDataLen);
	
	/*
	* @description : 对pGateMask填充内容
	* @input/output param : pGateMask	-- 填充ClientGateMask_t的指针
	* @input param : e_dev_type			-- e_Notice_PC:0,e_Notice_Android:1,e_Notice_IOS:2,e_Notice_Web:3,e_Notice_AllType:4
	* @input param : roomid				-- 房间ID,如果为0,则为所有房间广播
	* @input param : except_connid		-- 如果不为0,广播则剔除该客户端连接
	* @return: void
	*/
	static void Build_BroadCastRoomGate(ClientGateMask_t * pGateMask, e_NoticeDevType e_dev_type, unsigned int roomid, unsigned int except_connid = 0, bool inGroupOnly = false);

	/*
	* @description : 对pGateMask填充内容
	* @input/output param : pGateMask	-- 填充ClientGateMask_t的指针
	* @input param : roomid				-- 房间ID
	* @input param : minRole			-- 最小角色值 >= minRole
	* @input param : maxRole			-- 最大角色值 <= maxRole, 0：表示无上限
	* @input param : e_dev_type			-- e_Notice_PC:0,e_Notice_Android:1,e_Notice_IOS:2,e_Notice_Web:3,e_Notice_AllType:4
	* @input param : userid				-- 需要剔除掉的userid
	* @return: void
	*/
	static void Build_BroadCastRoomRoleType(ClientGateMask_t * pGateMask, unsigned int roomid, unsigned int minRole, unsigned int maxRole = 0, e_NoticeDevType e_dev_type = e_Notice_AllType, unsigned int userid = 0);
	
	/*
	* @description : 对pGateMask填充内容
	* @input/output param : pGateMask	-- 填充ClientGateMask_t的指针
	* @input param : roomid				-- 房间ID
	* @input param : gender				-- 性别 0：未知  1：男  2：女
	* @return: void
	*/
	static void Build_BroadCastRoomGender_Gate(ClientGateMask_t * pGateMask, unsigned int roomid, unsigned int gender);

	/*
	* @description : 对pGateMask填充内容
	* @input/output param : pGateMask	-- 填充ClientGateMask_t的指针
	* @input param : e_dev_type			-- e_Notice_PC:0,e_Notice_Android:1,e_Notice_IOS:2,e_Notice_Web:3,e_Notice_AllType:4
	* @input param : except_connid		-- 如果不为0,广播则剔除该客户端连接
	* @return: void
	*/
	static void Build_BroadCastOnLine_Gate(ClientGateMask_t * pGateMask, e_NoticeDevType e_dev_type, unsigned int except_connid = 0);

	/*
	* @description : 对pGateMask填充内容
	* @input/output param : pGateMask	-- 填充ClientGateMask_t的指针
	* @input param : e_dev_type			-- e_Notice_PC:0,e_Notice_Android:1,e_Notice_IOS:2,e_Notice_Web:3,e_Notice_AllType:4
	* @input param : except_connid		-- 如果不为0,广播则剔除该客户端连接
	* @return: void
	*/
	static void Build_MultiCastOnLine_Gate(ClientGateMask_t * pGateMask, e_NoticeDevType e_dev_type, unsigned int except_connid = 0);

	/*
	* @description : 对pGateMask填充内容,订阅命令字的广播
	* @input/output param : pGateMask	-- 填充ClientGateMask_t的指针
	* @input param : e_dev_type			-- e_Notice_PC:0,e_Notice_Android:1,e_Notice_IOS:2,e_Notice_Web:3,e_Notice_AllType:4
	* @return: void
	*/
	static void Build_BroadCastSubscribeGate(ClientGateMask_t * pGateMask, e_NoticeDevType e_dev_type);

	/*
	* @description : 对pGateMask填充内容
	* @input/output param : pGateMask	-- 填充ClientGateMask_t的指针
	* @input param : userid				-- userid,如果为0,则为全广播
	* @input param : e_dev_type			-- e_Notice_PC:0,e_Notice_Android:1,e_Notice_IOS:2,e_Notice_Web:3,e_Notice_AllType:4
	* @input param : except_connid		-- 如果不为0,广播则剔除该客户端连接
	* @return: void
	*/
	static void Build_BroadCastUser_Gate(ClientGateMask_t * pGateMask, unsigned int userid, e_NoticeDevType e_dev_type = e_Notice_AllType, unsigned int except_connid = 0);

	/*
	* @description : 对pGateMask填充内容
	* @input/output param : pGateMask	-- 填充ClientGateMask_t的指针
	* @input param : e_dev_type			-- e_Notice_PC:0,e_Notice_Android:1,e_Notice_IOS:2,e_Notice_Web:3,e_Notice_AllType:4
	* @input param : except_connid		-- 如果不为0,广播则剔除该客户端连接
	* @return: void
	*/
	static void Build_BroadCastUsersOnOneSvr_Gate(ClientGateMask_t * pGateMask, e_NoticeDevType e_dev_type = e_Notice_AllType, unsigned int except_connid = 0);

	/*
	* @description : 对pGateMask填充内容
	* @input/output param : pGateMask	-- 填充ClientGateMask_t的指针
	* @input param : roomid				-- 房间ID
	* @input param : e_dev_type			-- e_Notice_PC:0,e_Notice_Android:1,e_Notice_IOS:2,e_Notice_Web:3,e_Notice_AllType:4
	* @input param : except_connid		-- 如果不为0,广播则剔除该客户端连接
	* @return: void
	*/
	static void Build_BroadCastUsersOnOneSvr_Gate(ClientGateMask_t * pGateMask, unsigned int roomid, e_NoticeDevType e_dev_type = e_Notice_AllType, unsigned int except_connid = 0);

	/*
	* @description : 对pGateMask填充内容
	* @input/output param : pGateMask	-- 填充ClientGateMask_t的指针
	* @input param : strVersion		-- 客户端版本号 for example: 1.0.2
	* @input param : e_dev_type		-- e_Notice_PC:0,e_Notice_Android:1,e_Notice_IOS:2,e_Notice_Web:3,e_Notice_AllType:4
	* @return: void
	*/
	static void Build_BroadCastAppVersion_Gate(ClientGateMask_t * pGateMask, std::string strVersion, e_NoticeDevType e_dev_type = e_Notice_AllType);

	/*
	* @description : 对szBuf填充内容:COM_MSG_HEADER + int + "svr_type=6&cmdlist=2019,2137,22011,2140&cmdrange_1=31001:32000"
	* @input param : szBuf				-- 填充内容的内存指针,需事先分配好
	* @input param : nBufLen			-- 事先分配好的内存大小
	* @input param : cmdlist			-- 命令字列表,例如:2019,2137,22011,2140
	* @input param : cmdrange_lst		-- 命令字范围段的列表,例如:31001:32000,41001:42000
	* @return: int  >0--msglen -1--fail
	*/
	static int Build_RegisterCmd_Msg(char * szBuf, unsigned int nBufLen, e_SvrType e_svr_type, const std::string & cmdlist, const std::list<std::string> & cmdrange_lst);

	/*
	* @description  : 去掉服务器报文的GateMask
	* @input  param : szBuf		-- 服务器报文缓存指针
	* @input  param : nBufLen	-- 服务器报文大小
	* @output param : buff		-- 客户端报文缓存
	*/
	static bool convSvrMsgToClientMsg(const char *szBuf, unsigned int nBufLen, SL_ByteBuffer &buff);

	/************************************************************************/
	/* protobuf                                                             */
	/************************************************************************/
	template<typename T>
	static bool ParseProtoMessage(const char *data, size_t len, T & msg)
	{
		if (NULL == data || len <= SIZE_IVM_HEAD_TOTAL)
		{
			return false;
		}
		
		const char * pProtobuf = data + SIZE_IVM_HEAD_TOTAL;
		size_t nProtoLen = len - SIZE_IVM_HEAD_TOTAL;

		return msg.ParseFromArray(pProtobuf, nProtoLen);
	}
};

#endif //__MSGCOMMAPI_H__
