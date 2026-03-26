// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <vector>
#include <string>
#include <memory>
#include "offset_table.hxx"
#include "whyname.cxx"

#ifndef WHY3_AST
#define WHY3_AST

///
/// @brief This is the namespace containing all the Why3 nodes
///
namespace Why3
{

///
/// @brief Base class of all Why3 nodes
///
/// All Why3 nodes must inherit from this class
///
class WhyNode
{
public:
	///
	/// @brief Constructor
	///
	WhyNode();

	///
	/// @brief Pretty-printer
	///
	/// @return This node as a pretty-printed string
	///
	virtual std::string pp() = 0;

	///
	/// @brief Provide child classes for traversals
	///
	/// When traversing over the Why3 AST, these functions are called to get a
	/// list of child Why3 nodes that should be traversed before traversing
	/// this node.
	///
	/// @return A vector of WhyNodes that are children of this class
	///
	virtual std::vector<std::shared_ptr<WhyNode>> traverse() = 0;
};

///
/// @brief Base class for all Why3 types
///
class WhyType : public WhyNode
{
private:
	bool is_const;
	bool is_reference;

public:
	WhyType();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();

	virtual int get_size() = 0;

	virtual bool get_const();

	virtual void set_const();

	virtual bool get_reference();

	virtual void set_reference();
};

///
/// @brief Represents the type of a label
///
class WhyLabelType : public WhyType
{
public:
	WhyLabelType();

	virtual std::string pp();

	virtual int get_size();
};

///
/// @brief Represents a unit type
///
class WhyUnitType : public WhyType
{
public:
	WhyUnitType();

	virtual std::string pp();

	virtual int get_size();
};

///
/// @brief Represents a bool type
///
class WhyBoolType : public WhyType
{
public:
	WhyBoolType();

	virtual int get_size();
};

///
/// @brief Represents an aggregate type
///
class WhyClassType : public WhyType
{
private:
	WhyName class_name;
	OffsetTable offset_table;

public:
	WhyClassType(WhyName class_name, OffsetTable offset_table);

	virtual OffsetTable get_offset_table();

	virtual int get_size();
};

///
/// @brief Base class for all integer types
///
class WhyIntegralType : public WhyType
{
private:
	int bits;
	bool is_signed;

public:
	WhyIntegralType(int bits, bool is_signed);

	virtual int get_bits();

	virtual bool get_is_signed();

	virtual int get_size();
};

///
/// @brief Represents a signed 64-bit integer
///
class WhyS64Type : public WhyIntegralType
{
public:
	WhyS64Type();
};

///
/// @brief Represents a signed 32-bit integer
///
class WhyS32Type : public WhyIntegralType
{
public:
	WhyS32Type();
};

///
/// @brief Represents a signed 16-bit integer
///
class WhyS16Type : public WhyIntegralType
{
public:
	WhyS16Type();
};

///
/// @brief Represents a signed 8-bit integer
///
class WhyS8Type : public WhyIntegralType
{
public:
	WhyS8Type();
};

///
/// @brief Represents an unsigned 64-bit integer
///
class WhyU64Type : public WhyIntegralType
{
public:
	WhyU64Type();
};

///
/// @brief Represents an unsigned 32-bit integer
///
class WhyU32Type : public WhyIntegralType
{
public:
	WhyU32Type();
};

///
/// @brief Represents an unsigned 16-bit integer
///
class WhyU16Type : public WhyIntegralType
{
public:
	WhyU16Type();
};

///
/// @brief Represents an unsigned 8-bit integer
///
class WhyU8Type : public WhyIntegralType
{
public:
	WhyU8Type();
};

///
/// @brief Represents an unbounded integer (has no minimum or maximum)
///
class WhyUnboundedIntegralType : public WhyIntegralType
{
public:
	WhyUnboundedIntegralType();
};

///
/// @brief Represents a pointer type
///
/// Has a pointer to the base type
///
class WhyPointerType : public WhyUnboundedIntegralType
{
protected:
	std::shared_ptr<WhyType> base_type;

public:
	WhyPointerType(std::shared_ptr<WhyType> base_type);

