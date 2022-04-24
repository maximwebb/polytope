#define MAX_INT 2147483647

#include <optional>
#include <utility>
#include <stdexcept>

struct SNF {
	std::vector<std::vector<int>> L;
	std::vector<std::vector<int>> D;
	std::vector<std::vector<int>> R;

	SNF(std::vector<std::vector<int>> L, std::vector<std::vector<int>> D, std::vector<std::vector<int>> R)
			: L(std::move(L)), D(std::move(D)), R(std::move(R)) {};
};

/* Static methods for integer programming */
class IntegerSolver {
public:
	/* Returns k such that n - k * q is the least residue modulo q, ie. 0 <= n - k * q < q */
	static int SignedDiv(int n, int q) {
		if (q > 0) {
			return (n - ((n % q + q) % q)) / q;
		}
		return (n - ((n % q - q) % q)) / q;
	}

	static std::vector<std::vector<int>> IdentityMatrix(unsigned dim) {
		std::vector<std::vector<int>> res;
		for (int i = 0; i < dim; i++) {
			std::vector<int> row(dim, 0);
			row[i] = 1;
			res.push_back(row);
		}
		return res;
	}

	static SNF SmithNormal(const std::vector<std::vector<int>>& A) {
		std::vector<std::vector<int>> D = A;
		unsigned h = D.size();
		unsigned w = D[0].size();
		unsigned dim = std::min(h, w);
		auto L = IdentityMatrix(h);
		auto R = IdentityMatrix(w);

		for (int i = 0; i < dim; i++) {
			std::optional<int> pivotIndex;
			/* Initialise column index to i, in order to prevent column swapping when no changes occur */
			int colIndex = i;
			while ((pivotIndex = GetRowPivot(D, i))) {
				colIndex = pivotIndex.value();
				int pivot = D[i][colIndex];
				/* Apply column operations to matrices D and R to reduce values */
				for (int col = 0; col < w; col++) {
					if (col != colIndex) {
						int scale = SignedDiv(D[i][col], pivot);
						UpdateCol(D, colIndex, col, scale);
						UpdateCol(R, colIndex, col, scale);
					}
				}
			}
			/* Move non-zero entry to diagonal */
			if (colIndex != i) {
				SwapCols(D, colIndex, i);
				SwapCols(R, colIndex, i);
			}
			int rowIndex = i;
			while ((pivotIndex = GetColPivot(D, i))) {
				rowIndex = pivotIndex.value();
				int pivot = D[rowIndex][i];
				/* Apply row operations to matrices D and L to reduce values */
				for (int row = 0; row < h; row++) {
					if (row != rowIndex) {
						int scale = SignedDiv(D[row][i], pivot);
						UpdateRow(D, rowIndex, row, scale);
						UpdateRow(L, rowIndex, row, scale);
					}
				}
			}
			/* Move non-zero entry to diagonal */
			if (rowIndex != i) {
				SwapRows(D, rowIndex, i);
				SwapRows(L, rowIndex, i);
			}
		}

		return {L, D, R};
	}

	/* Used for determining transformed loop bounds for a lattice */
	static std::vector<std::vector<int>> HermiteNormal(const std::vector<std::vector<int>>& A) {
		std::vector<std::vector<int>> D = A;
		unsigned N = A.size();
		int i = 0;

		while (i < N) {
			bool rowComplete = true;
			for (int j = i + 1; j < N; j++) {
				rowComplete &= D[i][j] == 0;
			}
			if (rowComplete) {
				if (D[i][i] < 0) {
					NegateCol(D, i);
				}
				i++;
				continue;
			}
			/* Choose smallest absolute element in row to act as pivot */
			int pivot = MAX_INT;
			int pivot_index = -1;
			for (int j = 0; j < N; j++) {
				int el = D[i][j];
				if (abs(el) < pivot && el != 0) {
					pivot_index = j;
					pivot = el;
				}
			}
			SwapCols(D, i, pivot_index);
			for (int j = i + 1; j < N; j++) {
				int q = D[i][j]/pivot;
				for (int k = 0; k < N; k++) {
					D[k][j] -= q * D[k][i];
				}
			}
		}

		for (i = 0; i < N; i++) {
			for (int j = 0; j < i; j++) {
				if (D[i][j] < 0) {
					int b = SignedDiv(D[i][j], D[i][i]);
					for (int k = 0; k < N; k++) {
						D[k][j] -= b * D[k][i];
					}
				}
			}
		}


		return D;
	}

