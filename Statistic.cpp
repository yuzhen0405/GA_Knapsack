#include "Statistic.h"
#include "GA.h"
#include <iostream>
#include <iomanip>
#include <cstring>

using namespace std;

int cntOverweight = 0; // count overweight times
int totalValue = 0;
int bestValue = 0;
int bestWeight = 0;
int bestFitness = 0;
int bestCase[10];
int cntBestCase = 0;

bool isOverweight(int w) {
    return w > KNAPSACK_SIZE;
}

// statistic of result of total rounds
void statistic() {
    parent_t *best;
    best = getBestGene();

    // check if it is overweight
    if (isOverweight(best->weight)) {
        cntOverweight++;
#if DEBUG_MODE
        cout << "Overweight!!!" << endl;
#endif
    } else if (best->fitness == 620) {
        cntBestCase++;
    }

    totalValue += best->value;

    // check if it needs to update best fitness
    if (best->fitness > bestFitness) {
        bestWeight = best->weight;
        bestValue = best->value;
        bestFitness = best->fitness;
        memcpy(bestCase, best->gene, sizeof(bestCase));
    }
}

void prtLine() {
    for (int i = 0; i < 78; i++) {
        if (i % 11 == 0)
            cout << '+';
        else
            cout << '-';
    }
    cout << endl;
}

void finalResult() {
    cout << "==================== STATISTIC ====================\n";
#if SELECTION_TYPE == 0
    cout << "Selection type: Tournament" << endl;
#else
    cout << "Selection type: Roulette Wheel" << endl;
#endif

#if CROSSOVER_TYPE == 0
    cout << "Crossover type: Single-Point" << endl;
#elif CROSSOVER_TYPE == 1
    cout << "Crossover type: K-Point" << endl;
#else
    cout << "Crossover type: Mask" << endl;
#endif

#if MUTATION_TYPE == 0
    cout << "Mutation type: Single-Point" << endl;
#else
    cout << "Mutation type: Multiple-Point" << endl;
#endif

    cout << "Statistic round: " << ROUND << endl;
    cout << "Generation: " << GENERATION << endl;
    cout << "Population: " << POPULATION_SIZE << endl;
    cout << "Crossover rate: " << CROSSOVER_RATE << "%" << endl;
    cout << "Mutation rate: " << MUTATION_RATE << "%" << endl;

    prtLine();

    cout << '|'
         << setw(10) << "Overweight" << '|'     // col. 1
         << setw(10) << "Overweight" << '|'     // col. 2
         << setw(10) << "Best case" << '|'      // col. 3
         << setw(10) << "Avg.   " << '|'        // col. 4
         << setw(10) << "Best   " << '|'        // col. 5
         << setw(10) << "Best   " << '|'        // col. 6
         << setw(10) << "Best case" << '|' << endl; // col. 7

    cout << '|'
         << setw(10) << "count  " << '|'        // col. 1
         << setw(10) << "rate   " << '|'        // col. 2
         << setw(10) << "weight  " << '|'       // col. 3
         << setw(10) << "value  " << '|'        // col. 4
         << setw(10) << "value  " << '|'        // col. 5
         << setw(10) << "fitness " << '|'       // col. 6
         << setw(10) << "count  " << '|' << endl; // col. 7

    prtLine();

    cout << '|'
         << setw(9) << cntOverweight << " |"    // col. 1
         << setw(7) << fixed << setprecision(2) << cntOverweight / (double) ROUND * 100  // col. 2
         << setw(2) << " %" << " |"
         << setw(9) << bestWeight << " |"       // col. 3
         << setw(9) << setprecision(2) << totalValue / (double) ROUND << " |"    // col. 4
         << setw(9) << bestValue << " |"        // col. 5
         << setw(9) << bestFitness << " |"      // col. 6
         << setw(9) << cntBestCase << " |" << endl; // col. 7

    prtLine();

    cout << "Best case: ";
    for (int k = 0; k < 10; k++) {
        cout << name[k] << ": " << bestCase[k] << "  ";
    }
    cout << endl;
}