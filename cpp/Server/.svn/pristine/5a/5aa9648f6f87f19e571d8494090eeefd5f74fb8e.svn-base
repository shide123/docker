
#ifndef __TASK_NETWORK_MSG_PROCESS__HH__
#define __TASK_NETWORK_MSG_PROCESS__HH__

#include "clienthandler.h"
#include "connecthandler.h"


//#define TASK_TYPE_CENTERSVR_NETMSG  1            //centersvr网络消息
//#define TASK_TYPE_INET_ROOM_TIMER    2            //定时器消息
//#define TASK_TYPE_INET_ROOM_INITDATA 3           //初始化房间数据
#define TASK_TYPE_LOADA4USER        4           //读入/删除机器人
#define TESK_TYPE_AUTH              6

#define TEACHER_SCORE_ERROR         1000             //其他错误
#define ROOM_NOT_FIND_ERROR         1001             //请求房间未找到
#define SCORE_ILLEGAL_ERROR         1002             //请求分数非法
#define TEACHER_NOT_FIND_ERROR      1003             //讲师未找到
#define USER_NOT_FIND_ERROR         1004             //用户未找到
#define SCORE_MORE_ERROR            1005             //两个小时内重复给一个讲师打分
#define TEACHER_SCORE_SUCCEED         0              //成功
#define ZHUANBO_ROOMS_MAX           50               //转播房间最大个数，即配置的一个子房间最多可转播的子房间个数
#define WEEK_FANS_MAX               5                //返回给客户端的周榜最大值

#define QUERY_FANS_ERROR            2000             //查询忠实排行榜其他错误

//
class CTaskNetMsgProc: public SL_Task<SL_ByteBuffer>
{
public:
	CTaskNetMsgProc(void);
	virtual ~CTaskNetMsgProc(void);

	int svc(SL_ByteBuffer &buf, void *svc_data = NULL, bool *change_svc_data = NULL);

private:
	int proc_syncdata_req(MSG_PROC_TASK_NODE *task_node);
	int proc_syncdata_resp(MSG_PROC_TASK_NODE *task_node);
	int proc_syncdata_from_log(MSG_PROC_TASK_NODE *task_node, uint64_t log_index);
	int proc_handle_redismsg(MSG_PROC_TASK_NODE *task_node);
};

#endif //__TASK_NETWORK_MSG_PROCESS__HH__

