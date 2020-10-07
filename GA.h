#define GENERATION 3000 // number of generation
#define POPULATION_SIZE 20 // number of population
#define CROSSOVER_RATE 80 // percentage, e.g., 10 stands for  10%
#define MUTATION_RATE 10 // percentage, e.g., 10 stands for  10%
#define KNAPSACK_SIZE 275 // number of max knapsack size
#define GENE_LENGTH 10 // length of gene

/**
 * DEBUG_MODE 1: print debugging information
 *            0: do not print debugging information
 *
 * EACH_ROUND_RESULT 1: print each round result respectively
 *                   0: do not print each round result respectively
 */
#define DEBUG_MODE 0
#define EACH_ROUND_RESULT 0

/**
 * SELECTION_TYPE 0: Tournament Selection
 *                1: Roulette Wheel Selection
 *
 * CROSSOVER_TYPE 0: Single-Point Crossover
 *                1: K-Point Crossover
 *
 * K_POINT_CROSSOVER k: set k-point crossover, it should set between 2 and (GENE_LENGTH - 1)
 *
 * MUTATION_TYPE 0: Single-Point Mutation
 *               1: Multiple-Point Mutation
 */
#define SELECTION_TYPE 0
#define CROSSOVER_TYPE 0
#define K_POINT_CROSSOVER 3
#define MUTATION_TYPE 0
/**
 * CALC_FITNESS_MODE 1:
 *      Calculating fitness by drop the lowest CP ratio item while it's overweight.
 *
 * CALC_FITNESS_MODE 0:
 *      Calculating fitness by punishment coefficient while it's overweight.
 */
#define CALC_FITNESS_MODE 1

typedef struct tag_parent_t {
    int gene[GENE_LENGTH];
    int weight;
    int value;
    int fitness;
} parent_t;

const char name[10] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
const int weight[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const int value[10] = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

void init();

void calcFitness(parent_t *x);

void selectTournament();

void selectRW(); // roulette wheel selection

void crossoverSP(); // single-point crossover

void crossoverKP(); // k-point crossover

void mutateSP(); // single-point mutation

void mutateMP(); // multiple-point mutation

void showResult(); // print result of current round

parent_t *getBestGene();