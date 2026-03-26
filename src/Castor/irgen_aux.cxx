// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

// traversal.cxx
#include "irgenerator.hxx"
#include "class_table.hxx"
#include "function_table.hxx"
#include "settings.hxx"
#include "types.hxx"
#include "parser.hxx"
#include "helper.cxx"
#include "messaging.hxx"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <regex>

using namespace Rose;

extern Settings settings;
extern FunctionTable function_table;

///
/// @brief This is a helper function to parse binary operators
///
/// @param astNode Binary operator to parse
/// @param list Synthesized attributes so far
/// @param scope The current scope
/// @return The parsed binary operation
///
std::shared_ptr<IRNode> IRGenerator::parseBinaryOp(SgNode* astNode, SynthesizedAttributesList list, std::shared_ptr<R2WML_Scope> scope)
{
	// this whole function should be self-explanatory
	if (auto ast = isSgAddOp(astNode))
	{
		return std::make_shared<IRAdditionOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgSubtractOp(astNode))
	{
		return std::make_shared<IRSubtractionOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgMultiplyOp(astNode))
	{
		return std::make_shared<IRMultiplyOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgDivideOp(astNode))
	{
		return std::make_shared<IRDivideOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgModOp(astNode))
	{
		return std::make_shared<IRModuloOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgLshiftOp(astNode))
	{
		return std::make_shared<IRBitLShiftOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgRshiftOp(astNode))
	{
		return std::make_shared<IRBitRShiftOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgBitAndOp(astNode))
	{
		return std::make_shared<IRBitAndOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgBitOrOp(astNode))
	{
		return std::make_shared<IRBitOrOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgBitXorOp(astNode))
	{
		return std::make_shared<IRBitXorOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgEqualityOp(astNode))
	{
		return std::make_shared<IREqualsOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgNotEqualOp(astNode))
	{
		return std::make_shared<IRNotEqualsOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgLessThanOp(astNode))
	{
		return std::make_shared<IRLessThanOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgLessOrEqualOp(astNode))
	{
		return std::make_shared<IRLessEqualsOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgGreaterThanOp(astNode))
	{
		return std::make_shared<IRGreaterThanOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgGreaterOrEqualOp(astNode))
	{
		return std::make_shared<IRGreaterEqualsOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgAndOp(astNode))
	{
		return std::make_shared<IRAndOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgOrOp(astNode))
	{
		return std::make_shared<IROrOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgPlusAssignOp(astNode))
	{
		return std::make_shared<IRAdditionAssignOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRLValue>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgMinusAssignOp(astNode))
	{
		return std::make_shared<IRSubtractionAssignOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRLValue>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgMultAssignOp(astNode))
	{
		return std::make_shared<IRMultiplyAssignOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRLValue>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgDivAssignOp(astNode))
	{
		return std::make_shared<IRDivideAssignOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRLValue>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgModAssignOp(astNode))
	{
		return std::make_shared<IRModuloAssignOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRLValue>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgLshiftAssignOp(astNode))
	{
		return std::make_shared<IRBitLShiftAssignOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRLValue>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgRshiftAssignOp(astNode))
	{
		return std::make_shared<IRBitRShiftAssignOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRLValue>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgAndAssignOp(astNode))
	{
		return std::make_shared<IRBitAndAssignOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRLValue>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgIorAssignOp(astNode))
	{
		return std::make_shared<IRBitOrAssignOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRLValue>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgXorAssignOp(astNode))
	{
		return std::make_shared<IRBitXorAssignOp>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRLValue>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgCommaOpExp(astNode))
	{
		// comma operators can be either lvalues or rvalues, so we need to check for that
		if (ast->isLValue())
			return std::make_shared<IRCommaOp<IRLValue>>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRLValue>(list[1]));
		else
			return std::make_shared<IRCommaOp<IRRValue>>(getIRTypeFromSgType(ast->get_type()),
					safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgArrowExp(astNode)) // for arrow expressions
						   // a->b is decomposed into (*a).b
	{
		auto lhs = safety_cast<IRExpression>(list[0]);

		if (auto rhs = std::dynamic_pointer_cast<IRVariableReference>(list[1])) // accessing member variable
			return std::make_shared<IRFieldReference>(getIRTypeFromSgType(ast->get_type()),
				std::make_shared<IRPointerDereference>(
					safety_cast<IRPointerType>(lhs->get_type())->get_base_type(),
					lhs),
				rhs->get_variable()->get_name());
		else
			return std::make_shared<IRFieldReference>(std::make_shared<IRNonRealType>(), // accessing member function
				std::make_shared<IRPointerDereference>(
					safety_cast<IRPointerType>(lhs->get_type())->get_base_type(),
					lhs),
				safety_cast<IRFunctionRefExpr>(list[1])->get_name());
	}
	else if (auto ast = isSgDotExp(astNode)) // for dot expressions
	{
		auto lhs = safety_cast<IRExpression>(list[0]);

		// CodeThorn will implement constructor and destructor calls, but
		// for class variables it doesn't emit "this->" as part of the variable.
		// We need to do this ourselves
		//
		// check if the lhs is a variable reference
		if (auto varref = std::dynamic_pointer_cast<IRVariableReference>(lhs))
		{
			auto var = varref->get_variable();

			if (scope->get_class_variable(var)) // make sure it's a class variable
			{
				auto this_ = scope->lookup("this"); // get the "this" pointer
				auto deref_this = std::make_shared<IRPointerDereference>(
					safety_cast<IRPointerType>(this_->get_type())->get_base_type(),
					std::make_shared<IRVariableReference>(this_)); // create the "this->" indirection
				auto rhs = std::dynamic_pointer_cast<IRFunctionRefExpr>(list[1]); // make sure we're calling a function

				if (rhs) // if we are calling a function, it's a constructor or destructor call from CodeThorn
				{
					// instead of emitting "a.ctor()", we emit "this->a.ctor()"
					return std::make_shared<IRFieldReference>(std::make_shared<IRNonRealType>(),
						std::make_shared<IRFieldReference>(lhs->get_type(), deref_this, varref->get_name()),
						rhs->get_name());
				}
			}
		}

		// this is the same as arrow, but simpler
		if (auto rhs = std::dynamic_pointer_cast<IRVariableReference>(list[1]))
			return std::make_shared<IRFieldReference>(getIRTypeFromSgType(ast->get_type()),
				lhs, rhs->get_variable()->get_name());
		else
			return std::make_shared<IRFieldReference>(std::make_shared<IRNonRealType>(),
				lhs, safety_cast<IRFunctionRefExpr>(list[1])->get_name());
	}
	else if (auto ast = isSgAssignOp(astNode))
	{
		auto lhs = safety_cast<IRExpression>(list[0]);

		// CodeThorn will implement constructor and destructor calls, but
		// for class variables it doesn't emit "this->" as part of the variable.
		// We need to do this ourselves
		//
		// check if the lhs is a variable reference
		if (auto varref = std::dynamic_pointer_cast<IRVariableReference>(lhs))
		{
			auto var = varref->get_variable();

			if (scope->get_class_variable(var)) // make sure it's a class variable
			{
				auto this_ = scope->lookup("this"); // get the "this" pointer
				auto deref_this = std::make_shared<IRPointerDereference>(
					safety_cast<IRPointerType>(this_->get_type())->get_base_type(),
					std::make_shared<IRVariableReference>(this_)); // create the "this->" indirection
				auto new_lhs = std::make_shared<IRFieldReference>(lhs->get_type(), deref_this, var->get_name());

				// instead of emitting "a = b", we emit "this->a = b"
				return std::make_shared<IRAssignOp>(getIRTypeFromSgType(ast->get_type()),
					new_lhs, safety_cast<IRExpression>(list[1]));
			}
		}

		return std::make_shared<IRAssignOp>(getIRTypeFromSgType(ast->get_type()),
				safety_cast<IRLValue>(list[0]), safety_cast<IRExpression>(list[1]));
	}
	else
	{
		throw UnknownSgNodeException(astNode->class_name());
	}
}

