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

//ָ��userid�û��Ƿ���ڡ�0��ʾ�û������ڡ�1��ʾ�û�����,-1��ʾ��ѯ����
int get_userid_records(Sink *sink, unsigned int userid);
//��ð汾��Ϣ
int get_client_version(Sink *sink, char* szVersion);
//��ȡ���ż�¼(ͨ��userid �� langid)
int get_user_langid_DB(Sink* sink, DDUserLangID_t* ptul, int userid, int langid);
int get_user_platform_info_DB(Sink* sink, int userid, char* nopenid, char* cOpentoken, int platformType);
//����û�������Ϣ
int get_user_pwdinfo_DB(Sink* sink, DDUserPwdInfo_t* ppf, int userid);
//�����û���������Ϣ
int update_userpwd_DB(Sink* sink, int userid, char* pPwd,int pwdtype,char **strQuery);
//�����û��Ļ�����Ϣ
int update_user_baseinfo_DB(Sink* sink,DDUserBaseInfo_t* ppf, int userid);
//�����û�k�ҵ����:�û����û���/��k��.�����û����е�k,b�����.type=1��Ǯ,
int update_user_money_DB(Sink *sink, int64 *ble, int64 *giftble, int userid, int64 changemoney, int type);
//�����û�b�ҵ����:�û����û���/��b��.�����û����е�k,b�����.type=1��Ǯ
int update_user_bmoney_DB(Sink *sink, int64 *ble, int64 *giftble, int userid, int64 changebmoney, int type);
//��ȡ�û�k,b�ҵ����:���ڿ۷�/����֮ǰ��ѯ�û������.�����û����е�k,b�����.
int get_user_kmoney_DB(Sink *sink, int64 *ble, int64 *giftble, int64* depositble, int userid);
//�����û�������Ϣ
int update_user_seal_DB(Sink* sink, int userid, int sealid, int sealtime);
//����û���������Ϣ
//int get_userfreeze_DB(Sink* sink, DDUserFreezeInfo_t* puf, int userid); 
// �Ƿ��ڷ�IP,MAC��
// return (-1)���ݿ��ѯ����;(0)�ں�������;(-2)�޺�������¼
int isin_blackIpMac_DB(Sink* sink, char*pQuery, int qlen, int userid, int vcbid, int scopetype, char* szip, char* szmac, char* szdiannao, DDViolationInfo_t* pInfo);
//��ȡ�û�����ʱ�ȼ�
int get_user_templevel(Sink *sink, int nuserid, time_t tnow);
//��ȡ�û�������id
int get_usercurcarid(Sink *sink, DDUserCarInfo_t* pusercar, int nuserid);
//��ȡ�û��ı�����Ŀ
int get_userchestnum(Sink *sink, int nuserid);
//����(�޸�)�û��ı�����Ŀ
int update_userchestnum(Sink *sink, int nuserid, int change_chestnum);

//��ȡĳ���û���Ӧ�����������б�(Ϊ�û������Ǳ�־��)
int get_lastweekstarview_DB(Sink *sink, int* stararray, int stararraysize, int nuserid);
//��ȡĳ���û���Ӧ�ķ��ư��б�(Ϊ�û������Ǳ�־��)
int get_topweekstarview_DB(Sink *sink, int* stararray, int stararraysize, int nuserid);
//��ȡĳ���û���Ӧ�Ļ���б�(Ϊ�û����ϻ��־��)
int get_activitystarview_DB(Sink *sink, int* stararray, int stararraysize, int nuserid);
//���ݻ�����id�ҵ���Ӧ��ʦ����ͨ�û���
int get_userid_from_robotid(Sink* sink, int subroomid, int robotid, int *userid, int *mroomid);
//���������ҵ���ʦid
int get_userid_from_room(Sink* sink, int roomid, int *userid);

//���뷿����
int get_roomgrouplist_DB(Sink *sink, SList** list, int glevel);
//���뷿��������
int get_vcbgroupusernumlist_DB(Sink* sink, SList** list);
//��ȡ�����ִ������(Ŀǰ���ڼ���ɾ��������)
int get_vcbcommands(Sink *sink, SList** list);

