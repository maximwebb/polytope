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
int bar(int N) {
	for (int i = 0; i < 6; i++) {
		for (int k = 3; k < 9; k++) {
			j += i;
			if (N > 2) {
				j -= k;
			}
		}
	}
	return N;
}

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


int main() {
	int N = 10;
	bar(N);
	return bar(N);
}
