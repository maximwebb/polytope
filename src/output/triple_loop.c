#include <stdio.h>
#include <stdlib.h>

#define N 3


int main() {

	int A[N][N][N] = {{{3, 1, 1}, {2, 1, 3}, {5, 2, 2}},
					  {{1, 1, 3}, {3, 1, 0}, {3, 2, 1}},
					  {{0, 2, 4}, {2, 4, 3}, {1, 1, 0}}};

	for (int i = 1; i < N; ++i) {
		for (int j = 1; j < N; ++j) {
			for (int k = 1; k < N; ++k) {
				A[i][j][k] = A[i-1][j][k] + A[i][j-1][k] + A[i][j][k-1];
			}
		}
	}

	for (int i = 1; i < N; ++i) {
		for (int j = 1; j < N; ++j) {
			printf("( ");
			for (int k = 1; k < N; ++k) {
				printf("%d,", A[i][j][k]);
			}
			printf(") ");
		}
		printf("\n");
	}
	return 0;
}
