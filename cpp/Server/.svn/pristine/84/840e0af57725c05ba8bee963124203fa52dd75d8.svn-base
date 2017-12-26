/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>

#include <iostream>
#include <stdexcept>
#include <sstream>

#include "TPushMsg.h"
#include "SL_ByteBuffer.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

class TPushMsgHandler: virtual public TPushMsgIf
{
public:

	TPushMsgHandler();

	/************************************************************************/
	/* function    ：proc_modVisitSwitch                                    */
	/* description ：修改群设置                                             */
	/* input param ：runid   执行者userid                                   */
	/*             ：groupid  群id                                          */
	/*             ：mSetting  设置key-value                                */
	/************************************************************************/
	virtual int32_t proc_modGroupSettings(const int32_t runid, const int32_t groupid, const std::map< ::E_THRIFT_GROUP_SETTING::type, std::string> & mSetting);

	/************************************************************************/
	/* function    ：proc_modVisitSwitch                                    */
	/* description ：修改围观限制                                           */
	/* input param ：runid   执行者userid                                   */
	/*             ：Switch  是否开放围观                                   */
	/*             ：kickOnlooker  禁止围观时是否踢出游客                   */
	/************************************************************************/
	virtual int32_t proc_modVisitSwitch(const int32_t runid, const int32_t groupid, const bool Switch, const bool kickOnlooker);                           

	/************************************************************************/
	/* function    ：proc_modJoinGroupMode                                  */
	/* description ：修改入群方式                                           */
	/* input param ：runid   执行者userid                                   */
	/*             ：setting  进群开关设置                                  */
	/************************************************************************/
	virtual int32_t proc_modJoinGroupMode(const int32_t runid, const int32_t groupid, const  ::TJoinGroupCondition& maleCondition, const  ::TJoinGroupCondition& femaleCondition);

	/************************************************************************/
	/* function    ：proc_kickAllOnlookers                                  */
	/* description ：踢除所有围观用户                                       */
	/* input param ：runid   执行者userid                                   */
	/*             ：groupid  群ID                                          */
	/************************************************************************/
	virtual int32_t proc_kickAllOnlookers(const int32_t runid, const int32_t groupid);

	/************************************************************************/
	/* function    ：proc_modContributionListSwitch                         */
	/* description ：修改贡献榜开关                                         */
	/* input param ：runid   执行者userid                                   */
	/*             ：groupid  群ID                                          */
	/*             ：Switch  开关，false 关闭，true 开启                    */
	/************************************************************************/
	virtual int32_t proc_modContributionListSwitch(const int32_t runid, const int32_t groupid, const bool Switch);

	/************************************************************************/
	/* function    ：proc_modVoiceChatSwitch                                */
	/* description ：修改群语音聊天开关                                     */
	/* input param ：runid   执行者userid                                   */
	/*             ：groupid  群ID                                          */
	/*             ：Switch  开关，false 关闭，true 开启                    */
	/************************************************************************/
	virtual int32_t proc_modVoiceChatSwitch(const int32_t runid, const int32_t groupid, const bool Switch, const bool visitOnMic);

	/************************************************************************/
	/* function    ：proc_modVideoAuthSwitch                                */
	/* description ：修改群视频认证开关                                     */
	/* input param ：runid   执行者userid                                   */
	/*             ：groupid  群ID                                          */
	/*             ：Switch  开关，false 关闭，true 开启                    */
	/************************************************************************/
	virtual int32_t proc_modVideoAuthSwitch(const int32_t runid, const int32_t groupid, const bool Switch);

	/************************************************************************/
	/* function    ：proc_modGroupGainSetting                               */
	/* description ：修改群收入分成比例                                     */
	/* input param ：runid   执行者userid                                   */
	/*             ：groupid  群ID                                          */
	/*             ：rangeType 收入分成范围，0 游客，1 仅正式成员，2 所有人 */
	/*             ：percent  分成比例                                      */
	/************************************************************************/
	virtual int32_t proc_modGroupGainSetting(const int32_t runid, const int32_t groupid, const bool Switch, const int8_t rangeType, const int8_t percent);

	/************************************************************************/
	/* function    ：proc_modGroupUserGainSetting                           */
	/* description ：修改群红包分成比例                                     */
	/* input param ：runid   执行者userid                                   */
	/*             ：groupid  群ID                                          */
	/*             ：userid  用户ID                                         */
	/*             ：percent  分成比例                                      */
	/************************************************************************/
	virtual int32_t proc_modGroupUserGainSetting(const int32_t runid, const int32_t groupid, const int32_t userid, const int8_t percent);

	/************************************************************************/
	/* function    ：proc_modGroupUserRole                                  */
	/* description ：调整成员等级                                           */
	/* input param ：runid   执行者userid                                   */
	/*             ：groupid  群ID                                          */
	/*             ：userid  用户ID                                         */
	/*             ：roleType  角色类型                                     */
	/************************************************************************/
	virtual int32_t proc_modGroupUserRole(const int32_t runid, const int32_t groupid, const int32_t userid, const int32_t roleType);

