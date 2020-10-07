#include "GA.h"
#include <iostream>
#include <random>
#include <iomanip>
#include <cstring>

using namespace std;

parent_t population[POPULATION_SIZE]; // population pool
parent_t pool[POPULATION_SIZE]; // crossover pool
parent_t bestGene;

void showState(); // print detail state of population. mainly use it as debugging.

int myRandom(int, int); // return a random number

void quickSort(int *, int, int);

void swap(int *, int *);


void init() {
    for (int i = 0; i < POPULATION_SIZE; i++) {
        for (int j = 0; j < GENE_LENGTH; j++) {
            population[i].gene[j] = myRandom(0, 10);
        }
        calcFitness(&population[i]);

        // update the bestGene
        if (i == 0) {
            memcpy(&bestGene, &population[i], sizeof(parent_t));
        } else if (population[i].fitness > bestGene.fitness) {
            memcpy(&bestGene, &population[i], sizeof(parent_t));
        }
    }

#if DEBUG_MODE
    cout << "========== Initialization ==========" << endl;
    showState();
#endif
}

void processOW(parent_t *x) {
    // drop an item which is the lowest cost-performance ratio one
    for (int i = GENE_LENGTH - 1; i >= 0; i--) {
        if (x->gene[i] > 0) {
            x->gene[i]--;
            x->weight -= weight[i];
            x->value -= value[i];

            if (x->weight <= KNAPSACK_SIZE) {
                x->fitness = x->value;
                break;
            } else
                i++;
        }
    }
}

void calcFitness(parent_t *x) {
    x->weight = 0;
    x->value = 0;

    // calculate its weight and value
    for (int j = 0; j < GENE_LENGTH; j++) {
        x->weight += x->gene[j] * weight[j];
        x->value += x->gene[j] * value[j];
    }

    // calculate its fitness
    if (x->weight <= KNAPSACK_SIZE) {
        x->fitness = x->value;
    } else {
#if CALC_FITNESS_MODE
        // drop a lowest CP ratio item
        processOW(x);
#else
        // it got a punishment coefficient if it is overweight
        x->fitness = x->value * (0.9 - ((x->weight - KNAPSACK_SIZE) / (float) (2 * KNAPSACK_SIZE)));
#endif
    }
}

void selectTournament() {
#if DEBUG_MODE
    cout << "========== Tournament Selection ==========" << endl;
#endif

    int iPool1, iPool2;

    for (int i = 0; i < POPULATION_SIZE; i++) {
        // pick 2 individuals randomly
        iPool1 = myRandom(0, POPULATION_SIZE - 1);
        do {
            iPool2 = myRandom(0, POPULATION_SIZE - 1);
        } while (iPool1 == iPool2);

        // compare their fitness and then put it in crossover pool
        if (population[iPool1].fitness > population[iPool2].fitness) {
            memcpy(&pool[i], &population[iPool1], sizeof(parent_t));
        } else {
            memcpy(&pool[i], &population[iPool2], sizeof(parent_t));
        }

#if DEBUG_MODE
        cout << "picked genes: index: [" << iPool1 << "] and [" << iPool2 << "]" << endl;
        if (population[iPool1].fitness > population[iPool2].fitness) {
            cout << "selected index: [" << iPool1 << "]\tfitness: " << population[iPool1].fitness << endl;
        } else {
            cout << "selected index: [" << iPool2 << "]\tfitness: " << population[iPool2].fitness << endl;
        }
        cout << "----------" << endl;
#endif
    }
}

/** Roulette Wheel Selection
 * probability = fitness / totalFitness
 *
 * E.g., we have four individuals, and its probability is as below respectively:
 *  A    B   C   D
 * 0.15 0.3 0.2 0.35 -> 15% 30% 20% 35%
 *
 * randomly generate a number 'arrow' belongs to [0, 99]
 * [0, 15) -> arrow points to A
 * [15, 45) -> arrow points to B
 * [45, 65) -> arrow points to C
 * [65, 100) -> arrow points to D
 *
 * so now we could generalize a conclusion from above example
 * scope[0] belongs to [0, p1)
 * scope[1] belongs to [p1, p1+p2)
 * scope[2] belongs to [p1+p2, p1+p2+p3)
 * scope[3] belongs to [p1+p2+p3, p1+p2+p3+p4)
 */
