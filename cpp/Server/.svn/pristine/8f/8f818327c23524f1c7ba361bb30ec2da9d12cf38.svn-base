#include "ChatLogic.h"
#include "ChatServerApp.h"
#include "msgcommapi.h"
#include "ChatRedisMgr.h"
#include "ChatMongoMgr.h"
#include "ChatCommon.h"
#include "ChatSvr.pb.h"
#include "RoomSvr.pb.h"
#include "CUserBasicInfo.h"
#include "CUserGroupinfo.h"
#include "GroupBasicInfo.h"
#include "errcode.h"
#include "DBSink.h"
#include "GroupAssistLogic.h"
#include "CThriftSvrMgr.h"
#include "tppushsvr/TpPushSvr.h"
#include "message_vchat.h"
#include "CommonLogic.h"
#include "HttpClient.h"
#include "common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "RoomManager.h"
#include "TRSpeex.h"
#include "utils.h"

redis_map_map CChatLogic::m_rmGroupChatMsg;
redis_map_map CChatLogic::m_rmUnSuccessGroupChatMsg;

redis_map_map CChatLogic::m_rmCommentAudit;
int			  CChatLogic::m_isCommentAudit=0;

std::map<uint32, stUserBasicInfo> CChatLogic::m_mapUser;
std::map<std::string ,CMDGroupMsgReq> CChatLogic::m_VoiceCompleteInfo;
std::map<std::string,uint32> CChatLogic::m_req_last;
boost::mutex CChatLogic::m_voice_mutex;
boost::mutex CChatLogic::m_unsucc_mutex;
CKeywordMgr CChatLogic::m_keyworkmgr;
std::map<uint32,CMDGroupMsgReq> CChatLogic::m_unSuccessReq;
int CChatLogic::m_timerid = 0;
redis_map_map CChatLogic::m_rmRoomuserSetting;


const std::string qqurl = "https://api.weixin.qq.com/cgi-bin/media/get/jssdk?";// "http://file.api.weixin.qq.com/cgi-bin/media/get?";
 
const std::string media_backup = "/usr/local/services_ql/chatsvr/media_backup/";

int trans2Wav(const char *from, const char* to)
{

	FILE		  *fpInput;
	FILE		  *fpOutput;

	char aInputBuffer[MAX_FRAME_SIZE * 10];
	char aOutputBuffer[MAX_FRAME_SIZE * 10];

	int ret;
	int buffer_size;

	int nOutSize;
	//int nPackNo;

	TRSpeexDecodeContex SpeexDecode;

	int nTotalLen;
	char buf[44];

 


	memset(aInputBuffer, 0, sizeof(char)*MAX_FRAME_SIZE * 10);



	memset(buf, 0, 44);


	buf[0] = 'R';
	buf[1] = 'I';
	buf[2] = 'F';
	buf[3] = 'F';

	buf[8] = 'W';
	buf[9] = 'A';
	buf[10] = 'V';
	buf[11] = 'E';
	buf[12] = 'f';
	buf[13] = 'm';
	buf[14] = 't';
	buf[15] = 0x20;

	buf[16] = 0x10;
	buf[20] = 0x01;
	buf[22] = 0x01;
	buf[24] = 0x80;
	buf[25] = 0x3E;
	buf[29] = 0x7D;
	buf[32] = 0x02;
	buf[34] = 0x10;
	buf[36] = 'd';
	buf[37] = 'a';
	buf[38] = 't';
	buf[39] = 'a';




	TRSpeexDecodeInit(&SpeexDecode);

	fpInput = fopen(from, "rb");


	if (fpInput == NULL)
	{
		printf("can't open input spx file");
		return 0;
	}

	fpOutput = fopen(to, "wb");

	if (fpOutput == NULL)
	{
		printf("can't open output file");
		return 0;
	}

	fwrite(buf, 1, 44, fpOutput);


	nTotalLen = 0;


	buffer_size = 6;

	ret = fread(aInputBuffer, 1, buffer_size, fpInput);

	while (1)
	{
		TRSpeexDecode(&SpeexDecode, aInputBuffer, buffer_size, aOutputBuffer, &nOutSize);

		ret = fread(aInputBuffer, 1, buffer_size, fpInput);
		if (ret != buffer_size)
			break;

		fwrite(aOutputBuffer, 1, nOutSize, fpOutput);
		nTotalLen += nOutSize;

	}

	TRSpeexDecodeRelease(&SpeexDecode);

	fseek(fpOutput, 40, SEEK_SET);
	fwrite(&nTotalLen, 1, 4, fpOutput);

	fseek(fpOutput, 4, SEEK_SET);
	nTotalLen += 36;
	fwrite(&nTotalLen, 1, 4, fpOutput);
	fclose(fpOutput);
	fclose(fpInput);





	return 0;
}

//msgtype=16时,content 格式 media_id=55CjMxadfTncWlgtUW0xbJynqHfwUVQYhvha_yQbKn0C1WmypFRbKOvn6k0k7AC7&&picmediaid=FrYYwOGUylVXbDu-8qfG8PmIZbS2
//取出语音media_id，和图片picmediaid,media_id 须非空,picmediaid可空
bool CChatLogic::getMediaId(const std::string &sInputContent,std::string &sVoiceMediaId,std::string &sPicMediaId)
{
	//std::string::size_type pos = sInputContent.find("media_id");
	if (sInputContent.find("media_id") == std::string::npos ||sInputContent.find("picmediaid") == std::string::npos
		||sInputContent.find("&&") == std::string::npos)
	{
		return false;
	}

	std::vector<std::string> str_vec;
	splitStrToVec(sInputContent,"&&", str_vec);
	if (str_vec.size()!=2)
	{
		return false;
	}

	std::vector<std::string> media_vec;
	splitStrToVec(str_vec[0],"=", media_vec);

	if (media_vec.size()!=2)
	{
		return false;
	}

	sVoiceMediaId=media_vec[1];

	std::vector<std::string> picmedia_vec;
	splitStrToVec(str_vec[1],"=", picmedia_vec);
	if (picmedia_vec.size()==2)
	{
		sPicMediaId=picmedia_vec[1];
	}

	return true;
}


 //1， 图片从微信下载，上传到七牛后，广播群消息，更新mongodb聊天记录
//2，语音从微信下载后,从speex转码为wav格式,再上传到七牛,然后七牛转码成功后会通知php,php调用handle_voice_callback，这个时候才会更新mongodb聊天记录content和广播群消息
void CChatLogic::processoffline(CMDGroupMsgReq &req)
{
	int msgtype = req.msg().msgtype();
	//get media
	std::string content = "";
	unsigned int msgid = req.msg().msgid();
	unsigned int userid = req.msg().srcuser().userid();

	std::string filename = "";
	std::string speex = "";
	std::string speexpath = "";
	std::string fileext = "";
	std::string path = media_backup;

	Json::Reader reader(Json::Features::strictMode());
	Json::Value root;

	std::string sVoiceMediaId;
	std::string sPicMediaId;

	switch(msgtype)
	{
	case 1:
		filename = getTime("%Y%m%d%H%M%S") + stringFormat("%d%d.jpg",msgid,userid);
		content = "http://file.api.weixin.qq.com/cgi-bin/media/get?" + req.msg().content();
		break;
	case 2:
	
		fileext = getTime("%Y%m%d%H%M%S") + stringFormat("%d%d",msgid,userid);
		filename = fileext+ ".speex";
		speex = fileext + ".wav";
		content = "https://api.weixin.qq.com/cgi-bin/media/get/jssdk?" + req.msg().content();
		break;
	case 16:
		fileext = getTime("%Y%m%d%H%M%S") + stringFormat("%d%d",msgid,userid);
		filename = fileext+ ".speex";
		speex = fileext + ".wav";

		/*if (reader.parse(req.msg().content(), root))
		{
			if (root.isMember("media_id"))
			{
				Json::Value value = root["media_id"];
				sVoiceMediaId=value.asString();
			}
		}*/

		
		if (!getMediaId(req.msg().content(),sVoiceMediaId,sPicMediaId))
		{
			LOG_PRINT(log_error, "MSGTYPE_PPT_VOICE  parse sVoiceMediaId error, groupid: %u,req.msg().content()=%s", req.groupid(),req.msg().content().c_str());
			return;
		}

		if (!sVoiceMediaId.empty())
		{
			std::string code ="";
			unsigned int codeTime = 0;
			CGlobalInfo::getGzhtoken(code,codeTime);

			content = "https://api.weixin.qq.com/cgi-bin/media/get/jssdk?media_id=" + sVoiceMediaId+"&access_token="+code;
		}
		else
		{
			LOG_PRINT(log_error, "MSGTYPE_PPT_VOICE json parse sVoiceMediaId empty error, groupid: %u", req.groupid());
			return;
		}
		
	 
		break;
	default:
		break;
	}
	if(msgtype == 2||msgtype==16)
	{
		speexpath = path + speex;
		CChatLogic::m_req_last[filename] = SL_Socket_CommonAPI::util_process_clock_ms();
		LOG_PRINT(log_info, "------ processoffline.");

	}
	
	path += filename;
	
	CHttpClient httpClient;
	LOG_PRINT(log_info, "downloadmedia url[%s]  outfile[%s].", content.c_str(),path.c_str());
	bool bdel = false;
	if(httpClient.downloadmedia(content.c_str(),path.c_str()))
	{
		unsigned int ntime_begin = 0, ntime_end = 0;
		ntime_begin = CChatLogic::m_req_last[filename];
		ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
		LOG_PRINT(log_info, "------ processoffline====downloadmedia ms = %d.", ntime_end - ntime_begin);
		LOG_PRINT(log_info, "downloadmedia ok key[%s],extfile[%s]", filename.c_str(),path.c_str());
		LOG_PRINT(log_info, "QINIU_UPLOAD_URL:%s", CDBSink().qrySysParameterString("QINIU_UPLOAD_URL").c_str());
		if (msgtype == 2||msgtype==16)
		{
			
			trans2Wav(path.c_str(), speexpath.c_str());//speex文件转码为wav,path为下载下来的speex文件,speexpath 即wav文件

			/*std::string cmd = "rm ";
			cmd += path;
			system(cmd.c_str());
			*/
			filename = speex;
			path = speexpath;
		}
		
		//put
		if (uploadfile(CDBSink().qrySysParameterString("QINIU_UPLOAD_URL").c_str(), filename.c_str(), path.c_str(), (msgtype == 2||msgtype == 16)) == 0)
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			ntime_begin = CChatLogic::m_req_last[filename];
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "------ processoffline====uploadfile ms = %d.", ntime_end - ntime_begin);
			LOG_PRINT(log_info, "uploadfile key[%s],extfile[%s]", filename.c_str(),path.c_str());
			if(msgtype == 1)//pic
			{
				std::string outfile = "http://os700oap7.bkt.clouddn.com/";
				outfile += filename;
				CChatMongoMgr::updateMsgContent(req.msg().msgid(),req.groupid(),outfile);
				req.mutable_msg()->set_content(outfile);
				notifyGroupMsg(req, false, isMsgPushOnlineOnly(req.msg()));
			}
			else if(msgtype == 2||msgtype == 16)//voice
			{
//				api convert
//				std::string saveaskey = "";
//				saveaskey = fileext + ".aac";
//				fopConvert(filename.c_str(),saveaskey.c_str());
				CChatLogic::addVoiceInfo(filename,req);
				unsigned int ntime_begin = 0, ntime_end = 0;
				ntime_begin = CChatLogic::m_req_last[filename];
				ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
				LOG_PRINT(log_info, "------ processoffline====fopconvert ms = %d.", ntime_end - ntime_begin);
			}
			bdel =true;
		}
	}else
	{
		LOG_PRINT(log_info, "downloadmedia fail key[%s],extfile[%s]", filename.c_str(),path.c_str());
	}
	if(bdel)
	{
		std::string cmd = "rm ";
		cmd += path;
		system(cmd.c_str());
	}
	else
	{
		CChatLogic::addUnsuccReq(filename,req);
	}
}
void onTimer()
{
	/*time_t now = time(0);
	for(std::map<uint32,CMDGroupMsgReq>::iterator it = CChatLogic::m_unSuccessReq.begin() ; it != CChatLogic::m_unSuccessReq.end() ;)
	{
		if(now - it->first >24*3600)
		{
			CChatLogic::m_unsucc_mutex.lock();
			CChatLogic::m_unSuccessReq.erase(it++);
			CChatLogic::m_unsucc_mutex.unlock();
		}else
		{
			it++;
		}
	}*/
	vector<string> vGroupChatMsg;
	CChatLogic::m_rmUnSuccessGroupChatMsg.redis_keys("", vGroupChatMsg, 9);

	LOG_PRINT(log_info, "onTimer keys:%d", vGroupChatMsg.size());
	std::string token = "";
	//if(CChatLogic::m_unSuccessReq.size()>0)
	if (vGroupChatMsg.size()>0)
	{
		std::string code ="";
		unsigned int codeTime = 0;
		CGlobalInfo::getGzhtoken(code,codeTime);
		token = "access_token=" + code;
	}
	vector<string>::iterator it = vGroupChatMsg.begin();
	for (; it != vGroupChatMsg.end(); it++)
	{
		std::string inputkey = *it;
		CMDGroupMsgReq req;
		if (!CChatLogic::getUnsuccReq(inputkey, req))
		{
			LOG_PRINT(log_info, "onTimer not find  keys:%d", vGroupChatMsg.size());
			continue;
		}
		std::string content = req.msg().content();
		int pos = content.find("access_token=");
		content.replace(pos, content.length(), token);
		LOG_PRINT(log_info, "onTimer() content:%s", content.c_str());
		req.mutable_msg()->set_content(content);	
		CChatLogic::m_rmUnSuccessGroupChatMsg.erase(inputkey);
		CChatLogic::processoffline(req);
	}
	/*for(std::map<uint32,CMDGroupMsgReq>::iterator it = CChatLogic::m_unSuccessReq.begin() ; it != CChatLogic::m_unSuccessReq.end() ;)
	{
		CMDGroupMsgReq req = it->second;
		std::string content = req.msg().content();
		int pos = content.find("access_token=");
		content.replace(pos,content.length(),token);
		LOG_PRINT(log_info, "onTimer() content:%s", content.c_str());
		req.mutable_msg()->set_content(content);
		CChatLogic::m_unsucc_mutex.lock();
		CChatLogic::m_unSuccessReq.erase(it++);
		CChatLogic::m_unsucc_mutex.unlock();
		processoffline(req);
	}*/
}

