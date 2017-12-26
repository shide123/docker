#include "JsonCmdConfigMgr.h"
#include "CLogThread.h"
#include "message_comm.h"
#include "message_vchat.h"
#include "utils.h"

//protobuf
#include "LogonSvr.pb.h"
#include "UserMgrSvr.pb.h"
#include "RoomSvr.pb.h"
#include "ChatSvr.pb.h"
#include "ConsumeSvr.pb.h"

CJsonCmdConfigMgr::CJsonCmdConfigMgr(const std::string configpath, const std::string procname)
	:CCmdConfigMgr(configpath, procname)
{
	m_paramtype_size["byte"]	= sizeof(byte);
	m_paramtype_size["char"]	= sizeof(char);
	m_paramtype_size["int8"]	= sizeof(int8);
	m_paramtype_size["uint8"]	= sizeof(uint8);
	m_paramtype_size["int16"]	= sizeof(int16);
	m_paramtype_size["uint16"]	= sizeof(uint16);
	m_paramtype_size["int"]		= sizeof(int32);
	m_paramtype_size["int32"]	= sizeof(int32);
	m_paramtype_size["uint32"]	= sizeof(uint32);
	m_paramtype_size["int64"]	= sizeof(int64);
	m_paramtype_size["uint64"]	= sizeof(uint64);
	m_paramtype_size["float"]	= sizeof(float);
	m_paramtype_size["variable_uint8"] = sizeof(uint8);
	m_paramtype_size["variable_uint16"] = sizeof(uint16);
	m_paramtype_size["variable_uint32"] = sizeof(uint32);
	m_paramtype_size["member_num_uint8"] = sizeof(uint8);
	m_paramtype_size["member_num_uint16"] = sizeof(uint16);
	m_paramtype_size["member_num_uint32"] = sizeof(uint32);

	//for variable_array, the length is variable, not fixed
	m_paramtype_size["variable_char_array"] = 0;
}

CJsonCmdConfigMgr::~CJsonCmdConfigMgr(void)
{
}


void CJsonCmdConfigMgr::loadCmdParamConfig()
{
	if(m_configpath.empty())
	{
		LOG_PRINT(log_debug, "config path is empty.");
		return;
	}

	std::string cmd_param_config = m_configpath + m_strProcName + "_cmdparam.conf";
	LOG_PRINT(log_debug, "config path:%s", cmd_param_config.c_str());

	if(!m_cmd_param.loadfile(cmd_param_config))
	{
		LOG_PRINT(log_error, "load cmd param config error, path:%s", cmd_param_config.c_str());
		return;
	}

	std::string keywordlist = m_cmd_param.getString("server", "keywordlist");
	std::list<std::string> keyword_lst;
	splitStrToLst(keywordlist, ',', keyword_lst);
	if (keyword_lst.empty())
	{
		LOG_PRINT(log_warning, "cmdlist is empty.config path:%s", cmd_param_config.c_str());
		return;
	}
	std::list<std::string>::iterator iter = keyword_lst.begin();
	for (; iter != keyword_lst.end(); ++iter)
	{
		const std::string & keyword = *iter;
		std::string paramlist = m_cmd_param.getString(keyword, "paramlist");
		if (paramlist.empty())
		{
			LOG_PRINT(log_warning, "cmd:%s has no param list.", keyword.c_str());
			continue;
		}

		std::list<std::string> param_lst;
		splitStrToLst(paramlist, ',', param_lst);
		std::list<std::string>::iterator iter_param = param_lst.begin();
		for (; iter_param != param_lst.end(); ++iter_param)
		{
			const std::string & param_name = *iter_param;

			std::map<std::string, PARAM_NAME_TYPE >::iterator iter_map = m_cmd_param_map.find(keyword);
			if (iter_map != m_cmd_param_map.end())
			{
				iter_map->second.push_back(param_name);
			}
			else
			{
				PARAM_NAME_TYPE oParam;
				oParam.push_back(param_name);
				m_cmd_param_map.insert(std::make_pair(keyword, oParam));
			}
		}
	}
}

