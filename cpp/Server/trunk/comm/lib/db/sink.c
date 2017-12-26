
#include <stdlib.h>
#include <string.h>

#include <com/hash.h>
#include <mysql.h>

#include <db/sink.h>

static HashTable *hash = 0;

int register_sink_ops(const char *type, SinkOperations *sos)
{
    if(!hash) {
	hash = hash_table_new_full(str_hash, str_equal, free, 0);
    }
    
    hash_table_insert(hash, strdup(type), sos);
    return 0;
}
SinkOperations *get_sink_ops(const char *type)
{
    if(hash) 
	return hash_table_lookup(hash, type);
    return 0;
}
void unregister_sink_ops(const char *type)
{
    if(hash) 
	hash_table_remove(hash, type);
}


/***************sink operations *****************/
void  sink_init(Sink* sink)
{
    if(!sink)
	return;

	pthread_mutex_init(&sink->lock, 0);
    sink->busy = 0;
}
void  sink_cleanup(Sink* sink) 
{
    if(!sink)
	return;

	pthread_mutex_destroy(&sink->lock);
}
int sink_connect(Sink* sink)
{
    if(sink && sink->sos && sink->sos->connect) {
	return sink->sos->connect(sink);
    }
    return -1;
}
int sink_state(Sink *sink)
{
    if(sink && sink->sos && sink->sos->conn_state) {
	return sink->sos->conn_state(sink);
    }
    return CONN_BAD;
}
int sink_disconnect(Sink* sink)
{
    if(sink && sink->sos && sink->sos->disconnect) {
	return sink->sos->disconnect(sink);
    }
    return -1;
}
int sink_reset(Sink *sink)
{
    if(sink && sink->sos && sink->sos->reset) {
	return sink->sos->reset(sink);
    }
    return -1;
}
Result*  sink_exec(Sink *sink, const char* query)
{
	/*
    static FILE *fp = 0;
    if(fp == 0) {
	fp = fopen("/home/tac/sink.log", "w");
    }
    if(fp) {
        fprintf(fp, "%s\n", query);
        fflush(fp);
    }
	*/

    if(query && sink && sink->sos && sink->sos->exec) {
	return sink->sos->exec(sink, query);
    }
    return 0;
}

int sink_get_affected_rows(Sink *sink)
{
	if(sink && sink->sos && sink->sos->get_affected_rows) {
		return sink->sos->get_affected_rows(sink);
	}
	return -1;
}

unsigned long sink_get_insert_id(Sink *sink)
{
	if (sink && sink->sos && sink->sos->get_insert_id) {
		return sink->sos->get_insert_id(sink);
	}

	return 0;
}

void sink_exec2(Sink* sink, const char* query, Result* res)
{
	if(query && sink && sink->sos && sink->sos->exec2) {
		return sink->sos->exec2(sink, query, res);
	}
}

const char*  result_get(Sink* sink, Result *result, int row, int col)
{
    if(sink && result && sink->sos && sink->sos->get) {
	return sink->sos->get(result, row, col);
    }
    return 0;
}

void  result_clean (Sink* sink, Result* result)
{
    if(sink && result && sink->sos && sink->sos->clean) {
	sink->sos->clean(result);
    }
}
    
int result_rn (Sink *sink, Result *result)
{
    if(sink && result && sink->sos && sink->sos->get_row_number)
	return sink->sos->get_row_number(result);
    
    return -1;
}

int result_cmd_rn(Sink* sink, Result *result)
{
	if(sink && result && sink->sos && sink->sos->get_cmd_row_val)
		return sink->sos->get_cmd_row_val(result);

	return -1;
}

int result_cn (Sink *sink, Result *result)
{
    if(sink && result && sink->sos && sink->sos->get_col_number)
		return sink->sos->get_col_number(result);
    
    return -1;
}
const char*  result_fname(Sink *sink, Result *result, int col)
{
    if (sink && result && sink->sos && sink->sos->fname) {
		return sink->sos->fname(result, col);
    }
    return 0;
}
int result_state(Sink *sink, Result *result)
{
    if(sink && result && sink->sos && sink->sos->command_state) 
		return sink->sos->command_state(result);

    return RES_FATAL_ERROR;
}
    

void sink_lock(Sink* sink)
{
    if(sink)
		pthread_mutex_lock(&sink->lock);
}
void  sink_unlock(Sink* sink)
{
    if(sink)
		pthread_mutex_unlock(&sink->lock);
}
void sink_set_busy(Sink* sink, int val)
{
    sink->busy = val;
}
int sink_is_busy(Sink* sink)
{
    return sink->busy;
}
int trans_begin(Sink* sink)
{
    Result *result;
    int     ret;
    if(!sink)
	return -1;
    result = sink_exec(sink, "begin");
    if(!result)
	return -1;
    if(result_state(sink, result) == RES_COMMAND_OK) {
	ret = 0;
    } else {
	ret = -1;
    }
    
    result_clean(sink, result);
	free(result);
    return ret;
}
void  trans_rollback(Sink* sink)
{
    Result *result;
    if(sink) {
	result = sink_exec(sink, "rollback");
	if(result) {
	    result_clean(sink, result);
	    free(result);
	}
    }
}
void  trans_commit(Sink* sink)
{
    Result *result;
    if(sink) {
	result = sink_exec(sink, "commit");
	if(result) {
	    result_clean(sink, result);
		free(result);
	}
    }
}

int sink_get_errno(Sink *sink)
{
	if (sink && sink->sos && sink->sos->get_errno) 
	{
		return sink->sos->get_errno(sink);		
	}

	return -1;
}

const char *sink_get_error(Sink *sink)
{
	if (sink && sink->sos && sink->sos->get_error) 
	{
		return sink->sos->get_error(sink);		
	}

	return "";
}
