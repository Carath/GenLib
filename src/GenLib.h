////////////////////////////////////////////////////////////////////////////////
// Small library implementing a general purpose genetic search algorithm.
// Said library is thread-safe if used correctly, i.e several searches can be run in parallel
// on different species. The user must provide genetic operators, for GenLib to run the search.
// To use it, copy the files: GenLib.c, GenLib.h, get_time.c, get_time.h and rng32.h.
////////////////////////////////////////////////////////////////////////////////

#ifndef GENLIB_H
#define GENLIB_H

#if __cplusplus
extern "C" {
#endif

#define GENLIB_VERSION 1.7

////////////////////////////////////////////////////////////////////////////////
// Settings:

// Recommended setting. This has a negligeable impact on execution time, yet assures
// that fitness values are correct for use with the SEL_PROPORTIONATE selection, by shifting
// those values so that they are > 0. Incorrect values may result in array overflows!
// In addition, this as both the benefit of enabling the use of negative fitness values,
// and of bringing initial fitness values close to 0, to improve on SEL_PROPORTIONATE effects.
#define GL_SHIFTING_ENABLED 1


// Enable messages to be printed at the end of the genetic search, conveying useful information
// such as the elapsed time, the ratio of epochs at which the global best gene has (first) been
// found, and the best found fitness value.
#define GL_VERBOSE_MODE 1

// For speed benchmarks:
#define GL_DETERMINISTIC 0
#define GL_DEFAULT_SEED 123456 // used when GL_DETERMINISTIC = 1

////////////////////////////////////////////////////////////////////////////////
// Genetic struct:

// In order to be able to run several genetic searches in parallel, one would need to not use rand(), but
// instead use the Random Number Generator furnished as 'rng', in the crossover, mutation and copyGene operators.
// Instructions on how to use this rng are detailed in the 'rng32.h' file. Note that this rng will be initialized
// internally, and that its use is totally optional, as rand() may still be used for non-parallel searches,
// although doing this may be slower...


// SEL_PROPORTIONATE: choice made with probability proportional to the ratio between the gene's fitness, and
// the sum of all fitness values. An UNIFORM selection is worse theorically, but it works and is way faster.
typedef enum {SEL_PROPORTIONATE, SEL_UNIFORM} SelectionMode;


// N.B: genes, and context can be numerical types, structs, NULL, or dynamically allocated arrays.
typedef struct
{
	////////////////////////////////////////////////////////////////////////////////
	// Choice of selection mechanism.
	SelectionMode selectionMode;

	////////////////////////////////////////////////////////////////////////////////
	// Gene basic functions:

	// Must allocate dynamically, and initialize the gene.
	void* (*createGene)(const void *context, void *rng);

	// A deep copy may be needed here, depending on the gene type.
	void (*copyGene)(const void *context, void *gene_tofill, const void *gene);

	// Freeing a gene.
	void (*destroyGene)(const void *context, void *gene);

	////////////////////////////////////////////////////////////////////////////////
	// Genetic functions - problem dependant:

	// Fitness function, to evaluate a gene performance. The higher the value, the better the gene must be.
	// As long as GL_SHIFTING_ENABLED = 1, there is no limitation on the sign of the fitness values.
	// Thus a maximization problem can easily be transformed to a minimization one, by playing with the sign.
	// Furthermore, the outputs of this function can be made to change through time - time being represented by 'epoch'.
	double (*fitness)(const void *context, const void *gene, long epoch);

	// Crossover beetween two genes. Note that the given fitness values will be shifted, as to be > 0.
	void (*crossover)(const void *context, void *rng, void *gene_tofill, const void *gene_1, const void *gene_2,
		double fitness_1, double fitness_2, long epoch);

	// Mutates the newborn gene. A mutation probability (e.g of 0.1) can easily be added by doing inside the function:
	// if (rng32_nextFloat(rng) < 0.1f) { /* do the mutation */ }
	void (*mutation)(const void *context, void *rng, void *gene, long epoch);

	// Will trigger an updatePopulationFitness() when returning 1. Do not use it too often,
	// for this will slow down the genetic search and hinder the convergence. Can be left to NULL.
	int (*setFitnessUpdateStatus)(const void *context, long epoch);

} GeneticMethods;


////////////////////////////////////////////////////////////////////////////////
// Genetic public function:

typedef struct
{
	const int populationSize;
	void **population;
	double *fitnessArray; // may contain *shifted* fitnesses.
	void *geneBuffer;
	double sumFitnesses;
	double fitnessShift;

	// Saved here for convenience:
	const GeneticMethods *genMeth;
	const void *context; // This represents the environment. It can be NULL.
} Species;


// Creating a new species. Note: 'population_size' shouldn't be greater than a few thousands to keep the search fast.
Species* createSpecies(const GeneticMethods *genMeth, const void *context, int population_size);


// Freeing the given species, passed by address.
void destroySpecies(Species **species_address);


// Genetic search. 'Good' genes are beeing seeked by evolving from a population, and the best
// found gene is saved in 'species -> geneBuffer' and its (unshifted) fitness is returned.
double geneticSearch(Species *species, long epoch_number);


#if __cplusplus
}
#endif

#endif
