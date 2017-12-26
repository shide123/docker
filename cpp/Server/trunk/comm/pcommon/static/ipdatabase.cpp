
#include "ipdatabase.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif

CCzip*	CCzip::m_instance = NULL;

CCzip * CCzip::GetInstance(const char *filename)
{
	if (!m_instance)
	{
		m_instance = new CCzip();
		m_instance->init(filename);
	}
	return m_instance;
}
CCzip * CCzip::GetInstance()
{
	if (!m_instance)
	{
		m_instance = new CCzip();
	}
	return m_instance;
}

CCzip::CCzip():m_data(NULL),m_ipstart(0),m_ipend(0),m_ipcount(0),m_datasize(0)
{
}

CCzip::~CCzip()
{
	if(m_data)
	{
		free(m_data);
		m_data = NULL;
	}
}


void CCzip::init(const char * fileName)
{
	if (m_data)
	{
		return;
	}

	int fd = open(fileName,O_RDONLY);
	if (fd < 0)
	{
		printf("Open %s file error\n",fileName);
		return;
	}
	//Get file size.
	if ((m_datasize = lseek(fd,0,SEEK_END )) == 0)
	{
		printf("Seek file error");
	}
	m_data = (char*)malloc(m_datasize);
	memset(m_data,0,m_datasize);
	if (!m_data)
	{
		printf("malloc memory error\n");
		return;
	}
	lseek(fd,0,SEEK_SET);
#ifdef WIN32
	if(setmode(fd,_O_BINARY)<0)
	{
		printf("win 设置二进制文件模式失败\n");
		return;
	}

#endif

	//Read all data in.
	char *buf = m_data;
	size_t res = 1;
	while(res != 0)
	{
		res = read(fd,(void*)buf,4096);
		buf += res;

	}
	close(fd);
	unsigned int *data = (unsigned int*)m_data;
	m_ipstart = data[0];
	m_ipend = data[1];
	m_ipcount = ((data[1]-data[0])/RECORD_LENGTH +1);
}

char * CCzip::searchip(const char *ip, char *location)
{
	if (!m_data)
	{
		printf("未初始化\n");
		return NULL;
	}
	//convert the ip string into a 32bit integer..
	unsigned int ipadd = stringIpToInt(ip);
	if (ipadd == 0)
	{
		sprintf(location,"未知IP");
		return location;
	}
	//search the positon
	unsigned int ippos = seekIPpostion(ipadd);
// 	printf("%08X\n",ipadd);

	unsigned int *endip = (unsigned int*)(m_data+ippos);
	if (ipadd > *endip)
	{
		sprintf(location,"未知IP");
		return location;
	}
	//get the area string.
	getareastring(location,ippos);
	return location;
}
char *CCzip::searchip(unsigned int ip, char *location)
{
	if (ip == 0)
	{
		sprintf(location,"未知IP");
		return location;
	}
	//search the positon
	unsigned int ippos = seekIPpostion(ip);
// 	printf("%08X\n",ip);
	unsigned int *endip = (unsigned int*)((char*)m_data+ippos);
	if (ip > *endip)
	{
		sprintf(location,"未知IP");
		return location;
	}
	//get the area string.
	getareastring(location,ippos);
	return location;

}

/**
 * 把ip字符串转换为unsigned long
 * @param char *ipString
 * @return unsigned long ret
 */
unsigned int CCzip::stringIpToInt(const char *ipString)
{
    int num[4];
    char ch =0;
    unsigned int ret = 0;
    if(sscanf(ipString, "%3d.%3d.%3d.%3d%c",&num[0], &num[1], &num[2], &num[3], &ch) != 4
            || num[0] > 0xFF
            || num[1] > 0xFF
            || num[2] > 0xFF
            || num[3] > 0xFF
            || num[0] < 0x00
            || num[1] < 0x00
            || num[2] < 0x00
            || num[3] < 0x00)
    {
		printf("ip地址错误\n");
		return 0;
    }
 
    ret = ret | (num[0] << 24) | (num[1] << 16) | (num[2] << 8) | (num[3]); 

	return ret;
}

