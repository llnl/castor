// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <climits>
#include "settings.hxx"
#include "fill_types.hxx"
#include "function_table.hxx"
#include "exception.hxx"
#include "messaging.hxx"

extern FunctionTable function_table; ///< The function table
extern Settings settings;

///
/// @brief Functor so that instances of this class may be used as a function
///
/// @param project The IR AST to traverse
/// @result The resulting IR AST
///
std::shared_ptr<IRNode> FillTypes::operator()(std::shared_ptr<IRNode> project)
{
	return traverse(project, { false, safety_cast<IRProgram>(project)->get_scope(),
			std::vector<std::shared_ptr<IRVariable>>(),
			std::vector<std::shared_ptr<IRClass>>(),
			std::make_shared<IRUnknownType>(), false, false } );
}

///
/// @brief Downwards traversal
///
/// @param astNode Input IRNode
/// @param inheritedValue The inherited value
/// @return The inheritedValue we should pass down
///
FillTypesIV FillTypes::evaluateInheritedAttribute(std::shared_ptr<IRNode> astNode, FillTypesIV inheritedValue)
{
	auto scope = inheritedValue.current_scope;
	auto classes = inheritedValue.classes;
	auto in_checked = inheritedValue.in_checked;
	auto ref_func = inheritedValue.ref_func;

	if (auto ast = std::dynamic_pointer_cast<IRScopedStatement>(astNode))
	{
		scope = ast->get_scope();
	}

	if (auto ast = std::dynamic_pointer_cast<IRProgram>(astNode))
	{
		classes = ast->get_classes();
	}

	if (auto ast = std::dynamic_pointer_cast<IRFunctionCallExpr<IRRValue>>(astNode))
	{
		if (inheritedValue.in_vc && ast->get_function_name() == "checked")
			in_checked = true;
	}

	if (auto ast = std::dynamic_pointer_cast<IRFunction>(astNode))
	{
		std::vector<std::shared_ptr<IRVariable>> template_params = ast->get_template_vars();

		return { false, scope, template_params, classes, ast->get_return_type(), in_checked, ast->get_is_ref() };
	}
	else if (std::dynamic_pointer_cast<IRVerificationCondition>(astNode) || std::dynamic_pointer_cast<IRAssert>(astNode))
	{
		return { true, scope, inheritedValue.non_scoped_variables, classes, inheritedValue.function_return_type, in_checked, ref_func };
	}
	else if (auto ast = std::dynamic_pointer_cast<IRQuantifier>(astNode))
	{
		auto vec = inheritedValue.non_scoped_variables;
		auto vec2 = ast->get_vars();
		vec.insert(vec.end(), vec2.begin(), vec2.end());
		return { inheritedValue.in_vc, scope, vec, classes, inheritedValue.function_return_type, in_checked, ref_func };
	}
	else
	{
		return { inheritedValue.in_vc, scope, inheritedValue.non_scoped_variables, classes, inheritedValue.function_return_type, in_checked, ref_func };
	}
}

void FillTypes::assign_class_definition(std::shared_ptr<IRType> type, std::vector<std::shared_ptr<IRClass>> classes)
{
	while (std::dynamic_pointer_cast<IRPointerType>(type) ||
			std::dynamic_pointer_cast<IRArrayType>(type) ||
			std::dynamic_pointer_cast<IRConstType>(type))
	{
		if (auto ptrtype = std::dynamic_pointer_cast<IRPointerType>(type))
			type = ptrtype->get_base_type();
		else if (auto arrtype = std::dynamic_pointer_cast<IRArrayType>(type))
			type = arrtype->get_base_type();
		else if (auto consttype = std::dynamic_pointer_cast<IRConstType>(type))
			type = consttype->get_base_type();
	}

	if (auto clas = std::dynamic_pointer_cast<IRClassType>(type))
	{
		auto class_name = clas->get_class_name();

		for (auto c : classes)
		{
			if (c->get_name() == class_name)
			{
				clas->set_class_definition(c);
				break;
			}
		}
	}
}

