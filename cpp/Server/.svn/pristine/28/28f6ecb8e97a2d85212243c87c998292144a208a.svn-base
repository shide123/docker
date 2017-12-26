#ifndef __GROUP_ASSIST_LOGIC_H__
#define __GROUP_ASSIST_LOGIC_H__

#include "comm_struct.h"
#include "ProtocolsBase.h"
#include "ChatSvr.pb.h"
#include "Config.h"

class CGroupAssistLogic
{
public:
	CGroupAssistLogic();
	virtual ~CGroupAssistLogic();

	static void initMsgFormat(Config & config);
	static void procUnreadMsgReq(task_proc_data & task_data);
	static void procMsgNotifyRecv(task_proc_data & task_data);
	static void procSyncTaskStatusReq(task_proc_data & task_data);
	static void procAssistMsgHisReq(task_proc_data & task_data);
	static bool addAssistMsg(StGPAssistMsg_t & oMsg, bool bNotyAdmin = true, bool bNotyUser = true);
	static bool approveAssistMsg(unsigned int userID, unsigned int groupID, unsigned int msgID, bool agree, unsigned int apply_userID);
	static void procInvestMsgReq(task_proc_data & task_data);

private:
	static void handleAssistMsgNotify(const StGPAssistMsg_t & oMsg, const std::string & strNotifyMsg, bool notyUser);

	static std::string m_kickoutMsgAdminFormat;
	static std::string m_kickoutMsgUserFormat;

	static std::string m_levelchgMemUpAdminFormat;
	static std::string m_levelchgMemUpUserFormat;
	static std::string m_levelchgGuestUpAdminFormat;
	static std::string m_levelchgGuestUpUserFormat;
	static std::string m_levelchgAdminUpAdminFormat;
	static std::string m_levelchgAdminUpUserFormat;
	static std::string m_levelchgOwnerUpAdminFormat;
	static std::string m_levelchgOwnerUpUserFormat;

	static std::string m_levelchgQuitGroupAdminFormat;
	static std::string m_levelchgMemDownAdminFormat;
	static std::string m_levelchgMemDownUserFormat;
	static std::string m_levelchgGuestDownAdminFormat;
	static std::string m_levelchgGuestDownUserFormat;
	static std::string m_levelchgAdminDownAdminFormat;
	static std::string m_levelchgAdminDownUserFormat;

	static std::string m_pchatOpenAdminFormat;
	static std::string m_pchatOpenUserFormat;
	static std::string m_pchatCloseAdminFormat;
	static std::string m_pchatCloseUserFormat;

	static std::string m_viewAuthAdminFormat;
	static std::string m_viewAuthUserPassFormat;
	static std::string m_viewAuthUserRefuseFormat;

	static std::string m_msgAuthAdminFormat;
	static std::string m_msgAuthUserPassFormat;
	static std::string m_msgAuthUserRefuseFormat;

	static std::string m_investmsg_photo;
	static std::string m_investmsg_viewauth;
	
	static std::string m_redpacket_balance;
	static std::string m_dir_redpacket_balance;

	std::map<unsigned int, std::string > m_UserIDAliasMap;
	std::map<unsigned int, std::string > m_UserIDHeadMap;
	std::map<unsigned int, std::string > m_GroupNameMap;

	static std::string genAdminNotifyString(const StGPAssistMsg_t & oMsg, const std::string & userHead, const std::string & userAlias, const std::string & groupName);
	static std::string genUserNotifyString(const StGPAssistMsg_t & oMsg, const std::string & groupName, const std::string & userAlias);
	static std::string genAssistMsgIcon(unsigned int msgType, const std::string & strUserHead);
	static std::string genAssistMsgTitle(unsigned int msgType, const std::string & strUserAlias);

	//member function not static,because cache is not static
	void getUserInfoFromCache(unsigned int userID, std::string & strUserAlias, std::string & strUserHead);
	void getGroupNameFromCache(unsigned int groupID, std::string & groupName);
};

#endif //__GROUP_ASSIST_LOGIC_H__
