// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "whytraversal.hxx"

///
/// @brief The inherited value struct for setting the WhyML metadata
///
struct SetMetaIV
{
	std::vector<std::shared_ptr<WhyVariable>> vars;            ///< Function parameters 
	bool in_vc;                                                ///< Whether or not we're in a verification condition
	bool in_quantifier;                                        ///< Whether or not we're in a quantifier
	std::vector<std::shared_ptr<WhyVariable>> quantifier_vars; ///< Quantifier variables
	bool in_loop;                                              ///< Whether or not we're in a loop
};

///
/// @brief This sets a bunch of metadata on the WhyML AST
///
/// Without this step, the resulting WhyML would be invalid, or may not even pretty-print properly.
/// After generating the WhyML, we have to go back through and make sure a lot of metadata is set.
///
class SetMetadata : public WhyTraversal<SetMetaIV, std::shared_ptr<WhyNode>>
{
protected:
	using SynthesizedAttributesList = WhyTraversal<SetMetaIV, std::shared_ptr<WhyNode>>::SynthesizedAttributesList;

	///
	/// @brief The downwards traversal.
	///
	/// @param astNode The WhyNode we're looking at
	/// @param inheritedValue The inherited value that's been passed down
	/// @return The new inherited value
	///
	SetMetaIV evaluateInheritedAttribute(std::shared_ptr<WhyNode> astNode, SetMetaIV inheritedValue);

	///
	/// @brief The upwards traversal.
	///
	/// @param astNode The WhyNode we're looking at
	/// @param inheritedValue The inherited value we've received
	/// @param list List of synthesized attributes so far
	/// @return The new synthesized attribute
	///
	std::shared_ptr<WhyNode> evaluateSynthesizedAttribute(std::shared_ptr<WhyNode> astNode, SetMetaIV inheritedValue, SynthesizedAttributesList list);

public:
	///
	/// @brief Constructor
	///
	SetMetadata();

	///
	/// @brief Functor, so that we can use instances of this class as a function
	///
	/// @param base The base WhyNode to traverse on
	/// @return The new WhyNode
	///
	std::shared_ptr<WhyNode> operator()(std::shared_ptr<WhyNode> base);
};
