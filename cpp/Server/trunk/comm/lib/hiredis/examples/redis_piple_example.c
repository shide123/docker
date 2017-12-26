#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "hiredis.h"

#define CHECK(X) if ( !X || X->type == REDIS_REPLY_ERROR ) { printf("Error\n"); exit(-1); }

/******************************************************************************/
/* Fill Redis with some dummy data */

void fill( redisContext *c, int n )
{
  redisReply *reply;
  unsigned int i,j,k;
  unsigned int count = 0;

  reply = (redisReply *) redisCommand(c,"SELECT %d",n);
  CHECK(reply);
  freeReplyObject(reply);

  reply = (redisReply *) redisCommand(c,"FLUSHDB");
  CHECK(reply);
  freeReplyObject(reply);

  for (i=0; i<100000; ++i )
  {
    int cmd = 0;
    
    for (j=0; j<100; ++j )
    {
      if ( j % 2 == 0 )
      {
        ++count;
        redisAppendCommand(c,"SADD %d %d",i,j );
        ++cmd;
      }
    }
    while ( cmd-- > 0 )
    {
      int r = redisGetReply(c, (void **) &reply );
      if ( r == REDIS_ERR ) { printf("Error\n"); exit(-1); }
      CHECK(reply);        
      freeReplyObject(reply);
    }
  }
}

/******************************************************************************/
/* Query function: the queries are batched using pipelining to improve efficiency */

void query( redisContext *c, int n )
{
  redisReply *reply;
  unsigned int i,j,k;
  unsigned int count = 0;
  unsigned int *s = 0;
  unsigned int *v = 0;
  unsigned int *f = 0;

  unsigned int batch = 100;
  /* Use this instead to remove pipelining */
  /* unsigned int batch = 1; */

  reply = (redisReply *) redisCommand(c,"SELECT %d",n);
  CHECK(reply);
  freeReplyObject(reply);

  s = (unsigned int *) malloc( batch * sizeof(unsigned int) );
  v = (unsigned int *) malloc( batch * sizeof(unsigned int) );
  f = (unsigned int *) malloc( batch * sizeof(unsigned int) );

  /* Loop on queries */
  for (i=0; i<1000000/batch; ++i )
  {
    unsigned int cmd = 0;

    /* First step: run a batch of SISMEMBER commands */
    for ( j=0; j<batch; ++j )
    {
      s[j] = rand() % 100000;
      v[j] = rand() % 100;
      redisAppendCommand(c,"SISMEMBER %d %d", s[j], v[j] );
      ++cmd;
    }

    /* Read the replies (i.e. all the booleans) */
    j = 0;
    while ( cmd-- > 0 )
    {
      int r = redisGetReply(c, (void **) &reply );
      if ( r == REDIS_ERR ) { printf("Error\n"); exit(-1); }
      CHECK(reply);
      f[j++] = reply->integer;
      freeReplyObject(reply);
    }

    /* Second step: run a batch of SADD commands */
    cmd = 0;
    for ( j=0; j<batch; ++j )
    {
      /* printf( "%d %d %d\n", s[j], v[j], f[j] ); */
      if ( f[j] )
      {
        redisAppendCommand(c,"SADD %d %d",s[j]-1,v[j]+1 );
        ++cmd;
      }
    }

    /* Read (and ignore) the replies */
    while ( cmd-- > 0 )
    {
      int r = redisGetReply(c, (void **) &reply );
      if ( r == REDIS_ERR ) { printf("Error\n"); exit(-1); }
      CHECK(reply);        
      freeReplyObject(reply);
    }
  }

  free( s );
  free( v );
  free( f );
}

/******************************************************************************/
/* Just used to extract the number of queries done by the Redis server */

const char *CRLF="\r\n";
const char *USED="used_memory_human:";
const char *CMD ="total_commands_processed:";

void redisTop( redisContext *c )
{
  redisReply *reply;
  char *p1, *p2;
  char bU[32], bC[32];
  long nCmd = 0;

  reply = (redisReply *) redisCommand(c,"INFO");
  CHECK(reply);
  if ( reply->type == REDIS_REPLY_STRING )
  {
    
    p1 = strstr(reply->str, USED );
    if (!p1) return;
    p1 += strlen(USED);
    p2 = strstr(p1,CRLF); 
    memcpy(bU,p1,p2-p1);
    bU[p2-p1] = 0;
    
    p1 = strstr(p2, CMD );
    if (!p1) return;
    p1 += strlen(CMD);
    p2 = strstr(p1,CRLF); 
    memcpy(bC,p1,p2-p1);
    bC[p2-p1] = 0;
    
    nCmd = atol(bC);
    printf("%ld\n", nCmd );
    
    freeReplyObject(reply);
  }
}

/******************************************************************************/
/* Calling with first parameter = 0 will flush and create dummy data in Redis.
   Calling with first parameter = 1 will run a query benchmark */

int main(int argc, char *argv[] ) {

  redisContext *c;
  redisReply *reply;
  int n = 0;
  int db = 0;
    
  if ( argc != 2 )
  {
    fprintf(stderr,"Usage: %s n\n",argv[0]);
    exit(-1);
  }

  n = atoi(argv[1]);

  c = redisConnect( "localhost", 6379);
  /* c = redisConnectUnix("/tmp/redis.sock"); */
  if (c->err) {
    printf("Connection error: %s\n", c->errstr);
    exit(1);
  }

  reply = (redisReply *) redisCommand(c,"CONFIG RESETSTAT");
  CHECK(reply);
  freeReplyObject(reply);

  if ( n == 0 )
    fill(c,0);
  else
    query(c,0);

  redisTop(c);

  redisFree(c);
  return 0;
}

/******************************************************************************/