bool CJsonCmdConfigMgr::fillCmdParam(const std::string & keyword, char * pContent, unsigned int maxlen, const Json::Value & jsonData)
{
	std::map<std::string, PARAM_NAME_TYPE >::iterator iter_map = m_cmd_param_map.find(keyword);
	if (iter_map == m_cmd_param_map.end())
	{
		LOG_PRINT(log_error, "keyword:%s is not in config file.", keyword.c_str());
		return false;
	}

	bool bErr = false;

	char * pEnd = pContent + maxlen;
	char * pInput = pContent;
	unsigned int variable_len = 0;

	try
	{
		const PARAM_NAME_TYPE & params = iter_map->second;
		PARAM_NAME_TYPE::const_iterator iter_param = params.begin();
		for (; iter_param != params.end() && pInput < pEnd; ++iter_param)
		{
			const std::string & param_name = *iter_param;
			const std::string & param_type = m_cmd_param.getString(keyword, param_name);

			std::string data_type;
			int data_len = 0;
			std::vector<std::string> vDataType;
			SplitStringA(param_type.c_str(), ',', vDataType);

			switch(vDataType.size())
			{
			case 2:
				data_len = atoi(vDataType[1].c_str());
			case 1:
				data_type = vDataType[0];
				break;
			default:
				LOG_PRINT(log_error,"param config is wrong, keyword:%s, param_name:%s, param_type:%s", keyword.c_str(), param_name.c_str(), param_type.c_str());
				return false;
			}

			std::map<std::string, unsigned int>::iterator it = m_paramtype_size.find(data_type);
			if (it == m_paramtype_size.end())
			{
				LOG_PRINT(log_error, "data_type was not found, keyword:%s,param_name:%s,param_type:%s", keyword.c_str(), param_name.c_str(), param_type.c_str());
				return false;
			}
			else if (pInput + it->second > pEnd) 
			{
				LOG_PRINT(log_error, "pInput > pEnd,keyword:%s,param_name:%s,param_type:%s", keyword.c_str(), param_name.c_str(), param_type.c_str());
				return false;
			}

			const Json::Value &value = jsonData[param_name];
			if ("uint8" == data_type || "byte" == data_type)
			{
				if(value.isUInt())
				{
					uint8 data = value.asUInt();
					memcpy(pInput, &data, sizeof(uint8));
					pInput += sizeof(uint8);
				}
				else
				{
					bErr = true;
				}
			}
			else if ("int8" == data_type || ("char" == data_type && data_len <= 1))
			{
				if(value.isIntegral())
				{
					int8 data = value.asInt();
					memcpy(pInput, &data, sizeof(int8));
					pInput += sizeof(int8);
				}
				else
				{
					bErr = true;
				}
			}
			else if ("uint32" == data_type)
			{
				if(value.isUInt())
				{
					uint32 data = value.asUInt();
					memcpy(pInput, &data, sizeof(uint32));
					pInput += sizeof(uint32);
				}
				else
				{
					bErr = true;
				}
			}
			else if ("uint64" == data_type)
			{
				if(value.isUInt64())
				{
					uint64 data = value.asUInt64();
					memcpy(pInput, &data, sizeof(uint64));
					pInput += sizeof(uint64);
				}
				else
				{
					bErr = true;
				}
			}
			else if ("int64" == data_type)
			{
				if(value.isInt64())
				{
					int64 data = value.asInt64();
					memcpy(pInput, &data, sizeof(int64));
					pInput += sizeof(int64);
				}
				else
				{
					bErr = true;
				}
			}
			else if ("int32" == data_type || "int" == data_type)
			{
				if(value.isIntegral())
				{
					int32 data = value.asInt();
					memcpy(pInput, &data, sizeof(int32));
					pInput += sizeof(int32);
				}
				else
				{
					bErr = true;
				}
			}
			else if ("int16" == data_type)
			{
				if(value.isIntegral())
				{
					int16 data = value.asInt();
					memcpy(pInput, &data, sizeof(int16));
					pInput += sizeof(int16);
				}
				else
				{
					bErr = true;
				}
			}
			else if ("uint16" == data_type)
			{
				if(value.isUInt())
				{
					uint16 data = value.asUInt();
					memcpy(pInput, &data, sizeof(uint16));
					pInput += sizeof(uint16);
				}
				else
				{
					bErr = true;
				}
			}
			else if ("char" == data_type && data_len > 1)
			{
				if (value.isString())
				{
					if (pInput + data_len > pEnd) 
					{
						LOG_PRINT(log_error, "pInput > pEnd, keyword:%s,param_name:%s,param_type:%s,data_len:%d", keyword.c_str(), param_name.c_str(), data_type.c_str(), data_len);
						return false;
					}
					std::string strdata = value.asString();
					memset(pInput, 0, data_len);
					strncpy(pInput, strdata.c_str(), data_len);
					pInput += data_len;
				}
				else
				{
					bErr = true;
				}
			}
			else if ("variable_uint8" == data_type)
			{
				if(value.isUInt())
				{
					uint8 data = value.asUInt();
					variable_len += data;
					memcpy(pInput, &data, sizeof(uint8));
					pInput += sizeof(uint8);
				}
				else
				{
					bErr = true;
				}
			}
			else if ("variable_uint16" == data_type)
			{
				if(value.isUInt())
				{
					uint16 data = value.asUInt();
					variable_len += data;
					memcpy(pInput, &data, sizeof(uint16));
					pInput += sizeof(uint16);
				}
				else
				{
					bErr = true;
				}
			}
			else if ("variable_uint32" == data_type)
			{
				if(value.isUInt())
				{
					uint32 data = value.asUInt();
					variable_len += data;
					memcpy(pInput, &data, sizeof(uint32));
					pInput += sizeof(uint32);
				}
				else
				{
					bErr = true;
				}
			}
			else if ("variable_char_array" == data_type)
			{
				if (value.isString())
				{
					if (pInput + variable_len > pEnd) 
					{
						LOG_PRINT(log_error, "pInput > pEnd, keyword:%s,param_name:%s,param_type:%s,variable_len:%d", keyword.c_str(), param_name.c_str(), data_type.c_str(), variable_len);
						return false;
					}
					std::string strdata = value.asString();
					memset(pInput, 0, variable_len);
					strncpy(pInput, strdata.c_str(), variable_len);
					pInput += variable_len;
					variable_len  = 0;
				}
				else
				{
					bErr = true;
				}
			}	
			else
			{
				LOG_PRINT(log_error, "Code do not handle this param_type:%s.", param_type.c_str());
				bErr = true;
			}

			if (bErr)
			{
				LOG_PRINT(log_error,"%s, json %s error....", param_name.c_str(), data_type.c_str());
				break;
			}
		}

		if (bErr)
		{
			Json::FastWriter fast_writer;
			std::string logprint = fast_writer.write(jsonData);
			LOG_PRINT(log_error, "Json data format is wrong!Json:%s.", logprint.c_str());
			return false;
		}
	}
	catch (exception &e)
	{
		LOG_PRINT(log_error, "fillCmdParam error: %s", e.what());
		return false;
	}
	catch (...)
	{
		LOG_PRINT(log_error, "fillCmdParam throw unknown error..");
		return false;
	}


	return true;
}

