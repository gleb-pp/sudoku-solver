# Sudoku Solver

This project implements a Sudoku solver based on a genetic algorithm. Instead of traditional backtracking, it uses evolutionary ideas: population, crossover, and mutations.

#### Input
```
4 - - 6 - - 5 - -
- - 2 - - 1 6 - -
- 1 - - - - - - -
- - 7 - - 6 - 1 8
- - - - 8 4 - 5 3
- 3 - 1 - - - - 6
8 - 3 - - - - - 2
1 - - - 2 8 3 7 4
- - - - - - 1 - -
```

#### Output
```
4 8 9 6 7 2 5 3 1 
7 5 2 8 3 1 6 4 9 
3 1 6 5 4 9 8 2 7 
5 4 7 3 9 6 2 1 8 
6 9 1 2 8 4 7 5 3 
2 3 8 1 5 7 4 9 6 
8 7 3 4 1 5 9 6 2 
1 6 5 9 2 8 3 7 4 
9 2 4 7 6 3 1 8 5 
```

## Preprocessing

Before applying the genetic algorithm, the model checks if there are any numbers whose positions are already obvious at the initial stage.

1. Detecting if there is a cell with only one possible number to put in it.
2. Detecting if there is only one possible cell in the **square** to put some digit.
3. Detecting if there is only one possible cell in the **row** to put some digit.
4. Detecting if there is only one possible cell in the **column** to put some digit.

## Genetic Algorithm

### Parameters
```
POPULATION_SIZE = 100;
MAX_REPEATING_RESULT = 300;
CRAZY_MUTATION_INITIAL_PERCENT = 2;
CRAZY_MUTATION_MAXIMUM_PERCENT = 40;
```

### Create Population

The model creates the population of sudoku fields.

1. The model creates `POPULATION_SIZE` sudoku fields with the initial digits in it.
2. For all initially empty cells, the model randomly chooses the digit to put in it from the list of allowed digits.

### Make Children

The model creates new entities of the sudoku fields. 

1. Shuffling the entire population.
2. Construct new sudoku fields (children) from two adjacent existing ones (parents): take odd rows from one parent and even rows from another.
3. Making **mutation** in the child: swapping two random non-initial numbers in the row.
4. With probability `CRAZY_MUTATION_INITIAL_PERCENT` shuffle all non-initial numbers in the row.

### Remove Dregs

The model removes the worst sudoku fields.

1. Measuring **fitness function** of each field within the population: 
![fitness](https://latex.codecogs.com/svg.latex?\text{fitness}=\left(\sum_{\text{columns}}\sum_{k=1}^{9}(\text{count}_k-1)\cdot\text{count}_k+\sum_{\text{grids}}\sum_{k=1}^{9}(\text{count}_k-1)\cdot\text{count}_k\right)^2)
2. Remove the `POPULATION_SIZE / 2` worst entities (with the highest fitness function).

### Stochastic Processing

The model may get stuck in one solution and remain there for a long time.

1. If there was `MAX_REPEATING_RESULT` stochastic generations (i.e. without decreasing of minimal fitness value), then we increase the probability of crazy mutation to `CRAZY_MUTATION_MAXIMUM_PERCENT`.
2. If there was `MAX_REPEATING_RESULT` stochastic generations after that, we create a totally new population.
