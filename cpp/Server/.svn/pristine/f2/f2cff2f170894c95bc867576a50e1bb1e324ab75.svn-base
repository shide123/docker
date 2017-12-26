#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd_error.h"

static const struct  
{
	int errcode;
	const char* msg;
}err_str[] = 
{
	BUILD_ERROR(101,"帐号被封！"),
	BUILD_ERROR(103,"帐号不存在！"),
	BUILD_ERROR(104,"帐号密码错误！"),
	BUILD_ERROR(201,"赠送用户不存在！"),
	BUILD_ERROR(202,"赠送用户钱不够！"),
	BUILD_ERROR(203,"接受用户不存在！"),
	BUILD_ERROR(204,"赠送礼物未知错误！"),

	BUILD_ERROR(602,"协议版本错误！")
};

static const struct 
{
	int reasonid;
    const char* msg;
}violation_str[] = 
{
	BUILD_ERROR(1,"查询数据库错误！"),

	BUILD_ERROR(602,"协议版本错误！")
};

const char * get_error_str(int errid)
{
	size_t i;
	for(i = 0; i < sizeof(err_str)/sizeof(err_str[0]); i++)
	{
		if (err_str[i].errcode == errid)
		{
			return err_str[i].msg;
		}
	}
	return 0;
}

const char* get_violation_str(int reasonid)
{
	size_t i;
	for(i=0; i<sizeof(violation_str)/sizeof(violation_str[0]); i++)
	{
		if(violation_str[i].reasonid == reasonid)
		{
			return violation_str[i].msg;
		}
	}
	return 0;
}


