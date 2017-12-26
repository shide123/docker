/*
 * Copyright (c) 2013 Pavel Shramov <shramov@mexmat.net>
 *
 * json2pb is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "TransTool.h"

#include <stdio.h>
#include <stdexcept>
#include <errno.h>

#include "CLogThread.h"
#include "bin2ascii.h"

using google::protobuf::MessageFactory;
using google::protobuf::Descriptor;
using google::protobuf::EnumDescriptor;
using google::protobuf::EnumValueDescriptor;
using google::protobuf::Reflection;

class j2pb_error: public std::exception 
{
public:
	j2pb_error(const std::string & e): _error(e) {}
	j2pb_error(const FieldDescriptor * field, const std::string & e) : _error(field->name() + ": " + e) {}
	virtual ~j2pb_error() throw() {};
	virtual const char * what() const throw () { return _error.c_str(); }

	std::string _error;
};

Json::Value CTransTool::_field2json(const Message & msg, const FieldDescriptor * field, size_t index)
{
	if (!field)
	{
		throw j2pb_error("_field2json field input error.field is null.");
	}

	const bool repeated = field->is_repeated();
	const Reflection * ref = msg.GetReflection();
	if (!ref)
	{
		throw j2pb_error("_field2json msg input error.GetReflection in msg is null.");
	}

#define FIELD_CONVERT_JSON(type, ctype, sfunc, afunc) \
	case FieldDescriptor::type: \
		{									\
			ctype value = (repeated)?		\
			ref->afunc(msg, field, index) : \
			ref->sfunc(msg, field);			\
			json = Json::Value(value);		\
			break;							\
		}

	Json::Value json;
	switch (field->cpp_type())
	{
		FIELD_CONVERT_JSON(CPPTYPE_DOUBLE, double, GetDouble, GetRepeatedDouble);
		FIELD_CONVERT_JSON(CPPTYPE_FLOAT, float, GetFloat, GetRepeatedFloat);
		FIELD_CONVERT_JSON(CPPTYPE_INT64, Json::Int64, GetInt64, GetRepeatedInt64);
		FIELD_CONVERT_JSON(CPPTYPE_UINT64, Json::UInt64, GetUInt64, GetRepeatedUInt64);
		FIELD_CONVERT_JSON(CPPTYPE_INT32, Json::Int, GetInt32, GetRepeatedInt32);
		FIELD_CONVERT_JSON(CPPTYPE_UINT32, Json::UInt, GetUInt32, GetRepeatedUInt32);
		FIELD_CONVERT_JSON(CPPTYPE_BOOL, bool, GetBool, GetRepeatedBool);

	case FieldDescriptor::CPPTYPE_STRING: 
		{
			std::string scratch;
			const std::string & value = (repeated)?
				ref->GetRepeatedStringReference(msg, field, index, &scratch):
				ref->GetStringReference(msg, field, &scratch);
			if (field->type() == FieldDescriptor::TYPE_BYTES)
				json = Json::Value(b64_encode(value).c_str());
			else
				json = Json::Value(value.c_str());
			break;
		}
	case FieldDescriptor::CPPTYPE_MESSAGE: 
		{
			const Message & mf = (repeated)?
				ref->GetRepeatedMessage(msg, field, index):
				ref->GetMessage(msg, field);
			_pb2json(mf, json);
			break;
		}
	case FieldDescriptor::CPPTYPE_ENUM: 
		{
			const EnumValueDescriptor * ef = (repeated)?
				ref->GetRepeatedEnum(msg, field, index):
				ref->GetEnum(msg, field);

			json = Json::Value(ef->number());
			break;
		}
	default:
		break;
	}

	if (json.empty())
	{
		//throw j2pb_error(field, "Fail to convert to json");
	}

	return json;
}

bool CTransTool::_json2pb(Json::Value & root, Message & msg)
{

	//Json::FastWriter writer;  
	//std::string sData=writer.write(root);
	//LOG_PRINT(log_info, "forcontent ,maybe data  or msg=%s\n",sData.c_str());


	const Descriptor * d = msg.GetDescriptor();
	const Reflection * ref = msg.GetReflection();
	//LOG_PRINT(log_info, "_json2pb, d: [%p] ref:%p", d, ref);
	if (!d || !ref)
		return false;

	Json::Value::Members mem = root.getMemberNames();  
//	LOG_PRINT(log_info, "_json2pb, memsize:%d",mem.size());
	for (int i = 0; i < mem.size(); i++)  
	{
//		LOG_PRINT(log_info, "_json2pb, begin:%d",mem.size());
		std::string & name = mem[i];
		LOG_PRINT(log_info, "---forcontent,_json2pb,  json column name:%s",name.c_str());
		Json::Value & value = root[name];


		const FieldDescriptor * field = d->FindFieldByName(name);
		if (!field)
			field = ref->FindKnownExtensionByName(name);

		if (!field)
			continue;

		if (field->is_repeated()) 
		{
			LOG_PRINT(log_info, "forcontent ,field->is_repeated\n");
			if (!value.isArray())
			{
				LOG_PRINT(log_error, "field[%s] is repeated, but found type [%d] in json string.", name.c_str(), value.type())
				return false;
			}
			LOG_PRINT(log_info, "forcontent ,field->is_repeated,_json2pb, value:%d",value.size());
			for (int j = 0; j < value.size(); j++)
			{
				_json2field(msg, field, value[j]);
			}
		}
		else
		{
			//LOG_PRINT(log_info, "---forcontent--------\n");
			_json2field(msg, field, value);
		}
//		LOG_PRINT(log_info, "_json2pb, end:%d",mem.size());
	}
    return true;
}

//dele "/" before " " " or "\"
std::string  delSpecChar(std::string &sData )
{
	char buf[4096]={0};
	int j=0;
	for (int i=0;i<sData.size();i++)
	{
		
		if(sData[i]=='\\' && sData[i+1]=='\\')
		{
			buf[j++]=sData[i++];
			continue;
		}
		else if (sData[i]=='\\' && sData[i+1]=='"' )
		{
			continue;
		}
		buf[j++]=sData[i];
	}
	
	return std::string(buf);
	
}

void CTransTool::_json2field(Message & msg, const FieldDescriptor * field, Json::Value & json)
{
	const Reflection * ref = msg.GetReflection();
	if (!ref)
	{
		LOG_PRINT(log_error, "msg GetReflection is null.");
		return;
	}

	const bool repeated = field->is_repeated();
	switch (field->cpp_type())
	{
#define _SET_OR_ADD(sfunc, afunc, value) \
		if (repeated)				\
			ref->afunc(&msg, field, value); \
		else						\
			ref->sfunc(&msg, field, value);

#define JSON_CONVERT_FIELD(type, ctype, asFunc, sfunc, afunc)	\
		case FieldDescriptor::type: \
		{ \
			ctype value; \
			try \
			{ \
				value = json.asFunc(); \
			} \
			catch(std::exception & e) \
			{ \
				throw j2pb_error(field, std::string("Failed to unpack: ") + e.what()); \
			} \
			catch(...) \
			{ \
				throw j2pb_error(field, std::string("Failed to unpack: Unknow execption.")); \
			} \
			_SET_OR_ADD(sfunc, afunc, value); \
			break; \
		}

		JSON_CONVERT_FIELD(CPPTYPE_DOUBLE, double, asDouble, SetDouble, AddDouble);
		JSON_CONVERT_FIELD(CPPTYPE_FLOAT, float, asFloat, SetFloat, AddFloat);
		JSON_CONVERT_FIELD(CPPTYPE_INT64, Json::Int64, asInt64, SetInt64, AddInt64);
		JSON_CONVERT_FIELD(CPPTYPE_UINT64, Json::UInt64, asUInt64, SetUInt64, AddUInt64);
		JSON_CONVERT_FIELD(CPPTYPE_INT32, Json::Int, asInt, SetInt32, AddInt32);
		JSON_CONVERT_FIELD(CPPTYPE_UINT32, Json::UInt, asUInt, SetUInt32, AddUInt32);
		JSON_CONVERT_FIELD(CPPTYPE_BOOL, bool, asBool, SetBool, AddBool);

	case FieldDescriptor::CPPTYPE_STRING:
		{
			if (!json.isString())
			{
				throw j2pb_error(field, "Not a string");
			}

			const char * value = json.asString().c_str();//sometimes  when the content is longer it works  bug

			Json::FastWriter writer2;  
			std::string sColumnData=writer2.write(json);//用这种方式,需要转换到原始样式
			LOG_PRINT(log_info, "forcontent ,sColumnData=%s\n",sColumnData.c_str());
			LOG_PRINT(log_info, "forcontent ,sColumnData.size()=%u\n",sColumnData.size());
			LOG_PRINT(log_info, "forcontent ,sColumnData last char=%02hhx\n",sColumnData[sColumnData.size()-1]); //y the tail of string have a char 0a? 

			//需要去掉前后引号
			std::string delData;
			if (sColumnData.find("\"")!=std::string::npos &&  sColumnData.size()>3 && std::count(sColumnData.begin(),sColumnData.end(),'"')>=2)
			{
				sColumnData=sColumnData.substr(1,sColumnData.size()-3);
				if(std::count(sColumnData.begin(),sColumnData.end(),'"')>0 )
				{
					if(sColumnData.find("{\\\"")!=std::string::npos &&sColumnData.find("\\\"}")!=std::string::npos && 
						sColumnData.find(":")!=std::string::npos)
					{
						sColumnData= delSpecChar(sColumnData);
						//sColumnData= delSpecChar(sColumnData);//maybe have "
				
						LOG_PRINT(log_info, "forcontent ,is json");
					}
					else
					{
						sColumnData= delSpecChar(sColumnData);
					}
				}
				else
				{
					sColumnData= delSpecChar(sColumnData);
				}
				
			}

			LOG_PRINT(log_info, "forcontent ,sColumnData %s\n",sColumnData.c_str());

			//LOG_PRINT(log_info, "---forcontent,value:%s\n",value); print value may be core 

			if (field->type() == FieldDescriptor::TYPE_BYTES)
            {
				LOG_PRINT(log_info, "---forcontent,field type is FieldDescriptor::TYPE_BYTES\n");
				_SET_OR_ADD(SetString, AddString, b64_decode(value));
            }
			else
            {
				_SET_OR_ADD(SetString, AddString, sColumnData.c_str());
            }
			break;
		}
	case FieldDescriptor::CPPTYPE_MESSAGE:
		{
			//LOG_PRINT(log_info, "---forcontent,CPPTYPE_MESSAGE\n");
			Message * mf = (repeated)?
				ref->AddMessage(&msg, field) :
				ref->MutableMessage(&msg, field);
			_json2pb(json, *mf);
			break;
		}
	case FieldDescriptor::CPPTYPE_ENUM: 
		{
			const EnumDescriptor * ed = field->enum_type();
			const EnumValueDescriptor * ev = NULL;
			if (json.isInt()) 
			{
				//LOG_PRINT(log_info, "---forcontent,value:%d\n",json.isInt());
				ev = ed->FindValueByNumber(json.asInt());
			}
			else if (json.isString()) 
			{
				ev = ed->FindValueByName(json.asString());
			}
			else
				throw j2pb_error(field, "Not an integer or string");

			if (!ev)
				throw j2pb_error(field, "Enum value not found");

			_SET_OR_ADD(SetEnum, AddEnum, ev);
			break;
		}
	default:
		break;
	}
}

bool CTransTool::json2pb(Json::Value &root, google::protobuf::Message & msg)
{
	if (root.type() == Json::nullValue)
	{
		LOG_PRINT(log_error, "invalid json type.");
		return false;
	}


	try 
	{
		return _json2pb(root, msg);
	}
	catch (std::exception & e)
	{
		LOG_PRINT(log_error, " error: %s json2pb %s", e.what(),Json::FastWriter().write(root).c_str());
		return false;
	}
	catch (...)
	{
		LOG_PRINT(log_error, "json2pb %s throw unknown error..", Json::FastWriter().write(root).c_str());
		return false;
	}
}

std::string CTransTool::pb2json(const google::protobuf::Message & msg)
{
	std::string strJson;
	Json::Value root;
	if (pb2json(msg, root))
	{
		strJson = Json::FastWriter().write(root);
	}

	return strJson;
}


bool CTransTool::pb2json(const google::protobuf::Message & msg, Json::Value &root)
{
	try
	{
		return _pb2json(msg, root);
	}
	catch (std::exception & e)
	{
		LOG_PRINT(log_error, "pb2json %s error:%s.", msg.DebugString().c_str(), e.what());
	}
	catch (...)
	{
		LOG_PRINT(log_error, "pb2json %s throw unknown error..", msg.DebugString().c_str());
	}

	return false;
}

bool CTransTool::_pb2json(const Message & msg, Json::Value & root)
{
	const Descriptor * d = msg.GetDescriptor();
	const Reflection * ref = msg.GetReflection();
	if (!d || !ref)
		return false;

	std::vector<const FieldDescriptor *> fields;
	ref->ListFields(msg, &fields);

	for (size_t i = 0; i != fields.size(); i++)
	{
		const FieldDescriptor * field = fields[i];
		if (!field)
		{
			continue;
		}

		Json::Value value;
		if(field->is_repeated()) 
		{
			size_t count = ref->FieldSize(msg, field);
			if (!count)
				continue;

			for (int j = 0; j < count; j++)
				value.append(_field2json(msg, field, j));
		}
		else if (ref->HasField(msg, field))
			value = _field2json(msg, field, 0);
		else
			continue;

		const std::string & name = (field->is_extension())? field->full_name() : field->name();
		root[name.c_str()] = value;
	}
	return true;
}
