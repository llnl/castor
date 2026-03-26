// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <string>
#include <vector>
#include <memory>

#ifndef IR_AST
#define IR_AST

using IRName = std::string;

class R2WML_Scope; //include for handling of scopes

///
/// @brief This is the namespace containing all the IR nodes
///
namespace IR
{

///
/// @brief Base class of all IR nodes
///
/// All IR nodes must inherit from this class
///
class IRNode
{
public:
	///
	/// @brief Constructor
	///
	IRNode();

	///
	/// @brief Pretty-printer
	///
	/// @return This node as a pretty-printed string
	///
	virtual std::string pp() = 0;

	///
	/// @brief Provide child classes for traversals
	///
	/// When traversing over the IR AST, these functions are called to get a
	/// list of child IR nodes that should be traversed before traversing
	/// this node.
	///
	/// @return A vector of IRNodes that are children of this class
	///
	virtual std::vector<std::shared_ptr<IRNode>> traverse() = 0;
};

///
/// @brief Base class for all IR types
///
class IRType : public IRNode
{
protected:
	bool is_reference;

public:
	IRType();

	virtual void set_is_reference();
	
	virtual bool get_is_reference();

	virtual std::string pp() = 0;

	virtual size_t get_sizeof() = 0;

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Type of label references
///
class IRLabelType : public IRType
{
public:
	IRLabelType();

	virtual std::string pp();

	virtual size_t get_sizeof();
};

///
/// @brief Type of void
///
class IRVoidType : public IRType
{
public:
	IRVoidType();

	virtual std::string pp();

	virtual size_t get_sizeof();
};

///
/// @brief Type of bool
///
class IRBoolType : public IRType
{
public:
	IRBoolType();

	virtual std::string pp();

	virtual size_t get_sizeof();
};

///
/// @brief Type of uninstantiated template types
///
class IRNonRealType : public IRType
{
public:
	IRNonRealType();

	virtual std::string pp();

	virtual size_t get_sizeof();
};

///
/// @brief Used when type is unknown
///
class IRUnknownType : public IRType
{
public:
	IRUnknownType();

	virtual std::string pp();

	virtual size_t get_sizeof();
};

///
/// @brief Type of arrays
///
class IRArrayType : public IRType
{
private:
	std::shared_ptr<IRType> base_type;
	int size;

public:
	IRArrayType(std::shared_ptr<IRType> base_type, int size);

	virtual std::shared_ptr<IRType> get_base_type();

	virtual int get_size();

	virtual std::string pp();

	virtual size_t get_sizeof();
};

class IRClass;

///
/// @brief Used as a catch-all for object types
///
class IRClassType : public IRType
{
private:
	IRName class_name;

	std::shared_ptr<IRClass> class_definition;

public:
	IRClassType(IRName class_name);

	virtual IRName get_class_name();

	virtual std::shared_ptr<IRClass> get_class_definition();

	virtual void set_class_definition(std::shared_ptr<IRClass> clas);

	virtual std::string pp();

	virtual size_t get_sizeof();
};

///
/// @brief Integral types
///
class IRIntegralType : public IRType
{
private:
	bool is_signed;
	int bits;

public:
	IRIntegralType(int bits, bool is_signed);

	virtual std::string pp();

	bool get_is_signed();

	int get_bits();

	virtual size_t get_sizeof();
};

///
/// @brief Unbounded integers (i.e. no minimum or maximum value)
///
class IRUnboundedIntegralType : public IRIntegralType
{
public:
	IRUnboundedIntegralType();
};

///
/// @brief Signed 64-bit integers
///
class IRS64Type : public IRIntegralType
{
public:
	IRS64Type();
};

///
/// @brief Signed 32-bit integers
///
class IRS32Type : public IRIntegralType
{
public:
	IRS32Type();
};

///
/// @brief Signed 16-bit integers
///
class IRS16Type : public IRIntegralType
{
public:
	IRS16Type();
};

///
/// @brief Signed 8-bit integers
///
class IRS8Type : public IRIntegralType
{
public:
	IRS8Type();
};

///
/// @brief Unsigned 64-bit integers
///
class IRU64Type : public IRIntegralType
{
public:
	IRU64Type();
};

///
/// @brief Unsigned 32-bit integers
///
class IRU32Type : public IRIntegralType
{
public:
	IRU32Type();
};

///
/// @brief Unsigned 16-bit integers
///
class IRU16Type : public IRIntegralType
{
public:
	IRU16Type();
};

///
/// @brief Unsigned 8-bit integers
///
class IRU8Type : public IRIntegralType
{
public:
	IRU8Type();
};

///
/// @brief Pointer types
///
/// Contains a pointer to the base type
///
class IRPointerType : public IRIntegralType
{
private:
	std::shared_ptr<IRType> base_type;
public:
	IRPointerType(std::shared_ptr<IRType> base_type);

