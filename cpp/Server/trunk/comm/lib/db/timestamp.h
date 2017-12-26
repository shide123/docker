#ifndef __TIMESTAMP_H
#define __TIMESTAMP_H
#include <time.h>
#include <sys/time.h>
#ifdef __cplusplus
extern "C" {
#endif

time_t toNumericTimestamp(const char *ts);
time_t toNumericDate(const char *ts);
void toStringTimestamp(time_t t, char *ts);
void toStringTimestamp2(time_t t, char* ts);
void toStringDate(time_t t, char *ts);

void toStringTimestamp3(time_t t, char* ts);
void toStringTimestamp4(time_t t, char* ts);
void toStringTimestamp5(time_t t, char* ts);
void toStringTimestamp6(struct timeval *tv, char* ts);
void toStringTimestamp7(time_t t, char* ts);

#ifdef __cplusplus
}
#endif

#endif
