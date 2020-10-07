#include "GA.h"
#include "Statistic.h"
#include <iostream>
#include <time.h>

using namespace std;

int main() {
    auto start = chrono::steady_clock::now();// count execution time

    // Safety check
#if K_POINT_CROSSOVER > GENE_LENGTH - 1
    cout << "ERROR: K_POINT_CROSSOVER is out of bound.";
    return 0;
#endif

    for (int i = 0; i < ROUND; i++) {
        cout << "Round " << (i + 1) << "/" << ROUND << endl;
        init();

        for (int j = 0; j < GENERATION; j++) {
#if SELECTION_TYPE == 0
            selectTournament();
#else
            selectRW();
#endif
#if CROSSOVER_TYPE == 0
            crossoverSP();
#else
            crossoverKP();
#endif
#if MUTATION_TYPE == 0
            mutateSP();
#else
            mutateMP();
#endif
        }
#if EACH_ROUND_RESULT
        showResult();
#endif
        statistic();
    }
    finalResult();

    // count execution time
    auto end = chrono::steady_clock::now();
    cout << "Time taken: " << chrono::duration<double>(end - start).count() << " s" << endl;
    return 0;
}
