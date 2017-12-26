/*********************************************************************
 *  Copyright 2016 by 99letou.
 *  All right reserved.
 *
 *  功能：通用文件
 *
 *  Edit History:
 *
 *    2016/04/13 - Created by baoshengjun to print output in the new form. 
 */

#include "common.h" 
extern "C" {

#include "qiniu/io.h"
#include "qiniu/resumable_io.h"
#include "qiniu/rs.h"
#include "qiniu/fop.h"
#include "qiniu/base.h"
}
static bool qiniu_inited = false;
static Qiniu_Mac    qiniu_mac;
static const char* bucketName = "qiaoliaohd";
std::wstring s2ws(std::string strTemp, bool bGBK/* = true*/)
{
	setlocale(LC_ALL, bGBK ? "zh_CN.gbk": "zh_CN.UTF-8");
	size_t size = strTemp.length() + 1;
	wchar_t *Dest = new wchar_t[size];
	wmemset(Dest, 0, size);
	mbstowcs(Dest, strTemp.c_str(), size);
	std::wstring result = Dest;
	delete[] Dest;
	setlocale(LC_ALL,"c");
	return result;
}

//全角转半角
std::string ToHalf(std::string str) 
{   
	std::string result = "";   
	unsigned char tmp; unsigned char tmp1;   
	for (unsigned int i = 0; i < str.length(); i++) 
	{   
		tmp = str[i];   
		tmp1 = str[i + 1];
		///第一个字节是163，标志着是全角字符  
		if (tmp == 163) 
		{ 
			result += (unsigned char) str[i + 1] - 128;   
			i++;   
			continue;   
		}
		//汉字
		else if (tmp > 163) 
		{   
			result += str.substr(i, 2);   
			i++;   
			continue;   
		} 
		///处理全角空格 
		else if (tmp == 161 && tmp1 == 161) 
		{  
			result += "";   
			i++;   
		} 
		else 
		{   
			result += str.substr(i, 1); 
		} 
	} 
	return result;   
}

std::string ToHalf_UTF8(std::string input) {
	std::string temp;
	for (size_t i = 0; i < input.size(); i++) 
	{
		if (((input[i] & 0xF0) ^ 0xE0) == 0) 
		{
			int old_char = (input[i] & 0xF) << 12 | ((input[i + 1] & 0x3F) << 6 | (input[i + 2] & 0x3F));
			if (old_char == 0x3000) // blank
			{
				char new_char = 0x20;
				temp += new_char;
			} 
			else if (old_char >= 0xFF01 && old_char <= 0xFF5E) // full char
			{
				char new_char = old_char - 0xFEE0;
				temp += new_char;
			} 
			else // other 3 bytes char
			{
				temp += input[i];
				temp += input[i + 1];
				temp += input[i + 2];
			}
			i = i + 2;
		} 
		else 
		{
			temp += input[i];
		}
	}
	return temp;
}

bool getFigure(std::string strContent, bool bGBK/* = true*/)
{
	return (bGBK ? getFigureGBK(strContent, CHAT_MAX_NUMBER) : getFigureUTF8(strContent, CHAT_MAX_NUMBER)) >= CHAT_MAX_NUMBER;
}

//过滤关键字
bool filterKeyword(std::string content, bool bGBK/* = true*/)
{	
	std::string strContent(bGBK ? ToHalf(content) : ToHalf_UTF8(content));
	LOG_PRINT(log_debug, "after ToHalf content from strContent:%s", strContent.c_str());
	return getFigure(strContent, bGBK);
} 

//pick-up chat message content
string pickupchatcontent(const char *content,int len)
{
	std::string strContent = "";
	for (int i = 0; i < len; i++)
	{
		//提取聊天信息
		if ((content[i] == '>') && (i < len - 1) && (content[i + 1] != '<'))
		{
			for (int j = i + 1; j < len && content[j] != '<'; j++)
			{
				strContent += content[j];
			}
			break;
		}
	}
	
	return strContent;
} 

size_t getFigureUTF8(const std::string &input, size_t max /*= 0*/)
{
	size_t count = 0;
	for (size_t i = 0, len = 0; i < input.length(); i += len) 
	{
		unsigned char byte = (unsigned)input[i];
		if (byte >= 0xFC) // lenght 6
			len = 6;  
		else if (byte >= 0xF8)
			len = 5;
		else if (byte >= 0xF0)
			len = 4;
		else if (byte >= 0xE0)
			len = 3;
		else if (byte >= 0xC0)
			len = 2;
		else
			len = 1;

		if (strKeyword.find(input.substr(i, len)) != std::string::npos)
			count++;

		if (max && count >= max)
			break;
	}

	return count;
}