	/************************************************************************/
	/* function    ：proc_modGroupBlackList                                 */
	/* description ：修改黑名单列表                                         */
	/* input param ：runid   执行者userid                                   */
	/*             ：groupid  群ID                                          */
	/*             ：userid  用户ID                                         */
	/*             ：isAdd  是否加入黑名单, false 否, true 是               */
	/************************************************************************/
	virtual int32_t proc_modGroupBlackList(const int32_t runid, const int32_t groupid, const int32_t userid, const bool isAdd);

	/************************************************************************/
	/* function     ：proc_getGroupVisitorLst                               */
	/* description  ：获取群观众列表                                        */
	/* input param  ：groupid  群ID                                         */
	/* output param ：visitorLst  群观众列表                                */
	/************************************************************************/
	virtual void proc_getGroupVisitorLst(std::vector< ::TUserGroupInfo> & _return, const int32_t groupid);

	/************************************************************************/
	/* function    ：proc_reloadUserInfo                                    */
	/* description :刷新用户基本信息                                        */
	/* input param ：userid   用户ID                                        */
	/************************************************************************/
	virtual int32_t proc_reloadUserInfo(const int32_t userid);

	/************************************************************************/
	/* function    ：proc_sendSysNoticeMsg                                  */
	/* description ：发送系统消息                                           */
	/* input param ：msg   	系统消息详情                                    */
	/* input param ：lstCondition   发送条件                                */
	/* input param ：onlineOnly   是否只发给在线用户                        */
	/************************************************************************/
	virtual int32_t proc_sendSysNoticeMsg(const  ::TNoticeMsg& msg, const std::vector< ::TCondition> & lstCondition, const bool onlineOnly);
	
	/************************************************************************/
	/* function    ：proc_sendUserNoticeMsg                                 */
	/* description ：发送用户系统消息                                       */
	/* input param ：userid   	用户ID                                      */
	/* input param ：msg   用户系统消息详情                                 */
	/************************************************************************/
	virtual int32_t proc_sendUserNoticeMsg(const int32_t userid, const  ::TNoticeMsg& msg);

	/************************************************************************/
	/* function    ：proc_sendAdminVideoAuditMsg                            */
	/* description ：发送用户视频审核消息                                   */
	/* input param ：userid   	用户ID                                      */
	/* input param ：groupid   	群ID                                        */
	/* input param ：url   用户视频审核url                                  */
	/************************************************************************/
	virtual int32_t proc_sendAdminVideoAuditMsg(const int32_t userid, const int32_t groupid, const std::string& url);

	/************************************************************************/
	/* function    ：proc_handleVideoAuditMsg                               */
	/* description ：处理用户视频审核消息                                   */
	/* input param ：handle_userid   	处理用户ID                          */
	/* input param ：groupid   	群ID                                        */
	/* input param ：msgid   	消息ID                                      */
	/* input param ：agree   同意或拒绝                                     */
	/************************************************************************/
	virtual bool proc_handleVideoAuditMsg(const int32_t handle_userid, const int32_t groupid, const int32_t msgid, const bool agree, const int32_t apply_userid);

	/************************************************************************/
	/* function    ：proc_loadGroup                                         */
	/* description ：加载群信息		                                        */
	/* input param ：groupid   群ID                                         */
	/************************************************************************/
	virtual int32_t proc_loadGroup(const int32_t groupid, const std::string& host);

	/************************************************************************/
	/* function    ：proc_modGroup                                          */
	/* description ：修改加载群信息		                                    */
	/* input param ：groupid   群ID                                         */
	/************************************************************************/
	virtual int32_t proc_modGroup(const int32_t groupid, const std::string& host);

	/************************************************************************/
	/* function    ：proc_setGroupOwner                                     */
	/* description ：设置群主		                                        */
	/* input param ：groupid   群ID                                         */
	/* input param ：new_ownerid   新的群主ID                               */
	/************************************************************************/
	virtual bool proc_setGroupOwner(const int32_t groupid, const int32_t new_ownerid);

	/************************************************************************/
	/* function    ：proc_queryInGroupList                                  */
	/* description ：查询当前在群列表                                       */
	/* input param ：userid   查询者I                                       */
	/* input param ：groupid  群ID                                          */
	/************************************************************************/
	virtual void proc_queryInGroupList(std::map< ::e_INGROUP_TYPE::type, std::vector<int32_t> > & _return, const int32_t userid, const int32_t groupid);

	/************************************************************************/
	/* function    ：proc_joingroup                                         */
	/* description ：进群请求                                               */
	/* input param ：oReq   进群请求结构体                                  */
	/************************************************************************/
	virtual int32_t proc_joingroup(const  ::TJoinGroup& oReq);
};