CChatLogic::CChatLogic()
{
}

CChatLogic::~CChatLogic(void)
{
}
void CChatLogic::init(redisMgr* predisMgr)
{
	m_timerid = Timer::getInstance()->add(5,boost::bind(&onTimer));
	m_rmRoomuserSetting.init(DB_ROOM, KEY_HASH_ROOM_USER_SETTING":", predisMgr);
	m_rmGroupChatMsg.init(8, KEY_HASH_ROOM_GROUP_CHAT_MSG":", predisMgr);
	m_rmUnSuccessGroupChatMsg.init(9, KEY_HASH_ROOM_GROUP_CHAT_MSG":", predisMgr);

	m_rmCommentAudit.init(1,KEY_HASH_COMMENT_AUDIT_SWITCH":",predisMgr);
	if (access(media_backup.c_str(), 0) == -1)
	{
		mkdir(media_backup.c_str(),0755);
	}
}
void CChatLogic::uninit()
{
	if(m_timerid)
	{
		Timer::getInstance()->del(m_timerid);
	}
}
void CChatLogic::addUnsuccReq(std::string inputkey, const CMDGroupMsgReq& req)
{
	const ChatMsg_t &msg = req.msg();
	std::map<string, string>  fieldvaluemap;

	fieldvaluemap["groupid"] = bitTostring(req.groupid());
	fieldvaluemap["msgid"] = bitTostring(msg.msgid());
	fieldvaluemap["srcuid"] = bitTostring(msg.srcuser().userid());
	fieldvaluemap["dstuid"] = bitTostring(msg.dstuser().userid());
	fieldvaluemap["msgtime"] = bitTostring(msg.msgtime());
	fieldvaluemap["msgtype"] = bitTostring((int)msg.msgtype());
	fieldvaluemap["content"] = msg.content();
	fieldvaluemap["media_length"] = bitTostring(msg.medialength());

	m_rmUnSuccessGroupChatMsg.insert(inputkey, fieldvaluemap);
	m_rmUnSuccessGroupChatMsg.expireat(inputkey, time(NULL) + 24 * 3600);

	/*m_unsucc_mutex.lock();
	m_unSuccessReq[ts] = req;
	m_unsucc_mutex.unlock();*/
}
void CChatLogic::delUnsuccReq(const CMDGroupMsgReq& req)
{
	/*m_unsucc_mutex.lock();
	m_unSuccessReq.erase(ts);
	m_unsucc_mutex.unlock();*/
	const ChatMsg_t &msg = req.msg();
	std::string strMsgid = CChatCommon::getFormatString("%u", msg.msgid());
	m_rmUnSuccessGroupChatMsg.erase(strMsgid);
	 

}
bool CChatLogic::getUnsuccReq(std::string inputkey, CMDGroupMsgReq &req)
{
	std::map<string, string>  fieldvaluemap;
	CChatLogic::m_rmUnSuccessGroupChatMsg.getall(inputkey, fieldvaluemap);
	if (fieldvaluemap.empty())
	{
		LOG_PRINT(log_warning, "getVoiceInfo:%s info from redis is empty.", inputkey.c_str());
		return false;
	}
	req.set_groupid(atoi(fieldvaluemap["groupid"].c_str()));
	req.mutable_msg()->set_msgid(atoi(fieldvaluemap["msgid"].c_str()));
	req.mutable_msg()->mutable_srcuser()->set_userid(atoi(fieldvaluemap["srcuid"].c_str()));
	req.mutable_msg()->mutable_dstuser()->set_userid(atoi(fieldvaluemap["dstuid"].c_str()));
	req.mutable_msg()->set_msgtime(atoi(fieldvaluemap["msgtime"].c_str()));
	req.mutable_msg()->set_msgtype((e_MsgType)atoi(fieldvaluemap["msgtype"].c_str()));
	req.mutable_msg()->set_content(fieldvaluemap["content"]);
	req.mutable_msg()->set_medialength(atoi(fieldvaluemap["media_length"].c_str()));
	return true;


 
}
 
bool CChatLogic::getVoiceInfo(std::string inputkey, CMDGroupMsgReq &req)
{	 
	std::map<string, string>  fieldvaluemap;
	CChatLogic::m_rmGroupChatMsg.getall(inputkey, fieldvaluemap);
	if (fieldvaluemap.empty())
	{
		LOG_PRINT(log_error, "getVoiceInfo:%s info from redis is empty.", inputkey.c_str());
		return false;
	}
	req.set_groupid(atoi(fieldvaluemap["groupid"].c_str()));
	req.mutable_msg()->set_msgid(atoi(fieldvaluemap["msgid"].c_str()));
	req.mutable_msg()->mutable_srcuser()->set_userid(atoi(fieldvaluemap["srcuid"].c_str()));
	req.mutable_msg()->mutable_dstuser()->set_userid(atoi(fieldvaluemap["dstuid"].c_str()));
	req.mutable_msg()->set_msgtime(atoi(fieldvaluemap["msgtime"].c_str()));
	req.mutable_msg()->set_msgtype((e_MsgType)atoi(fieldvaluemap["msgtype"].c_str()));
	req.mutable_msg()->set_content(fieldvaluemap["content"]);
	req.mutable_msg()->set_medialength(atoi(fieldvaluemap["media_length"].c_str()));

	req.mutable_msg()->set_extendtype(atoi(fieldvaluemap["extendtype"].c_str()));

	//LOG_PRINT(log_info, "getVoiceInfo:%s extendtype=%s.", inputkey.c_str(),fieldvaluemap["extendtype"].c_str());

	return true;

 

	/*CChatLogic::m_voice_mutex.lock();
	CChatLogic::m_VoiceCompleteInfo[inputkey] = req;
	CChatLogic::m_voice_mutex.unlock();*/
}
void CChatLogic::addVoiceInfo(std::string inputkey,CMDGroupMsgReq &req)
{
	const ChatMsg_t &msg = req.msg();
	std::map<string, string>  fieldvaluemap;
	 
	fieldvaluemap["groupid"] = bitTostring(req.groupid());
	fieldvaluemap["msgid"] = bitTostring( msg.msgid() );
	fieldvaluemap["srcuid"] = bitTostring(msg.srcuser().userid());
	fieldvaluemap["dstuid"] = bitTostring(msg.dstuser().userid());
	fieldvaluemap["msgtime"] = bitTostring(msg.msgtime());
	fieldvaluemap["msgtype"] = bitTostring((int)msg.msgtype());
	fieldvaluemap["content"] = msg.content();
	fieldvaluemap["media_length"] = bitTostring(msg.medialength());
	fieldvaluemap["extendtype"] = bitTostring((int)msg.extendtype());
	 
	m_rmGroupChatMsg.insert(inputkey, fieldvaluemap);
	m_rmGroupChatMsg.expireat(inputkey, time(NULL) + 24 * 3600);
	 
	/*CChatLogic::m_voice_mutex.lock();
	CChatLogic::m_VoiceCompleteInfo[inputkey] = req;
	CChatLogic::m_voice_mutex.unlock();*/
}
void CChatLogic::delVoiceInfo(std::string inputkey)
{
	/*CChatLogic::m_voice_mutex.lock();
	CChatLogic::m_VoiceCompleteInfo.erase(inputkey);
	CChatLogic::m_voice_mutex.unlock();*/
	 
	m_rmGroupChatMsg.erase(inputkey);
}
void CChatLogic::procPrivateMsgReq(task_proc_data &task_data)
{
	CMDPrivateMsgReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s]  failed.", req.GetTypeName().c_str());
		return;
	}

	if (!dealPrivateMsgReq(req))
	{
		task_data.resperrinf(ERR_CODE_INVALID_PARAMETER);
		LOG_PRINT(log_warning, "dealPrivateMsgReq failed..");
		return;
	}
	
	//send receive to src user
	CMDPrivateMsgRecv recv;
	recv.set_msgid(req.msg().msgid());
	recv.set_dstuid(req.msg().dstuser().userid());
	recv.set_clientmsgid(req.msg().clientmsgid());
	task_data.respProtobuf(recv, Sub_Vchat_PrivateMsgRecv);

	//if (chkUserOnline(dstuid))
	{
		notifyPrivateMsg(req);
	}
}

bool CChatLogic::dealPrivateMsgReq(CMDPrivateMsgReq &req)
{
	if (!CChatCommon::chkPMsgValid(req.msg()))
	{
		LOG_PRINT(log_warning, "chkPMsgValid return false, from [%u] to [%u].", req.msg().srcuser().userid(), req.msg().dstuser().userid());
		return false;
	}
	uint32 srcuid = req.msg().srcuser().userid();
	uint32 dstuid = req.msg().dstuser().userid();

	LOG_PRINT(log_debug, "Recv Private Chat Msg from [%u] to [%u].", srcuid, dstuid);
	uint32 msgid;
	if (!CChatMongoMgr::getNextSequence(CChatCommon::getPChatSeqName(srcuid, dstuid), msgid))
	{
		LOG_PRINT(log_error, "getNextSequence error, srcid: %u, dstid: %u.", srcuid, dstuid);
		return false;
	}
	req.mutable_msg()->set_msgtime(time(NULL));
	req.mutable_msg()->set_msgid(msgid);

	if (FIRST_SEQUENCE_VALUE == msgid)
	{
		CChatMongoMgr::updPrivateMsgId(srcuid, dstuid, msgid);
		CChatMongoMgr::updPrivateMsgId(dstuid, srcuid, 0);
	}

	//write msg to mysql/mongodb
	return CChatMongoMgr::writePrivateMsg(req);
}

void CChatLogic::notifyPrivateMsg(const CMDPrivateMsgReq &req)
{
	//notify user 
	CMDPrivateMsgNotify notify;
	notify.mutable_msg()->CopyFrom(req.msg());
	getUserInfo(*notify.mutable_msg()->mutable_srcuser());
	 

	int nProtoLen = notify.ByteSize();
	SL_ByteBuffer buff(SIZE_IVM_HEAD_TOTAL + nProtoLen);
	CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), 0, Sub_Vchat_PrivateMsgNotify, buff.buffer_size());
	DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
	CMsgComm::Build_BroadCastUser_Gate(pGateMask, notify.msg().dstuser().userid());
	notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);

	if (AppInstance()->m_GateConnMgr.sendMsgToGate(buff.buffer(), buff.buffer_size()))
		LOG_PRINT(log_debug, "Notify private chat msg from [%u] to [%u], msgid[%u].", req.msg().srcuser().userid(), req.msg().dstuser().userid(), req.msg().msgid());

	CMsgComm::Build_BroadCastUser_Gate(pGateMask, notify.msg().srcuser().userid());
	AppInstance()->m_GateConnMgr.sendMsgToGate(buff.buffer(), buff.buffer_size());
}