bool CJsonCmdConfigMgr::fillJsonParam(const std::string & keyword, const char ** pBeginContent, const char * pEnd, Json::Value & jsonData)
{
	std::map<std::string, PARAM_NAME_TYPE >::iterator iter_map = m_cmd_param_map.find(keyword);
	if (iter_map == m_cmd_param_map.end())
	{
		LOG_PRINT(log_error, "keyword:%s is not in config file.", keyword.c_str());
		return false;
	}

	const char * pOutput = *pBeginContent;

	unsigned int variable_len = 0;
	unsigned int member_num = 0;

	const PARAM_NAME_TYPE & params = iter_map->second;
	PARAM_NAME_TYPE::const_iterator iter_param = params.begin();
	Json::Value value;
	for (; iter_param != params.end() && pOutput < pEnd; ++iter_param)
	{
		const std::string & param_name = *iter_param;
		const std::string & param_type = m_cmd_param.getString(keyword, param_name);

		std::string data_type;
		int data_len = 0;
		bool bIgnore = false;
		std::vector<std::string> vDataType;
		SplitStringA(param_type.c_str(), ',', vDataType);

        switch(vDataType.size())
        {
        case 3:
            bIgnore = (vDataType[2] == "i");	//±íÃ÷WEB²»ÐèÒª¸ÃÊôÐÔ
        case 2:
            data_len = atoi(vDataType[1].c_str());
        case 1:
            data_type = vDataType[0];
            break;
        default:
            LOG_PRINT(log_error,"param config is wrong, keyword:%s, param_name:%s, param_type:%s", keyword.c_str(), param_name.c_str(), param_type.c_str());
            return false;
        }
        
		if (param_name != "list_member")
		{
            std::map<std::string, unsigned int>::iterator it = m_paramtype_size.find(data_type);
            if (it == m_paramtype_size.end())
            {
                LOG_PRINT(log_error, "data_type was not found, keyword:%s,param_name:%s,param_type:%s", keyword.c_str(), param_name.c_str(), param_type.c_str());
                return false;
            }
            else if (pOutput + it->second > pEnd) 
            {
                LOG_PRINT(log_error, "pInput > pEnd,keyword:%s,param_name:%s,param_type:%s", keyword.c_str(), param_name.c_str(), param_type.c_str());
                return false;
            }
		}
		
		if ("uint8" == data_type || "byte" == data_type)
		{
			uint8 data = 0;
			memcpy(&data, pOutput, sizeof(uint8));
			value = Json::Value(data);
			pOutput += sizeof(uint8);
		}
		else if ("uint32" == data_type)
		{
			uint32 data = 0;
			memcpy(&data, pOutput, sizeof(uint32));
			value = Json::Value(data);
			pOutput += sizeof(uint32);
		}
		else if ("int16" == data_type)
		{
			int16 data = 0;
			memcpy(&data, pOutput, sizeof(int16));
			value = Json::Value(data);
			pOutput += sizeof(int16);
		}
		else if ("int32" == data_type || "int" == data_type)
		{
			int32 data = 0;
			memcpy(&data, pOutput, sizeof(int32));
			value = Json::Value(data);
			pOutput += sizeof(int32);
		}
		else if ("uint16" == data_type)
		{
			uint16 data = 0;
			memcpy(&data, pOutput, sizeof(uint16));
			value = Json::Value(data);
			pOutput += sizeof(uint16);
		}
		else if ("float" == data_type)
		{
			float data = 0.0;
			memcpy(&data, pOutput, sizeof(float));
			value = Json::Value(data);
			pOutput += sizeof(float);
		}
		else if (("char" == data_type && data_len <= 1) || "int8" == data_type)
		{
			int data = 0;
			data = *pOutput;
			value = Json::Value(data);
			pOutput += sizeof(char);
		}
		else if ("char" == data_type && data_len > 1)
		{
			if (pOutput + data_len > pEnd) 
			{
				LOG_PRINT(log_error, "pInput > pEnd, keyword:%s,param_name:%s,param_type:%s,data_len:%d", keyword.c_str(), param_name.c_str(), data_type.c_str(), data_len);
				return false;
			}
			char * data = new char[data_len + 1];
			memset(data, 0, data_len + 1);
			memcpy(data, pOutput, data_len);
 			value = Json::Value(data);
			pOutput += data_len;
			delete []data;
			data = NULL;
		}
		else if ("uint64" == data_type)
		{
			uint64 data = 0;
			memcpy(&data, pOutput, sizeof(uint64));
			std::stringstream strdata;
			strdata << data;
			value = Json::Value(strdata.str());
			pOutput += sizeof(uint64);
		}
		else if ("int64" == data_type)
		{
			int64 data = 0;
			memcpy(&data, pOutput, sizeof(int64));
			std::stringstream strdata;
			strdata << data;
			value = Json::Value(strdata.str());
			pOutput += sizeof(int64);
		}
		else if ("member_num_uint8" == data_type)
		{
			uint8 data = 0;
			memcpy(&data, pOutput, sizeof(uint8));
			member_num = data;
			pOutput += sizeof(uint8);
		}
		else if ("member_num_uint16" == data_type)
		{
			uint16 data = 0;
			memcpy(&data, pOutput, sizeof(uint16));
			member_num = data;
			pOutput += sizeof(uint16);
		}
		else if ("member_num_uint32" == data_type)
		{
			uint32 data = 0;
			memcpy(&data, pOutput, sizeof(uint32));
			member_num = data;
			pOutput += sizeof(uint32);
		}
		else if ("list_member" == param_name)
		{
			//std::string mem_class_name = m_cmd_param.getString(keyword, param_name);
			for (int i = 0; i < member_num; ++i)
			{
				Json::Value jdata;
				if (fillJsonParam(data_type, &pOutput, pEnd, jdata))
				{
					char member[64] = {0};
					sprintf(member, "%u", i+1);
					if (!bIgnore)
						jsonData[member] = jdata;
				}
			}

			//list_member is the last parameter in the msg,so just break here.
			//break;
			member_num = 0;
			continue;
		}
		else if ("variable_uint16" == data_type)
		{
			uint16 data = 0;
			memcpy(&data, pOutput, sizeof(uint16));
			value = Json::Value(data);
			pOutput += sizeof(uint16);

			variable_len += data;
		}
		else if ("variable_uint32" == data_type)
		{
			uint32 data = 0;
			memcpy(&data, pOutput, sizeof(uint32));
			value = Json::Value(data);
			pOutput += sizeof(uint32);

			variable_len += data;
		}
		else if ("variable_char_array" == data_type)
		{
			if (pOutput + variable_len > pEnd) 
			{
				LOG_PRINT(log_error, "pInput > pEnd, keyword:%s,param_name:%s,param_type:%s,variable_len:%d", keyword.c_str(), param_name.c_str(), data_type.c_str(), variable_len);
				return false;
			}
			
			if (variable_len)
			{
				char * data = new char[variable_len + 1];
				if (data != NULL)
				{
					memset(data, 0, variable_len + 1);
					memcpy(data, pOutput, variable_len);
					if (!bIgnore)
						jsonData[param_name.c_str()] = Json::Value(data);
				}
				if (data != NULL)
				{
					delete[] data;
					data = NULL;
				}

				pOutput += variable_len;
				variable_len = 0;
			}
			continue;
		}
		else
		{
			LOG_PRINT(log_error, "date_type was not found, keyword:%s,param_name:%s,param_type:%s", keyword.c_str(), param_name.c_str(), param_type.c_str())
			return false;
		}

		if (!bIgnore)
			jsonData[param_name.c_str()] = value;
	}

	*pBeginContent = pOutput;

	return true;
}


