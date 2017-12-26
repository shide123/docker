#ifndef __THRIFT_CHATSVR_HANDLER_H__
#define __THRIFT_CHATSVR_HANDLER_H__
#include "TChatSvr.h"
#include "message_vchat.h"
#include "SL_Socket_CommonAPI.h"
class TChatSvrHandler :public TChatSvrIf
{
public:
	TChatSvrHandler();
	virtual ~TChatSvrHandler();

	virtual bool procGroupChatReq(const int32_t groupid, const TChatMsg& chatMsg, const TChatMsgExtra& extra);
	virtual bool procUserJoinGroupNotify(const int32_t userid, const int32_t groupid, const int32_t roletype = e_MemberRole);
	virtual int32_t proc_addGroupAssistMsg(const TGroupAssistMsg & assistMsg, const bool bNotyAdmin, const bool bNotyUser);
	virtual bool handle_approveGroupAssistMsg(const int32_t handle_userid, const int32_t groupid, const int32_t msgid, const bool agree, const int32_t apply_userid);
	virtual bool handle_voice_callback(const std::string& inputkey, const int32_t code, const std::string& key);
	virtual bool handle_keyword_action(const int32_t action, const std::string& value, const std::string& replace) ;

	virtual bool proc_optPPTPic( const std::vector<TPPTPicInfo>& vecPicId,const int32_t optType) ;
	virtual bool proc_commentAudit( const TChatAuditMsg &tTChatMsg) ;
};

#endif //__THRIFT_CHATSVR_HANDLER_H__
