/*
 * Copyright (c) 2013 Pavel Shramov <shramov@mexmat.net>
 *
 * json2pb is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __JSON2PB_H__
#define __JSON2PB_H__

#include <string>
#include <json/json.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

using google::protobuf::Message;
using google::protobuf::FieldDescriptor;

class CTransTool
{
public:
	static bool json2pb(Json::Value &root, Message & msg);
	static std::string pb2json(const Message & msg);
	static bool pb2json(const Message & msg, Json::Value &root);

private:
	static bool _pb2json(const Message &msg, Json::Value &root);
	static Json::Value _field2json(const Message & msg, const FieldDescriptor * field, size_t index);

	static bool _json2pb(Json::Value &root, Message &msg);
	static void _json2field(Message &msg, const FieldDescriptor *field, Json::Value &json);
};

#endif//__JSON2PB_H__
