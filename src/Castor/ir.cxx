// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "ir.hxx"
#include <iostream>
#include <climits>
#include "exception.hxx"
#include "function_table.hxx"

using namespace IR;

IRNode::IRNode() = default;

IRType::IRType()
{
	this->is_reference = false;
};

void IRType::set_is_reference()
{
	this->is_reference = true;
}

bool IRType::get_is_reference()
{
	return this->is_reference;
}

IRVoidType::IRVoidType() = default;

std::string IRVoidType::pp()
{
	return "void" + std::string(is_reference ? "&" : "");
}

size_t IRVoidType::get_sizeof()
{
	return 0;
}

IRLabelType::IRLabelType() = default;

std::string IRLabelType::pp()
{
	return "label" + std::string(is_reference ? "&" : "");
}

size_t IRLabelType::get_sizeof()
{
	return 0;
}

IRBoolType::IRBoolType() = default;

std::string IRBoolType::pp()
{
	return "bool" + std::string(is_reference ? "&" : "");
}

size_t IRBoolType::get_sizeof()
{
	return 1;
}

IRNonRealType::IRNonRealType() = default;

std::string IRNonRealType::pp()
{
	return "nonreal" + std::string(is_reference ? "&" : "");
}

size_t IRNonRealType::get_sizeof()
{
	throw UnknownSizeException();
}

IRUnknownType::IRUnknownType() = default;

std::string IRUnknownType::pp()
{
	return "UNKNOWN";
}

size_t IRUnknownType::get_sizeof()
{
	throw UnknownSizeException();
}

IRArrayType::IRArrayType(std::shared_ptr<IRType> base_type, int size)
{
	this->base_type = base_type;
	this->size = size;
}

size_t IRArrayType::get_sizeof()
{
	return this->size * this->base_type->get_sizeof();
}

std::string IRArrayType::pp()
{
	return this->base_type->pp() + "[" + std::to_string(this->size) + "]" + std::string(is_reference ? "&" : "");
}

int IRArrayType::get_size()
{
	return this->size;
}

std::shared_ptr<IRType> IRArrayType::get_base_type()
{
	return this->base_type;
}

IRClassType::IRClassType(IRName class_name)
{
	this->class_name = class_name;
}

size_t IRClassType::get_sizeof()
{
	auto vars = this->get_class_definition()->get_vars();
	size_t size = 0;

	for (auto v : vars)
		size += v->get_var()->get_type()->get_sizeof();

	return size;
}

std::string IRClassType::pp()
{
	return this->class_name + std::string(is_reference ? "&" : "");
}

IRName IRClassType::get_class_name()
{
	return this->class_name;
}

std::shared_ptr<IRClass> IRClassType::get_class_definition()
{
	return this->class_definition;
}

void IRClassType::set_class_definition(std::shared_ptr<IRClass> clas)
{
	this->class_definition = clas;
}

IRIntegralType::IRIntegralType(int bits, bool is_signed)
{
	this->bits = bits;
	this->is_signed = is_signed;
}

size_t IRIntegralType::get_sizeof()
{
	return this->bits / 8;
}

std::string IRIntegralType::pp()
{
	if (dynamic_cast<IRUnboundedIntegralType*>(this))
		return "int";

	std::string schar = this->is_signed ? "s" : "u";

	return schar + "int" + std::to_string(this->bits) + std::string(is_reference ? "&" : "");
}

int IRIntegralType::get_bits()
{
	return this->bits;
}

bool IRIntegralType::get_is_signed()
{
	return this->is_signed;
}

IRUnboundedIntegralType::IRUnboundedIntegralType() : IRIntegralType(INT32_MAX, true) { }

IRS64Type::IRS64Type() : IRIntegralType(64, true) { }

IRS32Type::IRS32Type() : IRIntegralType(32, true) { }

IRS16Type::IRS16Type() : IRIntegralType(16, true) { }

IRS8Type::IRS8Type()   : IRIntegralType(8, true) { }

IRU64Type::IRU64Type() : IRIntegralType(64, false) { }

IRU32Type::IRU32Type() : IRIntegralType(32, false) { }

IRU16Type::IRU16Type() : IRIntegralType(16, false) { }

IRU8Type::IRU8Type()   : IRIntegralType(8, false) { }

IRPointerType::IRPointerType(std::shared_ptr<IRType> base_type) :
	IRIntegralType(64, false),
	base_type(base_type) { }

std::shared_ptr<IRType> IRPointerType::get_base_type()
{
	return this->base_type;
}

std::string IRPointerType::pp()
{
	return "(" + this->base_type->pp() + ")*" + std::string(is_reference ? "&" : "");
}

IRConstType::IRConstType(std::shared_ptr<IRType> base_type)
{
	this->base_type = base_type;
}

std::string IRConstType::pp()
{
	return "const (" + this->base_type->pp() + ")" + std::string(is_reference ? "&" : "");
}

std::shared_ptr<IRType> IRConstType::get_base_type()
{
	return this->base_type;
}

size_t IRConstType::get_sizeof()
{
	return this->base_type->get_sizeof();
}

IRExpression::IRExpression(std::shared_ptr<IRType> type)
{
	this->type = type;
}

std::shared_ptr<IRType> IRExpression::get_type()
{
	return this->type;
}

void IRExpression::set_type(std::shared_ptr<IRType> type)
{
	this->type = type;
}

IRLValue::IRLValue(std::shared_ptr<IRType> type) : IRExpression(type) { }

IRFieldReference::IRFieldReference(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_object, IRName field_name) :
	IRLValue(type)
{
	this->base_object = base_object;
	this->field_name = field_name;
}

std::string IRFieldReference::pp()
{
	return "(" + this->base_object->pp() + ").(" + this->type->pp() + ")" + this->field_name;
}

std::shared_ptr<IRExpression> IRFieldReference::get_base_object()
{
	return this->base_object;
}

IRName IRFieldReference::get_field_name()
{
	return this->field_name;
}

void IRFieldReference::set_field_name(IRName name)
{
	this->field_name = name;
}

IRArrayIndex::IRArrayIndex(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> array, std::shared_ptr<IRExpression> index) :
	IRLValue(type)
{
	this->array = array;
	this->index = index;
}

std::string IRArrayIndex::pp()
{
	return "(" + this->array->pp() + ")[" + this->index->pp() + "]";
}

IRArrayRangeIndex::IRArrayRangeIndex(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> array, std::shared_ptr<IRExpression> begin, std::shared_ptr<IRExpression> end) :
	IRRValue(type)
{
	this->array = array;
	this->begin = begin;
	this->end = end;
}

