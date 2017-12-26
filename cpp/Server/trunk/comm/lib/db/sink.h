
#ifndef __DB_POSTGRES_SINK_H__
#define __DB_POSTGRES_SINK_H__

#include "com/lock_def.h"

/*
  typedef struct _Sink   Sink;
  typedef struct _Result Result;
*/
typedef struct _Sink             Sink;
typedef struct _Result           Result;
typedef struct _SinkOperations   SinkOperations;
struct _SinkOperations{
/*operations*/
    int         (*connect)    (Sink *);
    int         (*conn_state) (Sink *);
    int         (*disconnect) (Sink *);
    int         (*reset)      (Sink *);
    Result*     (*exec)       (Sink *, const char*);
	void        (*exec2)      (Sink *, const char*, Result*);
    const char* (*get)        (Result *, int row, int col);
    void        (*clean)      (Result*);
    
    int         (*get_row_number) (Result*);
    int         (*get_col_number) (Result*);
    const char* (*fname)          (Result*, int);
    int         (*command_state)  (Result*);
	int         (*get_cmd_row_val)(Result*);
	int         (*get_affected_rows)(Sink *);
	unsigned long (*get_insert_id)(Sink *);
	int			(*select_db)(Sink *, const char *);
	int			(*get_errno)(Sink *);
	const char* (*get_error)(Sink *);
};

struct _Sink {
	LOCK_T  lock;
	int                platid;
    int                busy;
    char               user[20];
    char               password[1024];
    char               dbname[50];
    char               host[50];
    int                port;
    SinkOperations    *sos;
    union {
	/*MYSQL*/void   *c_mysql;
	/*maybe mysql will be added later*/
    } u;
};
struct _Result { 
    void   *priv;
    int    state;
}; 

#define UNKNOWN_STATE  -1
enum {
    CONN_OK,
    CONN_BAD,
    CONN_STARTED,
    CONN_MADE,
    CONN_AWAITING_RESPONSE,
    CONN_AUTH_OK,
    CONN_SETENV,
};
enum {
    RES_EMPTY_QUERY, /* -- The string sent to the backend was empty.*/
    RES_COMMAND_OK,  /* -- Successful completion of a command returning no data*/
    RES_TUPLES_OK,   /* -- The query successfully executed*/
    RES_COPY_OUT,    /* -- Copy Out (from server) data transfer started*/
    RES_COPY_IN,     /* -- Copy In (to server) data transfer started */
    RES_BAD_RESPONSE,/* -- The server's response was not understood */
    /* PGRES_NONFATAL_ERROR
     */
    RES_FATAL_ERROR
};

enum {
    O_GET,
    O_SET,
    O_CREATE,
    O_DELETE,
};

#ifdef __cplusplus
extern "C" {
#endif

    int  register_sink_ops(const char *type, SinkOperations *sos);
    void unregister_sink_ops(const char *type);

    void    sink_init(Sink*);
    void    sink_cleanup(Sink*);
    int     sink_connect(Sink *);
    int     sink_state(Sink *);
    int     sink_disconnect(Sink*);
    int     sink_reset(Sink*);
    Result* sink_exec(Sink *, const char*);
	void    sink_exec2(Sink*, const char*, Result*);
    void    sink_lock(Sink*);
    void    sink_unlock(Sink*);
    void    sink_set_busy(Sink*, int);
    int     sink_is_busy(Sink*);
    int     trans_begin(Sink*);
    void    trans_rollback(Sink*);
    void    trans_commit(Sink*);

    const char*  result_get   (Sink*, Result *, int row, int col);
    void         result_clean (Sink*, Result*);
    int          result_rn (Sink*, Result*);
	int          result_cmd_rn (Sink*, Result*);
    int          result_cn (Sink*, Result*);
    const char*  result_fname(Sink*, Result*, int);
    int          result_state(Sink*, Result*);
    
    int     sink_get_affected_rows(Sink*);
	unsigned long sink_get_insert_id(Sink*);
	int sink_get_errno(Sink*);
	const char* sink_get_error(Sink*);

#ifdef __cplusplus
}
#endif
#endif /*__DB_POSTGRES_SINK_H__ */


