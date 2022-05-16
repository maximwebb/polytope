#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nodiscard"

#include <utility>
#include <vector>
#include <set>
#include "IntegerSolver.h"

/* Stores a linear system of equations of the form Ax = b */
struct EquationSystem {
	const std::vector<std::vector<int>> lhs;
	const std::vector<int> rhs;

	EquationSystem(std::vector<std::vector<int>> lhs, std::vector<int> rhs)
			: lhs(std::move(lhs)), rhs(std::move(rhs)) {};
};

/* Stores the affine functions used in an array assignment, ie. the array index written to,
 * and the list of array reads, and determine if it exhibits loop carrier dependencies. */
class LoopDependencies {
public:
	std::vector<std::vector<std::vector<int>>> writes;
	std::vector<std::vector<std::vector<int>>> reads;

	LoopDependencies(std::vector<std::vector<std::vector<int>>> writes_,
					 std::vector<std::vector<std::vector<int>>> reads_) :
			reads(std::move(reads_)) {
		/* Remove duplicates from writes */
		std::set<std::vector<std::vector<int>>> tmp(writes_.begin(), writes_.end());
		writes = std::vector<std::vector<std::vector<int>>>(tmp.begin(), tmp.end());
	};

	bool HasLoopCarrierDependencies() const {
		auto equationSystems = ComputeEquations();
		for (auto& eqs: equationSystems) {
			auto res = IntegerSolver::SolveSystem(eqs.lhs, eqs.rhs);
			if (res) {
				return true;
			}
		}
		return false;
	}

private:
	/* Compute a set of equations where an integer solution represents a loop-carried dependency. */
	std::vector<EquationSystem> ComputeEquations() const {
		std::vector<EquationSystem> equationSystems;

		for (auto& write: writes) {
			for (auto& read: reads) {
				if (read != write) {
					std::vector<std::vector<int>> lhs;
					std::vector<int> rhs;
					for (int i = 0; i < read.size(); i++) {
						auto readIndex = read.at(i);
						auto writeIndex = write.at(i);
						std::vector<int> eq;
						/* Ensure that outermost induction variable is fixed between iterations */
						eq.push_back(writeIndex.at(0) - readIndex.at(0));
						/* Insert remaining write induction variable coefficients into equation, excluding the constant */
						eq.insert(eq.begin() + 1, writeIndex.begin() + 1, writeIndex.end() - 1);
						/* Insert remaining read induction variable coefficients. These are negated, as they were on the RHS of
						 * the equation. */
						for (int j = 1; j < readIndex.size() - 1; j++) {
							eq.push_back(-readIndex.at(j));
						}

						lhs.push_back(eq);
						/* Rearrange equation such that constant term is on the RHS */
						rhs.push_back(readIndex.back() - writeIndex.back());
					}
					equationSystems.emplace_back(lhs, rhs);
				}
			}
		}

		return equationSystems;
	}
};

#pragma clang diagnostic pop