///
/// @brief This is a helper function to parse literals
///
/// @param astNode Literal to parse
/// @param list Synthesized attributes so far
/// @return The parsed literal
///
std::shared_ptr<IRNode> IRGenerator::parseLiteral(SgNode* astNode, SynthesizedAttributesList list)
{
	std::shared_ptr<IRLiteral> ret;

	// this should be easy to understand
	if (auto ast = isSgIntVal(astNode))
	{
		ret = std::make_shared<IRS32Literal>(ast->get_value());
	}
	else if (auto ast = isSgLongIntVal(astNode))
	{
		ret = std::make_shared<IRS64Literal>(ast->get_value());
	}
	else if (auto ast = isSgUnsignedIntVal(astNode))
	{
		ret = std::make_shared<IRU32Literal>(ast->get_value());
	}
	else if (auto ast = isSgUnsignedLongVal(astNode))
	{
		ret = std::make_shared<IRU64Literal>(ast->get_value());
	}
	else if (auto ast = isSgLongLongIntVal(astNode))
	{
		ret = std::make_shared<IRS64Literal>(ast->get_value());
	}
	else if (auto ast = isSgUnsignedLongLongIntVal(astNode))
	{
		ret = std::make_shared<IRU64Literal>(ast->get_value());
	}
	else if (auto ast = isSgShortVal(astNode))
	{
		ret = std::make_shared<IRS16Literal>(ast->get_value());
	}
	else if (auto ast = isSgUnsignedShortVal(astNode))
	{
		ret = std::make_shared<IRU16Literal>(ast->get_value());
	}
	else if (auto ast = isSgBoolValExp(astNode))
	{
		return std::make_shared<IRBoolLiteral>(ast->get_value());
	}
	else if (auto ast = isSgCharVal(astNode))
	{
		ret = std::make_shared<IRS8Literal>(ast->get_value());
	}
	else if (auto ast = isSgUnsignedCharVal(astNode))
	{
		ret = std::make_shared<IRU8Literal>(ast->get_value());
	}
	else if (auto ast = isSgWcharVal(astNode))
	{
		ret = std::make_shared<IRU8Literal>(ast->get_value());
	}
	else if (auto ast = isSgEnumVal(astNode))
	{
		auto base_type = safety_cast_raw<SgEnumDeclaration*>(
					safety_cast_raw<SgEnumType*>(
						ast->get_type())->
					get_declaration())->
				get_field_type();

		auto ir_type = base_type ? getIRTypeFromSgType(base_type) : std::make_shared<IRS32Type>();

		if (std::dynamic_pointer_cast<IRS8Type>(ir_type))
			ret = std::make_shared<IRS8Literal>(ast->get_value());
		else if (std::dynamic_pointer_cast<IRU8Type>(ir_type))
			ret = std::make_shared<IRU8Literal>(ast->get_value());
		else if (std::dynamic_pointer_cast<IRS16Type>(ir_type))
			ret = std::make_shared<IRS16Literal>(ast->get_value());
		else if (std::dynamic_pointer_cast<IRU16Type>(ir_type))
			ret = std::make_shared<IRU16Literal>(ast->get_value());
		else if (std::dynamic_pointer_cast<IRS32Type>(ir_type))
			ret = std::make_shared<IRS32Literal>(ast->get_value());
		else if (std::dynamic_pointer_cast<IRU32Type>(ir_type))
			ret = std::make_shared<IRU32Literal>(ast->get_value());
		else if (std::dynamic_pointer_cast<IRS64Type>(ir_type))
			ret = std::make_shared<IRS64Literal>(ast->get_value());
		else if (std::dynamic_pointer_cast<IRU64Type>(ir_type))
			ret = std::make_shared<IRU64Literal>(ast->get_value());
		else if (std::dynamic_pointer_cast<IRUnboundedIntegralType>(ir_type))
			ret = std::make_shared<IRS64Literal>(ast->get_value());
		else
			throw CastorException("Unknown integral type as base type for enum: " + base_type->class_name());
	}
	else if (isSgNullptrValExp(astNode))
	{
		return std::make_shared<IRNullptr>();
	}
	else
	{
		throw UnknownSgNodeException(astNode->class_name());
	}

	if (!settings.overflow_checking)
		return std::make_shared<IRCast<IRRValue>>(std::make_shared<IRUnboundedIntegralType>(), ret);
	else
		return ret;
}

