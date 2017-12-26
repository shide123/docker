# TConsumeSvr.thrift

############################################################################

service TConsumeSvr 
{
	#修改贡献榜开关
	i32 proc_modContributionListSwitch(1:i32 runid, 2:i32 groupid, 3: bool Switch),
	#修改群收入分成比例
	i32 proc_modGroupGainSetting(1:i32 runid, 2:i32 groupid, 3:bool Switch, 4:byte rangeType, 5: byte percent),
	#修改群红包分成比例
	i32 proc_modGroupUserGainSetting(1:i32 runid, 2:i32 groupid, 3:i32 userid, 4:byte percent),
}
