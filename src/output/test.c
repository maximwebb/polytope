#include <stdio.h>
#include <stdlib.h>

#define N 40
#define M 2000
#define seed 7


int main() {
    int (* A)[N] = malloc(sizeof(int[N][N]));


    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i][j] = (i + j * seed) % 2;
        }
    }

	for (int i = 1; i < N; ++i) {
		for (int j = 1; j < N; ++j) {
			A[i][j] = ((i==j) & A[i-1][j-1]) +
					  ((i!=j) & (A[i][j-1]<A[i-1][j]) & A[i-1][j]) +
					  ((i!=j) & (A[i-1][j]<A[i][j-1]) & A[i][j-1]);
		}
	}


    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            printf("%d, ", A[i][j]);
        }
        printf("\n");
    }
    free(A);
    return 0;
}
