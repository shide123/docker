/**
 * Autogenerated by Thrift Compiler (0.9.3)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef TTpPushSvrIf_TYPES_H
#define TTpPushSvrIf_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>




struct e_PushMsgType {
  enum type {
    PrivateChat = 0,
    GroupChat = 1,
    GroupPrivateChat = 2
  };
};

extern const std::map<int, const char*> _e_PushMsgType_VALUES_TO_NAMES;

class TChatMsgPush;

typedef struct _TChatMsgPush__isset {
  _TChatMsgPush__isset() : msg_type(false), srcuid(false), title(false), description(false), create_time(false), notify_id(false), include_muted(true), userids(false) {}
  bool msg_type :1;
  bool srcuid :1;
  bool title :1;
  bool description :1;
  bool create_time :1;
  bool notify_id :1;
  bool include_muted :1;
  bool userids :1;
} _TChatMsgPush__isset;

class TChatMsgPush {
 public:

  TChatMsgPush(const TChatMsgPush&);
  TChatMsgPush& operator=(const TChatMsgPush&);
  TChatMsgPush() : msg_type((e_PushMsgType::type)0), srcuid(0), title(), description(), create_time(0), notify_id(), include_muted(false) {
  }

  virtual ~TChatMsgPush() throw();
  e_PushMsgType::type msg_type;
  int32_t srcuid;
  std::string title;
  std::string description;
  int64_t create_time;
  std::string notify_id;
  bool include_muted;
  std::vector<int32_t>  userids;

  _TChatMsgPush__isset __isset;

  void __set_msg_type(const e_PushMsgType::type val);

  void __set_srcuid(const int32_t val);

  void __set_title(const std::string& val);

  void __set_description(const std::string& val);

  void __set_create_time(const int64_t val);

  void __set_notify_id(const std::string& val);

  void __set_include_muted(const bool val);

  void __set_userids(const std::vector<int32_t> & val);

  bool operator == (const TChatMsgPush & rhs) const
  {
    if (!(msg_type == rhs.msg_type))
      return false;
    if (!(srcuid == rhs.srcuid))
      return false;
    if (!(title == rhs.title))
      return false;
    if (!(description == rhs.description))
      return false;
    if (!(create_time == rhs.create_time))
      return false;
    if (!(notify_id == rhs.notify_id))
      return false;
    if (!(include_muted == rhs.include_muted))
      return false;
    if (!(userids == rhs.userids))
      return false;
    return true;
  }
  bool operator != (const TChatMsgPush &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TChatMsgPush & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(TChatMsgPush &a, TChatMsgPush &b);

inline std::ostream& operator<<(std::ostream& out, const TChatMsgPush& obj)
{
  obj.printTo(out);
  return out;
}



#endif
