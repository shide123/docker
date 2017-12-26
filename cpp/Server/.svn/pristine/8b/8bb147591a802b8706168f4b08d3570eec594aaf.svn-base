/*
 *  Description: This is the core for rommsvr, above the full flow for common.h
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "crc32.h"
#include "md5.h"
#include "utils.h"

#define CHAT_MAX_NUMBER 7
static std::string strKeyword = "0123456789一二三四五六七八九零壹贰叁肆伍陆柒捌玖①②③④⑤⑥⑦⑧⑨㈠㈡㈢㈣㈤㈥㈦㈧㈨⑴⑵⑶⑷⑸⑹⑺⑻⑼。";

std::wstring s2ws(std::string strTemp, bool bGBK = true);

std::string ToHalf(std::string str);
std::string ToHalf_UTF8(std::string input);
bool getFigure(std::string strContent, bool bGBK = true);
size_t getFigureUTF8(const std::string &input, size_t max = 0);
size_t getFigureGBK(const std::string &input, size_t max = 0);
bool filterKeyword(std::string content, bool bGBK = true); 
string pickupchatcontent(const char *content,int len);
int uploadfile(const char* qiniuUrl, const char* filename, const char* localPath, bool bfop = false);
int fopConvert(const char* key,const char* saveaskey);
char* getUpLoadToken(const char* qiniuUrl, bool bfop = false);
#endif 