std::string IRArrayRangeIndex::pp()
{
	return "(" + this->array->pp() + ")[" + this->begin->pp() + " .. " + this->end->pp() + "]";
}

IRVariableReference::IRVariableReference(std::shared_ptr<IRVariable> variable) :
	IRLValue(variable->get_type())
{
	this->variable = variable;
}

std::string IRVariableReference::pp()
{
	if (this->variable->has_constexpr_value())
		return this->variable->get_constexpr_value()->pp();
	else
		return "(" + this->variable->get_type()->pp() + ")" + this->variable->get_name();
}

std::shared_ptr<IRVariable> IRVariableReference::get_variable()
{
	return this->variable;
}

std::string IRVariableReference::get_name()
{
	return this->variable->get_name();
}

IRPointerDereference::IRPointerDereference(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item) :
	IRLValue(type)
{
	this->base_item = base_item;
}

std::string IRPointerDereference::pp()
{
	return "*(" + this->base_item->pp() + ")";
}

IRReference::IRReference(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> base_item) :
	IRLValue(type)
{
	this->base_item = base_item;
}

std::string IRReference::pp()
{
	return "ref (" + this->base_item->pp() + ")";
}

IRVariable::IRVariable(std::shared_ptr<IRType> type, IRName name, IRName frontend_name, bool is_program_variable)
{
	this->type = type;
	this->name = name;
	this->frontend_name = frontend_name;
	this->constexpr_value = nullptr;
	this->is_program_variable = is_program_variable;
}

std::string IRVariable::pp()
{
	throw CastorException("Should never call IRVariable's pretty-printer!");
}

IRVariable::IRVariable(std::shared_ptr<IRType> type, IRName name, IRName frontend_name, std::shared_ptr<IRExpression> constexpr_value, bool is_program_variable)
{
	this->type = type;
	this->name = name;
	this->frontend_name = frontend_name;
	this->constexpr_value = constexpr_value;
	this->is_program_variable = is_program_variable;
}

bool IRVariable::get_is_program_variable()
{
	return this->is_program_variable;
}

bool IRVariable::has_constexpr_value()
{
	return this->constexpr_value != nullptr;
}

std::shared_ptr<IRExpression> IRVariable::get_constexpr_value()
{
	return this->constexpr_value;
}

IRName IRVariable::get_name()
{
	return this->name;
}

IRName IRVariable::get_frontend_name()
{
	return this->frontend_name;
}

std::shared_ptr<IRType> IRVariable::get_type()
{
	return this->type;
}

IRRValue::IRRValue(std::shared_ptr<IRType> type) : IRExpression(type) { }

IRMaterialize::IRMaterialize(std::shared_ptr<IRRValue> base_expr) : IRLValue(base_expr->get_type())
{
	this->base_expr = base_expr;
}

std::string IRMaterialize::pp()
{
	return this->base_expr->pp();
}

std::vector<std::shared_ptr<IRNode>> IRMaterialize::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->base_expr });
}

template <typename T>
IRTernary<T>::IRTernary(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> condition, std::shared_ptr<IRExpression> then, std::shared_ptr<IRExpression> els) :
	T(type)
{
	this->condition = condition;
	this->then = then;
	this->els = els;
}

template <typename T>
std::string IRTernary<T>::pp()
{
	return "(" + this->condition->pp() + " ? " + this->then->pp() + " : " + this->els->pp() + ")";
}

template <typename T>
std::vector<std::shared_ptr<IRNode>> IRTernary<T>::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->condition, this->then, this->els });
}

IRLabelReference::IRLabelReference(IRName name) : IRRValue(std::make_shared<IRLabelType>())
{
	this->name = name;
	this->name[0] = std::toupper(this->name[0]);
}

std::string IRLabelReference::pp()
{
	return "@" + this->name;
}

IRName IRLabelReference::get_name()
{
	return this->name;
}

std::vector<std::shared_ptr<IRNode>> IRLabelReference::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

IRLabel::IRLabel(std::shared_ptr<IRLabelReference> label)
{
	this->label = label;
}

std::string IRLabel::pp()
{
	return this->label->pp() + ":\n";
}

std::vector<std::shared_ptr<IRNode>> IRLabel::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->label });
}

template <typename T>
IRCast<T>::IRCast(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_expr) :
	T(type)
{
	this->base_expr = base_expr;
}

template <typename T>
std::vector<std::shared_ptr<IRNode>> IRCast<T>::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->base_expr });
}

template <typename T>
std::shared_ptr<IRExpression> IRCast<T>::get_base_expr()
{
	return this->base_expr;
}

template <typename T>
std::string IRCast<T>::pp()
{
	return "cast<" + this->type->pp() + ">(" + this->base_expr->pp() + ")";
}

template class IRCast<IRLValue>;
template class IRCast<IRRValue>;

IRUnaryOperation::IRUnaryOperation(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item) :
	IRRValue(type)
{
	this->base_item = base_item;
}

std::shared_ptr<IRExpression> IRUnaryOperation::get_base_item()
{
	return this->base_item;
}

IRIncrementOp::IRIncrementOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item) :
	IRUnaryOperation(type, base_item) { }

std::string IRIncrementOp::pp()
{
	return "(" + this->base_item->pp() + ")++";
}

IRDecrementOp::IRDecrementOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item) :
	IRUnaryOperation(type, base_item) { }

std::string IRDecrementOp::pp()
{
	return "(" + this->base_item->pp() + ")--";
}

IRNegationOp::IRNegationOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item) :
	IRUnaryOperation(type, base_item) { }

std::string IRNegationOp::pp()
{
	return "-(" + this->base_item->pp() + ")";
}

IRBoolNegationOp::IRBoolNegationOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item) :
	IRUnaryOperation(type, base_item) { }

std::string IRBoolNegationOp::pp()
{
	return "!(" + this->base_item->pp() + ")";
}

IRBitNegationOp::IRBitNegationOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_item) :
	IRUnaryOperation(type, base_item) { }

std::string IRBitNegationOp::pp()
{
	return "~(" + this->base_item->pp() + ")";
}

