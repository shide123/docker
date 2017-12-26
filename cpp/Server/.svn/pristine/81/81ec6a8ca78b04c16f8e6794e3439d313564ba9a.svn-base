drop function if exists nextval;
CREATE FUNCTION `nextval`(name VARCHAR(50), area_id INTEGER) RETURNS bigint(20)
BEGIN
DECLARE current BIGINT;
	set current = 0;
	UPDATE mc_sequence SET current_value = (
	case when current_value >= max_value or current_value < min_value
		then min_value 
	else current_value + increment
	end
	) WHERE seq_name = name and area = area_id;
	SELECT current_value INTO current FROM mc_sequence WHERE seq_name = name and area = area_id;

	RETURN current;
END