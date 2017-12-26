#ifndef _ipdb99cj_H_
#define _ipdb99cj_H_

#include <string>
#include <algorithm>
using namespace std;


#define IPDB99CJRECORD_LENGTH 7
#define IPDB99CJNAME "ipdb99cj151120.dat"
char* getarea(const char* ip);

#define citynum 298
typedef struct TYcity
{
	char des[16];
	char code[8];
	int sfindex;
	double x;
	double y;
	bool valid;
	int ispindex;
}TYcity;

class ipdb99cj
{
public:
	static ipdb99cj* GetInstance(const char *filename)
	{
		if(!m_instance)
		{
			m_instance = new ipdb99cj();
			if(!m_instance->init(filename))
			{
				delete m_instance;
				m_instance = NULL;
			}
		}
		return m_instance;
	}
	static ipdb99cj* GetInstance()
	{
		return m_instance;
	}
	void searchip(unsigned int ip, unsigned short& areaindex, unsigned char& ispindex)
	{
		unsigned int start = 0L; //[
		unsigned int end = m_ipcount; //)
		unsigned int mid = 0L;
		unsigned int *addr = NULL;
		while(start != end)
		{
			mid = (start + end) / 2;
			addr = (unsigned int*)(m_data + mid * IPDB99CJRECORD_LENGTH);
			if(ip == *addr || end == start +1)
			{
				areaindex = *(unsigned short*)(addr + 1);
				ispindex = *((unsigned char*)addr + 6);
				return;
			}
			else if(ip > *addr)
			{
				start = mid;
			}
			else // <
			{
				end = mid;
			}
		}
		printf("searchip err %u\n", ip);
		areaindex = 0; ispindex = 0;
	}
	void searchip(const char *ipString, unsigned short& areaindex, unsigned char& ispindex)
	{
		searchip(stringIpToInt(ipString), areaindex, ispindex);
	}
protected:
	char 			*m_data;	//将所有的数据库数据保存在内存中（效率+并行）
	unsigned int	m_ipcount;
	static ipdb99cj*	m_instance;
	ipdb99cj() :m_data(NULL), m_ipcount(0){}
	~ipdb99cj()	{if(m_data){free(m_data);m_data = NULL;}}
	bool init(const char * fileName)
	{
		if(m_data) return true;
		FILE* fp = fopen(fileName, "rb");
		if(!fp)
		{
			printf("Open %s file error\n", fileName);
			return false;
		}
		//Get file size.
		if(fseek(fp, 0, SEEK_END))
		{
			printf("Seek file error");
			fclose(fp);
			return false;
		}
		unsigned int m_datasize = ftell(fp);
		m_data = (char*)malloc(m_datasize);
		if(!m_data)
		{
			printf("malloc memory error\n");
			fclose(fp);
			return false;
		}
		if(fseek(fp, 0, SEEK_SET))
		{
			printf("Seek file error");
			fclose(fp);
			return false;
		}
		//Read all data in.
		if (m_datasize != fread(m_data,1,m_datasize,fp))
		{
			printf("fread file error");
			fclose(fp);
			return false;
		}
		fclose(fp);
		m_ipcount = m_datasize/IPDB99CJRECORD_LENGTH;
		return true;
	}
	unsigned int stringIpToInt(const char *ipString)
	{
		int num[4];
		char ch = 0;
		unsigned int ret = 0;
		if(sscanf(ipString, "%3d.%3d.%3d.%3d%c", &num[0], &num[1], &num[2], &num[3], &ch) != 4
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
};


class ipdbInitwork
{
public:
	ipdbInitwork();
	void updatenearestcity();
};

bool less_second(const TYcity* m1, const TYcity* m2);


#endif //_ipdb99cj_H_