//����muc(����) livetype:1-��Ƶ����;2-���ַ���
int get_vcbfullinfolist_DB(Sink* sink, SList** list,int livetype);   //���Ĵ���
int get_vcballroominfolist_DB(Sink* sink, SList** list,int livetype);
int get_one_vcbfullinfo_DB(Sink* sink, SList** list, int roomid, int livetype);   //���Ĵ���
int update_room_popularity(Sink* sink, uint32_t vcbid, uint64_t accesstimes);
int update_room_activetime(Sink* sink, uint32_t roomid, uint64_t tActive);
//����ɾ���ķ����б� livetype:1-��Ƶ����;2-���ַ���
int get_delvcbIdlist_DB(Sink* sink, int* vcbIds, int* arrsize,int livetype);

//���뷿��Ĺ���id�б�
int get_vcbmanagerlist_DB(Sink* sink, int* pmanagers, int* arrsize, int vcbid);

//���¶��뷿��Ļ�����Ϣ livetype:1-��Ƶ����;2-���ַ���
int get_vcbbaseinfolist_DB(Sink* sink, SList** list,int livetype);
//д�뷿�������Ϣ(���û�����Ϣ)
int write_roombaseinfo_DB(Sink* sink, int vcbid, DDRoomBaseInfo_t* pInfo);
//д�뷿�����״̬��Ϣ
int write_roomoptstat_DB(Sink* sink, int vcbid, int nopstat);

//���¶��뷿���ý�������������Ϣ
int update_vcbmedia_DB(Sink* sink,int vcbid,char* szmediasvr); //gch++
int get_vcbmedialist_DB(Sink* sink,SList** list);
int get_vcbmedialist_DB_filter_areaid(Sink* sink,SList** list);
//��������id��ȡý�������ip��ַ
int get_vcbmedia_addr_byareaid(Sink* sink, int vcbid, const char* areadid, char * mediaip);
int get_vcbareaid_from_UserInfo(Sink* sink, int userid, char* areaid);


int get_AdKeywordList_DB(Sink* sink, SList** list, int startline, int linenum);
int exist_AdKeyword_DB(Sink* sink, char	*pKeyword);
int insert_AdKeyword_DB(Sink* sink, DDAdKeywordInfo_t *pKeyword);
int del_AdKeyword_DB(Sink* sink, DDAdKeywordInfo_t *pKeyword);
//���ӽ�ʦ����ռ�
int write_teacherscorelog_DB(Sink* sink, CMDTeacherScoreRecord_t2* teacherlog);
//�ж��û��Ƿ�������Сʱ���ظ���һ����ʦ���
int check_teacher_score_DB(Sink* sink, int teacherid,int userid,char* logtime);
int get_mainroom_zhuanbo_DB(Sink* sink, SList** list, int roomid,int userid);
int get_zhuanbo_mainroom_DB(Sink* sink, int* list, int roomid,int userid);
int select_user_score_DB(Sink* sink, int teacherid,int userid,int *score);
int select_user_scorelist_DB(Sink* sink,int roomid,int teacherid, SList** list);


//���뷿�����ת����������
int get_add_zhuanbo_robots_DB(Sink* sink, SList** list);
int get_del_zhuanbo_robots_DB(Sink* sink, SList** list);

//���¶��뷿���id��Ϣ
int get_vcbidlist_DB(Sink* sink, int* pvcbids, int* arrsize, char* szfilter);

int get_flygiftlst_DB(Sink* sink, SList** list);     //��ȡ�������¼�б�
int write_flygiftlog_DB(Sink* sink, DDFlyGiftInfo_t* tvf);   //д���������Ϣ

int get_privilegelist_DB(Sink* sink, SList** list);    //��ȡȨ���б�
int get_privilegeupdatestatus_DB(Sink* sink);   //���Ȩ���Ƿ�Ҫ����

//TODO: int get_giftjackpot(Sink* sink, int index, int *jiner);
int get_giftjackpot(Sink* sink,int jackpot_type, int *jiner);    //����������ｱ��(TODO:ĳ������)���
int update_giftjackpot(Sink* sink,int giftid, int optype, int64 money, int64 *pcount);  //optype=1:add =2 sub   //�����������ｱ�ؽ��
int get_giftlotterypro(Sink* sink, DDGiftLotteryPro_t* tpro, int giftid);    //��ȡĳ������������н���
int add_giftlotterywinlog(Sink* sink,int userid, int giftid, int64 giftprice, int wincount);   //д�����������н���Ϣ
int update_lotterypool(Sink *sink, int64 in_money, int64* out_money);    //�������˱��佱�ؽ��
//int get_xingyunrenpro(Sink* sink, DDXingyunrenPro_t* xpro, int userid, int giftid);

int get_isroomquzhang_DB(Sink* sink, int userid, int vcbid);   //ȷ���ǲ�������
int get_isroommanager_DB(Sink* sink, int userid, int vcbid);   //ȷ���ǲ��ǹ���
int get_isroomagent_DB(Sink* sink, int userid, int vcbid);     //ȷ���ǲ��Ǵ���
int add_roommanager_DB(Sink* sink, int userid, int vcbid,int mgrtype);    //���ӷ������
int del_roommanager_DB(Sink* sink, int userid, int vcbid,int mgrtype);    //ɾ���������


int write_userloginlog_DB(Sink* sink, DDUserLoginLog_t* loginlog);  //���ӵ�½�ռ�

int is_collectroom_DB(Sink* sink, int userid, int vcbid);  //�Ƿ����ղط���
int write_collectroom_DB(Sink* sink, int userid, int vcbid, int action);   //д���ղ���Ϣ
int get_room_collect_num(Sink* sink, uint32_t vcbid, uint32_t *pNum);
int write_addlastgoroom_DB(Sink* sink, int userid, int vcbid, int type);  //д�����ȥ��������Ϣ

int get_syscasts_DB(Sink* sink, SList** list,int casttype, int recid);   //���ϵͳ�㲥��Ϣ,casttype=1:һ������Ϣ

int insert_tbfireworkslog_DB(Sink* sink,DDFireworksLogTab_t* pTab);
int insert_sendseallog_DB(Sink* sink,DDSendSealLogTab_t* pTab);
int insert_userloginroomlog_DB(Sink* sink,DDUserLoginRoomLogTab_t* pTab);
int insert_useronmictimelog_DB(Sink* sink,DDUserOnMicLogTab_t* pTab);
int get_vcbgateaddr_DB(Sink* sink,int vcbid,char* pAddr);
int write_useropnknblog_DB(Sink* sink, DDUsernknbOplog_t* pkboplog);
int update_roommgronlinetime(Sink* sink, DDUpdateMgrOnlineTime_t* pdata);
int get_single_siegelog_DB(Sink* sink, struct tbviewsiegelog* tvs);  //��õ������Ǽ�¼(��߼�¼)
int insert_useropenchestlog_DB(Sink* sink, int userid, int invcbid, int ncost, int winid, int winval, char* sztime);  //д���û��������¼
int get_useropenchest_stainfo_DB(Sink* sink, int userid, DDUserOpenChestStatInfo_t* pDDinfo, char * sztime);  //��ȡ�û��Ŀ�����ͳ��
int get_user_secure_info_DB(Sink* sink, CMDSecureInfo_t* pTab, int userid);//������ȡ�û������䡢QQ���绰���롢�����Ѵ�����Ʈ��
int get_user_exit_message(Sink* sink, CMDSecureInfo_t* pTab, int* hit_times, int userid);//�û��˳����ʱ��ĸ�����ʾ
int get_teacher_all_score_DB(Sink* sink, SList** list, int teatcher_userid);
int get_teacher_gift_list_DB(Sink* sink, SList** list, int roomid,int teacherid);
int get_team_top_money(Sink* sink, Result *res, SList** list);
unsigned int func_get_visitor_unique_id(Sink * sink, unsigned int area);
int get_user_token_DB(Sink* sink, int userid, char* token, char* codetime);
int update_user_token_DB(Sink* sink, int userid, const char* token, const char* validtime);