void selectRW() {
    float probabilities[POPULATION_SIZE], scope[POPULATION_SIZE];
    int arrow, totalFitness = 0;

    for (int i = 0; i < POPULATION_SIZE; i++) {
        totalFitness += population[i].fitness; // count total fitness
    }

    for (int i = 0; i < POPULATION_SIZE; i++) {
        probabilities[i] = population[i].fitness / (float) totalFitness * 100; // calculate probabilities

        // create scope array
        if (i == 0)
            scope[0] = probabilities[0];
        else
            scope[i] = scope[i - 1] + probabilities[i];
    }

#if DEBUG_MODE
    cout << "========== Roulette Wheel Selection ==========" << endl;
    for (int i = 0; i < POPULATION_SIZE; i++) {
        cout << "probabilities[" << i << "]: " << probabilities[i] << endl;
    }
    for (int i = 0; i < POPULATION_SIZE; i++) {
        cout << "scope[" << i << "]: " << scope[i] << endl;
    }
#endif

    for (int i = 0; i < POPULATION_SIZE; i++) {
        arrow = myRandom(0, 99); // arrow points to [0, 99] randomly

#if DEBUG_MODE
        cout << "arrow points at: " << arrow << endl;
#endif

        for (int j = 0; j < POPULATION_SIZE; j++) {
            if (arrow < scope[j]) {
                // arrow points to population[j], i.e., copy population[j] to crossover pool
                memcpy(&pool[i], &population[j], sizeof(parent_t));
#if DEBUG_MODE
                cout << "selected index: [" << j << "]\tfitness: " << population[j].fitness << endl;
#endif
                break; // break for-loop if you already selected.
            }
        }
    }
}

// Single-Point Crossover
void crossoverSP() {
#if DEBUG_MODE
    cout << "========== Single-Point Crossover ==========" << endl;
#endif

    for (int i = 0; i < POPULATION_SIZE; i += 2) { // execute crossover once will get two children, so here is i +=2
        if (myRandom(0, 99) < CROSSOVER_RATE) { // do crossover
            // pick 2 individuals in crossover pool randomly
            int iPool1 = myRandom(0, POPULATION_SIZE - 1);

            int iPool2;
            do {
                iPool2 = myRandom(0, POPULATION_SIZE - 1);
            } while (iPool1 == iPool2); // do it again if pick the same individual

            // generate a crossover point randomly
            int crossoverPoint = myRandom(1, GENE_LENGTH - 1);

            // crossover execution
            for (int j = 0; j < crossoverPoint; j++) {
                population[i].gene[j] = pool[iPool1].gene[j];
                population[i + 1].gene[j] = pool[iPool2].gene[j];
            }

            for (int j = crossoverPoint; j < GENE_LENGTH; j++) {
                population[i + 1].gene[j] = pool[iPool1].gene[j];
                population[i].gene[j] = pool[iPool2].gene[j];
            }

            // calculate fitness after crossover done
            calcFitness(&population[i]);
            calcFitness(&population[i + 1]);

#if DEBUG_MODE
            cout << "pool[" << iPool1
                 << "] and pool[" << iPool2
                 << "] crossover start at position: [" << crossoverPoint << ']' << endl;
            cout << "replaced population[" << i
                 << "] and population[" << i + 1
                 << "] with newborn children" << endl;
#endif
        } else { // don't crossover, so just put it back to population
            memcpy(&population[i], &pool[i], sizeof(parent_t));
            memcpy(&population[i + 1], &pool[i + 1], sizeof(parent_t));
        }
    }

#if DEBUG_MODE
    cout << "Crossover pool: " << endl; // print crossover pool

    for (int i = 0; i < POPULATION_SIZE; i++) {
        cout << "pool[" << i << "]: ";

        for (int j = 0; j < GENE_LENGTH; j++) {
            cout << setw(2) << pool[i].gene[j] << ' ';
        }
        cout << endl;
    }

    for (int i = 0; i < POPULATION_SIZE; i++) {
        cout << "index: " << i
             << "\tweight: " << pool[i].weight
             << "\tvalue: " << pool[i].value
             << "\tfitness: " << pool[i].fitness << endl;
    }

    // print the state after crossover
    cout << "Population pool: " << endl;
    showState();
#endif
}