bool CJsonCmdConfigMgr::parse_json_to_msg(const char * pdata, int msglen, SL_ByteBuffer &outMsg)
{
	if (pdata == NULL || msglen == 0)
	{
		LOG_PRINT(log_error, "parse_json_to_msg input error.");
		return false;
	}

	std::string strData(pdata, msglen);
    LOG_PRINT(log_debug, "receive json: %s", strData.c_str())

	try
	{
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;

		if (reader.parse(strData, root))
		{
			unsigned int length = root["length"].asInt();
			unsigned int version = root["version"].asInt();
			unsigned int checkcode = root["checkcode"].asInt();
			unsigned int maincmd = root["maincmd"].asInt();
			unsigned int subcmd = root["subcmd"].asInt();
			Json::Value jsonData = root.get("data", "");

			/*Json::FastWriter writer;  
			string sData=writer.write(jsonData);
			LOG_PRINT(log_info, "sData=%s\n",sData.c_str());*/

			::google::protobuf::Message *pMsg = NULL;

			pMsg = getMsgInstanceByCmd(subcmd);
			if (NULL == pMsg)	//走私有协议
			{
				//LOG_PRINT(log_info, "private proto... ");
				if (!outMsg.reserve(length))
				{
					LOG_PRINT(log_error, "malloc memory failed,subcmd:%u,length:%u.", subcmd, length);
					return false;
				}
				char * pData = outMsg.buffer();
				COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)pData;
				pHead->length = length;
				pHead->version = version;
				pHead->checkcode = checkcode;
				pHead->maincmd = maincmd;
				pHead->subcmd = subcmd;
	
				unsigned int content_len = length - sizeof(COM_MSG_HEADER);
	
				char sub[128] = {0};
				sprintf(sub, "%u", subcmd);
				if (!fillCmdParam(std::string(sub), pHead->content, content_len, jsonData))
				{
					LOG_PRINT(log_error, "fillCmdParam error.");
					return false;
				}
			}
			else	//走protobuf
			{
				//LOG_PRINT(log_info, "going to json2pb... ");
				bool ret = false;
				do
				{
					if (!CTransTool::json2pb(jsonData, *pMsg))
					{
						LOG_PRINT(log_error, "json2pb failed. msg:%s", strData.c_str());
						break;
					}
					int nProtoLen = pMsg->ByteSize();
					int nTotalLen = SIZE_IVM_HEADER + nProtoLen;
					outMsg.clear();
					if (!outMsg.reserve(nTotalLen))
					{
						LOG_PRINT(log_error, "malloc memory failed,subcmd:%u,length:%u.", subcmd, length);
						break;
					}
					COM_MSG_HEADER *pHead = (COM_MSG_HEADER *)outMsg.buffer();
					pHead->length = nTotalLen;
					pHead->version = version;
					pHead->checkcode = checkcode;
					pHead->maincmd = maincmd;
					pHead->subcmd = subcmd;
					pMsg->SerializeToArray(outMsg.buffer() + SIZE_IVM_HEADER, nProtoLen);
					ret = true;
				} while(0);

				delete pMsg;
				if (!ret)
					return false;
			}
		}
		else
		{
			LOG_PRINT(log_error, "parse Json failed.Json:%s.", strData.c_str());
			return false;
		}
	}
	catch (exception &e)
	{
		LOG_PRINT(log_error, "handle_subscribe_cmdlst error: %s", e.what());
		return false;
	}
	catch (...)
	{
		LOG_PRINT(log_error, "handle_subscribe_cmdlst throw unknown error..");
		return false;
	}
	

	return true;
}