	virtual std::string pp();

	std::shared_ptr<IRType> get_base_type();
};

///
/// @brief Constant types
///
/// Contains a pointer to the base type
///
class IRConstType : public IRType
{
private:
	std::shared_ptr<IRType> base_type;
public:
	IRConstType(std::shared_ptr<IRType> base_type);

	virtual std::string pp();

	std::shared_ptr<IRType> get_base_type();

	virtual size_t get_sizeof();
};

class IRExpression;

///
/// @brief A variable in the IR
///
class IRVariable : public IRNode
{
private:
	std::shared_ptr<IRType> type;
	IRName name;
	IRName frontend_name;
	std::shared_ptr<IRExpression> constexpr_value;
	bool is_program_variable;

public:
	IRVariable(std::shared_ptr<IRType> type, IRName name, IRName frontend_name, bool is_program_variable);

	IRVariable(std::shared_ptr<IRType> type, IRName name, IRName frontend_name, std::shared_ptr<IRExpression> constexpr_value, bool is_program_variable);

	virtual std::string pp();

	virtual std::shared_ptr<IRType> get_type();

	virtual IRName get_name();

	virtual IRName get_frontend_name();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual bool has_constexpr_value();

	virtual std::shared_ptr<IRExpression> get_constexpr_value();

	virtual bool get_is_program_variable();
};

///
/// @brief Base abstract class for expressions (things that return a value)
///
/// All expressions have an associated type
///
class IRExpression : public IRNode
{
protected:
	std::shared_ptr<IRType> type;

public:
	IRExpression(std::shared_ptr<IRType> type);

	virtual std::string pp() = 0;

	virtual std::shared_ptr<IRType> get_type();

	virtual void set_type(std::shared_ptr<IRType> type);
};

///
/// @brief Lvalue expressions
///
class IRLValue : public IRExpression
{
public:
	IRLValue(std::shared_ptr<IRType> type);
};

///
/// @brief Rvalue expressions
///
class IRRValue : public IRExpression
{
public:
	IRRValue(std::shared_ptr<IRType> type);
};

///
/// @brief Represents temporary value materialization
///
class IRMaterialize : public IRLValue
{
private:
	std::shared_ptr<IRRValue> base_expr;

public:
	IRMaterialize(std::shared_ptr<IRRValue> base_expr);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Ternary expressions (expressions of the form `a ? b : c`)
///
/// @tparam T Either IRLValue or IRRValue
///
template <typename T>
class IRTernary : public T
{
private:
	std::shared_ptr<IRExpression> condition;
	std::shared_ptr<IRExpression> then;
	std::shared_ptr<IRExpression> els;

public:
	IRTernary(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> condition, std::shared_ptr<IRExpression> then, std::shared_ptr<IRExpression> els);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

template class IRTernary<IRLValue>;
template class IRTernary<IRRValue>;

///
/// @brief Label references
///
class IRLabelReference : public IRRValue
{
private:
	IRName name;

public:
	IRLabelReference(IRName name);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual IRName get_name();
};

///
/// @brief `sizeof` expressions
///
class IRSizeOf : public IRRValue
{
private:
	std::shared_ptr<IRType> contained_type;

public:
	IRSizeOf(std::shared_ptr<IRType> type);

	virtual std::string pp();

	virtual std::shared_ptr<IRType> get_contained_type();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Represents the concept of `null`
///
class IRNull : public IRRValue
{
public:
	IRNull();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Represents C-style casts (i.e. `(type)expression`)
///
template <typename T>
class IRCast : public T
{
private:
	std::shared_ptr<IRExpression> base_expr;

public:
	IRCast(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_expr);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::shared_ptr<IRExpression> get_base_expr();
};

///
/// @brief Class field references (i.e. `object.dataMember`)
///
class IRFieldReference : public IRLValue
{
private:
	std::shared_ptr<IRExpression> base_object;
	IRName field_name;

public:
	IRFieldReference(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_object, IRName field_name);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::shared_ptr<IRExpression> get_base_object();

	virtual IRName get_field_name();

