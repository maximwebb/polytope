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
					 std::vector<std::vector<std::vector<int>>> reads_) {
		/* Remove duplicates from reads & writes */
		std::set<std::vector<std::vector<int>>> tmp1(writes_.begin(), writes_.end());
		writes = std::vector<std::vector<std::vector<int>>>(tmp1.begin(), tmp1.end());

		std::set<std::vector<std::vector<int>>> tmp2(reads_.begin(), reads_.end());
		reads = std::vector<std::vector<std::vector<int>>>(tmp2.begin(), tmp2.end());
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

	bool HasCacheMisses() const {
		auto writePatterns = GetMatchingWrites();
		auto readPatterns = GetMatchingReads();

		for (auto& writePattern : writePatterns) {
			if (std::find(writes.begin(), writes.end(), writePattern) == writes.end()) {
				return false;
			}
		}
		for (auto& readPattern : readPatterns) {
			if (std::find(reads.begin(), reads.end(), readPattern) == reads.end()) {
				return false;
			}
		}
		return true;
	}

private:
	/* Compute a set of equations where an integer solution represents a loop-carried dependency. */
	std::vector<EquationSystem> ComputeEquations() const {
		std::vector<EquationSystem> equationSystems;
		auto accesses = reads;
		accesses.insert(accesses.end(), writes.begin(), writes.end());
		for (auto& write: writes) {
			/* Form dependency equations from both reads and writes */
			for (auto& access: accesses) {
				if (access != write) {
					std::vector<std::vector<int>> lhs;
					std::vector<int> rhs;
					for (int i = 0; i < access.size(); i++) {
						auto readIndex = access.at(i);
						auto writeIndex = write.at(i);
						std::vector<int> eq;
						/* Ensure that outermost induction variable is fixed between iterations */
						eq.push_back(writeIndex.at(0) - readIndex.at(0));
						/* Insert remaining write induction variable coefficients into equation, excluding the constant */
						eq.insert(eq.begin() + 1, writeIndex.begin() + 1, writeIndex.end() - 1);
						/* Insert remaining access induction variable coefficients. These are negated, as they were on the RHS of
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

	std::vector<std::vector<std::vector<int>>> GetMatchingReads() const {
		return {{{0, 0, 0}, {0, 1, 0}},
				{{0, 1, 0}, {1, 0, 0}}};
	}

	std::vector<std::vector<std::vector<int>>> GetMatchingWrites() const {
		return {{{0, 0, 0},
				 {1, 0, 0}}};
	}
};

#pragma clang diagnostic pop