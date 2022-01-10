#define MAX_INT 2147483647

#include <optional>
#include <utility>

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

	static std::optional<std::vector<int>>
	SolveSystem(const std::vector<std::vector<int>>& A, const std::vector<int>& b) {
		auto snf = SmithNormal(A);
		unsigned h = A.size();
		unsigned w = A[0].size();
		std::vector<int> c = LinearTransform(snf.L, b);


		for (int i = 0; i < h; i++) {
			/* System has no solution */
			if (b[i] % snf.D[i][i] != 0) {
				return {};
			}
			if (snf.D[i][i] == 0) {
				c[i] = 0;
			} else {
				c[i] /= snf.D[i][i];
			}
		}

		c.insert(c.end(), w - h, 0);
		return LinearTransform(snf.R, c);
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
};

