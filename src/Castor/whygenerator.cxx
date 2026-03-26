// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "whygenerator.hxx"
#include "irgenerator.hxx"
#include "exception.hxx"
#include <iostream>
#include <algorithm>
#include <cassert>

///
/// @brief Downwards traversal.
///
/// @param astNode The IRNode to look at
/// @param inheritedValue Inherited values passed down
/// @return Inherited value to pass down
///
WhyGenIV WhyGenerator::evaluateInheritedAttribute(std::shared_ptr<IRNode> astNode, WhyGenIV inheritedValue)
{
	auto in_vc = inheritedValue.in_vc;
	auto in_func = inheritedValue.in_func;
	auto globals = inheritedValue.globals;
	auto in_class = inheritedValue.in_class;

	if (std::dynamic_pointer_cast<IRVerificationCondition>(astNode) || // note if we're in a verificaton condition
			std::dynamic_pointer_cast<IRAssert>(astNode))
		in_vc = true;

	if (std::dynamic_pointer_cast<IRFunction>(astNode)) // note if we're in a function
		in_func = true;

	if (std::dynamic_pointer_cast<IRClass>(astNode)) // note if we're in a class
		in_class = true;

	// add a variable to the list of globals if...
	if (auto var = std::dynamic_pointer_cast<IRVariableDeclarationStmt>(astNode);
			var && !IRGenerator::are_same_type_without_const(var->get_var()->get_type(),
				std::make_shared<IRNonRealType>()) && // it's not an uninstantiated template
			((!in_func && !in_class) || var->get_is_static())) // and it's either an IR global or IR static variable
	{
		if (std::dynamic_pointer_cast<IRConstType>(var->get_var()->get_type()))
			globals->push_back(std::make_pair(var->get_var(), var->get_initial_value()));
		else
			globals->push_back(std::make_pair(var->get_var(), nullptr));
	}

	return { in_vc, in_func, globals, in_class };
}

