#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"
#include "rng32.h"


// Every field is initialized to 0.
num_map** createFloatMatrix(int rows, int cols)
{
	num_map **matrix = (num_map**) calloc(rows, sizeof(num_map*));

	if (matrix == NULL)
	{
		printf("\nImpossible to allocate enough memory for a matrix.\n\n");
		return NULL;
	}

	for (int i = 0; i < rows; ++i)
	{
		matrix[i] = (num_map*) calloc(cols, sizeof(num_map));

		if (matrix[i] == NULL)
		{
			printf("\nImpossible to allocate enough memory for matrix[%d].\n\n", i);
			return NULL;
		}
	}

	return matrix;
}


void freeFloatMatrix(num_map **matrix, int rows)
{
	if (matrix == NULL)
		return;

	for (int i = 0; i < rows; ++i)
		free(matrix[i]);

	free(matrix);
}


void printFloatMatrix(num_map **matrix, int rows, int cols)
{
	if (matrix == NULL)
		return;

	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
			printf("%8.2f", matrix[i][j]);

		printf("\n");
	}

	printf("\n");
}


void copyFloatMatrix(num_map **source, num_map **dest, int rows, int cols)
{
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
			dest[i][j] = source[i][j];
	}
}


// Filling randomly a num_map matrix with uniform distribution:
void randomFloatMatrix_uniform(void *rng, num_map **matrix, int rows, int cols, num_map min, num_map max)
{
	if (matrix == NULL)
		return;

	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			matrix[i][j] = rng32_nextFloat(rng) * (max - min) + min;
		}
	}
}
