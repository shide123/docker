-- ----------------------------
-- Records of mc_pay_channel
-- ----------------------------
truncate table mc.mc_pay_channel;
INSERT INTO mc.mc_pay_channel(key, name, status, version) VALUES 
('ali_pay', '支付宝支付', '1', ''),
('wx_pay', '微信支付', '1', ''),
('apple_pay', '苹果支付', '0', ''),
('union_pay', '银联支付', '0', ''),
('jd_pay', '京东支付', '0', ''),
('apple_inter_pay', '苹果内购', '1', '1.0'),
('sweet_pay', '糖果兑换', '0', '')
;