	virtual void set_field_name(IRName name);
};

///
/// @brief Indexing an array (i.e. `array[index]`)
///
class IRArrayIndex : public IRLValue
{
private:
	std::shared_ptr<IRLValue> array;
	std::shared_ptr<IRExpression> index;

public:
	IRArrayIndex(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> array, std::shared_ptr<IRExpression> index);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Ranged array indexing (i.e. `array[start .. end]`)
///
class IRArrayRangeIndex : public IRRValue
{
private:
	std::shared_ptr<IRLValue> array;
	std::shared_ptr<IRExpression> begin;
	std::shared_ptr<IRExpression> end;

public:
	IRArrayRangeIndex(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> array, std::shared_ptr<IRExpression> begin, std::shared_ptr<IRExpression> end);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Represents a reference to a variable
///
class IRVariableReference : public IRLValue
{
private:
	std::shared_ptr<IRVariable> variable;

public:
	IRVariableReference(std::shared_ptr<IRVariable> variable);

	virtual std::string pp();

	virtual std::string get_name();

	virtual std::shared_ptr<IRVariable> get_variable();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual void set_var(std::shared_ptr<IRVariable> var);
};

///
/// @brief Pointer dereferencing (i.e. `*ptr`)
///
class IRPointerDereference : public IRLValue
{
private:
	std::shared_ptr<IRExpression> base_item;

public:
	IRPointerDereference(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Prevents lvalue-to-rvalue conversion
///
/// This is used in cases where an expression is being aliased by a C++ reference.
/// Usually in these cases, Castor would perform lvalue-to-rvalue conversion, but this
/// node indicates that no such conversion should take place.
///
class IRReference : public IRLValue
{
private:
	std::shared_ptr<IRLValue> base_item;

public:
	IRReference(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> base_item);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Base class for unary operations
///
class IRUnaryOperation : public IRRValue
{
protected:
	std::shared_ptr<IRExpression> base_item;

public:
	IRUnaryOperation(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item);

	virtual std::shared_ptr<IRExpression> get_base_item();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Postifx increment (i.e. `a++`)
///
class IRIncrementOp : public IRUnaryOperation
{
public:
	IRIncrementOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item);

	virtual std::string pp();
};

///
/// @brief Postifx decrement (i.e. `a--`)
///
class IRDecrementOp : public IRUnaryOperation
{
public:
	IRDecrementOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item);

	virtual std::string pp();
};

///
/// @brief Negation (i.e. `-a`)
///
class IRNegationOp : public IRUnaryOperation
{
public:
	IRNegationOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item);

	virtual std::string pp();
};

///
/// @brief Boolean negation (i.e. `!a`)
///
class IRBoolNegationOp : public IRUnaryOperation
{
public:
	IRBoolNegationOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item);

	virtual std::string pp();
};

///
/// @brief Bit complement (i.e. `~a`)
///
class IRBitNegationOp : public IRUnaryOperation
{
public:
	IRBitNegationOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item);

