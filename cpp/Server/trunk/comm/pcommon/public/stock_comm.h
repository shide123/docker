#ifndef __STOCK_COMM_H__
#define __STOCK_COMM_H__

#include <stdint.h>
#include <set>
#include <map>
#include <string>
#include "mword.h"

#pragma pack(1)

enum
{
	CLIENTTYPE_ALIVE			= 33000,	//!<33000, 心跳
	CLIENTTYPE_STK_STOCKNOTICE	= 33350,
	CLIENTTYPE_STK_DYNA_DIFF	= 33351,	//!<动态数据差异信息
	CLIENTTYPE_STK_ATTENTION	= 33352,	//!<需要预警的股票列表
};

struct CMDHEAD
{
	uint16_t m_wCmdType;	//request type
	uint16_t m_wAttr;		//property
	uint32_t m_nLen;		//pakcet len
	uint32_t m_nExpandInfo;
	char m_pData[0];		//Data
};

//动态数据变更类型
enum DynaDiffType
{
	DYNADIFFTYPE_TURNOVERRATE		= 0,	//换手率
	DYNADIFFTYPE_PRICE_RISE			= 1,	//价格涨
	DYNADIFFTYPE_PRICE_DROP			= 2,	//价格跌
	DYNADIFFTYPE_DAYRANGE_RISE		= 3,	//日涨幅
	DYNADIFFTYPE_DAYRANGE_DROP		= 4,	//日跌幅
	DYNADIFFTYPE_ONEAMOUNT_BUY		= 5,	//单笔成交(买入)
	DYNADIFFTYPE_ONEAMOUNT_SELL		= 6,	//单笔成交(卖出)
	DYNADIFFTYPE_FASTCHANGE_RISE	= 7,	//走势异动(拉伸)
	DYNADIFFTYPE_FASTCHANGE_DROP	= 8		//走势异动(打击)
};

//个人预警开关（位操作）
enum STOCK_WARNING_SWITCH
{
	WARNING_SWITCH_TURNOVERRATE 	= 0x0001,	//换手率
	WARNING_SWITCH_HIGHPRICE		= 0x0002,	//高价阈值
	WARNING_SWITCH_LOWPRICE			= 0x0004,	//低价阈值
	WARNING_SWITCH_DAYRISE			= 0x0008,	//日涨幅
	WARNING_SWITCH_DAYDROP			= 0x0010,	//日跌幅
	WARNING_SWITCH_LARGEBUY			= 0x0020,	//大笔买入
	WARNING_SWITCH_LARGESELL		= 0x0040,	//大笔卖出
	WARNING_SWITCH_FASTRISE			= 0x0080,	//急速拉升
	WARNING_SWITCH_FASTDROP			= 0x0100,	//猛烈打压
};

enum eRadarWarnType
{
	RADAR_ASTOCK				= 1,	//A股
	RADAR_SELECTSTOCK			= 2,	//自选股
	//RADAR_BOARD 板块
	RADAR_INDUSTRY				= 3,	//行业
	RADAR_CONCEPT				= 4,	//概念
	RADAR_AREA					= 5		//地区
};

enum eRadarNotyType
{
	RADAR_NOTF_ASTOCK			= 1,	//A股
	RADAR_NOTF_SELECTSTOCK		= 2,	//自选股
	RADAR_NOTF_BOARD			= 3		//板块
};

struct CMDStockDynaDiff
{
	uint32_t	m_timestamp;
	uint8_t		m_type;
	char		m_szStkCode[32];
	MWORD		m_mPreValue;
	MWORD		m_mCurValue;
};

typedef std::set<unsigned int> USERID_SET;
//key:threshold   value setting, value:userid set. 这里的阈值是客户端设置的值*10000
typedef std::map<unsigned int, USERID_SET > THRESHOLD_SET_MAP;
//key:stockcode   value:THRESHOLD_SET_MAP
typedef std::map<std::string, THRESHOLD_SET_MAP > STOCK_THRESHOLD_MAP;
//key:DynaDiffType  value:threshold value. 这里的阈值是客户端设置的值*10000
typedef std::map<DynaDiffType, unsigned int> DYNA_THRESHOLD_MAP;

struct StockCode_t
{
	char m_szCode[32];
};

struct CMDStockAttention
{
	enum AttentionType
	{
		ATTENTION_INITIALIZE = 0,	//初始化列表
		ATTENTION_ADD,				//新增列表
		ATTENTION_DEL,				//删除列表
	};
	uint8_t		m_type;
	uint16_t	m_nNum;
	StockCode_t m_pList[0];
};

#pragma pack()


#endif //__STOCK_COMM_H__