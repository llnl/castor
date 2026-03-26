// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <memory>
#include <map>
#include <utility>
#include <tuple>
#include "irtraversal.hxx"
#include "whytraversal.hxx"

using DebugNothing = std::tuple<>;

using namespace IR;
using namespace Why3;

///
/// @brief Traverses the IR and fills in debug information.
///
/// The debug information is used:
/// - When generating WhyML, attaching markers in \@expl attributes
/// - When parsing the output of Why3 to provide a human-readable output
///
class FillDebugInfo : public IRTraversal<std::string, DebugNothing>
{
private:
	int idx; ///< A global counter, incrementing every time a VC is annotated
	std::map<std::string, std::string> data_map; ///< A map from \@expl attribute markers to raw VCs

protected:
	using SynthesizedAttributesList = IRTraversal<std::string, DebugNothing>::SynthesizedAttributesList;

	///
	/// @brief Checks if a node is a VC, calls its set_debug_str function, and adds it to data_map
	///
	/// @param astNode The AST node to check
	/// @param functionName The name of the function we're currently in (unused)
	/// @return The name of the function we're currently in (unused)
	///
	std::string evaluateInheritedAttribute(std::shared_ptr<IRNode> astNode, std::string functionName);

	///
	/// @brief This function does nothing
	///
	/// @param astNode Unused
	/// @param unused Unused
	/// @param unused2 Unused
	/// @return Unused
	///
	DebugNothing evaluateSynthesizedAttribute(std::shared_ptr<IRNode> astNode, std::string unused, SynthesizedAttributesList unused2) { return DebugNothing(); }

public:
	///
	/// @brief Default constructor
	///
	FillDebugInfo() = default;

	///
	/// @brief Traverses the IR, filling in debug info
	///
	void operator()(std::shared_ptr<IRNode> base);

	///
	/// @brief Looks up the VC from a data_map based on the \@expl attribute marker
	///
	/// @param key The \@expl attribute marker from Why3
	/// @return The raw VC string that it's associated with
	///
	std::string lookup(std::string key);
};

///
/// @brief Traverses a Why3 AST to find the IR name of a Why3 function name
///
class LookupIRName : public WhyTraversal<std::string, std::string>
{
protected:
	using SynthesizedAttributesList = WhyTraversal<std::string, std::string>::SynthesizedAttributesList;

	///
	/// @brief This just passes down the Why3 function name
	///
	/// @param astNode Unused
	/// @param name The Why3 function name
	/// @return Returns the "name" parameter
	///
	std::string evaluateInheritedAttribute(std::shared_ptr<WhyNode> astNode, std::string name);

	///
	/// @brief This checks if a node is a WhyFunction and if that function's Why3 name is equal to the "name" parameter. If it is, return its IR name.
	///
	/// @param astNode The AST node to check
	/// @param name The Why3 function name
	/// @param list List of possibly found IR names
	/// @return The IR name, if we've found it. If not, return "$$NOT_FOUND"
	std::string evaluateSynthesizedAttribute(std::shared_ptr<WhyNode> astNode, std::string name, SynthesizedAttributesList list);

public:
	///
	/// @brief Default constructor
	///
	LookupIRName() = default;

	///
	/// @brief Returns the IR name of a Why3 function name
	///
	/// @param base The Why3 AST to search
	/// @param name The Why3 function name
	/// @return The IR name of the Why3 function name
	///
	std::string operator()(std::shared_ptr<WhyNode> base, std::string name);
};