//////////////////////////////////////////////////////////////////////////
//���ӱ�������
int func_addmonthgift_DB(Sink* sink, int userid, int giftid, int giftcount);
//��ȡ����������Ŀ
int func_getmonthgift_DB(Sink* sink, int userid, int giftid);
//��������
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

//�������������
int func_tradesiege_DB(Sink* sink, int vcbid, int srcid, int toid, int giftid, int giftcount);

//������ѵȼ�
int func_getusercaifugrade_DB(Sink* sink, int userid);
int func_getusercostlevel_DB(Sink* sink, int userid);
int func_getuserlastmonthcostlevel_DB(Sink* sink, int userid);
int func_getuserthismonthcostlevel_DB(Sink* sink, int userid);
int func_getuserthismonthcostgrade_DB(Sink* sink, int userid);
int func_isquzhang_DB(Sink* sink,int vcbid,int userid);

//�����û������Ѽ�¼
int func_updateusermonthcost_DB(Sink* sink, int userid, int64 cost_nk);

//���д�ȡ�ת�ˣ����ֶһ�
int func_moneyandpointop(Sink* sink,int64* src_nk,int64* src_nb,int64* srcnkdeposit,
						 int64* dst_nk, int64* dst_nb, int64* dst_nkdeposit,
						 int src_userid, int dst_userid, int op_type, int64 op_money1,int64 op_money2);

int insert_gift_ticket(Sink* sink, int userid, int analystid, const char* analystname, int votecount);

//////////////////////////////////////////////////////////////////////////////////
//����ֱ�����¼
int insert_textlivelog_DB(Sink* sink, CMDTextRoomUserInfo_t *userinfo);
//��ý�ʦ��Ϣ
int get_teacher_info_DB(Sink* sink, CMDTextRoomTeacherNoty_t2* ptf, int roomid);
//��������(type=1)/ֱ������(type=2)
int64 get_textlivetoday_DB(Sink* sink, int roomid ,int type);
//��˿����
int64 get_textlivefans_DB(Sink* sink, int teacherid);
//ֱ����������
int64 get_textlivemessagezans_DB(Sink* sink, int teacherid);
//�۵��������
int64 get_textliveviewzans_DB(Sink* sink, int teacherid);
//ֱ����ʷ����
int64 get_textlivehistorys_DB(Sink* sink, int teacherid);
//ֱ����¼
int get_TextRoomLive_list_DB(Sink* sink, long fromDate,int offset,unsigned char bHistory/*��ȡ������ʷֱ����¼�����1*/,SList** list, CMDTextRoomLiveListReq_t2 *listreq);
//ֱ���ص�/����Ԥ��(���ͣ�1-����ֱ����2-ֱ���ص㣻3-����Ԥ�⣨�ѹ�ע���û��ɲ鿴����4-�۵㣻)
int get_TextRoomPoint_list_DB(Sink* sink, SList** list, CMDTextRoomLiveListReq_t2 *listreq);
//����ֱ����¼
int64 insert_textlivemessage_DB(Sink* sink, CMDTextRoomLiveMessageReq_t2 *messageinfo);
//������ʦ�۵����ͷ���
long int insert_viewtype_DB(Sink* sink, CMDTextRoomViewTypeReq_t2 *info);
//�޸Ľ�ʦ�۵����ͷ���
int update_viewtype_DB(Sink* sink, CMDTextRoomViewTypeReq_t2 *info);
//ɾ����ʦ�۵����ͷ���
int delete_viewtype_DB(Sink* sink, CMDTextRoomViewTypeReq_t2 *info);
//��ȡ�û���Ϣ
int get_userinfo_DB(Sink* sink, CMDTextRoomUserInfo_t2 *info);
//��ȡ�����ظ�
int get_textlivechataction_DB(Sink* sink, SList** list, CMDHallMessageReq_t2 *info);
//����δ����ʶ
int update_textlivechatread_DB(Sink* sink, int64 messageid);
//�Ƿ�ʦ
int get_teacherflag_DB(Sink* sink,int userid);
//�����ظ�δ����¼��
int get_interactunreads_DB(Sink* sink,int userid);
//�۵�ظ�δ����¼��
int get_viewunreads_DB(Sink* sink,int userid);
//����δ����¼��
int get_questionunreads_DB(Sink* sink,int teacherid);
//�ش�δ����¼��
int get_answerunreads_DB(Sink* sink,int userid);
//ϵͳ�ظ�δ����¼��
//�ش�����
int64 insert_textliveanswer_DB(Sink* sink, CMDViewAnswerReq_t2 *info);