///
/// @brief Attaches verification conditions to functions.
///
/// This function attaches a verification condition to the next legal construct which can accept a verification condition.
///
/// @param item The verification condition to attach
/// @param start_idx The place in list to start searching from
/// @param list Vector of IRNode pointers that we might want to attach a VC to
///
void IRGenerator::attach_ver(std::shared_ptr<IRVerificationCondition> item, int start_idx, std::vector<std::shared_ptr<IRNode>> list)
{
	auto added = 0;
	for (int i = start_idx; i < list.size(); i++) // iterate through the list
	{
		if (auto f = std::dynamic_pointer_cast<IRFunction>(list[i])) // if we find a function
		{
			f->add_vc(item); // attach the VC
			//if (f->get_is_template())
			//	propagate_ver(item, f->get_name(), list); // and propagate the VC if it's a template function
			added++;
			break;
		}
	}

	if (!added) // if we couldn't find a function to attach a VC to, we need to let the user know
		throw CastorException("Was not able to find a definition to attach VC to -> " + item->get_str());
}

///
/// @brief Propagates verification conditions to template instantiations.
///
/// This function ensures that verification conditions are properly attached to functions when dealing with templates.
/// In the source, we annotate template functions with verification conditions.
/// ROSE will instantiate these functions and create new definitions for each unique instantiation.
/// We want to make sure to attach the original verification conditions to each instantiation.
///
/// @param item The verification condition to attach
/// @param name The name of the non-instantiated function
/// @param list Vector of IRNode pointers that we might want to attach a VC to
///
void IRGenerator::propagate_ver(std::shared_ptr<IRVerificationCondition> item, IRName name, std::vector<std::shared_ptr<IRStatement>> list)
{
	auto added = false; // we want to throw an exception if we couldn't find an instantiation to attach to

	for (auto l : list) if (auto func = std::dynamic_pointer_cast<IRFunction>(l)) // find all IRFunctions in the list
	{
		if (func->get_name() == name || !func->get_is_template_instantiation()) // if it's either the function we're looking at
											// or not a template instantiation, ignore it
			continue;

		//auto abstract_function_name = std::string(name); // get the name of the non-instantiated template function (has the VC to propagate)
		auto concrete_function_name = func->get_name().substr(0, func->get_name().rfind('#') - 1);  // get the name of the instantiated template function (receives the VC)

		std::string regex_matcher;

		for (auto c : name.substr(0, name.rfind('#') - 1)) // iterate through the name of the non-instantiated function
		{
			if (std::string(".*+?^${}()|[]\\").find(c) != std::string::npos) // if it's a regex special character
				regex_matcher += "\\"; // make sure to escape it

			regex_matcher += c; // add the character to our string
		}

		regex_matcher = std::regex_replace(regex_matcher, std::regex("nonreal"), ".*"); // replace any instance of "nonreal" with a repeated wildcard
		concrete_function_name = std::regex_replace(concrete_function_name, std::regex(" < .* > "), ""); // get rid of any angle brackets

		auto regex = std::regex(regex_matcher); // create the regex

		if (std::regex_match(concrete_function_name, regex)) // if the functions are a match, that means that the instantiated template function
								     // is an instantiation of our non-instantiated function. this means that we attach the VC
		{
			log("Matched template function \"" + std::string(name)
					+ "\" with template instantiation \"" + func->get_name() + "\"", LogType::INFO, 2); // log to output if necessary

			auto new_vc = safety_cast<IRVerificationCondition>(
				VCParser::parse_ver(item->get_str(),
					func->get_is_ref() ? VCParser::LValueFunc : VCParser::RValueFunc)); // re-parse the VC to get a new IRVerificationCondition object

			new_vc->attach_str(item->get_str()); // attach the VC string

			func->add_vc(new_vc); // add it to the function
			added = true; // note that we've attached the VC to at least one instantiation
		}
	}

	// if we couldn't find any instantiations to attach the VC to, let's let the user know
	//if (!added)
	//	throw CastorException("Was not able to find an instantiation to attach VC to -> " + item->get_str());
}

