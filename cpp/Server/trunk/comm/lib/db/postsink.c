
#include <stdlib.h>

#include <sink.h>
#include <mysql.h>

#include "db/postsink.h"

static char null_char = '\0';

#ifndef USE_SINK_POOL_NULL
int post_connect(Sink * sink)
{
    if (!sink)
	    return -1;
    
	MYSQL * conn = mysql_init((MYSQL *)sink->u.c_mysql);
	if(!conn)
	{
		return -1;
	}

	int timeout_s = 10;
	mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout_s);
	mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout_s);

	MYSQL * conn1 = mysql_real_connect(conn,
    		sink->host,
			sink->user,
			sink->password,
			sink->dbname,
			sink->port,
			NULL,
			CLIENT_FOUND_ROWS);

    if(conn1)
    {
    	sink->u.c_mysql = conn;  //MYSQL*
    	//set client encoding
    	//before use GBK, GB18030
    	mysql_set_character_set(conn, "utf8mb4");
    	return 0;
    }
    else
    {
    	printf("Error connecting to database: %s\n", mysql_error(conn));
		return -1;
    }
}

int post_disconnect(Sink * sink)
{
    if(sink && sink->u.c_mysql)
    {
    	mysql_close((MYSQL *)sink->u.c_mysql);
        sink->u.c_mysql = NULL;
    	return 0;
    }
    return -1;
}

int post_reset(Sink * sink)
{
	if(sink)
    {
		return post_connect(sink);
	}
	else
	{
		return -1;
	}
}

Result * post_exec(Sink * sink, const char * query)
{
    Result * res = 0;
	if(sink && sink->u.c_mysql && query)
	{
		int ret = mysql_query((MYSQL *)sink->u.c_mysql, query);
		if (!ret)
		{
			MYSQL_RES * result = mysql_store_result((MYSQL *)sink->u.c_mysql);
			if (result)
			{
				res = (Result *)malloc(sizeof(Result));
				res->priv = result;
				res->state = ret;
			}
		}
	}
    return res;
}

void post_exec2(Sink * sink, const char * query, Result * res)
{
	if(res)
	{
		res->priv = 0;
		if(sink && sink->u.c_mysql && query)
		{
			//MYSQL_RES * r;
			int ret = mysql_query((MYSQL *)sink->u.c_mysql, query);
			if (!ret)
			{
				MYSQL_RES * result = mysql_store_result((MYSQL *)sink->u.c_mysql);
				if (result)
				{
					res->priv = result;
				}
				res->state = post_get_errno(sink);
			}
			else
			{
				res->state = ret;
			}
		}
	}
}

int post_conn_state(Sink * sink)
{
	if(sink && sink->u.c_mysql)
	{
		if (!mysql_ping((MYSQL *)sink->u.c_mysql))
		{
			return CONN_OK;
		}
		else
		{
			return CONN_BAD;
		}
	}
	return UNKNOWN_STATE;
}

int post_get_affected_rows(Sink * sink)
{
	if(sink && sink->u.c_mysql)
	{
		return mysql_affected_rows((MYSQL *)sink->u.c_mysql);
	}
	else
	{
		return -1;
	}
}

unsigned long post_get_insert_id(Sink *sink)
{
	if (sink && sink->u.c_mysql)
	{
		return mysql_insert_id((MYSQL *)sink->u.c_mysql);
	}
	else
	{
		return 0;
	}
}

const char * post_get(Result * res, int row, int col)
{
    if (!res || !res->priv)
    	return &null_char;

	MYSQL_RES * result = (MYSQL_RES *) res->priv;
	int row_num = mysql_num_rows(result);
	int field_num = mysql_num_fields(result);

	if (row_num < row + 1)
	{
		return &null_char;
	}

	if (field_num < col + 1)
	{
		return &null_char;
	}

	mysql_data_seek(result, row);
	MYSQL_ROW res_row = mysql_fetch_row(result);
	if(!res_row)
	{
		return &null_char;
	}

	if(res_row[col])
	{
		return res_row[col];
	}
	else
	{
		return &null_char;
	}
}

void post_clean(Result * res)
{
	if(!res)
		return;

	MYSQL_RES * result = (MYSQL_RES *)res->priv;
	if(result != 0)
		mysql_free_result(result);

	res->priv = 0;
}

int post_get_row_number(Result * res)
{
    if(!res || !res->priv)
    	return 0;

    MYSQL_RES * result = (MYSQL_RES *)res->priv;
    return mysql_num_rows(result);
}

int post_get_col_number(Result * res)
{
    if(!res || !res->priv)
    	return 0;

    MYSQL_RES * result = (MYSQL_RES *)res->priv;
    return mysql_num_fields(result);
}

int post_get_cmd_row_val(Result * res)
{
	if(!res || !res->priv)
		return 0;

    MYSQL_RES * result = (MYSQL_RES *)res->priv;
    return mysql_num_rows(result);
}

const char* post_fname(Result * res, int n)
{
    if(!res || !res->priv)
    	return &null_char;

    MYSQL_RES * result = (MYSQL_RES *)res->priv;
    int field_num = mysql_num_fields(result);
    if(field_num < n + 1)
    {
    	return &null_char;
    }

    MYSQL_FIELD * field = mysql_fetch_field_direct(result, n);
    if(!field)
    {
    	return &null_char;
    }

    if(!field->name)
    {
    	return &null_char;
    }
    else
    {
        return field->name;
    }
}

int post_cmd_state(Result * res)
{
    if(!res || res->state)
    {
    	return RES_FATAL_ERROR;
    }
    else if(!res->priv)
    {
    	return RES_COMMAND_OK;
    }
    else
    {
    	return RES_TUPLES_OK;
    }
}

int post_select_db(Sink *sink, const char *dbname)
{
	if(sink && sink->u.c_mysql && dbname)
	{
		return mysql_select_db((MYSQL *)sink->u.c_mysql, dbname);
	}
	else
	{
		return -1;
	}
}

int post_get_errno(Sink *sink)
{
	if(sink && sink->u.c_mysql)
	{
		return mysql_errno((MYSQL *)sink->u.c_mysql);
	}
	else
	{
		return -1;
	}
}

const char *post_get_error(Sink *sink)
{
	if(sink && sink->u.c_mysql)
	{
		return mysql_error((MYSQL *)sink->u.c_mysql);
	}
	else
	{
		return "";
	}
}
#endif

#ifdef USE_SINK_POOL_NULL
SinkOperations sos = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
#else
SinkOperations sos = {
    post_connect,
    post_conn_state,
    post_disconnect,
    post_reset,
    post_exec,
	post_exec2,
    post_get,
    post_clean,
    post_get_row_number,
    post_get_col_number,
    post_fname,
    post_cmd_state,
	post_get_cmd_row_val,
	post_get_affected_rows,
	post_get_insert_id,
	post_select_db,
	post_get_errno,
	post_get_error,
};
#endif
