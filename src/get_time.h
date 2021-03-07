#ifndef GET_TIME_H
#define GET_TIME_H

#if __cplusplus
extern "C" {
#endif


#include <stdint.h>


// Used to measure elapsed time, in seconds. Thread safe.
double get_time(void);


// Used to create RNG different seeds per nanosecond. A pointer to a memory block can
// also be given to yield a unique seed per process, however it can be left to NULL.
uint64_t create_seed(void *address);


#if __cplusplus
}
#endif

#endif
