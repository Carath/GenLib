# GenLib

*Small genetic algorithm library, and applications*


## How to use

Copy the following files in your project: ``` GenLib.c ```, ``` GenLib.h ```, ``` get_time.c ```, ``` get_time.h ``` and ``` rng32.h ```. The other files are for demonstration purposes.


## Compilation

Compiling is done by running ``` make ```.


## Runtime

For testing the library:

```
./genlibtest.exe
```


## TODO

- Integrate non-genetic local search functions to the core library, without performance loss.
- Try other data structures to speedup the worst gene search for large population size.
- Add an example of a parallel search.