	virtual std::shared_ptr<WhyType> get_base_type();
};

///
/// @brief Represents an array type
///
class WhyArrayType : public WhyPointerType
{
protected:
	int length;

public:
	WhyArrayType(std::shared_ptr<WhyType> base_type, int length);

	virtual int get_length();

	virtual int get_size();
};

///
/// @brief Placeholder for unknown types
///
class WhyUnknownType : public WhyType
{
public:
	WhyUnknownType();

	virtual std::string pp();

	virtual int get_size();
};

class WhyVariable;

///
/// @brief Wrapper class for holding a list of variables
///
class __WhyVariableList : public WhyNode
{
public:
	std::vector<std::shared_ptr<WhyVariable>> vars;

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

class WhyFunction;

///
/// @brief Wrapper class for holding a list of functions
///
class __WhyFunctions : public WhyNode
{
public:
	std::vector<std::shared_ptr<WhyFunction>> funcs;

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Parent class for expressions
///
/// All expressions have an associated type.
///
class WhyExpression : public WhyNode
{
protected:
	std::shared_ptr<WhyType> type;

public:
	WhyExpression(std::shared_ptr<WhyType> type);

	virtual std::shared_ptr<WhyType> get_type();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents an lvalue expression
///
class WhyLValue : public WhyExpression
{
public:
	WhyLValue(std::shared_ptr<WhyType> type);
};

///
/// @brief Represents an rvalue expression
///
class WhyRValue : public WhyExpression
{
public:
	WhyRValue(std::shared_ptr<WhyType> type);
};

///
/// @brief Represents the notion of a null pointer
///
class WhyNullptr : public WhyRValue
{
public:
	WhyNullptr();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents temporary object materialization
///
class WhyMaterialize : public WhyLValue
{
private:
	std::shared_ptr<WhyRValue> base_expr;

public:
	WhyMaterialize(std::shared_ptr<WhyRValue> base_expr);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a ternary expression
///
/// @tparam T Either WhyRValue or WhyLValue
///
template <typename T>
class WhyTernary : public T
{
private:
	std::shared_ptr<WhyRValue> condition;
	std::shared_ptr<T> then;
	std::shared_ptr<T> els;

public:
	WhyTernary(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> condition, std::shared_ptr<T> then, std::shared_ptr<T> els);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

template class WhyTernary<WhyLValue>;
template class WhyTernary<WhyRValue>;

#ifdef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
///
/// @brief Allocates memory and gets a pointer to that memory
///
class WhyNew : public WhyRValue
{
private:
	std::shared_ptr<WhyExpression> constructed_object;

public:
	WhyNew(std::shared_ptr<WhyType> type, std::shared_ptr<WhyExpression> constructed_object);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Frees dynamically allocated memory
///
class WhyDelete : public WhyRValue
{
private:
	std::shared_ptr<WhyLValue> base_item;

public:
	WhyDelete(std::shared_ptr<WhyLValue> base_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};
#endif

///
/// @brief Represents a reference to a label
///
class WhyLabel : public WhyRValue
{
private:
	WhyName name;

public:
	WhyLabel(WhyName name);

	virtual std::string pp();
};	

///
/// @brief Base class for quantifiers (forall, exists)
///
class WhyQuantifier : public WhyRValue
{
protected:
	std::vector<std::shared_ptr<WhyVariable>> vars;
	std::shared_ptr<WhyRValue> expr;

public:
	WhyQuantifier(std::vector<std::shared_ptr<WhyVariable>> vars, std::shared_ptr<WhyRValue> expr);

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();

	virtual std::vector<std::shared_ptr<WhyVariable>> get_vars();
};

///
/// @brief Universal quantification (forall)
///
class WhyForall : public WhyQuantifier
{
public:
	WhyForall(std::vector<std::shared_ptr<WhyVariable>> vars, std::shared_ptr<WhyRValue> expr);

