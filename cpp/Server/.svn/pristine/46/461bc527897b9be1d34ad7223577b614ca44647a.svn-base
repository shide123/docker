AUTOMAKE_OPTIONS = foreign

noinst_LIBRARIES = libdb.a
libdb_a_SOURCES = sink.c postsink.c pool.c optable2.c timestamp.c 
CFLAGS = @CFLAGS@ -I/usr/local/pgsql/include -I.. -I../../


#lib_LTLIBRARIES = libcom.la
#libcom_la_SOURCES = base64.c buffer.c lh_stats.c lhash.c log.c md5.c hash.c queue.c stack.c tree.c mem.c
#include_HEADERS = base64.h buffer.h lhash.h log.h md5.h hash.h queue.h stack.h
#libcom_la_LDFLAGS = -version-info 0:0:0
