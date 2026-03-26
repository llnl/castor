// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "whytraversal.hxx"
#include "whyml.hxx"
#include <memory>
#include <set>
#include <map>
#include <boost/graph/adjacency_list.hpp>

using namespace Why3;

///
/// @brief Allows us to annotate graph nodes with WhyNames
///
struct VertexProperties { WhyName name; };

///
/// @brief Type of the call graph
///
using CallGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProperties>;

///
/// @brief Type of the total graph, including the mapping from names to nodes
///
using TotalGraph = std::pair<CallGraph, std::map<WhyName, CallGraph::vertex_descriptor>>;

///
/// @brief Generates a function call graph based on a WhyProgram
///
class GenerateFunctionCallGraph : public WhyTraversal<WhyName, std::set<WhyName>>
{
protected:
	///
	/// @brief The synthesized attributes list
	///
	using SynthesizedAttributesList = WhyTraversal<WhyName, std::set<WhyName>>::SynthesizedAttributesList;

	///
	/// @brief Downwards traversal (unused)
	///
	/// @param astNode The AST node
	/// @param inheritedValue Passed down
	/// @return = inheritedValue
	///
	WhyName evaluateInheritedAttribute(std::shared_ptr<WhyNode> astNode, WhyName inheritedValue);

	///
	/// @brief Checks if a function is called, and if so, add it to the set
	///
	/// @param astNode The AST node we're visiting
	/// @param inheritedValue The current function name
	/// @param list Function calls encountered so far
	/// @return All of the functions encountered so far
	///
	std::set<WhyName> evaluateSynthesizedAttribute(std::shared_ptr<WhyNode> astNode, WhyName inheritedValue, SynthesizedAttributesList list);

public:
	///
	/// @brief Default constructor
	///
	GenerateFunctionCallGraph() = default;

	///
	/// @brief Traverses over the tree, then aggregates the set into a graph
	/// @param program The WhyProgram to investigate
	/// @return The total graph representing the call graph
	///
	TotalGraph operator()(std::shared_ptr<WhyProgram> program);
};

///
/// @brief This generates the proper order to emit Why3 functions
///
/// We also annotate mutually recursive functions that should use "with" instead of "let rec".
///
/// This function operates by creating a condensation graph of the call graph in order to get a DAG.
/// Once we have a condensation graph, we perform a topological sort in order to determine
/// the order in which we emit functions into Why3. The condensation graph is important for handling
/// recursion and mutual recursion. If a single function equates to a strongly connected component,
/// it is not mutually recursive, and we can emit it alone. If multiple functions are in a strongly
/// connected component, they are mutually recursive, and we should mark them so. That way, the
/// pretty-printers can properly emit them into Why3 using "with".
///
/// @param graph The call graph generated from GenerateFunctionCallGraph
/// @param program The current WhyProgram to reorder
/// @return The new WhyProgram, reordered
///
std::shared_ptr<WhyNode> generate_correct_order(TotalGraph graph, std::shared_ptr<WhyProgram> program);

