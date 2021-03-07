#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "local_search.h"
#include "sales_gen.h"
#include "get_time.h"
#include "rng32.h"


#define EPSILON 0.000001


static const char *LC_StringArray[] = {"STOCHASTIC", "GREEDY", "SA", "TA"}; // hardcoded for now.


// Stochastically greedy, can easily be trapped in local minima, although it may still go out early on.
// Still, close in solutions quality to GA for small-medium problems, but quite faster. Will always output
// the best found solution found during the run.
static void stochastic_method(const void *context, const Map *map, void *rng, int **population, int population_size, int epoch_number)
{
	epoch_number /= population_size; // To be fair compared to previous algorithms.

	const int cities_number = map -> CitiesNumber;

	for (int epoch = 0; epoch < epoch_number; ++epoch)
	{
		for (int path_index = 0; path_index < population_size; ++path_index)
		{
			int *path = population[path_index];

			int i, j;

			getStrictCouple(rng, &i, &j, cities_number - 1); // First city fixed!

			++i;
			++j;

			// Preventing useless symmetric representation:
			if (SYMMETRY_PREVENTION && i == 1 && path[j] > path[1])
				++i; // To not lose a mutation!

			int city_pi = path[i - 1], city_i = path[i];
			int city_j = path[j], city_sj = j == cities_number - 1 ? 0 : path[j + 1];

			// Old length - new length:
			double delta = map -> Net[city_pi][city_i] + map -> Net[city_j][city_sj]
						 - map -> Net[city_pi][city_j] - map -> Net[city_i][city_sj];

			if (delta > EPSILON) // Shorter path! Mirroring the subpath of indexes [i, j]:
			{
				mirror(path, i, j);
			}
		}
	}
}


// ...
static void simulated_annealing(const void *context, const Map *map, void *rng, int **population, int population_size, int epoch_number)
{
	float temperature = *(float*) context;

	epoch_number /= population_size; // To be fair compared to previous algorithms.

	const int cities_number = map -> CitiesNumber;

	double *best_found_length_array = NULL;

	if (SAVE_BEST_PATH)
	{
		best_found_length_array = (double*) calloc(population_size, sizeof(double));

		for (int path_index = 0; path_index < population_size; ++path_index)
		{
			best_found_length_array[path_index] = pathLength(map, population[path_index]); // initial length.
		}
	}

	for (int epoch = 0; epoch < epoch_number; ++epoch)
	{
		for (int path_index = 0; path_index < population_size; ++path_index)
		{
			int *path = population[path_index];

			int i, j;

			getStrictCouple(rng, &i, &j, cities_number - 1); // First city fixed!

			++i;
			++j;

			// Preventing useless symmetric representation:
			if (SYMMETRY_PREVENTION && i == 1 && path[j] > path[1])
				++i; // To not lose a mutation!

			int city_pi = path[i - 1], city_i = path[i];
			int city_j = path[j], city_sj = j == cities_number - 1 ? 0 : path[j + 1];

			// Old length - new length:
			double delta = map -> Net[city_pi][city_i] + map -> Net[city_j][city_sj]
						 - map -> Net[city_pi][city_j] - map -> Net[city_i][city_sj];

			float move_probability = delta > 0. ? 1.f : expf(delta / temperature); // ... precomputing?
			// float move_probability = 1.f / (1.f + expf(-delta / temperature));

			float roll = rng32_nextFloat(rng);

			if (roll < move_probability)
			{
				mirror(path, i, j);

				if (SAVE_BEST_PATH)
				{
					double new_length = pathLength(map, path);

					if (new_length < best_found_length_array[path_index])
						best_found_length_array[path_index] = new_length;
				}
			}
		}

		temperature *= SA_TEMP_MULTIPLIER;
	}

	printf("Final temperature: %f\n", temperature);

	if (SAVE_BEST_PATH)
	{
		for (int path_index = 0; path_index < population_size; ++path_index)
		{
			if (best_found_length_array[path_index] < pathLength(map, population[path_index]))
				printf("Best found path than final for index %2d: %.3f\n",
					path_index, best_found_length_array[path_index]);
		}

		free(best_found_length_array);
	}
}


// ...
static void threshold_acceptance(const void *context, const Map *map, void *rng, int **population, int population_size, int epoch_number)
{
	float temperature = *(float*) context;

	epoch_number /= population_size; // To be fair compared to previous algorithms.

	const int cities_number = map -> CitiesNumber;

	for (int epoch = 0; epoch < epoch_number; ++epoch)
	{
		for (int path_index = 0; path_index < population_size; ++path_index)
		{
			int *path = population[path_index];

			int i, j;

			getStrictCouple(rng, &i, &j, cities_number - 1); // First city fixed!

			++i;
			++j;

			// Preventing useless symmetric representation:
			if (SYMMETRY_PREVENTION && i == 1 && path[j] > path[1])
				++i; // To not lose a mutation!

			int city_pi = path[i - 1], city_i = path[i];
			int city_j = path[j], city_sj = j == cities_number - 1 ? 0 : path[j + 1];

			// Old length - new length:
			double delta = map -> Net[city_pi][city_i] + map -> Net[city_j][city_sj]
						 - map -> Net[city_pi][city_j] - map -> Net[city_i][city_sj];

			if (delta > -temperature)
			{
				mirror(path, i, j);
			}
		}

		temperature *= SA_TEMP_MULTIPLIER;
	}

	printf("Final temperature: %f\n", temperature);
}