///
/// @brief Attaches loop annotations to loops.
///
/// This function takes a vector of statements, and makes sure that invariants which need to be attached to loops
/// (like invariants, variants, and so on) are properly attached to their loops and deleted from the vector.
///
/// @param statements Reference to vector of IRStatement pointers
///
void IRGenerator::attach_invariants(std::vector<std::shared_ptr<IRStatement>>& statements)
{
	for (int i = 0; i < statements.size(); i++) // iterate through the statement block
	{
		if (auto vc = std::dynamic_pointer_cast<IRVerificationCondition>(statements[i])) // look for verification conditions
		{
			for (int j = i + 1; j < statements.size(); j++) // iterate through the rest of the block
			{
				if (auto loop = std::dynamic_pointer_cast<IRLoopStmt>(statements[j])) // and attach the VC to loops
				{
					loop->add_vc(vc);
					break;
				}
				else if (auto block = std::dynamic_pointer_cast<IRStatementBlock>(statements[j])) // the loop might be in a statement block
														  // in the case of range-based for loops.
														  // for this, we check for blocks
				{
					for (auto statement : block->get_statements()) // then iterate through its statements
					{
						if (auto loop = std::dynamic_pointer_cast<IRLoopStmt>(statement)) // and attach the VC to loops
						{
							loop->add_vc(vc);
							j = statements.size();
							break;
						}
					}
				}
			}

			statements.erase(std::next(statements.begin(), i--)); // now we can get rid of the verification condition from the statement block
		}
	}
}