void CChatLogic::procPrivateMsgNotifyRecv(task_proc_data &task_data)
{
	CMDPrivateMsgNotifyRecv rcv;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, rcv))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", rcv.GetTypeName().c_str());
		return;
	}

	if (rcv.rcvuid() == rcv.srcuid())
	{
		return;
	}
	LOG_PRINT(log_debug, "dstuid[%u] has received msg[%u] from srcuid[%u]", rcv.rcvuid(), rcv.msgid(), rcv.srcuid());
	if (!rcv.rcvuid() || !rcv.srcuid() || !rcv.msgid())
	{
		LOG_PRINT(log_warning, "check param failed.");
		return;
	}
	//update the msgid of mongodb
	if (!CChatMongoMgr::updPrivateMsgId(rcv.rcvuid(), rcv.srcuid(), rcv.msgid()))
	{
		LOG_PRINT(log_warning, "updPrivateMsgId error.");
	}
}

void CChatLogic::procPrivateMsgHisReq(task_proc_data &task_data)
{
	CMDPrivateMsgHisReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}

	LOG_PRINT(log_info, "receive private msg history request: user[%u] dstuid[%u] forward[%u] msgid[%u] count[%u]",
		req.requid(), req.dstuid(), req.forward(), req.msgid(), req.count());

	CMDPrivateMsgHisResp resp;
	resp.set_requid(req.requid());
	resp.set_dstuid(req.dstuid());

	uint32 count = 0;
	std::list<ChatMsg_t> lstMsg;
	if (CChatMongoMgr::qryPrivateMsgList(req.requid(), req.dstuid(), lstMsg, req.msgid(), req.count(), req.forward()) > 0)
	{
		for (std::list<ChatMsg_t>::iterator it = lstMsg.begin(); it != lstMsg.end();)
		{
			ChatMsg_t *pMsg = resp.add_msgs();
			pMsg->CopyFrom(*it++);	//be careful of ++
			getUserInfoWithMap(*pMsg->mutable_srcuser());
			count++;
			if (it != lstMsg.end() && resp.ByteSize() >= MAX_CHAT_PACKET_SIZE)
			{
				task_data.respProtobuf(resp, Sub_Vchat_PrivateMsgHisResp);
				resp.clear_msgs();
				count = 0;
			}
		}
		m_mapUser.clear();
	}

	task_data.respProtobuf(resp, Sub_Vchat_PrivateMsgHisResp);
}

void CChatLogic::procUnreadPrivateMsgReq(task_proc_data &task_data)
{
	CMDUnreadPrivateMsgReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}

	LOG_PRINT(log_info, "procUnreadGroupMsgReq begin: user[%u]", req.userid());
	if (!req.userid())
	{
		task_data.resperrinf(ERR_CODE_INVALID_PARAMETER);
		LOG_PRINT(log_warning, "check param failed, user[%u]", req.userid());
		return;
	}

	std::map<uint32, uint32> mMsgId;
	if (CChatMongoMgr::qryUserPrivateMsgIdList(req.userid(), mMsgId))
	{
		for (std::map<uint32, uint32>::iterator it = mMsgId.begin(); it != mMsgId.end(); it++)
		{
			std::list<ChatMsg_t> lstPMsg;
			size_t count = 0, total = 0;
			total = CChatMongoMgr::qryUnreadPrivateMsg(req.userid(), it->first, lstPMsg, it->second, 0);
			if (total > 0)
			{
				CMDUnreadPrivateMsgNotify notify;
				notify.set_dstuid(it->first);
				notify.set_unreadcount(total);
				for (std::list<ChatMsg_t>::iterator itNotify = lstPMsg.begin(); itNotify != lstPMsg.end();)
				{
					ChatMsg_t *pPMsg = notify.add_msgs();
					pPMsg->CopyFrom(*itNotify++);
					getUserInfoWithMap(*pPMsg->mutable_srcuser());
					if (itNotify != lstPMsg.end() && notify.ByteSize() >= MAX_CHAT_PACKET_SIZE)
					{
						task_data.respProtobuf(notify, Sub_Vchat_UnreadPrivateMsgNotify);
						LOG_PRINT(log_info, "send unread private msg: user[%u] dstuser[%u] from msgid[%u] count[%u]", 
							req.userid(), notify.dstuid(), it->second, count);
						count = 0;
						notify.clear_msgs();
					}
				}
				task_data.respProtobuf(notify, Sub_Vchat_UnreadPrivateMsgNotify);
				LOG_PRINT(log_info, "send unread private msg: user[%u] dstuser[%u] from msgid[%u] count[%u]", 
					req.userid(), notify.dstuid(), it->second, count);

			}
		}
	}

}

void CChatLogic::procGroupMsgReq(task_proc_data &task_data)
{
	CMDGroupMsgReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}
	LOG_PRINT(log_info, "recv group msg of group:%u, srcuid: %u. msgtype:%u,extendtype:%d", req.groupid(), req.msg().srcuser().userid(),req.msg().msgtype(),req.msg().extendtype());

	int msgtype = req.msg().msgtype();
	if(msgtype == 0 )
	{
		std::string content = req.msg().content();
		CKeywordMgr::replaceKey(content);
		req.mutable_msg()->set_content(content);
	}
	COM_MSG_HEADER* in_msg = (COM_MSG_HEADER*)task_data.pdata;
	 
	int ret = dealGroupMsgReq(req, false, in_msg->subcmd);	
	//send receive to src user
	CMDGroupMsgRecv recv;
	recv.mutable_errinfo()->set_errid(ret);
	recv.set_groupid(req.groupid());
	recv.set_msgid(req.msg().msgid());
	recv.set_clientmsgid(req.msg().clientmsgid());
	task_data.respProtobuf(recv, Sub_Vchat_GroupMsgRecv);

	if (ERR_CODE_SUCCESS != ret)
	{
		LOG_PRINT(log_warning, "dealGroupMsgReq %s, group[%u] srcuid[%u].", CErrMsg::strerror(ret).c_str(), req.groupid(), req.msg().srcuser().userid());
		return ;
	}
	if(msgtype == 1 || msgtype == 2)
	{
		std::string content = req.msg().content();

		std::string code ="";
		unsigned int codeTime = 0;
		CGlobalInfo::getGzhtoken(code,codeTime);
		//get
		content += "&access_token=";
		content += code;
		req.mutable_msg()->set_content(content);
	}

	LOG_PRINT(log_info, "dealGroupMsgReq %s,msgtype:%d,msgid:%d, conten:%s,get_io_service post, group[%u] srcuid[%u].",CErrMsg::strerror(ret).c_str(), msgtype, req.msg().msgid(),req.msg().content().c_str(),  req.groupid(), req.msg().srcuser().userid());
	//LOG_PRINT(log_warning, "dealGroupMsgReq %s, group[%u] srcuid[%u].msgtype:%d", CErrMsg::strerror(ret).c_str(), req.groupid(), req.msg().srcuser().userid(),msgtype);

	if(msgtype == 1|| msgtype == 2||msgtype == 16) //1 pic; 2,16 voice 
	{
		Application::get_io_service().post(boost::bind(processoffline,req));

	}
	else
	{
		notifyGroupMsg(req, false, isMsgPushOnlineOnly(req.msg()));//voice  需要 异步通知
	}

	 
	int date = atoi(stamp_to_standard(time(0), "%Y%m%d").c_str());

	
	if (msgtype!=MSGTYPE_RECALL && msgtype!=MSGTYPE_CLOSE_LIVE_COURSE)
	{
		if (req.groupid() > BROADCAST_ROOM_BEGIN_NUM)//直播间发言次数
		{
			CDBSink().updateStatistics(req.msg().srcuser().userid(), req.groupid(), 2, date, 1);
		}
		else//课程发言次数
		{
			CDBSink().updateStatistics(req.msg().srcuser().userid(), req.groupid(), 3, date, 1);
		}
	}
	
	LOG_PRINT(log_info, "procGroupMsgReq Statistics vcbid[%d], date[%d],userid[%d],groupid[%u] ", req.groupid(), date, req.msg().srcuser().userid(),req.groupid());
	 
	//int ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
	notifyLiveUpdate(req.groupid(),req.msg().srcuser().userid());
	//int ntime_end= SL_Socket_CommonAPI::util_process_clock_ms();
	//LOG_PRINT(log_info, "notifyLiveUpdate ,groupid[%u] ,take [%d] ms ", req.groupid(),ntime_end-ntime_begin);
}


void CChatLogic::notifyLiveUpdate(int iGroupID,int iTeacherUid)
{
	if (iGroupID<BROADCAST_ROOM_BEGIN_NUM)
	{
		return;
	}
	static string sLastDateMorning,sLastDateHourAfternoon;

	string sNowDate=getTime("%Y%m%d");
	string sHour= getTime("%H");
	if (sHour=="08"||sHour=="09"||sHour=="10")
	{
		if (!sLastDateMorning.empty())
		{
			if (sLastDateMorning!=sNowDate)
			{
				CMDGroupMsgReq req;
				req.mutable_msg()->mutable_srcuser()->set_userid(iTeacherUid);
				notifyPHPMsgCenter(req,"LIVE_ROOM_UPDATE");
				sLastDateMorning=sNowDate;
			}
		}
		else
		{
			sLastDateMorning=sNowDate;
		}
		
	}
	else if (sHour=="13"||sHour=="14")
	{
		if (!sLastDateHourAfternoon.empty())
		{
			if (sLastDateHourAfternoon!=sNowDate)
			{
				CMDGroupMsgReq req;
				req.mutable_msg()->mutable_srcuser()->set_userid(iTeacherUid);
				notifyPHPMsgCenter(req,"LIVE_ROOM_UPDATE");
				sLastDateHourAfternoon=sNowDate;
			}
		}
		else
		{
			sLastDateHourAfternoon=sNowDate;
		}
	}

	return;
}
long CChatLogic::getRecommendid(const string &content)
{
	Json::Reader reader(Json::Features::strictMode());
	Json::Value root;

	if (reader.parse(content, root))
	{
		if (root.isMember("recommendid"))
		{
			Json::Value value = root["recommendid"];
			return atoll(value.asString().c_str());
		}
	}
	return 0;
}