///
/// @brief Upwards traversal.
///
/// @param astNode The IRNode to look at
/// @param inheritedValue The inherited values
/// @param list The synthesized attributes so far
/// @return The new synthesized attribute
///
std::shared_ptr<WhyNode> WhyGenerator::evaluateSynthesizedAttribute(std::shared_ptr<IRNode> astNode, WhyGenIV inheritedValue, SynthesizedAttributesList list)
{
	if (auto ast = std::dynamic_pointer_cast<IRVariable>(astNode))
	{
		if (ast->has_constexpr_value()) // if we have a constexpr value, parse the constexpr value and return that
			return (*this)(ast->get_constexpr_value());
		else // otherwise we create a WhyVariable
			return std::make_shared<WhyVariable>(getWhyTypeFromIRType(ast->get_type()), WhyName(ast->get_name()));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRVariableDeclarationStmt>(astNode))
	{
		if (!std::dynamic_pointer_cast<WhyVariable>(list[0]) || ast->get_is_static()) // this can trigger if we parsed a constexpr value
											      // or if it's a static variable
			return std::make_shared<WhyEmptyStmt>();

		// create a variable declaration, optionally giving an initializer
		if (list[1])
			return std::make_shared<WhyVariableDecl>(safety_cast<WhyVariable>(list[0]), safety_cast<WhyExpression>(list[1]));
		else
			return std::make_shared<WhyVariableDecl>(safety_cast<WhyVariable>(list[0]), nullptr);
	}
	else if (auto ast = std::dynamic_pointer_cast<IRVariableList>(astNode))
	{
		std::vector<std::shared_ptr<WhyVariable>> vars(list.size());
		std::transform(list.begin(), list.end(), vars.begin(), [](std::shared_ptr<WhyNode> node) { return std::dynamic_pointer_cast<WhyVariable>(node); }); // downcast the elements in list to variables

		auto ret = std::make_shared<__WhyVariableList>();
		ret->vars = vars;
		return ret;
	}
	else if (auto ast = std::dynamic_pointer_cast<IRVariableReference>(astNode))
	{
		if (std::dynamic_pointer_cast<WhyRValue>(list[0])) // this is triggered if we parsed a constexpr value
			return list[0];
		else
			return std::make_shared<WhyVariableReference>(safety_cast<WhyVariable>(list[0]));
	}
	// This is observed to match the C++17 standard draft, section 8.3.1.1 [expr.unary.op]
	else if (auto ast = std::dynamic_pointer_cast<IRPointerDereference>(astNode))
	{
		return std::make_shared<WhyPointerDereference>(getWhyTypeFromIRType(ast->get_type()), makeRValue(safety_cast<WhyExpression>(list[0])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRFieldReference>(astNode))
	{
		if (auto base_object = std::dynamic_pointer_cast<WhyLValue>(list[0]))
		{
			auto field_name = ast->get_field_name(); // first we get the field name
			auto offset_table = safety_cast<WhyClassType>(base_object->get_type())->get_offset_table(); // then we grab the offset table

			// and generate a field reference, making sure to grab the offset from the offset table
			return std::make_shared<WhyFieldReference>(
					getWhyTypeFromIRType(ast->get_type()), base_object, offset_table[field_name].first);
		}
		else
		{
			auto r_base_object = safety_cast<WhyRValue>(list[0]);
			auto field_name = ast->get_field_name(); // first we get the field name
			auto offset_table = safety_cast<WhyClassType>(r_base_object->get_type())->get_offset_table(); // then we grab the offset table

			// and generate a field reference, making sure to grab the offset from the offset table
			return std::make_shared<WhyFieldReferenceRValue>(
					getWhyTypeFromIRType(ast->get_type()), r_base_object, offset_table[field_name].first);
		}
	}
	// We do not support variable declarations in the condition, i.e. "while (T t = x)"
	// Aside from that, this is observed to match the C++17 standard draft, section 9.5.1, 9.5.3 [stmt.while, stmt.for]
	else if (auto ast = std::dynamic_pointer_cast<IRLoopStmt>(astNode))
	{
		auto comp = makeRValue(safety_cast<WhyExpression>(list[0]));  // comparison
		auto body = safety_cast<WhyStatement>(list[1]);               // body

		std::vector<std::shared_ptr<WhyVerificationCondition>> vcs;
		for (int i = 2; i < list.size(); i++) // verification conditions
			vcs.push_back(safety_cast<WhyVerificationCondition>(list[i]));

		return std::make_shared<WhyLoop>(comp, body, vcs, get_variables(ast->get_scope()));
	}
	// We do not support variable declarations in the condition, i.e. "if (T t = x)"
	// We do not support "if constexpr"
	// Aside from that, this is observed to match the C++17 standard draft, section 9.4.1 [stmt.if]
	else if (auto ast = std::dynamic_pointer_cast<IRIfStatement>(astNode))
	{
		auto comp = makeRValue(safety_cast<WhyExpression>(list[0])); // comparison
		auto then = safety_cast<WhyStatement>(list[1]);              // if body
		auto els = std::dynamic_pointer_cast<WhyStatement>(list[2]); // else body

		return std::make_shared<WhyIf>(comp, then, els);
	}
	else if (auto ast = std::dynamic_pointer_cast<IRBinaryOperation>(astNode))
	{
		// special handler for binary operations
		return handleBinaryOperation(ast, inheritedValue, list);
	}
	else if (auto ast = std::dynamic_pointer_cast<IRAssignmentOperation>(astNode))
	{
		// special handler for assignment operations
		return handleAssignmentOperation(ast, inheritedValue, list);
	}
	else if (auto ast = std::dynamic_pointer_cast<IRUnaryOperation>(astNode))
	{
		// special handler for unary operations
		return handleUnaryOperation(ast, list);
	}
	else if (auto ast = std::dynamic_pointer_cast<IRLiteral>(astNode))
	{
		// special handler for literals
		return handleLiteral(ast);
	}
	// This is observed to match the C++17 standard draft, section 7.11, 5.13.7 [conv.ptr, lex.nullptr]
	else if (auto ast = std::dynamic_pointer_cast<IRNullptr>(astNode))
	{
		return std::make_shared<WhyNullptr>();
	}
	// This is observed to match the C++17 standard draft, section 8.19 [expr.comma]
	else if (auto ast = std::dynamic_pointer_cast<IRCommaOp<IRLValue>>(astNode))
	{
		return std::make_shared<WhyCommaOp<WhyLValue>>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])), safety_cast<WhyLValue>(list[1]));
	}
	// This is observed to match the C++17 standard draft, section 8.19 [expr.comma]
	else if (auto ast = std::dynamic_pointer_cast<IRCommaOp<IRRValue>>(astNode))
	{
		return std::make_shared<WhyCommaOp<WhyRValue>>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])), makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 9.2 [stmt.expr]
	else if (auto ast = std::dynamic_pointer_cast<IRExpressionStmt>(astNode))
	{
		return std::make_shared<WhyDiscardResultStmt>(safety_cast<WhyExpression>(list[0]));
	}
	// This is observed to match the C++17 standard draft, section 9.3 [stmt.block]
	else if (auto ast = std::dynamic_pointer_cast<IRStatementBlock>(astNode))
	{
		// for statement blocks, we need to translate a vector of statements into a single statement
		// containing a chain of continuations
		
		// if there are no statements, just return an empty statement
		if (list.size() == 0)
			return std::make_shared<WhyEmptyStmt>();
		
		// the head is the first statement
		auto head = safety_cast<WhyStatement>(list[0]);

		// then we attach all the continuations
		for (int i = 0; i < list.size() - 1; i++)
		{
			auto first_item = safety_cast<WhyStatement>(list[i]);
			auto second_item = safety_cast<WhyStatement>(list[i + 1]);

			first_item->set_continuation(second_item);
		}

		// return the head as a statement collection
		return std::make_shared<WhyStatementCollection>(head);
	}
	// This is observed to match the C++17 standard draft, section 11.4 [dcl.fct.def]
	// Note: we do not support the following:
	// - the "__func__" variable (see section 11.4.1.8 [dcl.fct.def.general])
	else if (auto ast = std::dynamic_pointer_cast<IRFunction>(astNode))
	{
		auto vars = std::dynamic_pointer_cast<__WhyVariableList>(list[0])->vars; // grab the vector of parameters

		if (vars.size() > 0 && vars[0]->get_name() == "this") // "this" needs to go at the end because it's evaluated first at the callsite
		{
			auto this_ = vars[0];
			vars.erase(vars.begin());
			vars.push_back(this_);
		}

		auto body = std::dynamic_pointer_cast<WhyStatement>(list[2]); // the body
		auto name = ast->get_name(); // and the name

		std::vector<std::shared_ptr<WhyVerificationCondition>> vcs;
		for (int i = 3; i < list.size(); i++) // grab all the verification conditions
			vcs.push_back(safety_cast<WhyVerificationCondition>(list[i]));

		std::vector<std::shared_ptr<WhyVariable>> globals;
		for (auto v : *inheritedValue.globals)
		{
			auto possibly_global = (*this)(v.first); // parse all the globals
			auto possibly_literal = (*this)(v.second); // parse the literal init value
			if (auto var = std::dynamic_pointer_cast<WhyVariable>(possibly_global)) // though it could be a constexpr
												// in this case, ignore it
			{
				if (v.second == nullptr) // if no const-init value, just push the variable
				{
					globals.push_back(var);
				}
				else if (auto literal = std::dynamic_pointer_cast<IRLiteral>(v.second)) // check if the frontend was able to
												        // constant-fold the initial value.
												        // if not, we need to throw an exception.
				{
					var->add_literal_init(safety_cast<WhyRValue>(possibly_literal));
					globals.push_back(var);
				}
				else
				{
					throw CastorException("Cannot discern initial value of const global variable " + v.first->get_name());
				}
			}
		}

		if (ast->get_is_template()) // if it's a template (not a template instantiation), we just ignore it
			return std::make_shared<WhyEmptyNode>();
		else // otherwise we'll create a function
			return std::make_shared<WhyFunction>(getWhyTypeFromIRType(ast->get_return_type()), name, vars, globals, vcs, body, ast->get_is_ref());
	}
	else if (auto ast = std::dynamic_pointer_cast<IRClass>(astNode))
	{
		// classes are weird. we don't actually need the concept of a class in Why3, so classes go away
		// we only need the member functions
		// we don't need any notion of member variables, since those are translated to offsets in memory elsewhere

		std::vector<std::shared_ptr<WhyFunction>> funcs;

		for (auto l : list)
			if (auto func = std::dynamic_pointer_cast<WhyFunction>(l))
				funcs.push_back(func); // grab the functions

		auto ret = std::make_shared<__WhyFunctions>(); // and return a wrapper class
		ret->funcs = funcs;
		return ret;
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRAssume>(astNode))
	{
		return std::make_shared<WhyAssume>(makeRValue(safety_cast<WhyExpression>(list[0])));
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRAssert>(astNode))
	{
		return std::make_shared<WhyAssert>(makeRValue(safety_cast<WhyExpression>(list[0])), ast->get_debug_str());
	}
	// This is NOT observed to match the C++17 standard draft, section 9.6.3 [stmt.return]
	// Note: Due to a CodeThorn bug, we do not abide by 9.6.3.3
	else if (auto ast = std::dynamic_pointer_cast<IRReturnStmt>(astNode))
	{
		return std::make_shared<WhyReturn>(safety_cast<WhyExpression>(list[0]));
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRAxiom>(astNode))
	{
		return std::make_shared<WhyAxiom>(ast->get_name(), safety_cast<WhyRValue>(list[0]));
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRLemma>(astNode))
	{
		return std::make_shared<WhyLemma>(ast->get_name(), safety_cast<WhyRValue>(list[0]));
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IREnsures>(astNode))
	{
		return std::make_shared<WhyEnsures>(makeRValue(safety_cast<WhyExpression>(list[0])), ast->get_debug_str());
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRRequires>(astNode))
	{
		return std::make_shared<WhyRequires>(makeRValue(safety_cast<WhyExpression>(list[0])), ast->get_debug_str());
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRLoopInvariant>(astNode))
	{
		return std::make_shared<WhyInvariant>(makeRValue(safety_cast<WhyExpression>(list[0])), ast->get_debug_str());
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRLoopVariant>(astNode))
	{
		return std::make_shared<WhyVariant>(makeRValue(safety_cast<WhyExpression>(list[0])), ast->get_debug_str());
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRResult<IRRValue>>(astNode))
	{
		return std::make_shared<WhyResult<WhyRValue>>(getWhyTypeFromIRType(ast->get_type()));
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRResult<IRLValue>>(astNode))
	{
		return std::make_shared<WhyResult<WhyLValue>>(getWhyTypeFromIRType(ast->get_type()));
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRWrites>(astNode))
	{
		// for writes clauses, we grab all the variables and make sure they're all LValues
		std::vector<std::shared_ptr<WhyLValue>> vars;
		for (auto l : list)
			vars.push_back(safety_cast<WhyLValue>(l));
		return std::make_shared<WhyWrites>(vars, ast->get_debug_str());
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRFrees>(astNode))
	{
		// for frees clauses, we do the same as writes
		std::vector<std::shared_ptr<WhyLValue>> vars;
		for (auto l : list)
			vars.push_back(safety_cast<WhyLValue>(l));
		return std::make_shared<WhyFrees>(vars, ast->get_debug_str());
	}
	// This is observed to support the following conversions from the C++17 standard draft:
	// - 7.6 [conv.prom] (Note: this is handled by ROSE)
	// - 7.8 [conv.integral]
	// - 7.14 [conv.bool]
	// - 7.15 [conv.rank] (Note: this is handled by ROSE)
	else if (auto ast = std::dynamic_pointer_cast<IRCast<IRRValue>>(astNode))
	{
		return std::make_shared<WhyCast<WhyRValue>>(getWhyTypeFromIRType(ast->get_type()), makeRValue(safety_cast<WhyExpression>(list[0])));
	}
	// This is observed to support the following conversions from the C++17 standard draft:
	// - 7.6 [conv.prom] (Note: this is handled by ROSE)
	// - 7.8 [conv.integral]
	// - 7.14 [conv.bool]
	// - 7.15 [conv.rank] (Note: this is handled by ROSE)
	else if (auto ast = std::dynamic_pointer_cast<IRCast<IRLValue>>(astNode))
	{
		return std::make_shared<WhyCast<WhyLValue>>(getWhyTypeFromIRType(ast->get_type()), safety_cast<WhyLValue>(list[0]));
	}
	// We do not support address-of-function and pointer-to-function
	// Aside from that, this is observed to match the C++17 standard draft, section 8.3.1.2-3 [expr.unary.op]
	else if (auto ast = std::dynamic_pointer_cast<IRAddressOf>(astNode))
	{
		return std::make_shared<WhyAddressOfExpr>(getWhyTypeFromIRType(ast->get_type()), safety_cast<WhyLValue>(list[0]));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRNull>(astNode))
	{
		return std::make_shared<WhyEmptyExpr>();
	}
	else if (auto ast = std::dynamic_pointer_cast<IRFunctionRefExpr>(astNode))
	{
		auto ret = std::make_shared<__WhyFunctionRef>(); // function references go away, so this is just a wrapper class
		ret->function_name = ast->get_name();
		return ret;
	}
	// This is observed to match the C++17 standard draft, section 8.2.2 [expr.call], 12.2.3 [class.static]
	// Note: va_arg is not supported (see 8.2.2.9)
	else if (auto ast = std::dynamic_pointer_cast<IRFunctionCallExpr<IRRValue>>(astNode))
	{
		// special handler for function calls
		return handleFunctionCall(ast, list);
	}
	// This is observed to match the C++17 standard draft, section 8.2.2 [expr.call], 12.2.2 [class.mfct.non-static]
	// Note: ref-qualifiers are not supported (see 12.2.2.5, 16.3.1)
	// Note: va_arg is not supported (see 8.2.2.9)
	else if (auto ast = std::dynamic_pointer_cast<IRMemberFunctionCallExpr<IRRValue>>(astNode))
	{
		// member function calls are a bit harder
		// we turn functions of the form a.foo(b, c, ...) into foo(&a, b, c, ...)

		auto name = safety_cast<__WhyFunctionRef>(list[0])->function_name;
		std::vector<std::shared_ptr<WhyExpression>> params;

		std::shared_ptr<WhyLValue> this_;
		// if "a" is an lvalue, we can use it directly
		if (auto lv = std::dynamic_pointer_cast<WhyLValue>(list[1]))
			this_ = lv;
		// otherwise, it undergoes temporary object materialization
		else
			this_ = std::make_shared<WhyMaterialize>(safety_cast<WhyRValue>(list[1]));

		// then we can add all the other parameters
		for (int i = 2; i < list.size(); i++)
			params.push_back(safety_cast<WhyExpression>(list[i]));

		// take a's adddress and make it the last parameter
		params.push_back(std::make_shared<WhyAddressOfExpr>(std::make_shared<WhyPointerType>(this_->get_type()), this_));

		// then we make a normal function call
		return std::make_shared<WhyFunctionCall<WhyRValue>>(getWhyTypeFromIRType(ast->get_type()), name, params);
	}
	// This is observed to match the C++17 standard draft, section 8.2.2 [expr.call], 12.2.3 [class.static]
	// Note: va_arg is not supported (see 8.2.2.9)
	else if (auto ast = std::dynamic_pointer_cast<IRFunctionCallExpr<IRLValue>>(astNode))
	{
		// special handler for function calls
		return handleFunctionCall(ast, list);
	}
	// This is observed to match the C++17 standard draft, section 8.2.2 [expr.call], 12.2.2 [class.mfct.non-static]
	// Note: ref-qualifiers are not supported (see 12.2.2.5, 16.3.1)
	// Note: va_arg is not supported (see 8.2.2.9)
	else if (auto ast = std::dynamic_pointer_cast<IRMemberFunctionCallExpr<IRLValue>>(astNode))
	{
		// this is the same as earlier, except for LValue member function calls

		auto name = safety_cast<__WhyFunctionRef>(list[0])->function_name;
		std::vector<std::shared_ptr<WhyExpression>> params;

		std::shared_ptr<WhyLValue> this_;
		// if "a" is an lvalue, we can use it directly
		if (auto lv = std::dynamic_pointer_cast<WhyLValue>(list[1]))
			this_ = lv;
		// otherwise, it undergoes temporary object materialization
		else
			this_ = std::make_shared<WhyMaterialize>(safety_cast<WhyRValue>(list[1]));

		for (int i = 2; i < list.size(); i++)
			params.push_back(safety_cast<WhyExpression>(list[i]));

		params.push_back(std::make_shared<WhyAddressOfExpr>(std::make_shared<WhyPointerType>(this_->get_type()), this_));

		return std::make_shared<WhyFunctionCall<WhyLValue>>(getWhyTypeFromIRType(ast->get_type()), name, params);
	}
	else if (auto ast = std::dynamic_pointer_cast<IRProgram>(astNode))
	{
		std::vector<std::shared_ptr<WhyFunction>> funcs;
		std::vector<std::shared_ptr<WhyGlobalVariableDecl>> vars;
		std::vector<std::shared_ptr<WhyLemma>> lemmas;

		for (auto l : list) // iterate through everything that can go into a program
		{
			if (auto f = std::dynamic_pointer_cast<WhyFunction>(l)) // grab individual functions
			{
				funcs.push_back(f);
			}
			else if (auto f = std::dynamic_pointer_cast<__WhyFunctions>(l)) // decompose function wrapper objects
			{
				funcs.insert(funcs.end(), f->funcs.begin(), f->funcs.end());
			}
			else if (auto f = std::dynamic_pointer_cast<WhyLemma>(l))
			{
				lemmas.push_back(f);
			}
			// instead of reading variables from the program declaration,
			// instead we read them from the globals inherited value

			//else if (auto f = std::dynamic_pointer_cast<WhyVariableDecl>(l)) // grab global variables
			//{
			//	vars.push_back(std::make_shared<WhyGlobalVariableDecl>(f->get_variable()));
			//}
		}

		// iterate through the list of global variables
		for (auto g : *inheritedValue.globals)
		{
			// convert to a WhyVariable, create WhyGlobalVariableDecl, and add to our vars
			vars.push_back(std::make_shared<WhyGlobalVariableDecl>(
						safety_cast<WhyVariable>((*this)(g.first))));
		}

		return std::make_shared<WhyProgram>(vars, funcs, lemmas);
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRForall>(astNode))
	{
		auto expr = makeRValue(safety_cast<WhyExpression>(list[0])); // grab the base expression
		std::vector<std::shared_ptr<WhyVariable>> vars;
		for (int i = 1; i < list.size(); i++) // then all the variables
			vars.push_back(safety_cast<WhyVariable>(list[i]));

		return std::make_shared<WhyForall>(vars, expr);
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRExists>(astNode))
	{
		// same as IRForall
		auto expr = makeRValue(safety_cast<WhyExpression>(list[0]));
		std::vector<std::shared_ptr<WhyVariable>> vars;
		for (int i = 1; i < list.size(); i++)
			vars.push_back(safety_cast<WhyVariable>(list[i]));

		return std::make_shared<WhyExists>(vars, expr);
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRArrayIndex>(astNode))
	{
		auto base = makeRValue(safety_cast<WhyExpression>(list[0]));
		auto index = makeRValue(safety_cast<WhyExpression>(list[1]));

		return std::make_shared<WhyArrayIndex>(getWhyTypeFromIRType(ast->get_type()), base, index);
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRArrayRangeIndex>(astNode))
	{
		auto base = makeRValue(safety_cast<WhyExpression>(list[0]));
		auto start = makeRValue(safety_cast<WhyExpression>(list[1]));
		auto end = makeRValue(safety_cast<WhyExpression>(list[2]));

		return std::make_shared<WhyArrayRange>(getWhyTypeFromIRType(ast->get_type()), base, start, end);
	}
	// Not a C++ node
	else if (auto ast = std::dynamic_pointer_cast<IRLabelReference>(astNode))
	{
		return std::make_shared<WhyLabel>(ast->get_name());
	}
