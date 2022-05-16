#include <vector>
#include <iostream>
#include "IntegerSolver.h"

int main() {
	std::vector<std::vector<int>> A = {{3,  5, 11},
									   {-5, 7, 9}};

	std::vector<std::vector<int>> B = {{-6, 111,  -36, 6},
									   {5,  -672, 210, 74},
									   {0,  -255, 81,  24},
									   {-7, 255,  -81, -10}};

	auto D = IntegerSolver::SmithNormal(A);
	auto E = IntegerSolver::SmithNormal(B);

	auto sol = IntegerSolver::SolveSystem(A, {2, 4});

	auto sol2 = IntegerSolver::SolveSystem({{2, 2}}, {1});


	std::vector<std::vector<int>> C = {{4, 7},
									  {2, 6}};
	auto sol3 = IntegerSolver::HermiteNormal(C);
	int det = IntegerSolver::Det(C);

	return 0;
}