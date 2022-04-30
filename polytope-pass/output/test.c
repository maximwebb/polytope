#include <stdio.h>
#include <stdlib.h>
#define N 1000
#define M 1000
#define seed 7


int main() {
	int (*A)[M] = malloc(sizeof(int[N][M]));
//	int A[][4] = {{1, 3, 5, 8},
//					  {4, 2, 2, 3},
//					  {5, 3, 1, 7}};

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = (i + j*seed) % 4;
        }
    }


    for (int i = 2; i < 8; ++i) {
        for (int j = 2; j < 8; ++j) {
//            A[j-2][2*j-1] = (A[2*i+2*j-1][j-1] + A[i+j-2][j-2] + A[i+2*j][j]) % 17;
//			A[i][j] = A[i][j-1];
			A[i][j] = A[i-1][j] + A[i][j-1] + A[i-1][j-1];
			printf("(%d, %d)", i, j);
        }
    }
	printf("\n");

    for (int i = 2; i < 8; ++i) {
        for (int j = 2; j < 8; ++j) {
            printf("%d, ", A[i][j]);
        }
        printf("\n");
    }
    return 0;
}
