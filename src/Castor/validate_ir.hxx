// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <memory>
#include "irtraversal.hxx"

using namespace IR;

///
/// @brief Inherited values struct for the ValidateIR class
///
struct ValidateIVs
{
	bool inside_ensures; ///< True iff we're inside an "ensures" clause
	bool inside_vc;      ///< True iff we're inside a verification condition
	bool inside_lemma;   ///< True iff we're inside a "lemma" or "axiom" clause
	bool inside_loop;    ///< True iff we're inside a loop structure (for/while/etc)
};

///
/// @brief This validates the IR to ensure it's good to move on to the next step.
///
/// In reality, this only does one thing right now (check for IRUnknownType and throw an exception if found).
/// We can grow this in the future for more robust validation.
///
class ValidateIR : public IRTraversal<ValidateIVs, std::shared_ptr<IRNode>>
{
private:
	///
	/// @brief Checks if a function or loop has a writes clause
	///
	/// @param structure A shared pointer to the function to check
	/// @tparam T IRFunction or IRLoopStmt
	/// @return How many writes clauses it has
	///
	template <typename T>
	int has_writes_clause(std::shared_ptr<T> structure);

	///
	/// @brief Checks if a function or lop has a writes clause
	///
	/// @param structure A shared pointer to the function to check
	/// @tparam T IRFunction or IRLoopStmt
	/// @return How many frees clauses it has
	///
	template <typename T>
	int has_frees_clause(std::shared_ptr<T> structure);

protected:
	using SynthesizedAttributesList = IRTraversal<ValidateIVs, std::shared_ptr<IRNode>>::SynthesizedAttributesList;

	///
	/// @brief Downwards traversal
	///
	/// @param astNode The IR node we're looking at
	/// @param inheritedValue The inherited values so far
	/// @return The new inherited values
	///
	ValidateIVs evaluateInheritedAttribute(std::shared_ptr<IRNode> astNode, ValidateIVs inheritedValue);

	///
	/// @brief Upwards traversal, validates the IR
	///
	/// @param astNode The IR node to check
	/// @param inheritedValue The inherited values
	/// @param list The child nodes
	/// @return Equal to astNode
	///
	std::shared_ptr<IRNode> evaluateSynthesizedAttribute(std::shared_ptr<IRNode> astNode, ValidateIVs inheritedValue, SynthesizedAttributesList list);

public:
	///
	/// @brief Constructor
	///
	ValidateIR() = default;

	///
	/// @brief Functor, so that this class can be called as a function
	///
	/// @param base The base IR node to start checking from
	///
	void operator()(std::shared_ptr<IRNode> base);
};
