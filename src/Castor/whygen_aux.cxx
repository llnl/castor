// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "whygenerator.hxx"
#include "exception.hxx"
#include <iostream>
#include <algorithm>
#include <cassert>

///
/// @brief Turns an arbitrary expression into an rvalue expression.
///
/// This handles lvalue-to-rvalue conversion. If we already have an rvalue, just return that.
///
/// @param expr The expression to convert
/// @return The converted expression
///
std::shared_ptr<WhyRValue> WhyGenerator::makeRValue(std::shared_ptr<WhyExpression> expr)
{
	if (auto rv = std::dynamic_pointer_cast<WhyRValue>(expr)) // if it's already an rvalue, just return that
		return rv;
	else // otherwise do lvalue-to-rvalue conversion
		return std::make_shared<WhyMemoryGetExpr>(expr->get_type(), safety_cast<WhyLValue>(expr));
}

///
/// @brief Gets a Why3 literal representing the size of the argument
///
/// @param type The type to get the size of
/// @return A Why3 literal representing the size of the argument
///
std::shared_ptr<WhyRValue> WhyGenerator::get_sizeof(std::shared_ptr<WhyType> type)
{
	if (std::dynamic_pointer_cast<WhyS8Type>(type) || std::dynamic_pointer_cast<WhyU8Type>(type))
		return std::make_shared<WhyLiteral<uint64_t>>(std::make_shared<WhyU64Type>(), 1);
	else if (std::dynamic_pointer_cast<WhyS16Type>(type) || std::dynamic_pointer_cast<WhyU16Type>(type))
		return std::make_shared<WhyLiteral<uint64_t>>(std::make_shared<WhyU64Type>(), 2);
	else if (std::dynamic_pointer_cast<WhyS32Type>(type) || std::dynamic_pointer_cast<WhyU32Type>(type))
		return std::make_shared<WhyLiteral<uint64_t>>(std::make_shared<WhyU64Type>(), 4);
	else if (std::dynamic_pointer_cast<WhyS64Type>(type) || std::dynamic_pointer_cast<WhyU64Type>(type))
		return std::make_shared<WhyLiteral<uint64_t>>(std::make_shared<WhyU64Type>(), 8);
	else if (std::dynamic_pointer_cast<WhyClassType>(type))
		return std::make_shared<WhyArbitraryLiteral>(0);
	else
		return std::make_shared<WhyArbitraryLiteral>(0);
}

///
/// @brief This is a handler for unary operations.
///
/// @param astNode The IRUnaryOperation to look at
/// @param list The synthesized attributes so far
/// @return The synthesized unary operation
///
std::shared_ptr<WhyNode> WhyGenerator::handleUnaryOperation(std::shared_ptr<IRUnaryOperation> astNode, SynthesizedAttributesList list)
{
	// This is observed to match the C++17 standard draft, section 8.3.1.8 [expr.unary.op]
	if (auto ast = std::dynamic_pointer_cast<IRNegationOp>(astNode))
	{
		return std::make_shared<WhyNegationOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])));
	}
	// This is observed to match the C++17 standard draft, section 8.3.1.9 [expr.unary.op]
	else if (auto ast = std::dynamic_pointer_cast<IRBoolNegationOp>(astNode))
	{
		return std::make_shared<WhyBoolNegationOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])));
	}
	// This is observed to match the C++17 standard draft, section 8.3.1.10 [expr.unary.op]
	else if (auto ast = std::dynamic_pointer_cast<IRBitNegationOp>(astNode))
	{
		return std::make_shared<WhyBitNegationOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])));
	}
	// This is observed to match the C++17 standard draft, section 8.2.6 [expr.post.incr]
	else if (auto ast = std::dynamic_pointer_cast<IRIncrementOp>(astNode))
	{
		return std::make_shared<WhyIncrementOp>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]));
	}
	// This is observed to match the C++17 standard draft, section 8.2.6 [expr.post.incr]
	else if (auto ast = std::dynamic_pointer_cast<IRDecrementOp>(astNode))
	{
		return std::make_shared<WhyDecrementOp>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]));
	}
	else
	{
		throw UnknownIRNodeException(astNode->pp());
	}
}