IRBinaryOperation::IRBinaryOperation(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRRValue(type)
{
	this->lhs = lhs;
	this->rhs = rhs;
}

std::shared_ptr<IRExpression> IRBinaryOperation::get_lhs()
{
	return this->lhs;
}

std::shared_ptr<IRExpression> IRBinaryOperation::get_rhs()
{
	return this->rhs;
}

IRAssignmentOperation::IRAssignmentOperation(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRLValue(type)
{
	this->lhs = lhs;
	this->rhs = rhs;
}

std::shared_ptr<IRLValue> IRAssignmentOperation::get_lhs()
{
	return this->lhs;
}

std::shared_ptr<IRExpression> IRAssignmentOperation::get_rhs()
{
	return this->rhs;
}

template <typename T>
IRCommaOp<T>::IRCommaOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	T(type)
{
	this->lhs = lhs;
	this->rhs = rhs;
}

template <typename T>
std::string IRCommaOp<T>::pp()
{
	return "(" + this->lhs->pp() + ", " + this->rhs->pp() + ")";
}

template class IRCommaOp<IRLValue>;
template class IRCommaOp<IRRValue>;

IRAssignOp::IRAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRAssignmentOperation(type, lhs, rhs) { }

std::string IRAssignOp::pp()
{
	return "(" + this->lhs->pp() + " = " + this->rhs->pp() + ")";
}

IRGreaterEqualsOp::IRGreaterEqualsOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRGreaterEqualsOp::pp()
{
	return "(" + this->lhs->pp() + " >= " + this->rhs->pp() + ")";
}

IRGreaterThanOp::IRGreaterThanOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRGreaterThanOp::pp()
{
	return "(" + this->lhs->pp() + " > " + this->rhs->pp() + ")";
}

IRLessEqualsOp::IRLessEqualsOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRLessEqualsOp::pp()
{
	return "(" + this->lhs->pp() + " <= " + this->rhs->pp() + ")";
}

IRLessThanOp::IRLessThanOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRLessThanOp::pp()
{
	return "(" + this->lhs->pp() + " < " + this->rhs->pp() + ")";
}

IRDivideOp::IRDivideOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRDivideOp::pp()
{
	return "(" + this->lhs->pp() + " / " + this->rhs->pp() + ")";
}

IRDivideAssignOp::IRDivideAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRAssignmentOperation(type, lhs, rhs) { }

std::string IRDivideAssignOp::pp()
{
	return "(" + this->lhs->pp() + " /= " + this->rhs->pp() + ")";
}

IRModuloOp::IRModuloOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRModuloOp::pp()
{
	return "(" + this->lhs->pp() + " % " + this->rhs->pp() + ")";
}

IRModuloAssignOp::IRModuloAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRAssignmentOperation(type, lhs, rhs) { }

std::string IRModuloAssignOp::pp()
{
	return "(" + this->lhs->pp() + " %= " + this->rhs->pp() + ")";
}

IRMultiplyOp::IRMultiplyOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRMultiplyOp::pp()
{
	return "(" + this->lhs->pp() + " * " + this->rhs->pp() + ")";
}

IRMultiplyAssignOp::IRMultiplyAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRAssignmentOperation(type, lhs, rhs) { }

std::string IRMultiplyAssignOp::pp()
{
	return "(" + this->lhs->pp() + " *= " + this->rhs->pp() + ")";
}

IRNotEqualsOp::IRNotEqualsOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRNotEqualsOp::pp()
{
	return "(" + this->lhs->pp() + " != " + this->rhs->pp() + ")";
}

IREqualsOp::IREqualsOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IREqualsOp::pp()
{
	return "(" + this->lhs->pp() + " == " + this->rhs->pp() + ")";
}

IRBitRShiftOp::IRBitRShiftOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRBitRShiftOp::pp()
{
	return "(" + this->lhs->pp() + " >> " + this->rhs->pp() + ")";
}

IRBitRShiftAssignOp::IRBitRShiftAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRAssignmentOperation(type, lhs, rhs) { }

std::string IRBitRShiftAssignOp::pp()
{
	return "(" + this->lhs->pp() + " >>= " + this->rhs->pp() + ")";
}

IRBitLShiftOp::IRBitLShiftOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRBitLShiftOp::pp()
{
	return "(" + this->lhs->pp() + " << " + this->rhs->pp() + ")";
}

IRBitLShiftAssignOp::IRBitLShiftAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRAssignmentOperation(type, lhs, rhs) { }

std::string IRBitLShiftAssignOp::pp()
{
	return "(" + this->lhs->pp() + " <<= " + this->rhs->pp() + ")";
}

IRBitOrOp::IRBitOrOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRBitOrOp::pp()
{
	return "(" + this->lhs->pp() + " | " + this->rhs->pp() + ")";
}

IRBitOrAssignOp::IRBitOrAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRAssignmentOperation(type, lhs, rhs) { }

std::string IRBitOrAssignOp::pp()
{
	return "(" + this->lhs->pp() + " |= " + this->rhs->pp() + ")";
}

IRBitXorOp::IRBitXorOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRBitXorOp::pp()
{
	return "(" + this->lhs->pp() + " ^ " + this->rhs->pp() + ")";
}

IRBitXorAssignOp::IRBitXorAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRAssignmentOperation(type, lhs, rhs) { }

std::string IRBitXorAssignOp::pp()
{
	return "(" + this->lhs->pp() + " ^= " + this->rhs->pp() + ")";
}

IRBitAndOp::IRBitAndOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRBitAndOp::pp()
{
	return "(" + this->lhs->pp() + " & " + this->rhs->pp() + ")";
}

IRBitAndAssignOp::IRBitAndAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRAssignmentOperation(type, lhs, rhs) { }

std::string IRBitAndAssignOp::pp()
{
	return "(" + this->lhs->pp() + " &= " + this->rhs->pp() + ")";
}

IRSubtractionOp::IRSubtractionOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRSubtractionOp::pp()
{
	return "(" + this->lhs->pp() + " - " + this->rhs->pp() + ")";
}

IRSubtractionAssignOp::IRSubtractionAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRAssignmentOperation(type, lhs, rhs) { }

std::string IRSubtractionAssignOp::pp()
{
	return "(" + this->lhs->pp() + " -= " + this->rhs->pp() + ")";
}

IRAdditionOp::IRAdditionOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRAdditionOp::pp()
{
	return "(" + this->lhs->pp() + " + " + this->rhs->pp() + ")";
}

IRAdditionAssignOp::IRAdditionAssignOp(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> lhs, std::shared_ptr<IRExpression> rhs) :
	IRAssignmentOperation(type, lhs, rhs) { }

std::string IRAdditionAssignOp::pp()
{
	return "(" + this->lhs->pp() + " += " + this->rhs->pp() + ")";
}

IRAndOp::IRAndOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRAndOp::pp()
{
	return "(" + this->lhs->pp() + " && " + this->rhs->pp() + ")";
}

IROrOp::IROrOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IROrOp::pp()
{
	return "(" + this->lhs->pp() + " || " + this->rhs->pp() + ")";
}

IRImpliesOp::IRImpliesOp(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> lhs, std::shared_ptr<IRExpression> rhs) :
	IRBinaryOperation(type, lhs, rhs) { }

std::string IRImpliesOp::pp()
{
	return "(" + this->lhs->pp() + " -> " + this->rhs->pp() + ")";
}

IRNullptr::IRNullptr() : IRRValue(std::make_shared<IRPointerType>(std::make_shared<IRU8Type>())) { }

std::string IRNullptr::pp()
{
	return "nullptr";
}

std::vector<std::shared_ptr<IRNode>> IRNullptr::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

IRLiteral::IRLiteral(std::shared_ptr<IRType> type) : IRRValue(type) { }

IRBoolLiteral::IRBoolLiteral(bool value) :
	IRLiteral(std::make_shared<IRBoolType>())
{
	this->value = value;
}

std::string IRBoolLiteral::pp()
{
	return this->value ? "true" : "false";
}

bool IRBoolLiteral::get_value()
{
	return this->value;
}

IRU8Literal::IRU8Literal(uint8_t value) :
	IRLiteral(std::make_shared<IRU8Type>())
{
	this->value = value;
}

std::string IRU8Literal::pp()
{
	return std::to_string(this->value);
}

uint8_t IRU8Literal::get_value()
{
	return this->value;
}

IRS8Literal::IRS8Literal(int8_t value) :
	IRLiteral(std::make_shared<IRS8Type>())
{
	this->value = value;
}

std::string IRS8Literal::pp()
{
	return std::to_string(this->value);
}

int8_t IRS8Literal::get_value()
{
	return this->value;
}

IRU16Literal::IRU16Literal(uint16_t value) :
	IRLiteral(std::make_shared<IRU16Type>())
{
	this->value = value;
}

std::string IRU16Literal::pp()
{
	return std::to_string(this->value);
}

uint16_t IRU16Literal::get_value()
{
	return this->value;
}

IRS16Literal::IRS16Literal(int16_t value) :
	IRLiteral(std::make_shared<IRS16Type>())
{
	this->value = value;
}

std::string IRS16Literal::pp()
{
	return std::to_string(this->value);
}

int16_t IRS16Literal::get_value()
{
	return this->value;
}

IRU32Literal::IRU32Literal(uint32_t value) :
	IRLiteral(std::make_shared<IRU32Type>())
{
	this->value = value;
}

std::string IRU32Literal::pp()
{
	return std::to_string(this->value);
}

uint32_t IRU32Literal::get_value()
{
	return this->value;
}

IRS32Literal::IRS32Literal(int32_t value) :
	IRLiteral(std::make_shared<IRS32Type>())
{
	this->value = value;
}

std::string IRS32Literal::pp()
{
	return std::to_string(this->value);
}

int32_t IRS32Literal::get_value()
{
	return this->value;
}

IRU64Literal::IRU64Literal(uint64_t value) :
	IRLiteral(std::make_shared<IRU64Type>())
{
	this->value = value;
}

std::string IRU64Literal::pp()
{
	return std::to_string(this->value);
}

uint64_t IRU64Literal::get_value()
{
	return this->value;
}

IRS64Literal::IRS64Literal(int64_t value) :
	IRLiteral(std::make_shared<IRS64Type>())
{
	this->value = value;
}

std::string IRS64Literal::pp()
{
	return std::to_string(this->value);
}

int64_t IRS64Literal::get_value()
{
	return this->value;
}

IRQuantifier::IRQuantifier(std::vector<std::shared_ptr<IRVariable>> variables, std::shared_ptr<IRExpression> expression) :
	IRRValue(std::make_shared<IRBoolType>())
{
	this->variables = variables;
	this->expression = expression;
}

std::vector<std::shared_ptr<IRVariable>> IRQuantifier::get_vars()
{
	return this->variables;
}

IRForall::IRForall(std::vector<std::shared_ptr<IRVariable>> variables, std::shared_ptr<IRExpression> expression) :
	IRQuantifier(variables, expression) { }

std::string IRForall::pp()
{
	std::string ret = "(forall";
	for (std::shared_ptr<IRVariable> var : this->variables)
		ret += " (" + var->get_type()->pp() + ")" + var->get_name();
	ret += ". " + this->expression->pp() + ")";

	return ret;
}

IRExists::IRExists(std::vector<std::shared_ptr<IRVariable>> variables, std::shared_ptr<IRExpression> expression) :
	IRQuantifier(variables, expression) { }

std::string IRExists::pp()
{
	std::string ret = "(exists";
	for (std::shared_ptr<IRVariable> var : this->variables)
		ret += " (" + var->get_type()->pp() + ")" + var->get_name();
	ret += ". " + this->expression->pp() + ")";

	return ret;
}

IRStatement::IRStatement() = default;

IRGhostStatement::IRGhostStatement() = default;

void IRGhostStatement::attach_stmt(std::shared_ptr<IRStatement> statement)
{
	this->statement = statement;
}

std::string IRGhostStatement::pp()
{
	if (this->statement)
		return "ghost { " + this->statement->pp() + " }";
	else
		return "ghost { }";
}

std::shared_ptr<IRStatement> IRGhostStatement::get_stmt()
{
	return this->statement;
}

std::vector<std::shared_ptr<IRNode>> IRGhostStatement::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->statement });
}

