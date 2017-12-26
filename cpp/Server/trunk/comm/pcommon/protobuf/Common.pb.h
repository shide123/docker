// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: Common.proto

#ifndef PROTOBUF_Common_2eproto__INCLUDED
#define PROTOBUF_Common_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_Common_2eproto();
void protobuf_AssignDesc_Common_2eproto();
void protobuf_ShutdownFile_Common_2eproto();

class CMDErrCode;
class CMDString;
class CMDUInt32;
class UserInfo_t;

enum e_GenderType {
  e_Gender_Unknown = 0,
  e_Gender_Male = 1,
  e_Gender_Female = 2,
  e_GenderType_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  e_GenderType_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool e_GenderType_IsValid(int value);
const e_GenderType e_GenderType_MIN = e_Gender_Unknown;
const e_GenderType e_GenderType_MAX = e_Gender_Female;
const int e_GenderType_ARRAYSIZE = e_GenderType_MAX + 1;

const ::google::protobuf::EnumDescriptor* e_GenderType_descriptor();
inline const ::std::string& e_GenderType_Name(e_GenderType value) {
  return ::google::protobuf::internal::NameOfEnum(
    e_GenderType_descriptor(), value);
}
inline bool e_GenderType_Parse(
    const ::std::string& name, e_GenderType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<e_GenderType>(
    e_GenderType_descriptor(), name, value);
}
enum e_MsgType {
  MSGTYPE_TEXT = 0,
  MSGTYPE_IMAGE_WEIXIN = 1,
  MSGTYPE_VOICE_WEIXIN = 2,
  MSGTYPE_VEDIO = 3,
  MSGTYPE_REDPACK = 4,
  MSGTYPE_ASSIST = 5,
  MSGTYPE_SYSTEMINFO = 6,
  MSGTYPE_REMIND = 7,
  MSGTYPE_RECALL = 8,
  MSGTYPE_LINKS = 9,
  MSGTYPE_STICKER = 10,
  MSGTYPE_TIP = 11,
  MSGTYPE_IMAGE = 12,
  MSGTYPE_VOICE = 13,
  MSGTYPE_UPLOADWALL = 14,
  MSGTYPE_CLOSE_LIVE_COURSE = 15,
  MSGTYPE_PPT_VOICE = 16,
  MSGTYPE_CANCELL_KEYPOINT = 17,
  e_MsgType_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  e_MsgType_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool e_MsgType_IsValid(int value);
const e_MsgType e_MsgType_MIN = MSGTYPE_TEXT;
const e_MsgType e_MsgType_MAX = MSGTYPE_CANCELL_KEYPOINT;
const int e_MsgType_ARRAYSIZE = e_MsgType_MAX + 1;

const ::google::protobuf::EnumDescriptor* e_MsgType_descriptor();
inline const ::std::string& e_MsgType_Name(e_MsgType value) {
  return ::google::protobuf::internal::NameOfEnum(
    e_MsgType_descriptor(), value);
}
inline bool e_MsgType_Parse(
    const ::std::string& name, e_MsgType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<e_MsgType>(
    e_MsgType_descriptor(), name, value);
}
enum e_RemindType {
  REMIND_TAKE_REDPACKET = 0,
  REMIND_BE_TAKEN_REDPACKET = 1,
  REMIND_JOINGROUP = 2,
  REMIND_ROLECHANGE = 3,
  e_RemindType_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  e_RemindType_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool e_RemindType_IsValid(int value);
const e_RemindType e_RemindType_MIN = REMIND_TAKE_REDPACKET;
const e_RemindType e_RemindType_MAX = REMIND_ROLECHANGE;
const int e_RemindType_ARRAYSIZE = e_RemindType_MAX + 1;

const ::google::protobuf::EnumDescriptor* e_RemindType_descriptor();
inline const ::std::string& e_RemindType_Name(e_RemindType value) {
  return ::google::protobuf::internal::NameOfEnum(
    e_RemindType_descriptor(), value);
}
inline bool e_RemindType_Parse(
    const ::std::string& name, e_RemindType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<e_RemindType>(
    e_RemindType_descriptor(), name, value);
}
// ===================================================================

class CMDString : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:CMDString) */ {
 public:
  CMDString();
  virtual ~CMDString();

  CMDString(const CMDString& from);