int CChatLogic::dealGroupMsgReq(CMDGroupMsgReq &req, bool blobby, uint16 subcmd/* = 0*/)
{
	int ret = ERR_CODE_SUCCESS;
	do
	{
		if (!req.groupid() || !CChatCommon::chkGMsgValid(req.msg()))
		{
			LOG_PRINT(log_warning, "chkGMsgValid failed, group[%u] srcuid[%u] msgtype[%d] msgsize[%u].", req.groupid(), req.msg().srcuser().userid(), req.msg().msgtype(), req.msg().content().size());
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}
		uint32 forbitchat = 0;
		LOG_PRINT(log_debug, "before getRoomUserStatus(): forbitchat = %u, subcmd = %u, group = %u, srcuid = %u, msgtype = %d",
			forbitchat, subcmd, req.groupid(), req.msg().srcuser().userid(), req.msg().msgtype());
		getRoomUserStatus(req.groupid(), req.msg().srcuser().userid(),forbitchat);
		LOG_PRINT(log_debug, "after getRoomUserStatus(): forbitchat = %u", forbitchat);
		if (forbitchat == 1 && subcmd == Sub_Vchat_GroupPrivateMsgReq &&  req.msg().msgtype() == MSGTYPE_TEXT)
		{
			ret = ERR_CODE_FORBIT_CHAT;
			break;
		}


		if (req.msg().msgtype() == MSGTYPE_RECALL)
		{
			uint32 oldMsgid = atoi(req.msg().content().c_str());
			ChatMsg_t oldMsg;
			LOG_PRINT(log_warning, "dealGroupMsgReq blobby:%d",blobby);
			 
			if (!CChatMongoMgr::qryGroupMsg(req.groupid(), oldMsgid, oldMsg, blobby))//|| oldMsg.srcuser().userid() != req.msg().srcuser().userid()
			{
				ret = ERR_CODE_RECALL_NOT_EXISTS;
				break;
			}
			long recommendid = getRecommendid(oldMsg.content());
			if (recommendid > 0)
			{
				
				int nret = CDBSink().delTalkRecommendLog(recommendid);
				LOG_PRINT(log_debug, "delTalkRecommendLog  :%d", nret);
			}
			if (!CChatMongoMgr::recallGroupMsg(req.groupid(), oldMsgid,blobby))
			{
				ret = ERR_CODE_FAILED_DB;
				break;
			}
			LOG_PRINT(log_info, "group:%u, user:%u has recall msg:%u.", req.groupid(), req.msg().srcuser().userid(), oldMsgid);
		}
		else if (req.msg().msgtype() == MSGTYPE_CANCELL_KEYPOINT)
		{
			uint32 Msgid = atoi(req.msg().content().c_str());
			ChatMsg_t Msg;
			LOG_PRINT(log_info, "MSGTYPE_CANCELL_KEYPOINT,dealGroupMsgReq blobby:%d",blobby);

			if (!CChatMongoMgr::qryGroupMsg(req.groupid(), Msgid, Msg, blobby))//|| oldMsg.srcuser().userid() != req.msg().srcuser().userid()
			{
				ret = ERR_CODE_DATA_NOT_EXISTS;
				break;
			}

			CChatMongoMgr::updateMsgExtendType(Msgid,req.groupid(),req.msg().extendtype());
			break;
		}
	/*	if (!CUserGroupinfo::isMember(req.msg().srcuser().userid(), req.groupid()))
		{
			LOG_PRINT(log_warning, "user[%u] is not the member of group[%u].", req.msg().srcuser().userid(), req.groupid());
			return false;
		}
	*/
		uint32 msgid;
		if(blobby)
		{
			if (!CChatMongoMgr::getNextSequence(CChatCommon::getPChatSeqName(req.groupid()), msgid))
			{
				LOG_PRINT(log_error, "getNextSequence error, groupid: %u", req.groupid());
				ret = ERR_CODE_FAILED_DB;
				break;
			}
		}else
		{
			if (!CChatMongoMgr::getNextSequence(CChatCommon::getGChatSeqName(req.groupid()), msgid))
			{
				LOG_PRINT(log_error, "getNextSequence error, groupid: %u", req.groupid());
				ret = ERR_CODE_FAILED_DB;
				break;
			}
		}


		//私聊(聊天室评论,聊天室里面的评论回复 3020) 普通用户需要审核 
		//普通用户上墙3010需要审核
		if (subcmd == Sub_Vchat_GroupPrivateMsgReq ||req.msg().msgtype() == MSGTYPE_UPLOADWALL)
		{
			
			//查审核开关
			std::string live_comment_statu,course_comment_status;
			CChatRedisMgr::getCommentAuditState(live_comment_statu,course_comment_status);
			LOG_PRINT(log_info, "live_comment_statu: %s,course_comment_status:%s,src userid=%d ,groupid:%d", live_comment_statu.c_str(),course_comment_status.c_str(),req.msg().srcuser().userid(), req.groupid());
			if ((req.groupid()>BROADCAST_ROOM_BEGIN_NUM && live_comment_statu=="1")|| (req.groupid()<BROADCAST_ROOM_BEGIN_NUM && course_comment_status=="1" ))
			{
				
				m_isCommentAudit=1;
				commentAudit tcommentAudit;

				if(CDBSink().get_jiabing_DB(req.msg().srcuser().userid(), req.groupid()) == 0)
				{
					LOG_PRINT(log_info, " is jiabing ,groupid:%d,src userid=%d",req.groupid(),req.msg().srcuser().userid());
					tcommentAudit.iStatus=1;
				}
				else if (CDBSink().qryUserAssistantType(req.msg().srcuser().userid(), req.groupid()))
				{
					LOG_PRINT(log_info, " is UserAssistant ,groupid:%d,src userid=%d",req.groupid(),req.msg().srcuser().userid());
					tcommentAudit.iStatus=1;
				}
				else if (CDBSink().get_isroommanager_DB(req.msg().srcuser().userid(), req.groupid())==0)
				{
					LOG_PRINT(log_info, " is teacher ,groupid:%d,src userid=%d",req.groupid(),req.msg().srcuser().userid());
					tcommentAudit.iStatus=1;
				}
				else if (CDBSink().check_isMaJia(req.msg().srcuser().userid())==0)
				{
					LOG_PRINT(log_info, " is isMaJia ,groupid:%d,src userid=%d",req.groupid(),req.msg().srcuser().userid());
					tcommentAudit.iStatus=1;
				}
				else
				{
					tcommentAudit.iStatus=2;
				}
				
				//入评论审核表
				
				tcommentAudit.iUid=req.msg().srcuser().userid();
				tcommentAudit.iGroupid=req.groupid();
				tcommentAudit.iMsgid=msgid;
				tcommentAudit.sContent=req.msg().content();

				tcommentAudit.iPmsgid=req.msg().mastermsgid();
				tcommentAudit.iMsgtype=req.msg().msgtype();

				CDBTools::insertCommentAudit(tcommentAudit);

				if (tcommentAudit.iStatus==2)
				{
					ret = ERR_CODE_COMMENT_NEED_AUDIT;
					break;
				}
				
			}


		}


//PASS_AUDIT:
		uint32 now = time(NULL);
		req.mutable_msg()->set_msgtime(now);
		req.mutable_msg()->set_msgid(msgid);

		//更新用户最后聊天时间
		if (req.msg().msgtype() != MSGTYPE_REMIND && req.msg().msgtype() != MSGTYPE_RECALL)
			CChatMongoMgr::updUserGroupChatTime(req.msg().srcuser().userid(), req.groupid(), now);

		//写入mongodb
		if (!CChatMongoMgr::writeGroupMsg(req,blobby))
		{
			ret = ERR_CODE_FAILED_DB;
			break;
		}

		if (req.msg().msgtype() == MSGTYPE_PPT_VOICE)//ppt直播间语音
		{
			/*string sPicMediaId;
			Json::Reader reader(Json::Features::strictMode());
			Json::Value root;

			if (reader.parse(req.msg().content(), root))
			{
				if (root.isMember("picmediaid"))
				{
					Json::Value value = root["picmediaid"];
					sPicMediaId=value.asString();
				}
			}*/
			std::string sVoiceMediaId;
			std::string sPicMediaId;
			if (!getMediaId(req.msg().content(),sVoiceMediaId,sPicMediaId))
			{
				LOG_PRINT(log_error, "MSGTYPE_PPT_VOICE  parse sVoiceMediaId error, groupid: %u,req.msg().content()=%s", req.groupid(),req.msg().content().c_str());
				break;
			}


			if (!sPicMediaId.empty())
			{
				vector<int32> vAddMsgId;
				vAddMsgId.push_back(req.msg().msgid());

				CChatMongoMgr::updatePPTrelationMsg(req.groupid(),sPicMediaId,vAddMsgId);
			}
			
		}
	} while (0);

	return ret;
}

void CChatLogic::notifyGroupMsg(const CMDGroupMsgReq &req, bool bOnlooker/* = false*/, bool onlineOnly/* = false*/, bool memberOnly/* = false*/,bool blobby /* = false */)
{
	//notify user 
	CMDGroupMsgNotify notify;
	notify.set_groupid(req.groupid());
	notify.mutable_msg()->CopyFrom(req.msg());
	getUserInfo(*notify.mutable_msg()->mutable_srcuser());
	int inroomstate = 1;
	if(CDBSink().get_isroommanager_DB(req.msg().srcuser().userid(), req.groupid()) == 0)
	{
		inroomstate = 10;
	}
	if(CDBSink().get_jiabing_DB(req.msg().srcuser().userid(), req.groupid()) == 0)
	{
		inroomstate = 10;
	}
	notify.mutable_msg()->mutable_srcuser()->set_roletype(inroomstate);
	bool buserType = CDBSink().qryUserAssistantType(req.msg().srcuser().userid(), req.groupid());
	notify.mutable_msg()->mutable_srcuser()->set_usertype(buserType);
	int nProtoLen = notify.ByteSize();
	SL_ByteBuffer buff(SIZE_IVM_HEAD_TOTAL + nProtoLen);
	if(blobby)
	{
		CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), 0, (bOnlooker ? Sub_Vchat_GroupOnlookerChatNotify: Sub_Vchat_GroupPrivateMsgNotify), SIZE_IVM_HEAD_TOTAL + nProtoLen);
	}
	else
	{
		LOG_PRINT(log_debug, "Notify subcmd:%d", Sub_Vchat_GroupMsgNotify);
		CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), 0, (bOnlooker ? Sub_Vchat_GroupOnlookerChatNotify: Sub_Vchat_GroupMsgNotify), SIZE_IVM_HEAD_TOTAL + nProtoLen);
	}
	DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
	notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
	LOG_PRINT(log_debug, "Notify group[%u] srcuid[%u].nProtoLen:%d", req.groupid(), req.msg().srcuser().userid(),nProtoLen);
	if (memberOnly)
	{
		CMsgComm::Build_BroadCastRoomRoleType(pGateMask, req.groupid(), e_MemberRole);
		AppInstance()->m_GateConnMgr.sendMsgToGate(buff.buffer(), buff.buffer_size());
	}
	else
	{
		CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, notify.groupid());
		if (AppInstance()->m_GateConnMgr.sendMsgToGate(buff.buffer(), buff.buffer_size()))
		{
			if (bOnlooker)
			{
				LOG_PRINT(log_debug, "Notify Onlooker group chat msg of group[%u] srcuid[%u].", req.groupid(), req.msg().srcuser().userid());
			}
			else
			{
				LOG_PRINT(log_debug, "[ONLINE]Notify group chat msg of group[%u] srcuid[%u], msgid[%u].", req.groupid(), req.msg().srcuser().userid(), req.msg().msgid());
			}
		}
		else
		{
			LOG_PRINT(log_warning, "cast group msg error, group[%u] msgid[%u].", req.groupid(), req.msg().msgid());
		}
	}

#if 0

	if (!bOnlooker && !onlineOnly)
	{
		CThriftHttpClient<TpPushSvrConcurrentClient> client(e_tppushsvr_type);
		if (client)
		{
			TChatMsgPush push;
			push.msg_type = e_PushMsgType::GroupChat;
			push.srcuid = notify.msg().srcuser().userid();
			CGroupBasicInfo::getValue(req.groupid(), CGroupBasicInfo::e_Group_Name, push.title);
			push.title = notify.msg().srcuser().alias() + "（" + push.title + "）";	//推送标题：聊天者（群名称）
			push.description = genPushMsgContent(notify.msg());			//推送描述
			push.create_time = notify.msg().msgtime();
			push.include_muted = (MSGTYPE_REDPACK == notify.msg().msgtype());

			int atsize = notify.msg().atlist_size();
			if (atsize)
			{
				push.userids.resize(atsize);
				for (int i = 0; i < atsize; i++)
				{
					push.userids[i] = notify.msg().atlist(i);
				}
			}

			client->proc_notifyChatMsg(push, req.groupid(), true);
			LOG_PRINT(log_debug, "[OFFLINE]Notify group chat msg of group[%u] srcuid[%u], msgid[%u].", req.groupid(), req.msg().srcuser().userid(), req.msg().msgid());
		}
	}
#endif

}