IREmptyStatement::IREmptyStatement() = default;

std::string IREmptyStatement::pp()
{
	return "";
}

std::vector<std::shared_ptr<IRNode>> IREmptyStatement::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

IRVerificationCondition::IRVerificationCondition() = default;

void IRVerificationCondition::attach_str(std::string ver)
{
	this->ver = ver;
}

std::string IRVerificationCondition::get_str()
{
	return this->ver;
}

void IRVerificationCondition::set_debug_str(std::string debug_str)
{
	this->debug_str = debug_str;
}

std::string IRVerificationCondition::get_debug_str()
{
	return this->debug_str;
}

IRLemma::IRLemma(IRName name, std::shared_ptr<IRExpression> vc)
{
	this->name = name;
	this->vc = vc;
}

std::string IRLemma::pp()
{
	return "lemma " + this->name + ": " + this->vc->pp() + "\n";
}

IRName IRLemma::get_name()
{
	return this->name;
}

std::shared_ptr<IRExpression> IRLemma::get_vc()
{
	return this->vc;
}

IRAxiom::IRAxiom(IRName name, std::shared_ptr<IRExpression> vc) : IRLemma(name, vc) { }

std::string IRAxiom::pp()
{
	return "axiom " + this->name + ": " + this->vc->pp() + "\n";
}