  inline CMDString& operator=(const CMDString& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const CMDString& default_instance();

  void Swap(CMDString* other);

  // implements Message ----------------------------------------------

  inline CMDString* New() const { return New(NULL); }

  CMDString* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const CMDString& from);
  void MergeFrom(const CMDString& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(CMDString* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string data = 1;
  void clear_data();
  static const int kDataFieldNumber = 1;
  const ::std::string& data() const;
  void set_data(const ::std::string& value);
  void set_data(const char* value);
  void set_data(const char* value, size_t size);
  ::std::string* mutable_data();
  ::std::string* release_data();
  void set_allocated_data(::std::string* data);

  // @@protoc_insertion_point(class_scope:CMDString)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr data_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_Common_2eproto();
  friend void protobuf_AssignDesc_Common_2eproto();
  friend void protobuf_ShutdownFile_Common_2eproto();

  void InitAsDefaultInstance();
  static CMDString* default_instance_;
};
// -------------------------------------------------------------------

class CMDUInt32 : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:CMDUInt32) */ {
 public:
  CMDUInt32();
  virtual ~CMDUInt32();

  CMDUInt32(const CMDUInt32& from);

  inline CMDUInt32& operator=(const CMDUInt32& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const CMDUInt32& default_instance();

  void Swap(CMDUInt32* other);

  // implements Message ----------------------------------------------

  inline CMDUInt32* New() const { return New(NULL); }

  CMDUInt32* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const CMDUInt32& from);
  void MergeFrom(const CMDUInt32& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(CMDUInt32* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional uint32 data = 1;
  void clear_data();
  static const int kDataFieldNumber = 1;
  ::google::protobuf::uint32 data() const;
  void set_data(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:CMDUInt32)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::uint32 data_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_Common_2eproto();
  friend void protobuf_AssignDesc_Common_2eproto();
  friend void protobuf_ShutdownFile_Common_2eproto();

  void InitAsDefaultInstance();
  static CMDUInt32* default_instance_;
};
// -------------------------------------------------------------------

class UserInfo_t : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:UserInfo_t) */ {
 public:
  UserInfo_t();
  virtual ~UserInfo_t();

  UserInfo_t(const UserInfo_t& from);

  inline UserInfo_t& operator=(const UserInfo_t& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const UserInfo_t& default_instance();

  void Swap(UserInfo_t* other);

  // implements Message ----------------------------------------------

  inline UserInfo_t* New() const { return New(NULL); }

  UserInfo_t* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const UserInfo_t& from);
  void MergeFrom(const UserInfo_t& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(UserInfo_t* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional uint32 userId = 1;
  void clear_userid();
  static const int kUserIdFieldNumber = 1;
  ::google::protobuf::uint32 userid() const;
  void set_userid(::google::protobuf::uint32 value);

  // optional string alias = 2;
  void clear_alias();
  static const int kAliasFieldNumber = 2;
  const ::std::string& alias() const;
  void set_alias(const ::std::string& value);
  void set_alias(const char* value);
  void set_alias(const char* value, size_t size);
  ::std::string* mutable_alias();
  ::std::string* release_alias();
  void set_allocated_alias(::std::string* alias);

  // optional string head = 3;
  void clear_head();
  static const int kHeadFieldNumber = 3;
  const ::std::string& head() const;
  void set_head(const ::std::string& value);
  void set_head(const char* value);
  void set_head(const char* value, size_t size);
  ::std::string* mutable_head();
  ::std::string* release_head();
  void set_allocated_head(::std::string* head);

  // optional uint32 roleType = 4;
  void clear_roletype();
  static const int kRoleTypeFieldNumber = 4;
  ::google::protobuf::uint32 roletype() const;
  void set_roletype(::google::protobuf::uint32 value);

  // optional uint32 authState = 5;
  void clear_authstate();
  static const int kAuthStateFieldNumber = 5;
  ::google::protobuf::uint32 authstate() const;
  void set_authstate(::google::protobuf::uint32 value);

  // optional .e_GenderType gender = 6;
  void clear_gender();
  static const int kGenderFieldNumber = 6;
  ::e_GenderType gender() const;
  void set_gender(::e_GenderType value);

  // optional uint32 UserType = 7;
  void clear_usertype();
  static const int kUserTypeFieldNumber = 7;
  ::google::protobuf::uint32 usertype() const;
  void set_usertype(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:UserInfo_t)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr alias_;
  ::google::protobuf::uint32 userid_;
  ::google::protobuf::uint32 roletype_;
  ::google::protobuf::internal::ArenaStringPtr head_;
  ::google::protobuf::uint32 authstate_;
  int gender_;
  ::google::protobuf::uint32 usertype_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_Common_2eproto();
  friend void protobuf_AssignDesc_Common_2eproto();
  friend void protobuf_ShutdownFile_Common_2eproto();

  void InitAsDefaultInstance();
  static UserInfo_t* default_instance_;
};
// -------------------------------------------------------------------

class CMDErrCode : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:CMDErrCode) */ {
 public:
  CMDErrCode();
  virtual ~CMDErrCode();

