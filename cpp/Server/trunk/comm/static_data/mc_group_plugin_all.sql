-- ----------------------------
-- Records of mc_group_plugin_all
-- ----------------------------
truncate table mc.mc_group_plugin_all;
INSERT INTO mc.mc_group_plugin_all(name, alias, status, is_del, img_src, href, type) VALUES 
('plus_group', '进群验证', '1', '0', '/static/images/zgmp/group_06.png', '/Groupmanager/pluginDetailPage?page_name=plus_group', '0'),
('contribution', '贡献榜', '1', '0', '/static/images/zgmp/group_02.png', '/Groupmanager/pluginDetailPage?page_name=contribution', '0'),
('gain', '分成', '1', '0', '/static/images/zgmp/group_03.png', '/Groupmanager/pluginDetailPage?page_name=gain', '0'),
('voice_chat', '语音', '1', '0', '/static/images/zgmp/group_04.png', '/Groupmanager/pluginDetailPage?page_name=voice_chat', '0'),
('self_chat', '私聊限制', '1', '0', '/static/images/zgmp/group_11.png', '/Groupmanager/pluginDetailPage?page_name=self_chat', '0'),
('video_auth', '视频认证', '1', '0', '/static/images/zgmp/group_13.png', '/Groupmanager/pluginDetailPage?page_name=video_auth', '0')
;
