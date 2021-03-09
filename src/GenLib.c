#include <stdio.h>
#include <stdlib.h>

#include "GenLib.h"
#include "get_time.h"
#include "rng32.h"


#define EPSILON 0.00001


////////////////////////////////////////////////////////////////////////////////
// Private genetic functions - problem independant:


// Searching for the gene of lower fitness:
static int indexWorst(const Species *species)
{
	double worst_fitness = species -> fitnessArray[0]; // populationSize > 0
	int index_worst = 0;

	for (int index = 1; index < species -> populationSize; ++index)
	{
		if (species -> fitnessArray[index] < worst_fitness)
		{
			worst_fitness = species -> fitnessArray[index];
			index_worst = index;
		}
	}

	return index_worst;
}


// Searching for the gene of greater fitness:
static int indexBest(const Species *species)
{
	double best_fitness = species -> fitnessArray[0]; // populationSize > 0
	int index_best = 0;

	for (int index = 1; index < species -> populationSize; ++index)
	{
		if (species -> fitnessArray[index] > best_fitness)
		{
			best_fitness = species -> fitnessArray[index];
			index_best = index;
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

		for (int i = 0; i < species -> populationSize; ++i)
			species -> fitnessArray[i] += species -> fitnessShift;
	}
}


// Must be called at least once during the setup phase, and everytime the fitness function changes.
static void updatePopulationFitness(Species *species, int epoch)
{
	species -> sumFitnesses = 0.;

	for (int i = 0; i < species -> populationSize; ++i)
	{
		species -> fitnessArray[i] = species -> genMeth -> fitness(species -> context, species -> population[i], epoch);
		species -> sumFitnesses += species -> fitnessArray[i];
	}

	shiftFitnesses(species);
}


// Selecting a gene from the population, according to the given SelectionMode.
// SEL_PROPORTIONATE: choice made with probability proportional to the ratio between the gene's fitness,
// and the sum of all fitness values. An SEL_UNIFORM selection is worse theorically, but it works and is way faster.
inline static int selection(const Species *species, rng32 *rng)
{
	if (species -> genMeth -> selectionMode == SEL_UNIFORM)
		return rng32_nextInt(rng) % species -> populationSize; // faster, but theorically less good...

	else // SEL_PROPORTIONATE
	{
		const double threshold = species -> sumFitnesses * rng32_nextFloat(rng);
		double partial_sum = 0.;
		int index = 0;

		while (index < species -> populationSize && partial_sum <= threshold)
		{
			partial_sum += species -> fitnessArray[index];
			++index;
		}

		return index - 1;
	}
}


// Replacing the worst gene by a new one, if the latter is better, and if so updates the sum of fitnesses
// and returns 1. Also, assures that no negative fitness can be added when using SEL_PROPORTIONATE.
static int replace(Species *species, int index_worst, int epoch, int *epoch_best)
{
	// Computing the fitness of the newborn gene:
	double new_fitness = species -> fitnessShift + species -> genMeth -> fitness(species -> context, species -> bestGene, epoch);

	if (new_fitness > species -> fitnessArray[index_worst]) // optimization!
	{
		// Updating the sum of the fitness values:
		species -> sumFitnesses += new_fitness - species -> fitnessArray[index_worst];

		// Updating the length of the new path:
		species -> fitnessArray[index_worst] = new_fitness;

		// Replacing the worst path:
		species -> genMeth -> copyGene(species -> context, species -> population[index_worst], species -> bestGene);

		*epoch_best = epoch;

		return 1;
	}

	return 0;
}


// Finds the current gene of best fitness, save it in 'species -> bestGene', and returns its fitness (unshifted).
static double getBestResult(const Species *species)
{
	// Finding the gene of higher fitness:
	int index_best = indexBest(species);

	// Saving the best found gene:
	species -> genMeth -> copyGene(species -> context, species -> bestGene, species -> population[index_best]);

	// Returning the best found fitness:
	return species -> fitnessArray[index_best] - species -> fitnessShift; // shifting the fitness back to normal.
}


////////////////////////////////////////////////////////////////////////////////
// Public genetic functions - problem independant:


// Creating a new species.
Species* createSpecies(const GeneticMethods *genMeth, const void *context, int population_size)
{
	if (!genMeth)
	{
		printf("\nNULL 'genMeth' in 'createSpecies()'.\n");
		return NULL;
	}

	if (population_size < 1)
	{
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
	species -> bestGene = genMeth -> createGene(context, &rng);
	species -> genMeth = genMeth;
	species -> context = context;

	if (!(species -> population) || !(species -> fitnessArray))
	{
		printf("\nNot enough memory to create a new species.\n");
		destroySpecies(&species);
		return NULL;
	}

	// Initializing the population:
	for (int i = 0; i < population_size; ++i)
		species -> population[i] = genMeth -> createGene(context, &rng);

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

	if (!genMeth)
	{
		printf("\nNULL 'genMeth' in 'destroySpecies()'.\n");
		return;
	}

	if ((*species_address) -> population)
	{
		for (int i = 0; i < (*species_address) -> populationSize; ++i)
		{
			if ((*species_address) -> population[i])
				genMeth -> destroyGene(context, (*species_address) -> population[i]);
		}
	}

	if ((*species_address) -> bestGene)
		genMeth -> destroyGene(context, (*species_address) -> bestGene);

	free((*species_address) -> fitnessArray);
	free((*species_address) -> population);
	free(*species_address);
	*species_address = NULL;
}


// Genetic search. A 'good' gene is beeing seeked by evolving from a population, and then saved
// in 'species -> bestGene'. Returns the (unshifted) best found fitness. 'context' may be NULL.
double geneticSearch(Species *species, int epoch_number)
{
	double time_start = get_time();

	if (!species || !species -> genMeth || species -> populationSize < 1 || epoch_number < 0)
	{
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

	int replaced = 0, index_worst = 0, epoch_best = 0;

	for (int epoch = 0; epoch < epoch_number; ++epoch)
	{
		// Checking if the fitness values have to be updated:
		if (genMeth -> setFitnessUpdateStatus && genMeth -> setFitnessUpdateStatus(context, epoch))
			updatePopulationFitness(species, epoch);

		// Selecting two random genes from the given population:
		int index_selected_1 = selection(species, &rng);
		int index_selected_2 = selection(species, &rng);

		// Doing a crossover between the two, and saving the result in the buffer:
		const void *gene_1 = species -> population[index_selected_1];
		const void *gene_2 = species -> population[index_selected_2];

		double fitness_1 = species -> fitnessArray[index_selected_1];
		double fitness_2 = species -> fitnessArray[index_selected_2];

		genMeth -> crossover(context, &rng, species -> bestGene, gene_1, gene_2, fitness_1, fitness_2, epoch);

		// Mutates the newborn gene:
		genMeth -> mutation(context, &rng, species -> bestGene, epoch);

		if (replaced) // Searching for the gene of lower fitness:
			index_worst = indexWorst(species);

		// Replacing the worst gene by a new one, if the latter is better, and if so updates the sum of fitnesses
		// and returns 1. Also, assures that no negative fitness can be added when using SEL_PROPORTIONATE.
		replaced = replace(species, index_worst, epoch, &epoch_best);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Returning the best result:

	double best_fitness = getBestResult(species);
	double elapsed_time = get_time() - time_start;
	double epoch_ratio = (double) epoch_best / epoch_number;

	if (GL_VERBOSE_MODE)
	{
		printf("\nGenetic search:\n -> Time elapsed: %.3f s, epoch ratio: %.3f, best found fitness: %.6f\n\n",
			elapsed_time, epoch_ratio, best_fitness);
	}

	return best_fitness;
}