  CMDErrCode(const CMDErrCode& from);

  inline CMDErrCode& operator=(const CMDErrCode& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const CMDErrCode& default_instance();

  void Swap(CMDErrCode* other);

  // implements Message ----------------------------------------------

  inline CMDErrCode* New() const { return New(NULL); }

  CMDErrCode* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const CMDErrCode& from);
  void MergeFrom(const CMDErrCode& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(CMDErrCode* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional uint32 errID = 1;
  void clear_errid();
  static const int kErrIDFieldNumber = 1;
  ::google::protobuf::uint32 errid() const;
  void set_errid(::google::protobuf::uint32 value);

  // optional string errMsg = 2;
  void clear_errmsg();
  static const int kErrMsgFieldNumber = 2;
  const ::std::string& errmsg() const;
  void set_errmsg(const ::std::string& value);
  void set_errmsg(const char* value);
  void set_errmsg(const char* value, size_t size);
  ::std::string* mutable_errmsg();
  ::std::string* release_errmsg();
  void set_allocated_errmsg(::std::string* errmsg);

  // repeated string paramValue = 3;
  int paramvalue_size() const;
  void clear_paramvalue();
  static const int kParamValueFieldNumber = 3;
  const ::std::string& paramvalue(int index) const;
  ::std::string* mutable_paramvalue(int index);
  void set_paramvalue(int index, const ::std::string& value);
  void set_paramvalue(int index, const char* value);
  void set_paramvalue(int index, const char* value, size_t size);
  ::std::string* add_paramvalue();
  void add_paramvalue(const ::std::string& value);
  void add_paramvalue(const char* value);
  void add_paramvalue(const char* value, size_t size);
  const ::google::protobuf::RepeatedPtrField< ::std::string>& paramvalue() const;
  ::google::protobuf::RepeatedPtrField< ::std::string>* mutable_paramvalue();

  // @@protoc_insertion_point(class_scope:CMDErrCode)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr errmsg_;
  ::google::protobuf::RepeatedPtrField< ::std::string> paramvalue_;
  ::google::protobuf::uint32 errid_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_Common_2eproto();
  friend void protobuf_AssignDesc_Common_2eproto();
  friend void protobuf_ShutdownFile_Common_2eproto();

  void InitAsDefaultInstance();
  static CMDErrCode* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// CMDString

// optional string data = 1;
inline void CMDString::clear_data() {
  data_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& CMDString::data() const {
  // @@protoc_insertion_point(field_get:CMDString.data)
  return data_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void CMDString::set_data(const ::std::string& value) {
  
  data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:CMDString.data)
}
inline void CMDString::set_data(const char* value) {
  
  data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:CMDString.data)
}
inline void CMDString::set_data(const char* value, size_t size) {
  
  data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:CMDString.data)
}
inline ::std::string* CMDString::mutable_data() {
  
  // @@protoc_insertion_point(field_mutable:CMDString.data)
  return data_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* CMDString::release_data() {
  // @@protoc_insertion_point(field_release:CMDString.data)
  
  return data_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void CMDString::set_allocated_data(::std::string* data) {
  if (data != NULL) {
    
  } else {
    
  }
  data_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), data);
  // @@protoc_insertion_point(field_set_allocated:CMDString.data)
}

// -------------------------------------------------------------------

// CMDUInt32

// optional uint32 data = 1;
inline void CMDUInt32::clear_data() {
  data_ = 0u;
}
inline ::google::protobuf::uint32 CMDUInt32::data() const {
  // @@protoc_insertion_point(field_get:CMDUInt32.data)
  return data_;
}
inline void CMDUInt32::set_data(::google::protobuf::uint32 value) {
  
  data_ = value;
  // @@protoc_insertion_point(field_set:CMDUInt32.data)
}

// -------------------------------------------------------------------

// UserInfo_t

// optional uint32 userId = 1;
inline void UserInfo_t::clear_userid() {
  userid_ = 0u;
}
inline ::google::protobuf::uint32 UserInfo_t::userid() const {
  // @@protoc_insertion_point(field_get:UserInfo_t.userId)
  return userid_;
}
inline void UserInfo_t::set_userid(::google::protobuf::uint32 value) {
  
  userid_ = value;
  // @@protoc_insertion_point(field_set:UserInfo_t.userId)
}

// optional string alias = 2;
inline void UserInfo_t::clear_alias() {
  alias_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& UserInfo_t::alias() const {
  // @@protoc_insertion_point(field_get:UserInfo_t.alias)
  return alias_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void UserInfo_t::set_alias(const ::std::string& value) {
  
  alias_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:UserInfo_t.alias)
}
inline void UserInfo_t::set_alias(const char* value) {
  
  alias_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:UserInfo_t.alias)
}
inline void UserInfo_t::set_alias(const char* value, size_t size) {
  
  alias_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:UserInfo_t.alias)
}
inline ::std::string* UserInfo_t::mutable_alias() {
  
  // @@protoc_insertion_point(field_mutable:UserInfo_t.alias)
  return alias_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* UserInfo_t::release_alias() {
  // @@protoc_insertion_point(field_release:UserInfo_t.alias)
  
  return alias_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void UserInfo_t::set_allocated_alias(::std::string* alias) {
  if (alias != NULL) {
    
  } else {
    
  }
  alias_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), alias);
  // @@protoc_insertion_point(field_set_allocated:UserInfo_t.alias)
}

// optional string head = 3;
inline void UserInfo_t::clear_head() {
  head_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& UserInfo_t::head() const {
  // @@protoc_insertion_point(field_get:UserInfo_t.head)
  return head_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void UserInfo_t::set_head(const ::std::string& value) {
  
  head_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:UserInfo_t.head)
}
inline void UserInfo_t::set_head(const char* value) {
  
  head_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:UserInfo_t.head)
}
inline void UserInfo_t::set_head(const char* value, size_t size) {
  
  head_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:UserInfo_t.head)
}
inline ::std::string* UserInfo_t::mutable_head() {
  
  // @@protoc_insertion_point(field_mutable:UserInfo_t.head)
  return head_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* UserInfo_t::release_head() {
  // @@protoc_insertion_point(field_release:UserInfo_t.head)
  
  return head_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void UserInfo_t::set_allocated_head(::std::string* head) {
  if (head != NULL) {
    
  } else {
    
  }
  head_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), head);
  // @@protoc_insertion_point(field_set_allocated:UserInfo_t.head)
}

// optional uint32 roleType = 4;
inline void UserInfo_t::clear_roletype() {
  roletype_ = 0u;
}
inline ::google::protobuf::uint32 UserInfo_t::roletype() const {
  // @@protoc_insertion_point(field_get:UserInfo_t.roleType)
  return roletype_;
}
inline void UserInfo_t::set_roletype(::google::protobuf::uint32 value) {
  
  roletype_ = value;
  // @@protoc_insertion_point(field_set:UserInfo_t.roleType)
}

// optional uint32 authState = 5;
inline void UserInfo_t::clear_authstate() {
  authstate_ = 0u;
}
inline ::google::protobuf::uint32 UserInfo_t::authstate() const {
  // @@protoc_insertion_point(field_get:UserInfo_t.authState)
  return authstate_;
}
inline void UserInfo_t::set_authstate(::google::protobuf::uint32 value) {
  
  authstate_ = value;
  // @@protoc_insertion_point(field_set:UserInfo_t.authState)
}

// optional .e_GenderType gender = 6;
inline void UserInfo_t::clear_gender() {
  gender_ = 0;
}
inline ::e_GenderType UserInfo_t::gender() const {
  // @@protoc_insertion_point(field_get:UserInfo_t.gender)
  return static_cast< ::e_GenderType >(gender_);
}
inline void UserInfo_t::set_gender(::e_GenderType value) {
  
  gender_ = value;
  // @@protoc_insertion_point(field_set:UserInfo_t.gender)
}

// optional uint32 UserType = 7;
inline void UserInfo_t::clear_usertype() {
  usertype_ = 0u;
}
inline ::google::protobuf::uint32 UserInfo_t::usertype() const {
  // @@protoc_insertion_point(field_get:UserInfo_t.UserType)
  return usertype_;
}
inline void UserInfo_t::set_usertype(::google::protobuf::uint32 value) {
  
  usertype_ = value;
  // @@protoc_insertion_point(field_set:UserInfo_t.UserType)
}

// -------------------------------------------------------------------

// CMDErrCode

// optional uint32 errID = 1;
inline void CMDErrCode::clear_errid() {
  errid_ = 0u;
}
inline ::google::protobuf::uint32 CMDErrCode::errid() const {
  // @@protoc_insertion_point(field_get:CMDErrCode.errID)
  return errid_;
}
inline void CMDErrCode::set_errid(::google::protobuf::uint32 value) {
  
  errid_ = value;
  // @@protoc_insertion_point(field_set:CMDErrCode.errID)
}

// optional string errMsg = 2;
inline void CMDErrCode::clear_errmsg() {
  errmsg_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& CMDErrCode::errmsg() const {
  // @@protoc_insertion_point(field_get:CMDErrCode.errMsg)
  return errmsg_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void CMDErrCode::set_errmsg(const ::std::string& value) {
  
  errmsg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:CMDErrCode.errMsg)
}
inline void CMDErrCode::set_errmsg(const char* value) {
  
  errmsg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:CMDErrCode.errMsg)
}
inline void CMDErrCode::set_errmsg(const char* value, size_t size) {
  
  errmsg_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:CMDErrCode.errMsg)
}
inline ::std::string* CMDErrCode::mutable_errmsg() {
  
  // @@protoc_insertion_point(field_mutable:CMDErrCode.errMsg)
  return errmsg_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* CMDErrCode::release_errmsg() {
  // @@protoc_insertion_point(field_release:CMDErrCode.errMsg)
  
  return errmsg_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void CMDErrCode::set_allocated_errmsg(::std::string* errmsg) {
  if (errmsg != NULL) {
    
  } else {
    
  }
  errmsg_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), errmsg);
  // @@protoc_insertion_point(field_set_allocated:CMDErrCode.errMsg)
}

// repeated string paramValue = 3;
inline int CMDErrCode::paramvalue_size() const {
  return paramvalue_.size();
}
inline void CMDErrCode::clear_paramvalue() {
  paramvalue_.Clear();
}
inline const ::std::string& CMDErrCode::paramvalue(int index) const {
  // @@protoc_insertion_point(field_get:CMDErrCode.paramValue)
  return paramvalue_.Get(index);
}
inline ::std::string* CMDErrCode::mutable_paramvalue(int index) {
  // @@protoc_insertion_point(field_mutable:CMDErrCode.paramValue)
  return paramvalue_.Mutable(index);
}
inline void CMDErrCode::set_paramvalue(int index, const ::std::string& value) {
  // @@protoc_insertion_point(field_set:CMDErrCode.paramValue)
  paramvalue_.Mutable(index)->assign(value);
}
inline void CMDErrCode::set_paramvalue(int index, const char* value) {
  paramvalue_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:CMDErrCode.paramValue)
}
inline void CMDErrCode::set_paramvalue(int index, const char* value, size_t size) {
  paramvalue_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:CMDErrCode.paramValue)
}
inline ::std::string* CMDErrCode::add_paramvalue() {
  // @@protoc_insertion_point(field_add_mutable:CMDErrCode.paramValue)
  return paramvalue_.Add();
}
inline void CMDErrCode::add_paramvalue(const ::std::string& value) {
  paramvalue_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:CMDErrCode.paramValue)
}
inline void CMDErrCode::add_paramvalue(const char* value) {
  paramvalue_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:CMDErrCode.paramValue)
}
inline void CMDErrCode::add_paramvalue(const char* value, size_t size) {
  paramvalue_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:CMDErrCode.paramValue)
}
inline const ::google::protobuf::RepeatedPtrField< ::std::string>&
CMDErrCode::paramvalue() const {
  // @@protoc_insertion_point(field_list:CMDErrCode.paramValue)
  return paramvalue_;
}
inline ::google::protobuf::RepeatedPtrField< ::std::string>*
CMDErrCode::mutable_paramvalue() {
  // @@protoc_insertion_point(field_mutable_list:CMDErrCode.paramValue)
  return &paramvalue_;
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::e_GenderType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::e_GenderType>() {
  return ::e_GenderType_descriptor();
}
template <> struct is_proto_enum< ::e_MsgType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::e_MsgType>() {
  return ::e_MsgType_descriptor();
}
template <> struct is_proto_enum< ::e_RemindType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::e_RemindType>() {
  return ::e_RemindType_descriptor();
}

}  // namespace protobuf
}  // namespace google
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_Common_2eproto__INCLUDED