///
/// @brief Upwards traversal
///
/// @param astNode Input IRNode
/// @param inheritedValue The inherited value
/// @param list List of synthesized attributes so far
/// @return The new synthesized attribute
///
std::shared_ptr<IRNode> FillTypes::evaluateSynthesizedAttribute(std::shared_ptr<IRNode> astNode, FillTypesIV inheritedValue, SynthesizedAttributesList list)
{
	auto scope = inheritedValue.current_scope;
	auto vars = inheritedValue.non_scoped_variables;

	// check anything that could have a type, and assign class definitions to the type node
	if (auto var = std::dynamic_pointer_cast<IRVariable>(astNode))
		assign_class_definition(var->get_type(), inheritedValue.classes);
	else if (auto sof = std::dynamic_pointer_cast<IRSizeOf>(astNode))
		assign_class_definition(sof->get_contained_type(), inheritedValue.classes);
	else if (auto expr = std::dynamic_pointer_cast<IRExpression>(astNode))
		assign_class_definition(expr->get_type(), inheritedValue.classes);
	else if (auto func = std::dynamic_pointer_cast<IRFunction>(astNode))
		assign_class_definition(func->get_return_type(), inheritedValue.classes);

	// check if we should give some debug output here
	if (auto is_expr = std::dynamic_pointer_cast<IRExpression>(astNode);
			is_expr &&							// if it's an expression
			std::dynamic_pointer_cast<IRUnknownType>(is_expr->get_type()))	// and it's an unknown type
		log("Attempting to infer type of expression: " + astNode->pp(), LogType::INFO, 2); // log it to the debug output

	// detect the type of variables for which the current type is unknown
	// this happens most often in verification conditions, where the parser emits an IRVariable with unknown type
	if (auto var = std::dynamic_pointer_cast<IRVariable>(astNode);
		var && std::dynamic_pointer_cast<IRUnknownType>(var->get_type()))
	{
		auto name = var->get_name();
		auto frontend_name = var->get_frontend_name();
		std::shared_ptr<IRType> type = nullptr;
		std::shared_ptr<IRExpression> constexpr_value = nullptr;

		for (auto v : vars) // first we should check variables that aren't in a scope, like quantifier variables or template instantiations
		{
			if (v->get_name() == name)
			{
				type = v->get_type();
				name = v->get_name();
				frontend_name = v->get_frontend_name();
				constexpr_value = v->get_constexpr_value();
			}
		}

		if (type) return std::make_shared<IRVariable>(type, name, frontend_name, constexpr_value, false);

		if (auto var = scope->lookup(name)) // if it's not found in vars, we'll just check the scope
		{
			if (scope->get_class_variable(name))
				throw CastorException("Cannot directly reference \"" + name + "\" in the verification language. Try \"[this]->" + name + "\".");
			else
				return var;
		}
		else
			throw UnknownVariableException(name); // we've run out of options, no idea what this variable is
	}
	// this inserts an IRReference in the return statement of an lvalue function
	else if (auto ret = std::dynamic_pointer_cast<IRReturnStmt>(astNode);
		ret && inheritedValue.ref_func)
	{
		if (auto base_stmt = std::dynamic_pointer_cast<IRLValue>(ret->get_stmt())) // Check if this is an lvalue.
											   // If not, we're returning a const T&,
											   // meaning that we need to materialize a value.
		{
			auto new_ret = std::make_shared<IRReference>(base_stmt->get_type(), base_stmt);
			ret->set_stmt(new_ret);
		}
		else
		{
			auto rv = safety_cast<IRRValue>(ret->get_stmt());
			auto new_ret = std::make_shared<IRReference>(rv->get_type(),
					std::make_shared<IRMaterialize>(rv));
			ret->set_stmt(new_ret);
		}

		return astNode;
	}
	// earlier, we might have returned a new IRVariable object
	// we should consume that
	else if (auto var = std::dynamic_pointer_cast<IRVariableReference>(astNode);
		var && std::dynamic_pointer_cast<IRUnknownType>(var->get_type()))
	{
		var->set_var(safety_cast<IRVariable>(list[0]));
		return astNode;
	}
	// set the type of IRResult objects
	else if (auto var = std::dynamic_pointer_cast<IRResult<IRRValue>>(astNode);
		var && std::dynamic_pointer_cast<IRUnknownType>(var->get_type()))
	{
		var->set_type(inheritedValue.function_return_type);
		assign_class_definition(var->get_type(), inheritedValue.classes);
		return astNode;
	}
	// cont.
	else if (auto var = std::dynamic_pointer_cast<IRResult<IRLValue>>(astNode);
		var && std::dynamic_pointer_cast<IRUnknownType>(var->get_type()))
	{
		var->set_type(inheritedValue.function_return_type);
		assign_class_definition(var->get_type(), inheritedValue.classes);
		return astNode;
	}
	// let's fill in the types and values of function calls in verification conditions
	// i.e. builtin functions
	else if (auto var = std::dynamic_pointer_cast<IRFunctionCallExpr<IRRValue>>(astNode);
		var && inheritedValue.in_vc)
	{
		std::shared_ptr<IRLiteral> res = nullptr;

		if (var->get_function_name() == "max_int")           // evaluate the max_int function
		{
			res = parse_max_int(var->get_params());
		}
		else if (var->get_function_name() == "min_int")      // evaluate the min_int function
		{
			res = parse_min_int(var->get_params());
		}
		else if (var->get_function_name() == "is_integral")  // evaluate the is_integral function
		{
			if (var->get_params().size() != 1)
				throw IncorrectArgumentCountException(1, var->get_params().size(), "is_integral");
			else
				res = std::dynamic_pointer_cast<IRIntegralType>(var->get_params()[0]->get_type())
					? std::make_shared<IRBoolLiteral>(true) : std::make_shared<IRBoolLiteral>(false);
		}
		else if (var->get_function_name() == "is_class")     // evaluate the is_class function
		{
			if (var->get_params().size() != 1)
				throw IncorrectArgumentCountException(1, var->get_params().size(), "is_class");
			else
				res = std::dynamic_pointer_cast<IRClassType>(var->get_params()[0]->get_type())
					? std::make_shared<IRBoolLiteral>(true) : std::make_shared<IRBoolLiteral>(false);
		}
		else if (var->get_function_name() == "is_pointer")   // evaluate the is_pointer function
		{
			if (var->get_params().size() != 1)
				throw IncorrectArgumentCountException(1, var->get_params().size(), "is_pointer");
			else
				res = std::dynamic_pointer_cast<IRPointerType>(var->get_params()[0]->get_type())
					? std::make_shared<IRBoolLiteral>(true) : std::make_shared<IRBoolLiteral>(false);
		}

		if (res) // if we've evaluated the function, set the current IRFunctionCallExpr node to the identity function
			 // and set its argument accordingly
		{
			var->set_function(std::make_shared<IRFunctionRefExpr>("id"));
			var->set_params(std::vector<std::shared_ptr<IRExpression>>({ res }));
		}

		if (var->get_function_name() == "checked") // replace "checked" function with "id," we'll handle the rest later
		{
			var->set_function(std::make_shared<IRFunctionRefExpr>("id"));
		}

		if (std::dynamic_pointer_cast<IRUnknownType>(var->get_type())) // if the type is unknown, let's set the types
		{
			auto name = var->get_function_name();
			auto size = var->get_params().size();

			// check to ensure a correct number of arguments
			// these functions require exactly 1 argument
			if (size != 1 && (name == "old" || name == "id" || name == "max_int" || name == "min_int" || name == "sizeof" ||
					name == "is_uint8" || name == "is_sint8" || name == "is_uint16" || name == "is_sint16" ||
					name == "is_uint32" || name == "is_sint32" || name == "is_uint64" || name == "is_sint64" ||
					name == "unchanged" || name == "is_integral" || name == "is_class" || name == "is_pointer" ||
					name == "to_uint8" || name == "to_sint8" || name == "to_uint16" || name == "to_sint16" ||
					name == "to_uint32" || name == "to_sint32" || name == "to_uint64" || name == "to_sint64" ||
					name == "checked"))
				throw IncorrectArgumentCountException(1, size, name);
			// these functions require exactly 2 arguments
			else if (size != 2 && (name == "at" || name == "valid_array" || name == "alias_of" || name == "nth"))
				throw IncorrectArgumentCountException(2, size, name);
			// these functions require at least 1 argument
			else if (size < 1 && (name == "valid" || name == "freed"))
				throw IncorrectArgumentCountException(1, size, name);
			// these functions require at least 2 arguments
			else if (size < 2 && (name == "separated"))
				throw IncorrectArgumentCountException(2, size, name);


			if (name == "old" || name == "id" || name == "at")        // these functions retain the type of their first argument
				var->set_type(var->get_params()[0]->get_type());
			else if (name == "max_int" || name == "min_int" || name == "sizeof") // these functions should be unbounced integrals
				var->set_type(std::make_shared<IRUnboundedIntegralType>());
			else if (name == "is_uint8" || name == "is_sint8" || name == "is_uint16" || name == "is_sint16" ||
				name == "is_uint32" || name == "is_sint32" || name == "is_uint64" || name == "is_sint64" ||
				name == "valid" || name == "separated" || name == "valid_array" || name == "freed" ||
				name == "unchanged" || name == "is_integral" || name == "is_class" || name == "is_pointer" ||
				name == "alias_of" || name == "nth") // all of these return bools
				var->set_type(std::make_shared<IRBoolType>());
			// the rest are conversions, this should be self-explanatory
			else if (name == "to_sint8")
				var->set_type(std::make_shared<IRS8Type>());
			else if (name == "to_sint16")
				var->set_type(std::make_shared<IRS16Type>());
			else if (name == "to_sint32")
				var->set_type(std::make_shared<IRS32Type>());
			else if (name == "to_sint64")
				var->set_type(std::make_shared<IRS64Type>());
			else if (name == "to_uint8")
				var->set_type(std::make_shared<IRU8Type>());
			else if (name == "to_uint16")
				var->set_type(std::make_shared<IRU16Type>());
			else if (name == "to_uint32")
				var->set_type(std::make_shared<IRU32Type>());
			else if (name == "to_uint64")
				var->set_type(std::make_shared<IRU64Type>());
		}

		return astNode;
	}
	// now let's fill in the types of all other expression types (i.e. not rvalue function calls)
	else if (auto node = std::dynamic_pointer_cast<IRExpression>(astNode); node && std::dynamic_pointer_cast<IRUnknownType>(node->get_type()))
	{
		// a bunch of these are comparison operators, we'll set the return type to bool
		if (auto node = std::dynamic_pointer_cast<IRGreaterEqualsOp>(astNode))
		{
			node->set_type(std::make_shared<IRBoolType>());
		}
		else if (auto node = std::dynamic_pointer_cast<IRGreaterThanOp>(astNode))
		{
			node->set_type(std::make_shared<IRBoolType>());
		}
		else if (auto node = std::dynamic_pointer_cast<IRLessEqualsOp>(astNode))
		{
			node->set_type(std::make_shared<IRBoolType>());
		}
		else if (auto node = std::dynamic_pointer_cast<IRLessThanOp>(astNode))
		{
			node->set_type(std::make_shared<IRBoolType>());
		}
		else if (auto node = std::dynamic_pointer_cast<IRNotEqualsOp>(astNode))
		{
			node->set_type(std::make_shared<IRBoolType>());
		}
		else if (auto node = std::dynamic_pointer_cast<IREqualsOp>(astNode))
		{
			node->set_type(std::make_shared<IRBoolType>());
		}
		else if (auto node = std::dynamic_pointer_cast<IRAndOp>(astNode))
		{
			node->set_type(std::make_shared<IRBoolType>());
		}
		else if (auto node = std::dynamic_pointer_cast<IROrOp>(astNode))
		{
			node->set_type(std::make_shared<IRBoolType>());
		}
		// for these we need to call get_bitwise_type to calculate the type
		else if (auto node = std::dynamic_pointer_cast<IRBitAndOp>(astNode))
		{
			auto exp1 = safety_cast<IRExpression>(list[0]);
			auto exp2 = safety_cast<IRExpression>(list[1]);

			node->set_type(get_bitwise_type(make_integral_type(exp1->get_type()), make_integral_type(exp2->get_type())));
		}
		else if (auto node = std::dynamic_pointer_cast<IRBitOrOp>(astNode))
		{
			auto exp1 = safety_cast<IRExpression>(list[0]);
			auto exp2 = safety_cast<IRExpression>(list[1]);

			node->set_type(get_bitwise_type(make_integral_type(exp1->get_type()), make_integral_type(exp2->get_type())));
		}
		else if (auto node = std::dynamic_pointer_cast<IRBitXorOp>(astNode))
		{
			auto exp1 = safety_cast<IRExpression>(list[0]);
			auto exp2 = safety_cast<IRExpression>(list[1]);

			node->set_type(get_bitwise_type(make_integral_type(exp1->get_type()), make_integral_type(exp2->get_type())));
		}
		// for these we'll call get_bitshift_type to calculate the type
		else if (auto node = std::dynamic_pointer_cast<IRBitLShiftOp>(astNode))
		{
			auto exp1 = safety_cast<IRExpression>(list[0]);
			auto exp2 = safety_cast<IRExpression>(list[1]);

			node->set_type(get_bitshift_type(make_integral_type(exp1->get_type()), make_integral_type(exp2->get_type())));
		}
		else if (auto node = std::dynamic_pointer_cast<IRBitRShiftOp>(astNode))
		{
			auto exp1 = safety_cast<IRExpression>(list[0]);
			auto exp2 = safety_cast<IRExpression>(list[1]);

			node->set_type(get_bitshift_type(make_integral_type(exp1->get_type()), make_integral_type(exp2->get_type())));
		}
		// implies returns a boolean
		else if (auto node = std::dynamic_pointer_cast<IRImpliesOp>(astNode))
		{
			node->set_type(std::make_shared<IRBoolType>());
		}
		// address-of adds a pointer to the type
		else if (auto node = std::dynamic_pointer_cast<IRAddressOf>(astNode))
		{
			auto base_type = safety_cast<IRExpression>(list[0])->get_type();

			// unravel const
			while (auto cnst = std::dynamic_pointer_cast<IRConstType>(base_type))
				base_type = cnst->get_base_type();

			// address of an array doesn't return a pointer-to-array, just a pointer
			if (auto array = std::dynamic_pointer_cast<IRArrayType>(base_type))
				node->set_type(std::make_shared<IRPointerType>(array->get_base_type()));
			else
				node->set_type(std::make_shared<IRPointerType>(base_type));
		}
		// boolean negation returns a boolean
		else if (auto node = std::dynamic_pointer_cast<IRBoolNegationOp>(astNode))
		{
			node->set_type(std::make_shared<IRBoolType>());
		}
		// pointer deteference gets the base type of a pointer type
		else if (auto node = std::dynamic_pointer_cast<IRPointerDereference>(astNode))
		{
			auto base_type = safety_cast<IRExpression>(list[0])->get_type();

			// unravel const
			while (auto cnst = std::dynamic_pointer_cast<IRConstType>(base_type))
				base_type = cnst->get_base_type();

			// dereferencing an array is the same as dereferencing a pointer
			if (auto pointer_type = std::dynamic_pointer_cast<IRPointerType>(base_type))
				node->set_type(pointer_type->get_base_type());
			else if (auto array_type = std::dynamic_pointer_cast<IRArrayType>(base_type))
				node->set_type(array_type->get_base_type());
			else
				throw CastorException("Cannot dereference a non-pointer type!");
		}
		// indexing an array does the same to the types as dereferencing a pointer
		// however, we could be dealing with a pointer type or an array type, and while the code looks the same,
		//  the code isn't reusable without templating
		else if (auto node = std::dynamic_pointer_cast<IRArrayIndex>(astNode))
		{
			auto base_type = safety_cast<IRExpression>(list[0])->get_type();

			// unravel const
			while (auto cnst = std::dynamic_pointer_cast<IRConstType>(base_type))
				base_type = cnst->get_base_type();

			if (auto base = std::dynamic_pointer_cast<IRArrayType>(base_type))
				node->set_type(base->get_base_type());
			else if (auto base = std::dynamic_pointer_cast<IRPointerType>(base_type))
				node->set_type(base->get_base_type());
		}
		// same as above
		else if (auto node = std::dynamic_pointer_cast<IRArrayRangeIndex>(astNode))
		{
			auto base_type = safety_cast<IRExpression>(list[0])->get_type();

			// unravel const
			while (auto cnst = std::dynamic_pointer_cast<IRConstType>(base_type))
				base_type = cnst->get_base_type();

			if (auto base = std::dynamic_pointer_cast<IRArrayType>(base_type))
				node->set_type(base->get_base_type());
			else if (auto base = std::dynamic_pointer_cast<IRPointerType>(base_type))
				node->set_type(base->get_base_type());
		}
		// for field references, we need to look up the type of the field
		else if (auto node = std::dynamic_pointer_cast<IRFieldReference>(astNode))
		{
			auto field_name = node->get_field_name();
			auto base_object_type = node->get_base_object()->get_type();
			if (auto consttype = std::dynamic_pointer_cast<IRConstType>(base_object_type))
				base_object_type = consttype->get_base_type();
			auto base_type = safety_cast<IRClassType>(base_object_type);
			auto base_class = base_type->get_class_definition();
			auto class_scope = base_class->get_scope();
			auto field_name_var = class_scope->lookup(field_name);

			if (field_name_var)
			{
				node->set_type(field_name_var->get_type());
				node->set_field_name(field_name_var->get_name());
			}
			else
				throw UnknownVariableException(field_name + " (field reference)");
		}
		// if we AREN'T in a checked region
		else if (!inheritedValue.in_checked)
		{
			// all of our integer functions return unbounded integers
			if (auto node = std::dynamic_pointer_cast<IRDivideOp>(astNode))
			{
				node->set_type(std::make_shared<IRUnboundedIntegralType>());
			}
			else if (auto node = std::dynamic_pointer_cast<IRModuloOp>(astNode))
			{
				node->set_type(std::make_shared<IRUnboundedIntegralType>());
			}
			else if (auto node = std::dynamic_pointer_cast<IRMultiplyOp>(astNode))
			{
				node->set_type(std::make_shared<IRUnboundedIntegralType>());
			}
			else if (auto node = std::dynamic_pointer_cast<IRSubtractionOp>(astNode))
			{
				node->set_type(std::make_shared<IRUnboundedIntegralType>());
			}
			else if (auto node = std::dynamic_pointer_cast<IRAdditionOp>(astNode))
			{
				auto expr1_base_type = safety_cast<IRExpression>(list[0])->get_type();
				auto expr2_base_type = safety_cast<IRExpression>(list[1])->get_type();

				// unravel const
				while (auto cnst = std::dynamic_pointer_cast<IRConstType>(expr1_base_type))
					expr1_base_type = cnst->get_base_type();
				while (auto cnst = std::dynamic_pointer_cast<IRConstType>(expr2_base_type))
					expr2_base_type = cnst->get_base_type();

				// handle pointer arithmetic
				if (std::dynamic_pointer_cast<IRPointerType>(expr1_base_type))
					node->set_type(expr1_base_type);
				else if (std::dynamic_pointer_cast<IRPointerType>(expr2_base_type))
					node->set_type(expr2_base_type);
				// pointer arithmetic with array-to-pointer conversion
				else if (auto arr = std::dynamic_pointer_cast<IRArrayType>(expr1_base_type))
					node->set_type(std::make_shared<IRPointerType>(arr->get_base_type()));
				else if (auto arr = std::dynamic_pointer_cast<IRArrayType>(expr2_base_type))
					node->set_type(std::make_shared<IRPointerType>(arr->get_base_type()));
				// default
				else
					node->set_type(std::make_shared<IRUnboundedIntegralType>());
			}
			else if (auto node = std::dynamic_pointer_cast<IRNegationOp>(astNode))
			{
				node->set_type(std::make_shared<IRUnboundedIntegralType>());
			}
			else if (auto node = std::dynamic_pointer_cast<IRBitNegationOp>(astNode))
			{
				node->set_type(node->get_base_item()->get_type());
			}
		}
		// ...but if we are in a checked function
		else
		{
			// we need to calculate their return types
			// all of these call out to get_integral_type
			// code is not reusable without templating, so this will suffice
			if (auto node = std::dynamic_pointer_cast<IRDivideOp>(astNode))
			{
				auto exp1 = safety_cast<IRExpression>(list[0]);
				auto exp2 = safety_cast<IRExpression>(list[1]);

				node->set_type(get_integral_type(make_integral_type(exp1->get_type()), make_integral_type(exp2->get_type())));
			}
			else if (auto node = std::dynamic_pointer_cast<IRModuloOp>(astNode))
			{
				auto exp1 = safety_cast<IRExpression>(list[0]);
				auto exp2 = safety_cast<IRExpression>(list[1]);

				node->set_type(get_integral_type(make_integral_type(exp1->get_type()), make_integral_type(exp2->get_type())));
			}
			else if (auto node = std::dynamic_pointer_cast<IRMultiplyOp>(astNode))
			{
				auto exp1 = safety_cast<IRExpression>(list[0]);
				auto exp2 = safety_cast<IRExpression>(list[1]);

				node->set_type(get_integral_type(make_integral_type(exp1->get_type()), make_integral_type(exp2->get_type())));
			}
			else if (auto node = std::dynamic_pointer_cast<IRSubtractionOp>(astNode))
			{
				auto exp1 = safety_cast<IRExpression>(list[0]);
				auto exp2 = safety_cast<IRExpression>(list[1]);

				node->set_type(get_integral_type(make_integral_type(exp1->get_type()), make_integral_type(exp2->get_type())));
			}
			else if (auto node = std::dynamic_pointer_cast<IRAdditionOp>(astNode))
			{
				auto expr1_base_type = safety_cast<IRExpression>(list[0])->get_type();
				auto expr2_base_type = safety_cast<IRExpression>(list[1])->get_type();

				// unravel const
				while (auto cnst = std::dynamic_pointer_cast<IRConstType>(expr1_base_type))
					expr1_base_type = cnst->get_base_type();
				while (auto cnst = std::dynamic_pointer_cast<IRConstType>(expr2_base_type))
					expr2_base_type = cnst->get_base_type();

				// handle pointer arithmetic
				if (std::dynamic_pointer_cast<IRPointerType>(expr1_base_type))
					node->set_type(expr1_base_type);
				else if (std::dynamic_pointer_cast<IRPointerType>(expr2_base_type))
					node->set_type(expr2_base_type);
				// pointer arithmetic with array-to-pointer conversion
				else if (auto arr = std::dynamic_pointer_cast<IRArrayType>(expr1_base_type))
					node->set_type(std::make_shared<IRPointerType>(arr->get_base_type()));
				else if (auto arr = std::dynamic_pointer_cast<IRArrayType>(expr2_base_type))
					node->set_type(std::make_shared<IRPointerType>(arr->get_base_type()));
				// default
				else
					node->set_type(get_integral_type(make_integral_type(expr1_base_type), make_integral_type(expr2_base_type)));
			}
			else if (auto node = std::dynamic_pointer_cast<IRNegationOp>(astNode))
			{
				node->set_type(node->get_base_item()->get_type());
			}
			else if (auto node = std::dynamic_pointer_cast<IRBitNegationOp>(astNode))
			{
				node->set_type(node->get_base_item()->get_type());
			}
		}

		return astNode;
	}
	// for rvalue function calls, we just need to make sure we insert IRReference where the type signature requires it
	else if (auto ast = std::dynamic_pointer_cast<IRFunctionCallExpr<IRRValue>>(astNode))
	{
		auto name = ast->get_function_name();
		auto params = ast->get_params();

		for (int i = 0; i < params.size(); i++)
			if (function_table.get_ref(name, i))
				if (auto lval = std::dynamic_pointer_cast<IRLValue>(params[i])) // If we have an lvalue, we can directly bind to it
					params[i] = std::make_shared<IRReference>(params[i]->get_type(), lval);
				else // if we have an rvalue, we need to temporarily materialize a value. This is only possible with a non-volatile const reference.
					params[i] = std::make_shared<IRReference>(params[i]->get_type(),
							std::make_shared<IRMaterialize>(safety_cast<IRRValue>(params[i])));

		ast->set_params(params);

		return astNode;
	}
	// same for rvalue member function calls
	else if (auto ast = std::dynamic_pointer_cast<IRMemberFunctionCallExpr<IRRValue>>(astNode))
	{
		auto name = ast->get_function_name();
		auto params = ast->get_params();

		for (int i = 0; i < params.size(); i++)
			if (function_table.get_ref(name, i))
				if (auto lval = std::dynamic_pointer_cast<IRLValue>(params[i]))
					params[i] = std::make_shared<IRReference>(params[i]->get_type(), lval);
				else
					params[i] = std::make_shared<IRReference>(params[i]->get_type(),
							std::make_shared<IRMaterialize>(safety_cast<IRRValue>(params[i])));

		ast->set_params(params);

		return astNode;
	}
	// for lvalue function calls, we just need to make sure we insert IRReference where the type signature requires it
	else if (auto ast = std::dynamic_pointer_cast<IRFunctionCallExpr<IRLValue>>(astNode))
	{
		auto name = ast->get_function_name();
		auto params = ast->get_params();

		for (int i = 0; i < params.size(); i++)
			if (function_table.get_ref(name, i))
				if (auto lval = std::dynamic_pointer_cast<IRLValue>(params[i]))
					params[i] = std::make_shared<IRReference>(params[i]->get_type(), lval);
				else
					params[i] = std::make_shared<IRReference>(params[i]->get_type(),
							std::make_shared<IRMaterialize>(safety_cast<IRRValue>(params[i])));

		ast->set_params(params);

		return astNode;
	}
	// same for lvalue member function calls
	else if (auto ast = std::dynamic_pointer_cast<IRMemberFunctionCallExpr<IRLValue>>(astNode))
	{
		auto name = ast->get_function_name();
		auto params = ast->get_params();

		for (int i = 0; i < params.size(); i++)
			if (function_table.get_ref(name, i))
				if (auto lval = std::dynamic_pointer_cast<IRLValue>(params[i]))
					params[i] = std::make_shared<IRReference>(params[i]->get_type(), lval);
				else
					params[i] = std::make_shared<IRReference>(params[i]->get_type(),
							std::make_shared<IRMaterialize>(safety_cast<IRRValue>(params[i])));

		ast->set_params(params);

		return astNode;
	}
	else return astNode;
}

