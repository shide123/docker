#ifndef _CZIP_H_
#define _CZIP_H_

#include <iostream>

#define RECORD_LENGTH	0x07  //ip记录长度
#define AREA_FOLLOWED1	0X01  //模式1
#define AREA_FOLLOWED2	0x02  //模式2

class CCzip
{
public:
	~CCzip();

	static CCzip * GetInstance(const char *filename);
	static CCzip * GetInstance();

	char *searchip(const char *ip, char *location);
	char *searchip(unsigned int ip, char *location);
	void init(const char *filename);

protected:
	CCzip();
	unsigned int stringIpToInt(const char *ipString);
	void getareastring(char *area,int offset);
	unsigned int seekIPpostion(unsigned int ip);

protected:
	char 			*m_data;	//将所有的数据库数据保存在内存中（效率+并行）
	unsigned int	m_ipstart;
	unsigned int	m_ipend;
	unsigned int	m_ipcount;
	unsigned int	m_datasize;

	static CCzip*	m_instance;

};



#endif