	virtual std::string pp();
};

///
/// @brief Base class for binary operations
///
class IRBinaryOperation : public IRRValue
{
protected:
	std::shared_ptr<IRExpression> lhs;
	std::shared_ptr<IRExpression> rhs;

public:
	IRBinaryOperation(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp() = 0;

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::shared_ptr<IRExpression> get_lhs();
	
	virtual std::shared_ptr<IRExpression> get_rhs();
};

///
/// @brief Base class for assignment binary operations
///
class IRAssignmentOperation : public IRLValue
{
protected:
	std::shared_ptr<IRLValue> lhs;
	std::shared_ptr<IRExpression> rhs;

public:
	IRAssignmentOperation(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp() = 0;

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::shared_ptr<IRLValue> get_lhs();

	virtual std::shared_ptr<IRExpression> get_rhs();
};

///
/// @brief Assignment (i.e. `a = b`)
///
class IRAssignOp : public IRAssignmentOperation
{
public:
	IRAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Comma operator (i.e. `a, b`)
///
/// @tparam T Either IRLValue or IRRValue, depending on the value category of this expression
///
template <typename T>
class IRCommaOp : public T
{
private:
	std::shared_ptr<IRExpression> lhs;
	std::shared_ptr<IRExpression> rhs;

public:
	IRCommaOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Greater than or equal to (i.e. `a >= b`)
///
class IRGreaterEqualsOp : public IRBinaryOperation
{
public:
	IRGreaterEqualsOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Greater than (i.e. `a > b`)
///
class IRGreaterThanOp : public IRBinaryOperation
{
public:
	IRGreaterThanOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Less than or equal to (i.e. `a <= b`)
///
class IRLessEqualsOp : public IRBinaryOperation
{
public:
	IRLessEqualsOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Less than (i.e. `a < b`)
///
class IRLessThanOp : public IRBinaryOperation
{
public:
	IRLessThanOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Division (i.e. `a / b`)
///
class IRDivideOp : public IRBinaryOperation
{
public:
	IRDivideOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Division-assignment (i.e. `a /= b`)
///
class IRDivideAssignOp : public IRAssignmentOperation
{
public:
	IRDivideAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Modulo (i.e. `a % b`)
///
class IRModuloOp : public IRBinaryOperation
{
public:
	IRModuloOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Modulo-assignment (i.e. `a %= b`)
///
class IRModuloAssignOp : public IRAssignmentOperation
{
public:
	IRModuloAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Multiplication (i.e. `a * b`)
///
class IRMultiplyOp : public IRBinaryOperation
{
public:
	IRMultiplyOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Multiplication-assignment (i.e. `a *= b`)
///
class IRMultiplyAssignOp : public IRAssignmentOperation
{
public:
	IRMultiplyAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Not equal to (i.e. `a != b`)
///
class IRNotEqualsOp : public IRBinaryOperation
{
public:
	IRNotEqualsOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Equal to (i.e. `a == b`)
///
class IREqualsOp : public IRBinaryOperation
{
public:
	IREqualsOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Bit shift right (i.e. `a >> b`)
///
class IRBitRShiftOp : public IRBinaryOperation
{
public:
	IRBitRShiftOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Bit shift right-assignment (i.e. `a >>= b`)
///
class IRBitRShiftAssignOp : public IRAssignmentOperation
{
public:
	IRBitRShiftAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Bit shift left (i.e. `a << b`)
///
class IRBitLShiftOp : public IRBinaryOperation
{
public:
	IRBitLShiftOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Bit shift left-assignment (i.e. `a <<= b`)
///
class IRBitLShiftAssignOp : public IRAssignmentOperation
{
public:
	IRBitLShiftAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise OR (i.e. `a | b`)
///
class IRBitOrOp : public IRBinaryOperation
{
public:
	IRBitOrOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise OR-assignment (i.e. `a |= b`)
///
class IRBitOrAssignOp : public IRAssignmentOperation
{
public:
	IRBitOrAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise XOR (i.e. `a ^ b`)
///
class IRBitXorOp : public IRBinaryOperation
{
public:
	IRBitXorOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise XOR-assignment (i.e. `a ^= b`)
///
class IRBitXorAssignOp : public IRAssignmentOperation
{
public:
	IRBitXorAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise AND (i.e. `a & b`)
///
class IRBitAndOp : public IRBinaryOperation
{
public:
	IRBitAndOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Bitwise AND-assignment (i.e. `a &= b`)
///
class IRBitAndAssignOp : public IRAssignmentOperation
{
public:
	IRBitAndAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Subtraction (i.e. `a - b`)
///
class IRSubtractionOp : public IRBinaryOperation
{
public:
	IRSubtractionOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Subtraction-assignment (i.e. `a -= b`)
///
class IRSubtractionAssignOp : public IRAssignmentOperation
{
public:
	IRSubtractionAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Addition (i.e. `a + b`)
///
class IRAdditionOp : public IRBinaryOperation
{
public:
	IRAdditionOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Addition-assignment (i.e. `a += b`)
///
class IRAdditionAssignOp : public IRAssignmentOperation
{
public:
	IRAdditionAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Logical conjunction (i.e. `a && b`)
///
class IRAndOp : public IRBinaryOperation
{
public:
	IRAndOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Logical disjunction (i.e. `a || b`)
///
class IROrOp : public IRBinaryOperation
{
public:
	IROrOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Logical implication (i.e. `a -> b`)
///
class IRImpliesOp : public IRBinaryOperation
{
public:
	IRImpliesOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs);

	virtual std::string pp();
};

///
/// @brief Represents a nullptr expression
///
class IRNullptr : public IRRValue
{
public:
	IRNullptr();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Base class for literal values
///
class IRLiteral : public IRRValue
{
public:
	IRLiteral(std::shared_ptr<IRType> type);

	std::string pp() = 0;

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Boolean literal
///
class IRBoolLiteral : public IRLiteral
{
private:
	bool value;

public:
	IRBoolLiteral(bool value);

	std::string pp();

	bool get_value();
};

///
/// @brief Unsigned 8-bit integer literal
///
class IRU8Literal : public IRLiteral
{
private:
	uint8_t value;

public:
	IRU8Literal(uint8_t value);

	std::string pp();

	uint8_t get_value();
};

///
/// @brief Signed 8-bit integer literal
///
class IRS8Literal : public IRLiteral
{
private:
	int8_t value;

public:
	IRS8Literal(int8_t value);

	std::string pp();