IREnsures::IREnsures(std::shared_ptr<IRExpression> vc)
{
	this->vc = vc;
	this->debug_str = "ensures clause";
}

std::string IREnsures::pp()
{
	return "ensures { " + this->vc->pp() + " } ";
}

IRRequires::IRRequires(std::shared_ptr<IRExpression> vc)
{
	this->vc = vc;
	this->debug_str = "requires cause";
}

std::string IRRequires::pp()
{
	return "requires { " + this->vc->pp() + " } ";
}

IRLoopInvariant::IRLoopInvariant(std::shared_ptr<IRExpression> vc)
{
	this->vc = vc;
	this->debug_str = "loop invariant clause";
}

std::shared_ptr<IRExpression> IRLoopInvariant::get_vc()
{
	return this->vc;
}

std::string IRLoopInvariant::pp()
{
	return "invariant { " + this->vc->pp() + " } ";
}

IRLoopVariant::IRLoopVariant(std::shared_ptr<IRExpression> vc)
{
	this->vc = vc;
	this->debug_str = "loop variant clause";
}

std::string IRLoopVariant::pp()
{
	return "variant { " + this->vc->pp() + " } ";
}

IRWrites::IRWrites(std::vector<std::shared_ptr<IRExpression>> vars)
{
	this->vars = vars;
	this->debug_str = "writes clause";
}

std::string IRWrites::pp()
{
	std::string ret = "writes { ";

	if (this->vars.size())
	{
		for (int i = 0; i < this->vars.size() - 1; i++)
		{
			ret += this->vars[i]->pp() + ", ";
		}
		ret += this->vars[this->vars.size() - 1]->pp();
	}

	ret += " } ";

	return ret;
}

IRFrees::IRFrees(std::vector<std::shared_ptr<IRExpression>> vars)
{
	this->vars = vars;
	this->debug_str = "frees clause";
}

std::string IRFrees::pp()
{
	std::string ret = "frees { ";

	if (this->vars.size())
	{
		for (int i = 0; i < this->vars.size() - 1; i++)
		{
			ret += this->vars[i]->pp() + ", ";
		}
		ret += this->vars[this->vars.size() - 1]->pp();
	}

	ret += " } ";

	return ret;
}

IRAssert::IRAssert(std::shared_ptr<IRExpression> vc)
{
	this->vc = vc;
	this->debug_str = "assertion";
}

std::string IRAssert::pp()
{
	return "assert { " + this->vc->pp() + " } ";
}

void IRAssert::set_debug_str(std::string debug_str)
{
	this->debug_str = debug_str;
}

std::string IRAssert::get_debug_str()
{
	return this->debug_str;
}

void IRAssert::attach_str(std::string ver)
{
	this->ver = ver;
}

std::string IRAssert::get_str()
{
	return this->ver;
}

IRAssume::IRAssume(std::shared_ptr<IRExpression> vc) : IRAssert(vc) { }

std::string IRAssume::pp()
{
	return "assume { " + this->vc->pp() + " } ";
}

IRFunctionRefExpr::IRFunctionRefExpr(IRName function_name) :
	IRRValue(std::make_shared<IRNonRealType>())
{
	this->function_name = function_name;
}

std::string IRFunctionRefExpr::pp()
{
	return this->function_name;
}

std::string IRFunctionRefExpr::get_name()
{
	return this->function_name;
}

template <typename T>
IRFunctionCallExpr<T>::IRFunctionCallExpr(std::shared_ptr<IRType> type, std::shared_ptr<IRFunctionRefExpr> function, std::vector<std::shared_ptr<IRExpression>> params) :
	T(type)
{
	this->function = function;
	this->params = params;
	this->is_constructor = false;
}

template <typename T>
void IRFunctionCallExpr<T>::set_is_constructor()
{
	this->is_constructor = true;
}

template <typename T>
bool IRFunctionCallExpr<T>::get_is_constructor()
{
	return this->is_constructor;
}

template <typename T>
std::string IRFunctionCallExpr<T>::get_function_name()
{
	return this->function->get_name();
}

template <typename T>
std::string IRMemberFunctionCallExpr<T>::get_function_name()
{
	return this->function->get_name();
}

template <typename T>
std::vector<std::shared_ptr<IRExpression>> IRFunctionCallExpr<T>::get_params()
{
	return this->params;
}

template <typename T>
std::vector<std::shared_ptr<IRExpression>> IRMemberFunctionCallExpr<T>::get_params()
{
	return this->params;
}

template <typename T>
void IRFunctionCallExpr<T>::set_function(std::shared_ptr<IRFunctionRefExpr> function)
{
	this->function = function;
}

template <typename T>
void IRFunctionCallExpr<T>::set_params(std::vector<std::shared_ptr<IRExpression>> params)
{
	this->params = params;
}

template <typename T>
void IRMemberFunctionCallExpr<T>::set_params(std::vector<std::shared_ptr<IRExpression>> params)
{
	this->params = params;
}

template <typename T>
std::string IRFunctionCallExpr<T>::pp()
{
	std::string ret = this->function->pp() + "(";

	for (int i = 0; i < (int)this->params.size() - 1; i++)
	{
		ret += this->params[i]->pp() + ", ";
	}

	if (this->params.size())
		ret += this->params[this->params.size() - 1]->pp();

	ret += ")";

	return ret;
}

template <typename T>
IRMemberFunctionCallExpr<T>::IRMemberFunctionCallExpr(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_object, std::shared_ptr<IRFunctionRefExpr> function, std::vector<std::shared_ptr<IRExpression>> params) :
	T(type)
{
	this->base_object = base_object;
	this->function = function;
	this->params = params;
}

template <typename T>
std::string IRMemberFunctionCallExpr<T>::pp()
{
	std::string ret = this->base_object->pp() + "." + this->function->pp() + "(";

	for (int i = 0; i < (int)this->params.size() - 1; i++)
	{
		ret += this->params[i]->pp() + ", ";
	}

	if (this->params.size())
		ret += this->params[this->params.size() - 1]->pp();
	
	ret += ")";

	return ret;
}

IREmptyNode::IREmptyNode() = default;

std::string IREmptyNode::pp() { return ""; }

std::vector<std::shared_ptr<IRNode>> IREmptyNode::traverse() { return std::vector<std::shared_ptr<IRNode>>(); }

