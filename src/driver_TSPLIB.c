#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> // for isdigit()

#include "driver_TSPLIB.h"


#define BUFFER_SIZE 100


Map* getMapFromDataset(const char *filename, DistanceRounding distMode)
{
	FILE *file = fopen(filename, "r");

	if (!file)
	{
		printf("\nFile '%s' not found.\n", filename);
		exit(EXIT_FAILURE);
	}

	char buffer[BUFFER_SIZE] = {0};
	long end_header_pos = 0;

	while (fgets(buffer, BUFFER_SIZE, file) && buffer[0] != ' ' && !isdigit(buffer[0]))
		end_header_pos = ftell(file); // saving header's end position.

	// printf("buffer: %s\n", buffer);

	// Going back to header's end:
	fseek(file, end_header_pos, SEEK_SET);

	int rank, cities_number = 0;
	float x, y;

	while (fscanf(file, "%d %f %f", &rank, &x, &y) == 3)
		++cities_number;

	// printf("Cities number: %d\n", cities_number);

	// Going back to header's end:
	fseek(file, end_header_pos, SEEK_SET);

	Map *map = createMap(cities_number, CUSTOM, distMode);

	int city_index = 0;

	while (city_index < cities_number && fscanf(file, "%d %f %f", &rank, &x, &y) == 3)
	{
		map -> Locations[city_index][0] = x;
		map -> Locations[city_index][1] = y;
		// printf("%5d %11.3f %11.3f\n", rank, x, y);
		++city_index;
	}

	fclose(file);

	initMap(map, CUSTOM, distMode);

	// printMap(map);

	return map;
}
