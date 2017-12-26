
#ifndef __YC_COMM_DEFINES_HH__20130906__
#define __YC_COMM_DEFINES_HH__20130906__

//注意: 最大值为2^4= 16,传输时限制
enum {
	RoomMgrType_Null      = 0,    //没有
	RoomMgrType_Fangzhu   = 1,    //房主
	RoomMgrType_FuFangzhu = 2,    //副房主
	RoomMgrType_Guan      = 3,    //正管
	RoomMgrType_LinGuan   = 4,    //临管
	RoomMgrType_Quzhang   = 5,    //区长
	RoomMgrType_Daili    = 6,    //真代理
	RoomMgrType_Quzhang2  = 7,    //大区长
	RoomMgrType_FuQuzhang = 8,    //副区长
	RoomMgrType_FuQuzhang2  = 9,  //大区副长
	RoomMgrType_ZengsongLinGuan   = 10, //某些等级或者消费大户额外附带的管理标志,跟真正的管理和临管部一样,只是具备房间某些操作权限,
	//具体什么操作权限,根据其消费来决定
	RoomMgrType_Quzhang_zhuli = 11, //区长助理
	RoomMgrType_Quzhang_zhuli2 = 12, //大区长助理
};

#endif //__YC_COMM_DEFINES_HH__20130906__
