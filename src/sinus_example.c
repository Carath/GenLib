#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "sinus_example.h"
#include "GenLib.h"
#include "rng32.h" // optional


#define PI 3.14159265359
#define MUT_RANGE 0.3


void* createGene(const void *context, void *rng)
{
	double *new_gene = (double*) calloc(1, sizeof(double));

	// *new_gene = 2. * PI * (double) rand() / RAND_MAX;
	*new_gene = 2. * PI * rng32_nextFloat(rng); // optional

	return new_gene;
}


void copyGene(const void *context, void *gene_tofill, const void *gene)
{
	*(double*) gene_tofill = *((double*) gene);
}


void destroyGene(const void *context, void *gene)
{
	free(gene);
}


double fitness(const void *context, const void *gene, long epoch)
{
	return sin(*(double*) gene);
}


void crossover(const void *context, void *rng, void *new_gene, const void *gene_1, const void *gene_2,
	double fitness_1, double fitness_2, long epoch)
{
	*(double*) new_gene = (*(double*) gene_1 + *(double*) gene_2) / 2.f;

	// double ratio = fitness_1 / (fitness_1 + fitness_2);
	// *(double*) new_gene = ratio * *(double*) gene_1 + (1.f - ratio) * *(double*) gene_2;
}


void mutation(const void *context, void *rng, void *gene, long epoch)
{
	// double threshold = ((double) rand() / RAND_MAX * 2.f * MUT_RANGE - MUT_RANGE) / (epoch + 1);
	double threshold = (rng32_nextFloat(rng) * 2.f * MUT_RANGE - MUT_RANGE) / (epoch + 1); // optional

	double *gene_ = (double*) gene;

	*gene_ += threshold;

	if (*gene_ < 0.f)
		*gene_ = 0.f;

	if (*gene_ > 2. * PI)
		*gene_ = 2. * PI;
}


const GeneticMethods GeneMeth_sinus =
{
	.createGene = createGene,
	.copyGene = copyGene,
	.destroyGene = destroyGene,
	.fitness = fitness,
	.crossover = crossover,
	.mutation = mutation,
	.setFitnessUpdateStatus = NULL,
	.selectionMode = SEL_PROPORTIONATE
};


void sinus_example(void)
{
	int population_size = 64;
	long epoch_number = 100000L;

	Species *species_sinus = createSpecies(&GeneMeth_sinus, NULL, population_size);

	geneticSearch(species_sinus, epoch_number);

	double bestResult = *(double*) species_sinus -> geneBuffer;
	double ref = PI / 2.;
	printf("Best found value: %.12f\nExpected value:   %.12f\nRelative error: %.3e\n",
		bestResult, ref, fabs(bestResult - ref) / ref);

	destroySpecies(&species_sinus);
}
