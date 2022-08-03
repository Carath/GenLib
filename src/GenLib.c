// #define NDEBUG // before including assert.h, to disable assert() calls.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "GenLib.h"
#include "get_time.h"
#include "rng32.h"

#define EPSILON 0.00001

////////////////////////////////////////////////////////////////////////////////
// Private genetic functions - problem independant:

static int indexWorst(const Species *species)
{
	const int remainder = species -> populationSize % 4;
	double worst_fitness = INFINITY;
	int index_worst = 0;

	// Loop unrolling to gain speed when populationSize is large:
	for (int i = remainder; i < species -> populationSize; i += 4)
	{
		double fitness_0 = species -> fitnessArray[i];
		double fitness_1 = species -> fitnessArray[i+1];
		double fitness_2 = species -> fitnessArray[i+2];
		double fitness_3 = species -> fitnessArray[i+3];

		if (fitness_0 < worst_fitness) {
			worst_fitness = fitness_0;
			index_worst = i;
		}
		if (fitness_1 < worst_fitness) {
			worst_fitness = fitness_1;
			index_worst = i+1;
		}
		if (fitness_2 < worst_fitness) {
			worst_fitness = fitness_2;
			index_worst = i+2;
		}
		if (fitness_3 < worst_fitness) {
			worst_fitness = fitness_3;
			index_worst = i+3;
		}
	}

	// Handling the remaining values:
	for (int i = 0; i < remainder; ++i) {
		double fitness = species -> fitnessArray[i];
		if (fitness < worst_fitness) {
			worst_fitness = fitness;
			index_worst = i;
		}
	}

	return index_worst;
}


// Searching for the gene of greater fitness:
static int indexBest(const Species *species)
{
	double best_fitness = -INFINITY;
	int index_best = 0;

	for (int i = 0; i < species -> populationSize; ++i)
	{
		double fitness = species -> fitnessArray[i];
		if (fitness > best_fitness) {
			best_fitness = fitness;
			index_best = i;
		}
	}

	return index_best;
}


// Shifting the fitness values, as to force them to be > 0,
// which is necessary when using SEL_PROPORTIONATE.
static void shiftFitnesses(Species *species)
{
	if (GL_SHIFTING_ENABLED)
	{
		int index_worst = indexWorst(species);
		double worst_fitness = species -> fitnessArray[index_worst];

		species -> fitnessShift = EPSILON - worst_fitness;
		species -> sumFitnesses += species -> populationSize * species -> fitnessShift;

		for (int i = 0; i < species -> populationSize; ++i) {
			species -> fitnessArray[i] += species -> fitnessShift;
		}
	}
}


// Must be called at least once during the setup phase, and everytime the fitness function changes.
static void updatePopulationFitness(Species *species, size_t epoch)
{
	species -> sumFitnesses = 0.;

	for (int i = 0; i < species -> populationSize; ++i)
	{
		species -> fitnessArray[i] = species -> genMeth -> fitness(species -> context, species -> population[i], epoch);
		species -> sumFitnesses += species -> fitnessArray[i];
	}

	shiftFitnesses(species);
}


// Selecting a gene from the population, according to the given SelectionMode. Note: populationSize must be > 0.
// SEL_PROPORTIONATE: choice made with probability proportional to the ratio between the gene's fitness,
// and the sum of all fitness values. An SEL_UNIFORM selection is worse theorically, but it works and is way faster.
inline static int selection(const Species *species, rng32 *rng)
{
	if (species -> genMeth -> selectionMode == SEL_UNIFORM)
		return rng32_nextInt(rng) % species -> populationSize; // faster, but theorically less good...

	else // SEL_PROPORTIONATE
	{
		assert(species -> sumFitnesses > 0.); // in case fitnesses have not been shifted...
		const double threshold = species -> sumFitnesses * rng32_nextFloat(rng);
		double partial_sum = 0.;
		int i = 0;

		do {
			partial_sum += species -> fitnessArray[i];
			++i;
		}
		while (i < species -> populationSize && partial_sum <= threshold);

		return i - 1;
	}
}


// Replacing the worst gene by a new one if the latter is better, and if so updates the sum of fitnesses
// and 'index_worst'. Also, assures that no negative fitness can be added when using SEL_PROPORTIONATE.
static void replaceWorst(Species *species, int *index_worst, double new_fitness, size_t epoch, size_t *epoch_last_update)
{
	if (*index_worst < 0) {
		*index_worst = indexWorst(species); // Searching for the gene of lower fitness.
	}

	if (new_fitness > species -> fitnessArray[*index_worst]) // optimization!
	{
		// Updating the sum of the fitness values:
		species -> sumFitnesses += new_fitness - species -> fitnessArray[*index_worst];

		// Updating the length of the new gene:
		species -> fitnessArray[*index_worst] = new_fitness;

		// Replacing the worst gene:
		species -> genMeth -> copyGene(species -> context, species -> population[*index_worst], species -> geneBuffer);

		*epoch_last_update = epoch;

		*index_worst = -1; // index_worst will need to be found again.
	}
}


// Finds the current gene of best fitness, save it in 'species -> geneBuffer', and returns its fitness (unshifted).
static double getBestResult(const Species *species)
{
	// Finding the gene of higher fitness:
	int index_best = indexBest(species);

	// Saving the best found gene:
	species -> genMeth -> copyGene(species -> context, species -> geneBuffer, species -> population[index_best]);

	// Returning the best found fitness:
	return species -> fitnessArray[index_best] - species -> fitnessShift; // shifting the fitness back to normal.
}