bool CJsonCmdConfigMgr::parse_msg_to_json(const char *pdata, unsigned int datalen, SL_ByteBuffer &outBuff)
{
	if (NULL == pdata || 0 == datalen)
	{
		return false;
	}
	Json::Value root;
	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)pdata;
	root["length"] = Json::Value(pOutMsg->length);
	root["version"] = Json::Value(pOutMsg->version);
	root["checkcode"] = Json::Value(pOutMsg->checkcode);
	root["maincmd"] = Json::Value(pOutMsg->maincmd);
	root["subcmd"] = Json::Value(pOutMsg->subcmd);

	if (pOutMsg->length == sizeof(COM_MSG_HEADER))
	{
		root["data"] = Json::Value("");
	}
	else
	{
		::google::protobuf::Message *pMsg = NULL;
		pMsg = getMsgInstanceByCmd(pOutMsg->subcmd);
		if (NULL == pMsg)
		{
			unsigned int contentLen = pOutMsg->length - sizeof(COM_MSG_HEADER);
			const char * pBegin = pOutMsg->content;
			const char * pEnd = pOutMsg->content + contentLen;
			Json::Value jdata;
			char sub[128] = {0};
			sprintf(sub, "%u", pOutMsg->subcmd);
			if (fillJsonParam(std::string(sub), &pBegin, pEnd, jdata))
			{
				if (!jdata.isNull())
				{
					root["data"] = jdata;
				}
			}
			else
			{
				root["data"] = Json::Value("");
			}
		}
		else
		{
			bool ret = false;
			do
			{
				unsigned int contentLen = pOutMsg->length - sizeof(COM_MSG_HEADER);
				if (!pMsg->ParseFromArray(pOutMsg->content, contentLen))
				{
					LOG_PRINT(log_error, "parse [%s] failed.", pMsg->GetTypeName().c_str());
					break;
				}

				Json::Value jdata;
				if (!CTransTool::pb2json(*pMsg, jdata))
				{
					LOG_PRINT(log_error, "json2pb failed. cmd:%d", pOutMsg->subcmd);
					break;
				}

				if (!jdata.isNull())
				{
					root["data"] = jdata;
				}
				else
				{
					root["data"] = Json::Value("");
				}
				ret = true;
			} while(0);
			delete pMsg;
			if (!ret)
				return false;
		}
	}

	Json::FastWriter fast_writer;
	std::string strJRecList = fast_writer.write(root);

	if (Sub_Vchat_ClientPingResp != pOutMsg->subcmd)
	{
		LOG_PRINT(log_debug, "subcmd:%u, response json:%s.", pOutMsg->subcmd, strJRecList.c_str());
	}

	SL_ByteBuffer out(strJRecList.size());
	out.write(strJRecList.c_str(), strJRecList.size());

	unsigned int new_len = calc_new_len(out.data_size());
	//websocket head length max is 10 byte without mask
	outBuff.clear();
	outBuff.reserve(new_len);
	format_write_buf(out.buffer(), out.data_end(), outBuff.buffer());
	outBuff.data_end(new_len);

	return true;
}

unsigned int CJsonCmdConfigMgr::calc_new_len(unsigned int in_len)
{
	if (in_len < 126)
	{
		return in_len + 2;

	}else if (in_len < 0xFFFF) 
	{
		return in_len + 4;
	}
	else
	{
		return in_len + 10;
	}
}


void CJsonCmdConfigMgr::format_write_buf(char * in_data, unsigned long long in_len, char * output)
{
	char * data = output;
	if (in_len < 126)
	{
		memset(data, 0, in_len + 2);
		data[0] = 0x81;
		data[1] = in_len;
		memcpy(data + 2, in_data, in_len);

	}else if (in_len < 0xFFFF) 
	{ 
		memset(data, 0, in_len + 4); 
		data[0] = 0x81; 
		data[1] = 126; 
		data[2] = (in_len >> 8 & 0xFF);
		data[3] = (in_len & 0xFF); 
		memcpy(data + 4, in_data, in_len);
	}
	else
	{
		memset(data, 0, in_len + 10);
		data[0] = 0x81;
		data[1] = 127;
		data[2] = (in_len >> 56 & 0xFF); //7*8
		data[3] = (in_len >> 48 & 0xFF); //6*8
		data[4] = (in_len >> 40 & 0xFF); //5*8
		data[5] = (in_len >> 32 & 0xFF); //4*8
		data[6] = (in_len >> 24 & 0xFF); //3*8
		data[7] = (in_len >> 16 & 0xFF); //2*8
		data[8] = (in_len >>  8 & 0xFF); //1*8
		data[9] = (in_len       & 0xFF);
		memcpy(data + 10, in_data, in_len);
	}
}