///
/// @brief Attaches implementations to function prototypes.
///
/// The IR does not support "prototypes" (i.e. declarations without definitions).
/// An IRFunction must contain its definition, so after traversing the SAGE AST, we call this function
/// in order to consolidate this issue.
///
/// @param globals A reference to a vector of statements in the global namespace
///
void IRGenerator::attach_impl(std::vector<std::shared_ptr<IRStatement>>& globals)
{
	for (int i = 0; i < globals.size(); i++) // iterate through the global items
	{
		auto g = globals[i];

		if (auto impl = std::dynamic_pointer_cast<IRFunction>(g)) // look for functions
		{
			auto name = impl->get_name();

			for (int j = 0; j < globals.size(); j++) // iterate through the global items again
			{
				if (auto clas = std::dynamic_pointer_cast<IRClass>(globals[j])) // look for classes
				{
					auto funcs = clas->get_funcs(); // get their list of functions

					for (int f_idx = 0; f_idx < funcs.size(); f_idx++) // iterate through class functions
					{
						auto f = funcs[f_idx];

						if (f->get_name() == name) // if the names match
						{
							merge_vcs(impl, f); // merge the VCs

							clas->set_func(impl, f_idx); // set the function implementation
							globals.erase(std::next(globals.begin(), i--)); // erase the function from the global namespace
							goto attach_impl_outer_break;
						}
					}
				}
				else if (auto f = std::dynamic_pointer_cast<IRFunction>(globals[j])) // look for functions
				{
					if (f->get_name() == name) // if the names match
					{
						merge_vcs(impl, f); // merge the VCs
					}
				}
			}
		}

attach_impl_outer_break:;
	}
}

///
/// @brief Merges the verification conditions of two functions.
///
/// This usually takes a function prototype and a function definition.
/// In the case that one function pointer has VCs and the other one doesn't, the VCs are copied
/// from the function with VCs to the one without. In the case that both functions have VCs,
/// they are checked to make sure they're identical (but potentially in a different order).
///
/// @param func1 The first function
/// @param func2 The second function
///
void IRGenerator::merge_vcs(std::shared_ptr<IRFunction> func1, std::shared_ptr<IRFunction> func2)
{
	// get the VCs of both functions
	auto f1_vcs = func1->get_vcs();
	auto f2_vcs = func2->get_vcs();

	// if func1 has VCs and func2 doesn't
	if (f1_vcs.size() && !f2_vcs.size())
	{
		// copy func1's VCs to func2
		for (auto vc : f1_vcs)
		{
			auto reparsed = safety_cast<IRVerificationCondition>(VCParser::parse_ver(vc->get_str(), func2->get_is_ref() ? VCParser::LValueFunc : VCParser::RValueFunc));
			reparsed->attach_str(vc->get_str());
			func2->add_vc(reparsed);
		}
	}
	// if func2 has VCs and func1 doesn't
	else if (f2_vcs.size() && !f1_vcs.size())
	{
		// copy func2's VCs to func1
		for (auto vc : f2_vcs)
		{
			auto reparsed = safety_cast<IRVerificationCondition>(VCParser::parse_ver(vc->get_str(), func1->get_is_ref() ? VCParser::LValueFunc : VCParser::RValueFunc));
			reparsed->attach_str(vc->get_str());
			func1->add_vc(reparsed);
		}
	}
	// if both functions have VCs
	else if (f1_vcs.size() && f2_vcs.size())
	{
		// create a basic error message
		const auto error_message = "Function " + func1->get_name() + " has differing VCs between prototype and implementation.";

		// throw an exception if there's differing numbers of VCs between the prototype and implementation
		if (f1_vcs.size() != f2_vcs.size())
			throw CastorException(error_message);

		// we want to be agnostic to the order of VCs, so this approach is intended to enable that
		// in the future: make this whitespace agnostic
		//
		// iterate through func1's VCs
		for (auto vc1 : f1_vcs)
		{
			bool found = false;

			// iterate through func2's VCs
			for (int i = 0; i < f2_vcs.size(); i++)
			{
				auto vc2 = f2_vcs[i];

				// if we've found a matching pair
				if (vc1->get_str() == vc2->get_str())
				{
					// erase it from func2's VCs so we won't see it again
					f2_vcs.erase(std::next(f2_vcs.begin(), i));
					// mark it as found
					found = true;
					break;
				}
			}

			// if we didn't find a matching VC, throw that exception
			if (!found)
				throw CastorException(error_message);
		}
	}
}