////////////////////////////////////////////////////////////////////////////////
// Public genetic functions - problem independant:


// Creating a new species. Note: 'population_size' shouldn't be greater than a few thousands to keep the search fast.
Species* createSpecies(const GeneticMethods *genMeth, const void *context, int population_size)
{
	if (!genMeth) {
		printf("\nNULL 'genMeth' in 'createSpecies()'.\n");
		return NULL;
	}

	if (population_size < 1) {
		printf("\nPopulation size must be at least 1.\n");
		return NULL;
	}

	Species *species = (Species*) calloc(1, sizeof(Species));

	rng32 rng; // 32-bit RNG.
	uint64_t seed = create_seed(species);
	rng32_init(&rng, seed, 0);

	*(int*) &(species -> populationSize) = population_size;
	species -> population = (void**) calloc(population_size, sizeof(void*));
	species -> fitnessArray = (double*) calloc(population_size, sizeof(double));
	species -> geneBuffer = genMeth -> createGene(context, &rng);
	species -> genMeth = genMeth;
	species -> context = context;

	if (!(species -> population) || !(species -> fitnessArray)) {
		printf("\nNot enough memory to create a new species.\n");
		destroySpecies(&species);
		return NULL;
	}

	// Initializing the population:
	for (int i = 0; i < population_size; ++i) {
		species -> population[i] = genMeth -> createGene(context, &rng);
	}

	updatePopulationFitness(species, 0);

	return species;
}


// Freeing the given species, passed by address.
void destroySpecies(Species **species_address)
{
	if (!species_address || !*species_address)
		return;

	const GeneticMethods *genMeth = (*species_address) -> genMeth;
	const void *context = (*species_address) -> context;

	if (!genMeth) {
		printf("\nNULL 'genMeth' in 'destroySpecies()'.\n");
		return;
	}

	if ((*species_address) -> population)
	{
		for (int i = 0; i < (*species_address) -> populationSize; ++i)
		{
			if ((*species_address) -> population[i]) {
				genMeth -> destroyGene(context, (*species_address) -> population[i]);
			}
		}
	}

	if ((*species_address) -> geneBuffer) {
		genMeth -> destroyGene(context, (*species_address) -> geneBuffer);
	}

	free((*species_address) -> fitnessArray);
	free((*species_address) -> population);
	free(*species_address);
	*species_address = NULL;
}


// Genetic search. 'Good' genes are beeing seeked by evolving from a population, and the best
// found gene is saved in 'species -> geneBuffer' and its (unshifted) fitness is returned.
double geneticSearch(Species *species, size_t epoch_number)
{
	double time_start = get_time();

	if (!species || !species -> genMeth || species -> populationSize < 1) {
		printf("\nInvalid argument in 'geneticSearch'.\n\n");
		return 0.;
	}

	const GeneticMethods *genMeth = species -> genMeth;
	const void *context = species -> context;

	rng32 rng; // 32-bit RNG.
	uint64_t seed = create_seed(species);
	rng32_init(&rng, seed, 0);

	////////////////////////////////////////////////////////////////////////////////
	// Starting the evolution process:

	size_t epoch_last_update = 0;
	int index_worst = -1;

	for (size_t epoch = 0; epoch < epoch_number; ++epoch)
	{
		// Checking if the fitness values have to be updated:
		if (genMeth -> setFitnessUpdateStatus && genMeth -> setFitnessUpdateStatus(context, epoch)) {
			updatePopulationFitness(species, epoch);
		}

		// Selecting two random genes from the given population:
		int index_selected_1 = selection(species, &rng);
		int index_selected_2 = selection(species, &rng);

		const void *gene_1 = species -> population[index_selected_1];
		const void *gene_2 = species -> population[index_selected_2];

		double fitness_1 = species -> fitnessArray[index_selected_1];
		double fitness_2 = species -> fitnessArray[index_selected_2];

		// Doing a crossover between the two selected genes, and saving the result in the buffer:
		genMeth -> crossover(context, &rng, species -> geneBuffer, gene_1, gene_2, fitness_1, fitness_2, epoch);

		// Mutates the newborn gene:
		genMeth -> mutation(context, &rng, species -> geneBuffer, epoch);

		// Computing the fitness of the newborn gene:
		double new_fitness = species -> fitnessShift + genMeth -> fitness(context, species -> geneBuffer, epoch);

		// Replacing the worst gene by a new one if the latter is better, and if so updates the sum of fitnesses
		// and 'index_worst'. Also, assures that no negative fitness can be added when using SEL_PROPORTIONATE.
		replaceWorst(species, &index_worst, new_fitness, epoch, &epoch_last_update);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Returning the best result:

	double best_fitness = getBestResult(species);
	double elapsed_time = get_time() - time_start;
	double epoch_ratio = (double) epoch_last_update / epoch_number;

	if (GL_VERBOSE_MODE) {
		printf("\nGenetic search:\n -> Time elapsed: %.3f s, epoch ratio: %.3f, best found fitness: %.6f\n\n",
			elapsed_time, epoch_ratio, best_fitness);
	}

	return best_fitness;
}
