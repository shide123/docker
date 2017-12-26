#ifndef _NODEMGR_H_
#define _NODEMGR_H_

#include "message_vchat.h"

enum {
    Sub_NodeMgr_Ping = 1,                   //心跳协议，只有消息头
    Sub_NodeMgr_RegisterServerReq = 2,      //请求新服务器注册
    Sub_NodeMgr_RegisterServerResp = 3,     //新服务器注册返回
    Sub_NodeMgr_UnregisterServerReq = 4,    //服务器退出注册
    Sub_NodeMgr_SubscribeServerReq = 5,     //清求订阅关联进程的服务信息
    Sub_NodeMgr_SubscribeServerResp = 6,    //订阅关联进程服务器信息
    Sub_NodeMgr_SubscribeServerAdd = 7,      //通知添加新关联服务器
    Sub_NodeMgr_SubscribeServerRemove = 8,    //通知移除新关联服务器
    Sub_NodeMgr_SubscribeServerUpdate = 9,     //更新订阅关系(控制台使用)
    Sub_NodeMgr_Echo = 10,                  //定时发送，用于确认响应时间
    Sub_NodeMgr_MultiCardRegisterServerReq = 11,     //多网卡服务注册ip
    Sub_NodeMgr_NoticeIpStatus = 12,        //机器状态通知(内部协议使用)    
    Sub_NodeMgr_MultiCardRegisterServerV2Req     = 13,           //注册是携带私有数据字段.
    Sub_NodeMgr_SubscribeServerV2Resp   = 14,           //携带私有数据字段
    Sub_NodeMgr_SubscribeServerAddV2    = 15,
};

enum {
	en_node_client_version_default = 1,
		
	en_node_client_v1	//Sub_NodeMgr_RegisterServerReq
		= en_node_client_version_default,	
	en_node_client_v2,	//Sub_NodeMgr_MultiCardRegisterServerReq
	en_node_client_v3,	//Sub_NodeMgr_MultiCardRegisterServerV2Req
};

#define SERVER_NAME_LEN 32

#define VAR_DEF(field) 		\
    uint32  field##len; 	\
    char    field[0];     	\

#define GET_VAR_DEF_LEN(p, field)	\
	((p)->field##len)

#define GET_VAR_DEF_DATA(p, field)	\
	((p)->field)

#define GET_DESC_LEN(p)			GET_VAR_DEF_LEN(p, desc)
#define GET_DESC_DATA(p)		GET_VAR_DEF_DATA(p, desc)
#define SET_DESC_LEN(p, l)		(GET_DESC_LEN(p) = (l))
#define SET_DESC_DATA(p, d, l)					\
	do {										\
		if (l > 0 && d != NULL && p != NULL) {	\
			(memcpy(GET_DESC_DATA(p), d, l));	\
		}										\
	} while (0)

#pragma pack(1)

//注册服务器节点
typedef struct tag_CMDRegisterServer
{
    char servername[SERVER_NAME_LEN];  //服务器名字
    uint32 ip;          //ip
    uint16 port;        //端口
}CMDRegisterServer_t;

typedef struct tag_CMDRegisterServerV2
{
    char servername[SERVER_NAME_LEN];  //服务器名字
    uint32 ip;          //ip
    uint16 port;        //端口
    VAR_DEF(desc);
} CMDRegisterServerV2_t;

//注册节点返回
typedef struct tag_CMDRegisterServerResp
{
    uint8 res;          //返回码    0 成功 1 失败
    uint16 port;        //端口较验
}CMDRegisterServerResp_t;

//取消注册服务器节点(关掉连接也可达到此效果)
typedef struct tag_CMDUnregisterServer
{
    char servername[SERVER_NAME_LEN];  //服务器名字
}CMDUnregisterServer_t;

//订阅关联服务器
typedef struct tag_CMDSubscribeServer
{
    char servername[SERVER_NAME_LEN];  //服务器名字,用于做较验
    uint32 servernamelen;
    char servernames[0];     //多个服务器名称用逗号隔开
}CMDSubscribeServer_t;

//单个服务器的信息
typedef struct tag_SubscribeServerInfo
{
    char servername[SERVER_NAME_LEN];  //服务器名称
    uint32 ip;      //ip
    uint16 port;            //端口
}CMDSubscribeServerInfo_t;

typedef struct tag_SubscribeServerInfoV2
{
    char servername[SERVER_NAME_LEN];  //服务器名称
    uint32 ip;      //ip
    uint16 port;            //端口
    VAR_DEF(desc);
} CMDSubscribeServerInfoV2_t;

//返回订阅服务的进程列表
typedef struct tag_CMDSubscribeServerResp
{
    uint32 num;         //数量
    char content[0];   //信息 CMDSubscribeServerInfo_t ...
}CMDSubscribeServerResp_t;


//通知添加新服务器
typedef struct tag_CMDNoticeServerAdd
{
    char servername[SERVER_NAME_LEN];          //服务器类型
    uint32 ip;              //ip
    uint16 port;            //端口
}CMDNoticeServerAdd_t;

//通知添加新服务器
typedef struct tag_CMDNoticeServerAddV2
{
    char servername[SERVER_NAME_LEN];          //服务器类型
    uint32 ip;              //ip
    uint16 port;            //端口
    VAR_DEF(desc);
} CMDNoticeServerAddV2_t;

//通知移除服务器
typedef struct tag_CMDNoticeServerRemove
{
    char servername[SERVER_NAME_LEN];          //服务器类型
    uint32 ip;              //ip
    uint16 port;            //端口
}CMDNoticeServerRemove_t;

//更新订阅关系(控制台使用)
typedef struct tag_CMDUpdateSubscribe
{
    uint8 isadd;            //1 添加 0删除
    char servername[SERVER_NAME_LEN];          //服务器名称
    char subscribedservername[SERVER_NAME_LEN]; //被订阅服务器名称
}CMDUpdateSubscribe_t;

//响应消息
typedef struct tag_CMDEcho
{
    uint32 seq;             //序列号
    uint32 second;          //秒
    uint32 usecond;         //微秒
}CMDEcho_t;

//注册服务器节点
typedef struct tag_CMDMultiCardRegisterServer
{
    char servername[SERVER_NAME_LEN];  //服务器名字
    char ip[128];          //多个ip以","分隔
    uint16 port;        //端口
}CMDMultiCardRegisterServer_t;


//机器ip状态通知
typedef struct tag_CMDNoticeIpStatus
{
    char ip[32];            //ip
    uint8 status;           //0正常 1断连接
    uint32 msecond;         //ping值, 微秒
}CMDNoticeIpStatus_t;

//注册服务器节点
typedef struct tag_CMDMultiCardRegisterServerV2
{
    char servername[SERVER_NAME_LEN];  //服务器名字
    char ip[128];          //多个ip以","分隔
    uint16 port;        //端口
    VAR_DEF(desc);
} CMDMultiCardRegisterServerV2_t;

#pragma pack()
#endif  //_NODEMGR_H_
