truncate table mc.mc_sys_parameter;
insert into mc.mc_sys_parameter(paramname, paramvalue, status, notifyall, description) values
('GOLD_CANDY_EXCHANGE',						'100',									1,		0,		'1个金币能兑换100个糖果数'),
('RMB_CANDY_EXCHANGE',						'100',									1,		0,		'1RMB可以兑换100个糖果'),
('RMB_GOLD_EXCHANGE',							'1',										1,		0,		'1RMB可以兑换1个金币'),
('GOLD_CONTRIBUTE_EXCHANGE',			'1',										1,		0,		'1金币兑1贡献值'),
('RED_PACKET_MESSAGE',						'有钱任性',							1,		1,		'红包默认留言'),
('PICTURE_COMM_URL',							'https://file.ogod.xin',1,		1,		'图片的公共域名')
('LEAST_GOLD_PER_LIGHT_REDPACKET','1000',									1,		0,		'照片炮弹平均最低金币数')
;