void CChatLogic::procGroupMsgNotifyRecv(task_proc_data &task_data)
{
	CMDGroupMsgNotifyRecv recv;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, recv))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", recv.GetTypeName().c_str());
		return;
	}
	LOG_PRINT(log_debug, "user[%u] has received msg[%u] from group[%u]", recv.rcvuid(), recv.msgid(), recv.groupid());
	if (!recv.rcvuid() || !recv.msgid() || !recv.groupid())
	{
		task_data.resperrinf(ERR_CODE_INVALID_PARAMETER);
		LOG_PRINT(log_warning, "check param failed.");
		return;
	}

	if (!CChatMongoMgr::updUserGroupMsgId(recv.rcvuid(), recv.groupid(), recv.msgid()))
	{
		LOG_PRINT(log_warning, "updUserGroupMsgId error.");
	}
}
bool CChatLogic::dealUnsuccessData()
{
	std::vector<int> vgids;
	vgids.push_back(618);
	vgids.push_back(620);
	vgids.push_back(624);
	vgids.push_back(626);
	vgids.push_back(628);
	vgids.push_back(632);
	vgids.push_back(634);
	vgids.push_back(640);
	vgids.push_back(648);
	vgids.push_back(650);
	vgids.push_back(654);
	vgids.push_back(658);
	vgids.push_back(660);
	vgids.push_back(662);
	vgids.push_back(664);
	vgids.push_back(666);
	vgids.push_back(668);
	vgids.push_back(670);
	vgids.push_back(682);
	vgids.push_back(684);
	vgids.push_back(688);
	vgids.push_back(690);
	vgids.push_back(694);
	vgids.push_back(700);
	vgids.push_back(704);
	vgids.push_back(708);
	vgids.push_back(712);
	vgids.push_back(714);
	vgids.push_back(716);
	vgids.push_back(718);
	vgids.push_back(720);
	vgids.push_back(722);
	vgids.push_back(724);
	vgids.push_back(726);
	vgids.push_back(736);
	vgids.push_back(738);
	vgids.push_back(740);
	vgids.push_back(742);
	vgids.push_back(744);
	vgids.push_back(746);
	vgids.push_back(748);
	vgids.push_back(750);
	vgids.push_back(754);
	vgids.push_back(756);
	vgids.push_back(758);
	vgids.push_back(762);
	vgids.push_back(766);
	vgids.push_back(774);
	vgids.push_back(778);
	vgids.push_back(780);
	vgids.push_back(782);
	vgids.push_back(784);
	vgids.push_back(786);
	vgids.push_back(796);
	vgids.push_back(798);
	vgids.push_back(800);
	vgids.push_back(804);
	vgids.push_back(810);
	vgids.push_back(814);
	vgids.push_back(816);
	vgids.push_back(818);
	vgids.push_back(820);
	vgids.push_back(822);
	vgids.push_back(824);
	vgids.push_back(828);
	vgids.push_back(830);
	vgids.push_back(832);
	vgids.push_back(834);
	vgids.push_back(836);
	vgids.push_back(838);
	vgids.push_back(840);
	vgids.push_back(842);
	vgids.push_back(844);
	vgids.push_back(846);
	vgids.push_back(848);
	vgids.push_back(850);
	vgids.push_back(858);
	vgids.push_back(860);
	vgids.push_back(862);
	vgids.push_back(864);
	vgids.push_back(866);
	vgids.push_back(868);
	vgids.push_back(870);
	vgids.push_back(876);
	vgids.push_back(878);
	vgids.push_back(880);
	vgids.push_back(882);
	vgids.push_back(890);
	vgids.push_back(894);
	vgids.push_back(900);
	vgids.push_back(908);
	vgids.push_back(910);
	vgids.push_back(912);
	vgids.push_back(914);
	vgids.push_back(916);
	vgids.push_back(918);
	vgids.push_back(920);
	vgids.push_back(928);
	vgids.push_back(930);
	vgids.push_back(932);
	vgids.push_back(934);
	vgids.push_back(938);
	vgids.push_back(940);
	vgids.push_back(942);
	vgids.push_back(944);
	vgids.push_back(946);
	vgids.push_back(948);
	vgids.push_back(950);
	vgids.push_back(952);
	vgids.push_back(958);
	vgids.push_back(960);
	vgids.push_back(968);
	vgids.push_back(970);
	vgids.push_back(972);
	vgids.push_back(974);
	vgids.push_back(976);
	vgids.push_back(978);
	vgids.push_back(980);
	vgids.push_back(982);
	vgids.push_back(984);
	vgids.push_back(985);
	vgids.push_back(986);
	vgids.push_back(987);
	vgids.push_back(988);
	vgids.push_back(989);

	for(int i = 0 ; i < vgids.size() ; i++)
	{
		std::list<ChatMsg_t> lstMsg;
		//int total = 0;
		CChatMongoMgr::getgroupmsg_t(vgids[i], lstMsg);
		LOG_PRINT(log_warning, "getgroupmsg_t.size:%d.",lstMsg.size());
		for(std::list<ChatMsg_t>::iterator it = lstMsg.begin() ; it != lstMsg.end() ; it++)
		{
			if(it->msgtype() == 2)
			{
				if(it->content().find("media_id") != string::npos)
				{
					LOG_PRINT(log_warning, "getgroupmsg_t media_id");
					std::string content = it->content();
					std::string code ="";
					unsigned int codeTime = 0;
					CGlobalInfo::getGzhtoken(code,codeTime);
					//get
					content += "&access_token=";
					content += code;

					CMDGroupMsgReq req;

					req.mutable_msg()->CopyFrom(*it);
					req.set_groupid(vgids[i]);
					req.mutable_msg()->set_content(content);
					Application::get_io_service().post(boost::bind(processoffline,req));
				}
			}
		}
	}

	return true;
}

void CChatLogic::getPPTPics(const CMDGroupMsgHisReq &req,string &courseInfo)
{
	RoomObject_Ref roomObj =CChatServerApp::getInstance()->m_room_mgr->FindRoom(req.groupid());
	LOG_PRINT(log_debug, "form[%d],groupid[%u],classname[%s]", roomObj->form, roomObj->nvcbid_,roomObj->class_name.c_str());
	if (roomObj->form==3 && req.msgid()==0)//第一次进PPT课程直播间时拉
	{
		std::vector<pptPicInfo> vPicInfo;
		CDBTools::getPPTPicIdsByGroupID(req.groupid(),vPicInfo);
		if (vPicInfo.size())
		{

			Json::Value root;
			Json::Value arrayObj;
			Json::Value item;

			for (unsigned int i=0;i<vPicInfo.size();i++)
			{
				item["groupid"]=vPicInfo[i].iGroupId;
				item["rank"]=vPicInfo[i].iRank;
				item["picurl"]=vPicInfo[i].sPicUrl;
				item["picId"]=vPicInfo[i].sPicId;

				arrayObj.append(item);
			}

			root["pics"]=arrayObj;

			Json::FastWriter fast_writer;
			courseInfo = fast_writer.write(root);

			//root.toStyledString()
			LOG_PRINT(log_debug, "groupid[%u], courseinfo[%s]", roomObj->nvcbid_,courseInfo.c_str());
			//pptGMsg.set_msgtype(MSGTYPE_PPT_PIC_SHOW);

		}
	}

	return ;
}
void CChatLogic::procGroupMsgHisReq(task_proc_data &task_data)
{
	CMDGroupMsgHisReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}
	LOG_PRINT(log_debug, "request from user[%u] groupid[%u] forward[%d] msgid[%u] count[%u] querytime[%ld]", req.userid(), req.groupid(), req.forward(), req.msgid(), req.count(),req.querytime());

	if (!req.userid() || !req.groupid())
	{
		task_data.resperrinf(ERR_CODE_INVALID_PARAMETER);
		LOG_PRINT(log_debug, "check param failed, user[%u] groupid[%u]", req.userid(), req.groupid());
		return;
	}

//	if (!CUserGroupinfo::isMember(req.userid(), req.groupid()))
//	{
//		task_data.resperrinf(ERR_CODE_USER_NOT_IN_GROUP);
//		LOG_PRINT(log_warning, "user[%u] is not the member of group[%u].", req.userid(), req.groupid());
//		return;
//	}

	CMDGroupMsgHisResp resp;
	resp.set_groupid(req.groupid());
	resp.set_reversemsgorder(req.reversemsgorder());
	bool needsCurrentDayMsg = (0 == req.querytime());
	uint32 msgCount = 0;
	if (needsCurrentDayMsg)
		CChatMongoMgr::queryCurrentDayGroupMsgCount(req.groupid(),msgCount);
	resp.set_hismsgcount(msgCount);
	LOG_PRINT(log_debug, "req reversemsgorder: %d, resp reversemsgorder: %d, resp hismsgcount: %d",
		req.reversemsgorder(), resp.reversemsgorder(), resp.hismsgcount());
	resp.set_reqtag(req.reqtag());
	LOG_PRINT(log_debug, "req reqtag: %s, resp reqtag: %s", req.reqtag().c_str(), resp.reqtag().c_str());

	std::list<ChatMsg_t> lstMsg;
	int total = 0;

	string courseInfo;

	//如果是ppt课程直播间,需要把ppt图片返回	
	getPPTPics(req,courseInfo);
	
	if (!courseInfo.empty())
	{
		resp.set_courseinfo(courseInfo);
	}

	total = CChatMongoMgr::qryGroupMsgList_V2(req.userid(), req.groupid(), lstMsg,req.querytime(), req.msgid(), req.count(), req.forward());
	if (total > 0)
	{
		for (std::list<ChatMsg_t>::iterator it = lstMsg.begin(); it != lstMsg.end(); it++)
		{
			LOG_PRINT(log_info, "==msgid[%d],srcUser.Uid[%d],content[%s]",it->msgid(),it->mutable_srcuser()->userid(),it->content().c_str());//,


			if (it->msgtype() == MSGTYPE_CLOSE_LIVE_COURSE)
				continue;
			ChatMsg_t *pGMsg = resp.add_msgs();
		
			pGMsg->CopyFrom(*it);
			getUserInfoWithMap(*pGMsg->mutable_srcuser(), req.groupid());
			if (resp.ByteSize() >= MAX_CHAT_PACKET_SIZE)
			{
				break;
			}
		}
		m_mapUser.clear();
	}

	task_data.respProtobuf(resp, Sub_Vchat_GroupMsgHisResp);
	LOG_PRINT(log_info, "send group msg his: group[%u] user[%u] from msgid[%u] total[%u] send[%u]", 
		req.groupid(), req.userid(), req.msgid(), total, resp.msgs_size());
}

void CChatLogic::procUnreadGroupMsgReq(task_proc_data &task_data)
{
	CMDUnreadGroupMsgReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}

	LOG_PRINT(log_info, "procUnreadGroupMsgReq begin: user[%u] group[%u]", req.userid(), req.groupid());
	if (!req.userid() || !req.groupid())
	{
		task_data.resperrinf(ERR_CODE_INVALID_PARAMETER);
		LOG_PRINT(log_warning, "check param failed, user[%u] group[%u]", req.userid(), req.groupid());
		return;
	}

// 	if (!CUserGroupinfo::isMember(req.userid(), req.groupid()))
// 	{
// 		task_data.resperrinf(ERR_CODE_USER_NOT_IN_GROUP);
// 		LOG_PRINT(log_warning, "user[%u] is not the member of group[%u].", req.userid(), req.groupid());
// 		return;
// 	}
	
	//send unread group chat
	size_t count = 0, unread = 0, total = 0;
	int maxnum = 20;	//当前默认只下发20条属于该用户的未读消息
	CMDUnreadGroupMsgNotify notify;
	notify.set_groupid(req.groupid());
	std::list<ChatMsg_t> lstMsg;
	if ((total = CChatMongoMgr::qryUnreadGroupMsg(req.groupid(), req.userid(), lstMsg, 0)) > 0)
	{
		for (std::list<ChatMsg_t>::iterator it = lstMsg.begin(); it != lstMsg.end();it++)
		{
			if (count < maxnum && notify.ByteSize() < MAX_CHAT_PACKET_SIZE)
			{
				ChatMsg_t *pMsg = notify.add_msgs();
				pMsg->CopyFrom(*it);
				getUserInfoWithMap(*pMsg->mutable_srcuser(), req.groupid());

				//用户真正关注的未读消息
				if (it->msgtype() != MSGTYPE_REMIND || it->dstuser().userid() == req.userid() || it->dstuser().userid() == 0)
				{
					count++;
				}
			}

			//计算未读数的消息，MSGTYPE_REMIND消息不算未读数
			if (it->msgtype() != MSGTYPE_REMIND)
			{
				unread++;
			}

			for (int i = 0; i < it->atlist_size(); i++)
			{
				if (it->atlist(i) == 0 || it->atlist(i) == req.userid())
				{
					notify.add_atmsgidlist(it->msgid());
					break;
				}
			}
		}
	}
	notify.set_unreadcount(unread);
	task_data.respProtobuf(notify, Sub_Vchat_UnreadGroupMsgNotify);
	LOG_PRINT(log_info, "response unread group msg: user[%u] group[%u] count[%u] unread[%u] send[%u] atlist[%u]", req.userid(), req.groupid(), count, unread, notify.msgs_size(), notify.atmsgidlist_size());
	
	//send unread group private chat
	std::map<uint32, uint32> mGPchatMsgId;
	if (CChatMongoMgr::qryUserGroupPMsgIdList(req.userid(), req.groupid(), mGPchatMsgId))
	{
		for (std::map<uint32, uint32>::iterator itMsgid = mGPchatMsgId.begin(); itMsgid != mGPchatMsgId.end(); itMsgid++)
		{
			std::list<ChatMsg_t> lstPMsg;
			count = 0;
			unread = 0;
			total = CChatMongoMgr::qryUnreadGroupPrivateMsg(req.groupid(), req.userid(), itMsgid->first, lstPMsg, itMsgid->second, 0);
			if (total > 0)
			{
				CMDUnreadGroupPrivateMsgNotify notify;
				notify.set_groupid(req.groupid());
				notify.set_dstuid(itMsgid->first);

				for (std::list<ChatMsg_t>::iterator iter = lstPMsg.begin(); iter != lstPMsg.end(); iter++)
				{
					if (count < maxnum && notify.ByteSize() < MAX_CHAT_PACKET_SIZE)
					{
						ChatMsg_t *pPMsg = notify.add_msgs();
						pPMsg->CopyFrom(*iter);
						getUserInfoWithMap(*pPMsg->mutable_srcuser());

						//用户真正关注的未读消息
						if (iter->msgtype() != MSGTYPE_REMIND || iter->dstuser().userid() == req.userid() || iter->dstuser().userid() == 0)
						{
							count++;
						}
					}

					//计算未读数的消息，MSGTYPE_REMIND消息不算未读数
					if (iter->msgtype() != MSGTYPE_REMIND)
					{
						unread++;
					}
				}

				notify.set_unreadcount(unread);
				task_data.respProtobuf(notify, Sub_Vchat_UnreadGroupPrivateMsgNotify);
				LOG_PRINT(log_info, "send unread group private msg: group[%u] user[%u] dstuser[%u] msgid[%u] count[%u] unread[%u] send[%u]", 
					req.groupid(), req.userid(), notify.dstuid(), itMsgid->second, count, unread, notify.msgs_size());
			}
		}
	}
	m_mapUser.clear();

	CMDUInt32 end;
	end.set_data(req.groupid());
	task_data.respProtobuf(end, Sub_Vchat_GroupUnreadMsgEnd);
	LOG_PRINT(log_info, "procUnreadGroupMsgReq end: user[%u] group[%u]", req.userid(), req.groupid());
}

