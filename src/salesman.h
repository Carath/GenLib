#ifndef SALESMAN_H
#define SALESMAN_H


// Settings:

#define DEFAULT_INIT_MODE RANDOM_INIT // RANDOM_INIT doesn't seem to help much...

#define SYMMETRIC_TSP 1
#define SYMMETRY_PREVENTION_OPTION 0 // appealing idea, but terrible in practice... Do _not_ use it!


// Other parameters:

#define DIST_BOUND 20.f // arbitrary.

#define SYMMETRY_PREVENTION (SYMMETRIC_TSP && SYMMETRY_PREVENTION_OPTION) // do not modify this.

#define num_map float
// #define num_map double


typedef enum {EXACT, ROUNDED} DistanceRounding;
typedef enum {RANDOM, CUSTOM} FillingMode;
typedef enum {TRIVIAL_INIT, RANDOM_INIT} InitMode;


typedef struct
{
	const int CitiesNumber;
	num_map **Locations; // CitiesNumber x 2
	num_map **Net; // CitiesNumber x CitiesNumber
} Map;


// Euclidean distance between (x1, y1) and (x2, y2).
num_map distance(num_map x1, num_map y1, num_map x2, num_map y2);


// No need to call initMap() after this if fillMode == RANDOM.
Map* createMap(int citiesNumber, FillingMode fillMode, DistanceRounding distMode);


// Passed by address:
void freeMap(Map **map);


// Must be called after the map has been filled, if fillMode != RANDOM.
void initMap(Map *map, FillingMode fillMode, DistanceRounding distMode);


void printMap(const Map *map);


void printPath(const int *path, int length);


// Init a path, randomly or not, starting from a valid position. First city fixed !!!
void initPath(void *rng, int *path, int length, InitMode initMode);


// Length of the total path, coming back to the start:
num_map pathLength(const Map *map, const int *path);


#endif