void crossoverKP() {
#if DEBUG_MODE
    cout << "========== K-Point Crossover ==========" << endl;
#endif

    // execute crossover once will get two children, so it is i +=2
    for (int i = 0; i < POPULATION_SIZE; i += 2) {
        if (myRandom(0, 99) < CROSSOVER_RATE) { // do crossover
            int crossoverPoints[K_POINT_CROSSOVER];
            // generate kp random numbers for crossover points
            for (int j = 0; j < K_POINT_CROSSOVER; j++) {
                crossoverPoints[j] = myRandom(1, GENE_LENGTH - 1);

                // check if crossover points repeat
                int k = 1;
                while (k < K_POINT_CROSSOVER) {
                    if (j < k) // no need to check if j < k, so break while-loop
                        break;

                    // j > k
                    if (crossoverPoints[j] == crossoverPoints[j - k]) {
                        crossoverPoints[j] = myRandom(1, GENE_LENGTH - 1);
                        // when crossoverPoints[i] got a new number,
                        // you should recheck from index [j-1] to [0], so it is k = 1 here
                        k = 1;
                    } else {
                        k++;
                    }
                }
            }

            quickSort(crossoverPoints, 0, K_POINT_CROSSOVER - 1); // sort crossoverPoints

            // pick 2 individuals in crossover pool randomly
            int iPool1 = myRandom(0, POPULATION_SIZE - 1);

            int iPool2;
            do {
                iPool2 = myRandom(0, POPULATION_SIZE - 1);
            } while (iPool1 == iPool2);

            // use a bucket for dispatching
            struct bucket {
                int section1;
                int section2;
            };

            // create a bucket array
            bucket buckets[K_POINT_CROSSOVER + 1];

            // section1 <= range < section2
            for (int j = 0; j < K_POINT_CROSSOVER + 1; j++) { // "K_POINT_CROSSOVER + 1" stands for the number of bucket
                if (j == 0) { // first bucket
                    buckets[j].section1 = 0;
                    buckets[j].section2 = crossoverPoints[j];

                } else if (j == K_POINT_CROSSOVER) { // the last bucket
                    buckets[j].section1 = crossoverPoints[j - 1];
                    buckets[j].section2 = GENE_LENGTH;

                } else { // others bucket
                    buckets[j].section1 = crossoverPoints[j - 1];
                    buckets[j].section2 = crossoverPoints[j];
                }

#if DEBUG_MODE
                cout << "bucket section1: " << buckets[j].section1
                     << "\tbucket section2: " << buckets[j].section2 << endl;
#endif
            }

            // crossover process
            for (int k = 0; k < K_POINT_CROSSOVER + 1; k++) { // "K_POINT_CROSSOVER + 1" stands for the number of bucket
                for (int l = buckets[k].section1; l < buckets[k].section2; l++) {
                    // dispatching
                    if (k % 2) { // even number of bucket
                        population[i].gene[l] = pool[iPool1].gene[l];
                        population[i + 1].gene[l] = pool[iPool2].gene[l];

                    } else { // odd number of bucket
                        population[i + 1].gene[l] = pool[iPool1].gene[l];
                        population[i].gene[l] = pool[iPool2].gene[l];
                    }
                }
            }

            // calculate fitness after crossover done
            calcFitness(&population[i]);
            calcFitness(&population[i + 1]);

#if DEBUG_MODE
            cout << "crossover points: ";
            for (int j = 0; j < K_POINT_CROSSOVER; j++) {
                cout << crossoverPoints[j] << ' ';
            }

            cout << endl;

            cout << "children of pool[" << iPool1
                 << "] and pool[" << iPool2 << "] ";
            cout << "have replaced population[" << i
                 << "] and population[" << i + 1
                 << "]" << endl;
#endif

        } else { // don't crossover, so just put it back to population
            memcpy(&population[i], &pool[i], sizeof(parent_t));
            memcpy(&population[i + 1], &pool[i + 1], sizeof(parent_t));
        }
    }

#if DEBUG_MODE
    cout << "Crossover pool: " << endl; // print crossover pool

    for (int i = 0; i < POPULATION_SIZE; i++) {
        cout << "pool[" << i << "]: ";

        for (int j = 0; j < GENE_LENGTH; j++) {
            cout << setw(2) << pool[i].gene[j] << ' ';
        }
        cout << endl;
    }

    for (int i = 0; i < POPULATION_SIZE; i++) {
        cout << "index: " << i
             << "\tweight: " << pool[i].weight
             << "\tvalue: " << pool[i].value
             << "\tfitness: " << pool[i].fitness << endl;
    }

    // print the state after crossover
    cout << "Population pool: " << endl;
    showState();
#endif
}