///
/// @brief Gets the resulting type of an integer operation
///
/// C++ type rules are NOT respected here. This is used ONLY in the verification langauge.
///
/// @param t1 First parameter in the operation
/// @param t2 Second parameter in the operation
/// @return Resulting type after the operation
///
std::shared_ptr<IRIntegralType> FillTypes::get_integral_type(std::shared_ptr<IRIntegralType> t1, std::shared_ptr<IRIntegralType> t2)
{
	auto bits = std::max(t1->get_bits(), t2->get_bits());   // get the max bits of both parameters
	auto sign = t1->get_is_signed() || t2->get_is_signed(); // should the result be signed?

	if (sign) // if one of the parameters is signed, we need to have the result be signed
	{
		switch (bits) // self-explanatory
		{
			case 8:
				return std::make_shared<IRS8Type>();
			case 16:
				return std::make_shared<IRS16Type>();
			case 32:
				return std::make_shared<IRS32Type>();
			case 64:
				return std::make_shared<IRS64Type>();
			default:
				return std::make_shared<IRS64Type>();
		}
	}
	else // if both parameters are unsigned, the result is unsigned
	{
		switch (bits) // self-explanatory
		{
			case 8:
				return std::make_shared<IRU8Type>();
			case 16:
				return std::make_shared<IRU16Type>();
			case 32:
				return std::make_shared<IRU32Type>();
			case 64:
				return std::make_shared<IRU64Type>();
			default:
				return std::make_shared<IRU64Type>();
		}
	}
}

