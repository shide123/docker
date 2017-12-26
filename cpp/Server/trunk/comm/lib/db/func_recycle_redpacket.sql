DROP function if exists recycle_redpacket;

CREATE FUNCTION `recycle_redpacket`(packet_id int, gold_candy_exchange integer) RETURNS int
BEGIN
	DECLARE groupID	int unsigned default 0;
	DECLARE userID int unsigned default 0;
	DECLARE left_money BIGINT default 0;
	DECLARE recycle int default 0;

	IF packet_id <= 0 OR packet_id IS NULL THEN
		#RETURN CONCAT('input packet_id is invalid: ', IFNULL(packet_id, 'null'));
		RETURN -1;
	END IF;
	
	SELECT src_user, group_id, (packet_money-take_money), is_recycle INTO userID, groupID, left_money, recycle from mc_redpacket_info a WHERE a.id = packet_id FOR UPDATE;
	
	IF !found_rows() THEN
		#RETURN CONCAT('can not find red packet, id: ', packet_id);
		RETURN -2;
	END IF;
	
	IF recycle = 1 THEN
		#RETURN CONCAT('red packet has been recycled yet, id: ', packet_id);
		RETURN -3;
	END IF;

	/*更新红包信息表is_recycle为1 已回收*/
	UPDATE mc_redpacket_info SET is_recycle = 1 WHERE id = packet_id;
	
	/*按金币退回用户账户*/
	update mc_useraccount set gold_coin=gold_coin+left_money where user_id=userID;
	insert into mc_useraccount_log(user_id,source_type,source_id,income_type,amount,real_money,balance,insert_time,group_id)
		select userID,3,packet_id,0,left_money,left_money,gold_coin,UNIX_TIMESTAMP(),groupID from mc_useraccount where user_id=userID;

	#RETURN CONCAT('recycle success: ', packet_id,',',groupID,',',left_money);
	RETURN left_money;
END