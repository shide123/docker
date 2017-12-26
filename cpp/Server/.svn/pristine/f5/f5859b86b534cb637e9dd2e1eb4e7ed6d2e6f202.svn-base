drop function if exists take_redpacket;

CREATE FUNCTION `take_redpacket`(
v_redpacket_id integer, /*炮弹id*/
v_send_user integer, /*发送者*/
v_packet_type integer, /*炮弹类型*/
v_group integer, /*群*/
v_take_user integer, /*领取者*/
v_take_money integer, /*金额乘以100*/
v_take_percent integer, /*分成比例*/
v_gold_candy_exchange integer) RETURNS bigint(20)
BEGIN
DECLARE v_money_scale int DEFAULT 100;			#v_take_money的比例，默认100
DECLARE v_ret_code bigint DEFAULT 0;			#返回值（<0出错，>0糖果数）
DECLARE v_take_candy bigint DEFAULT 0;			#用户领取糖果数*v_money_scale
DECLARE v_real_candy bigint DEFAULT 0;			#用户领取糖果数（单位：个）
DECLARE v_person_income bigint DEFAULT 0;		#用户实际入账糖果数（单位：个）
DECLARE v_owner_income bigint DEFAULT 0;		#群主分成金额（单位：分）
DECLARE v_now_time integer DEFAULT UNIX_TIMESTAMP(now());	#当前时间
DECLARE v_charm_up bigint DEFAULT 0;			#魅力值提升
DECLARE v_gain_group integer DEFAULT 0; 		#群抽水比例
DECLARE v_candy_gift_type int DEFAULT 1;		#糖果礼物类型（默认为1）
DECLARE v_candy_price decimal(8,2) DEFAULT 0.01;#糖果价值（元）
DECLARE v_candy_charm decimal(8,2) DEFAULT 1;	#糖果魅力值
DECLARE v_user_real_income bigint DEFAULT 0;	#用户实际入账金额（单位：分）
DECLARE v_owner_real_income bigint DEFAULT 0;	#群主实际入账金额（单位：分）
DECLARE v_group_real_income bigint DEFAULT 0;	#群账户实际入账金额（单位：分）

DECLARE v_group_master integer DEFAULT (select group_master from mc_group where id = v_group limit 1);	#群主

if exists (select 1 from mc_redpacket_log where packet_id=v_redpacket_id and take_user=v_take_user) then
	set v_ret_code = -2;	/*已领过*/
else
	update mc_redpacket_info set take_money=take_money+v_take_money,take_num=take_num+1,is_recycle=if(packet_num=take_num/*TM此时take_num已+1*/,1,0),finish_time=if(packet_num=take_num/*TM此时take_num已+1*/,v_now_time,0) where id=v_redpacket_id and type=v_packet_type and group_id=v_group and take_money+v_take_money<=packet_money;
	if row_count() then
		select gain_group into v_gain_group from mc_group_plugin_setting_detail where group_id = v_group limit 1;
		select price, charm into v_candy_price, v_candy_charm from mc_gift where id = v_candy_gift_type limit 1;
		
		if (v_group_master is null || v_group_master = 0 || v_take_user = v_group_master || v_send_user = v_take_user) then 
			set v_take_percent = 0; /*群主领取不用分成，没有群主则不分成, 抢自己的炮弹不分成*/
		elseif v_take_percent>100 then
			set v_take_percent = 100;
		end if;
		
		/*真实金额（单位：分）*/
		set v_owner_real_income = floor(v_take_money * v_take_percent / 100);
		set v_user_real_income = v_take_money - v_owner_real_income;
#		set v_group_real_income = floor(v_owner_real_income * v_gain_group / 100);
		set v_owner_real_income = v_owner_real_income - v_group_real_income;
		
		/*糖果数*v_money_scale*/