	virtual std::string pp();
};

///
/// @brief Existential quantification (exists)
///
class WhyExists : public WhyQuantifier
{
public:
	WhyExists(std::vector<std::shared_ptr<WhyVariable>> vars, std::shared_ptr<WhyRValue> expr);

	virtual std::string pp();
};

///
/// @brief Represents the result of a function in a verification condition
///
/// @tparam T Either a WhyRValue or WhyLValue, stating whether the result is an rvalue or an lvalue
///
template <typename T>
class WhyResult : public T
{
public:
	WhyResult(std::shared_ptr<WhyType> type);

	virtual std::string pp();
};

template class WhyResult<WhyRValue>;
template class WhyResult<WhyLValue>;

///
/// @brief Casts a value from one type to another type
///
template <typename T>
class WhyCast : public T
{
private:
	std::shared_ptr<T> base_object;

public:
	WhyCast(std::shared_ptr<WhyType> to_type, std::shared_ptr<T> base_object);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

template class WhyCast<WhyRValue>;
template class WhyCast<WhyLValue>;

///
/// @brief Wrapper class for holding a reference to a function
///
class __WhyFunctionRef : public WhyNode
{
public:
	WhyName function_name;

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a literal value
///
/// @tparam T The type of the literal (not the WhyType, but a C++ type)
///
template <typename T>
class WhyLiteral : public WhyRValue
{
protected:
	T literal;

public:
	WhyLiteral(std::shared_ptr<WhyType> type, T literal);

	virtual std::string pp();

	virtual T get_literal();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents an arbitrary literal
///
class WhyArbitraryLiteral : public WhyLiteral<uint64_t>
{
public:
	WhyArbitraryLiteral(int minimum);

