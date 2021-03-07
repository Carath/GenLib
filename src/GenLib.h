////////////////////////////////////////////////////////////////////////////////
// Small library implementing a general purpose genetic search algorithm.
// Said genetic search is thread-safe, and several can be run in parallel on different
// species. The user must provide genetic operators, for GenLib to run the search.
// To use it, copy the files: GenLib.c, GenLib.h, get_time.c, get_time.h and rng32.h.
////////////////////////////////////////////////////////////////////////////////

#ifndef GENLIB_H
#define GENLIB_H

#if __cplusplus
extern "C" {
#endif

#define GENLIB_VERSION 1.4

////////////////////////////////////////////////////////////////////////////////
// Settings:

// Recommended setting. This has a negligeable impact on execution time, yet assures
// that fitness values are correct for use with the SEL_PROPORTIONATE selection, by shifting
// those values so that they are > 0. Incorrect values may result in array overflows!
// In addition, this as the benefit of bringing initial fitness values close to 0, to improve on
// SEL_PROPORTIONATE effects.
#define GL_SHIFTING_ENABLED 1


// Enable messages to be printed at the end of the genetic search, conveying useful information
// such as the elapsed time, the ratio of epochs at which the global best gene has (first) been
// found, and the best found fitness value.
#define GL_VERBOSE_MODE 1

////////////////////////////////////////////////////////////////////////////////
// Genetic struct:

// In order to be able to run several genetic search in parallel, one would need to not use rand(), but
// instead use the Random Number Generator furnished as 'rng', in the crossover, mutation and copyGene operators.
// Instructions on how to use this rng is detailed in the 'rng32.h' file. Note that this rng will be initialed
// internally, and that its use is totally optional, as rand() may still be used for non-parallel searches,
// although doing this may be slightly slower...


// PROBABILISTIC: choice made with probability proportional to the ratio between the gene's fitness, and
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

	// Must allocate dynamically, and initialize.
	void* (*createGene)(const void *context, void *rng);

	// A deep copy may be wanted here.
	void (*copyGene)(const void *context, void *gene_tofill, const void *gene);

	// Free a gene.
	void (*destroyGene)(const void *context, void *gene);

	////////////////////////////////////////////////////////////////////////////////
	// Genetic functions - problem dependant:

	// Fitness function, to evaluate a gene performance. Values must be > 0.
	double (*fitness)(const void *context, const void *gene, int epoch);

	// Crossover beetween two genes:
	void (*crossover)(const void *context, void *rng, void *gene_tofill, const void *gene_1, const void *gene_2,
		double fitness_1, double fitness_2, int epoch);

	// Mutates the newborn gene.
	void (*mutation)(const void *context, void *rng, void *gene, int epoch);

	// Will trigger an updatePopulationFitness() when returning 1. Do not use it too often,
	// for this will slow down the genetic search and hinder the convergence. Can be left to NULL.
	int (*setFitnessUpdateStatus)(const void *context, int epoch);

} GeneticMethods;


////////////////////////////////////////////////////////////////////////////////
// Genetic public function:

typedef struct
{
	const int populationSize;
	void **population;
	double *fitnessArray;
	void *bestGene;
	double sumFitnesses;
	double fitnessShift;

	// Saved only for convenience:
	const GeneticMethods *genMeth;
	const void *context;
} Species;


// Creating a new species.
Species* createSpecies(const GeneticMethods *genMeth, const void *context, int population_size);


// Freeing the given species, passed by address.
void destroySpecies(Species **species_address);


// Genetic search. A 'good' gene is beeing seeked by evolving from a population, and then saved
// in 'species -> bestGene'. Returns the (unshifted) best found fitness. 'context' may be NULL.
double geneticSearch(Species *species, int epoch_number);


#if __cplusplus
}
#endif

#endif