/** Single-Point Mutation
 * only one bit of each individual has a chance to be a mutation point.
 */
void mutateSP() {
#if DEBUG_MODE
    cout << "========== Single-Point Mutation ==========" << endl;
#endif

    for (int i = 0; i < POPULATION_SIZE; i++) {
        // each individual has a chance to be a mutation point
        if ((myRandom(0, 99)) < MUTATION_RATE) {
            int pos = myRandom(0, GENE_LENGTH - 1); // set mutating position
            population[i].gene[pos] = myRandom(0, 10);

            // calculate fitness as mutation happened
            calcFitness(&population[i]);
            // check if best gene changed
            if (population[i].fitness > bestGene.fitness) {
                memcpy(&bestGene, &population[i], sizeof(parent_t));
            }

#if DEBUG_MODE
            cout << "population[" << i
                 << "] mutated at position: [" << pos << ']' << endl;
#endif
        }
    }

#if DEBUG_MODE
    showState();
#endif
}

/** Multiple-Point Mutation
 * every single bit of each individual has a chance to be a mutation point.
 */
void mutateMP() {
#if DEBUG_MODE
    cout << "========== Multiple-Point Mutation ==========" << endl;
#endif

    for (int i = 0; i < POPULATION_SIZE; i++) {
#if DEBUG_MODE
        cout << "population[" << i
             << "] mutated at position: ";
#endif

        for (int pos = 0; pos < GENE_LENGTH; pos++) {
            // every single bit has a mutation chance
            if ((myRandom(0, 99)) < MUTATION_RATE) {
                population[i].gene[pos] = myRandom(0, 10);

#if DEBUG_MODE
                cout << "[" << pos << "] ";
#endif
            }
        }
#if DEBUG_MODE
        cout << endl;
#endif

        // calculate fitness after mutation done
        calcFitness(&population[i]);
        // check if best gene changed
        if (population[i].fitness > bestGene.fitness) {
            memcpy(&bestGene, &population[i], sizeof(parent_t));
        }
    }

#if DEBUG_MODE
    showState();
#endif
}

// Print each chromosome state
void showState() {
    for (int i = 0; i < POPULATION_SIZE; i++) {
        cout << "population[" << i << "]: ";

        for (int j = 0; j < GENE_LENGTH; j++) {
            cout << setw(2) << population[i].gene[j] << ' ';
        }
        cout << endl;
    }

    for (int i = 0; i < POPULATION_SIZE; i++) {
        cout << "index: " << i << "\tweight: " << population[i].weight << "\tvalue: " << population[i].value
             << "\tfitness: " << population[i].fitness << endl;
    }

    cout << "----------" << endl;
    cout << "Best gene weight: " << bestGene.weight
         << "\tvalue: " << bestGene.value
         << "\tfitness: " << bestGene.fitness << endl;
}

// Print the number of items taken
void showResult() {
    cout << "========== Result at this round ==========" << endl;
    cout << "Best case: ";

    // See how many each item is taken
    for (int i = 0; i < GENE_LENGTH; i++) {
        cout << name[i] << ": " << bestGene.gene[i] << "  ";
    }
    cout << endl;

    cout << "bestGene weight: " << bestGene.weight
         << "\t value: " << bestGene.value
         << "\t fitness: " << bestGene.fitness << endl;
}

parent_t *getBestGene() {
    return &bestGene;
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void quickSort(int *arr, int l, int u) {
    if (l < u) {
        int i, j, pk;
        i = l;
        j = u;
        pk = arr[l];

        while (i < j) {
            while (arr[i] < pk) {
                i += 1;
            }

            while (arr[j] > pk) {
                j -= 1;
            }

            if (i < j) {
                swap(&arr[i], &arr[j]);
            }
        }

        swap(&arr[i], &arr[j]);
        quickSort(arr, l, j - 1);
        quickSort(arr, j + 1, u);
    }
}

int myRandom(int start, int end) {
    random_device rd;
    static default_random_engine gen = default_random_engine(rd());
    uniform_int_distribution<int> dis(start, end);
    return dis(gen);
}