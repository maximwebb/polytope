#include <stdio.h>
#include <stdlib.h>
#define N 500
#define M 500
#define seed 7


int main() {
    int (*A)[M] = malloc(sizeof(int[N][M]));

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i][j] = (i + j*seed) % 4;
        }
    }

    for (int i = 1; i < N; ++i) {
        for (int j = 1; j < N; ++j) {
//            A[j][i] = (2*A[j-1][i])%17;
//            A[j][i] = A[j-1][i];
            A[i][j] = A[i-1][j] + A[i][j-1];
        }
    }

//    for (int i = 0; i < N; ++i) {
//        for (int j = 0; j < N; ++j) {
//            printf("%d, ", A[i][j]);
//        }
//        printf("\n");
//    }
    printf("%d", A[5][2]);
    free(A);
    return 0;
}