// Completely deterministic - and quite greedy! This _will_ get stuck in local minima.
// This requires the population to be initialized randomly (else, better have a 'population_size' of 1).
// Empirically provides good results, but squales quadratically with the number of cities.
static void greedy_method(const void *context, const Map *map, void *rng, int **population, int population_size, int epoch_number)
{
	epoch_number /= population_size; // To be fair compared to previous algorithms.

	const int cities_number = map -> CitiesNumber;

	for (int epoch = 0; epoch < epoch_number; ++epoch)
	{
		int change_number = 0;

		for (int path_index = 0; path_index < population_size; ++path_index)
		{
			int *path = population[path_index];

			double max_delta = EPSILON;
			int best_i = 0, best_j = 0;

			for (int i = 1; i < cities_number - 1; ++i)
			{
				for (int j = i + 1; j < cities_number; ++j)
				{
					// Preventing useless symmetric representation:
					if (SYMMETRY_PREVENTION && i == 1 && path[j] > path[1])
						continue;

					int city_pi = path[i - 1], city_i = path[i];
					int city_j = path[j], city_sj = j == cities_number - 1 ? 0 : path[j + 1];

					// Old length - new length:
					double delta = map -> Net[city_pi][city_i] + map -> Net[city_j][city_sj]
								 - map -> Net[city_pi][city_j] - map -> Net[city_i][city_sj];

					if (delta > max_delta)
					{
						max_delta = delta;
						best_i = i;
						best_j = j;
					}
				}
			}

			if (max_delta > EPSILON) // Shorter path! Mirroring the subpath of indexes [i, j]:
			{
				mirror(path, best_i, best_j);
				++change_number;
			}
		}

		// The following stopping mechanisms would hinder tasks parallelization:

		if ((float) change_number / population_size < STOPPING_THRESHOLD)
		{
			printf("\nStopping! (change number: %d)\n", change_number);
			return;
		}
	}
}


// Local search using 2-op heuristic. Returns the best found length:
double localSearch(const void *context, const Map *map, int population_size, int epoch_number, localSearchMode mode)
{
	printf("\nLocal search mode: %s\n", LC_StringArray[mode]);

	double time_start = get_time();

	int **population = (int**) calloc(population_size, sizeof(int*));

	if (!map || !population)
	{
		printf("Big error!\n");
		exit(EXIT_FAILURE);
	}

	const int cities_number = map -> CitiesNumber;

	if (cities_number <= 3)
	{
		printf("Trivial case, to be handled separately.\n");
		exit(EXIT_FAILURE);
	}

	rng32 rng;
	uint64_t seed = create_seed(population);
	rng32_init(&rng, seed, 0);

	// Initialization:

	for (int i = 0; i < population_size; ++i)
	{
		population[i] = (int*) calloc(cities_number, sizeof(int));

		if (population[i] == NULL)
		{
			printf("Memory error.\n");
			exit(EXIT_FAILURE);
		}

		initPath(&rng, population[i], cities_number, RANDOM_INIT); // RANDOM_INIT best for greedy_method()
	}

	// Search:

	if (mode == STOCHASTIC)
		stochastic_method(context, map, &rng, population, population_size, epoch_number);
	else if (mode == GREEDY)
		greedy_method(context, map, &rng, population, population_size, epoch_number);
	else if (mode == SA)
		simulated_annealing(context, map, &rng, population, population_size, epoch_number);
	else if (mode == TA)
		threshold_acceptance(context, map, &rng, population, population_size, epoch_number);
	else
	{
		printf("\nUnsupported local search mode.\n");
		exit(EXIT_FAILURE);
	}

	// Getting the best found path:

	double best_length = +INFINITY;

	int *best_path = (int*) calloc(cities_number, sizeof(int));

	for (int i = 0; i < population_size; ++i)
	{
		double current_length = pathLength(map, population[i]);

		if (current_length < best_length)
		{
			best_length = current_length;

			memcpy(best_path, population[i], cities_number * sizeof(int));
		}
	}

	double elapsed_time = get_time() - time_start;

	printf("\n2-opt search:\n -> Time elapsed: %.3f s, best found length: %.3f\n\nBest path:\n",
		elapsed_time, best_length);

	// Printing the best found path:

	printPath(best_path, cities_number);

	// Freeing everything:

	free(best_path);

	for (int i = 0; i < population_size; ++i)
		free(population[i]);

	free(population);

	return best_length;
}