IRVariableDeclarationStmt::IRVariableDeclarationStmt(std::shared_ptr<IRVariable> var, std::shared_ptr<IRExpression> initial_value, bool is_static)
{
	this->var = var;
	this->initial_value = initial_value;
	this->is_static = is_static;
}

std::string IRVariableDeclarationStmt::pp()
{
	std::string ret = this->var->get_type()->pp() + " " + this->var->get_name();

	if (this->initial_value)
		ret += " = " + this->initial_value->pp();

	ret += ";";
	return ret;
}

std::shared_ptr<IRVariable> IRVariableDeclarationStmt::get_var()
{
	return this->var;
}

bool IRVariableDeclarationStmt::get_is_static()
{
	return this->is_static;
}

std::shared_ptr<IRExpression> IRVariableDeclarationStmt::get_initial_value()
{
	return this->initial_value;
}

IRVariableList::IRVariableList(std::vector<std::shared_ptr<IRVariable>> vars)
{
	this->vars = vars;
}

std::string IRVariableList::pp()
{
	std::string ret;
	for (auto v : vars)
	{
		ret += "(" + v->get_type()->pp() + " " + v->get_name() + ")" + " ";
	}
	return ret;
}

void IRVariableList::add_var(std::shared_ptr<IRVariable> var, int pos)
{
	this->vars.insert(std::next(this->vars.begin(), pos), var);
}

IRReturnStmt::IRReturnStmt(std::shared_ptr<IRExpression> ret)
{
	this->ret = ret;
}

std::string IRReturnStmt::pp()
{
	return "return " + this->ret->pp() + ";";
}

std::shared_ptr<IRExpression> IRReturnStmt::get_stmt()
{
	return ret;
}

void IRReturnStmt::set_stmt(std::shared_ptr<IRExpression> ret)
{
	this->ret = ret;
}

IRStatementBlock::IRStatementBlock(std::vector<std::shared_ptr<IRStatement>> stmts, std::shared_ptr<R2WML_Scope> scope) : IRScopedStatement(scope)
{
	this->stmts = stmts;
}

std::string IRStatementBlock::pp()
{
	std::string ret = "{\n";
	for (auto s : this->stmts)
	{
		ret += s->pp() + "\n";
	}
	ret += "}";
	return ret;
}

std::vector<std::shared_ptr<IRStatement>> IRStatementBlock::get_statements()
{
	return this->stmts;
}

IRFunction::IRFunction(IRName function_name, std::shared_ptr<IRVariableList> vars, std::shared_ptr<IRStatementBlock> body, std::shared_ptr<IRType> return_type,
		bool is_template, bool is_template_instantiation, std::vector<std::shared_ptr<IRVariable>> template_vars,
		std::shared_ptr<R2WML_Scope> scope, bool is_ref) : IRScopedStatement(scope)
{
	this->function_name = function_name;
	this->vars = vars;
	this->body = body;
	this->return_type = return_type;
	this->is_template = is_template;
	this->is_template_instantiation = is_template_instantiation;
	this->template_vars = std::make_shared<IRVariableList>(template_vars);
	this->is_ref_function = is_ref;
}

bool IRFunction::get_is_ref()
{
	return this->is_ref_function;
}

std::vector<std::shared_ptr<IRVerificationCondition>> IRFunction::get_vcs()
{
	return this->vcs;
}

std::vector<std::shared_ptr<IRVariable>> IRFunction::get_template_vars()
{
	return this->template_vars->get_vars();
}

bool IRFunction::get_is_template()
{
	return this->is_template;
}

bool IRFunction::get_is_template_instantiation()
{
	return this->is_template_instantiation;
}

void IRFunction::add_vc(std::shared_ptr<IRVerificationCondition> vc)
{
	this->vcs.push_back(vc);
}

std::string IRFunction::pp()
{
	std::string ret;

	for (auto vc : this->vcs)
		ret += vc->pp() + "\n";

	ret += this->return_type->pp() + " ";
	ret += this->function_name + "(";
	ret += this->vars->pp() + ")";
	if (this->body)
		ret += "\n" + this->body->pp() + "\n";
	else
		ret += ";\n";

	return ret;
}

std::shared_ptr<IRStatementBlock> IRFunction::get_body()
{
	return this->body;
}

void IRFunction::set_body(std::shared_ptr<IRStatementBlock> body)
{
	this->body = body;
}

bool IRFunction::has_body()
{
	return this->body != nullptr;
}

IRProgram::IRProgram(std::vector<std::shared_ptr<IRStatement>> globals, std::shared_ptr<R2WML_Scope> scope)
{
	this->globals = globals;
	this->scope = scope;
}

std::string IRProgram::pp()
{
	std::string ret;
	for (auto g : this->globals)
	{
		ret += g->pp() + "\n";
	}
	return ret;
}

std::shared_ptr<R2WML_Scope> IRProgram::get_scope()
{
	return this->scope;
}

std::string __IRExpressionListWrapper::pp()
{
	return "";
}

__IRLabelWrapper::__IRLabelWrapper(std::shared_ptr<IRLabel> label, std::shared_ptr<IRStatement> statement)
{
	this->label = label;
	this->statement = statement;
}

std::string __IRLabelWrapper::pp()
{
	return "";
}

std::vector<std::shared_ptr<IRNode>> __IRLabelWrapper::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->label, this->statement });
}

IRExpressionStmt::IRExpressionStmt(std::shared_ptr<IRExpression> expr)
{
	this->expr = expr;
}

std::string IRExpressionStmt::pp()
{
	return "ignore(" + this->expr->pp() + ");";
}

std::shared_ptr<IRExpression> IRExpressionStmt::get_expr()
{
	return this->expr;
}

std::vector<std::shared_ptr<IRNode>> IRType::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

std::vector<std::shared_ptr<IRNode>> IRVariable::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

std::vector<std::shared_ptr<IRNode>> IRFieldReference::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->base_object });
}

std::vector<std::shared_ptr<IRNode>> IRArrayIndex::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->array, this->index });
}

std::vector<std::shared_ptr<IRNode>> IRArrayRangeIndex::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->array, this->begin, this->end });
}

std::vector<std::shared_ptr<IRNode>> IRVariableReference::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->variable });
}

std::vector<std::shared_ptr<IRNode>> IRPointerDereference::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->base_item });
}

std::vector<std::shared_ptr<IRNode>> IRReference::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->base_item });
}

std::vector<std::shared_ptr<IRNode>> IRUnaryOperation::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->base_item });
}

std::vector<std::shared_ptr<IRNode>> IRBinaryOperation::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->lhs, this->rhs });
}

