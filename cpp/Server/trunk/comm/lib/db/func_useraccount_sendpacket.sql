drop function if exists useraccount_sendpacket;

CREATE FUNCTION `useraccount_sendpacket`(send_userid integer,dst_userid integer,packetMoney bigint,rangeType integer,rangeGender integer,packetNum integer,fixPerMoney bigint,packet_type integer,packet_groupid integer,packetMessage varchar(64),contribut_exchange integer, now_time integer, isPrivate integer) RETURNS integer
BEGIN
DECLARE ret_success integer DEFAULT 0;
DECLARE total_money bigint DEFAULT 0;
DECLARE contribute_value bigint DEFAULT 0;

if packet_type=0 then 
	set total_money=packetMoney; 
elseif packet_type=1 then
	set total_money=fixPerMoney*packetNum;
elseif packet_type=2 then
	set total_money=packetMoney;
elseif packet_type=3 then
	set total_money=packetMoney;
elseif packet_type=4 then
	/* 4 是 入群红包类型 */
	set total_money=packetMoney;
else
	set total_money=packetMoney;
end if;

update mc_useraccount set gold_coin=gold_coin-total_money where user_id=send_userid and gold_coin>=total_money;
if row_count() then
	insert into mc_redpacket_info(group_id,src_user,dst_user,private_flag,type,range_type,range_gender,packet_num,fix_per_money,packet_money,message,create_time)values(packet_groupid,send_userid,dst_userid,isPrivate,packet_type,rangeType,rangeGender,packetNum,fixPerMoney,total_money,packetMessage,now_time);
	select last_insert_id() into ret_success;
	/*source_type 0：金币充值（金币+）1：糖果兑金币（金币+）2：金币发红包 （金币-） 10：抢红包（糖果+） 11：糖果兑金币（糖果-）*/
	/*income_type 0 表示收入  1表示支出*/
	insert into mc_useraccount_log(user_id,source_type,source_id,income_type,amount,real_money,balance,insert_time,group_id)
		select send_userid,2,ret_success,1,total_money,total_money,gold_coin,now_time,packet_groupid from mc_useraccount where user_id=send_userid;
	RETURN ret_success;
else
	set ret_success=-3;
	RETURN ret_success;
end if;
END
