// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "validate_ir.hxx"
#include "class_table.hxx"
#include "function_table.hxx"
#include "exception.hxx"
#include "symbol_table.hxx"
#include "settings.hxx"
#include "irgenerator.hxx"
#include "messaging.hxx"
#include <tuple>
#include <climits>
#include <iostream>
#include <boost/algorithm/string.hpp>

extern ClassTable class_table;
extern FunctionTable function_table;
extern Settings settings;

using namespace Sawyer::Message::Common;

///
/// @brief Downwards traversal
///
/// @param astNode The IR node we're looking at
/// @param inheritedValue The inherited values so far
/// @return The new inherited values
///
ValidateIVs ValidateIR::evaluateInheritedAttribute(std::shared_ptr<IRNode> astNode, ValidateIVs inheritedValue)
{
	auto inside_ensures = inheritedValue.inside_ensures;
	auto inside_vc = inheritedValue.inside_vc;
	auto inside_lemma = inheritedValue.inside_lemma;
	auto inside_loop = inheritedValue.inside_loop;

	if (std::dynamic_pointer_cast<IREnsures>(astNode)) // make a note if we're inside an ensures clause
		inside_ensures = true;

	if (std::dynamic_pointer_cast<IRVerificationCondition>(astNode) || // make a note if we're inside a verification condition
			std::dynamic_pointer_cast<IRAssume>(astNode) ||
			std::dynamic_pointer_cast<IRAssert>(astNode))
		inside_vc = true;

	if (std::dynamic_pointer_cast<IRLemma>(astNode)) // same for lemmas
		inside_lemma = true;

	if (std::dynamic_pointer_cast<IRLoopStmt>(astNode) ||
			std::dynamic_pointer_cast<IRSwitchStmt>(astNode)) // same for loops and switch statements
		inside_loop = true;

	return { inside_ensures, inside_vc, inside_lemma, inside_loop };
}