std::vector<std::shared_ptr<IRNode>> IRAssignmentOperation::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->lhs, this->rhs });
}

template <typename T>
std::vector<std::shared_ptr<IRNode>> IRCommaOp<T>::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->lhs, this->rhs });
}

std::vector<std::shared_ptr<IRNode>> IRLiteral::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

std::vector<std::shared_ptr<IRNode>> IRQuantifier::traverse()
{
	std::vector<std::shared_ptr<IRNode>> ret;
	ret.push_back(this->expression);
	for (auto v: this->variables)
		ret.push_back(v);
	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRLemma::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->vc });
}

std::vector<std::shared_ptr<IRNode>> IREnsures::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->vc });
}

std::vector<std::shared_ptr<IRNode>> IRRequires::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->vc });
}

std::vector<std::shared_ptr<IRNode>> IRAssert::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->vc });
}

std::vector<std::shared_ptr<IRNode>> IRLoopInvariant::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->vc });
}

std::vector<std::shared_ptr<IRNode>> IRLoopVariant::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->vc });
}

std::vector<std::shared_ptr<IRNode>> IRWrites::traverse()
{
	std::vector<std::shared_ptr<IRNode>> ret;
	for (auto v : this->vars)
		ret.push_back(v);
	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRFrees::traverse()
{
	std::vector<std::shared_ptr<IRNode>> ret;
	for (auto v : this->vars)
		ret.push_back(v);
	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRFunctionRefExpr::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

template <typename T>
std::vector<std::shared_ptr<IRNode>> IRFunctionCallExpr<T>::traverse()
{
	auto ret = std::vector<std::shared_ptr<IRNode>>({ this->function });
	for (auto p : this->params)
		ret.push_back(p);
	return ret;
}

template <typename T>
std::vector<std::shared_ptr<IRNode>> IRMemberFunctionCallExpr<T>::traverse()
{
	auto ret = std::vector<std::shared_ptr<IRNode>>({ this->function, this->base_object });
	for (auto p : this->params)
		ret.push_back(p);
	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRVariableDeclarationStmt::traverse()
{
	if (this->initial_value)
		return std::vector<std::shared_ptr<IRNode>>({ this->var, this->initial_value });
	else
		return std::vector<std::shared_ptr<IRNode>>({ this->var, nullptr });
}

std::vector<std::shared_ptr<IRNode>> IRReturnStmt::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->ret });
}

std::vector<std::shared_ptr<IRNode>> IRStatementBlock::traverse()
{
	std::vector<std::shared_ptr<IRNode>> ret;
	for (auto s : this->stmts)
		ret.push_back(s);
	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRVariableList::traverse()
{
	std::vector<std::shared_ptr<IRNode>> ret;
	for (auto v : this->vars)
		ret.push_back(v);
	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRFunction::traverse()
{
	auto ret = std::vector<std::shared_ptr<IRNode>>({ this->vars, this->template_vars, this->body });
	for (auto v : this->vcs)
		ret.push_back(v);
	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRExpressionStmt::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->expr });
}

std::vector<std::shared_ptr<IRNode>> IRProgram::traverse()
{
	std::vector<std::shared_ptr<IRNode>> ret;
	for (auto g : this->globals)
		ret.push_back(g);
	return ret;
}

std::vector<std::shared_ptr<IRNode>> __IRExpressionListWrapper::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

std::vector<std::shared_ptr<IRVariable>> IRVariableList::get_vars()
{
	return this->vars;
}

std::vector<std::shared_ptr<IRVariable>> IRFunction::get_vars()
{
	return this->vars->get_vars();
}

std::shared_ptr<IRType> IRFunction::get_return_type()
{
	return this->return_type;
}

IRName IRFunction::get_name()
{
	return this->function_name;
}

void IRVariableReference::set_var(std::shared_ptr<IRVariable> var)
{
	this->type = var->get_type();
	this->variable = var;
}

template <typename T>
IRResult<T>::IRResult() : T(std::make_shared<IRUnknownType>()) { }

template <typename T>
std::string IRResult<T>::pp()
{
	return "result";
}

template <typename T>
std::vector<std::shared_ptr<IRNode>> IRResult<T>::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

IRClass::IRClass(std::vector<std::shared_ptr<IRVariableDeclarationStmt>> vars, std::vector<std::shared_ptr<IRFunction>> funcs, IRName name, std::shared_ptr<R2WML_Scope> scope) :
	IRScopedStatement(scope)
{
	this->vars = vars;
	this->funcs = funcs;
	this->name = name;
}

std::string IRClass::pp()
{
	std::string ret = "class " + this->name + " {\n";

	for (auto v : vars)
		ret += v->pp() + "\n";

	for (auto f : funcs)
		ret += "\n" + f->pp();

	ret += "\n}\n";

	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRClass::traverse()
{
	std::vector<std::shared_ptr<IRNode>> ret;

	for (auto v : vars)
		ret.push_back(v);

	for (auto f : funcs)
		ret.push_back(f);

	return ret;
}

void IRClass::set_name(IRName name)
{
	this->name = name;
}

IRName IRClass::get_name()
{
	return this->name;
}

std::vector<std::shared_ptr<IRFunction>> IRClass::get_funcs()
{
	return this->funcs;
}

std::vector<std::shared_ptr<IRVariableDeclarationStmt>> IRClass::get_vars()
{
	return this->vars;
}

void IRClass::set_func(std::shared_ptr<IRFunction> func, int idx)
{
	this->funcs[idx] = func;
}

extern FunctionTable function_table;

void IRClass::tidy_funcs()
{
	for (int i = 0; i < this->funcs.size(); i++)
	{
		auto func = this->funcs[i];

		if (!func->has_body() && !function_table.declared_without_definition(func->get_name()))
			this->funcs.erase(std::next(this->funcs.begin(), i--));
	}
}

IRNull::IRNull() : IRRValue(std::make_shared<IRVoidType>()) { }

std::string IRNull::pp()
{
	return "null";
}

std::vector<std::shared_ptr<IRNode>> IRNull::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

IRAddressOf::IRAddressOf(std::shared_ptr<IRType> type, std::shared_ptr<IRLValue> base_expr) :
	IRRValue(type)
{
	this->base_expr = base_expr;
}

std::string IRAddressOf::pp()
{
	return "&(" + this->base_expr->pp() + ")";
}

std::vector<std::shared_ptr<IRNode>> IRAddressOf::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->base_expr });
}