///
/// @brief This is a handler for assignment operations
///
/// @param astNode The IRAssignmentOperation to look at
/// @param inheritedValue The inherited values
/// @param list The synthesized attributes so far
/// @return The synthesized assigment operation
///
std::shared_ptr<WhyNode> WhyGenerator::handleAssignmentOperation(std::shared_ptr<IRAssignmentOperation> astNode, WhyGenIV inheritedValue, SynthesizedAttributesList list)
{
	// This is observed to match the C++17 standard draft, section 8.18 [expr.ass]
	if (auto ast = std::dynamic_pointer_cast<IRAssignOp>(astNode))
	{
		return std::make_shared<WhyAssignOp>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRAdditionAssignOp>(astNode))
	{
		return std::make_shared<WhyCompoundAssignOp<WhyAddOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRSubtractionAssignOp>(astNode))
	{
		return std::make_shared<WhyCompoundAssignOp<WhySubtractOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRMultiplyAssignOp>(astNode))
	{
		return std::make_shared<WhyCompoundAssignOp<WhyMultiplyOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRDivideAssignOp>(astNode))
	{
		return std::make_shared<WhyCompoundAssignOp<WhyDivideOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRModuloAssignOp>(astNode))
	{
		return std::make_shared<WhyCompoundAssignOp<WhyModuloOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRBitAndAssignOp>(astNode))
	{
		return std::make_shared<WhyCompoundAssignOp<WhyBitwiseAndOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRBitOrAssignOp>(astNode))
	{
		return std::make_shared<WhyCompoundAssignOp<WhyBitwiseOrOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRBitXorAssignOp>(astNode))
	{
		return std::make_shared<WhyCompoundAssignOp<WhyBitwiseXorOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRBitLShiftAssignOp>(astNode))
	{
		return std::make_shared<WhyCompoundAssignOp<WhyBitwiseLeftShiftOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRBitRShiftAssignOp>(astNode))
	{
		// strip any const qualifier
		auto type = ast->get_type();
		if (auto constt = std::dynamic_pointer_cast<IRConstType>(type)) type = constt->get_base_type();

		// if nonreal type, we just assume an arithmetic right shift op
		// this is a legal interpretation of the standard no matter if it's signed or unsigned
		if (std::dynamic_pointer_cast<IRNonRealType>(type))
			return std::make_shared<WhyCompoundAssignOp<WhyBitwiseArithmeticRightShiftOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));

		// if the result is signed, we do an arithmetic shift
		// otherwise do a logical shift
		else if (safety_cast<IRIntegralType>(type)->get_is_signed())
			return std::make_shared<WhyCompoundAssignOp<WhyBitwiseArithmeticRightShiftOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
		else
			return std::make_shared<WhyCompoundAssignOp<WhyBitwiseLogicalRightShiftOp>>(getWhyTypeFromIRType(ast->get_type()),
				safety_cast<WhyLValue>(list[0]),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else
	{
		throw UnknownIRNodeException(astNode->pp());
	}
}