size_t getFigureGBK(const std::string &input, size_t max /*= 0*/)
{
	size_t count = 0;
	for (size_t i = 0, len = 0; i < input.length(); i += len) 
	{
		unsigned char byte = (unsigned)input[i];
		if ((byte & 0x80) != 0)
			len = 2;
		else
			len = 1;

		if (strKeyword.find(input.substr(i, len)) != std::string::npos)
			count++;

		if (max && count >= max)
			break;
	}

	return count;
}
char* getUpLoadToken(const char* qiniuUrl, bool bfop/* = false*/)
{
  Qiniu_RS_PutPolicy putPolicy;
  Qiniu_Zero(putPolicy);
  putPolicy.scope = bucketName;
  if(bfop)
  {
	  putPolicy.persistentNotifyUrl = qiniuUrl;//"http://test.talk.99cj.com.cn/QiniuNotify/index";//"http://wechat.99cj.com.cn/QiniuNotify/index";//
	  putPolicy.persistentOps = "avthumb/aac";
	  putPolicy.persistentPipeline = "bndytestts";
  }
  return Qiniu_RS_PutPolicy_Token(&putPolicy, &qiniu_mac);
}

void QiniuInit()
{
  const char* bucketName = "qiaoliaohd";
  qiniu_mac.accessKey = "-A_E2mEZ14NOTYAq9dSR-Uhg4tAtT-HVJWX1m4m2";//argv[1];
  qiniu_mac.secretKey = "AgNMfRQT12HrKoSOKKwsYe8N49TBlvaTNe9Pjtjh";//argv[2];

  if (!qiniu_inited)
  {
    qiniu_inited = true;
    Qiniu_Servend_Init(-1);
  }
}
int uploadfile(const char* qiniuUrl, const char* filename, const char* localPath, bool bfop/* = false*/)
{
	QiniuInit();
	int ret = 0;
	Qiniu_Client client;

	Qiniu_Client_InitMacAuth(&client, 1024, &qiniu_mac);

	const char* uploadtoken = getUpLoadToken(qiniuUrl, bfop);
	Qiniu_Io_PutRet putRet;
	Qiniu_Error error = Qiniu_Io_PutFile(&client, &putRet, uploadtoken, filename, localPath, NULL);
	if (error.code != 200)
	{
		LOG_PRINT(log_debug, "Upload File %s To %s:%s : token:%s code:%d err:%s error.\n", localPath, bucketName, filename, uploadtoken, error.code, error.message);
		ret = 1;
	}
	else
	{
		LOG_PRINT(log_debug,"Upload apk File ok.");
	}

	Qiniu_Client_Cleanup(&client);
	return ret;
}
int fopConvert(const char* key,const char* saveaskey)
{
	QiniuInit();
	int ret = 0;
	Qiniu_Client client;

	Qiniu_Client_InitMacAuth(&client, 1024, &qiniu_mac);

	Qiniu_FOP_PfopArgs args ;
	args.bucket = bucketName;
	args.force = 1;
	args.key = key;
	args.notifyURL = "http://wechat.99cj.com.cn/QiniuNotify/index";
	args.pipeline = "bndytestts";
	Qiniu_FOP_PfopRet putRet;
	char* fop[1];
	fop[0] = (char*)stringFormat("avthumb/aac;saveas/%s",saveaskey).c_str();
//	fop[0] = "avthumb/avi/vb/100k";
	Qiniu_Error error = Qiniu_FOP_Pfop(&client, &putRet, &args,  fop, 1);

	if (error.code != 200)
	{
		LOG_PRINT(log_debug,"fopConvert key %s  error code:%d.msg:%s,args.bucket:%s,args.force:%d,args.key:%s,args.notifyurl:%s,args.pipiline:%s\n",
				key,error.code,error.message,args.bucket,args.force,args.key,args.notifyURL,args.pipeline);
		ret = 1;
	}
	else
	{
		if(putRet.persistentId)
		{
			LOG_PRINT(log_debug,"fopConvert apk File:%s ok.",putRet.persistentId);
		}
		else
		{
			LOG_PRINT(log_debug,"fopConvert error File:%s ok.",putRet.persistentId);
		}

	}

	Qiniu_Client_Cleanup(&client);
	return ret;
}
