#ifndef __TREASURE_MGR_H__
#define __TREASURE_MGR_H__

#include <map>
#include "ConsumeSvr.pb.h"
#include "ConsumeApp.h"

class CTreasureMgr
{
public:
	CTreasureMgr(){}
	~CTreasureMgr(){}

	static void procSponsorTreasureReq(task_proc_data &message);
	static void procBuyTreasure(task_proc_data &message);
	static void procQryMyTreasureDetail(task_proc_data &message);
	static void procQryGroupTreasureList(task_proc_data &message);
	static void procQryMyTreasureList(task_proc_data &message);
	static void procQryTreasureInfo(task_proc_data &message);

private:
	static void notifyTreasureUpdate(const CMDTreasureInfo &info, e_TreasureStatus oldStatus, e_TreasureStatus newStatus);
	static int getBuyError(const std::string &errinfo);
	static void qryTreasureList(CMDQryTreasureList &req, CMDTreasureList &resp, bool qryUser = false);
	static int qryTreasureList(const std::string &sql, uint32 userid, uint32 groupid, ::google::protobuf::RepeatedPtrField< ::CMDTreasureInfo > &list, bool qryUser = false);
	static bool qryTreasureBuyList(std::map<uint32, CMDTreasureInfo *> &mTreasure, uint32 userid = 0);
	static void initBuyError();
	static std::map<std::string, int> m_mBuyError;
};

#endif //__TREASURE_MGR_H__