IRIfStatement::IRIfStatement(std::shared_ptr<IRExpression> cond, std::shared_ptr<IRStatement> then, std::shared_ptr<IRStatement> els)
{
	this->cond = cond;
	this->then = then;
	this->els = els;
}

std::string IRIfStatement::pp()
{
	std::string ret = "if (" + this->cond->pp() + ")\n";
	ret += this->then->pp();
	ret += "\n";

	if (this->els)
		ret += "else\n" + this->els->pp() + "\n";

	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRIfStatement::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->cond, this->then, this->els });
}

IRSizeOf::IRSizeOf(std::shared_ptr<IRType> type) : IRRValue(std::make_shared<IRU64Type>())
{
	this->contained_type = type;
}

std::shared_ptr<IRType> IRSizeOf::get_contained_type()
{
	return this->contained_type;
}

std::string IRSizeOf::pp()
{
	return "sizeof(" + this->contained_type->pp() + ")";
}

std::vector<std::shared_ptr<IRNode>> IRSizeOf::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

IRLoopStmt::IRLoopStmt(std::shared_ptr<R2WML_Scope> scope, std::shared_ptr<IRExpression> comp, std::shared_ptr<IRStatement> body) : IRScopedStatement(scope)
{
	this->comp = comp;
	this->body = body;
}

std::vector<std::shared_ptr<IRNode>> IRLoopStmt::traverse()
{
	auto ret = std::vector<std::shared_ptr<IRNode>>({ this->comp, this->body });
	for (auto vc : this->vcs)
		ret.push_back(vc);
	return ret;
}

std::string IRLoopStmt::pp()
{
	std::string ret;
	for (auto vc : this->vcs)
		ret += vc->pp() + "\n";
	ret += "while (" + this->comp->pp() + ")\n";
	ret += this->body->pp();
	return ret;
}

void IRLoopStmt::add_vc(std::shared_ptr<IRVerificationCondition> vc)
{
	this->vcs.push_back(vc);
}

std::vector<std::shared_ptr<IRVerificationCondition>> IRLoopStmt::get_vcs()
{
	return this->vcs;
}

IRScopedStatement::IRScopedStatement(std::shared_ptr<R2WML_Scope> scope)
{
	this->scope = scope;
}

std::shared_ptr<R2WML_Scope> IRScopedStatement::get_scope()
{
	return this->scope;
}

std::vector<std::shared_ptr<IRStatement>> IRProgram::get_globals()
{
	return this->globals;
}

std::vector<std::shared_ptr<IRClass>> IRProgram::get_classes()
{
	std::vector<std::shared_ptr<IRClass>> ret;

	for (auto g : this->globals)
		if (auto c = std::dynamic_pointer_cast<IRClass>(g))
			ret.push_back(c);

	return ret;
}

std::vector<std::shared_ptr<IRFunction>> IRProgram::get_functions()
{
	std::vector<std::shared_ptr<IRFunction>> ret;

	for (auto g : this->globals)
		if (auto c = std::dynamic_pointer_cast<IRFunction>(g))
			ret.push_back(c);

	return ret;
}

std::vector<std::shared_ptr<IRVariableDeclarationStmt>> IRProgram::get_variables()
{
	std::vector<std::shared_ptr<IRVariableDeclarationStmt>> ret;

	for (auto g : this->globals)
		if (auto c = std::dynamic_pointer_cast<IRVariableDeclarationStmt>(g))
			ret.push_back(c);

	return ret;
}

#ifdef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
IRNewExpr::IRNewExpr(std::shared_ptr<IRType> type, std::shared_ptr<IRExpression> base_object) : IRRValue(type)
{
	this->base_object = base_object;
}

std::string IRNewExpr::pp()
{
	return "new " + this->base_object->pp();
}

std::vector<std::shared_ptr<IRNode>> IRNewExpr::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->base_object });
}

IRDeleteExpr::IRDeleteExpr(std::shared_ptr<IRLValue> base_object) : IRRValue(std::make_shared<IRVoidType>())
{
	this->base_object = base_object;
}

std::string IRDeleteExpr::pp()
{
	return "delete " + this->base_object->pp();
}

std::vector<std::shared_ptr<IRNode>> IRDeleteExpr::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>({ this->base_object });
}
#endif

IRCase::IRCase(std::shared_ptr<IRExpression> expr, std::vector<std::shared_ptr<IRStatement>> stmts)
{
	this->expr = expr;
	this->stmts = stmts;
}

std::string IRCase::pp()
{
	std::string ret = "case " + this->expr->pp() + ":\n";
	for (auto s : this->stmts)
		ret += s->pp() + "\n";
	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRCase::traverse()
{
	auto ret = std::vector<std::shared_ptr<IRNode>>({ this->expr });

	for (auto s : this->stmts)
		ret.push_back(s);

	return ret;
}

void IRCase::add_statement(std::shared_ptr<IRStatement> stmt)
{
	this->stmts.push_back(stmt);
}

IRDefault::IRDefault(std::vector<std::shared_ptr<IRStatement>> stmts) :
	IRCase(nullptr, stmts) { }

std::string IRDefault::pp()
{
	std::string ret = "default:\n";
	for (auto s : this->stmts)
		ret += s->pp() + "\n";
	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRDefault::traverse()
{
	std::vector<std::shared_ptr<IRNode>> ret;

	for (auto s : this->stmts)
		ret.push_back(s);

	return ret;
}

IRSwitchStmt::IRSwitchStmt(std::shared_ptr<IRExpression> expr, std::vector<std::shared_ptr<IRCase>> cases, std::shared_ptr<R2WML_Scope> scope) :
	IRScopedStatement(scope)
{
	this->expr = expr;
	this->cases = cases;
}

std::string IRSwitchStmt::pp()
{
	std::string ret = "switch (" + this->expr->pp() + ") {\n";
	for (auto c : this->cases)
		ret += c->pp();
	ret += "}\n";
	return ret;
}

std::vector<std::shared_ptr<IRNode>> IRSwitchStmt::traverse()
{
	std::vector<std::shared_ptr<IRNode>> ret({ this->expr });

	for (auto c : this->cases)
		ret.push_back(c);

	return ret;
}

IRBreak::IRBreak() = default;

std::string IRBreak::pp()
{
	return "break;";
}

std::vector<std::shared_ptr<IRNode>> IRBreak::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

IRContinue::IRContinue() = default;

std::string IRContinue::pp()
{
	return "continue;";
}

std::vector<std::shared_ptr<IRNode>> IRContinue::traverse()
{
	return std::vector<std::shared_ptr<IRNode>>();
}