#ifdef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
	else if (auto ast = std::dynamic_pointer_cast<IRNewExpr>(astNode))
	{
		return std::make_shared<WhyNew>(getWhyTypeFromIRType(ast->get_type()), safety_cast<WhyExpression>(list[0]));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRDeleteExpr>(astNode))
	{
		return std::make_shared<WhyDelete>(safety_cast<WhyLValue>(list[0]));
	}
#endif
	else if (auto ast = std::dynamic_pointer_cast<IRReference>(astNode))
	{
		return std::make_shared<WhyReference>(getWhyTypeFromIRType(ast->get_type()), safety_cast<WhyLValue>(list[0]));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRLabel>(astNode))
	{
		return std::make_shared<WhyLabelStmt>(safety_cast<WhyLabel>(list[0]));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRMaterialize>(astNode))
	{
		return std::make_shared<WhyMaterialize>(safety_cast<WhyRValue>(list[0]));
	}
	// This is observed to match the C++17 standard draft, section 8.3.3 [expr.sizeof]
	else if (auto ast = std::dynamic_pointer_cast<IRSizeOf>(astNode))
	{
		return get_sizeof(getWhyTypeFromIRType(ast->get_contained_type()));
	}
	// This is observed to match the C++17 standard draft, section 8.16 [expr.cond]
	else if (auto ast = std::dynamic_pointer_cast<IRTernary<IRRValue>>(astNode))
	{
		auto condition = makeRValue(safety_cast<WhyExpression>(list[0]));
		auto then = makeRValue(safety_cast<WhyExpression>(list[1]));
		auto els = makeRValue(safety_cast<WhyExpression>(list[2]));

		return std::make_shared<WhyTernary<WhyRValue>>(getWhyTypeFromIRType(ast->get_type()), condition, then, els);
	}
	// This is observed to match the C++17 standard draft, section 8.16 [expr.cond]
	else if (auto ast = std::dynamic_pointer_cast<IRTernary<IRLValue>>(astNode))
	{
		auto condition = makeRValue(safety_cast<WhyExpression>(list[0]));
		auto then = safety_cast<WhyLValue>(list[1]);
		auto els = safety_cast<WhyLValue>(list[2]);

		return std::make_shared<WhyTernary<WhyLValue>>(getWhyTypeFromIRType(ast->get_type()), condition, then, els);
	}
	// This is observed to match the C++17 standard draft, section 9.6.1 [stmt.break]
	else if (auto ast = std::dynamic_pointer_cast<IRBreak>(astNode))
	{
		return std::make_shared<WhyBreak>();
	}
	// This is observed to match the C++17 standard draft, section 9.6.2 [stmt.cont]
	else if (auto ast = std::dynamic_pointer_cast<IRContinue>(astNode))
	{
		return std::make_shared<WhyContinue>();
	}
	// This is NOT observed to match the C++17 standard draft, section 9.4.1 [stmt.switch]
	// Note: Case labels implicitly introduce a scope, preventing declarations from appearing
	// Note: See 9.4.2.6
	else if (auto ast = std::dynamic_pointer_cast<IRCase>(astNode))
	{
		// this could be a default case, in which case there is no expression
		std::shared_ptr<WhyExpression> expr;
		if (expr = std::dynamic_pointer_cast<WhyExpression>(list[0]))
			list.erase(list.begin());

		// here we do a similar process to IRStatementBlock

		// if there are no statements, just return an empty statement
		if (list.size() == 0)
			return std::make_shared<WhyEmptyStmt>();
		
		// the head is the first statement
		auto head = safety_cast<WhyStatement>(list[0]);

		// then we attach all the continuations
		for (int i = 0; i < list.size() - 1; i++)
		{
			auto first_item = safety_cast<WhyStatement>(list[i]);
			auto second_item = safety_cast<WhyStatement>(list[i + 1]);

			first_item->set_continuation(second_item);
		}

		// we only create an rvalue if we do have an expression
		// i.e. if it's NOT "default"
		auto case_ = std::make_shared<WhyCase>(expr ? makeRValue(expr) : nullptr);

		case_->set_continuation(head);

		return case_;
	}
	// This is NOT observed to match the C++17 standard draft, section 9.4.1 [stmt.switch]
	// Note: Case labels implicitly introduce a scope, preventing declarations from appearing
	// Note: See 9.4.2.6
	else if (auto ast = std::dynamic_pointer_cast<IRSwitchStmt>(astNode))
	{
		auto expr = safety_cast<WhyExpression>(list[0]); // the value to switch over
		std::vector<std::shared_ptr<WhyCase>> cases;

		for (int i = 1; i < list.size(); i++)
			cases.push_back(safety_cast<WhyCase>(list[i])); // collect our cases

		return std::make_shared<WhySwitch>(makeRValue(expr), cases);
	}
	else if (std::dynamic_pointer_cast<IREmptyStatement>(astNode))
	{
		return std::make_shared<WhyEmptyStmt>();
	}
	else
	{
		throw UnknownIRNodeException(astNode->pp());
	}
}

///
/// @brief Functor, so that instances of this class may be used as a function
///
/// @param project The base IRNode
/// @return The synthesized WhyNode
///
std::shared_ptr<WhyNode> WhyGenerator::operator()(std::shared_ptr<IRNode> project)
{
	return this->traverse(project, { false, false, std::make_shared<std::vector<std::pair<
			std::shared_ptr<IRVariable>, std::shared_ptr<IRExpression>>>>(), false });
}