	int8_t get_value();
};

///
/// @brief Unsigned 16-bit integer literal
///
class IRU16Literal : public IRLiteral
{
private:
	uint16_t value;

public:
	IRU16Literal(uint16_t value);

	std::string pp();

	uint16_t get_value();
};

///
/// @brief Signed 16-bit integer literal
///
class IRS16Literal : public IRLiteral
{
private:
	int16_t value;

public:
	IRS16Literal(int16_t value);

	std::string pp();

	int16_t get_value();
};

///
/// @brief Unsigned 32-bit integer literal
///
class IRU32Literal : public IRLiteral
{
private:
	uint32_t value;

public:
	IRU32Literal(uint32_t value);

	std::string pp();

	uint32_t get_value();
};

///
/// @brief Signed 32-bit integer literal
///
class IRS32Literal : public IRLiteral
{
private:
	int32_t value;

public:
	IRS32Literal(int32_t value);

	std::string pp();

	int32_t get_value();
};

///
/// @brief Unsigned 64-bit integer literal
///
class IRU64Literal : public IRLiteral
{
private:
	uint64_t value;

public:
	IRU64Literal(uint64_t value);

	std::string pp();

	uint64_t get_value();
};

///
/// @brief Signed 64-bit integer literal
///
class IRS64Literal : public IRLiteral
{
private:
	int64_t value;

public:
	IRS64Literal(int64_t value);

	std::string pp();

	int64_t get_value();
};

///
/// @brief Base class for quantifiers (forall, exists)
///
class IRQuantifier : public IRRValue
{
protected:
	std::vector<std::shared_ptr<IRVariable>> variables;
	std::shared_ptr<IRExpression> expression;

public:
	IRQuantifier(std::vector<std::shared_ptr<IRVariable>> variables, std::shared_ptr<IRExpression> expression);

	virtual std::string pp() = 0;

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::vector<std::shared_ptr<IRVariable>> get_vars();
};

///
/// @brief Universal quantification (forall)
///
class IRForall : public IRQuantifier
{
public:
	IRForall(std::vector<std::shared_ptr<IRVariable>> variables, std::shared_ptr<IRExpression> expression);

	virtual std::string pp();
};

///
/// @brief Existential quantification (exists)
///
class IRExists : public IRQuantifier
{
public:
	IRExists(std::vector<std::shared_ptr<IRVariable>> variables, std::shared_ptr<IRExpression> expression);

	virtual std::string pp();
};

///
/// @brief Base class for statements (program code that does not return a value)
///
class IRStatement : public IRNode
{
public:
	IRStatement();

	virtual std::string pp() = 0;
};

///
/// @brief Represents a statement or statement block of ghost code
///
class IRGhostStatement : public IRStatement
{
private:
	std::shared_ptr<IRStatement> statement;

public:
	IRGhostStatement();

	virtual void attach_stmt(std::shared_ptr<IRStatement> statement);

	virtual std::shared_ptr<IRStatement> get_stmt();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Empty statement, can be used as a placeholder
///
class IREmptyStatement : public IRStatement
{
public:
	IREmptyStatement();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Labels (i.e. `label:`)
///
class IRLabel : public IRStatement
{
private:
	std::shared_ptr<IRLabelReference> label;

public:
	IRLabel(std::shared_ptr<IRLabelReference> label);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Wrapper class for labels
///
/// Labels in the SAGE tree have an associated statement.
/// This packages up a label and its associated statement for decomposition later.
///
class __IRLabelWrapper : public IRStatement
{
public:
	std::shared_ptr<IRLabel> label;
	std::shared_ptr<IRStatement> statement;

	__IRLabelWrapper(std::shared_ptr<IRLabel> label, std::shared_ptr<IRStatement> statement);

	std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Represents the notion of a statement that carries an associated scope
///
class IRScopedStatement : public IRStatement
{
private:
	std::shared_ptr<R2WML_Scope> scope;

public:
	IRScopedStatement(std::shared_ptr<R2WML_Scope> scope);

	virtual std::shared_ptr<R2WML_Scope> get_scope();
};

///
/// @brief If statement
///
class IRIfStatement : public IRStatement
{
private:
	std::shared_ptr<IRExpression> cond;
	std::shared_ptr<IRStatement> then;
	std::shared_ptr<IRStatement> els;

public:
	IRIfStatement(std::shared_ptr<IRExpression> cond, std::shared_ptr<IRStatement> then, std::shared_ptr<IRStatement> els);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Verification condition
///
class IRVerificationCondition : public IRStatement
{
protected:
	std::string ver;
	std::string debug_str;

public:
	IRVerificationCondition();

	void attach_str(std::string ver);

	std::string get_str();

	virtual std::string pp() = 0;

