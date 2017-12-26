
#ifdef __MY_DEBUG_HH__20090923__
#define __MY_DEBUG_HH__20090923__


#include <stdio.h> /* vsprintf */
#include <stdarg.h>
//#include <crtdbg.h>

#define DPRINTF_BUF_SZ  1024


#ifdef DEBUG
#define MYTRACE(char*fmt, ...) \
	do{ \
	va_list args; \
	char buf[DPRINTF_BUF_SZ]; \
	va_start(args, fmt); \
	vsprintf(buf, fmt, args); \
	va_end (args); \
	printf(buf); \
	}while(0)
#else
#define MYTRACE(char*fmt, ...)
#endif

#endif //__MY_DEBUG_HH__20090923__