///
/// @brief This is a handler for binary operations.
///
/// @param astNode The IRBinaryOperation to look at
/// @param inheritedValue The inherited values
/// @param list The synthesized attributes so far
/// @return The synthesized binary operation
///
std::shared_ptr<WhyNode> WhyGenerator::handleBinaryOperation(std::shared_ptr<IRBinaryOperation> astNode, WhyGenIV inheritedValue, SynthesizedAttributesList list)
{
	// This is observed to match the C++17 standard draft, section 8.7 [expr.add]
	if (auto ast = std::dynamic_pointer_cast<IRAdditionOp>(astNode))
	{
		return std::make_shared<WhyAddOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.7 [expr.add]
	else if (auto ast = std::dynamic_pointer_cast<IRSubtractionOp>(astNode))
	{
		return std::make_shared<WhySubtractOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.6 [expr.mul]
	else if (auto ast = std::dynamic_pointer_cast<IRMultiplyOp>(astNode))
	{
		return std::make_shared<WhyMultiplyOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.6 [expr.mul]
	else if (auto ast = std::dynamic_pointer_cast<IRDivideOp>(astNode))
	{
		return std::make_shared<WhyDivideOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.6 [expr.mul]
	else if (auto ast = std::dynamic_pointer_cast<IRModuloOp>(astNode))
	{
		return std::make_shared<WhyModuloOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.10 [expr.eq]
	else if (auto ast = std::dynamic_pointer_cast<IREqualsOp>(astNode))
	{
		return std::make_shared<WhyEqualsOp>(makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.10 [expr.eq]
	else if (auto ast = std::dynamic_pointer_cast<IRNotEqualsOp>(astNode))
	{
		return std::make_shared<WhyNotEqualsOp>(makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else if (auto ast = std::dynamic_pointer_cast<IRImpliesOp>(astNode))
	{
		return std::make_shared<WhyImpliesOp>(makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.9 [expr.rel]
	else if (auto ast = std::dynamic_pointer_cast<IRLessThanOp>(astNode))
	{
		return std::make_shared<WhyLessThanOp>(makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.9 [expr.rel]
	else if (auto ast = std::dynamic_pointer_cast<IRLessEqualsOp>(astNode))
	{
		return std::make_shared<WhyLessEqualsOp>(makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.9 [expr.rel]
	else if (auto ast = std::dynamic_pointer_cast<IRGreaterThanOp>(astNode))
	{
		return std::make_shared<WhyGreaterThanOp>(makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.9 [expr.rel]
	else if (auto ast = std::dynamic_pointer_cast<IRGreaterEqualsOp>(astNode))
	{
		return std::make_shared<WhyGreaterEqualsOp>(makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.14 [expr.log.and]
	else if (auto ast = std::dynamic_pointer_cast<IRAndOp>(astNode))
	{
		// we need to determine if we're in a VC or not
		// if we are, we don't generate a short-circuiting conjunction
		if (inheritedValue.in_vc)
			return std::make_shared<WhyAndOp>(makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
		else
			return std::make_shared<WhyShortCircuitAndOp>(makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.15 [expr.log.or]
	else if (auto ast = std::dynamic_pointer_cast<IROrOp>(astNode))
	{
		// same thing for disjunction, VCs don't use a short-circuiting version
		if (inheritedValue.in_vc)
			return std::make_shared<WhyOrOp>(makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
		else
			return std::make_shared<WhyShortCircuitOrOp>(makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.11 [expr.bit.and]
	else if (auto ast = std::dynamic_pointer_cast<IRBitAndOp>(astNode))
	{
		return std::make_shared<WhyBitwiseAndOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.13 [expr.or]
	else if (auto ast = std::dynamic_pointer_cast<IRBitOrOp>(astNode))
	{
		return std::make_shared<WhyBitwiseOrOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.12 [expr.xor]
	else if (auto ast = std::dynamic_pointer_cast<IRBitXorOp>(astNode))
	{
		return std::make_shared<WhyBitwiseXorOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.8 [expr.shift]
	else if (auto ast = std::dynamic_pointer_cast<IRBitLShiftOp>(astNode))
	{
		return std::make_shared<WhyBitwiseLeftShiftOp>(getWhyTypeFromIRType(ast->get_type()),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	// This is observed to match the C++17 standard draft, section 8.8 [expr.shift]
	else if (auto ast = std::dynamic_pointer_cast<IRBitRShiftOp>(astNode))
	{
		// strip any const qualifier
		auto type = ast->get_type();
		if (auto constt = std::dynamic_pointer_cast<IRConstType>(type)) type = constt->get_base_type();

		// if nonreal type, we just assume an arithmetic right shift op
		// this is a legal interpretation of the standard no matter if it's signed or unsigned
		if (std::dynamic_pointer_cast<IRNonRealType>(type))
			return std::make_shared<WhyBitwiseArithmeticRightShiftOp>(getWhyTypeFromIRType(type),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));

		// if the result is signed, we do an arithmetic shift
		// otherwise do a logical shift
		else if (safety_cast<IRIntegralType>(type)->get_is_signed())
			return std::make_shared<WhyBitwiseArithmeticRightShiftOp>(getWhyTypeFromIRType(type),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
		else
			return std::make_shared<WhyBitwiseLogicalRightShiftOp>(getWhyTypeFromIRType(type),
				makeRValue(safety_cast<WhyExpression>(list[0])),
				makeRValue(safety_cast<WhyExpression>(list[1])));
	}
	else
	{
		throw UnknownIRNodeException(astNode->pp());
	}
}

///
/// @brief This is a handler for literals
///
/// @param astNode The IRLiteral to look at
/// @return The Why3 literal object
///
std::shared_ptr<WhyNode> WhyGenerator::handleLiteral(std::shared_ptr<IRLiteral> astNode)
{
	if (auto ast = std::dynamic_pointer_cast<IRS8Literal>(astNode))
	{
		return std::make_shared<WhyLiteral<int8_t>>(std::make_shared<WhyS8Type>(), ast->get_value());
	}
	else if (auto ast = std::dynamic_pointer_cast<IRU8Literal>(astNode))
	{
		return std::make_shared<WhyLiteral<uint8_t>>(std::make_shared<WhyU8Type>(), ast->get_value());
	}
	else if (auto ast = std::dynamic_pointer_cast<IRS16Literal>(astNode))
	{
		return std::make_shared<WhyLiteral<int16_t>>(std::make_shared<WhyS16Type>(), ast->get_value());
	}
	else if (auto ast = std::dynamic_pointer_cast<IRU16Literal>(astNode))
	{
		return std::make_shared<WhyLiteral<uint16_t>>(std::make_shared<WhyU16Type>(), ast->get_value());
	}
	else if (auto ast = std::dynamic_pointer_cast<IRS32Literal>(astNode))
	{
		return std::make_shared<WhyLiteral<int32_t>>(std::make_shared<WhyS32Type>(), ast->get_value());
	}
	else if (auto ast = std::dynamic_pointer_cast<IRU32Literal>(astNode))
	{
		return std::make_shared<WhyLiteral<uint32_t>>(std::make_shared<WhyU32Type>(), ast->get_value());
	}
	else if (auto ast = std::dynamic_pointer_cast<IRS64Literal>(astNode))
	{
		return std::make_shared<WhyLiteral<int64_t>>(std::make_shared<WhyS64Type>(), ast->get_value());
	}
	else if (auto ast = std::dynamic_pointer_cast<IRU64Literal>(astNode))
	{
		return std::make_shared<WhyLiteral<uint64_t>>(std::make_shared<WhyU64Type>(), ast->get_value());
	}
	else if (auto ast = std::dynamic_pointer_cast<IRBoolLiteral>(astNode))
	{
		return std::make_shared<WhyLiteral<bool>>(std::make_shared<WhyBoolType>(), ast->get_value());
	}
	else
	{
		throw UnknownIRNodeException(astNode->pp());
	}
}

///
/// @brief Gets the WhyType from an IR type.
///
/// This does not necessarily correlate 1:1 with a Why3 type. In fact, it usually doesn't.
/// The pretty-printers handle that.
///
/// @param type The IR type
/// @return The corresponding WhyType
///
std::shared_ptr<WhyType> WhyGenerator::getWhyTypeFromIRType(std::shared_ptr<IRType> type)
{
	std::shared_ptr<WhyType> ret;

	if (std::dynamic_pointer_cast<IRS32Type>(type))
	{
		ret = std::make_shared<WhyS32Type>();
	}
	else if (std::dynamic_pointer_cast<IRU32Type>(type))
	{
		ret = std::make_shared<WhyU32Type>();
	}
	else if (std::dynamic_pointer_cast<IRS8Type>(type))
	{
		ret = std::make_shared<WhyS8Type>();
	}
	else if (std::dynamic_pointer_cast<IRU8Type>(type))
	{
		ret = std::make_shared<WhyU8Type>();
	}
	else if (std::dynamic_pointer_cast<IRS16Type>(type))
	{
		ret = std::make_shared<WhyS16Type>();
	}
	else if (std::dynamic_pointer_cast<IRU16Type>(type))
	{
		ret = std::make_shared<WhyU16Type>();
	}
	else if (std::dynamic_pointer_cast<IRS64Type>(type))
	{
		ret = std::make_shared<WhyS64Type>();
	}
	else if (std::dynamic_pointer_cast<IRU64Type>(type))
	{
		ret = std::make_shared<WhyU64Type>();
	}
	else if (std::dynamic_pointer_cast<IRUnboundedIntegralType>(type))
	{
		ret = std::make_shared<WhyUnboundedIntegralType>();
	}
	else if (std::dynamic_pointer_cast<IRBoolType>(type))
	{
		ret = std::make_shared<WhyBoolType>();
	}
	else if (std::dynamic_pointer_cast<IRNonRealType>(type))
	{
		ret = std::make_shared<WhyUnknownType>();
	}
	else if (std::dynamic_pointer_cast<IRVoidType>(type))
	{
		ret = std::make_shared<WhyUnitType>();
	}
	else if (auto t = std::dynamic_pointer_cast<IRPointerType>(type))
	{
		ret = std::make_shared<WhyPointerType>(getWhyTypeFromIRType(t->get_base_type()));
	}
	else if (auto t = std::dynamic_pointer_cast<IRArrayType>(type))
	{
		ret = std::make_shared<WhyArrayType>(getWhyTypeFromIRType(t->get_base_type()), t->get_size());
	}
	else if (auto t = std::dynamic_pointer_cast<IRClassType>(type))
	{
		// make sure to calculate the offsets for class types
		ret = std::make_shared<WhyClassType>(t->get_class_name(), calculateOffsets(t));
	}
	else if (auto t = std::dynamic_pointer_cast<IRConstType>(type))
	{
		ret = getWhyTypeFromIRType(t->get_base_type());
		ret->set_const();
	}
	else
	{
		throw UnknownTypeException(type->pp());
	}

	if (type->get_is_reference())
	{
		ret->set_reference();
	}

	return ret;
}

///
/// @brief Handler for calculating the member offsets for a class.
///
/// See OffsetTable for further information.
///
/// @param clas The IR class type
/// @return The offset table
///
OffsetTable WhyGenerator::calculateOffsets(std::shared_ptr<IRClassType> clas)
{
	OffsetTable table;
	int ctr = 0;
	auto vardecls = clas->get_class_definition()->get_vars(); // get the variable declarations

	std::vector<std::shared_ptr<IRVariable>> vars(vardecls.size()); // create a vector of the same size

	// transform the variable declarations to variables
	std::transform(vardecls.begin(), vardecls.end(), vars.begin(),
		[](std::shared_ptr<IRVariableDeclarationStmt> vardecl) { return vardecl->get_var(); });

	// iterate through the variables
	for (auto v : vars)
	{
		auto type = getWhyTypeFromIRType(v->get_type()); // get the type
		table[WhyName(v->get_name())] = std::make_pair(ctr, type); // set the offset in the table to the counter
		ctr += type->get_size(); // increment the counter
	}

	table.set_size(ctr); // set the table size
	
	return table;
}

///
/// @brief Gets a list of WhyVariables from a scope object (which usually returns IRVariables).
///
/// @param scope The scope object
/// @return A list of WhyVariables
///
std::vector<std::shared_ptr<WhyVariable>> WhyGenerator::get_variables(std::shared_ptr<R2WML_Scope> scope)
{
	auto variable_list = scope->get_variable_list(); // get the IRVariables
	std::vector<std::shared_ptr<WhyVariable>> vars;

	for (auto v : variable_list)
		vars.push_back(safety_cast<WhyVariable>((*this)(v))); // convert IRVariables to WhyVariables

	return vars;
}

///
/// @brief This is a handler for function calls that return a non-reference
///
/// @param ast The function call expression
/// @param list The synthesized attributes list
/// @return Result of calling the function (might be a WhyFunctionCall, might be a builtin function)
///
std::shared_ptr<WhyNode> WhyGenerator::handleFunctionCall(std::shared_ptr<IRFunctionCallExpr<IRRValue>> ast, SynthesizedAttributesList list)
{
		auto name = safety_cast<__WhyFunctionRef>(list[0])->function_name;

		if (name == "id") // for the identity function, just return its parameter
			return list[1];
		else if (name == "valid")
		{
			std::vector<std::shared_ptr<WhyExpression>> params;

			for (int i = 1; i < list.size(); i++)
				params.push_back(safety_cast<WhyExpression>(list[i])); // "valid" needs to take an expression

			return std::make_shared<WhyValid>(params);
		}
		else if (name == "freed")
		{
			std::vector<std::shared_ptr<WhyLValue>> params;

			for (int i = 1; i < list.size(); i++)
				params.push_back(safety_cast<WhyLValue>(list[i])); // "freed" needs to take lvalues

			return std::make_shared<WhyFreed>(params);
		}
		else if (name == "separated")
		{
			std::vector<std::shared_ptr<WhyExpression>> params;

			for (int i = 1; i < list.size(); i++)
				params.push_back(safety_cast<WhyExpression>(list[i])); // "separated" needs to take expressions

			return std::make_shared<WhySeparated>(params);
		}
		else if (name == "valid_array")
		{
			std::shared_ptr<WhyExpression> arr = safety_cast<WhyExpression>(list[1]); // "valid_array" needs an expression as the
												  // first parameter
			std::shared_ptr<WhyRValue> size = makeRValue(safety_cast<WhyExpression>(list[2])); // second parameter needs
													   // to be an rvalue

			return std::make_shared<WhyValidArray>(arr, size);
		}
		else if (name == "old")
		{
			std::shared_ptr<WhyRValue> base = makeRValue(safety_cast<WhyExpression>(list[1])); // "old" needs to take an rvalue

			return std::make_shared<WhyOld>(base);
		}
		else if (name == "unchanged")
		{
			std::shared_ptr<WhyRValue> base = makeRValue(safety_cast<WhyExpression>(list[1])); // "unchanged" needs to take an
													   // rvalue

			return std::make_shared<WhyUnchanged>(base);
		}
		else if (name == "at")
		{
			std::shared_ptr<WhyRValue> base = makeRValue(safety_cast<WhyExpression>(list[1])); // "at" takes an rvalue
			std::shared_ptr<WhyLabel> label = safety_cast<WhyLabel>(list[2]); // and a label

			return std::make_shared<WhyAt>(base, label);
		}
		else if (name == "alias_of")
		{
			std::shared_ptr<WhyLValue> base = safety_cast<WhyLValue>(list[1]); // "alias_of" takes two lvalues
			std::shared_ptr<WhyLValue> aliased = safety_cast<WhyLValue>(list[2]);

			return std::make_shared<WhyAliasOf>(base, aliased);
		}
		else if (name == "nth")
		{
			std::shared_ptr<WhyRValue> base = makeRValue(safety_cast<WhyExpression>(list[1]));
			std::shared_ptr<WhyRValue> idx = makeRValue(safety_cast<WhyExpression>(list[2]));

			return std::make_shared<WhyNth>(base, idx);
		}
		else if (name == "sizeof")
		{
			std::shared_ptr<WhyExpression> base = safety_cast<WhyExpression>(list[1]);

			return get_sizeof(base->get_type());
		}
		// not a builtin function
		else
		{
			std::vector<std::shared_ptr<WhyExpression>> params;

			for (int i = 1; i < list.size(); i++)
				params.push_back(safety_cast<WhyExpression>(list[i]));

			if (ast->get_is_constructor()) // constructors are handled specially
				return std::make_shared<WhyConstructor>(getWhyTypeFromIRType(ast->get_type()), name, params);
			else // otherwise just create a regular function call
				return std::make_shared<WhyFunctionCall<WhyRValue>>(getWhyTypeFromIRType(ast->get_type()), name, params);
		}
}

///
/// @brief This is a handler for function calls that return a reference
///
/// @param ast The function call expression
/// @param list The synthesized attributes list
/// @return The WhyFunctionCall objet
///
std::shared_ptr<WhyNode> WhyGenerator::handleFunctionCall(std::shared_ptr<IRFunctionCallExpr<IRLValue>> ast, SynthesizedAttributesList list)
{
	// no builtins that return a reference
	// constructors also don't return references
	// so we just generate a regular function call
	
	auto name = safety_cast<__WhyFunctionRef>(list[0])->function_name;
	std::vector<std::shared_ptr<WhyExpression>> params;

	for (int i = 1; i < list.size(); i++)
		params.push_back(safety_cast<WhyExpression>(list[i]));

	return std::make_shared<WhyFunctionCall<WhyLValue>>(getWhyTypeFromIRType(ast->get_type()), name, params);
}