::google::protobuf::Message *CJsonCmdConfigMgr::getMsgInstanceByCmd(int subcmd)
{
	::google::protobuf::Message *pMsg = NULL;
#define GET_PROTOBUF_MSG(cmd, msg) \
	case cmd: \
		pMsg = new msg(); \
		break;

	switch(subcmd)
	{
    /*********logonsvr begin**********/
	GET_PROTOBUF_MSG(Sub_Vchat_logonReq5, CMDUserLogonReq)
	GET_PROTOBUF_MSG(Sub_Vchat_logonErr2, CMDUserLogonErrResp)
	GET_PROTOBUF_MSG(Sub_Vchat_logonSuccess2, CMDUserLogonSuccessResp)
    /*********logonsvr end**********/

    /*********roomsvr begin**********/
	GET_PROTOBUF_MSG(Sub_Vchat_JoinRoomReq, CMDJoinRoomReq)
	GET_PROTOBUF_MSG(Sub_Vchat_RoomUserExitReq, CMDUserExitRoomInfo)
	GET_PROTOBUF_MSG(Sub_Vchat_JoinRoomErr, CMDJoinRoomErr)
	GET_PROTOBUF_MSG(Sub_Vchat_JoinRoomResp, CMDJoinRoomResp)
	GET_PROTOBUF_MSG(Sub_Vchat_RoomKickoutUserReq, CMDUserKickoutRoomInfo)
	GET_PROTOBUF_MSG(Sub_Vchat_RoomKickoutUserResp, CMDUserKickoutRoomResp)
	GET_PROTOBUF_MSG(Sub_Vchat_SetRoomInfoReq, CMDRoomInfoReq)
	GET_PROTOBUF_MSG(Sub_Vchat_SetRoomInfoResp, CMDRoomInfoResp)
	GET_PROTOBUF_MSG(Sub_Vchat_RoomInfoNotify, CMDRoomBaseInfo)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupMemberReq, CMDGroupMemberReq)
	GET_PROTOBUF_MSG(Sub_Vchat_QryUserGroupInfoReq, CMDUserGroupInfoReq)
	GET_PROTOBUF_MSG(Sub_Vchat_QryUserGroupInfoResp, CMDUserGroupInfoResp)
	GET_PROTOBUF_MSG(Sub_Vchat_SetGroupMsgMuteReq, CMDSetGroupMsgMuteReq)
	GET_PROTOBUF_MSG(Sub_Vchat_SetGroupMsgMuteResp, CMDSetGroupMsgMuteResp)
	GET_PROTOBUF_MSG(Sub_Vchat_QuitGroupReq, CMDQuitGroupReq)
	GET_PROTOBUF_MSG(Sub_Vchat_QuitGroupResp, CMDQuitGroupResp)
	GET_PROTOBUF_MSG(Sub_Vchat_SetUserPriorityReq, CMDSetUserPriorityReq)
	GET_PROTOBUF_MSG(Sub_Vchat_SetUserPriorityResp, CMDSetUserPriorityResp)
	GET_PROTOBUF_MSG(Sub_Vchat_SetUserPriorityNotify, CMDSetUserPriorityNoty)
	GET_PROTOBUF_MSG(Sub_Vchat_QryGroupVisitCountReq, CMDQryGroupVisitCountReq)
	GET_PROTOBUF_MSG(Sub_Vchat_QryGroupVisitCountResp, CMDQryGroupVisitCountResp)
	GET_PROTOBUF_MSG(Sub_Vchat_JoinGroupReq, CMDJoinGroupReq)
	GET_PROTOBUF_MSG(Sub_Vchat_JoinGroupResp, CMDJoinGroupResp)
	GET_PROTOBUF_MSG(Sub_Vchat_ForbidJoinGroupKillVisitor, CMDForbidJoinGroupKillVisitor)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupSettingStatNotify, CMDGroupSettingStatNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupSettingStatReq, CMDGroupSettingStatReq)
	GET_PROTOBUF_MSG(Sub_Vchat_QryJoinGroupCondReq, CMDQryJoinGroupCondReq)
	GET_PROTOBUF_MSG(Sub_Vchat_QryJoinGroupCondResp, CMDQryJoinGroupCondResp)
	GET_PROTOBUF_MSG(Sub_Vchat_ApplyJoinGroupAuthReq, CMDApplyJoinGroupAuthReq)
	GET_PROTOBUF_MSG(Sub_Vchat_ApplyJoinGroupAuthResp, CMDApplyJoinGroupAuthResp)
	GET_PROTOBUF_MSG(Sub_Vchat_HandleJoinGroupAuthReq, CMDHandleJoinGroupAuthReq)
	GET_PROTOBUF_MSG(Sub_Vchat_HandleJoinGroupAuthResp, CMDHandleJoinGroupAuthResp)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupUserSettingReq, CMDGroupUserSettingReq)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupUserSettingResp, CMDGroupUserSettingResp)
	GET_PROTOBUF_MSG(Sub_Vchat_UserAuthStateNoty, CMDAuthStateNoty)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupOnlineMemberList, CMDGroupOnlineMemberList)
	GET_PROTOBUF_MSG(Sub_Vchat_MemberEnterGroup, CMDMemberEnterGroup)
	GET_PROTOBUF_MSG(Sub_Vchat_MemberExitGroup, CMDMemberExitGroup)
	GET_PROTOBUF_MSG(Sub_Vchat_ForbidUserChat, CMDForbidUserChat)
	GET_PROTOBUF_MSG(Sub_Vchat_CourseFinishReq, CMDCourseFinish)
	GET_PROTOBUF_MSG(Sub_Vchat_ForbidUserChatNoty, CMDForbidUserChat)
	GET_PROTOBUF_MSG(Sub_Vchat_ForbidUserChatRsp, CMDForbidUserChat)
	GET_PROTOBUF_MSG(Sub_Vchat_CourseFinishNoty, CMDCourseFinish)
	//GET_PROTOBUF_MSG(Sub_Vchat_TipsNoty, CMDGiveTip)
	GET_PROTOBUF_MSG(Sub_Vchat_SilenceNoty, CMDSilenceNoty)
	//GET_PROTOBUF_MSG(Sub_Vchat_PointLinkNoty, CMDPointLinkNoty)
	GET_PROTOBUF_MSG(Sub_Vchat_noticeNoty, CMDNoticeNoty)
	//GET_PROTOBUF_MSG(Sub_Vchat_CourseNoty, CMDCourseNoty)
	GET_PROTOBUF_MSG(Sub_Vchat_CloseLiveAndCourseNoty, CMDCloseLiveAndCourseeNoty)
	GET_PROTOBUF_MSG(Sub_Vchat_CourseStartNoty, CMDCourseStartNoty)
    /*********roomsvr end**********/

    /*********consumesvr begin**********/
    GET_PROTOBUF_MSG(Sub_Vchat_SendRedPacketReq, CMDSendRedPacketReq)
    GET_PROTOBUF_MSG(Sub_Vchat_SendRedPacketResp, CMDSendRedPacketResp)
    GET_PROTOBUF_MSG(Sub_Vchat_CatchRedPacketReq, CMDCatchRedPacketReq)
    GET_PROTOBUF_MSG(Sub_Vchat_CatchRedPacketResp, CMDCatchRedPacketResp)
    GET_PROTOBUF_MSG(Sub_Vchat_TakeRedPacketReq, CMDTakeRedPacketReq)
    GET_PROTOBUF_MSG(Sub_Vchat_TakeRedPacketResp, CMDTakeRedPacketResp)
    GET_PROTOBUF_MSG(Sub_Vchat_QryRedPacketInfoReq, CMDQryRedPacketInfoReq)
    GET_PROTOBUF_MSG(Sub_Vchat_RedPacketInfo, CMDRedPacketInfo)
    GET_PROTOBUF_MSG(Sub_Vchat_WeekCharmNotify, CMDWeekListNotify)
    GET_PROTOBUF_MSG(Sub_Vchat_WeekContributeNotify, CMDWeekListNotify)
    GET_PROTOBUF_MSG(Sub_Vchat_QryWeekListReq, CMDQryWeekListReq)
    GET_PROTOBUF_MSG(Sub_Vchat_QryRedPacketReq, CMDQryRedPacketReq)
    GET_PROTOBUF_MSG(Sub_Vchat_QryRedPacketRsp, CMDQryRedPacketRsp)
    GET_PROTOBUF_MSG(Sub_Vchat_SponsorTreasureReq, CMDSponsorTreasureReq)
    GET_PROTOBUF_MSG(Sub_Vchat_SponsorTreasureResp, CMDSponsorTreasureResp)
    GET_PROTOBUF_MSG(Sub_Vchat_BuyTreasure, CMDBuyTreasure)
    GET_PROTOBUF_MSG(Sub_Vchat_BuyTreasureResp, CMDBuyTreasureResp)
    GET_PROTOBUF_MSG(Sub_Vchat_QryMyTreasureDetail, CMDQryMyTreasureDetail)
    GET_PROTOBUF_MSG(Sub_Vchat_MyTreasureDetail, CMDMyTreasureDetail)
    GET_PROTOBUF_MSG(Sub_Vchat_QryGroupTreasureList, CMDQryTreasureList)
    GET_PROTOBUF_MSG(Sub_Vchat_QryMyTreasureList, CMDQryTreasureList)
    GET_PROTOBUF_MSG(Sub_Vchat_GroupTreasureList, CMDTreasureList)
    GET_PROTOBUF_MSG(Sub_Vchat_MyTreasureList, CMDTreasureList)
    GET_PROTOBUF_MSG(Sub_Vchat_TreasureInfoNotify, CMDTreasureInfoNotify)
    GET_PROTOBUF_MSG(Sub_Vchat_QryTreasureInfo, CMDQryTreasureInfo)
    GET_PROTOBUF_MSG(Sub_Vchat_QryTreasureInfoResp, CMDQryTreasureInfoResp)
    /*********consumesvr end**********/

    /*********chatsvr begin**********/
	GET_PROTOBUF_MSG(Sub_Vchat_PrivateMsgReq, CMDPrivateMsgReq)
	GET_PROTOBUF_MSG(Sub_Vchat_PrivateMsgRecv, CMDPrivateMsgRecv)
	GET_PROTOBUF_MSG(Sub_Vchat_PrivateMsgNotify, CMDPrivateMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_PrivateMsgNotifyRecv, CMDPrivateMsgNotifyRecv)
	GET_PROTOBUF_MSG(Sub_Vchat_UnreadPrivateMsgReq, CMDUnreadPrivateMsgReq)
	GET_PROTOBUF_MSG(Sub_Vchat_UnreadPrivateMsgNotify, CMDUnreadPrivateMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_PrivateMsgHisReq, CMDPrivateMsgHisReq)
	GET_PROTOBUF_MSG(Sub_Vchat_PrivateMsgHisResp, CMDPrivateMsgHisResp)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupMsgReq, CMDGroupMsgReq)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupOnlookerChatReq, CMDGroupMsgReq)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupMsgRecv, CMDGroupMsgRecv)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupMsgNotify, CMDGroupMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupOnlookerChatNotify, CMDGroupMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupMsgNotifyRecv, CMDGroupMsgNotifyRecv)
	GET_PROTOBUF_MSG(Sub_Vchat_UnreadGroupMsgReq, CMDUnreadGroupMsgReq)
	GET_PROTOBUF_MSG(Sub_Vchat_UnreadGroupMsgNotify, CMDUnreadGroupMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupMsgHisReq, CMDGroupMsgHisReq)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupMsgHisResp, CMDGroupMsgHisResp)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupUnreadMsgEnd, CMDUInt32)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupPrivateMsgReq, CMDGroupMsgReq)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupPrivateMsgRecv, CMDGroupMsgRecv)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupPrivateMsgNotify, CMDGroupMsgNotify)
