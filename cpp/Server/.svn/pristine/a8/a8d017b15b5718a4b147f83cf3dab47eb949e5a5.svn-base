#ifndef __OPTABLE2_H
#define __OPTABLE2_H
#include <db/sink.h>
#include <db/table.h>
#include <db/table2.h>
#include <com/slist.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define DB_OPTYPE_ADD 1
#define DB_OPTYPE_SUB 2


#define IF_TRUE_APPEND_SQL(cond, buf, apdfmt, value) \
	if (cond) sprintf(buf, "%s AND "apdfmt, buf, value);

#ifdef __cplusplus
extern "C" {
#endif

int exec_query (Sink *sink, const char * query);
int get_affected_rows(Sink * sink);
int get_recordset_number(Sink *sink, const char * query);

//指定userid用户是否存在。0表示用户不存在。1表示用户存在,-1表示查询错误。
int get_userid_records(Sink *sink, unsigned int userid);
//获得版本信息
int get_client_version(Sink *sink, char* szVersion);
//获取靓号记录(通过userid 或 langid)
int get_user_langid_DB(Sink* sink, DDUserLangID_t* ptul, int userid, int langid);
int get_user_platform_info_DB(Sink* sink, int userid, char* nopenid, char* cOpentoken, int platformType);
//获得用户密码信息
int get_user_pwdinfo_DB(Sink* sink, DDUserPwdInfo_t* ppf, int userid);
//更新用户的密码信息
int update_userpwd_DB(Sink* sink, int userid, char* pPwd,int pwdtype,char **strQuery);
//更新用户的基本信息
int update_user_baseinfo_DB(Sink* sink,DDUserBaseInfo_t* ppf, int userid);
//更新用户k币的余额:用户给用户加/减k币.返回用户现有的k,b的余额.type=1加钱,
int update_user_money_DB(Sink *sink, int64 *ble, int64 *giftble, int userid, int64 changemoney, int type);
//更新用户b币的余额:用户给用户加/减b币.返回用户现有的k,b的余额.type=1加钱
int update_user_bmoney_DB(Sink *sink, int64 *ble, int64 *giftble, int userid, int64 changebmoney, int type);
//获取用户k,b币的余额:用于扣费/赠送之前查询用户的余额.返回用户现有的k,b的余额.
int get_user_kmoney_DB(Sink *sink, int64 *ble, int64 *giftble, int64* depositble, int userid);
//更新用户的章信息
int update_user_seal_DB(Sink* sink, int userid, int sealid, int sealtime);
//获得用户被冻结信息
//int get_userfreeze_DB(Sink* sink, DDUserFreezeInfo_t* puf, int userid); 
// 是否在封IP,MAC中
// return (-1)数据库查询错误;(0)在黑名单中;(-2)无黑名单记录
int isin_blackIpMac_DB(Sink* sink, char*pQuery, int qlen, int userid, int vcbid, int scopetype, char* szip, char* szmac, char* szdiannao, DDViolationInfo_t* pInfo);
//获取用户的临时等级
int get_user_templevel(Sink *sink, int nuserid, time_t tnow);
//获取用户的座驾id
int get_usercurcarid(Sink *sink, DDUserCarInfo_t* pusercar, int nuserid);
//获取用户的宝箱数目
int get_userchestnum(Sink *sink, int nuserid);
//更新(修改)用户的宝箱数目
int update_userchestnum(Sink *sink, int nuserid, int change_chestnum);

//获取某个用户对应的上周周星列表(为用户打周星标志用)
int get_lastweekstarview_DB(Sink *sink, int* stararray, int stararraysize, int nuserid);
//获取某个用户对应的风云榜列表(为用户打周星标志用)
int get_topweekstarview_DB(Sink *sink, int* stararray, int stararraysize, int nuserid);
//获取某个用户对应的活动星列表(为用户打上活动标志用)
int get_activitystarview_DB(Sink *sink, int* stararray, int stararraysize, int nuserid);
//根据机器人id找到对应讲师（普通用户）
int get_userid_from_robotid(Sink* sink, int subroomid, int robotid, int *userid, int *mroomid);
//从主房间找到讲师id
int get_userid_from_room(Sink* sink, int roomid, int *userid);

//读入房间组
int get_roomgrouplist_DB(Sink *sink, SList** list, int glevel);
//读入房间组人数
int get_vcbgroupusernumlist_DB(Sink* sink, SList** list);
//获取房间的执行命令(目前限于加载删除机器人)
int get_vcbcommands(Sink *sink, SList** list);

//读入muc(可用) livetype:1-视频房间;2-文字房间
int get_vcbfullinfolist_DB(Sink* sink, SList** list,int livetype);   //核心代码
int get_vcballroominfolist_DB(Sink* sink, SList** list,int livetype);
int get_one_vcbfullinfo_DB(Sink* sink, SList** list, int roomid, int livetype);   //核心代码
int update_room_popularity(Sink* sink, uint32_t vcbid, uint64_t accesstimes);
int update_room_activetime(Sink* sink, uint32_t roomid, uint64_t tActive);
//读入删除的房间列表 livetype:1-视频房间;2-文字房间
int get_delvcbIdlist_DB(Sink* sink, int* vcbIds, int* arrsize,int livetype);

//读入房间的管理id列表
int get_vcbmanagerlist_DB(Sink* sink, int* pmanagers, int* arrsize, int vcbid);

//重新读入房间的基本信息 livetype:1-视频房间;2-文字房间
int get_vcbbaseinfolist_DB(Sink* sink, SList** list,int livetype);
//写入房间基本信息(设置基本信息)
int write_roombaseinfo_DB(Sink* sink, int vcbid, DDRoomBaseInfo_t* pInfo);
//写入房间管理状态信息
int write_roomoptstat_DB(Sink* sink, int vcbid, int nopstat);

//重新读入房间的媒体服务器配置信息
int update_vcbmedia_DB(Sink* sink,int vcbid,char* szmediasvr); //gch++
int get_vcbmedialist_DB(Sink* sink,SList** list);
int get_vcbmedialist_DB_filter_areaid(Sink* sink,SList** list);
//根据区域id获取媒体服务器ip地址
int get_vcbmedia_addr_byareaid(Sink* sink, int vcbid, const char* areadid, char * mediaip);
int get_vcbareaid_from_UserInfo(Sink* sink, int userid, char* areaid);


int get_AdKeywordList_DB(Sink* sink, SList** list, int startline, int linenum);
int exist_AdKeyword_DB(Sink* sink, char	*pKeyword);
int insert_AdKeyword_DB(Sink* sink, DDAdKeywordInfo_t *pKeyword);
int del_AdKeyword_DB(Sink* sink, DDAdKeywordInfo_t *pKeyword);
//增加讲师打分日记
int write_teacherscorelog_DB(Sink* sink, CMDTeacherScoreRecord_t2* teacherlog);
//判断用户是否在两个小时内重复给一个讲师打分
int check_teacher_score_DB(Sink* sink, int teacherid,int userid,char* logtime);
int get_mainroom_zhuanbo_DB(Sink* sink, SList** list, int roomid,int userid);
int get_zhuanbo_mainroom_DB(Sink* sink, int* list, int roomid,int userid);
int select_user_score_DB(Sink* sink, int teacherid,int userid,int *score);
int select_user_scorelist_DB(Sink* sink,int roomid,int teacherid, SList** list);


//读入房间的中转机器人配置
int get_add_zhuanbo_robots_DB(Sink* sink, SList** list);
int get_del_zhuanbo_robots_DB(Sink* sink, SList** list);

//重新读入房间的id信息
int get_vcbidlist_DB(Sink* sink, int* pvcbids, int* arrsize, char* szfilter);

int get_flygiftlst_DB(Sink* sink, SList** list);     //获取大礼物记录列表
int write_flygiftlog_DB(Sink* sink, DDFlyGiftInfo_t* tvf);   //写入大礼物信息

int get_privilegelist_DB(Sink* sink, SList** list);    //获取权限列表
int get_privilegeupdatestatus_DB(Sink* sink);   //检查权限是否要更新

//TODO: int get_giftjackpot(Sink* sink, int index, int *jiner);
int get_giftjackpot(Sink* sink,int jackpot_type, int *jiner);    //获得幸运礼物奖池(TODO:某个奖池)金额
int update_giftjackpot(Sink* sink,int giftid, int optype, int64 money, int64 *pcount);  //optype=1:add =2 sub   //更新幸运礼物奖池金额
int get_giftlotterypro(Sink* sink, DDGiftLotteryPro_t* tpro, int giftid);    //获取某个幸运礼物的中奖率
int add_giftlotterywinlog(Sink* sink,int userid, int giftid, int64 giftprice, int wincount);   //写入幸运礼物中奖信息
int update_lotterypool(Sink *sink, int64 in_money, int64* out_money);    //更新幸运宝箱奖池金额
//int get_xingyunrenpro(Sink* sink, DDXingyunrenPro_t* xpro, int userid, int giftid);

int get_isroomquzhang_DB(Sink* sink, int userid, int vcbid);   //确定是不是区长
int get_isroommanager_DB(Sink* sink, int userid, int vcbid);   //确定是不是管理
int get_isroomagent_DB(Sink* sink, int userid, int vcbid);     //确定是不是代理
int add_roommanager_DB(Sink* sink, int userid, int vcbid,int mgrtype);    //增加房间管理
int del_roommanager_DB(Sink* sink, int userid, int vcbid,int mgrtype);    //删除房间管理


int write_userloginlog_DB(Sink* sink, DDUserLoginLog_t* loginlog);  //增加登陆日记

int is_collectroom_DB(Sink* sink, int userid, int vcbid);  //是否是收藏房间
int write_collectroom_DB(Sink* sink, int userid, int vcbid, int action);   //写入收藏信息
int get_room_collect_num(Sink* sink, uint32_t vcbid, uint32_t *pNum);
int write_addlastgoroom_DB(Sink* sink, int userid, int vcbid, int type);  //写入最近去过房间信息

int get_syscasts_DB(Sink* sink, SList** list,int casttype, int recid);   //获得系统广播消息,casttype=1:一次性消息

int insert_tbfireworkslog_DB(Sink* sink,DDFireworksLogTab_t* pTab);
int insert_sendseallog_DB(Sink* sink,DDSendSealLogTab_t* pTab);
int insert_userloginroomlog_DB(Sink* sink,DDUserLoginRoomLogTab_t* pTab);
int insert_useronmictimelog_DB(Sink* sink,DDUserOnMicLogTab_t* pTab);
int get_vcbgateaddr_DB(Sink* sink,int vcbid,char* pAddr);
int write_useropnknblog_DB(Sink* sink, DDUsernknbOplog_t* pkboplog);
int update_roommgronlinetime(Sink* sink, DDUpdateMgrOnlineTime_t* pdata);
int get_single_siegelog_DB(Sink* sink, struct tbviewsiegelog* tvs);  //获得单个攻城纪录(最高纪录)
int insert_useropenchestlog_DB(Sink* sink, int userid, int invcbid, int ncost, int winid, int winval, char* sztime);  //写入用户开宝箱记录
int get_useropenchest_stainfo_DB(Sink* sink, int userid, DDUserOpenChestStatInfo_t* pDDinfo, char * sztime);  //获取用户的开宝箱统计
int get_user_secure_info_DB(Sink* sink, CMDSecureInfo_t* pTab, int userid);//仅仅获取用户的邮箱、QQ、电话号码、已提醒次数，飘窗
int get_user_exit_message(Sink* sink, CMDSecureInfo_t* pTab, int* hit_times, int userid);//用户退出软件时候的给出提示
int get_teacher_all_score_DB(Sink* sink, SList** list, int teatcher_userid);
int get_teacher_gift_list_DB(Sink* sink, SList** list, int roomid,int teacherid);
int get_team_top_money(Sink* sink, Result *res, SList** list);
unsigned int func_get_visitor_unique_id(Sink * sink, unsigned int area);
int get_user_token_DB(Sink* sink, int userid, char* token, char* codetime);
int update_user_token_DB(Sink* sink, int userid, const char* token, const char* validtime);

//////////////////////////////////////////////////////////////////////////
//增加本月礼物
int func_addmonthgift_DB(Sink* sink, int userid, int giftid, int giftcount);
//获取本月礼物数目
int func_getmonthgift_DB(Sink* sink, int userid, int giftid);
//赠送礼物
int func_tradegift_DB(Sink* sink, int src_id, int dst_id, 
					  int gift_id, int gift_count, int gift_type, 
					  int64 total_out, int64 total_in, int trade_type, int room_id, 
					  int64* src_now_nk, int64* src_now_nb, 
					  int64* dst_now_nk, int64* dst_now_nb,
					  int* src_add_nchestnum,
					  int* src_now_nchestnum); 
int function_tradegift_yanhua_DB(Sink* sink, int src_id, int dst_id, 
								 int gift_id, int gift_count, int gift_type, 
								 int to_type, int to_vcbid,
								 int64 total_out, int64 total_in, int room_id, 
								 int64* src_now_nk, int64* src_now_nb,
								 int64* dst_now_nk, int64* dst_now_nb);

//攻城类礼物更新
int func_tradesiege_DB(Sink* sink, int vcbid, int srcid, int toid, int giftid, int giftcount);

//检查消费等级
int func_getusercaifugrade_DB(Sink* sink, int userid);
int func_getusercostlevel_DB(Sink* sink, int userid);
int func_getuserlastmonthcostlevel_DB(Sink* sink, int userid);
int func_getuserthismonthcostlevel_DB(Sink* sink, int userid);
int func_getuserthismonthcostgrade_DB(Sink* sink, int userid);
int func_isquzhang_DB(Sink* sink,int vcbid,int userid);

//更新用户月消费记录
int func_updateusermonthcost_DB(Sink* sink, int userid, int64 cost_nk);

//银行存取款，转账，积分兑换
int func_moneyandpointop(Sink* sink,int64* src_nk,int64* src_nb,int64* srcnkdeposit,
						 int64* dst_nk, int64* dst_nb, int64* dst_nkdeposit,
						 int src_userid, int dst_userid, int op_type, int64 op_money1,int64 op_money2);

int insert_gift_ticket(Sink* sink, int userid, int analystid, const char* analystname, int votecount);

//////////////////////////////////////////////////////////////////////////////////
//登入直播间记录
int insert_textlivelog_DB(Sink* sink, CMDTextRoomUserInfo_t *userinfo);
//获得讲师信息
int get_teacher_info_DB(Sink* sink, CMDTextRoomTeacherNoty_t2* ptf, int roomid);
//今日人气(type=1)/直播人气(type=2)
int64 get_textlivetoday_DB(Sink* sink, int roomid ,int type);
//粉丝数量
int64 get_textlivefans_DB(Sink* sink, int teacherid);
//直播点赞数量
int64 get_textlivemessagezans_DB(Sink* sink, int teacherid);
//观点点赞数量
int64 get_textliveviewzans_DB(Sink* sink, int teacherid);
//直播历史数量
int64 get_textlivehistorys_DB(Sink* sink, int teacherid);
//直播记录
int get_TextRoomLive_list_DB(Sink* sink, long fromDate,int offset,unsigned char bHistory/*获取当天历史直播记录入口填1*/,SList** list, CMDTextRoomLiveListReq_t2 *listreq);
//直播重点/明日预测(类型：1-文字直播；2-直播重点；3-明日预测（已关注的用户可查看）；4-观点；)
int get_TextRoomPoint_list_DB(Sink* sink, SList** list, CMDTextRoomLiveListReq_t2 *listreq);
//文字直播记录
int64 insert_textlivemessage_DB(Sink* sink, CMDTextRoomLiveMessageReq_t2 *messageinfo);
//新增讲师观点类型分类
long int insert_viewtype_DB(Sink* sink, CMDTextRoomViewTypeReq_t2 *info);
//修改讲师观点类型分类
int update_viewtype_DB(Sink* sink, CMDTextRoomViewTypeReq_t2 *info);
//删除讲师观点类型分类
int delete_viewtype_DB(Sink* sink, CMDTextRoomViewTypeReq_t2 *info);
//获取用户信息
int get_userinfo_DB(Sink* sink, CMDTextRoomUserInfo_t2 *info);
//获取互动回复
int get_textlivechataction_DB(Sink* sink, SList** list, CMDHallMessageReq_t2 *info);
//更新未读标识
int update_textlivechatread_DB(Sink* sink, int64 messageid);
//是否讲师
int get_teacherflag_DB(Sink* sink,int userid);
//互动回复未读记录数
int get_interactunreads_DB(Sink* sink,int userid);
//观点回复未读记录数
int get_viewunreads_DB(Sink* sink,int userid);
//提问未读记录数
int get_questionunreads_DB(Sink* sink,int teacherid);
//回答未读记录数
int get_answerunreads_DB(Sink* sink,int userid);
//系统回复未读记录数
//回答提问
int64 insert_textliveanswer_DB(Sink* sink, CMDViewAnswerReq_t2 *info);

//获取讲师所有的个人秘籍数
int get_secretsbyteacher_info_DB(Sink* sink, int teacherid);
//获取用户已购买的对应讲师的个人秘籍数
int get_secretsbyuser_info_DB(Sink* sink, int teacherid, int userid);
//获取讲师所有的个人秘籍
int get_TextRoomSecretsAll_list_DB(Sink* sink, SList** list,CMDTextRoomLiveListReq_t2 *listreq);
//获取用户已购买的对应讲师的个人秘籍ID
int get_TextRoomSecretsUser_list_DB(Sink* sink, SList** list,CMDTextRoomLiveListReq_t2 *listreq);
//获取用户已购买的对应讲师的个人秘籍
int get_TextRoomSecretsOwn_list_DB(Sink* sink, SList** list,CMDTextRoomLiveListReq_t2 *listreq);
//判断用户是否已购买对应的秘籍
int check_TextRoomSecretsBuy_DB(Sink* sink,int messageid, int teacherid, int userid);
//更新订阅人数
int update_TextRoomSecretsBuy_DB(Sink* sink,int messageid, int teacherid);

//对观点点赞
int view_dianzan_DB(Sink* sink,long long viewid);
//对观点献花
int view_sendflower_DB(Sink* sink,long long viewid,int count);
//对观点评论
int view_comment_DB(Sink* sink,long long viewid,int fromid,int toid,long long reqtime,char* ccomments,long long srcinteractid,int reqcommentstype,long long* commentid);
//返回直播历史列表
int func_getlivehistorylist_DB(Sink* sink,long long datetime/*从哪个年月日开始取*/,int fromIndex,int bInc/*是否升序*/,int count/*取多少条记录*/,int teacherid,int vcbid,SList** list);
//返回某天的直播列表
//to do
//用户提问
int func_userquestion_DB(Sink* sink,int userid,int teacherid,long long reqtime,char* question,char* stokeid,unsigned char questiontype,long long* questionid);
//返回点赞数
int func_getviewdianzan_DB(Sink* sink,long long viewid);
//返回献花总数
int func_getviewflower_DB(Sink* sink,long long viewid);
//修改观点
int view_modify_view_DB(Sink* sink,long long* viewid/*传0代表是新增的观点*/,int viewtypeid,int teacherid,long long datetime,char* title,char* content);

/*
 * Comments:Get text room live viewpoint type info from db
 * Param IN:sink
 * Param IN:teacherID
 * Param IN/OUT:viewpoint type info
 * Return:int, 0 means success, others mean failed.
 */
int get_textroomlivegroup_DB(Sink* sink, SList** list, unsigned int teacherID);

/*
 * Comments:Get text room live viewpoint type info from db(partial show content)该查询即支持跳页，也支持滚动查询
 * Param IN:sink, db connection
 * Param IN:int, teacherID
 * Param IN:int, viewType
 * Param IN:int, startPos,查询结果的编号（从0开始编号），表示该编号开始从查询结果里取数据
 * Param IN:int, messageid,观点列表的观点ID
 * Param IN:int, lstCount,一次查询的观点个数
 * Param IN/OUT:list, viewpoint list info.
 * Return:int, -1 means failed, others mean row count.
 */
int get_textroomViewPointLst_DB(Sink* sink, SList** list, unsigned int teacherID, int viewType, long long startPos, long long messageid, int lstCount);

/*
 * Comments:like on text live
 * Param IN:sink, db connection
 * Param IN:int, messageID, text live ID
 * Return:int, -1 means failed, 0 means success.
 */
int update_likesOnTextLive_DB(Sink* sink, long long messageID);

/*
 * Comments:add one fans on teacher and update teacher fans count number.
 * Param IN:sink, db connection
 * Param IN:teacherID,int
 * Param IN:fansID,int
 * Return:int, -1 means failed, 0 means success.
 */
int insert_teacherfans_DB(Sink* sink, unsigned int teacherID, unsigned int fansID);

/*
 * Comments:Get fans count of teacherID from db
 * Param IN:sink
 * Param IN:teacherID
 * Return:int, 0 means result, -1 mean failed.
 */
long get_teacherfansCount_DB(Sink* sink, unsigned int teacherID);

/*
 * Comments:check if the user is the fans of teacher
 * Param IN:sink, db connection
 * Param IN:teacherID,int
 * Param IN:fansID,int
 * Return:int, -1 means failed, 0 means not exist, 1 means exist.
 */
int check_teacherfans_exist_DB(Sink* sink, unsigned int teacherID, unsigned int fansID);

/*
 * Comments:delete the fan of teacher
 * Param IN:sink, db connection
 * Param IN:teacherID,int
 * Param IN:fansID,int
 * Return:int, -1 means failed, 0 means success.
 */
int delete_teacherfans_DB(Sink* sink, unsigned int teacherID, unsigned int fansID);

//Return:int, -1 means failed, not -1 means query success and means row number in db.该查询即支持跳页，也支持滚动查询.
int get_fansInfoByTeacherID_DB(Sink* sink, SList** list, unsigned int teacherid, unsigned int startPos, long long messageid, unsigned int count);

//Return:int, -1 means failed, not -1 means query success and means row number in db.该查询即支持跳页，也支持滚动查询.
int get_teacherInfoByfansID_DB(Sink* sink, SList** records, unsigned int userid, unsigned int startPos, long long messageid, unsigned int count);

/*
 * Comments:获得讲师信息
 * Param IN:sink
 * Param IN:int, startPos,查询结果的编号（从0开始编号），表示该编号开始从查询结果里取数据
 * Param IN:int, messageid,观点列表的观点ID
 * Param IN:int, lstCount,一次查询的观点个数
 * Param IN/OUT:records, db query information
 * Return:int, 0 means result, -1 mean failed, others mean row number
 */
int get_teacher_info_list_DB(Sink* sink, SList** records, unsigned int startPos, long long messageid, unsigned int count);

/*
 * Comments:获得明日预测
 * Param IN:sink
 * Param IN:int, startPos,查询结果的编号（从0开始编号），表示该编号开始从查询结果里取数据
 * Param IN:int, messageid,明日预测的ID
 * Param IN:int, lstCount,一次查询的个数
 * Param IN/OUT:records, db query information
 * Return:int, 0 means result, -1 mean failed, others mean row number
 */
int get_forecast_info_list_DB(Sink* sink, SList** records, unsigned int userid, unsigned int startPos, long long messageid, unsigned int count);

//run all type SQL return state
int get_DB(Sink *sink, Result *res, const char *query, int *row);

//获取用户的未回答列表
//获取用户的已回答列表
//获取讲师的未回答列表
//获取讲师的已回答列表
int func_getquestionlist_DB(Sink* sink,int userid,unsigned char bteacher, unsigned char bunread,long long messageid,int startpos,int count,SList** list);
//查看用户有没有未读信息
//有未读就返回1，否则返回0
int func_getunreaddot_DB(Sink* sink,int userid);
//获取体验号
int get_supaccount_DB(Sink* sink, SList** list,char* date,int count);
//更新vip等级
int update_viplevel_DB(Sink* sink,int userid,int viplevel);
//查询系统公告
int func_getsyscast_DB(Sink* sink,SList** list);

//查询礼包的问股/献花免费次数
int func_getFreeTimes_DB(Sink* sink,int classid);
//获取师徒列表
int get_shitulist_DB(Sink* sink,unsigned char bTeacherTag,int userid,SList** list);
//徒弟表
int get_IsStudent_DB(Sink* sink,int teacherid,int userid);
//拜师
int insert_shitulist_DB(Sink* sink,int vcbid,int userid,int teacherid,long long startTime,int effectTime);

//query user notify config in db
int load_usernotify_config_DB(Sink * sink, SList ** list);
//insert user notify config in db
int insert_usernotify_config_DB(Sink * sink, DDLUserNotify * record);
//update user notify config in db
void update_usernotify_config_DB(Sink * sink, DDLUserNotify * record);
//update user notify config state in db.
void update_usernotify_state_DB(Sink * sink, DDLUserNotify * record);
//check the user config if exist.
int check_userconfig_exist_DB(Sink * sink, DDLUserNotify * record);

//查询数据库里极光推送的配置
int load_jpush_config_db(Sink * sink, SList ** list);
//更新数据库里极光推送配置的validtime
int update_jpush_config_db(Sink * sink, DDLJPushConfig * record);
//获取订阅房间用户
int get_tb_uservcbfavorite_DB(Sink *sink, SList** list, int *size, int nroomid);


//获取徒弟总数
int get_studentCount_DB(Sink* sink,int teacherid);
//获取特权列表
int get_PackagePrivilege_DB(Sink* sink, SList** list,int packageNum);

//获取讲师订阅列表
int get_tb_teachersubscription_DB(Sink *sink, SList** list, int *size, int nteacherid);

//课程订阅
int func_teacherSubscript_DB(Sink *sink,int userid,int teacherid,unsigned char bSub);
int func_getsubscriptstate_DB(Sink *sink,int userid,int teacherid);

int func_IsTeacherOnLive_DB(Sink *sink,int teacherid);

int get_zhuanborobot_DB(Sink *sink, SList** list);

int insert_vistorlogon_DB(Sink *sink, VistorLogon_t *vsl); 
int update_vistorlogon_DB(Sink *sink, VistorLogon_t *vsl);
int get_vistorlogon_DB(Sink *sink, char *_uuid, SList** list);
//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif
