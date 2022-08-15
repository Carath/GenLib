#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sales_gen.h"
#include "rng32.h"


static void* createGene(const void *context, void *rng)
{
	const Map *map = (Map*) context;
	int *gene_tofill = (int*) calloc(map -> CitiesNumber, sizeof(int));

	initPath(rng, gene_tofill, map -> CitiesNumber, DEFAULT_INIT_MODE);

	return gene_tofill;
}


static void copyGene(const void *context, void *gene_tofill, const void *gene)
{
	const Map *map = (Map*) context;

	memcpy(gene_tofill, gene, map -> CitiesNumber * sizeof(int)); // copying even the first city!
}


static void destroyGene(const void *context, void *gene)
{
	free(gene);
}


static double fitness(const void *context, const void *gene, long epoch)
{
	const Map *map = (Map*) context;

	return FITNESS_SCALE / pathLength(map, (int*) gene);
}


// This crossover only copies the given gene:
void crossover_0(const void *context, void *rng, void *gene_tofill, const void *gene_1, const void *gene_2,
	double fitness_1, double fitness_2, long epoch)
{
	copyGene(context, gene_tofill, gene_1);
}


// Will return length if the value isn't found!
inline static int findIndexOfValue(const int *array, int length, int start, int value)
{
	int i = start;

	while (i < length && array[i] != value)
		++i;

	return i;
}


void crossover_1(const void *context, void *rng, void *gene_tofill, const void *gene_1, const void *gene_2,
	double fitness_1, double fitness_2, long epoch)
{
	const Map *map = (Map*) context;
	const int length = map -> CitiesNumber;
	int *path_1 = (int*) gene_1, *path_2 = (int*) gene_2, *new_path = (int*) gene_tofill;

	const int start = 1; // First city fixed!

	for (int i = start; i < length; ++i) // Necessary setup.
		new_path[i] = -1;

	for (int i = start; i < length; ++i)
	{
		int pivot_index = i, pivot = path_1[i];
		int found_index = findIndexOfValue(path_2, length, start, pivot);

		if (new_path[pivot_index] >= 0 && new_path[found_index] >= 0)
		{
			int blanck_spot_index = findIndexOfValue(new_path, length, start, -1);
			new_path[blanck_spot_index] = pivot;
		}

		else if (new_path[pivot_index] >= 0)
			new_path[found_index] = pivot;

		else if (new_path[found_index] >= 0)
			new_path[pivot_index] = pivot;

		else
		{
			int picked = rng32_nextInt(rng) % 2;
			new_path[picked ? pivot_index : found_index] = pivot;
		}
	}

	// Preventing useless symmetric representation:
	if (SYMMETRY_PREVENTION && new_path[1] > new_path[length - 1])
		swap(new_path, 1, length - 1);
}


static int *CountBuffer = NULL; // No parallelization possible with this !!!
// This should be added to the context, eventually.

void crossover_2(const void *context, void *rng, void *gene_tofill, const void *gene_1, const void *gene_2,
	double fitness_1, double fitness_2, long epoch)
{
	const Map *map = (Map*) context;
	const int length = map -> CitiesNumber;
	int *path_1 = (int*) gene_1, *path_2 = (int*) gene_2, *new_path = (int*) gene_tofill;

	int start = 1; // First city fixed!
	const int pivot = rng32_nextInt(rng) % length;

	if (!CountBuffer)
		CountBuffer = (int*) calloc(length, sizeof(int));
	else
	{
		for (int i = start; i < length; ++i)
			CountBuffer[i] = 0;
	}

	for (int i = start; i <= pivot; ++i)
	{
		new_path[i] = path_1[i];
		++CountBuffer[path_1[i]];
	}

	for (int i = pivot + 1; i < length; ++i)
	{
		new_path[i] = path_2[i];
		++CountBuffer[path_2[i]];
	}

	// Do _not_ move the counting part to the next loop!

	int begin = start, end = pivot;

	if (rng32_nextInt(rng) % 2)
	{
		begin = pivot + 1;
		end = length - 1;
	}

	for (int i = begin; i <= end; ++i) // Less biased hopefully...
	{
		if (CountBuffer[new_path[i]] == 2)
		{
			int absent_value = findIndexOfValue(CountBuffer, length, start, 0);

			new_path[i] = absent_value;
			start = absent_value + 1;
		}
	}

	// Preventing useless symmetric representation:
	if (SYMMETRY_PREVENTION && new_path[1] > new_path[length - 1])
		swap(new_path, 1, length - 1);
}


// OK
void crossover_3(const void *context, void *rng, void *gene_tofill, const void *gene_1, const void *gene_2,
	double fitness_1, double fitness_2, long epoch)
{
	const Map *map = (Map*) context;
	const int length = map -> CitiesNumber;
	int *path_1 = (int*) gene_1, *path_2 = (int*) gene_2, *new_path = (int*) gene_tofill;

	const int start = 1; // First city fixed!
	const int pivot = rng32_nextInt(rng) % length;

	for (int i = start; i < length; ++i)
	{
		new_path[i] = path_2[i];
	}

	for (int i = start; i <= pivot; ++i)
	{
		int index = findIndexOfValue(path_2, length, start, path_1[i]);

		swap(new_path, i, index);
	}

	// Preventing useless symmetric representation:
	if (SYMMETRY_PREVENTION && new_path[1] > new_path[length - 1])
		swap(new_path, 1, length - 1);
}


// No mutation at all!
void mutation_0(const void *context, void *rng, void *gene, long epoch)
{
}


void mutation_1(const void *context, void *rng, void *gene, long epoch)
{
	const Map *map = (Map*) context;
	const int length = map -> CitiesNumber;
	int *new_path = (int*) gene;

	int city_1, city_2;

	if (SYMMETRY_PREVENTION)
	{
		getStrictCouple(rng, &city_1, &city_2, length - 1); // First city fixed!

		++city_1;
		++city_2;

		// Preventing useless symmetric representation:
		if (city_1 == 1 && new_path[city_2] > new_path[1])
			++city_1; // To not lose a mutation!
	}

	else
	{
		// First city fixed! May be a little faster than the alternative:
		city_1 = 1 + rng32_nextInt(rng) % (length - 1);
		city_2 = 1 + rng32_nextInt(rng) % (length - 1);
	}

	swap(new_path, city_1, city_2);
}


void mutation_2(const void *context, void *rng, void *gene, long epoch)
{
	const Map *map = (Map*) context;
	const int length = map -> CitiesNumber;
	int *new_path = (int*) gene;

	int city_1, city_2;

	getStrictCouple(rng, &city_1, &city_2, length - 1); // First city fixed!

	++city_1;
	++city_2;

	// Preventing useless symmetric representation:
	if (SYMMETRY_PREVENTION && city_1 == 1 && new_path[city_2] > new_path[1])
		++city_1; // To not lose a mutation!

	mirror(new_path, city_1, city_2);
}


const GeneticMethods GeneMeth_salesman_1 =
{
	.createGene = createGene,
	.copyGene = copyGene,
	.destroyGene = destroyGene,
	.fitness = fitness,
	.crossover = crossover_0,
	.mutation = mutation_2,
	.setFitnessUpdateStatus = NULL,

	.selectionMode = SEL_UNIFORM
	// .selectionMode = SEL_PROPORTIONATE
};


const GeneticMethods GeneMeth_salesman_2 =
{
	.createGene = createGene,
	.copyGene = copyGene,
	.destroyGene = destroyGene,
	.fitness = fitness,
	.crossover = crossover_2,
	.mutation = mutation_2,
	.setFitnessUpdateStatus = NULL,

	// .selectionMode = SEL_UNIFORM
	.selectionMode = SEL_PROPORTIONATE
};


const GeneticMethods GeneMeth_salesman_3 =
{
	.createGene = createGene,
	.copyGene = copyGene,
	.destroyGene = destroyGene,
	.fitness = fitness,
	.crossover = crossover_3,
	.mutation = mutation_2,
	.setFitnessUpdateStatus = NULL,

	.selectionMode = SEL_UNIFORM
	// .selectionMode = SEL_PROPORTIONATE
};


// N.B:
// .crossover = crossover_1, // slow...
// .mutation = mutation_0, // terrible
// .mutation = mutation_1, // terrible

// Compiling with -Wunused-function will remove the 'unused static function' warning.
