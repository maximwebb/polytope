#include <stdio.h>
#include <stdlib.h>
#define n 500
#define m 500
#define seed 7


int main() {
    int (*A)[m] = malloc(sizeof(int[n][m]));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            A[i][j] = (i + j*seed) % 4;
        }
    }

    for (int i = 1; i < n; ++i) {
        for (int j = 1; j < n; ++j) {
            for (int k = 1; k < n; ++k) {
                A[j][k] = A[j][k-1] + A[j-1][k];
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("%d, ", A[i][j]);
        }
        printf("\n");
    }
    free(A);
    return 0;
}
