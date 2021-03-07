/*
 * Wrapper for a generic 32-bit random number generator.
 *
 * Ideally, rng32.h should only be included in source files, for it contains functions bodies
 * to be inlined. Those functions are thread-safe, as long as each thread uses its own rng:
 *
 * - Initializing the internal state of the given rng. The 'stream' parameter
 *   is optional since several rng do not support this, and can be left to 0:
 *
 *     void rng32_init(void *generic_rng32, uint64_t seed, uint64_t stream);
 *
 * - Generating the next unsigned 32-bit integer, from the given rng:
 *
 *     uint32_t rng32_nextInt(void *generic_rng32);
 *
 * - Generating a float uniformly between 0. and 1., from the given rng:
 *
 *     float rng32_nextFloat(void *generic_rng32);
 *
 * A stream, when available, is useful for producing distincts sequences of random number,
 * from the same rng and the same seed. Finally, RNG32_MAX is the maximum value that the
 * defined 32-bit rng is able to output.
 *
 * A rng32 instance is to be declared (anywhere) in order to be used, and passed by
 * address in the previous functions. A basic example is shown below, where the rng
 * is initialized with the current system time (assuming <time.h> has been included),
 * and the stream ID is 0. This will print a random integer between 0 and 9 included
 * (with a negligeable bias due to 10 no dividing RNG32_MAX):
 *
 * rng32 rng;
 * rng32_init(&rng, time(NULL), 0);
 * printf("%d\n", rng32_nextInt(&rng) % 10);
 *
 * Be wary that the resolution of time() is only the second, thus it may not be suited to provide
 * parallel processes with unique seeds. Use either a finer time resolution for this, or better
 * an index unique for each processes, or even the address of a resource unique to each of them.
*/
/* --------------------------------------------------------------------------- */
/*
 * What follows basically is the implementation of the 32-bit PCG RNG, stripped of the
 * global rng, with the addition of the function rng32_nextFloat(), and everything wrapped
 * in a generic form for ease of use/change. Functions are also inlined for speed.
*/

/*
 * PCG Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *       http://www.pcg-random.org
 */


#ifndef RNG32_H
#define RNG32_H

#if __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <limits.h>

typedef struct
{
	uint64_t state;
	uint64_t inc;
} rng32;

#define RNG32_MAX UINT_MAX

static inline uint32_t rng32_nextInt(void *generic_rng32)
{
	rng32 *rng = (rng32*) generic_rng32;
	uint64_t oldstate = rng->state;
	rng->state = oldstate * 6364136223846793005ULL + rng->inc;
	uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

__attribute__((unused)) static void rng32_init(void *generic_rng32, uint64_t seed, uint64_t stream)
{
	rng32 *rng = (rng32*) generic_rng32;
	rng->state = 0U;
	rng->inc = (stream << 1u) | 1u;
	rng32_nextInt(rng);
	rng->state += seed;
	rng32_nextInt(rng);
}

static inline float rng32_nextFloat(void *generic_rng32)
{
	return (float) rng32_nextInt(generic_rng32) / RNG32_MAX;
}

#if __cplusplus
}
#endif

#endif
