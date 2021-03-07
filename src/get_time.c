#include <stdlib.h>

#include "get_time.h"


// Used to measure elapsed time, in seconds. Thread safe.
#ifndef _WIN32
#include <sys/time.h>
double get_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double) tv.tv_sec + (double) tv.tv_usec / 1000000.;
}
#else
#include <windows.h>
double get_time(void)
{
	return (double) GetTickCount() / 1000.;
}
#endif


// Used to create RNG different seeds per nanosecond. A pointer to a memory block can
// also be given to yield a unique seed per process, however it can be left to NULL.
uint64_t create_seed(void *address)
{
	return (uint64_t) (get_time() * 1e9) + (uintptr_t) address;
}