unsigned int CCzip::seekIPpostion(unsigned int ip)
{
	unsigned int start = 0L;
	unsigned int end = m_ipcount;  
	unsigned int mid = 0L;
	unsigned int *addr = NULL;
	while(start != end)
	{
		mid = (start + end) / 2;
// 		printf("mid %u",mid);
		addr = (unsigned int*)(m_data+m_ipstart + mid * RECORD_LENGTH);
// 		printf("  ip %u, addr %u\n",ip,*addr);
		if (ip == *addr)
		{
			unsigned int pos = 0;
			char *szb = (char*)(addr + 1);
			pos = *(unsigned int*)szb;
			pos &= 0x00FFFFFF;
// 			printf("pos %u\n",pos);
			return pos;
		}
		else if (end == start +1)
		{
			unsigned int pos = 0;
			char *szb = (char*)(addr + 1);
			pos = *(unsigned int*)szb;
			pos &= 0x00FFFFFF;
// 			printf("pos %u\n",pos);
			return pos;
		}
		else if (ip > *addr)
		{
			start = mid; 
		}
		else
		{
			end = mid;
		}
	}
	return 0;
}

void CCzip::getareastring(char *area,int offset)
{
	char *begin = (char*)m_data + offset + 4;
	char *areapos1,*areapos2;
	if (*begin == AREA_FOLLOWED1)
	{
		unsigned int newpos = 0;
		begin++;
		newpos = *(unsigned int*)begin;
		newpos &= 0x00FFFFFF;

		areapos1 = m_data + newpos;
		//国家
		if (*areapos1 == AREA_FOLLOWED2)
		{
			newpos = 0;
			areapos1++;

			newpos = *(unsigned int*)areapos1;
			newpos &= 0x00FFFFFF;

			areapos2 = m_data + newpos;
			strcpy(area,areapos2);
			strcat(area," ");
			areapos1 += 3;
		}
		else
		{
			strcpy(area,areapos1);
			strcat(area," ");
			areapos1 += strlen(area); //不需要再加1，因为字符串中已经加入一个空格。
		}
		//地区
		if(*areapos1 == AREA_FOLLOWED2)
		{
			newpos = 0;
			areapos1++;
			newpos = *(unsigned int*)areapos1;
			newpos &= 0x00FFFFFF;

			areapos2 = m_data + newpos;
			strcat(area,areapos2);
		}
		else
		{
			strcat(area,areapos1);
		}
		return;
	}
	else if (*begin == AREA_FOLLOWED2)
	{
		//国家
		unsigned int newpos = 0;

		begin++;
		newpos = *(unsigned int*)begin;
		newpos &= 0x00FFFFFF;

		areapos1 = m_data + newpos;
		strcpy(area,areapos1);
		strcat(area," ");
		areapos1 = begin + 3;

		//地区
		if(*areapos1 == AREA_FOLLOWED2)
		{
			newpos = 0;
			areapos1++;
			newpos = *(unsigned int*)areapos1;
			newpos &= 0x00FFFFFF;

			areapos2 = m_data + newpos;
			strcat(area,areapos2);
		}
		else
		{
			strcat(area,areapos1);
		}
		return;
	}
	else
	{
		//国家
		//begin++;
		unsigned int newpos = 0;

		strcpy(area,begin);
		strcat(area," ");
		areapos1 = begin+strlen(area); //不需要再加1，因为字符串中已经加入一个空格。
		//地区
		if(*areapos1 == AREA_FOLLOWED2)
		{
			newpos = 0;
			areapos1++;

			newpos = *(unsigned int*)areapos1;
			newpos &= 0x00FFFFFF;

			areapos2 = m_data + newpos;
			strcat(area,areapos2);
		}
		else
		{
			strcat(area,areapos1);
		}
		return;

	}

}
