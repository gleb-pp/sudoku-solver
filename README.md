# Sudoku Solver



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
2. Construct new sudoku fileds (children) from two adjacent existing ones (parents): take odd rows from one parent and even rows from another.
3. Making **mutation** in the child: swapping two random non-initial numbers in the row.
4. With probability `CRAZY_MUTATION_INITIAL_PERCENT` shuffle all non-initial numbers in the row.

### Remove Dregs

The model removes the worst sudoku fileds.

1. Measuring **fintenss function** of each field within the population: 
$$
\text{fitness} = \left( 
\sum_{\text{columns}} \sum_{k=1}^{9} (\text{count}_k - 1) \cdot \text{count}_k + 
\sum_{\text{grids}} \sum_{k=1}^{9} (\text{count}_k - 1) \cdot \text{count}_k 
\right)^2
$$
2. Remove the `POPULATION_SIZE / 2` worst entities (with the highest fitness function).

### Stochastic Processing

The model may get stuck in one solution and remain there for a long time.

1. If there was `MAX_REPEATING_RESULT` stochastic generations (i.e. without decreasing of minimal fitness value), then we increase the probability of crazy mutation to `CRAZY_MUTATION_MAXIMUM_PERCENT`.
2. If there was `MAX_REPEATING_RESULT` stochastic generations after that, we create a totally new population.