	virtual void set_debug_str(std::string debug_str);

	virtual std::string get_debug_str();
};

///
/// @brief `lemma` clause
///
class IRLemma : public IRVerificationCondition
{
protected:
	IRName name;	
	std::shared_ptr<IRExpression> vc;

public:
	IRLemma(IRName name, std::shared_ptr<IRExpression> vc);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual IRName get_name();

	virtual std::shared_ptr<IRExpression> get_vc();
};

///
/// @brief `axiom` clause
///
class IRAxiom : public IRLemma
{
public:
	IRAxiom(IRName name, std::shared_ptr<IRExpression> vc);

	virtual std::string pp();
};

///
/// @brief `ensures` clause
///
class IREnsures : public IRVerificationCondition
{
private:
	std::shared_ptr<IRExpression> vc;

public:
	IREnsures(std::shared_ptr<IRExpression> vc);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief `requires` clause
///
class IRRequires : public IRVerificationCondition
{
private:
	std::shared_ptr<IRExpression> vc;

public:
	IRRequires(std::shared_ptr<IRExpression> vc);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief `invariant` clause
///
class IRLoopInvariant : public IRVerificationCondition
{
private:
	std::shared_ptr<IRExpression> vc;

public:
	IRLoopInvariant(std::shared_ptr<IRExpression> vc);

	virtual std::string pp();

	virtual std::shared_ptr<IRExpression> get_vc();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief `variant` clause
///
class IRLoopVariant : public IRVerificationCondition
{
private:
	std::shared_ptr<IRExpression> vc;

public:
	IRLoopVariant(std::shared_ptr<IRExpression> vc);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief `writes` clause
///
class IRWrites : public IRVerificationCondition
{
private:
	std::vector<std::shared_ptr<IRExpression>> vars;

public:
	IRWrites(std::vector<std::shared_ptr<IRExpression>> vars);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief `frees` clause
///
class IRFrees : public IRVerificationCondition
{
private:
	std::vector<std::shared_ptr<IRExpression>> vars;

public:
	IRFrees(std::vector<std::shared_ptr<IRExpression>> vars);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief `assert` statement (as a verification condition, not a C++ `assert`)
///
class IRAssert : public IRStatement
{
protected:
	std::shared_ptr<IRExpression> vc;
	std::string debug_str;
	std::string ver;

public:
	IRAssert(std::shared_ptr<IRExpression> vc);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual void attach_str(std::string ver);

	virtual std::string get_str();

	virtual void set_debug_str(std::string debug_str);

	virtual std::string get_debug_str();
};

///
/// @brief `assume` statement
///
class IRAssume : public IRAssert
{
public:
	IRAssume(std::shared_ptr<IRExpression> vc);

	virtual std::string pp();
};

///
/// @brief Function reference expression
///
class IRFunctionRefExpr : public IRRValue
{
private:
	IRName function_name;

public:
	IRFunctionRefExpr(IRName function_name);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::string get_name();
};

///
/// @brief Wrapper class for a list of expressions
///
class __IRExpressionListWrapper : public IRNode
{
public:
	std::vector<std::shared_ptr<IRExpression>> params;

	std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Represents a call to a function (i.e. `foo(...)`)
///
/// @tparam T Either IRRValue or IRLValue, represents whether the function call is an rvalue or an lvalue
///
template <typename T>
class IRFunctionCallExpr : public T
{
private:
	bool is_constructor;
	std::shared_ptr<IRFunctionRefExpr> function;
	std::vector<std::shared_ptr<IRExpression>> params;

public:
	IRFunctionCallExpr(std::shared_ptr<IRType> type, std::shared_ptr<IRFunctionRefExpr> function, std::vector<std::shared_ptr<IRExpression>> params);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::string get_function_name();

	virtual std::vector<std::shared_ptr<IRExpression>> get_params();

	virtual void set_function(std::shared_ptr<IRFunctionRefExpr> function);

	virtual void set_params(std::vector<std::shared_ptr<IRExpression>> params);

	virtual void set_is_constructor();

	virtual bool get_is_constructor();
};

template class IRFunctionCallExpr<IRRValue>;
template class IRFunctionCallExpr<IRLValue>;

///
/// @brief Represents a call to a member function (i.e. `obj.foo(...)`)
///
/// @tparam T Either IRRValue or IRLValue, represents whether the function call is an rvalue or an lvalue
///
template <typename T>
class IRMemberFunctionCallExpr : public T
{
private:
	std::shared_ptr<IRExpression> base_object;
	std::shared_ptr<IRFunctionRefExpr> function;
	std::vector<std::shared_ptr<IRExpression>> params;

public:
	IRMemberFunctionCallExpr(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_object, std::shared_ptr<IRFunctionRefExpr> function, std::vector<std::shared_ptr<IRExpression>> params);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::string get_function_name();

