
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sink.h>
#include <mysql.h>

#include <pool.h>
#include <postsink.h>


static Sink **sinks = 0;
static int    sink_number = 0;
extern SinkOperations sos;

int sink_pool_init(const char *type,
		   int         platid,
		   int         nsize, 
		   const char *host,
		   int         port,
		   const char *options,
		   const char *dbname,
		   const char *login,
		   const char *pwd)
{
    int i = 0;
	if(sinks == NULL)
		sinks = (Sink **)malloc(sizeof(Sink*) * 100);
    //sinks = (Sink**)malloc(sizeof(Sink*) * nsize);
    //sink_number += nsize;
	if(sink_number + nsize > 100)
		return -2;
	
    for(i = sink_number; i < sink_number + nsize; i++) {
	    sinks[i] = (Sink *)malloc(sizeof(Sink));
	    memset(sinks[i], 0, sizeof(Sink));
    }
    
    if(strcmp(type, POSTSINK) == 0) {
		for(i = sink_number; i < sink_number + nsize; i++) {
			sinks[i]->platid = platid;
		    sinks[i]->sos = &sos;
		    strcpy(sinks[i]->user, login);
		    strcpy(sinks[i]->password, pwd);
		    strcpy(sinks[i]->dbname, dbname);
		    strcpy(sinks[i]->host, host);
		    sinks[i]->port = port;
	    
		    sink_init(sinks[i]);
        
		    if(sink_connect(sinks[i]) < 0) {
			    return -1;
		    }
		    if(sink_state(sinks[i]) != CONN_OK) {
			    return -1;
		    }
		}
    } else {
    	printf("not support this type sink so far yet\n");
    	return -1;
    }

	sink_number  =sink_number + nsize;

    return 0;
}

void sink_pool_reset(void)
{
   sink_pool_cleanup();
}

Sink*  sink_pool_get(int platid)
{
	int i=0;
	Sink *sink = 0;
	while(1) 
	{
		if (!sink_number)
		{
			printf("[sink_pool_get]init fail.sink_number is 0.\n");
			_exit(0);
		}
		
		//sink = sinks[random() % sink_number];
		for (i =0; i< sink_number; ++i)
		{
			sink = sinks[i];

			if (sink->platid != platid)
			{
				continue;
			}

			sink_lock(sink);
			if(sink_is_busy(sink)) {
				sink_unlock(sink);
				continue;
			}

			/* check connection status */
			if(sink_state(sink) != CONN_OK) 
			{
				printf("[sink_pool_get]sink platid=%d connect bad! then reset.\n", sink->platid);
				sink_reset(sink);
			}

			sink_set_busy(sink, 1);
			sink_unlock(sink);
			//break;
			return sink;
		}

		usleep(5000);
	}

	return 0;
}

void sink_pool_put(Sink *sink)
{
    if (sink != NULL) {
        sink_lock(sink);
        sink_set_busy(sink, 0);
        sink_unlock(sink);
    }
}

void sink_pool_cleanup()
{
	int i;
    Sink * sink = NULL;

    if (sinks != NULL) {
        
        for(i= 0; i < sink_number; i++) {
        	sink = sinks[i];
            
            if (sink != NULL) {
            	sink_disconnect(sink);
            	sink_cleanup(sink);
                free(sink);
                sinks[i] = NULL;
            }
        }
        
        free(sinks);
        sinks = NULL;
    }
    
    mysql_server_end(); //free thread variable.
    sink_number = 0;
}


void sink_pool_keeplive(int nPlatId, int bCheckBusy)
{
	int i;
	Sink * sink = 0;

	//sink = sinks[random() % sink_number];
	for(i=0; i< sink_number; ++i)
	{
		sink = sinks[i];

		if (nPlatId != -1 && sink->platid != nPlatId)
		{
			continue;
		}

		if (bCheckBusy)
		{
			sink_lock(sink);
			if(sink_is_busy(sink))
			{
				sink_unlock(sink);
				continue;
			}
		}

		//send a seq query to keep tcp-live
		//mysql_ping((MYSQL *)sink->u.c_mysql);
		
		if(sink_state(sink) != CONN_OK) 
		{
			printf("[sink_pool_keeplive]sink platid=%d connect bad! then reset.\n", sink->platid);
			if (sink_reset(sink) == 0)
			{
                printf("[sink_pool_keeplive]sink platid=%d connect bad! reset ok.\n", sink->platid);
            }
			else
			{
				printf("[sink_pool_keeplive]sink platid=%d connect bad! reset still fail!.\n", sink->platid);
			}
		}

		if (bCheckBusy)
		{
			sink_unlock(sink);
		}
	}
}

int sink_pool_status_ok(int nPlatId, int bCheckBusy)
{
	int i;
	Sink *sink = 0;
	int isok = 0;
	int num = 0;

	int total = 0;
	for(i=0; i < sink_number; ++i) {
		sink = sinks[i];

		if (!sink || ( nPlatId != -1 && sink->platid != nPlatId ))
		{
			continue;
		}

		total++;

		if (bCheckBusy)
		{
			sink_lock(sink);
			if(sink_is_busy(sink)) 
			{
				sink_unlock(sink);
				continue;
			}
		}

		if(sink_state(sink) != CONN_OK) 
		{
			num++;
		}
		if (bCheckBusy)
		{
			sink_unlock(sink);
		}
	}

	if (total == num)
		isok = -1;

	return isok;
}

int sink_reset_config(Sink *		sink, 
					   int			platid,
					   const char *	host,
					   int			port,
					   const char *	dbname,
					   const char *	login,
					   const char *	pwd)
{
	if (!sink)
	{
		printf("sink input is empty.");
		return -1;
	}

	sink->platid = platid;
	sink->sos = &sos;
	strcpy(sink->user, login);
	strcpy(sink->password, pwd);
	strcpy(sink->dbname, dbname);
	strcpy(sink->host, host);
	sink->port = port;

	sink_init(sink);

	if(sink_connect(sink) < 0) 
	{
		return -1;
	}

	if(sink_state(sink) != CONN_OK) 
	{
		return -1;
	}

	return 0;
}