//��ȡ��ʦ���еĸ����ؼ���
int get_secretsbyteacher_info_DB(Sink* sink, int teacherid);
//��ȡ�û��ѹ���Ķ�Ӧ��ʦ�ĸ����ؼ���
int get_secretsbyuser_info_DB(Sink* sink, int teacherid, int userid);
//��ȡ��ʦ���еĸ����ؼ�
int get_TextRoomSecretsAll_list_DB(Sink* sink, SList** list,CMDTextRoomLiveListReq_t2 *listreq);
//��ȡ�û��ѹ���Ķ�Ӧ��ʦ�ĸ����ؼ�ID
int get_TextRoomSecretsUser_list_DB(Sink* sink, SList** list,CMDTextRoomLiveListReq_t2 *listreq);
//��ȡ�û��ѹ���Ķ�Ӧ��ʦ�ĸ����ؼ�
int get_TextRoomSecretsOwn_list_DB(Sink* sink, SList** list,CMDTextRoomLiveListReq_t2 *listreq);
//�ж��û��Ƿ��ѹ����Ӧ���ؼ�
int check_TextRoomSecretsBuy_DB(Sink* sink,int messageid, int teacherid, int userid);
//���¶�������
int update_TextRoomSecretsBuy_DB(Sink* sink,int messageid, int teacherid);

//�Թ۵����
int view_dianzan_DB(Sink* sink,long long viewid);
//�Թ۵��׻�
int view_sendflower_DB(Sink* sink,long long viewid,int count);
//�Թ۵�����
int view_comment_DB(Sink* sink,long long viewid,int fromid,int toid,long long reqtime,char* ccomments,long long srcinteractid,int reqcommentstype,long long* commentid);
//����ֱ����ʷ�б�
int func_getlivehistorylist_DB(Sink* sink,long long datetime/*���ĸ������տ�ʼȡ*/,int fromIndex,int bInc/*�Ƿ�����*/,int count/*ȡ��������¼*/,int teacherid,int vcbid,SList** list);
//����ĳ���ֱ���б�
//to do
//�û�����
int func_userquestion_DB(Sink* sink,int userid,int teacherid,long long reqtime,char* question,char* stokeid,unsigned char questiontype,long long* questionid);
//���ص�����
int func_getviewdianzan_DB(Sink* sink,long long viewid);
//�����׻�����
int func_getviewflower_DB(Sink* sink,long long viewid);
//�޸Ĺ۵�
int view_modify_view_DB(Sink* sink,long long* viewid/*��0�����������Ĺ۵�*/,int viewtypeid,int teacherid,long long datetime,char* title,char* content);

/*
 * Comments:Get text room live viewpoint type info from db
 * Param IN:sink
 * Param IN:teacherID
 * Param IN/OUT:viewpoint type info
 * Return:int, 0 means success, others mean failed.
 */
int get_textroomlivegroup_DB(Sink* sink, SList** list, unsigned int teacherID);