	static std::vector<int> LinearTransform(const std::vector<std::vector<int>>& A, const std::vector<int>& x) {
		std::vector<int> res(A.size(), 0);
		unsigned h = A.size();
		unsigned w = A[0].size();

		if (w != x.size()) {
			throw std::domain_error("Cannot multiply vector by matrix of different width.");
		}

		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				res[i] += A[i][j] * x[j];
			}
		}
		return res;
	}

	static std::vector<std::vector<int>>
	Multiply(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
		std::vector<std::vector<int>> res;

		unsigned h = A.size();
		unsigned w = B[0].size();
		unsigned n = B.size();

		for (int i = 0; i < h; i++) {
			res.emplace_back(w, 0);
			for (int j = 0; j < w; j++) {
				for (int k = 0; k < n; k++) {
					res[i][j] += A[i][k] * B[k][j];
				}
			}
		}
		return res;
	}

	static std::optional<std::vector<int>>
	SolveSystem(const std::vector<std::vector<int>>& A, const std::vector<int>& b) {
		auto snf = SmithNormal(A);
		unsigned h = A.size();
		unsigned w = A[0].size();
		std::vector<int> c = LinearTransform(snf.L, b);


		for (int i = 0; i < h; i++) {
			/* Return no solution if any variable has a non-integer solution, or no solution at all. */
			if (snf.D[i][i] == 0) {
				if (c[i] != 0) {
					return {};
				}
			} else if (b[i] % snf.D[i][i] != 0) {
				return {};
			} else {
				c[i] /= snf.D[i][i];
			}
		}

		c.insert(c.end(), w - h, 0);
		return LinearTransform(snf.R, c);
	}

	static std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> GetGenerators(unsigned dim) {
		std::vector<std::vector<int>> A;
		std::vector<std::vector<int>> B = IdentityMatrix(dim);

		A.emplace_back(dim, 0);
		A.at(0).back() = -1;

		for (int i = 0; i < dim - 1; i++) {
			A.emplace_back(dim, 0);
			A[i + 1][i] = -1;
		}
		B.at(0).at(1) = 1;

		return {A, B};
	}

private:
	/* Test if all but one entry in a row is zero - if not, returns the least element */
	static std::optional<int> GetRowPivot(const std::vector<std::vector<int>>& A, int index) {
		int nonZeroCount = 0;
		int least = MAX_INT;
		int leastIndex = -1;
		for (int i = 0; i < A[index].size(); i++) {
			int e = A[index][i];
			if (e != 0) {
				nonZeroCount++;
				if (std::abs(e) < std::abs(least)) {
					least = e;
					leastIndex = i;
				}
			}
		}
		if (nonZeroCount < 2) {
			return {};
		}
		return leastIndex;
	}

	static std::optional<int> GetColPivot(const std::vector<std::vector<int>>& A, int index) {
		int nonZeroCount = 0;
		int least = MAX_INT;
		int leastIndex = -1;
		for (int i = 0; i < A.size(); i++) {
			const auto& row = A[i];
			int e = row[index];
			if (e != 0) {
				nonZeroCount++;
				if (std::abs(e) < std::abs(least)) {
					least = e;
					leastIndex = i;
				}
			}
		}
		if (nonZeroCount < 2) {
			return {};
		}
		return leastIndex;
	}

	static void UpdateRow(std::vector<std::vector<int>>& A, int src, int dst, int scale) {
		for (int i = 0; i < A[src].size(); i++) {
			A[dst][i] -= A[src][i] * scale;
		}
	}

	static void UpdateCol(std::vector<std::vector<int>>& A, int src, int dst, int scale) {
		for (auto& row: A) {
			row[dst] -= row[src] * scale;
		}
	}

	static void SwapRows(std::vector<std::vector<int>>& A, int row1, int row2) {
		auto tmp = A[row1];
		A[row1] = A[row2];
		A[row2] = tmp;
	}

	static void SwapCols(std::vector<std::vector<int>>& A, int col1, int col2) {
		for (auto& row: A) {
			int tmp = row[col1];
			row[col1] = row[col2];
			row[col2] = tmp;
		}
	}

	static void NegateCol(std::vector<std::vector<int>>& A, int i) {
		for (auto& row : A) {
			row[i] *= -1;
		}
	}
};

