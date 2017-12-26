/**
 * Autogenerated by Thrift Compiler (0.9.3)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "TCommonIf_types.h"

#include <algorithm>
#include <ostream>

#include <thrift/TToString.h>



int _kE_THRIFT_GROUP_SETTINGValues[] = {
  E_THRIFT_GROUP_SETTING::VISIT_SWITCH,
  E_THRIFT_GROUP_SETTING::KICK_ALL_VISITOR,
  E_THRIFT_GROUP_SETTING::MALE_JOIN_AUTH,
  E_THRIFT_GROUP_SETTING::MALE_JOIN_REDPACKET_NUM,
  E_THRIFT_GROUP_SETTING::MALE_JOIN_REDPACKET_AMOUNT,
  E_THRIFT_GROUP_SETTING::FEMALE_JOIN_AUTH,
  E_THRIFT_GROUP_SETTING::FEMALE_JOIN_REDPACKET_NUM,
  E_THRIFT_GROUP_SETTING::FEMALE_JOIN_REDPACKET_AMOUNT,
  E_THRIFT_GROUP_SETTING::CONTRIBUTION_LIST_SWITCH,
  E_THRIFT_GROUP_SETTING::VOICE_CHAT_SWITCH,
  E_THRIFT_GROUP_SETTING::ALLOW_VISITOR_ON_MIC,
  E_THRIFT_GROUP_SETTING::GAIN_RANGE_TYPE,
  E_THRIFT_GROUP_SETTING::GAIN_PERCENT,
  E_THRIFT_GROUP_SETTING::PRIVATE_CHAT_SWITCH,
  E_THRIFT_GROUP_SETTING::PCHAT_MIN_THRESHOLD_MALE,
  E_THRIFT_GROUP_SETTING::PCHAT_MIN_THRESHOLD_FEMALE,
  E_THRIFT_GROUP_SETTING::GAIN_SWITCH,
  E_THRIFT_GROUP_SETTING::GAIN_GROUP
};
const char* _kE_THRIFT_GROUP_SETTINGNames[] = {
  "VISIT_SWITCH",
  "KICK_ALL_VISITOR",
  "MALE_JOIN_AUTH",
  "MALE_JOIN_REDPACKET_NUM",
  "MALE_JOIN_REDPACKET_AMOUNT",
  "FEMALE_JOIN_AUTH",
  "FEMALE_JOIN_REDPACKET_NUM",
  "FEMALE_JOIN_REDPACKET_AMOUNT",
  "CONTRIBUTION_LIST_SWITCH",
  "VOICE_CHAT_SWITCH",
  "ALLOW_VISITOR_ON_MIC",
  "GAIN_RANGE_TYPE",
  "GAIN_PERCENT",
  "PRIVATE_CHAT_SWITCH",
  "PCHAT_MIN_THRESHOLD_MALE",
  "PCHAT_MIN_THRESHOLD_FEMALE",
  "GAIN_SWITCH",
  "GAIN_GROUP"
};
const std::map<int, const char*> _E_THRIFT_GROUP_SETTING_VALUES_TO_NAMES(::apache::thrift::TEnumIterator(18, _kE_THRIFT_GROUP_SETTINGValues, _kE_THRIFT_GROUP_SETTINGNames), ::apache::thrift::TEnumIterator(-1, NULL, NULL));

int _ke_INGROUP_TYPEValues[] = {
  e_INGROUP_TYPE::Member,
  e_INGROUP_TYPE::Visitor
};
const char* _ke_INGROUP_TYPENames[] = {
  "Member",
  "Visitor"
};
const std::map<int, const char*> _e_INGROUP_TYPE_VALUES_TO_NAMES(::apache::thrift::TEnumIterator(2, _ke_INGROUP_TYPEValues, _ke_INGROUP_TYPENames), ::apache::thrift::TEnumIterator(-1, NULL, NULL));

int _ke_JoinGroupAuthTypeValues[] = {
  e_JoinGroupAuthType::NoNeedAuth,
  e_JoinGroupAuthType::RedPacket,
  e_JoinGroupAuthType::VideoAuth,
  e_JoinGroupAuthType::MsgAuth,
  e_JoinGroupAuthType::Forbidden
};
const char* _ke_JoinGroupAuthTypeNames[] = {
  "NoNeedAuth",
  "RedPacket",
  "VideoAuth",
  "MsgAuth",
  "Forbidden"
};
const std::map<int, const char*> _e_JoinGroupAuthType_VALUES_TO_NAMES(::apache::thrift::TEnumIterator(5, _ke_JoinGroupAuthTypeValues, _ke_JoinGroupAuthTypeNames), ::apache::thrift::TEnumIterator(-1, NULL, NULL));

int _kSYSMSG_COND_TYPEValues[] = {
  SYSMSG_COND_TYPE::USER,
  SYSMSG_COND_TYPE::TELNUM,
  SYSMSG_COND_TYPE::APPVERSION
};
const char* _kSYSMSG_COND_TYPENames[] = {
  "USER",
  "TELNUM",
  "APPVERSION"
};
const std::map<int, const char*> _SYSMSG_COND_TYPE_VALUES_TO_NAMES(::apache::thrift::TEnumIterator(3, _kSYSMSG_COND_TYPEValues, _kSYSMSG_COND_TYPENames), ::apache::thrift::TEnumIterator(-1, NULL, NULL));


TJoinGroup::~TJoinGroup() throw() {
}


void TJoinGroup::__set_userid(const int32_t val) {
  this->userid = val;
}

void TJoinGroup::__set_groupid(const int32_t val) {
  this->groupid = val;
}

void TJoinGroup::__set_inway(const int8_t val) {
  this->inway = val;
}

void TJoinGroup::__set_action(const int8_t val) {
  this->action = val;
}

uint32_t TJoinGroup::read(::apache::thrift::protocol::TProtocol* iprot) {

  apache::thrift::protocol::TInputRecursionTracker tracker(*iprot);
  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->userid);
          this->__isset.userid = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->groupid);
          this->__isset.groupid = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_BYTE) {
          xfer += iprot->readByte(this->inway);
          this->__isset.inway = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_BYTE) {
          xfer += iprot->readByte(this->action);
          this->__isset.action = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t TJoinGroup::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("TJoinGroup");

  xfer += oprot->writeFieldBegin("userid", ::apache::thrift::protocol::T_I32, 1);
  xfer += oprot->writeI32(this->userid);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("groupid", ::apache::thrift::protocol::T_I32, 2);
  xfer += oprot->writeI32(this->groupid);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("inway", ::apache::thrift::protocol::T_BYTE, 3);
  xfer += oprot->writeByte(this->inway);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("action", ::apache::thrift::protocol::T_BYTE, 4);
  xfer += oprot->writeByte(this->action);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(TJoinGroup &a, TJoinGroup &b) {
  using ::std::swap;
  swap(a.userid, b.userid);
  swap(a.groupid, b.groupid);
  swap(a.inway, b.inway);
  swap(a.action, b.action);
  swap(a.__isset, b.__isset);
}

TJoinGroup::TJoinGroup(const TJoinGroup& other0) {
  userid = other0.userid;
  groupid = other0.groupid;
  inway = other0.inway;
  action = other0.action;
  __isset = other0.__isset;
}
TJoinGroup& TJoinGroup::operator=(const TJoinGroup& other1) {
  userid = other1.userid;
  groupid = other1.groupid;
  inway = other1.inway;
  action = other1.action;
  __isset = other1.__isset;
  return *this;
}
void TJoinGroup::printTo(std::ostream& out) const {
  using ::apache::thrift::to_string;
  out << "TJoinGroup(";
  out << "userid=" << to_string(userid);
  out << ", " << "groupid=" << to_string(groupid);
  out << ", " << "inway=" << to_string(inway);
  out << ", " << "action=" << to_string(action);
  out << ")";
}


TUserGroupInfo::~TUserGroupInfo() throw() {
}


void TUserGroupInfo::__set_userID(const int32_t val) {
  this->userID = val;
}

void TUserGroupInfo::__set_userAlias(const std::string& val) {
  this->userAlias = val;
}

void TUserGroupInfo::__set_userHead(const std::string& val) {
  this->userHead = val;
}

void TUserGroupInfo::__set_groupID(const int32_t val) {
  this->groupID = val;
}

void TUserGroupInfo::__set_roleType(const int32_t val) {
  this->roleType = val;
}

uint32_t TUserGroupInfo::read(::apache::thrift::protocol::TProtocol* iprot) {

  apache::thrift::protocol::TInputRecursionTracker tracker(*iprot);
  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->userID);
          this->__isset.userID = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->userAlias);
          this->__isset.userAlias = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->userHead);
          this->__isset.userHead = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->groupID);
          this->__isset.groupID = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 5:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->roleType);
          this->__isset.roleType = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t TUserGroupInfo::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("TUserGroupInfo");

  xfer += oprot->writeFieldBegin("userID", ::apache::thrift::protocol::T_I32, 1);
  xfer += oprot->writeI32(this->userID);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("userAlias", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString(this->userAlias);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("userHead", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeString(this->userHead);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("groupID", ::apache::thrift::protocol::T_I32, 4);
  xfer += oprot->writeI32(this->groupID);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("roleType", ::apache::thrift::protocol::T_I32, 5);
  xfer += oprot->writeI32(this->roleType);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(TUserGroupInfo &a, TUserGroupInfo &b) {
  using ::std::swap;
  swap(a.userID, b.userID);
  swap(a.userAlias, b.userAlias);
  swap(a.userHead, b.userHead);
  swap(a.groupID, b.groupID);
  swap(a.roleType, b.roleType);
  swap(a.__isset, b.__isset);
}

TUserGroupInfo::TUserGroupInfo(const TUserGroupInfo& other2) {
  userID = other2.userID;
  userAlias = other2.userAlias;
  userHead = other2.userHead;
  groupID = other2.groupID;
  roleType = other2.roleType;
  __isset = other2.__isset;
}
TUserGroupInfo& TUserGroupInfo::operator=(const TUserGroupInfo& other3) {
  userID = other3.userID;
  userAlias = other3.userAlias;
  userHead = other3.userHead;
  groupID = other3.groupID;
  roleType = other3.roleType;
  __isset = other3.__isset;
  return *this;
}
void TUserGroupInfo::printTo(std::ostream& out) const {
  using ::apache::thrift::to_string;
  out << "TUserGroupInfo(";
  out << "userID=" << to_string(userID);
  out << ", " << "userAlias=" << to_string(userAlias);
  out << ", " << "userHead=" << to_string(userHead);
  out << ", " << "groupID=" << to_string(groupID);
  out << ", " << "roleType=" << to_string(roleType);
  out << ")";
}


TJoinGroupCondition::~TJoinGroupCondition() throw() {
}


void TJoinGroupCondition::__set_type(const e_JoinGroupAuthType::type val) {
  this->type = val;
}

void TJoinGroupCondition::__set_redPacketNum(const int16_t val) {
  this->redPacketNum = val;
}

void TJoinGroupCondition::__set_redPacketAmount(const int32_t val) {
  this->redPacketAmount = val;
}

uint32_t TJoinGroupCondition::read(::apache::thrift::protocol::TProtocol* iprot) {

  apache::thrift::protocol::TInputRecursionTracker tracker(*iprot);
  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          int32_t ecast4;
          xfer += iprot->readI32(ecast4);
          this->type = (e_JoinGroupAuthType::type)ecast4;
          this->__isset.type = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_I16) {
          xfer += iprot->readI16(this->redPacketNum);
          this->__isset.redPacketNum = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->redPacketAmount);
          this->__isset.redPacketAmount = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t TJoinGroupCondition::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("TJoinGroupCondition");

  xfer += oprot->writeFieldBegin("type", ::apache::thrift::protocol::T_I32, 1);
  xfer += oprot->writeI32((int32_t)this->type);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("redPacketNum", ::apache::thrift::protocol::T_I16, 2);
  xfer += oprot->writeI16(this->redPacketNum);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("redPacketAmount", ::apache::thrift::protocol::T_I32, 3);
  xfer += oprot->writeI32(this->redPacketAmount);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(TJoinGroupCondition &a, TJoinGroupCondition &b) {
  using ::std::swap;
  swap(a.type, b.type);
  swap(a.redPacketNum, b.redPacketNum);
  swap(a.redPacketAmount, b.redPacketAmount);
  swap(a.__isset, b.__isset);
}

TJoinGroupCondition::TJoinGroupCondition(const TJoinGroupCondition& other5) {
  type = other5.type;
  redPacketNum = other5.redPacketNum;
  redPacketAmount = other5.redPacketAmount;
  __isset = other5.__isset;
}
TJoinGroupCondition& TJoinGroupCondition::operator=(const TJoinGroupCondition& other6) {
  type = other6.type;
  redPacketNum = other6.redPacketNum;
  redPacketAmount = other6.redPacketAmount;
  __isset = other6.__isset;
  return *this;
}
void TJoinGroupCondition::printTo(std::ostream& out) const {
  using ::apache::thrift::to_string;
  out << "TJoinGroupCondition(";
  out << "type=" << to_string(type);
  out << ", " << "redPacketNum=" << to_string(redPacketNum);
  out << ", " << "redPacketAmount=" << to_string(redPacketAmount);
  out << ")";
}


TCondition::~TCondition() throw() {
}


void TCondition::__set_type(const SYSMSG_COND_TYPE::type val) {
  this->type = val;
}

void TCondition::__set_oper(const std::string& val) {
  this->oper = val;
}

void TCondition::__set_value(const std::string& val) {
  this->value = val;
}

void TCondition::__set_values(const std::set<std::string> & val) {
  this->values = val;
}

uint32_t TCondition::read(::apache::thrift::protocol::TProtocol* iprot) {

  apache::thrift::protocol::TInputRecursionTracker tracker(*iprot);
  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          int32_t ecast7;
          xfer += iprot->readI32(ecast7);
          this->type = (SYSMSG_COND_TYPE::type)ecast7;
          this->__isset.type = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->oper);
          this->__isset.oper = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->value);
          this->__isset.value = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_SET) {
          {
            this->values.clear();
            uint32_t _size8;
            ::apache::thrift::protocol::TType _etype11;
            xfer += iprot->readSetBegin(_etype11, _size8);
            uint32_t _i12;
            for (_i12 = 0; _i12 < _size8; ++_i12)
            {
              std::string _elem13;
              xfer += iprot->readString(_elem13);
              this->values.insert(_elem13);
            }
            xfer += iprot->readSetEnd();
          }
          this->__isset.values = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t TCondition::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("TCondition");

  xfer += oprot->writeFieldBegin("type", ::apache::thrift::protocol::T_I32, 1);
  xfer += oprot->writeI32((int32_t)this->type);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("oper", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString(this->oper);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("value", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeString(this->value);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("values", ::apache::thrift::protocol::T_SET, 4);
  {
    xfer += oprot->writeSetBegin(::apache::thrift::protocol::T_STRING, static_cast<uint32_t>(this->values.size()));
    std::set<std::string> ::const_iterator _iter14;
    for (_iter14 = this->values.begin(); _iter14 != this->values.end(); ++_iter14)
    {
      xfer += oprot->writeString((*_iter14));
    }
    xfer += oprot->writeSetEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(TCondition &a, TCondition &b) {
  using ::std::swap;
  swap(a.type, b.type);
  swap(a.oper, b.oper);
  swap(a.value, b.value);
  swap(a.values, b.values);
  swap(a.__isset, b.__isset);
}

TCondition::TCondition(const TCondition& other15) {
  type = other15.type;
  oper = other15.oper;
  value = other15.value;
  values = other15.values;
  __isset = other15.__isset;
}
TCondition& TCondition::operator=(const TCondition& other16) {
  type = other16.type;
  oper = other16.oper;
  value = other16.value;
  values = other16.values;
  __isset = other16.__isset;
  return *this;
}
void TCondition::printTo(std::ostream& out) const {
  using ::apache::thrift::to_string;
  out << "TCondition(";
  out << "type=" << to_string(type);
  out << ", " << "oper=" << to_string(oper);
  out << ", " << "value=" << to_string(value);
  out << ", " << "values=" << to_string(values);
  out << ")";
}


TNoticeMsg::~TNoticeMsg() throw() {
}


void TNoticeMsg::__set_type(const int16_t val) {
  this->type = val;
}

void TNoticeMsg::__set_content(const std::string& val) {
  this->content = val;
}

void TNoticeMsg::__set_action(const std::string& val) {
  this->action = val;
}

void TNoticeMsg::__set_endTime(const int32_t val) {
  this->endTime = val;
}

uint32_t TNoticeMsg::read(::apache::thrift::protocol::TProtocol* iprot) {

  apache::thrift::protocol::TInputRecursionTracker tracker(*iprot);
  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I16) {
          xfer += iprot->readI16(this->type);
          this->__isset.type = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->content);
          this->__isset.content = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->action);
          this->__isset.action = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->endTime);
          this->__isset.endTime = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t TNoticeMsg::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("TNoticeMsg");

  xfer += oprot->writeFieldBegin("type", ::apache::thrift::protocol::T_I16, 1);
  xfer += oprot->writeI16(this->type);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("content", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString(this->content);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("action", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeString(this->action);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("endTime", ::apache::thrift::protocol::T_I32, 4);
  xfer += oprot->writeI32(this->endTime);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(TNoticeMsg &a, TNoticeMsg &b) {
  using ::std::swap;
  swap(a.type, b.type);
  swap(a.content, b.content);
  swap(a.action, b.action);
  swap(a.endTime, b.endTime);
  swap(a.__isset, b.__isset);
}

TNoticeMsg::TNoticeMsg(const TNoticeMsg& other17) {
  type = other17.type;
  content = other17.content;
  action = other17.action;
  endTime = other17.endTime;
  __isset = other17.__isset;
}
TNoticeMsg& TNoticeMsg::operator=(const TNoticeMsg& other18) {
  type = other18.type;
  content = other18.content;
  action = other18.action;
  endTime = other18.endTime;
  __isset = other18.__isset;
  return *this;
}
void TNoticeMsg::printTo(std::ostream& out) const {
  using ::apache::thrift::to_string;
  out << "TNoticeMsg(";
  out << "type=" << to_string(type);
  out << ", " << "content=" << to_string(content);
  out << ", " << "action=" << to_string(action);
  out << ", " << "endTime=" << to_string(endTime);
  out << ")";
}