	virtual std::vector<std::shared_ptr<IRExpression>> get_params();

	virtual void set_params(std::vector<std::shared_ptr<IRExpression>> params);
};

template class IRMemberFunctionCallExpr<IRRValue>;
template class IRMemberFunctionCallExpr<IRLValue>;

#ifdef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
///
/// @brief Dynamically allocates memory (analogous to C++ `new`)
///
class IRNewExpr : public IRRValue
{
private:
	std::shared_ptr<IRExpression> base_object;

public:
	IRNewExpr(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_object);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Dynamically frees memory (analogous to C++ `delete`)
///
class IRDeleteExpr : public IRRValue
{
private:
	std::shared_ptr<IRLValue> base_object;

public:
	IRDeleteExpr(std::shared_ptr<IRLValue> base_object);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};
#endif

///
/// @brief Represents a variable declaration (i.e. `type var (= init)?`)
class IRVariableDeclarationStmt : public IRStatement
{
private:
	std::shared_ptr<IRExpression> initial_value;
	std::shared_ptr<IRVariable> var;
	bool is_static;

public:
	IRVariableDeclarationStmt(std::shared_ptr<IRVariable> var, std::shared_ptr<IRExpression> initial_value, bool is_static);

	virtual std::string pp();

	virtual std::shared_ptr<IRVariable> get_var();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::shared_ptr<IRExpression> get_initial_value();

	virtual bool get_is_static();
};

///
/// @brief Represents a `return` statement
///
class IRReturnStmt : public IRStatement
{
private:
	std::shared_ptr<IRExpression> ret;

public:
	IRReturnStmt(std::shared_ptr<IRExpression> ret);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::shared_ptr<IRExpression> get_stmt();

	virtual void set_stmt(std::shared_ptr<IRExpression> ret);
};

///
/// @brief Represents a block of statements (statements collected via curly braces)
///
class IRStatementBlock : public IRScopedStatement
{
private:
	std::vector<std::shared_ptr<IRStatement>> stmts;

public:
	IRStatementBlock(std::vector<std::shared_ptr<IRStatement>> stmts, std::shared_ptr<R2WML_Scope> scope);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::vector<std::shared_ptr<IRStatement>> get_statements();
};

///
/// @brief Takes the address of an lvalue (i.e. `&a`)
///
class IRAddressOf : public IRRValue
{
private:
	std::shared_ptr<IRLValue> base_expr;

public:
	IRAddressOf(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> base_expr);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Represents a case label
///
class IRCase : public IRStatement
{
protected:
	std::shared_ptr<IRExpression> expr;

	std::vector<std::shared_ptr<IRStatement>> stmts;

public:
	IRCase(std::shared_ptr<IRExpression> expr, std::vector<std::shared_ptr<IRStatement>> stmts);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual void add_statement(std::shared_ptr<IRStatement> stmt);
};

///
/// @brief Represents a default case label
///
class IRDefault : public IRCase
{
public:
	IRDefault(std::vector<std::shared_ptr<IRStatement>> stmts);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Represents a switch block
///
class IRSwitchStmt : public IRScopedStatement
{
private:
	std::shared_ptr<IRExpression> expr;

	std::vector<std::shared_ptr<IRCase>> cases;

public:
	IRSwitchStmt(std::shared_ptr<IRExpression> expr, std::vector<std::shared_ptr<IRCase>> cases, std::shared_ptr<R2WML_Scope> scope);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Represents a break out of a switch or loop statement
///
class IRBreak : public IRStatement
{
public:
	IRBreak();

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Represents a continue in a loop statement
///
class IRContinue : public IRStatement
{
public:
	IRContinue();

	virtual std::string pp();
	
	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

///
/// @brief Empty node, can be used as a placeholder
///
class IREmptyNode : public IRNode
{
public:
	IREmptyNode();

	std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

};

///
/// @brief Represents a simple list of variables
///
class IRVariableList : public IRNode
{
private:
	std::vector<std::shared_ptr<IRVariable>> vars;

public:
	IRVariableList(std::vector<std::shared_ptr<IRVariable>> vars);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::vector<std::shared_ptr<IRVariable>> get_vars();

