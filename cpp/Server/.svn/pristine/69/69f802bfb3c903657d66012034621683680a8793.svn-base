#ifndef __DBPOOL_H__
#define __DBPOOL_H__
#include <db/sink.h>
#define POSTSINK   "postgresql"

#ifdef __cplusplus
extern "C" {
#endif

void sink_pool_reset(void);
int sink_pool_init(const char *type, 
	       int         platid,         //platform_id,for muti-db
		   int         size, 
		   const char *host,
		   int         port,
		   const char *options,
		   const char *dbname,
		   const char *login,
		   const char *pwd);
Sink*  sink_pool_get(int platid);
int    sink_reset_config(Sink * sink, 
						 int platid, 
						 const char * host, 
						 int port, 
						 const char * dbname, 
						 const char * login, 
						 const char * pwd);
void   sink_pool_put(Sink*);
void   sink_pool_cleanup();

//keep all db connection when nPlatId = -1
void   sink_pool_keeplive(int nPlatId, int bCheckBusy);
int    sink_pool_status_ok(int nPlatId, int bCheckBusy);

#ifdef __cplusplus
}
#endif

#endif  /*__DBPOOL_H__ */