bool CChatLogic::procUserJoinGroupNotify(uint32 groupid, uint32 userid, uint32 roletype)
{
	if (!groupid || !userid)
	{
		return false;
	}

	//新进群用户需要更新begin_msgid，以免查到进群之前的历史聊天消息
	uint32 msgid = 0;
	CChatMongoMgr::qryMaxGroupMsgId(groupid, msgid);
	bool ret = CChatMongoMgr::updUserGroupBeginMsgId(userid, groupid, msgid);

	if (ret && roletype != e_VisitorRole)
	{
		//发送进群身份变更群助手消息
		StGPAssistMsg_t oMsg;
		oMsg.userID = userid;
		oMsg.groupID = groupid;
		oMsg.msgSvrLevel = roletype;
		oMsg.msgSvrLevelUp = true;
		oMsg.msgSvrType = MSGTYPE_LEVEL_CHG;
		oMsg.msgTime = time(NULL);
		CGroupAssistLogic::addAssistMsg(oMsg, false, true);
	}
	return ret;
}

void CChatLogic::procGroupPMsgReq(task_proc_data &task_data)
{
	CMDGroupMsgReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}
	LOG_PRINT(log_info, "recv group msg of group:%u, srcuid: %u.", req.groupid(), req.msg().srcuser().userid());
	int msgtype = req.msg().msgtype();
	if(msgtype == 0 )
	{
		std::string content = req.msg().content();
		CKeywordMgr::replaceKey(content);
		req.mutable_msg()->set_content(content);
	}
	COM_MSG_HEADER* in_msg = (COM_MSG_HEADER*)task_data.pdata;
	int ret = dealGroupMsgReq(req,true,in_msg->subcmd);

	//send receive to src user
	CMDGroupMsgRecv recv;
	recv.mutable_errinfo()->set_errid(ret);
	recv.set_groupid(req.groupid());
	recv.set_msgid(req.msg().msgid());
	recv.set_clientmsgid(req.msg().clientmsgid());
	task_data.respProtobuf(recv, Sub_Vchat_GroupPrivateMsgRecv);

	string sCmd=req.groupid()>BROADCAST_ROOM_BEGIN_NUM?"ANSWER_LIVE_ROOM_QUESTION":"ANSWER_COURSE_ROOM_QUESTION";

	if (ERR_CODE_SUCCESS != ret)
	{
		if (ERR_CODE_COMMENT_NEED_AUDIT == ret && req.msg().mastermsgid())
		{
			notifyPHPMsgCenter(req,sCmd);
		}
		LOG_PRINT(log_warning, "dealGroupMsgReq %s, group[%u] srcuid[%u],ret[%d].", CErrMsg::strerror(ret).c_str(), req.groupid(), req.msg().srcuser().userid(),ret);
		return ;
	}
	if(msgtype != 0)
	{
		std::string content = req.msg().content();
		std::string code ="";
		unsigned int codeTime = 0;
		CGlobalInfo::getGzhtoken(code,codeTime);
		//get
		content += "&access_token=";
		content += code;
		req.mutable_msg()->set_content(content);
	}
	notifyGroupMsg(req, false, isMsgPushOnlineOnly(req.msg()),false,true);

	//通知php消息中心
	if (req.msg().mastermsgid())
	{
		notifyPHPMsgCenter(req,sCmd);
	}
	
}


int CChatLogic::notifyPHPMsgCenter(const CMDGroupMsgReq &req,const std::string &sCmd)
{

	int ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();

	CHttpClient httpClient;

	string url = CDBSink().qrySysParameterString("NOTIFY_COMMENT");
	if (url.empty())
	{
		LOG_PRINT(log_error, "NOTIFY_COMMENT url empty!");
		return -1;
	}
	string strcontent,strresp;

	Json::Value root;
	Json::FastWriter writer;  

	if (sCmd=="LIVE_ROOM_UPDATE")
	{
		Json::Value replaceArray;

		map<string,string> mUserInfo;
		if (CChatRedisMgr::getUserInfoByUid(req.msg().srcuser().userid(),mUserInfo))
		{
			replaceArray["lead"].append(mUserInfo["alias"]);//老师呢称
			replaceArray["content"].append(mUserInfo["alias"]);//老师呢称
		}

		root["replaceArray"]=replaceArray;
	}
	else if (sCmd=="ANSWER_LIVE_ROOM_QUESTION" || sCmd=="ANSWER_COURSE_ROOM_QUESTION")
	{
		root["const"]=sCmd;

		Json::Value linkInfos;
		Json::Value replaceArray;
		Json::Value userIdList;

		linkInfos["teacherId"]=req.msg().srcuser().userid();
		root["linkInfos"]=linkInfos;

		map<string,string> mUserInfo;
		
		ChatMsg_t msg;
		CChatMongoMgr::qryGroupMsg(req.groupid(),req.msg().mastermsgid(),msg,true);

	    if (CChatRedisMgr::getUserInfoByUid(req.msg().srcuser().userid(),mUserInfo))
	    {
			replaceArray["lead"].append(mUserInfo["alias"]);//老师呢称
			replaceArray["content"].append(mUserInfo["alias"]);//老师呢称
			replaceArray["content"].append(req.msg().content());//;老师回复内容
			replaceArray["content"].append(msg.content());//学生提问
	    }
		

		root["replaceArray"]=replaceArray;

		
		userIdList.append(msg.srcuser().userid());
		root["userIdList"]=userIdList;

	}

	strcontent=writer.write(root);
	string headtype="Content-Type:application/json;charset=UTF-8";
	httpClient.Post_Header(headtype,url,strcontent,strresp,200);

	int ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();

	LOG_PRINT(log_debug, "Notice push to web: %s,req:%s,", url.c_str(),strcontent.c_str());
	LOG_PRINT(log_debug, "Notice push to web: resp: %s,take [%d] msc", strresp.c_str(),ntime_end-ntime_begin);
	return 0;

}
int CChatLogic::dealGroupPMsgReq(CMDGroupPrivateMsgReq &req, CMDGroupPrivateMsgRecv *recv/* = NULL*/)
{
	if (!req.groupid() || !CChatCommon::chkPMsgValid(req.msg()))
	{
		LOG_PRINT(log_warning, "check param failed, group[%u] srcuid[%u] dstuid[%u] msgtype[%d] msgsize[%u]", req.groupid(), req.msg().srcuser().userid(), req.msg().dstuser().userid(), req.msg().msgtype(), req.msg().content().size());
		return ERR_CODE_INVALID_PARAMETER;
	}
	uint32 groupid = req.groupid();
	uint32 srcuid = req.msg().srcuser().userid();
	uint32 dstuid = req.msg().dstuser().userid();

	//消息撤回
	if (req.msg().msgtype() == MSGTYPE_RECALL)
	{
		uint32 oldMsgid = atoi(req.msg().content().c_str());
		ChatMsg_t oldMsg;
		if (!CChatMongoMgr::qryGroupPrivateMsg(groupid, srcuid, dstuid, oldMsgid, oldMsg) || oldMsg.srcuser().userid() != req.msg().srcuser().userid())
		{
			return ERR_CODE_RECALL_NOT_EXISTS;
		}

		if (!CChatMongoMgr::recallGroupPrivateMsg(req.groupid(), srcuid, dstuid, oldMsgid))
		{
			return ERR_CODE_FAILED_DB;
		}

		LOG_PRINT(log_info, "group:%u, user:%u dstuid:%u has recall msg:%u.", groupid, srcuid, dstuid, oldMsgid);
	}

	if (req.msg().msgtype() != MSGTYPE_REDPACK && req.msg().msgtype() != MSGTYPE_REMIND)
	{
		uint32 threshold = 0, intimacy = 0;
		int ret = CCommonLogic::chkGroupPrivateChatQualitication(*AppInstance()->m_pRedisMgr, groupid, srcuid, dstuid, &threshold, &intimacy);
		if (ERR_CODE_SUCCESS != ret)
		{
			if (ERR_CODE_GROUP_PRIVATE_CHAT_LIMIT == ret && recv)
			{
				recv->set_threshold(threshold);	//对方私聊门槛
				recv->set_intimacy(intimacy);	//当前与对方的亲密度
			}
			LOG_PRINT(log_warning, "chkGroupPrivateChatQualitication error, group[%u] user[%u] dst[%u] error[%s].", groupid, srcuid, dstuid, CErrMsg::strerror(ret).c_str());
			return ret;
		}
	}

	uint32 msgid = 0;
	if (!CChatMongoMgr::getNextSequence(CChatCommon::getGPChatSeqName(groupid, srcuid, dstuid), msgid))
	{
		LOG_PRINT(log_error, "getNextSequence error, srcuid[%u] dstuid[%u] group[%u]", srcuid, dstuid, groupid);
		return ERR_CODE_FAILED;
	}
	req.mutable_msg()->set_msgid(msgid);
	req.mutable_msg()->set_msgtime(time(NULL));

	if (FIRST_SEQUENCE_VALUE == msgid)
	{
		CChatMongoMgr::updUserGroupPMsgId(srcuid, groupid, dstuid, msgid);
		CChatMongoMgr::updUserGroupPMsgId(dstuid, groupid, srcuid, 0);
	}

	if (!CChatMongoMgr::writeGroupPrivateMsg(req))
	{
		LOG_PRINT(log_error, "writeGroupPrivateMsg error.");
		return ERR_CODE_FAILED_DB;
	}

	return ERR_CODE_SUCCESS;
}

