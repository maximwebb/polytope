#include <stdio.h>
int j = 0;

/* Imperfect loop */
//int foo(int N) {
//	for (int i = 0; i < 6; i += 1) {
//		if (N > 2) {
//			j += 3;
//		}
//		for (int k = 3; k < 9; k += 3) {
//			j += N;
//		}
//	}
//	return j;
//}

/* Perfect loop */
//int bar(int N) {
//	for (int i = 0; i < 6; i++) {
//		for (int k = 0; k < i * 2; k += 1) {
//			j += i;
////			if (N > 2) {
////				j -= k;
////			}
//		}
//	}
//	return j;
//}

/* Imperfect loop */
//int baz(int N) {
//	for (int i = 0; i < 10; i++) {
//		for (int k = 0; k < 11; k++) {
//			j += 2;
//		}
//		if (N > 2) {
//			j += 2;
//		}
//	}
//	return N;
//}

/* Triple nested - outer should not pass, inner should */
//int tee(int N) {
//	for (int i = 0; i < 10; i++) {
//		for (int k = 0; k < 20; k++) {
//			for (int t = 0; t < 30; t++) {
//				j += N;
//			}
//		}
//	}
//	return j;
//}

int arr(int N) {
	int A[][4] = {{1, 3, 5, 8},
				  {4, 2, 2, 3},
				  {5, 3, 1, 7}};
	for (int i = 1; i < 3; ++i) {
		for (int k = 1; k < 4; ++k) {
			A[i][k] = A[i - 1][k] + A[i][k - 1] + A[i - 1][k - 1];
		}
	}
	printf("(%d, %d)\n", A[2][2], A[1][1]);

	return A[2][2];
}

//int func() {
//	for (int i = 0; i < 4; i++) {
//		for (int k = 0; k < 3; k++) {
//			j++;
//		}
//	}
//	return 4;
//}

int main() {
	int N = 10;
//	arr(N);
	return arr(N);
}
