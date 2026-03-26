// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "whyml.hxx"
#include "helper.cxx"
#include <memory>

#ifndef __WHYTRAV
#define __WHYTRAV

using namespace Why3;

///
/// @brief Specifies a traversal over the Why3 AST
///
/// @tparam InValue Object to use for inherited values
/// @tparam SynthesizedValue Object to use for synthesized values
///
template <typename InValue, typename SynthesizedValue>
class WhyTraversal
{
protected:
	using SynthesizedAttributesList = std::vector<SynthesizedValue>;

public:
	///
	/// @brief Constructor
	///
	WhyTraversal() = default;

	///
	/// @brief Performs a top-down-bottom-up traversal
	///
	/// @param baseNode The base WhyNode to traverse over
	/// @param inheritedValue The default inherited value
	/// @return The synthesized value
	///
	virtual SynthesizedValue traverse(std::shared_ptr<WhyNode> baseNode, InValue inheritedValue)
	{
		if (baseNode)
		{
			auto newInheritedValue = this->evaluateInheritedAttribute(baseNode, inheritedValue);

			std::vector<SynthesizedValue> synths;

			for (auto n : baseNode->traverse())
			{
				synths.push_back(traverse(n, newInheritedValue));
			}
	
			return evaluateSynthesizedAttribute(baseNode, newInheritedValue, synths);
		}
		else
		{
			return SynthesizedValue();
		}
	}

	///
	/// @brief Abstract method for evaluating inherited attributes
	///
	/// @param astNode The WhyNode to look at
	/// @param inheritedValue The inherited values passed down
	/// @return The inherited value to pass down
	///
	virtual InValue evaluateInheritedAttribute(std::shared_ptr<WhyNode> astNode, InValue inheritedValue) = 0;

	///
	/// @brief Abstract method for synthesizing attributes
	///
	/// @param astNode The WhyNode to look at
	/// @param inheritedValue The inherited values
	/// @param list The synthesized attributes so far
	/// @return The new synthesized attribute
	///
	virtual SynthesizedValue evaluateSynthesizedAttribute(std::shared_ptr<WhyNode> astNode, InValue inheritedValue, SynthesizedAttributesList list) = 0;
};

#endif