void CChatLogic::notifyGroupPMsg(const CMDGroupPrivateMsgReq &req, bool onlineOnly/* = false*/)
{
	//notify user 
	CMDGroupPrivateMsgNotify notify;
	notify.set_groupid(req.groupid());
	notify.mutable_msg()->CopyFrom(req.msg());
	getUserInfo(*notify.mutable_msg()->mutable_srcuser());
	bool buserType = CDBSink().qryUserAssistantType(req.msg().srcuser().userid(), req.groupid());
	notify.mutable_msg()->mutable_srcuser()->set_usertype(buserType);
	int nProtoLen = notify.ByteSize();
	int len = SIZE_IVM_HEAD_TOTAL + nProtoLen;
	SL_ByteBuffer buff(len);
	CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), 0, Sub_Vchat_GroupPrivateMsgNotify, len);
	DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
	notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);

	if (AppInstance()->m_rdsOnlineUser.exist(req.msg().dstuser().userid()))
	{
		CMsgComm::Build_BroadCastUser_Gate(pGateMask, notify.msg().dstuser().userid());
		if (AppInstance()->m_GateConnMgr.sendMsgToGate(buff.buffer(), buff.buffer_size()))
		{
			LOG_PRINT(log_debug, "[ONLINE]Notify group private chat msg to user[%u], from [%u] to [%u], msgid[%u].",
				notify.msg().dstuser().userid(), req.msg().srcuser().userid(), req.msg().dstuser().userid(), req.msg().msgid());
		}
	}
	else if (notify.msg().msgtype() != MSGTYPE_REMIND && !onlineOnly)
	{
		CThriftHttpClient<TpPushSvrConcurrentClient> client(e_tppushsvr_type);
		if (client)
		{
			TChatMsgPush push;
			push.msg_type = e_PushMsgType::GroupPrivateChat;
			push.srcuid = notify.msg().srcuser().userid();
			push.title = notify.msg().srcuser().alias();
			push.description = genPushMsgContent(notify.msg());
			push.create_time = notify.msg().msgtime();
			push.userids.push_back(notify.msg().dstuser().userid());
			client->proc_notifyChatMsg(push, req.groupid(), false);
			LOG_PRINT(log_debug, "[OFFLINE]Notify group private chat msg to user[%u], from [%u] to [%u], msgid[%u].",
				notify.msg().dstuser().userid(), req.msg().srcuser().userid(), req.msg().dstuser().userid(), req.msg().msgid());
		}
	}
	
	if (notify.msg().srcuser().userid())
	{
		CMsgComm::Build_BroadCastUser_Gate(pGateMask, notify.msg().srcuser().userid());
		AppInstance()->m_GateConnMgr.sendMsgToGate(buff.buffer(), buff.buffer_size());
	}
}

void CChatLogic::procGroupPMsgNotifyRecv(task_proc_data &task_data)
{
	CMDGroupPrivateMsgNotifyRecv recv;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, recv))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", recv.GetTypeName().c_str());
		return;
	}

	LOG_PRINT(log_debug, "user[%u] has receive group private chat from group[%u] srcuid[%u] dstuid[%u] msgid[%u]", recv.rcvuid(), recv.groupid(), recv.srcuid(), recv.dstuid(), recv.msgid());
	if ((recv.rcvuid() != recv.srcuid() && recv.rcvuid() != recv.dstuid()) || !recv.srcuid() || !recv.dstuid() || !recv.groupid() || !recv.msgid())
	{
		LOG_PRINT(log_warning, "check param failed..");
		return;
	}

	if (!CChatMongoMgr::updUserGroupPMsgId(recv.rcvuid(), recv.groupid(), (recv.rcvuid() == recv.srcuid() ? recv.dstuid(): recv.srcuid()), recv.msgid()))
	{
		LOG_PRINT(log_warning, "updUserGroupMsgId error.");
	}
}

void CChatLogic::procGroupPMsgHisReq(task_proc_data &task_data)
{
	CMDGroupMsgHisReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}
	LOG_PRINT(log_debug, "request from user[%u] groupid[%u] forward[%d] msgid[%u] count[%u]", req.userid(), req.groupid(), req.forward(), req.msgid(), req.count());

	if (!req.userid() || !req.groupid())
	{
		task_data.resperrinf(ERR_CODE_INVALID_PARAMETER);
		LOG_PRINT(log_debug, "check param failed, user[%u] groupid[%u]", req.userid(), req.groupid());
		return;
	}

//	if (!CUserGroupinfo::isMember(req.userid(), req.groupid()))
//	{
//		task_data.resperrinf(ERR_CODE_USER_NOT_IN_GROUP);
//		LOG_PRINT(log_warning, "user[%u] is not the member of group[%u].", req.userid(), req.groupid());
//		return;
//	}

	CMDGroupMsgHisResp resp;
	uint32 msgCount = 0;
	resp.set_groupid(req.groupid());
	resp.set_reversemsgorder(req.reversemsgorder());
	LOG_PRINT(log_debug, "private: req reversemsgorder = %d, resp reversemsgorder = %d",
		req.reversemsgorder(), resp.reversemsgorder());
	resp.set_reqtag(req.reqtag());
	CChatMongoMgr::qryGroupPMsgCount(req.groupid(),msgCount);
	resp.set_hismsgcount(msgCount);
	resp.set_reqtag(resp.reqtag());
	LOG_PRINT(log_debug, "private: req reqtag: %s, resp reqtag: %s", req.reqtag().c_str(), resp.reqtag().c_str());

	std::list<ChatMsg_t> lstMsg;
	int total = 0;
	total = CChatMongoMgr::qryGroupMsgList(req.userid(), req.groupid(), lstMsg, req.msgid(), req.count(), req.forward(),true);
	if (total > 0)
	{
		for (std::list<ChatMsg_t>::iterator it = lstMsg.begin(); it != lstMsg.end(); it++)
		{
			ChatMsg_t *pGMsg = resp.add_msgs();
			pGMsg->CopyFrom(*it);
			getUserInfoWithMap(*pGMsg->mutable_srcuser());
			int inroomstate = 1;
			if(CDBSink().get_isroommanager_DB(pGMsg->mutable_srcuser()->userid(), req.groupid()) == 0)
			{
				inroomstate = 10;
			}
			if(CDBSink().get_jiabing_DB(pGMsg->mutable_srcuser()->userid(), req.groupid()) == 0)
			{
				inroomstate = 10;
			}

			pGMsg->mutable_srcuser()->set_roletype(inroomstate);

			bool buserType = CDBSink().qryUserAssistantType(pGMsg->mutable_srcuser()->userid(), req.groupid());

			pGMsg->mutable_srcuser()->set_usertype(buserType);

			if (resp.ByteSize() >= MAX_CHAT_PACKET_SIZE)
			{
				break;
			}
		}
		m_mapUser.clear();
	}

	task_data.respProtobuf(resp, Sub_Vchat_GroupPrivateMsgHisResp);
	LOG_PRINT(log_info, "send group msg his: group[%u] user[%u] from msgid[%u] total[%u] send[%u]",
		req.groupid(), req.userid(), req.msgid(), total, resp.msgs_size());


//	CMDGroupPrivateMsgHisReq req;
//	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
//	{
//		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
//		return;
//	}
//
//	if (!req.groupid() || !req.requid() || !req.dstuid())
//	{
//		task_data.resperrinf(ERR_CODE_INVALID_PARAMETER);
//		LOG_PRINT(log_warning, "check param failed, group[%u] user[%u] dstuid[%u]", req.groupid(), req.requid(), req.dstuid());
//		return;
//	}
//	LOG_PRINT(log_info, "receive group private msg history request: group[%u] user[%u] dstuid[%u] forward[%d] msgid[%u] count[%u]",
//		req.groupid(), req.requid(), req.dstuid(), req.forward(), req.msgid(), req.count());
//
//	if (0 == req.count() || req.count() > 50)
//	{
//		req.set_count(50);
//	}
//
//	CMDGroupPrivateMsgHisResp resp;
//	resp.set_requid(req.requid());
//	resp.set_groupid(req.groupid());
//	resp.set_dstuid(req.dstuid());
//
//	std::list<ChatMsg_t> lstPmsg;
//	int total = CChatMongoMgr::qryGroupPrivateMsgList(req.groupid(), req.requid(), req.dstuid(), lstPmsg, req.msgid(), req.count(), req.forward());
//	if (total > 0)
//	{
//		for (std::list<ChatMsg_t>::iterator it = lstPmsg.begin(); it != lstPmsg.end(); it++)
//		{
//			ChatMsg_t *pPMsg = resp.add_msgs();
//			pPMsg->CopyFrom(*it);
//			getUserInfoWithMap(*pPMsg->mutable_srcuser());
//			if (resp.ByteSize() >= MAX_CHAT_PACKET_SIZE)
//				break;
//		}
//		m_mapUser.clear();
//	}
//
//	task_data.respProtobuf(resp, Sub_Vchat_GroupPrivateMsgHisResp);
//	LOG_PRINT(log_info, "send group private msg history list: group[%u] user[%u] dstuid[%u] total[%u] send[%u]",
//		req.groupid(), req.requid(), req.dstuid(), total, resp.msgs_size());
}

void CChatLogic::procGroupOnlookerChatReq(task_proc_data &task_data)
{
	CMDGroupMsgReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}
	if (!req.groupid() || !CChatCommon::chkGMsgValid(req.msg()))
	{
		task_data.resperrinf(ERR_CODE_INVALID_PARAMETER);
		LOG_PRINT(log_warning, "check param failed, group[%u], srcuid[%u].", req.groupid(), req.msg().srcuser().userid());
		return;
	}
	LOG_PRINT(log_info, "recv onlooker msg of group:%u, srcuid: %u.", req.groupid(), req.msg().srcuser().userid());
	CChatCommon::pickOnlookerChatUTF8(*(req.mutable_msg()->mutable_content()), 22);
	req.mutable_msg()->set_msgtime(time(NULL));
	notifyGroupMsg(req, true);
}

void CChatLogic::procGroupPChatQualiticationReq(task_proc_data &task_data)
{
	CMDGroupPChatQualiticationReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}
	uint32 groupid = req.groupid();
	uint32 userid = req.userid();
	uint32 dstuid = req.dstuid();

	LOG_PRINT(log_info, "procGroupPChatQualiticationReq begin, group[%u] user[%u] dstuid[%u].", groupid, userid, dstuid);
	CMDGroupPChatQualiticationResp resp;
	resp.set_groupid(groupid);
	resp.set_userid(userid);
	resp.set_dstuid(dstuid);
	int ret = 0;
	do
	{
		if (!groupid || !userid || !dstuid)
		{
			ret = ERR_CODE_INVALID_PARAMETER;
			resp.mutable_errinfo()->set_errid(ret);
			LOG_PRINT(log_warning, "check param failed, group[%u], userid[%u] dstuid[%u].", groupid, userid, dstuid);
			break;
		}

		uint32 threshold = 0, intimacy = 0;
		ret = CCommonLogic::chkGroupPrivateChatQualitication(*AppInstance()->m_pRedisMgr, groupid, userid, dstuid, &threshold, &intimacy);
		resp.mutable_errinfo()->set_errid(ret);
		resp.set_threshold(threshold);
		resp.set_intimacy(intimacy);
	} while(0);

	task_data.respProtobuf(resp, Sub_Vchat_GroupPChatQualiticationResp);
	LOG_PRINT(log_info, "procGroupPChatQualiticationReq %s, group[%u] user[%u] dstuid[%u] threshold[%u] intimacy[%u].",
			CErrMsg::strerror(ret).c_str(), groupid, userid, dstuid, resp.threshold(), resp.intimacy());
}

std::string CChatLogic::genPushMsgContent(const ChatMsg_t &msg, bool inAlias/* = false*/)
{
	std::string pushMsg;
	if (inAlias)
	{
		pushMsg += msg.srcuser().alias() + ":";
	}
	switch (msg.msgtype())
	{
	case MSGTYPE_TEXT:
		return pushMsg + msg.content();
	case MSGTYPE_IMAGE_WEIXIN:
	case MSGTYPE_IMAGE:
		return pushMsg + "[图片]";
	case MSGTYPE_VOICE_WEIXIN:
	case MSGTYPE_VOICE:
		return pushMsg + "[语音]";
	case MSGTYPE_VEDIO:
		return pushMsg + "[视频]";
	case MSGTYPE_REDPACK:
		return pushMsg + "[炮弹]";
	case MSGTYPE_ASSIST:
		return pushMsg + "[群助手]";
	case MSGTYPE_SYSTEMINFO:
		return pushMsg + "[系统消息]";
	default:
		return "";
	}
}

void CChatLogic::getUserInfo(UserInfo_t &user)
{
	if (user.userid() == 0)
		return;

	stUserBasicInfo info;
	if (CUserBasicInfo::getUserInfo(user.userid(), info))
	{
		user.set_head(info.strHead);
		user.set_alias(info.strAlias);
		user.set_gender((e_GenderType)info.nGender);
	}
	else
		LOG_PRINT(log_error, "CUserBasicInfo::getUserInfo(unsigned int, stUserBasicInfo &) failed");
}