///
/// @brief Upwards traversal, validates the IR
///
/// @param astNode The IR node to check
/// @param inheritedValue The inherited values
/// @param list The child nodes
/// @return Equal to astNode
///
std::shared_ptr<IRNode> ValidateIR::evaluateSynthesizedAttribute(std::shared_ptr<IRNode> astNode, ValidateIVs inheritedValue, SynthesizedAttributesList list)
{
	// if we encounter an expression with an unknown type, immediately terminate
	// the type filler couldn't infer this type, so we can't continue
	if (auto expr = std::dynamic_pointer_cast<IRExpression>(astNode))
	{
		if (std::dynamic_pointer_cast<IRUnknownType>(expr->get_type()))
		{
			throw CannotInferTypeException(expr->pp());
		}
	}

	// if a "result" keyword is used outside an "ensures" clause, this is fatal
	if ((std::dynamic_pointer_cast<IRResult<IRRValue>>(astNode) ||
		std::dynamic_pointer_cast<IRResult<IRLValue>>(astNode)) &&
		!inheritedValue.inside_ensures)
	{
		throw CastorException("Cannot use \"result\" keyword outside of an \"ensures\" clause!");
	}

	// now we need to check that casts are being used properly
	if (std::dynamic_pointer_cast<IRCast<IRLValue>>(astNode) || std::dynamic_pointer_cast<IRCast<IRRValue>>(astNode))
	{
		// see if either the type we're casting to or from is a pointer type
		std::shared_ptr<IRPointerType> from_type, to_type;
		std::shared_ptr<IRExpression> base_expr;

		if (auto cast = std::dynamic_pointer_cast<IRCast<IRLValue>>(astNode))
		{
			from_type = std::dynamic_pointer_cast<IRPointerType>(cast->get_base_expr()->get_type());
			to_type = std::dynamic_pointer_cast<IRPointerType>(cast->get_type());
			base_expr = cast->get_base_expr();
		}
		else
		{
			auto rcast = safety_cast<IRCast<IRRValue>>(astNode);
			from_type = std::dynamic_pointer_cast<IRPointerType>(rcast->get_base_expr()->get_type());
			to_type = std::dynamic_pointer_cast<IRPointerType>(rcast->get_type());
			base_expr = rcast->get_base_expr();
		}

		// if they're both pointer types
		if (to_type && from_type)
		{
			// see if either of them is a pointer-to-class type
			auto from_class = std::dynamic_pointer_cast<IRClassType>(from_type->get_base_type());
			auto to_class = std::dynamic_pointer_cast<IRClassType>(to_type->get_base_type());

			// if they both are
			if (from_class && to_class)
			{
				// we need to ensure that one of them inherits from the other
				// otherwise, the cast is illegal
				if (!(class_table.inherits_from(from_class->get_class_name(), to_class->get_class_name()) ||
					class_table.inherits_from(to_class->get_class_name(), from_class->get_class_name())))
				{
					throw CastorException("Cannot cast between pointer-to-class types if one doesn't derive from the other!");
				}
			}
			// if we aren't converting from a nullptr, we need to throw an exception
			// casts between pointer-to-non-class types aren't supported
			else if (!std::dynamic_pointer_cast<IRNullptr>(base_expr))
			{
				throw CastorException("Cannot cast between pointer-to-non-class types!");
			}
		}
	}

	// if we're not inside a VC and we're calling a function, we want to check it to see
	// if it has VCs like it's supposed to
	// usually we don't throw exceptions here, just warnings
	if (!inheritedValue.inside_vc &&
			(std::dynamic_pointer_cast<IRFunctionCallExpr<IRLValue>>(astNode) ||
			 std::dynamic_pointer_cast<IRFunctionCallExpr<IRRValue>>(astNode) ||
			 std::dynamic_pointer_cast<IRMemberFunctionCallExpr<IRLValue>>(astNode) ||
			 std::dynamic_pointer_cast<IRMemberFunctionCallExpr<IRRValue>>(astNode)))
	{
		std::shared_ptr<IRFunction> func;
		std::string function_name;

		try
		{
			// first we attempt to get a pointer to the actual function based off of the function call
			// the way we do this is check every possible function type that could be encountered, and
			// look it up in the function table
			if (auto f = std::dynamic_pointer_cast<IRFunctionCallExpr<IRLValue>>(astNode))
			{
				function_name = f->get_function_name();
				func = function_table.get_function(function_name);
			}
			else if (auto f = std::dynamic_pointer_cast<IRFunctionCallExpr<IRRValue>>(astNode))
			{
				function_name = f->get_function_name();
				func = function_table.get_function(function_name);
			}
			else if (auto f = std::dynamic_pointer_cast<IRMemberFunctionCallExpr<IRLValue>>(astNode))
			{
				function_name = f->get_function_name();
				func = function_table.get_function(function_name);
			}
			else if (auto f = std::dynamic_pointer_cast<IRMemberFunctionCallExpr<IRRValue>>(astNode))
			{
				function_name = f->get_function_name();
				func = function_table.get_function(function_name);
			}
			// this should never be reached
			else
				throw CastorException("Encountered unreachable code in the IR validator.");
		}
		// if the function wasn't in the function table, make a note of it and move on
		catch (CastorException& e)
		{
			log("Found a function not in the function table when attempting to check for functions without verification conditions: " + function_name, LogType::WARN, 1);
		}

		// if it was in the function table, we enter this
		if (func)
		{
			// first we count the number of writes and frees clauses
			auto writes_clauses = has_writes_clause(func);
			auto frees_clauses = has_frees_clause(func);

			// then we get a readable name for the function
			auto func_name = func->get_name();
			std::vector<std::string> substrings;
	
			boost::split(substrings, func_name, boost::is_any_of("#"));
	
			auto real_name = substrings[0] + "(" + substrings[1] + ")";
	
			// if there's no writes clauses, we should warn the user
			if (writes_clauses == 0)
			{
				log("WARNING: Function " + real_name + " is being called, but has no writes clause.\n"
				"This could be an implicitly generated function.\n"
				"Expect any calling functions to be unverifiable!", LogType::WARN, 0);
			}

			// if there's no frees clauses
			if (frees_clauses == 0)
			{
#ifndef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
				// we insert one as long as dynamic memory allocation is disabled
				func->add_vc(std::make_shared<IRFrees>(std::vector<std::shared_ptr<IRExpression>>()));
				log("Adding empty frees VC to function " + real_name, LogType::INFO, 1);
#else
				// if dynamic memory allocation is enabled, we should warn the user about the missing frees clause
				log("WARNING: Function " + real_name + " is being called has no frees clause.\n"
				"This could be an implicitly generated function.\n"
				"Expect any calling functions to be unverifiable!", LogType::WARN, 0);
#endif
			}
		}
	}

	// here we check for writes and frees clauses for loop statements
	if (auto loop = std::dynamic_pointer_cast<IRLoopStmt>(astNode))
	{
		// first we count the number of writes and frees clauses
		auto writes_clauses = has_writes_clause(loop);
		auto frees_clauses = has_frees_clause(loop);

		// if there's no writes clauses, we should warn the user
		if (writes_clauses == 0)
		{
			log("WARNING: Loop statement found without writes clause.\n"
			"Loop invariants might be unverifiable!", LogType::WARN, 0);
		}

		// if there's no frees clauses
		if (frees_clauses == 0)
		{
#ifndef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
			// we insert one as long as dynamic memory allocation is disabled
			loop->add_vc(std::make_shared<IRFrees>(std::vector<std::shared_ptr<IRExpression>>()));
			log("Adding empty frees VC to loop", LogType::INFO, 1);
#else
			log("WARNING: Loop statement found without frees clause.\n"
			"Loop invariants might be unverifiable!", LogType::WARN, 0);
#endif
		}
	}

	// now we check functions to ensure no function has more than one writes/frees clause
	if (auto func = std::dynamic_pointer_cast<IRFunction>(astNode))
	{
		// get the readable name of the function
		auto func_name = func->get_name();
		std::vector<std::string> substrings;
	
		boost::split(substrings, func_name, boost::is_any_of("#"));
	
		auto real_name = substrings[0] + "(" + substrings[1] + ")";
	
		// and throw an exception if there's more than one writes or frees clause
		if (has_writes_clause(func) > 1)
			throw CastorException("Function " + real_name + " has more than one writes clause (expected: 1)");
		else if (has_frees_clause(func) > 1)
			throw CastorException("Function " + real_name + " has more than one frees clause (expected: 1)");
	}

	// and check loop statements to ensure there's not more than one writes/frees clause
	if (auto loop = std::dynamic_pointer_cast<IRLoopStmt>(astNode))
	{
		if (has_writes_clause(loop) > 1)
			throw CastorException("Loop statement found with more than one writes clause (expected: 1)");
		else if (has_frees_clause(loop) > 1)
			throw CastorException("Loop statement found with more than one frees clause (expected: 1)");
	}

	// we want to make sure that variable declarations with an initializer are being initialized to the same type as the variable
	// this can fail if there's a bug in EDG, SAGE, constant folding, or other places in ROSE
	// the user should not be able to trip this error manually
	if (auto decl = std::dynamic_pointer_cast<IRVariableDeclarationStmt>(astNode))
	{
		auto init = decl->get_initial_value();

		// if it has an initial value, and the initial value's type and the variable's type don't match (sans const qualifier)
		if (init && !(IRGenerator::are_same_type_without_const(decl->get_var()->get_type(), init->get_type()) ||
					(std::dynamic_pointer_cast<IRArrayType>(init->get_type())
					 && std::dynamic_pointer_cast<IRPointerType>(decl->get_var()->get_type()))))
			throw CastorException("Type mismatch: variable \"" + decl->get_var()->get_name() + "\" is declared type \"" +
					decl->get_var()->get_type()->pp() + "\" but is assigned type \"" + init->get_type()->pp() + "\". " +
					"This is probably not your fault."); // throw a rather verbose exception
	}

	// we want to ensure that lemmas are asserting a proposition (i.e. a boolean value)
	if (auto lemma = std::dynamic_pointer_cast<IRLemma>(astNode))
	{
		auto expr = lemma->get_vc();

		// make sure to strip const qualifiers!
		if (!IRGenerator::are_same_type_without_const(expr->get_type(), std::make_shared<IRBoolType>()))
			throw TypeMismatchException(expr->pp(), expr->get_type()->pp(), "bool");
	}

	// make sure that program variables aren't used inside lemmas or axioms (Why3 doesn't support this with our memory model)
	if (auto variable = std::dynamic_pointer_cast<IRVariable>(astNode);
			variable && inheritedValue.inside_lemma && variable->get_is_program_variable())
	{
		throw CastorException("Cannot use program variables inside lemma or axiom.");
	}

	// make sure that types for binary operations line up
	if (auto binop = std::dynamic_pointer_cast<IRBinaryOperation>(astNode))
	{
		auto lhs_t = binop->get_lhs()->get_type();
		auto rhs_t = binop->get_rhs()->get_type();

		// strip const qualifiers
		while (auto const_ = std::dynamic_pointer_cast<IRConstType>(lhs_t))
			lhs_t = const_->get_base_type();
		while (auto const_ = std::dynamic_pointer_cast<IRConstType>(rhs_t))
			rhs_t = const_->get_base_type();

		// comparison operators (except equality) and arithmetic operators
		// require an integral type on both sides
		if (std::dynamic_pointer_cast<IRGreaterEqualsOp>(binop) ||
			std::dynamic_pointer_cast<IRGreaterThanOp>(binop) ||
			std::dynamic_pointer_cast<IRLessEqualsOp>(binop) ||
			std::dynamic_pointer_cast<IRLessThanOp>(binop) ||
			std::dynamic_pointer_cast<IRDivideOp>(binop) ||
			std::dynamic_pointer_cast<IRModuloOp>(binop) ||
			std::dynamic_pointer_cast<IRMultiplyOp>(binop) ||
			std::dynamic_pointer_cast<IRBitRShiftOp>(binop) ||
			std::dynamic_pointer_cast<IRBitLShiftOp>(binop) ||
			std::dynamic_pointer_cast<IRBitOrOp>(binop) ||
			std::dynamic_pointer_cast<IRBitXorOp>(binop) ||
			std::dynamic_pointer_cast<IRBitAndOp>(binop) ||
			std::dynamic_pointer_cast<IRSubtractionOp>(binop) ||
			std::dynamic_pointer_cast<IRAdditionOp>(binop))
		{
			if (!(std::dynamic_pointer_cast<IRIntegralType>(lhs_t) ||
					std::dynamic_pointer_cast<IRArrayType>(lhs_t) ||
					std::dynamic_pointer_cast<IRNonRealType>(lhs_t)))
				throw TypeMismatchException(binop->pp(), lhs_t->pp(), "integral type");
			else if (!(std::dynamic_pointer_cast<IRIntegralType>(rhs_t) ||
					std::dynamic_pointer_cast<IRArrayType>(rhs_t) ||
					std::dynamic_pointer_cast<IRNonRealType>(rhs_t)))
				throw TypeMismatchException(binop->pp(), rhs_t->pp(), "integral type");
		}

		// boolean operators require a boolean type on both sides
		if (std::dynamic_pointer_cast<IRAndOp>(binop) ||
			std::dynamic_pointer_cast<IROrOp>(binop) ||
			std::dynamic_pointer_cast<IRImpliesOp>(binop))
		{
			if (!(std::dynamic_pointer_cast<IRBoolType>(lhs_t) ||
					std::dynamic_pointer_cast<IRNonRealType>(lhs_t)))
				throw TypeMismatchException(binop->pp(), lhs_t->pp(), "bool");
			else if (!(std::dynamic_pointer_cast<IRBoolType>(rhs_t) ||
					std::dynamic_pointer_cast<IRNonRealType>(rhs_t)))
				throw TypeMismatchException(binop->pp(), rhs_t->pp(), "bool");
		}

		// TODO: Type checking for IREqualsOp and IRNotEqualsOp...
	}

	// we also wanna check that unary operators have the right type
	if (auto unop = std::dynamic_pointer_cast<IRUnaryOperation>(astNode))
	{
		auto type = unop->get_base_item()->get_type();

		// strip const qualifiers
		while (auto const_ = std::dynamic_pointer_cast<IRConstType>(type))
			type = const_->get_base_type();

		// negation and bit negation should have an integral type
		if ((std::dynamic_pointer_cast<IRNegationOp>(unop) ||
			std::dynamic_pointer_cast<IRBitNegationOp>(unop)) &&
			!(std::dynamic_pointer_cast<IRIntegralType>(type) ||
			  std::dynamic_pointer_cast<IRArrayType>(type) ||
			  std::dynamic_pointer_cast<IRNonRealType>(type)))
		{
			throw TypeMismatchException(unop->pp(), type->pp(), "integral type");
		}

		// boolean negation should have a boolean
		if (std::dynamic_pointer_cast<IRBoolNegationOp>(unop) &&
			!(std::dynamic_pointer_cast<IRBoolType>(type) ||
			  std::dynamic_pointer_cast<IRNonRealType>(type)))
		{
			throw TypeMismatchException(unop->pp(), type->pp(), "bool");
		}
	}

	if (!inheritedValue.inside_loop &&
			(std::dynamic_pointer_cast<IRBreak>(astNode) ||
			 std::dynamic_pointer_cast<IRContinue>(astNode)))
	{
		throw CastorException("'break' or 'continue' statement found outside of a legal construct.\n"
				"'break' and 'continue' statements are unsupported inside do-while loops.\n"
				"If this is not a do-while loop, please open a bug report.");
	}

	return astNode;
}

///
/// @brief Functor, so that this class can be called as a function
///
/// @param base The base IR node to start checking from
///
void ValidateIR::operator()(std::shared_ptr<IRNode> base)
{
	traverse(base, { false, false, false });
}

///
/// @brief Checks if a function or loop has a writes clause
///
/// @param structure A shared pointer to the function to check
/// @tparam T IRFunction or IRLoopStmt
/// @return How many writes clauses it has
///
template <typename T>
int ValidateIR::has_writes_clause(std::shared_ptr<T> structure)
{
	auto vcs = structure->get_vcs();

	int writes = 0;

	for (auto vc : vcs)
		writes += (std::dynamic_pointer_cast<IRWrites>(vc) ? 1 : 0);

	return writes;
}


///
/// @brief Checks if a function or lop has a writes clause
///
/// @param structure A shared pointer to the function to check
/// @tparam T IRFunction or IRLoopStmt
/// @return How many frees clauses it has
///
template <typename T>
int ValidateIR::has_frees_clause(std::shared_ptr<T> structure)
{
	auto vcs = structure->get_vcs();

	int frees = 0;

	for (auto vc : vcs)
		frees += (std::dynamic_pointer_cast<IRFrees>(vc) ? 1 : 0);

	return frees;
}
