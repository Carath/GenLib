#ifndef MATRIX_H
#define MATRIX_H


#include "salesman.h"


// Every field is initialized to 0.
num_map** createFloatMatrix(int rows, int cols);


void freeFloatMatrix(num_map **matrix, int rows);


void printFloatMatrix(num_map **matrix, int rows, int cols);


// Filling randomly a num_map matrix with uniform distribution:
void randomFloatMatrix_uniform(void *rng, num_map **matrix, int rows, int cols, num_map min, num_map max);


#endif