//	GET_PROTOBUF_MSG(Sub_Vchat_GroupPrivateMsgReq, CMDGroupPrivateMsgReq)
//	GET_PROTOBUF_MSG(Sub_Vchat_GroupPrivateMsgRecv, CMDGroupPrivateMsgRecv)
//	GET_PROTOBUF_MSG(Sub_Vchat_GroupPrivateMsgNotify, CMDGroupPrivateMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupPrivateMsgNotifyRecv, CMDGroupPrivateMsgNotifyRecv)
	GET_PROTOBUF_MSG(Sub_Vchat_UnreadGroupPrivateMsgNotify, CMDUnreadGroupPrivateMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupPrivateMsgHisReq, CMDGroupMsgHisReq)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupPrivateMsgHisResp, CMDGroupMsgHisResp)
//	GET_PROTOBUF_MSG(Sub_Vchat_GroupPrivateMsgHisReq, CMDGroupPrivateMsgHisReq)
//	GET_PROTOBUF_MSG(Sub_Vchat_GroupPrivateMsgHisResp, CMDGroupPrivateMsgHisResp)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupPChatQualiticationReq, CMDGroupPChatQualiticationReq)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupPChatQualiticationResp, CMDGroupPChatQualiticationResp)
	GET_PROTOBUF_MSG(Sub_Vchat_DeleteChatMsgHis, CMDDeleteChatMsgHis)
	GET_PROTOBUF_MSG(Sub_Vchat_DeleteChatMsgHisResp, CMDDeleteChatMsgHisResp)
	GET_PROTOBUF_MSG(Sub_Vchat_UnreadGPAssistMsgReq, CMDUnreadAssistMsgReq)
	GET_PROTOBUF_MSG(Sub_Vchat_UnreadGPAssistMsgNotify, CMDUnreadAssistMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_GPAssistMsgNotify, CMDGroupAssistMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_GPAssistMsgNotifyRecv, CMDAssistMsgNotifyRecv)
	GET_PROTOBUF_MSG(Sub_Vchat_GPAssistMsgStateReq, CMDGroupAssistMsgStateReq)
	GET_PROTOBUF_MSG(Sub_Vchat_GPAssistMsgStateNotify, MsgState_t)
	GET_PROTOBUF_MSG(Sub_Vchat_GPAssistMsgStateResp, CMDGroupAssistMsgStateResp)
	GET_PROTOBUF_MSG(Sub_Vchat_GPAssistMsgHisReq, CMDGPAssistMsgHisReq)
	GET_PROTOBUF_MSG(Sub_Vchat_GPAssistMsgHisResp, CMDGPAssistMsgHisResp)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupInvestUserReq, CMDGroupInvestUserReq)
	GET_PROTOBUF_MSG(Sub_Vchat_GroupInvestUserResp, CMDGroupInvestUserResp)
    /*********chatsvr end**********/

    /*********pushmsg begin**********/
	GET_PROTOBUF_MSG(Sub_Vchat_SysNoticeMsgNotify, CMDSysNoticeMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_UnreadSysNoticeMsgNotify, CMDUnreadSysNoticeMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_SysNoticeMsgNotifyRecv, CMDNoticeMsgNotifyRecv)
	//GET_PROTOBUF_MSG(Sub_Vchat_UserNoticeMsgNotify, CMDUserNoticeMsgNotify)
//	GET_PROTOBUF_MSG(Sub_Vchat_UnreadUserNoticeMsgNotify, CMDUnreadUserNoticeMsgNotify)
//	GET_PROTOBUF_MSG(Sub_Vchat_UserNoticeMsgNotifyRecv, CMDNoticeMsgNotifyRecv)
	GET_PROTOBUF_MSG(Sub_Vchat_UnreadNoticeMsgReq, CMDUnreadNoticeMsgReq)

	GET_PROTOBUF_MSG(Sub_Vchat_PPTPicChangeNotify, CMDPPTPicChangeMsgNotify)
	GET_PROTOBUF_MSG(Sub_Vchat_LiveStart, CMDLiveStartNoty)
	GET_PROTOBUF_MSG(Sub_Vchat_LiveStop, CMDLiveStopNoty)
    /*********pushmsg end**********/

	default:
		break;
	}

	return pMsg;
}
