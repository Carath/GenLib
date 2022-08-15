# Changelog

*Note: version numbers prior to 1.3 were not coupled with proper GenLib versions. They have therefore been retconned, has to both keep a trace of the most important changes in GenLib, and to be consistent with the real versioning.*


## v1.7

- Added a deterministic mode.
- Epochs type changed from 'size_t' to 'long' for convenience.


## v1.6

- Used loop unrolling to speedup the worst gene search.
- Renamed more appropriately 'bestGene' to 'geneBuffer'.
- Fixed first worst gene index not being computed.
- Epochs type changed from 'int' to 'size_t', to be able to search for more than 4 billion epochs.


## v1.5

- Made GenLib API more consistent by forcing the fitness shifting (when GL_SHIFTING_ENABLED = 1), thus fitness values given in the crossover operator do not radically depend on the choice of selection.
- Added an unbiased random initialization option.
- Added more indications on the genetic operators usage.
- Proper git integration, improved the seeding mechanism.


## v1.4

- Added a macro version for GenLib.
- Allow shifts to be negative, in order to remove initial fitness offsets.
- Allowed bestGene to be NULL.
- SEL_PROBABILISTIC renamed to SEL_PROPORTIONATE.


## 1.3

- Added a macro version for GenLib.
- Allow shifts to be negative, in order to remove initial fitness offsets.
- Allowed bestGene to be NULL.
- SEL_PROBABILISTIC renamed to SEL_PROPORTIONATE.


## v1.25

- Added the rng32 wrapper for pcg32.
- Added thread-safety to GenLib thanks to rng32.
- Added a setting to check if the final paths found by SA are actually the best ones found.


## v1.2

- Probabilistic selection now by default, for a genetic search.
- Small improvement in mutations and the stochastic search, when SYMMETRY_PREVENTION is used, but it is still terrible to use.
- Added simulated annealing (SA), and SA with threshold acceptance (TA).


## v1.15

- Symmetry control now works, but it decreases drastically the produced solutions quality...
- Unbiased selection in stochastic search works properly (symmetry control was the culprit), and actually increases solutions quality.
- Removed unused Edges architecture.


## v1.1

- Removed testFixThingsUp(), it was too much of a hassle for no real gain in robustness.
- Added two-op local search.
- Proper inlining of critical functions has been added.
- The first city (that is, of index 0) is now fixed, instead of the last one.

Known issues:
- Unecessary symmetry control, for the genetic and the stochastic search, do not work.
- The terribly biased 'selection' in the stochastic search... is better than the unbiased one...


## v1.05

- Probabilistic/uniform selection is now a runtime parameter.
- Finished the driver for TSPLIB.
- Reintroduced PROBABILISTIC_SELECTION as an option.


## v1.0

- Putting the sinus things in a separate file.
- Regular TSP tests moved to a proper function
- Beginning to work on a driver for TSPLIB. NOT FINISHED!
- Alleviating some GenLib functions
- Fixed the case where epoch_best isn't updated on the last epoch.
- Added crossover_3, and epoch_best.


## v0.95

- Architecture changed. Now the population, fitness_array, etc are stored in the Species struct, which can be used to resume the search on. It just works.
- Default static functions for the GeneticMethods struct have been defined, but the error handling process does not work properly as of now...