#		set v_gain_group = v_group_real_income / v_candy_price;
		set v_take_candy = v_take_money / v_candy_price;
		set v_owner_income = v_owner_real_income / v_candy_price;
		set v_person_income = v_user_real_income / v_candy_price;
		
		/*真实糖果数（个）*/
		set v_real_candy = v_take_candy / v_money_scale;
		
		/*炮弹日志表*/
		insert into mc_redpacket_log(packet_id,take_user,take_money,person_income,group_income,time)
			values(v_redpacket_id,v_take_user,v_take_money,v_user_real_income,v_take_money-v_user_real_income,v_now_time);
	
		
		/*用户账户表 仅增加实际入账v_user_real_income,v_person_income*/
		update mc_useraccount set gold_coin=gold_coin+v_user_real_income where user_id=v_take_user;

		/*source_type 0：充值（金额+） 2：发炮弹（金额-） 3：炮弹余额退还（金额-）*/
		/*source_type 10：抢炮弹（糖果+） 12：提现（金额-） 13：群主收到炮弹分成（金额+） 14：领取炮弹分成给群主（金额-）*/
		/*source_type 15：群主收入提成给群账户（金额-）*/
		/*income_type 0 表示收入  1表示支出*/
		/*用户账户日志表 记两条记录，一条是领取总额v_take_money，一条是分成出去的糖果数（v_owner_income>0时记录）*/
		insert into mc_useraccount_log(user_id,source_type,source_id,income_type,gift_id,amount,real_money,balance,insert_time,group_id)
			select v_take_user,10,v_redpacket_id,0,v_candy_gift_type,v_real_candy,v_user_real_income,gold_coin,v_now_time,v_group
			  from mc_useraccount where user_id = v_take_user;
		
		
		if v_send_user != v_take_user then
			/*更新发炮弹用户贡献值（入群炮弹除外）*/
			if v_packet_type <> 4 then
				update mc_group_member set contribution_total=contribution_total+v_take_money,contribution_today=contribution_today+v_take_money where group_id=v_group and user_id=v_send_user;
			end if;
			
			/*更新领炮弹用户在该群收到的糖果数、魅力值*/
			set v_charm_up = if(v_packet_type = 2, v_real_candy * v_candy_charm, 0);	/*仅定向炮弹计算魅力值*/
			update mc_group_member set recv_candy_num=recv_candy_num+v_real_candy,
									charm_total=charm_total+v_charm_up,
									charm_today=charm_today+v_charm_up 
			 where group_id = v_group and user_id=v_take_user;
			
			/*更新领取用户礼物表*/
			update mc_user_gift set gift_num = gift_num + v_real_candy, update_time = now() 
			 where user_id = v_take_user and group_id = v_group and inout_type = 0 and gift_id = v_candy_gift_type;
			if row_count() = 0 then
				insert into mc_user_gift(user_id, group_id, inout_type, gift_id, gift_num)
					values(v_take_user, v_group, 0, v_candy_gift_type, v_real_candy);
			end if;
			
			/*更新发炮弹用户礼物表*/
			update mc_user_gift set gift_num = gift_num + v_real_candy, update_time = now() 
			 where user_id = v_send_user and group_id = v_group and inout_type = 1 and gift_id = v_candy_gift_type;
			if row_count() = 0 then
				insert into mc_user_gift(user_id, group_id, inout_type, gift_id, gift_num)
					values(v_send_user, v_group, 1, v_candy_gift_type, v_real_candy);
			end if;
			
			/*更新用户亲密度*/
			if v_person_income > 0 then
				update mc_user_relationship set intimacy=intimacy+v_user_real_income where user_id = v_take_user and relation_id = v_send_user and group_id = v_group;
				if row_count() = 0 then
					insert into mc_user_relationship(group_id, user_id, relation_id, intimacy) values(v_group, v_take_user, v_send_user, v_user_real_income);
				end if;
			end if;
		end if;
		
		/*群主分成*/
		if (v_owner_real_income > 0) then
			/*群主账户*/
			update mc_useraccount set gold_coin=gold_coin+v_owner_real_income where user_id=v_group_master;
			/*群主账户日志*/
			insert into mc_useraccount_log(user_id,source_type,source_id,income_type,related_id,gift_id,amount,real_money,balance,insert_time,group_id)
				select v_group_master,13,v_redpacket_id,0,v_take_user,0,v_owner_real_income,v_owner_real_income,gold_coin,v_now_time,v_group from mc_useraccount where user_id = v_group_master;
			/*领取分成日志*/
			insert into mc_useraccount_log(user_id,source_type,source_id,income_type,related_id,gift_id,amount,real_money,balance,insert_time,group_id)
				select v_take_user,14,v_redpacket_id,1,v_group_master,0,v_owner_real_income,v_owner_real_income,gold_coin,v_now_time,v_group from mc_useraccount where user_id = v_take_user;
			
#			/*群主分成抽成给群账户*/
#			if v_gain_group > 0 then
#					/*群账户表*/
#					UPDATE mc_group_account SET candy_num=candy_num+v_gain_group,week_candy_num=week_candy_num+v_gain_group,month_candy_num=month_candy_num+v_gain_group WHERE group_id=v_group;
#
#					/*群账户日志表*/
#					/*type 0：分成(糖果+)  1：群主收入提成(糖果+)  2：提现（糖果-） 3：群主收入抽水（糖果+）*/
#					INSERT INTO mc_group_account_log(group_id,user_id,source_id,candy_num,create_time,type)VALUES(v_group,v_take_user,v_redpacket_id,v_gain_group,v_now_time,3);
#					/*群主账户日志*/
#					insert into mc_useraccount_log(user_id,source_type,source_id,income_type,related_id,gift_id,amount,real_money,balance,insert_time,group_id)
#						select v_group_master,15,v_redpacket_id,1,v_take_user,v_candy_gift_type,v_gain_group,0,gold_coin,v_now_time,v_group from mc_useraccount where user_id = v_group_master;
#			end if;
		end if;
		
		set v_ret_code = v_take_candy;
	else
		set v_ret_code = -3;	/*已领完*/
	end if;
end if;
RETURN v_ret_code;
END