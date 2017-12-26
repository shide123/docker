#ifndef _MWORD_H_INCLUDE
#define _MWORD_H_INCLUDE

#include <stdint.h>

#pragma pack(1)

struct MWORD
{
	enum
	{
		MAXBASE = 0x1FFFFFFF,
	};

	unsigned int m_nBase:30;
	unsigned int m_nMul:2;    //16的m_nMul次方,用来乘以m_nBase以生成更大的数值

	MWORD(){};
	MWORD(int64_t n);
	MWORD(unsigned int nBase, uint32_t dwMul);
	operator int64_t() const { return GetValue();}
	int64_t GetValue() const;
	int64_t GetABSValue() const;		//绝对值
	MWORD operator-=(const MWORD& d);
	MWORD operator-=(const int64_t d);
	MWORD operator+=(const MWORD& d);
	MWORD operator+=(const int64_t d);
	MWORD operator*=(const MWORD& d);
	MWORD operator*=(const int64_t d);
	MWORD operator/=(const MWORD& d);
	MWORD operator/=(const int64_t d);
	int64_t operator+(const MWORD& d) const;
	int64_t operator-(const MWORD& d) const;
	int64_t operator*(const MWORD& d) const;
	int64_t operator/(const MWORD& d) const;
	int64_t operator+(const int d) const;
	int64_t operator-(const int d) const;
	int64_t operator*(const int d) const;
	int64_t operator/(const int d) const;
	MWORD operator=(const MWORD& d);
	MWORD operator=(const int64_t d);
	MWORD operator=(const int d);
	MWORD operator=(const uint32_t d);
	bool operator==(const MWORD& d) const;
	bool operator==(const int64_t d) const;
	bool operator==(const int d) const;
	bool operator!=(const MWORD& d) const;
	bool operator!=(const int64_t d) const;
	bool operator!=(const int d) const;
	bool operator>(int d) const;
	bool operator>(MWORD d) const;
	bool operator<(int d) const;
	bool operator<(MWORD d) const;
	bool IsZero() const {return m_nBase==0;}
	bool IsMinus() const {return m_nBase<0;}
	uint32_t GetMul(){return m_nMul;}
	uint32_t GetBase(){return m_nBase;}
	uint32_t GetRawData();
	void SetRawData(uint32_t dw);
};
#pragma pack()

#endif //_MWORD_H_INCLUDE
