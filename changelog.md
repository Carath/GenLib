# v2.4

Added a macro version for GenLib.


# v2.3.3

- Allow shifts to be negative, in order to remove initial fitness offsets.


# v2.3.1

- Allowed bestGene to be NULL.
- SEL_PROBABILISTIC renamed to SEL_PROPORTIONATE.


# v2.3

- Added the rng32 wrapper for pcg32.
- Added thread-safety to GenLib thanks to rng32.


# v2.2.7.1

Added a setting to check if the final paths found by SA are actually the best ones found.


# v2.2.7

Probabilistic selection now by default, for a genetic search.
Small improvement in mutations and the stochastic search, when SYMMETRY_PREVENTION is used,
  but it is still terrible to use.
Added simulated annealing (SA), and SA with threshold acceptance (TA).


# v2.2.6

Symmetry control now works, but it decreases drastically the produced solutions quality...
Unbiased selection in stochastic search works properly (symmetry control was the culprit),
  and actually increases solutions quality.
Removed unused Edges architecture.


# v2.2.5

Removed testFixThingsUp(), it was too much of a hassle for no real gain in robustness.
Added two-op local search.
Proper inlining of critical functions has been added.
The first city (that is, of index 0) is now fixed, instead of the last one.

Known issues:
- Unecessary symmetry control, for the genetic and the stochastic search, do not work.
- The terribly biased 'selection' in the stochastic search... is better than the unbiased one...


# v2.2.4

Probabilistic/uniform selection is now a runtime parameter.


# v2.2.3

Finished the driver for TSPLIB.
Reintroduced PROBABILISTIC_SELECTION as an option.


# v2.2.2

- putting the sinus things in a separate file.
- regular TSP tests moved to a proper function
- beginning to work on a driver for TSPLIB. NOT FINISHED!
- alleviating some GenLib functions
- Fixed the case where epoch_best isn't updated on the last epoch.


# v2.2.1

Added crossover_3, and epoch_best.


# v2.2

Architecture changed. Now the population, fitness_array, etc are stored
in the Species struct, which can be used to resume the search on. It just works.
Default static functions for the GeneticMethods struct have been defined, but the
error handling process does not work properly as of now...
