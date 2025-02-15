#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *deadlock_file;

int main() {
    deadlock_file = fopen("task7.txt", "r");
    if (deadlock_file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    int p = 0; // number of processes
    int r = 0; // number of resources

    fscanf(deadlock_file, "%d %d", &p, &r);
    int E[r]; // vector E
    for (int i = 0; i < r; i++) {
        fscanf(deadlock_file, "%d", &E[i]);
    }

    int A[p][r]; // allocation matrix
    for (int i = 0; i < p; i++) {
        for (int j = 0; j < r; j++) {
            fscanf(deadlock_file, "%d", &A[i][j]);
        }
    }

    int R[p][r]; // request matrix
    for (int i = 0; i < p; i++) {
        for (int j = 0; j < r; j++) {
            fscanf(deadlock_file, "%d", &R[i][j]);
        }
    }
    fclose(deadlock_file);

    int available[r];
    for (int i = 0; i < r; i++) {
        available[i] = E[i];
        for (int j = 0; j < p; j++) {
            available[i] -= A[j][i];
        }
    }

    int work[r];
    for (int i = 0; i < r; i++) {
        work[i] = available[i];
    }

    int finish[p];
    for (int i = 0; i < p; i++) {
        finish[i] = 0;
    }

    int found;
    do {
        found = 0;
        for (int i = 0; i < p; i++) {
            if (finish[i] == 0) {
                int can_finish = 1;
                for (int j = 0; j < r; j++) {
                    if (R[i][j] > work[j]) {
                        can_finish = 0;
                        break;
                    }
                }
                if (can_finish) {
                    for (int j = 0; j < r; j++) {
                        work[j] += A[i][j];
                    }
                    finish[i] = 1;
                    found = 1;
                }
            }
        }
    } while (found);

    int deadlock = 0;
    for (int i = 0; i < p; i++) {
        if (finish[i] == 0) {
            printf("Process %d is deadlocked\n", i);
            deadlock = 1;
        }
    }
    if (!deadlock) {
        printf("No deadlock\n");
    }

    return EXIT_SUCCESS;
}