///
/// @brief Cleans up the AST
///
/// @param globals Reference to vector of IRStatement pointers to tidy up.
///
void IRGenerator::tidy_ast(std::vector<std::shared_ptr<IRStatement>>& globals)
{
	for (int i = 0; i < globals.size(); i++) // iterate through the globals
	{
		if (auto f = std::dynamic_pointer_cast<IRFunction>(globals[i]); f && f->get_is_template()) // if we see a template function
		{
			auto vcs = f->get_vcs();
			
			for (auto vc : vcs)
				propagate_ver(vc, f->get_name(), globals); // propagate its VCs
		}
		else if (auto c = std::dynamic_pointer_cast<IRClass>(globals[i])) // if we see a class
		{
			for (auto f : c->get_funcs()) if (f->get_is_template()) // find all its template functions
			{
				auto vcs = f->get_vcs();

				for (auto vc : vcs)
					propagate_ver(vc, f->get_name(), globals); // propagate its VCs
			}
		}
	}

	for (int i = 0; i < globals.size(); i++) // iterate through the globals
	{
		if (auto f = std::dynamic_pointer_cast<IRFunction>(globals[i]))
		{
			if (!f->has_body() && (!function_table.declared_without_definition(f->get_name()) || f->get_is_template()))
				globals.erase(std::next(globals.begin(), i--)); // if we see a function without a body, get rid of it
		}
		else if (auto c = std::dynamic_pointer_cast<IRClass>(globals[i])) // if we see a class
			c->tidy_funcs(); // tell it to tidy its functions
	}

	for (int i = 0; i < globals.size(); i++) // iterate through the globals
	{
		if (auto f = std::dynamic_pointer_cast<IRFunction>(globals[i]))
		{
			auto name = f->get_name();

			// check for duplicate functions and get rid of them
			// this can occur if two or more function prototypes exist in the source without a corresponding definition
			for (int j = i + 1; j < globals.size(); j++)
			{
				if (auto g = std::dynamic_pointer_cast<IRFunction>(globals[j]);
						g && g->get_name() == name)
					globals.erase(std::next(globals.begin(), j--));
			}
		}
		else if (auto c = std::dynamic_pointer_cast<IRClass>(globals[i]))
		{
			auto name = c->get_name();

			// check for duplicate classes and get rid of them
			// I'm not sure how this situation happens
			// do we need to get rid of this code?
			for (int j = i + 1; j < globals.size(); j++)
			{
				if (auto d = std::dynamic_pointer_cast<IRClass>(globals[j]);
						d && d->get_name() == name)
					globals.erase(std::next(globals.begin(), j--));
			}
		}
	}
}

std::map<IRName, std::vector<std::shared_ptr<IRVariable>>> template_var_table; ///< Table to handle template variables

///
/// @brief This function parses template arguments from uninstantiated template functions.
///
/// @param ast_node The template function SAGE object
/// @tparam T Must be either SgTemplateFunctionDeclaration* or SgTemplateMemberFunctionDeclaration*
/// @return Vector of variables, initialized accordingly
///
template <typename T>
std::vector<std::shared_ptr<IRVariable>> IRGenerator::getTemplateParams(T ast_node)
{
	std::vector<std::shared_ptr<IRVariable>> template_vars;

	auto template_args = ast_node->get_templateParameters(); // get the template parameters
	if (auto cls = dynamic_cast<SgTemplateMemberFunctionDeclaration*>(ast_node)) // if this is a template member function...
	{
		if (auto templateclass = dynamic_cast<SgTemplateClassDeclaration*>(cls->get_associatedClassDeclaration())) // ...that lies inside a template class...
		{
			// copy over the class's template parameters
			auto class_template_args = templateclass->get_templateParameters();
			template_args.insert(template_args.end(), class_template_args.begin(), class_template_args.end());
		}
	}

	for (auto t : template_args) // iterate through the template parameters
		if (auto casted = isSgNonrealType(t->get_type())) // if it's a nonreal
			template_vars.push_back(std::make_shared<IRVariable>(std::make_shared<IRNonRealType>(), casted->get_name(), casted->get_name(), true)); // push a nonreal variable
		else // otherwise, we need its type
		{
			template_vars.push_back(std::make_shared<IRVariable>(getIRTypeFromSgType(t->get_type()),
				t->get_initializedName()->get_name(), t->get_initializedName()->get_name(), true));
		}

	template_var_table[ast_node->get_qualified_name()] = template_vars; // assign the found template variables to this function's entry in the table

	return template_vars;
}