/// 
/// @brief Gets the resulting type of a bitwise operation, excluding shifts.
///
/// C++ type rules are NOT respected here. This is used ONLY in the verification langauge.
///
/// @param t1 First parameter in the bitwise operation
/// @param t2 Second parameter in the bitwise operation
/// @return Resulting type after the operation
///
std::shared_ptr<IRIntegralType> FillTypes::get_bitwise_type(std::shared_ptr<IRIntegralType> t1, std::shared_ptr<IRIntegralType> t2)
{
	auto synthesize_type = [](bool is_signed, int bits) -> std::shared_ptr<IRIntegralType>
	{
		if (is_signed)
		{
			switch (bits)
			{
				case 8:
					return std::make_shared<IRS8Type>();
				case 16:
					return std::make_shared<IRS16Type>();
				case 32:
					return std::make_shared<IRS32Type>();
				case 64:
					return std::make_shared<IRS64Type>();
				default:
					return std::make_shared<IRS64Type>();
			}
		}
		else
		{
			switch(bits)
			{
				case 8:
					return std::make_shared<IRU8Type>();
				case 16:
					return std::make_shared<IRU16Type>();
				case 32:
					return std::make_shared<IRU32Type>();
				case 64:
					return std::make_shared<IRU64Type>();
				default:
					return std::make_shared<IRU64Type>();
			}
		}
	};

	auto decompose_type = [](std::shared_ptr<IRIntegralType> type) -> std::pair<bool, int>
	{
		return std::make_pair(type->get_is_signed(), type->get_bits());
	};

	auto t1_type = decompose_type(t1);
	auto t2_type = decompose_type(t2);

	auto bits = std::max(std::get<1>(t1_type), std::get<1>(t2_type)); // for filling in bitwise operation types, just get the max bits of the two arguments
	auto is_signed = std::get<0>(t1_type) || std::get<0>(t2_type);    // if either is signed, the result should be signed

	return synthesize_type(is_signed, bits);
}