	virtual void add_var(std::shared_ptr<IRVariable> var, int pos);
};

///
/// @brief Represents a function declaration
///
class IRFunction : public IRScopedStatement
{
private:
	IRName function_name;
	std::shared_ptr<IRVariableList> vars;
	std::shared_ptr<IRVariableList> template_vars;
	std::shared_ptr<IRStatementBlock> body;
	std::shared_ptr<IRType> return_type;
	std::vector<std::shared_ptr<IRVerificationCondition>> vcs;
	bool is_template;
	bool is_template_instantiation;
	bool is_ref_function;

public:
	IRFunction(IRName function_name, std::shared_ptr<IRVariableList> vars, std::shared_ptr<IRStatementBlock> body, std::shared_ptr<IRType> return_type,
			bool is_template, bool is_template_instantiation, std::vector<std::shared_ptr<IRVariable>> template_vars,
			std::shared_ptr<R2WML_Scope> scope, bool is_ref);

	virtual std::string pp();

	virtual void add_vc(std::shared_ptr<IRVerificationCondition> vc);

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::vector<std::shared_ptr<IRVariable>> get_vars();

	virtual IRName get_name();

	virtual bool get_is_template();

	virtual bool get_is_template_instantiation();

	virtual std::shared_ptr<IRStatementBlock> get_body();

	virtual void set_body(std::shared_ptr<IRStatementBlock> block);

	virtual bool has_body();

	virtual std::vector<std::shared_ptr<IRVariable>> get_template_vars();

	virtual std::vector<std::shared_ptr<IRVerificationCondition>> get_vcs();

	virtual std::shared_ptr<IRType> get_return_type();

	virtual bool get_is_ref();
};

///
/// @brief Represents an IR program
///
class IRProgram : public IRNode
{
private:
	std::vector<std::shared_ptr<IRStatement>> globals;
	std::shared_ptr<R2WML_Scope> scope;

public:
	IRProgram(std::vector<std::shared_ptr<IRStatement>> globals, std::shared_ptr<R2WML_Scope> scope);

	virtual std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::vector<std::shared_ptr<IRStatement>> get_globals();

	virtual std::vector<std::shared_ptr<IRClass>> get_classes();

	virtual std::vector<std::shared_ptr<IRFunction>> get_functions();

	virtual std::vector<std::shared_ptr<IRVariableDeclarationStmt>> get_variables();

	virtual std::shared_ptr<R2WML_Scope> get_scope();
};

///
/// @brief Discards the result of an expression so it can be used as a statement
///
class IRExpressionStmt : public IRStatement
{
private:
	std::shared_ptr<IRExpression> expr;

public:
	IRExpressionStmt(std::shared_ptr<IRExpression> expr);

	std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::shared_ptr<IRExpression> get_expr();
};

///
/// @brief Represents the concept of loops (i.e. `for`, `while`)
///
class IRLoopStmt : public IRScopedStatement
{
private:
	std::vector<std::shared_ptr<IRVerificationCondition>> vcs;
	std::shared_ptr<IRExpression> comp;
	std::shared_ptr<IRStatement> body;

public:
	IRLoopStmt(std::shared_ptr<R2WML_Scope> scope, std::shared_ptr<IRExpression> comp, std::shared_ptr<IRStatement> body);

	virtual std::vector<std::shared_ptr<IRNode>> traverse();

	virtual std::string pp();

	virtual void add_vc(std::shared_ptr<IRVerificationCondition> vc);

	virtual std::vector<std::shared_ptr<IRVerificationCondition>> get_vcs();
};

///
/// @brief Represents `result` in the verification language
/// 
/// @tparam T Either IRLValue or IRRValue, represents the value category of `result`
///
template <typename T>
class IRResult : public T
{
public:
	IRResult();

	std::string pp();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

template class IRResult<IRRValue>;
template class IRResult<IRLValue>;

///
/// @brief Represents a class declaration
///
class IRClass : public IRScopedStatement
{
private:
	std::vector<std::shared_ptr<IRVariableDeclarationStmt>> vars;
	std::vector<std::shared_ptr<IRFunction>> funcs;
	IRName name;

public:
	IRClass(std::vector<std::shared_ptr<IRVariableDeclarationStmt>> vars, std::vector<std::shared_ptr<IRFunction>> funcs, IRName name, std::shared_ptr<R2WML_Scope> scope);

	std::string pp();

	virtual void set_name(IRName name);

	virtual IRName get_name();

	virtual std::vector<std::shared_ptr<IRFunction>> get_funcs();

	virtual std::vector<std::shared_ptr<IRVariableDeclarationStmt>> get_vars();

	virtual void set_func(std::shared_ptr<IRFunction> func, int idx);

	virtual void tidy_funcs();

	virtual std::vector<std::shared_ptr<IRNode>> traverse();
};

}

#endif
