//#include "stdafx.h"
#include "mword.h"

MWORD::MWORD(int64_t n)
{
	int64_t d = n;
	int nInc = 0;
	m_nMul = 0;
	while(d+nInc>MAXBASE)
	{
		nInc = (d%16)>=8;	//四舍五入
		d /= 16;
		m_nMul++;
		if(m_nMul==3)
			break;
	}
	m_nBase = (unsigned int)(d+nInc);
}
	
MWORD::MWORD(unsigned int nBase,uint32_t dwMul)
{
	m_nBase = nBase;
	m_nMul = dwMul;
}

int64_t MWORD::GetValue() const
{
	int64_t n = m_nBase;
	for(uint32_t i=0;i<m_nMul;i++)
		n *= 16;
	return n;
}

int64_t MWORD::GetABSValue() const 
{
	return GetValue();
}

MWORD MWORD::operator-=(const MWORD& d)
{
	int64_t n = d;
	*this -= n;
	return *this;
}

MWORD MWORD::operator-=(const int64_t d)
{
	int64_t n = *this;
	n -= d;
	*this = n;
	return *this;
}

MWORD MWORD::operator+=(const MWORD& d)
{
	int64_t n = d;
	*this += n;
	return *this;
}

MWORD MWORD::operator+=(const int64_t d)
{
	int64_t n = *this;
	n += d;
	*this = n;
	return *this;
}

MWORD MWORD::operator*=(const MWORD& d)
{
	int64_t n = d;
	*this *= n;
	return *this;
}

MWORD MWORD::operator*=(const int64_t d)
{
	int64_t n = *this;
	n *= d;
	*this = n;
	return *this;
}

MWORD MWORD::operator/=(const MWORD& d)
{
	int64_t n = d;
	*this /= n;
	return *this;
}

MWORD MWORD::operator/=(const int64_t d)
{
	int64_t n = *this;
	n /= d;
	*this = n;
	return *this;
}

int64_t MWORD::operator+(const MWORD& d) const
{
	int64_t n = *this;
	int64_t m = d;
	return n+m;
}

int64_t MWORD::operator-(const MWORD& d) const
{
	int64_t n = *this;
	int64_t m = d;
	return n-m;
}

int64_t MWORD::operator*(const MWORD& d) const
{
	int64_t n = *this;
	int64_t m = d;
	return n*m;
}

int64_t MWORD::operator/(const MWORD& d) const
{
	int64_t n = *this;
	int64_t m = d;
	return n/m;
}

int64_t MWORD::operator+(const int d) const
{
	int64_t n = *this;
	return n+d;
}

int64_t MWORD::operator-(const int d) const
{
	int64_t n = *this;
	return n-d;
}

int64_t MWORD::operator*(const int d) const
{
	int64_t n = *this;
	return n*d;
}

int64_t MWORD::operator/(const int d) const
{
	int64_t n = *this;
	return n/d;
}

MWORD MWORD::operator=(const MWORD& d)
{
	m_nMul = d.m_nMul;
	m_nBase = d.m_nBase;
	return *this;
}

MWORD MWORD::operator=(const int64_t n)
{
	int64_t d = n;
	int nInc = 0;
	m_nMul = 0;
	while(d+nInc>MAXBASE || d+nInc<-MAXBASE)
	{
		nInc = (d%16)>=8;	//四舍五入
		d /= 16;
		m_nMul++;
		if(m_nMul==3)
			break;
	}
	m_nBase = (int)(d+nInc);
	return *this;
}

MWORD MWORD::operator=(int n)
{
	m_nMul = 0;
	while(n>MAXBASE || n<-MAXBASE)
	{
		m_nMul++;
		n /= 16;
	}
	m_nBase = n;
	return *this;
}

MWORD MWORD::operator=(uint32_t n)
{
	m_nMul = 0;
	while(n>MAXBASE)
	{
		m_nMul++;
		n /= 16;
	}
	m_nBase = n;
	return *this;
}
	
bool MWORD::operator==(const MWORD& d) const
{
	int64_t n = *this;
	int64_t m = d;
	return n==m;
}

bool MWORD::operator==(const int64_t d) const
{
	int64_t n = *this;
	return n==d;
}

bool MWORD::operator==(const int d) const
{
	int64_t n = *this;
	return n==d;
}

bool MWORD::operator!=(const MWORD& d) const
{
	int64_t n = *this;
	int64_t m = d;
	return n!=m;
}

bool MWORD::operator!=(const int64_t d) const
{
	int64_t n = *this;
	return n!=d;
}

bool MWORD::operator!=(const int d) const
{
	int64_t n = *this;
	return n!=d;
}

bool MWORD::operator>(int d) const
{
	int64_t n = *this;
	return n>d;
}

bool MWORD::operator>(MWORD d) const
{
	int64_t n = *this;
	int64_t m = d;
	return n>m;
}

bool MWORD::operator<(int d) const
{
	int64_t n = *this;
	return n<d;
}

bool MWORD::operator<(MWORD d) const
{
	int64_t n = *this;
	int64_t m = d;
	return n<m;
}

uint32_t MWORD::GetRawData()
{
	return *(uint32_t*)this;
}

void MWORD::SetRawData(uint32_t dw)
{
	uint32_t* p = (uint32_t*)this;
	*p = dw;
}