///
/// @brief Gets the resulting type of a bitwise shift operation
///
/// C++ type rules are NOT respected here. This is used ONLY in the verification langauge.
///
/// @param t1 First parameter in the bitshift
/// @param t2 Second parameter in the bitshift
/// @return Resulting type after the operation
///
std::shared_ptr<IRIntegralType> FillTypes::get_bitshift_type(std::shared_ptr<IRIntegralType> t1, std::shared_ptr<IRIntegralType> t2)
{
	auto synthesize_type = [](bool is_signed, int bits) -> std::shared_ptr<IRIntegralType>
	{
		if (is_signed)
		{
			switch (bits)
			{
				case 8:
					return std::make_shared<IRS8Type>();
				case 16:
					return std::make_shared<IRS16Type>();
				case 32:
					return std::make_shared<IRS32Type>();
				case 64:
					return std::make_shared<IRS64Type>();
				default:
					return std::make_shared<IRS64Type>();
			}
		}
		else
		{
			switch(bits)
			{
				case 8:
					return std::make_shared<IRU8Type>();
				case 16:
					return std::make_shared<IRU16Type>();
				case 32:
					return std::make_shared<IRU32Type>();
				case 64:
					return std::make_shared<IRU64Type>();
				default:
					return std::make_shared<IRU64Type>();
			}
		}
	};

	auto decompose_type = [](std::shared_ptr<IRIntegralType> type) -> std::pair<bool, int>
	{
		return std::make_pair(type->get_is_signed(), type->get_bits());
	};

	auto t1_type = decompose_type(t1);
	auto t2_type = decompose_type(t2);

	auto bits = std::get<1>(t1_type); // we should use the bit width of the first parameter
	auto is_signed = std::get<0>(t1_type); // and the sign should reflect the first parameter

	return synthesize_type(is_signed, bits);
}

