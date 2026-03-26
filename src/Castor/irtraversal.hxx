// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "ir.hxx"
#include <memory>
#include "helper.cxx"

#ifndef __IRTRAV
#define __IRTRAV

using namespace IR;

///
/// @brief Specifies a traversal over the IR AST
///
/// @tparam InValue The type of the inherited values
/// @tparam SynthesizedValue The type of the synthesized values
///
template <typename InValue, typename SynthesizedValue>
class IRTraversal
{
protected:
	using SynthesizedAttributesList = std::vector<SynthesizedValue>;

public:
	///
	/// @brief Constructor
	///
	IRTraversal() = default;

	///
	/// @brief Performs a top-down, bottom-up traversal
	///
	/// @param baseNode The IRNode to begin the traversal from
	/// @param inheritedValue Initial values for the inherited values
	/// @return The synthesized value
	///
	virtual SynthesizedValue traverse(std::shared_ptr<IRNode> baseNode, InValue inheritedValue)
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
	/// @brief Abstract function, specifies the behavior of the downwards traversal.
	///
	/// @param astNode The IRNode to look at
	/// @param inheritedValue The passed down inherited values
	/// @return The new inherited values
	///
	virtual InValue evaluateInheritedAttribute(std::shared_ptr<IRNode> astNode, InValue inheritedValue) = 0;

	///
	/// @brief Abstract function, specifies the behavior of the upwards traversal.
	///
	/// @param astNode the IRNode to look at
	/// @param inheritedValue The inherited values
	/// @param list The synthesized attributes so far
	/// @return The new synthesized value
	///
	virtual SynthesizedValue evaluateSynthesizedAttribute(std::shared_ptr<IRNode> astNode, InValue inheritedValue, SynthesizedAttributesList list) = 0;
};

#endif
