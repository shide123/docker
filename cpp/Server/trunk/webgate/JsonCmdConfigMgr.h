#ifndef __JSONCMDCONFIGMGR_H__
#define __JSONCMDCONFIGMGR_H__

#include "CCmdConfigMgr.h"
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "json/json.h"
#include "TransTool.h"
#include "SL_ByteBuffer.h"
typedef std::vector<std::string> PARAM_NAME_TYPE;

class CJsonCmdConfigMgr :
	public CCmdConfigMgr
{
public:
	CJsonCmdConfigMgr(const std::string configpath, const std::string procname);
	~CJsonCmdConfigMgr(void);


	void loadCmdParamConfig();
	//json转tcp协议
	bool parse_json_to_msg(const char * pdata, int msglen, SL_ByteBuffer &outMsg);
	//tcp协议转json
	bool parse_msg_to_json(const char *pdata, unsigned int datalen, SL_ByteBuffer &outBuff);
private:
	bool fillCmdParam(const std::string & keyword, char * pContent, unsigned int maxlen, const Json::Value & jsonData);
	bool fillJsonParam(const std::string & keyword, const char ** pBeginContent, const char * pEnd, Json::Value & jsonData);
	unsigned int calc_new_len(unsigned int in_len);
	void format_write_buf(char * in_data, unsigned long long in_len, char * output);
	::google::protobuf::Message *getMsgInstanceByCmd(int subcmd);

	std::map<std::string, PARAM_NAME_TYPE > m_cmd_param_map;

	Config m_cmd_param;

	std::map<std::string, unsigned int> m_paramtype_size;
};

#endif //__JSONCMDCONFIGMGR_H__