///
/// @brief Get the max int based on the type of the first parameter.
///
/// @param params List of parameters
/// @return Literal representing the max integral value
///
std::shared_ptr<IRLiteral> FillTypes::parse_max_int(std::vector<std::shared_ptr<IRExpression>> params)
{
	if (params.size() != 1)
		throw IncorrectArgumentCountException(1, params.size(), "max_int");

	auto type = params[0]->get_type(); // get the type of the argument

	// this is self-explanatory
	if (std::dynamic_pointer_cast<IRS8Type>(type))
		return std::make_shared<IRS8Literal>(SCHAR_MAX);
	else if (std::dynamic_pointer_cast<IRU8Type>(type))
		return std::make_shared<IRU8Literal>(UCHAR_MAX);
	else if (std::dynamic_pointer_cast<IRS16Type>(type))
		return std::make_shared<IRS16Literal>(SHRT_MAX);
	else if (std::dynamic_pointer_cast<IRU16Type>(type))
		return std::make_shared<IRU16Literal>(USHRT_MAX);
	else if (std::dynamic_pointer_cast<IRS32Type>(type))
		return std::make_shared<IRS32Literal>(INT_MAX);
	else if (std::dynamic_pointer_cast<IRU32Type>(type))
		return std::make_shared<IRU32Literal>(UINT_MAX);
	else if (std::dynamic_pointer_cast<IRS64Type>(type))
		return std::make_shared<IRS64Literal>(LLONG_MAX);
	else if (std::dynamic_pointer_cast<IRU64Type>(type))
		return std::make_shared<IRU64Literal>(ULLONG_MAX);
	else if (std::dynamic_pointer_cast<IRUnboundedIntegralType>(type))
		throw UnsupportedFeatureException("max_int in No Overflow mode");
	else // we should handle this case more gracefully, returning a null pointer is almost certainly NOT what we want when this happens
		return nullptr;
}