///
/// @brief This function parses template arguments from instantiated template functions.
///
/// This function is most useful for cases where the template parameter is a non-typename object (e.g. `int`).
/// In these cases, a variable is created with its constexpr set to the instantiated value.
/// This lets us use this template variable as a normal variable in verification conditions.
///
/// @param ast_node The template function instantiation SAGE object
/// @param scope The current scope
/// @tparam T Must be either SgTemplateInstantiationFunctionDecl* or SgTemplateInstantiationMemberFunctionDecl*
/// @return Vector of variables, initialized accordingly
///
template <typename T>
std::vector<std::shared_ptr<IRVariable>> IRGenerator::getTemplateArgs(T ast_node, std::shared_ptr<R2WML_Scope> scope)
{
	std::vector<std::shared_ptr<IRVariable>> template_vars;

	auto template_args = ast_node->get_templateArguments(); // get the template arguments
	if (auto cls = dynamic_cast<SgTemplateInstantiationMemberFunctionDecl*>(ast_node)) // if it's a template instantiation member function
	{
		// get the class's template arguments
		auto class_template_args = safety_cast_raw<SgTemplateInstantiationDecl*>(cls->get_associatedClassDeclaration())->get_templateArguments();
		template_args.insert(template_args.end(), class_template_args.begin(), class_template_args.end());
	}
	auto template_decl = template_var_table[ast_node->get_templateDeclaration()->get_qualified_name()]; // get the declared template variables

	for (int i = 0; i < template_args.size(); i++)
	{
		if (auto type = template_args[i]->get_type()) // if its type has been specified (i.e. it was a `typename` parameter
							      // then we just grab the instantiated type
			template_vars.push_back(std::make_shared<IRVariable>(getIRTypeFromSgType(type),
						template_decl[i]->get_name(), template_decl[i]->get_name(), true));
		else // otherwise, it was something like `template <int T>`
		{
			auto expr = safety_cast<IRLiteral>(evaluateSynthesizedAttribute(
					template_args[i]->get_expression(), { Nothing, scope }, SynthesizedAttributesList())); // evaluate the expression it was
															       // instantiated to
			template_vars.push_back(std::make_shared<IRVariable>(expr->get_type(),
						template_decl[i]->get_name(), template_decl[i]->get_name(), expr, true)); // and give the variable a
														    // constexpr value
		}
	}

	return template_vars;
}

///
/// @brief Gets a good function name for the IR.
///
/// @param qualified_name The qualified name of the function from the SAGE tree
/// @param params Function parameters
/// @param mangled_name ROSE's mangled name
/// @tparam T A shared pointer type to either IRVariable (for the function declaration) or IRExpression (callsite)
/// @return Name to refer to this function in the IR
///
template<typename T>
IRName IRGenerator::get_func_name(IRName qualified_name, std::vector<T> params, IRName mangled_name)
{
	auto ret = qualified_name + " #"; // "#" separates the qualified name from its parameters
	for (auto p : params)
		ret += p->get_type()->pp() + ","; // insert the parameter type followed by commas
	ret += " #" + mangled_name; // another "#" to separate the parameters from the mangled name (to guarantee no collisions occur)
	return ret;
}

///
/// @brief Finds the SgFunctionDeclaration that an SgPragma is attached to.
///
/// @param pragma Pragma to use as basis for search
/// @param found_function Set to the associated function if the function was found, otherwise do nothing
/// @return Whether or not we found the function
///
bool IRGenerator::find_function(SgPragma* pragma, SgFunctionDeclaration*& found_function)
{
	auto comp = pragma->get_parent();
	auto parent = comp->get_parent();
	auto container = parent->get_traversalSuccessorContainer();
	bool found_pragma = false;

	for (auto node : container) // iterate through the space that the pragma exists in
	{
		if (!found_pragma && node == comp) // have we found our pragma?
		{
			found_pragma = true;
			continue;
		}

		if (found_pragma && (found_function = isSgFunctionDeclaration(node))) // if we've found our pragma and we're looking at a function
										      // then set the function and return true
		{
			return true;
		}
	}

	return false;
}

