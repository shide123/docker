#ifndef __PRESSSTAMP_MGR_H__
#define __PRESSSTAMP_MGR_H__


#include "ConsumeSvr.pb.h"
#include "ConsumeApp.h"

class CPressStampMgr
{
public:
	CPressStampMgr();
	~CPressStampMgr();
	static void init(Dbconnection * dbconn);
	//查询印章信息
	static int handle_QryStampInfoReq(task_proc_data * message);
	//发送按印章请求
	static int handle_SendPressStampReq(task_proc_data * message);
	 

private:
	//印章起拍价
	static unsigned int m_stampStartPrice;
	//印章每次加价不低于多少元
	static unsigned int m_stampIncreasePrice;
	 
};

#endif //__TREASURE_MGR_H__