void CChatLogic::getUserInfoWithMap(UserInfo_t &user, unsigned int liveid)
{
	if (user.userid() == 0)
		return;

	stUserBasicInfo &userInfo = m_mapUser[user.userid()];
	if (!userInfo.userid)
	{
		userInfo.userid = user.userid();
		CUserBasicInfo::getUserInfo(userInfo.userid, userInfo);
	}
	bool buserType = false;
	if (liveid > 0)
		buserType = CDBSink().qryUserAssistantType(user.userid(), liveid);
	user.set_usertype(buserType ? 1 : 0);
	user.set_head(userInfo.strHead);
	user.set_alias(userInfo.strAlias);
	user.set_gender((e_GenderType)userInfo.nGender);
	user.set_roletype(userInfo.inroomstate);
}

bool CChatLogic::isMsgPushOnlineOnly(const ChatMsg_t &msg)
{
	switch (msg.msgtype())
	{
	case MSGTYPE_REMIND:
	case MSGTYPE_RECALL:
		return true;
	default:
		return false;
	}
}

void CChatLogic::procDeleteChatMsgHis(task_proc_data &task_data)
{
	CMDDeleteChatMsgHis req;	
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}

	uint32 groupid = req.groupid();
	uint32 userid = req.userid();
	uint32 dstuid = req.dstuid();
	LOG_PRINT(log_info, "user[%u] request delete msg history: group[%u] dstuid[%u].", userid, groupid, dstuid);
	CMDDeleteChatMsgHisResp resp;
	resp.mutable_req()->CopyFrom(req);

	do
	{
		if (!userid || !groupid)
		{
			LOG_PRINT(log_warning, "invalid parameter: user[%u] group[%u] dstuid[%u].", userid, groupid, dstuid);
			resp.mutable_errinfo()->set_errid(ERR_CODE_INVALID_PARAMETER);
			break;
		}
		
		CChatMongoMgr::delUserMsgHistory(groupid, userid, dstuid);
		resp.mutable_errinfo()->set_errid(ERR_CODE_SUCCESS);
	} while(0);

	task_data.respProtobuf(resp, Sub_Vchat_DeleteChatMsgHisResp);
	LOG_PRINT(log_debug, "invoke %s, group[%u] user[%u] dstuid[%u]", CErrMsg::strerror(resp.errinfo().errid()).c_str(), groupid, userid, dstuid);
}
void CChatLogic::getRoomUserStatus(uint32 groupid,uint32 userid,uint32& status)
{
	uint32 datetime = 0;
	char key[32] = {0};
	sprintf(key, "%u:%u", groupid, userid);
	map<string,string> mfield_values;
	m_rmRoomuserSetting.getall(string(key),mfield_values);
	findvalue(status, "forbitchat" ,  mfield_values);
	findvalue(datetime, "datetime" ,  mfield_values);
	if(time(0) - datetime > 24 * 60 * 60)
	{
		mfield_values.clear();
		mfield_values["forbitchat"] = bitTostring(0);
		mfield_values["datetime"] = bitTostring(int(time(0)));
		m_rmRoomuserSetting.insert(string(key),mfield_values);
	}
	// 20171127, wxc: Adds room status check.
	redisOpt *redisOpt = CChatServerApp::getInstance()->m_pRedisMgr->getOne(DB_ROOM);
	if (NULL != redisOpt)
	{
		char roomkey[64] = {0};
		sprintf(roomkey, "%s:%u", KEY_HASH_ROOM_INFO, groupid);
		string roomstatus;
		string statusfield("talk_status");
		if (redisOpt->redis_hget(roomkey, statusfield, roomstatus) < 0)
		{
			LOG_PRINT(log_warning, "failed to hget %s %s", roomkey, statusfield.c_str());
		}
		else
		{
			LOG_PRINT(log_debug, "hget %s %s --> %s", roomkey, statusfield.c_str(), roomstatus.c_str());
			// 0: not forbidden, 1: forbidden. Therefore status is forbidden
			// as long as at least one of them is 1.
			status |= atoi(roomstatus.c_str());
			status &= 0x1; // restricts status value range: 0 or 1, can not be other values.
		}
	}
	LOG_PRINT(log_debug, "mfield_values[\"forbitchat\"] = %s, status = %u", mfield_values["forbitchat"].c_str(), status);
}

bool CChatLogic::updateMongoPPTpic(uint32 groupid,const std::vector<int32> &vMsgId,const string &sMaxPicId)
{
	for (int i=0;i<vMsgId.size();i++)
	{
		ChatMsg_t msg;
		CChatMongoMgr::qryGroupMsg(groupid, vMsgId[i], msg);
		if (!msg.content().empty() && msg.msgtype()==MSGTYPE_PPT_VOICE)
		{
			string sVoiceMediaId;
			Json::Reader reader(Json::Features::strictMode());
			Json::Value root;
			if (reader.parse(msg.content(), root))
			{
				if (root.isMember("picmediaid"))
				{
					root["picmediaid"]=sMaxPicId;
					Json::FastWriter writer;  
					string content=writer.write(root);
					LOG_PRINT(log_info, "update,content: %s", content.c_str());
					CChatMongoMgr::updateMsgContent(vMsgId[i], groupid, content);
				}

			}
			else
			{
				LOG_PRINT(log_error, "MSGTYPE_PPT_VOICE json parse  error, groupid: %u", groupid);
				return false;

			}
			
		}
		
	}

	return true;
}

bool CChatLogic::procOptPPTPic( const std::vector<TPPTPicInfo>& vecPicId,const int32_t optType)
{
	int ret=false;

	LOG_PRINT(log_info, "vecPicId.size=%d,optType=%d", vecPicId.size(),optType);
	do 
	{
		if (optType==1||optType==3)//增加或者修改
		{
			notifyPPTPicChangeMsg(vecPicId,optType);
		}
		else if (optType==2)//删除图片,
		{
			for (int i=0;i<vecPicId.size();i++ )
			{
				const TPPTPicInfo &tTPPTPicInfo=vecPicId[i];
				LOG_PRINT(log_warning, "tTPPTPicInfo.picId=%s,tTPPTPicInfo.groupId=%d", tTPPTPicInfo.picId.c_str(),tTPPTPicInfo.groupId);

				if (!tTPPTPicInfo.picId.empty() && tTPPTPicInfo.groupId)
				{
					std::vector<int32> vMsgId;
					CChatMongoMgr::queryPPTrelationMsg(tTPPTPicInfo.groupId,tTPPTPicInfo.picId,vMsgId);
					LOG_PRINT(log_warning, "vMsgId.size=%d", vMsgId.size());
					if (vMsgId.size())
					{
						string sMaxPicId;
						CDBTools::getPPTMaxPicIdByID(tTPPTPicInfo.groupId,sMaxPicId);
						LOG_PRINT(log_info, "sMaxPicId=%s,old tTPPTPicInfo.picId=%s", sMaxPicId.c_str(),tTPPTPicInfo.picId.c_str());
						if (!sMaxPicId.empty() && sMaxPicId!=tTPPTPicInfo.picId)
						{
							//追加到排序第一张
							CChatMongoMgr::updatePPTrelationMsg(tTPPTPicInfo.groupId,sMaxPicId,vMsgId);

							//更新聊天记录的语音绑定信息
							updateMongoPPTpic(tTPPTPicInfo.groupId,vMsgId,sMaxPicId);

						}
					}

					//删除旧记录
					CChatMongoMgr::deletePPTrelationMsg(tTPPTPicInfo.groupId,tTPPTPicInfo.picId);
					//广播

					notifyPPTPicChangeMsg(vecPicId,optType);
				}
				

				break;//删除操作一次限定只传一张
			}
		}
		ret=true ;
	} while (0);

	return ret;
}


void CChatLogic::notifyPPTPicChangeMsg(const std::vector<TPPTPicInfo>& vecPicId,const int32_t optType)
{
	if (!vecPicId.size())
	{
		return ;
	}
	//string sQiNuiUrl="http://oqt46pvmm.bkt.clouddn.com/";
	string sQiNuiUrl="http://os700oap7.bkt.clouddn.com/";
	string urlFromDb;
	unsigned int groupId=vecPicId[0].groupId;
	//notify user 
	CMDPPTPicChangeMsgNotify notify;
	notify.set_opttype(optType);

	for (unsigned int i=0;i<vecPicId.size();i++)
	{
		PPTCoursePicInfo *tInfo = notify.add_vecpics();
		int dbId = 0;

		urlFromDb.clear();
		CDBSink().queryPptImageInfo(vecPicId[i].picId.c_str(), vecPicId[i].groupId, dbId, urlFromDb);
		tInfo->set_groupid(vecPicId[i].groupId);
		tInfo->set_picid(vecPicId[i].picId);
		tInfo->set_rank(vecPicId[i].rank);
		if (vecPicId[i].picUrl.length() > 0)
		{
			tInfo->set_picurl(vecPicId[i].picUrl);
			LOG_PRINT(log_debug, "picUrl taken from PHP: %s", tInfo->picurl().c_str());
		}
		else if (urlFromDb.length() > 0)
		{
			tInfo->set_picurl(urlFromDb);
			LOG_PRINT(log_debug, "picUrl taken from DB: %s", tInfo->picurl().c_str());
		}
		else
		{
			tInfo->set_picurl(sQiNuiUrl+vecPicId[i].picId);
			LOG_PRINT(log_debug, "picUrl taken from default value: %s", tInfo->picurl().c_str());
		}
		tInfo->set_dbid(dbId);
	}


	int nProtoLen = notify.ByteSize();
	SL_ByteBuffer buff(SIZE_IVM_HEAD_TOTAL + nProtoLen);


	LOG_PRINT(log_debug, "Notify subcmd:%d", Sub_Vchat_PPTPicChangeNotify);
	CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), 0,  Sub_Vchat_PPTPicChangeNotify, SIZE_IVM_HEAD_TOTAL + nProtoLen);
	
	DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
	notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
	LOG_PRINT(log_debug, "Notify group[%u] ,nProtoLen:%d", groupId, nProtoLen);

	CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, groupId);
	if (AppInstance()->m_GateConnMgr.sendMsgToGate(buff.buffer(), buff.buffer_size()))
	{

		LOG_PRINT(log_debug, "[ONLINE]Notify group chat msg of group[%u] .", groupId);
		
	}
	else
	{
		LOG_PRINT(log_warning, "cast group msg error, group[%u] ", groupId);
	}
	
}


bool CChatLogic::procCommentAudit( const TChatAuditMsg &tTChatMsg) 
{
	bool ret=false;
	if (!tTChatMsg.groupId || !tTChatMsg.srcUId || tTChatMsg.content.empty())
	{
		LOG_PRINT(log_error, "procCommentAudit parameter error ");
		return ret;
	}

	do 
	{
		CMDGroupMsgReq req;

		uint32 now = time(NULL);
		req.mutable_msg()->set_msgtime(now);
		req.mutable_msg()->set_msgid(tTChatMsg.msgId);
		req.mutable_msg()->set_content(tTChatMsg.content);
		req.mutable_msg()->set_msgtype((e_MsgType)tTChatMsg.msgType);
		req.mutable_msg()->set_mastermsgid(tTChatMsg.mastermsgId);
		req.mutable_msg()->mutable_srcuser()->set_userid(tTChatMsg.srcUId);

		req.set_groupid(tTChatMsg.groupId);

		//更新用户最后聊天时间
		CChatMongoMgr::updUserGroupChatTime(req.msg().srcuser().userid(), req.groupid(), now);

		//写入mongodb
		bool isPrive= tTChatMsg.msgType==MSGTYPE_UPLOADWALL ? false:true;

		if (tTChatMsg.notifyType==1/*评论或者评论回复审核或者上墙 通过*/)
		{
			if (!CChatMongoMgr::writeGroupMsg(req,isPrive))
			{
				break;
			}
		}
		else//管理后台回复
		{
			if (tTChatMsg.mastermsgId<=0)
			{
				LOG_PRINT(log_error, "mastermsgId error,tTChatMsg.mastermsgId : %d", tTChatMsg.mastermsgId);
				break;
			}
			uint32 msgid;
			if (!CChatMongoMgr::getNextSequence(CChatCommon::getPChatSeqName(req.groupid()), msgid))
			{
				LOG_PRINT(log_error, "getNextSequence error, groupid: %u", req.groupid());
				break;
			}
			
			req.mutable_msg()->set_msgid(msgid);
			if (!CChatMongoMgr::writeGroupMsg(req,true))
			{
				break;
			}

			//更新后台回复记录对应的msgid
			CDBTools::updateCommentAuditMsgId(tTChatMsg.updateId,msgid,tTChatMsg.mastermsgId);

		}
		

		notifyGroupMsg(req, false, false,false,true);

		ret=true;

	} while (0);
	
	return ret;
}