/*
 * Comments:Get text room live viewpoint type info from db(partial show content)�ò�ѯ��֧����ҳ��Ҳ֧�ֹ�����ѯ
 * Param IN:sink, db connection
 * Param IN:int, teacherID
 * Param IN:int, viewType
 * Param IN:int, startPos,��ѯ����ı�ţ���0��ʼ��ţ�����ʾ�ñ�ſ�ʼ�Ӳ�ѯ�����ȡ����
 * Param IN:int, messageid,�۵��б�Ĺ۵�ID
 * Param IN:int, lstCount,һ�β�ѯ�Ĺ۵����
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

//Return:int, -1 means failed, not -1 means query success and means row number in db.�ò�ѯ��֧����ҳ��Ҳ֧�ֹ�����ѯ.
int get_fansInfoByTeacherID_DB(Sink* sink, SList** list, unsigned int teacherid, unsigned int startPos, long long messageid, unsigned int count);

//Return:int, -1 means failed, not -1 means query success and means row number in db.�ò�ѯ��֧����ҳ��Ҳ֧�ֹ�����ѯ.
int get_teacherInfoByfansID_DB(Sink* sink, SList** records, unsigned int userid, unsigned int startPos, long long messageid, unsigned int count);

/*
 * Comments:��ý�ʦ��Ϣ
 * Param IN:sink
 * Param IN:int, startPos,��ѯ����ı�ţ���0��ʼ��ţ�����ʾ�ñ�ſ�ʼ�Ӳ�ѯ�����ȡ����
 * Param IN:int, messageid,�۵��б�Ĺ۵�ID
 * Param IN:int, lstCount,һ�β�ѯ�Ĺ۵����
 * Param IN/OUT:records, db query information
 * Return:int, 0 means result, -1 mean failed, others mean row number
 */
int get_teacher_info_list_DB(Sink* sink, SList** records, unsigned int startPos, long long messageid, unsigned int count);

/*
 * Comments:�������Ԥ��
 * Param IN:sink
 * Param IN:int, startPos,��ѯ����ı�ţ���0��ʼ��ţ�����ʾ�ñ�ſ�ʼ�Ӳ�ѯ�����ȡ����
 * Param IN:int, messageid,����Ԥ���ID
 * Param IN:int, lstCount,һ�β�ѯ�ĸ���
 * Param IN/OUT:records, db query information
 * Return:int, 0 means result, -1 mean failed, others mean row number
 */
int get_forecast_info_list_DB(Sink* sink, SList** records, unsigned int userid, unsigned int startPos, long long messageid, unsigned int count);

//run all type SQL return state
int get_DB(Sink *sink, Result *res, const char *query, int *row);

//��ȡ�û���δ�ش��б�
//��ȡ�û����ѻش��б�
//��ȡ��ʦ��δ�ش��б�
//��ȡ��ʦ���ѻش��б�
int func_getquestionlist_DB(Sink* sink,int userid,unsigned char bteacher, unsigned char bunread,long long messageid,int startpos,int count,SList** list);
//�鿴�û���û��δ����Ϣ
//��δ���ͷ���1�����򷵻�0
int func_getunreaddot_DB(Sink* sink,int userid);
//��ȡ�����
int get_supaccount_DB(Sink* sink, SList** list,char* date,int count);
//����vip�ȼ�
int update_viplevel_DB(Sink* sink,int userid,int viplevel);
//��ѯϵͳ����
int func_getsyscast_DB(Sink* sink,SList** list);

//��ѯ������ʹ�/�׻���Ѵ���
int func_getFreeTimes_DB(Sink* sink,int classid);
//��ȡʦͽ�б�
int get_shitulist_DB(Sink* sink,unsigned char bTeacherTag,int userid,SList** list);
//ͽ�ܱ�
int get_IsStudent_DB(Sink* sink,int teacherid,int userid);
//��ʦ
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

//��ѯ���ݿ��Ｋ�����͵�����
int load_jpush_config_db(Sink * sink, SList ** list);
//�������ݿ��Ｋ���������õ�validtime
int update_jpush_config_db(Sink * sink, DDLJPushConfig * record);
//��ȡ���ķ����û�
int get_tb_uservcbfavorite_DB(Sink *sink, SList** list, int *size, int nroomid);


//��ȡͽ������
int get_studentCount_DB(Sink* sink,int teacherid);
//��ȡ��Ȩ�б�
int get_PackagePrivilege_DB(Sink* sink, SList** list,int packageNum);

//��ȡ��ʦ�����б�
int get_tb_teachersubscription_DB(Sink *sink, SList** list, int *size, int nteacherid);

//�γ̶���
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
