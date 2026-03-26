// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "set_metadata.hxx"

///
/// @brief The downwards traversal.
///
/// @param astNode The WhyNode we're looking at
/// @param inheritedValue The inherited value that's been passed down
/// @return The new inherited value
///
SetMetaIV SetMetadata::evaluateInheritedAttribute(std::shared_ptr<WhyNode> astNode, SetMetaIV inheritedValue)
{
	auto vars = inheritedValue.vars;
	auto in_vc = inheritedValue.in_vc;
	auto in_loop = inheritedValue.in_loop;
	auto quant_vars = inheritedValue.quantifier_vars;
	auto in_quantifier = inheritedValue.in_quantifier;

	if (auto func = std::dynamic_pointer_cast<WhyFunction>(astNode)) // grab function parameters
		vars = func->get_vars();
	else if (std::dynamic_pointer_cast<WhyStatementCollection>(astNode)) // if we have a collection of statements
									     // get an empty vector of variables
		vars = std::vector<std::shared_ptr<WhyVariable>>();
	if (auto loop = std::dynamic_pointer_cast<WhyLoop>(astNode)) // for loops, grab its variables in scope
		vars = loop->get_vars(), in_loop = true;
	if (std::dynamic_pointer_cast<WhyVerificationCondition>(astNode) || std::dynamic_pointer_cast<WhyAssert>(astNode)
			|| std::dynamic_pointer_cast<WhyAssume>(astNode) || std::dynamic_pointer_cast<WhyLemma>(astNode)) // check if we're in a VC
		in_vc = true;
	else if (std::dynamic_pointer_cast<WhyStatement>(astNode)) // used if an assertion has a continuation
		in_vc = false;


	if (auto quant = std::dynamic_pointer_cast<WhyQuantifier>(astNode)) // for quantifiers
	{
		auto vars = quant->get_vars();
		quant_vars.insert(quant_vars.begin(), vars.begin(), vars.end()); // we need to fetch the quantifier variables

		in_quantifier = true;
	}

	return { vars, in_vc, in_quantifier, quant_vars, in_loop };
}

///
/// @brief The upwards traversal.
///
/// @param astNode The WhyNode we're looking at
/// @param inheritedValue The inherited value we've received
/// @param list List of synthesized attributes so far
/// @return The new synthesized attribute
///
std::shared_ptr<WhyNode> SetMetadata::evaluateSynthesizedAttribute(std::shared_ptr<WhyNode> astNode, SetMetaIV inheritedValue, SynthesizedAttributesList list)
{
	// this code sets the accessible variables for certain loop VC constructs
	// this is necessary to properly generate the VCs
	if (auto valid = std::dynamic_pointer_cast<WhyValid>(astNode); valid && !inheritedValue.in_loop)
		valid->set_vars(inheritedValue.vars);
	else if (auto valid = std::dynamic_pointer_cast<WhySeparated>(astNode); valid && !inheritedValue.in_loop)
		valid->set_vars(inheritedValue.vars);
	else if (auto valid = std::dynamic_pointer_cast<WhyValidArray>(astNode); valid && !inheritedValue.in_loop)
		valid->set_vars(inheritedValue.vars);
	// writes and frees can appear on a function or on a loop
	// we need to set this flag if it's on a loop
	else if (auto valid = std::dynamic_pointer_cast<WhyWrites>(astNode); valid && inheritedValue.in_loop)
		valid->set_in_loop();
	else if (auto valid = std::dynamic_pointer_cast<WhyFrees>(astNode); valid && inheritedValue.in_loop)
		valid->set_in_loop();
	// rvalue function calls are handled differently if in VCs
	else if (auto fc = std::dynamic_pointer_cast<WhyFunctionCall<WhyRValue>>(astNode); fc && inheritedValue.in_vc)
		fc->set_in_vc();
	// if we're in a quantifier and we see a variable, we need to do something special
	else if (auto varref = std::dynamic_pointer_cast<WhyVariableReference>(astNode); varref && inheritedValue.in_quantifier)
	{
		auto var = safety_cast<WhyVariable>(list[0]);
		auto vars = inheritedValue.quantifier_vars;

		bool is_quant = false;
		for (auto v : vars)
			if (v->get_name() == var->get_name())
				is_quant = true; // check if the variable is a quantifier variable

		if (is_quant)
			varref->set_in_quantifier(); // if it is, set this flag
	}
	// error handling
	else if (std::dynamic_pointer_cast<WhyAddressOfExpr>(astNode) || std::dynamic_pointer_cast<WhyPointerDereference>(astNode))
	{
		if (auto varref = std::dynamic_pointer_cast<WhyVariableReference>(list[0]); varref && varref->get_in_quantifier())
			throw CastorException("Cannot take the address of a quantifier variable!"); // can't take the address of a quantifier variable
	}

	return astNode;
}

///
/// @brief Constructor
///
SetMetadata::SetMetadata() = default;

///
/// @brief Functor, so that we can use instances of this class as a function
///
/// @param base The base WhyNode to traverse on
/// @return The new WhyNode
///
std::shared_ptr<WhyNode> SetMetadata::operator()(std::shared_ptr<WhyNode> base)
{
	// set the initial IVs and begin the traversal
	return this->traverse(base, { std::vector<std::shared_ptr<WhyVariable>>(), false, false, std::vector<std::shared_ptr<WhyVariable>>(), false });
}