template std::vector<std::shared_ptr<IRVariable>> IRGenerator::getTemplateParams<SgTemplateFunctionDeclaration*>(SgTemplateFunctionDeclaration* ast_node);
template std::vector<std::shared_ptr<IRVariable>> IRGenerator::getTemplateParams<SgTemplateMemberFunctionDeclaration*>(SgTemplateMemberFunctionDeclaration* ast_node);
template std::vector<std::shared_ptr<IRVariable>> IRGenerator::getTemplateArgs<SgTemplateInstantiationFunctionDecl*>(SgTemplateInstantiationFunctionDecl* ast_node, std::shared_ptr<R2WML_Scope> scope);
template std::vector<std::shared_ptr<IRVariable>> IRGenerator::getTemplateArgs<SgTemplateInstantiationMemberFunctionDecl*>(SgTemplateInstantiationMemberFunctionDecl* ast_node, std::shared_ptr<R2WML_Scope> scope);
template IRName IRGenerator::get_func_name(IRName qualified_name, std::vector<std::shared_ptr<IRVariable>> params, IRName mangled_name);
template IRName IRGenerator::get_func_name(IRName qualified_name, std::vector<std::shared_ptr<IRExpression>> params, IRName mangled_name);

///
/// @brief Tests if two IR types are the same type, sans "const" qualifiers
///
/// @param a The first type parameter
/// @param b The second type parameter
/// @return true if equal, false otherwise
///
bool IRGenerator::are_same_type_without_const(std::shared_ptr<IRType> a, std::shared_ptr<IRType> b)
{
	if (auto newfirst = std::dynamic_pointer_cast<IRLabelType>(a),         // if both label types, they are the same
			newsecond = std::dynamic_pointer_cast<IRLabelType>(b);
			newfirst && newsecond)
		return true;
	else if (auto newfirst = std::dynamic_pointer_cast<IRVoidType>(a),     // if both void types, they are the same
			newsecond = std::dynamic_pointer_cast<IRVoidType>(b);
			newfirst && newsecond)
		return true;
	else if (auto newfirst = std::dynamic_pointer_cast<IRBoolType>(a),     // if both bool types, they are the same
			newsecond = std::dynamic_pointer_cast<IRBoolType>(b);
			newfirst && newsecond)
		return true;
	else if (auto newfirst = std::dynamic_pointer_cast<IRNonRealType>(a),  // if both nonreal types, they are the same
			newsecond = std::dynamic_pointer_cast<IRNonRealType>(b);
			newfirst && newsecond)
		return true;
	else if (auto newfirst = std::dynamic_pointer_cast<IRUnknownType>(a),  // if both unknown types, they are the same
			newsecond = std::dynamic_pointer_cast<IRUnknownType>(b);
			newfirst && newsecond)
		return true;
	else if (auto newfirst = std::dynamic_pointer_cast<IRArrayType>(a),    // if both array types
			newsecond = std::dynamic_pointer_cast<IRArrayType>(b);
			newfirst && newsecond)
		return newfirst->get_size() == newsecond->get_size() &&        // they are the same if they have the same size and base type
			are_same_type_without_const(newfirst->get_base_type(), newsecond->get_base_type());
	else if (auto newfirst = std::dynamic_pointer_cast<IRClassType>(a),    // if both class types
			newsecond = std::dynamic_pointer_cast<IRClassType>(b);
			newfirst && newsecond)
		return newfirst->get_class_name() == newsecond->get_class_name(); // they are the same if they have the same qualified name
	else if (auto newfirst = std::dynamic_pointer_cast<IRPointerType>(a),  // if both pointer types
			newsecond = std::dynamic_pointer_cast<IRPointerType>(b); // they are the same if they have the same base type
			newfirst && newsecond)
		return are_same_type_without_const(newfirst->get_base_type(), newsecond->get_base_type());
	else if (auto newfirst = std::dynamic_pointer_cast<IRIntegralType>(a), // if both integral types
			newsecond = std::dynamic_pointer_cast<IRIntegralType>(b);
			newfirst && newsecond)
		return newfirst->get_bits() == newsecond->get_bits() &&        // they are the same if they have the same bits and signedness
			newfirst->get_is_signed() == newsecond->get_is_signed();
	else if (auto newfirst = std::dynamic_pointer_cast<IRConstType>(a))    // if a is a const type
		return are_same_type_without_const(newfirst->get_base_type(), b); // strip it and check again
	else if (auto newsecond = std::dynamic_pointer_cast<IRConstType>(b))   // if b is a const type
		return are_same_type_without_const(a, newsecond->get_base_type()); // strip it and check again
	else								       // not the same type
		return false;
}
