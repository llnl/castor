// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "debug_info.hxx"
#include <iostream>

///
/// @brief Checks if a node is a VC, calls its set_debug_str function, and adds it to data_map
///
/// @param astNode The AST node to check
/// @param functionName The name of the function we're currently in (unused)
/// @return The name of the function we're currently in (unused)
///
std::string FillDebugInfo::evaluateInheritedAttribute(std::shared_ptr<IRNode> astNode, std::string functionName)
{
	if (auto func = std::dynamic_pointer_cast<IRFunction>(astNode))
		return func->get_name().substr(0, func->get_name().find('#') - 1);

	if (auto vc = std::dynamic_pointer_cast<IRVerificationCondition>(astNode))
	{
		auto label = "vc" + std::to_string(this->idx++);
		vc->set_debug_str(label);
		data_map[label] = vc->get_str();
	}
	else if (auto assert = std::dynamic_pointer_cast<IRAssert>(astNode))
	{
		auto label = "vc" + std::to_string(this->idx++);
		assert->set_debug_str(label);
		data_map[label] = assert->get_str();
	}

	return functionName;
}

///
/// @brief Traverses the IR, filling in debug info
///
void FillDebugInfo::operator()(std::shared_ptr<IRNode> base)
{
	this->idx = 0;
	this->traverse(base, "");
}

///
/// @brief Looks up the VC from a data_map based on the \@expl attribute marker
///
/// @param key The \@expl attribute marker from Why3
/// @return The raw VC string that it's associated with
///
std::string FillDebugInfo::lookup(std::string key)
{
	if (data_map.count(key))
		return data_map[key];
	else
		return key;
}

///
/// @brief This just passes down the Why3 function name
///
/// @param astNode Unused
/// @param name The Why3 function name
/// @return Returns the "name" parameter
///
std::string LookupIRName::evaluateInheritedAttribute(std::shared_ptr<WhyNode> astNode, std::string name)
{
	return name;
}

///
/// @brief This checks if a node is a WhyFunction and if that function's Why3 name is equal to the "name" parameter. If it is, return its IR name.
///
/// @param astNode The AST node to check
/// @param name The Why3 function name
/// @param list List of possibly found IR names
/// @return The IR name, if we've found it. If not, return "$$NOT_FOUND"
std::string LookupIRName::evaluateSynthesizedAttribute(std::shared_ptr<WhyNode> astNode, std::string name, SynthesizedAttributesList list)
{
	if (auto func = std::dynamic_pointer_cast<WhyFunction>(astNode))
		if (func->get_name() == name)
			return func->get_ir_name();

	for (auto l : list)
		if (l != "$$NOT_FOUND" && l != "")
			return l;

	return "$$NOT_FOUND";
}

///
/// @brief Returns the IR name of a Why3 function name
///
/// @param base The Why3 AST to search
/// @param name The Why3 function name
/// @return The IR name of the Why3 function name
///
std::string LookupIRName::operator()(std::shared_ptr<WhyNode> base, std::string name)
{
	auto func_name = this->traverse(base, name);

	if (func_name == "$$NOT_FOUND")
		return name;
	else
		return func_name.substr(0, func_name.find('#') - 1);
}
