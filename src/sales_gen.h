#ifndef SALES_GEN_H
#define SALES_GEN_H


#include "GenLib.h"
#include "salesman.h"
#include "rng32.h" // necessary to be put here, for total inlining.


#define FITNESS_SCALE 10000. // arbitrary.


extern const GeneticMethods GeneMeth_salesman_1;
extern const GeneticMethods GeneMeth_salesman_2;
extern const GeneticMethods GeneMeth_salesman_3;


// Obtains uniformly (i, j) such as: 0 <= i < j < n.
// This is (almost) unbiased, and has a probability of 1 - 1/n to end in one pass.
// There is faster versions of this for some ranges of 'n', to be tried...
// However versions starting with a value > 0 seem to be way slower...
static inline void getStrictCouple(void *rng, int *i, int *j, int n)
{
	*i = rng32_next(rng) % n;

	do { *j = rng32_next(rng) % n; }
	while (*i == *j);

	if (*i > *j)
	{
		int temp = *i;
		*i = *j;
		*j = temp;
	}
}


// Swap the two values of the given array. It does not test
// for equality between i and j, for this is slower!
static inline void swap(int *array, int i, int j)
{
	int temp = array[i];
	array[i] = array[j];
	array[j] = temp;
}


// Mirror the values between start and end. Needed: start <= end.
static inline void mirror(int *array, int start, int end)
{
	int range = (end - start + 1) / 2; // central value isn't swapped if the number of value is odd.

	for (int i = 0; i < range; ++i)
		swap(array, start + i, end - i);
}


#endif
