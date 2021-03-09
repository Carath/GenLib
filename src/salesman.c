#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "salesman.h"
#include "matrix.h"
#include "sales_gen.h" // for swap()
#include "get_time.h" // for create_seed()


// Euclidean distance between (x1, y1) and (x2, y2).
inline num_map distance(num_map x1, num_map y1, num_map x2, num_map y2)
{
	num_map delta_x = x1 - x2, delta_y = y1 - y2;

	return sqrt(delta_x * delta_x + delta_y * delta_y);
}


Map* createMap(int citiesNumber, FillingMode fillMode, DistanceRounding distMode)
{
	Map *map = (Map*) calloc(1, sizeof(Map));

	*(int*) &(map -> CitiesNumber) = citiesNumber;

	map -> Locations = createFloatMatrix(citiesNumber, 2);

	map -> Net = createFloatMatrix(citiesNumber, citiesNumber);

	initMap(map, fillMode, distMode);

	return map;
}


// Passed by address:
void freeMap(Map **map)
{
	if (!*map || !map)
		return;

	freeFloatMatrix((*map) -> Locations, (*map) -> CitiesNumber);
	freeFloatMatrix((*map) -> Net, (*map) -> CitiesNumber);

	free(*map);
	*map = NULL;
}


void initMap(Map *map, FillingMode fillMode, DistanceRounding distMode)
{
	if (map == NULL)
		return;

	if (fillMode == RANDOM)
	{
		rng32 rng;
		uint64_t seed = create_seed(map);
		rng32_init(&rng, seed, 0);

		randomFloatMatrix_uniform(&rng, map -> Locations, map -> CitiesNumber, 2, 0, DIST_BOUND);
	}

	for (int i = 0; i < map -> CitiesNumber; ++i)
	{
		for (int j = 0; j < map -> CitiesNumber; ++j)
		{
			if (SYMMETRIC_TSP && i > j)
			{
				map -> Net[i][j] = map -> Net[j][i];
				continue;
			}

			num_map x1 = map -> Locations[i][0];
			num_map y1 = map -> Locations[i][1];

			num_map x2 = map -> Locations[j][0];
			num_map y2 = map -> Locations[j][1];

			num_map dist = distance(x1, y1, x2, y2);

			if (distMode == ROUNDED)
				dist = (int) (dist + 0.5f); // for TSPLIB

			map -> Net[i][j] = dist;
		}
	}
}


void printMap(const Map *map)
{
	if (map == NULL)
		return;

	printf("\nnum_map of cities: %d\n\n", map -> CitiesNumber);

	printf("Locations:\n\n");
	printFloatMatrix(map -> Locations, map -> CitiesNumber, 2);

	printf("Net:\n\n");
	printFloatMatrix(map -> Net, map -> CitiesNumber, map -> CitiesNumber);
}


void printPath(const int *path, int length)
{
	for (int i = 0; i < length; ++i)
		printf("%2d, ", path[i]);
	printf("\n");
}


// Fisher–Yates shuffle, for an array of integers:
void shuffle(void *rng, int *array, int len)
{
	// In case RNG32_MAX is small, for some (terrible) RNG...
	if (len >= RNG32_MAX)
		printf("\nArray length (%d) is greater or equal to RNG32_MAX!\n\n", len);

	for (unsigned int i = len - 1; i >= 1; --i)
	{
		unsigned int j = rng32_nextInt(rng) % (i + 1); // 0 ≤ j ≤ i. Biased, but negligeable.
		swap(array, i, j);
	}
}


// Init a path, randomly or not, starting from a valid position. First city fixed !!!
void initPath(void *rng, int *path, int length, InitMode initMode)
{
	for (int i = 0; i < length; ++i)
		path[i] = i;

	if (initMode == TRIVIAL_INIT)
		return;

	else if (initMode == BIASED_RANDOM_INIT)
	{
		// const int step_number = length; // too far.
		// No need of more than 'length' transpositions...
		const int step_number = rng32_nextInt(rng) % length;

		for (int step = 0; step < step_number; ++step)
		{
			// 0 fixed!
			int i = 1 + rng32_nextInt(rng) % (length - 1);
			int j = 1 + rng32_nextInt(rng) % (length - 1);
			swap(path, i, j);
		}
	}

	else // FULL_RANDOM_INIT
	{
		// This should be better in theory... but it's not...
		shuffle(rng, path + 1, length - 1); // 0 fixed!
	}

	// Preventing useless symmetric representation:
	if (SYMMETRY_PREVENTION && path[1] > path[length - 1])
		swap(path, 1, length - 1);
}


// Length of the total path, coming back to the start:
num_map pathLength(const Map *map, const int *path)
{
	const int len = map -> CitiesNumber;

	num_map length = map -> Net[path[len - 1]][path[0]];

	for (int i = 0; i < len - 1; ++i)
		length += map -> Net[path[i]][path[i + 1]];

	return length;
}
