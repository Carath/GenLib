#ifndef GET_TIME_H
#define GET_TIME_H

#if __cplusplus
extern "C" {
#endif


// Used to measure elapsed time. Thread safe.

#ifndef _WIN32
#include <sys/time.h>
__attribute__((unused)) static double get_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double) tv.tv_sec + (double) tv.tv_usec / 1000000.;
}
#else
#include <windows.h>
__attribute__((unused)) static double get_time(void)
{
	return (double) GetTickCount() / 1000.;
}
#endif


#if __cplusplus
}
#endif

#endif