	virtual std::string pp();
};

///
/// @brief Base class for binary operations
///
class WhyBinaryOperation : public WhyRValue
{
protected:
	std::shared_ptr<WhyRValue> lhs;
	std::shared_ptr<WhyRValue> rhs;

public:
	WhyBinaryOperation(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	std::string get_suffix();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Postfix increment
///
class WhyIncrementOp : public WhyRValue
{
protected:
	std::shared_ptr<WhyLValue> base;

public:
	WhyIncrementOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Postfix decrement
///
class WhyDecrementOp : public WhyRValue
{
protected:
	std::shared_ptr<WhyLValue> base;

public:
	WhyDecrementOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Base class for unary operations
///
class WhyUnaryOperation : public WhyRValue
{
protected:
	std::shared_ptr<WhyRValue> base;

public:
	WhyUnaryOperation(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base);

	std::string get_suffix();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Integer negation
///
class WhyNegationOp : public WhyUnaryOperation
{
public:
	WhyNegationOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base);

	virtual std::string pp();
};

///
/// @brief Boolean negation
///
class WhyBoolNegationOp : public WhyUnaryOperation
{
public:
	WhyBoolNegationOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base);

	virtual std::string pp();
};

///
/// @brief Bitwise complement
///
class WhyBitNegationOp : public WhyUnaryOperation
{
public:
	WhyBitNegationOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base);

	virtual std::string pp();
};

///
/// @brief Variable assignment
///
class WhyAssignOp : public WhyLValue
{
private:
	std::shared_ptr<WhyLValue> lhs;
	std::shared_ptr<WhyRValue> rhs;

public:
	WhyAssignOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Compound assignment operations
///
/// @tparam T Some WhyBinaryOperation
///
template <typename T>
class WhyCompoundAssignOp : public WhyLValue
{
private:
	std::shared_ptr<WhyAssignOp> base_assignment;
	std::shared_ptr<WhyLValue> lhs;

public:
	WhyCompoundAssignOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Comma operator
///
/// @tparam T Either WhyLValue or WhyRValue
///
template <typename T>
class WhyCommaOp : public T
{
private:
	std::shared_ptr<WhyRValue> lhs;
	std::shared_ptr<T> rhs;

public:
	WhyCommaOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<T> rhs);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Integer addition
///
class WhyAddOp : public WhyBinaryOperation
{
public:
	WhyAddOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Integer subtraction
///
class WhySubtractOp : public WhyBinaryOperation
{
public:
	WhySubtractOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Integer multiplication
///
class WhyMultiplyOp : public WhyBinaryOperation
{
public:
	WhyMultiplyOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Integer division
///
class WhyDivideOp : public WhyBinaryOperation
{
public:
	WhyDivideOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Integer modulo
///
class WhyModuloOp : public WhyBinaryOperation
{
public:
	WhyModuloOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Less-than comparison
///
class WhyLessThanOp : public WhyBinaryOperation
{
public:
	WhyLessThanOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Less-than-or-equal-to comparison
///
class WhyLessEqualsOp : public WhyBinaryOperation
{
public:
	WhyLessEqualsOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Greater-than comparison
///
class WhyGreaterThanOp : public WhyBinaryOperation
{
public:
	WhyGreaterThanOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Greater-than-or-equal-to comparison
///
class WhyGreaterEqualsOp : public WhyBinaryOperation
{
public:
	WhyGreaterEqualsOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Equal-to comparison
///
class WhyEqualsOp : public WhyBinaryOperation
{
public:
	WhyEqualsOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Not-equal-to comparison
///
class WhyNotEqualsOp : public WhyBinaryOperation
{
public:
	WhyNotEqualsOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Logical implication
///
class WhyImpliesOp : public WhyBinaryOperation
{
public:
	WhyImpliesOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Logical conjunction
///
class WhyAndOp : public WhyBinaryOperation
{
public:
	WhyAndOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Logical disjunction
///
class WhyOrOp : public WhyBinaryOperation
{
public:
	WhyOrOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Short-circuiting logical AND
///
class WhyShortCircuitAndOp : public WhyBinaryOperation
{
public:
	WhyShortCircuitAndOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Short-circuiting logical OR
///
class WhyShortCircuitOrOp : public WhyBinaryOperation
{
public:
	WhyShortCircuitOrOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise AND
///
class WhyBitwiseAndOp : public WhyBinaryOperation
{
public:
	WhyBitwiseAndOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise OR
///
class WhyBitwiseOrOp : public WhyBinaryOperation
{
public:
	WhyBitwiseOrOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise XOR
///
class WhyBitwiseXorOp : public WhyBinaryOperation
{
public:
	WhyBitwiseXorOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise shift left
///
class WhyBitwiseLeftShiftOp : public WhyBinaryOperation
{
public:
	WhyBitwiseLeftShiftOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise arithmetic shift right
///
class WhyBitwiseArithmeticRightShiftOp : public WhyBinaryOperation
{
public:
	WhyBitwiseArithmeticRightShiftOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise logical shift right
///
class WhyBitwiseLogicalRightShiftOp : public WhyBinaryOperation
{
public:
	WhyBitwiseLogicalRightShiftOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs);

	virtual std::string pp();
};

///
/// @brief Represents a function call
///
/// @tparam T Either WhyLValue or WhyRValue, represents whether the function call is an lvalue or an rvalue
///
template <typename T>
class WhyFunctionCall : public T
{
private:
	bool in_vc;
	WhyName name;
	std::vector<std::shared_ptr<WhyExpression>> params;

public:
	WhyFunctionCall(std::shared_ptr<WhyType> type, WhyName name, std::vector<std::shared_ptr<WhyExpression>> params);

	virtual void set_in_vc();

	virtual bool get_in_vc();

	virtual std::string pp();

	virtual std::string pp_in_vc();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();

	virtual void add_param(std::shared_ptr<WhyExpression> param);

	virtual WhyName get_name();
};

template class WhyFunctionCall<WhyRValue>;
template class WhyFunctionCall<WhyLValue>;

///
/// @brief Represents the notion of a constructor call
///
/// Creates the memory and calls the constructor on the newly created memory
///
class WhyConstructor : public WhyRValue
{
private:
	WhyName name;
	std::vector<std::shared_ptr<WhyExpression>> params;

public:
	WhyConstructor(std::shared_ptr<WhyType> type, WhyName name, std::vector<std::shared_ptr<WhyExpression>> params);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();

	virtual WhyName get_name();
};

///
/// @brief Performs lvalue-to-rvalue conversion
///
/// Fetches the memory from the tape based on the address
///
class WhyMemoryGetExpr : public WhyRValue
{
private:
	std::shared_ptr<WhyLValue> base_item;

public:
	WhyMemoryGetExpr(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Gets the address of an lvalue
///
class WhyAddressOfExpr : public WhyRValue
{
private:
	std::shared_ptr<WhyLValue> base_item;

public:
	WhyAddressOfExpr(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Asserts that pointers are valid pointers
///
class WhyValid : public WhyRValue
{
private:
	std::vector<std::shared_ptr<WhyVariable>> vars;
	std::vector<std::shared_ptr<WhyExpression>> base_items;

public:
	WhyValid(std::vector<std::shared_ptr<WhyExpression>> base_items);

	void set_vars(std::vector<std::shared_ptr<WhyVariable>> vars);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Checks if a bit is set
///
class WhyNth : public WhyRValue
{
private:
	std::shared_ptr<WhyRValue> base;
	std::shared_ptr<WhyRValue> idx;

public:
	WhyNth(std::shared_ptr<WhyRValue> base, std::shared_ptr<WhyRValue> idx);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Asserts that the memory pointed to has been freed
///
class WhyFreed : public WhyRValue
{
private:
	std::vector<std::shared_ptr<WhyLValue>> base_items;

public:
	WhyFreed(std::vector<std::shared_ptr<WhyLValue>> base_items);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents the old value of an item
///
class WhyOld : public WhyRValue
{
private:
	std::shared_ptr<WhyRValue> base_item;

public:
	WhyOld(std::shared_ptr<WhyRValue> base_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Asserts that an item is unchanged
///
class WhyUnchanged : public WhyRValue
{
private:
	std::shared_ptr<WhyRValue> base_item;

public:
	WhyUnchanged(std::shared_ptr<WhyRValue> base_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Asserts that two lvalues alias the same value
///
class WhyAliasOf : public WhyRValue
{
private:
	std::shared_ptr<WhyLValue> base_item;
	std::shared_ptr<WhyLValue> aliased_item;

public:
	WhyAliasOf(std::shared_ptr<WhyLValue> base_item, std::shared_ptr<WhyLValue> aliased_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a value at a point in time, based on a given address
///
class WhyAt : public WhyRValue
{
private:
	std::shared_ptr<WhyRValue> base_item;
	std::shared_ptr<WhyLabel> label;

public:
	WhyAt(std::shared_ptr<WhyRValue> base_item, std::shared_ptr<WhyLabel> label);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Asserts that an array is valid
///
class WhyValidArray : public WhyRValue
{
private:
	std::vector<std::shared_ptr<WhyVariable>> vars;
	std::shared_ptr<WhyExpression> base_item;
	std::shared_ptr<WhyRValue> size;

public:
	WhyValidArray(std::shared_ptr<WhyExpression> base_item, std::shared_ptr<WhyRValue> size);

	void set_vars(std::vector<std::shared_ptr<WhyVariable>> vars);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Asserts that a collection of pointers do not alias the same value
///
class WhySeparated : public WhyRValue
{
private:
	std::vector<std::shared_ptr<WhyVariable>> vars;
	std::vector<std::shared_ptr<WhyExpression>> base_items;

public:
	WhySeparated(std::vector<std::shared_ptr<WhyExpression>> base_items);

	void set_vars(std::vector<std::shared_ptr<WhyVariable>> vars);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents the notion of a variable in Why3
///
class WhyVariable : public WhyNode
{
private:
	std::shared_ptr<WhyType> type;
	std::shared_ptr<WhyRValue> literal_init;
	WhyName name;

public:
	WhyVariable(std::shared_ptr<WhyType> type, WhyName name);

	std::shared_ptr<WhyType> get_type();

	WhyName get_name();

	virtual bool get_const();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();

	virtual void add_literal_init(std::shared_ptr<WhyRValue> literal_init);

	virtual std::shared_ptr<WhyRValue> get_literal_init();
};

///
/// @brief Represents a reference to a variable in Why3
///
class WhyVariableReference : public WhyLValue
{
private:
	std::shared_ptr<WhyVariable> variable;
	bool in_quantifier;

public:
	WhyVariableReference(std::shared_ptr<WhyVariable> variable);

	virtual void set_in_quantifier();

	virtual bool get_in_quantifier();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents an index into an array
///
class WhyArrayIndex : public WhyLValue
{
private:
	std::shared_ptr<WhyRValue> base_object;
	std::shared_ptr<WhyRValue> index;

public:
	WhyArrayIndex(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base_object, std::shared_ptr<WhyRValue> index);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a range of indices into an array
///
class WhyArrayRange : public WhyLValue
{
private:
	std::shared_ptr<WhyRValue> base_object;
	std::shared_ptr<WhyRValue> start;
	std::shared_ptr<WhyRValue> end;

public:
	WhyArrayRange(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base_object, std::shared_ptr<WhyRValue> start, std::shared_ptr<WhyRValue> end);

	virtual std::string pp();

	virtual std::string generate_writes();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a pointer dereference
///
class WhyPointerDereference : public WhyLValue
{
private:
	std::shared_ptr<WhyRValue> base_item;

public:
	WhyPointerDereference(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a location where lvalue-to-rvalue conversion should not occur
///
/// See IR::IRReference for more information
///
class WhyReference : public WhyLValue
{
private:
	std::shared_ptr<WhyLValue> base_item;

public:
	WhyReference(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a reference into one of an object's fields
///
class WhyFieldReference : public WhyLValue
{
private:
	std::shared_ptr<WhyLValue> base_item;
	int offset;

public:
	WhyFieldReference(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base_item, int offset);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a reference into one of an object's fields
///
/// This version is used when the base_item is an rvalue.
///
class WhyFieldReferenceRValue : public WhyRValue
{
private:
	std::shared_ptr<WhyRValue> base_item;
	int offset;

public:
	WhyFieldReferenceRValue(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base_item, int offset);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Base class for statements, program code that does not return a value
///
/// All statements have an optional continuation. This is code that should execute
/// after the current statement executes.
///
class WhyStatement : public WhyNode
{
protected:
	std::shared_ptr<WhyStatement> continuation;

public:
	WhyStatement();

	void set_continuation(std::shared_ptr<WhyStatement> continuation);

	virtual std::string pp_continuation();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a collection of statements.
///
/// Even though a statement has continuations, this represents a collection of statements
/// wrapped in parenthesis `()`, giving a notion of scoping.
///
class WhyStatementCollection : public WhyStatement
{
private:
	std::shared_ptr<WhyStatement> collection;

public:
	WhyStatementCollection(std::shared_ptr<WhyStatement> collection);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

class WhyVerificationCondition;

///
/// @brief Represents a `while` loop in Why3
///
class WhyLoop : public WhyStatement
{
private:
	std::shared_ptr<WhyRValue> cond;
	std::shared_ptr<WhyStatement> body;
	std::vector<std::shared_ptr<WhyVerificationCondition>> vcs;
	std::vector<std::shared_ptr<WhyVariable>> vars;

public:
	WhyLoop(std::shared_ptr<WhyRValue> cond, std::shared_ptr<WhyStatement> body, std::vector<std::shared_ptr<WhyVerificationCondition>> vcs, std::vector<std::shared_ptr<WhyVariable>> vars);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();

	virtual std::vector<std::shared_ptr<WhyVariable>> get_vars();
};

///
/// @brief Represents an `if` statement in Why3
///
class WhyIf : public WhyStatement
{
private:
	std::shared_ptr<WhyRValue> cond;
	std::shared_ptr<WhyStatement> then;
	std::shared_ptr<WhyStatement> els;

public:
	WhyIf(std::shared_ptr<WhyRValue> cond, std::shared_ptr<WhyStatement> then, std::shared_ptr<WhyStatement> els);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a `label` statement in Why3
///
class WhyLabelStmt : public WhyStatement
{
private:
	std::shared_ptr<WhyLabel> label;

public:
	WhyLabelStmt(std::shared_ptr<WhyLabel> label);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents an `assert` statement in Why3
///
class WhyAssert : public WhyStatement
{
protected:
	std::shared_ptr<WhyRValue> assertion;
	std::string debug_str;

public:
	WhyAssert(std::shared_ptr<WhyRValue> assertion, std::string debug_str);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents an `assume` statement in Why3
///
class WhyAssume : public WhyAssert
{
public:
	WhyAssume(std::shared_ptr<WhyRValue> assumption);

	virtual std::string pp();
};

///
/// @brief Represents an expression whose result is discarded
///
/// This transforms expressions into statements
///
class WhyDiscardResultStmt : public WhyStatement
{
private:
	std::shared_ptr<WhyExpression> base_item;

public:
	WhyDiscardResultStmt(std::shared_ptr<WhyExpression> base_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents an empty statement
///
class WhyEmptyStmt : public WhyStatement
{
public:
	WhyEmptyStmt();

	virtual std::string pp();
};

///
/// @brief Represents an empty expression
///
class WhyEmptyExpr : public WhyRValue
{
public:
	WhyEmptyExpr();

	virtual std::string pp();
};

///
/// @brief Represents a variable declaration with `let`
///
class WhyVariableDecl : public WhyStatement
{
private:
	std::shared_ptr<WhyVariable> var;
	std::shared_ptr<WhyExpression> initial_value;

public:
	WhyVariableDecl(std::shared_ptr<WhyVariable> var, std::shared_ptr<WhyExpression> initial_value);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();

	virtual std::shared_ptr<WhyVariable> get_variable();
};

///
/// @brief Represents a global variable declaration with `val`
///
class WhyGlobalVariableDecl : public WhyStatement
{
private:
	std::shared_ptr<WhyVariable> var;

public:
	WhyGlobalVariableDecl(std::shared_ptr<WhyVariable> var);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();

	virtual std::shared_ptr<WhyVariable> get_variable();
};

///
/// @brief Represents a case-like construct
///
class WhyCase : public WhyStatement
{
private:
	std::shared_ptr<WhyRValue> expr;

public:
	WhyCase(std::shared_ptr<WhyRValue> expr);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a break statement
///
class WhyBreak : public WhyStatement
{
public:
	WhyBreak();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a continue statement
///
class WhyContinue : public WhyStatement
{
public:
	WhyContinue();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a switch-like construct
///
class WhySwitch : public WhyStatement
{
private:
	std::shared_ptr<WhyRValue> expr;

	std::vector<std::shared_ptr<WhyCase>> cases;

public:
	WhySwitch(std::shared_ptr<WhyRValue> expr, std::vector<std::shared_ptr<WhyCase>> cases);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a return from a function
///
class WhyReturn : public WhyStatement
{
private:
	std::shared_ptr<WhyExpression> base_item;

public:
	WhyReturn(std::shared_ptr<WhyExpression> base_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a lemma in Why3
///
class WhyLemma : public WhyNode
{
protected:
	std::shared_ptr<WhyRValue> vc;
	WhyName name;

public:
	WhyLemma(WhyName name, std::shared_ptr<WhyRValue> vc);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents an axiom in Why3
///
class WhyAxiom : public WhyLemma
{
public:
	WhyAxiom(WhyName name, std::shared_ptr<WhyRValue> vc);

	virtual std::string pp();
};

///
/// @brief Base class for verification conditions
///
class WhyVerificationCondition : public WhyNode
{
protected:
	std::shared_ptr<WhyRValue> vc;
	std::string debug_str;	

public:
	WhyVerificationCondition(std::shared_ptr<WhyRValue> vc, std::string debug_str);

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents an `ensures` clause in Why3
///
class WhyEnsures : public WhyVerificationCondition
{
public:
	WhyEnsures(std::shared_ptr<WhyRValue> vc, std::string debug_str);

	virtual std::string pp();
};

///
/// @brief Represents a `requires` clause in Why3
///
class WhyRequires : public WhyVerificationCondition
{
public:
	WhyRequires(std::shared_ptr<WhyRValue> vc, std::string debug_str);

	virtual std::string pp();
};

///
/// @brief Represents an `invariant` clause in Why3
///
class WhyInvariant : public WhyVerificationCondition
{
public:
	WhyInvariant(std::shared_ptr<WhyRValue> vc, std::string debug_str);

	virtual std::string pp();
};

///
/// @brief Represents a `variant` clause in Why3
///
class WhyVariant : public WhyVerificationCondition
{
public:
	WhyVariant(std::shared_ptr<WhyRValue> vc, std::string debug_str);

	virtual std::string pp();
};

///
/// @brief Represents a `writes` clause to be generated
///
class WhyWrites : public WhyVerificationCondition
{
private:
	std::vector<std::shared_ptr<WhyLValue>> vars;
	bool in_loop;

public:
	WhyWrites(std::vector<std::shared_ptr<WhyLValue>> vars, std::string debug_str);

	virtual void set_in_loop();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a `frees` clause to be generated
///
class WhyFrees : public WhyVerificationCondition
{
private:
	std::vector<std::shared_ptr<WhyLValue>> vars;
	bool in_loop;

public:
	WhyFrees(std::vector<std::shared_ptr<WhyLValue>> vars, std::string debug_str);

	virtual void set_in_loop();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents the notion of an empty node
///
class WhyEmptyNode : public WhyNode
{
public:
	WhyEmptyNode();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

///
/// @brief Represents a function declaration with `let`
///
class WhyFunction : public WhyNode
{
private:
	std::vector<std::shared_ptr<WhyVariable>> vars;
	std::vector<std::shared_ptr<WhyVariable>> globals;
	std::vector<std::shared_ptr<WhyVerificationCondition>> vcs;
	WhyName name;
	std::shared_ptr<WhyType> return_type;
	std::shared_ptr<WhyStatement> body;
	bool is_ref_function;
	bool is_mutually_recursive;
	std::string ir_name;

public:
	WhyFunction(std::shared_ptr<WhyType> return_type, std::string name, std::vector<std::shared_ptr<WhyVariable>> vars, std::vector<std::shared_ptr<WhyVariable>> globals, std::vector<std::shared_ptr<WhyVerificationCondition>> vcs, std::shared_ptr<WhyStatement> body, bool is_ref_function);

	std::vector<std::shared_ptr<WhyVariable>> get_vars();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();

	virtual WhyName get_name();

	virtual std::string get_ir_name();

	virtual void set_mutual();
};

///
/// @brief Represents a program in Why3
///
class WhyProgram : public WhyNode
{
private:
	std::vector<std::shared_ptr<WhyGlobalVariableDecl>> vars;
	std::vector<std::shared_ptr<WhyFunction>> funcs;
	std::vector<std::shared_ptr<WhyLemma>> lemmas;

public:
	WhyProgram(std::vector<std::shared_ptr<WhyGlobalVariableDecl>> vars, std::vector<std::shared_ptr<WhyFunction>> funcs, std::vector<std::shared_ptr<WhyLemma>> lemmas);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<WhyFunction>> get_funcs();

	virtual std::vector<std::shared_ptr<WhyGlobalVariableDecl>> get_vars();

	virtual std::vector<std::shared_ptr<WhyLemma>> get_lemmas();

	virtual std::vector<std::shared_ptr<WhyNode>> traverse();
};

}

#endif