///
/// @brief Get the min int based on the type of the first parameter.
///
/// @param params List of parameters
/// @return Literal representing the min integral value
///
std::shared_ptr<IRLiteral> FillTypes::parse_min_int(std::vector<std::shared_ptr<IRExpression>> params)
{
	// the structure is the exact same as parse_max_int
	
	if (params.size() != 1)
		throw IncorrectArgumentCountException(1, params.size(), "min_int");

	auto type = params[0]->get_type();

	if (std::dynamic_pointer_cast<IRS8Type>(type))
		return std::make_shared<IRS8Literal>(SCHAR_MIN);
	else if (std::dynamic_pointer_cast<IRU8Type>(type))
		return std::make_shared<IRU8Literal>(0);
	else if (std::dynamic_pointer_cast<IRS16Type>(type))
		return std::make_shared<IRS16Literal>(SHRT_MIN);
	else if (std::dynamic_pointer_cast<IRU16Type>(type))
		return std::make_shared<IRU16Literal>(0);
	else if (std::dynamic_pointer_cast<IRS32Type>(type))
		return std::make_shared<IRS32Literal>(INT_MIN);
	else if (std::dynamic_pointer_cast<IRU32Type>(type))
		return std::make_shared<IRU32Literal>(0);
	else if (std::dynamic_pointer_cast<IRS64Type>(type))
		return std::make_shared<IRS64Literal>(LLONG_MIN);
	else if (std::dynamic_pointer_cast<IRU64Type>(type))
		return std::make_shared<IRU64Literal>(0);
	else if (std::dynamic_pointer_cast<IRUnboundedIntegralType>(type))
		throw UnsupportedFeatureException("min_int in No Overflow mode");
	else
		return nullptr;
}

///
/// @brief Downcasts a type to an integral type, accounting for const
///
/// @param type The base type
/// @return The type pointer, downcasted to an integral type
///
std::shared_ptr<IRIntegralType> FillTypes::make_integral_type(std::shared_ptr<IRType> type)
{
	if (auto inte = std::dynamic_pointer_cast<IRIntegralType>(type))
		return inte;
	else if (auto constt = std::dynamic_pointer_cast<IRConstType>(type))
		return safety_cast<IRIntegralType>(constt->get_base_type());
	else
		return safety_cast<IRIntegralType>(type);
}

