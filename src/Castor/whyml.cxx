// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "whyml.hxx"
#include "exception.hxx"
#include "vc_generator.hxx"
#include "settings.hxx"
#include "helper.cxx"
#include <iostream>

extern Settings settings;

using namespace Why3;

WhyNode::WhyNode() = default;

WhyType::WhyType()
{
	this->is_const = false;
	this->is_reference = false;
};

std::string WhyType::pp()
{
	return "MemoryModel.var";
}

bool WhyType::get_const()
{
	return this->is_const;
}

void WhyType::set_const()
{
	this->is_const = true;
}

bool WhyType::get_reference()
{
	return this->is_reference;
}

void WhyType::set_reference()
{
	this->is_reference = true;
}

WhyLabelType::WhyLabelType() = default;

std::string WhyLabelType::pp()
{
	throw ImproperLabelUsageException();
}

int WhyLabelType::get_size()
{
	throw ImproperLabelUsageException();
}

WhyUnitType::WhyUnitType() = default;

std::string WhyUnitType::pp()
{
	return "()";
}

int WhyUnitType::get_size()
{
	return 0;
}

WhyUnknownType::WhyUnknownType() = default;

std::string WhyUnknownType::pp()
{
	return "UNKNOWN";
}

int WhyUnknownType::get_size()
{
	return 0;
}

WhyBoolType::WhyBoolType() = default;

int WhyBoolType::get_size()
{
	return 1;
}

WhyClassType::WhyClassType(WhyName class_name, OffsetTable offset_table)
{
	this->class_name = class_name;
	this->offset_table = offset_table;
}

int WhyClassType::get_size()
{
	return this->offset_table.get_size();
}

OffsetTable WhyClassType::get_offset_table()
{
	return this->offset_table;
}

WhyIntegralType::WhyIntegralType(int bits, bool is_signed)
{
	this->bits = bits;
	this->is_signed = is_signed;
}

int WhyIntegralType::get_size()
{
	return 1;
}

int WhyIntegralType::get_bits()
{
	return this->bits;
}

bool WhyIntegralType::get_is_signed()
{
	return this->is_signed;
}

WhyS64Type::WhyS64Type() : WhyIntegralType(64, true) { }

WhyS32Type::WhyS32Type() : WhyIntegralType(32, true) { }

WhyS16Type::WhyS16Type() : WhyIntegralType(16, true) { }

WhyS8Type::WhyS8Type() : WhyIntegralType(8, true) { }

WhyU64Type::WhyU64Type() : WhyIntegralType(64, false) { }

WhyU32Type::WhyU32Type() : WhyIntegralType(32, false) { }

WhyU16Type::WhyU16Type() : WhyIntegralType(16, false) { }

WhyU8Type::WhyU8Type() : WhyIntegralType(8, false) { }

WhyUnboundedIntegralType::WhyUnboundedIntegralType() : WhyIntegralType(INT32_MAX, true) { }

WhyPointerType::WhyPointerType(std::shared_ptr<WhyType> base_type)
{
	this->base_type = base_type;
}

std::shared_ptr<WhyType> WhyPointerType::get_base_type()
{
	return this->base_type;
}

WhyArrayType::WhyArrayType(std::shared_ptr<WhyType> base_type, int length) : WhyPointerType(base_type)
{
	this->length = length;
}

int WhyArrayType::get_length()
{
	return this->length;
}

int WhyArrayType::get_size()
{
	return this->base_type->get_size() * this->length;
}

std::string __WhyVariableList::pp()
{
	return "";
}

std::string __WhyFunctions::pp()
{
	return "";
}

WhyExpression::WhyExpression(std::shared_ptr<WhyType> type)
{
	this->type = type;
}

std::shared_ptr<WhyType> WhyExpression::get_type()
{
	return this->type;
}

WhyLValue::WhyLValue(std::shared_ptr<WhyType> type) : WhyExpression(type) { }

WhyRValue::WhyRValue(std::shared_ptr<WhyType> type) : WhyExpression(type) { }

template <typename T>
WhyTernary<T>::WhyTernary(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> condition, std::shared_ptr<T> then, std::shared_ptr<T> els) :
	T(type)
{
	this->condition = condition;
	this->then = then;
	this->els = els;
}

template <typename T>
std::string WhyTernary<T>::pp()
{
	return "(if " + this->condition->pp() + " then " + this->then->pp() + " else " + this->els->pp() + ")";
}

template <typename T>
std::vector<std::shared_ptr<WhyNode>> WhyTernary<T>::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->condition, this->then, this->els });
}

#ifdef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
WhyNew::WhyNew(std::shared_ptr<WhyType> type, std::shared_ptr<WhyExpression> constructed_object) : WhyRValue(type)
{
	this->constructed_object = constructed_object;
}

std::string WhyNew::pp()
{
	std::string ret;
	auto base_type = this->constructed_object->get_type();
	auto is_const = safety_cast<WhyPointerType>(this->get_type())->get_base_type()->get_const();

	if (std::dynamic_pointer_cast<WhyLValue>(this->constructed_object))
		ret = "(" + this->constructed_object->pp() + ")";
	else if (std::dynamic_pointer_cast<WhyIntegralType>(base_type))
		ret = "(MemoryModel.create_val " + this->constructed_object->pp() + ")";
	else if (std::dynamic_pointer_cast<WhyBoolType>(base_type))
		ret = "(MemoryModel.create_val (ArithmeticModel.bool_2_var " +
			this->constructed_object->pp() + "))";
	else if (std::dynamic_pointer_cast<WhyClassType>(base_type))
		ret = "(MemoryModel.create_slice (" + this->constructed_object->pp() + "))";
	else
		throw CastorException("Tell Phil about this exception, and tell him this type was found -> " +
				this->constructed_object->get_type()->pp());

	if (is_const)
		ret = "(let __base_obj = " + ret + " in (MemoryModel.set_const __base_obj " +
			std::to_string(base_type->get_size()) + "); __base_obj)";

	ret = "((" + ret + ").addr)";
	return ret;
}

WhyDelete::WhyDelete(std::shared_ptr<WhyLValue> base_item) : WhyRValue(std::make_shared<WhyUnitType>())
{
	this->base_item = base_item;
}

std::string WhyDelete::pp()
{
	return "(MemoryModel.free " + this->base_item->pp() + " " +
		std::to_string(safety_cast<WhyPointerType>(this->base_item->get_type())->get_base_type()->get_size()) + ")";
}
#endif

WhyLabel::WhyLabel(WhyName name) : WhyRValue(std::make_shared<WhyLabelType>())
{
	this->name = name;
}

std::string WhyLabel::pp()
{
	return this->name;
}

WhyQuantifier::WhyQuantifier(std::vector<std::shared_ptr<WhyVariable>> vars, std::shared_ptr<WhyRValue> expr) :
	WhyRValue(std::make_shared<WhyBoolType>())
{
	this->vars = vars;
	this->expr = expr;
}

std::vector<std::shared_ptr<WhyVariable>> WhyQuantifier::get_vars()
{
	return this->vars;
}

WhyForall::WhyForall(std::vector<std::shared_ptr<WhyVariable>> vars, std::shared_ptr<WhyRValue> expr) :
	WhyQuantifier(vars, expr)
{
}

std::string WhyForall::pp()
{
	std::string ret = "(forall ";
	for (auto v : this->vars)
		if (std::dynamic_pointer_cast<WhyIntegralType>(v->get_type()))
			ret += v->get_name() + " : int, ";
		else
			throw CastorException("Expected integral type in quantifier!");
	
	ret += "__:bool. (";

	for (auto v : this->vars)
	{
		auto type = safety_cast<WhyIntegralType>(v->get_type());

		if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(type)) continue;

		ret += std::string("is_") + (type->get_is_signed() ? "s" : "u") + "int" +
			std::to_string(type->get_bits()) + "(" + v->get_name() + ") /\\ ";
	}

	ret += "true) -> " + this->expr->pp() + ")";
	return ret;
}

WhyExists::WhyExists(std::vector<std::shared_ptr<WhyVariable>> vars, std::shared_ptr<WhyRValue> expr) :
	WhyQuantifier(vars, expr)
{
}

std::string WhyExists::pp()
{
	std::string ret = "(exists ";
	for (auto v : this->vars)
		if (std::dynamic_pointer_cast<WhyIntegralType>(v->get_type()))
			ret += v->get_name() + " : int, ";
		else
			throw CastorException("Expected integral type in quantifier!");
	
	ret += "__:bool. ";

	for (auto v : this->vars)
	{
		auto type = safety_cast<WhyIntegralType>(v->get_type());

		if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(type)) continue;

		ret += std::string("is_") + (type->get_is_signed() ? "s" : "u") + "int" +
			std::to_string(type->get_bits()) + "(" + v->get_name() + ") /\\ ";
	}

	ret += this->expr->pp() + ")";
	return ret;
}

template <typename T>
WhyResult<T>::WhyResult(std::shared_ptr<WhyType> type) : T(type) { }

template <typename T>
std::string WhyResult<T>::pp()
{
	return "result";
}

template <typename T>
WhyCast<T>::WhyCast(std::shared_ptr<WhyType> type, std::shared_ptr<T> base_object) :
	T(type)
{
	this->base_object = base_object;
}

template <>
std::string WhyCast<WhyRValue>::pp()
{
	auto to_type = this->type;
	auto from_type = this->base_object->get_type();

	auto integral_to = std::dynamic_pointer_cast<WhyIntegralType>(to_type);
	auto integral_from = std::dynamic_pointer_cast<WhyIntegralType>(from_type);

	auto pointer_to = std::dynamic_pointer_cast<WhyPointerType>(to_type);
	auto pointer_from = std::dynamic_pointer_cast<WhyPointerType>(from_type);

	auto bool_to = std::dynamic_pointer_cast<WhyBoolType>(to_type);
	auto bool_from = std::dynamic_pointer_cast<WhyBoolType>(from_type);

	auto class_to = std::dynamic_pointer_cast<WhyClassType>(to_type);
	auto class_from = std::dynamic_pointer_cast<WhyClassType>(from_type);

	if (auto literal_type = std::dynamic_pointer_cast<WhyLiteral<int32_t>>(this->base_object);
			literal_type && literal_type->get_literal() == 0 && pointer_to)
	{
		// frontend normalization can sometimes transform nullptr into a 0 literal
		// we accept casting a 0 literal to a pointer
		return this->base_object->pp();
	}
	else if (pointer_to && pointer_from)
	{
		// TODO: Implement proper support for this type of cast
		//throw CastorException("Pointer conversions are not yet supported!");
		return this->base_object->pp();
	}
	else if (pointer_to || pointer_from)
	{
		throw UnsupportedFeatureException("Pointer <-> Non-pointer conversions");
	}
	else if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(integral_to) && integral_from)
	{
		return this->base_object->pp();
	}
	else if (integral_to && integral_from)
	{
		return std::string("(ArithmeticModel.to_") +
			(integral_to->get_is_signed() ? "s" : "u") +
			"int" +
			std::to_string(integral_to->get_bits()) +
			" " + this->base_object->pp() + ")";
	}
	else if (bool_to && integral_from)
	{
		return "(ArithmeticModel.var_2_bool " + this->base_object->pp() + ")";
	}
	else if (integral_to && bool_from)
	{
		return "(ArithmeticModel.bool_2_var " + this->base_object->pp() + ")";
	}
	else if (class_to && class_from)
	{
		return "(MemoryModel.upcast " + this->base_object->pp() + " " + std::to_string(class_to->get_size()) + ")";
	}
	else
	{
		throw UnsupportedFeatureException("This type of cast");
	}
}

template <>
std::string WhyCast<WhyLValue>::pp()
{
	return this->base_object->pp();
}

WhyMemoryGetExpr::WhyMemoryGetExpr(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base_item) :
	WhyRValue(type)
{
	this->base_item = base_item;
}

std::string WhyMemoryGetExpr::pp()
{
	if (std::dynamic_pointer_cast<WhyBoolType>(this->base_item->get_type()))
		return "(ArithmeticModel.var_2_bool (MemoryModel.get tape read " + this->base_item->pp() + "))";
	else if (std::dynamic_pointer_cast<WhyArrayType>(this->base_item->get_type()))
		return "((" + this->base_item->pp() + ").addr)";
	else if (auto var = std::dynamic_pointer_cast<WhyVariableReference>(this->base_item);
			var && var->get_in_quantifier())
		return this->base_item->pp();
	else if (std::dynamic_pointer_cast<WhyClassType>(this->base_item->get_type()))
		return "(MemoryModel.get_slice tape read " + this->base_item->pp() + " " +
			std::to_string(this->base_item->get_type()->get_size()) + ")";
	else
		return "(MemoryModel.get tape read " + this->base_item->pp() + ")";
}

WhyMaterialize::WhyMaterialize(std::shared_ptr<WhyRValue> base_expr)
	: WhyLValue(base_expr->get_type())
{
	this->base_expr = base_expr;
}

std::string WhyMaterialize::pp()
{
	if (std::dynamic_pointer_cast<WhyClassType>(base_expr->get_type()))
		return "(MemoryModel.create_slice " + this->base_expr->pp() + ")";
	else
		return "(MemoryModel.create_val " + this->base_expr->pp() + ")";
}

std::vector<std::shared_ptr<WhyNode>> WhyMaterialize::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ base_expr });
}

WhyAddressOfExpr::WhyAddressOfExpr(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base_item) :
	WhyRValue(type)
{
	this->base_item = base_item;
}

std::string WhyAddressOfExpr::pp()
{
	return "((" + this->base_item->pp() + ").addr)";
}

WhyValid::WhyValid(std::vector<std::shared_ptr<WhyExpression>> base_items) : WhyRValue(std::make_shared<WhyBoolType>())
{
	this->base_items = base_items;
}

void WhyValid::set_vars(std::vector<std::shared_ptr<WhyVariable>> vars)
{
	this->vars = vars;
}

std::string WhyValid::pp()
{
	std::string ret;

	for (int i = 0; i < this->base_items.size() - 1; i++)
		ret += generate_valid(this->vars, this->base_items[i]) + " /\\ ";
	ret += generate_valid(this->vars, this->base_items[this->base_items.size() - 1]);
	return ret;
}

WhyNth::WhyNth(std::shared_ptr<WhyRValue> base, std::shared_ptr<WhyRValue> idx)
	: WhyRValue(std::make_shared<WhyBoolType>())
{
	this->base = base;
	this->idx = idx;
}

std::string WhyNth::pp()
{
	auto bits = safety_cast<WhyIntegralType>(this->base->get_type())->get_bits();

	if (bits != 8 && bits != 16 && bits != 32 && bits != 64)
		throw CastorException("Cannot use nth() on an unbounded integer!");

	return std::string("(ArithmeticModel.nth_") + std::to_string(bits) +
		" " + this->base->pp() + " " + this->idx->pp() + ")";
}

WhyFreed::WhyFreed(std::vector<std::shared_ptr<WhyLValue>> base_items) : WhyRValue(std::make_shared<WhyBoolType>())
{
	this->base_items = base_items;
}

std::string WhyFreed::pp()
{
	std::string ret = "(";
	for (auto b : this->base_items)
		ret += "(not read.data[tape[" + b->pp() + ".addr]] /\\ not write.data[tape[" + b->pp() + ".addr]]) /\\ ";
	ret += "true)";
	return ret;
}

WhyOld::WhyOld(std::shared_ptr<WhyRValue> base_item) : WhyRValue(base_item->get_type())
{
	this->base_item = base_item;
}

std::string WhyOld::pp()
{
	return "(old " + this->base_item->pp() + ")";
}

WhyUnchanged::WhyUnchanged(std::shared_ptr<WhyRValue> base_item) : WhyRValue(std::make_shared<WhyBoolType>())
{
	this->base_item = base_item;
}

std::string WhyUnchanged::pp()
{
	if (std::dynamic_pointer_cast<WhyClassType>(this->base_item->get_type()))
		return "(MemoryModel.eq " + this->base_item->pp() + " (old " + this->base_item->pp() + "))";
	else
		return "(" + this->base_item->pp() + " = (old " + this->base_item->pp() + "))";
}

WhyAliasOf::WhyAliasOf(std::shared_ptr<WhyLValue> base_item, std::shared_ptr<WhyLValue> aliased_item) : WhyRValue(std::make_shared<WhyBoolType>())
{
	this->base_item = base_item;
	this->aliased_item = aliased_item;
}

std::string WhyAliasOf::pp()
{
	return "(" + this->base_item->pp() + ".addr = " + this->aliased_item->pp() + ".addr)";
}

WhyAt::WhyAt(std::shared_ptr<WhyRValue> base_item, std::shared_ptr<WhyLabel> label) : WhyRValue(std::make_shared<WhyBoolType>())
{
	this->base_item = base_item;
	this->label = label;
}

std::string WhyAt::pp()
{
	return "(" + this->base_item->pp() + " at " + this->label->pp() + ")";
}

WhyValidArray::WhyValidArray(std::shared_ptr<WhyExpression> base_item, std::shared_ptr<WhyRValue> size) : WhyRValue(std::make_shared<WhyBoolType>())
{
	this->base_item = base_item;
	this->size = size;
}

void WhyValidArray::set_vars(std::vector<std::shared_ptr<WhyVariable>> vars)
{
	this->vars = vars;
}

std::string WhyValidArray::pp()
{
	return generate_valid_contiguous(this->vars, this->base_item, this->size);
}

WhySeparated::WhySeparated(std::vector<std::shared_ptr<WhyExpression>> base_items) : WhyRValue(std::make_shared<WhyBoolType>())
{
	this->base_items = base_items;
}

void WhySeparated::set_vars(std::vector<std::shared_ptr<WhyVariable>> vars)
{
	this->vars = vars;
}

std::string WhySeparated::pp()
{
	return generate_separated(this->vars, this->base_items);
}

WhyVariable::WhyVariable(std::shared_ptr<WhyType> type, WhyName name)
{
	this->type = type;
	this->name = name;
	this->literal_init = nullptr;
}

std::shared_ptr<WhyType> WhyVariable::get_type()
{
	return this->type;
}

WhyName WhyVariable::get_name()
{
	return this->name;
}

bool WhyVariable::get_const()
{
	return this->type->get_const();
}

std::string WhyVariable::pp()
{
	throw CastorException("Should never call WhyVariable's pretty-printer!");
}

void WhyVariable::add_literal_init(std::shared_ptr<WhyRValue> literal_init)
{
	this->literal_init = literal_init;
}

std::shared_ptr<WhyRValue> WhyVariable::get_literal_init()
{
	return this->literal_init;
}

WhyVariableReference::WhyVariableReference(std::shared_ptr<WhyVariable> variable) :
	WhyLValue(variable->get_type())
{
	this->variable = variable;
	this->in_quantifier = false;
}

void WhyVariableReference::set_in_quantifier()
{
	this->in_quantifier = true;
}

bool WhyVariableReference::get_in_quantifier()
{
	return this->in_quantifier;
}

std::string WhyVariableReference::pp()
{
	return this->variable->get_name();
}

WhyArrayIndex::WhyArrayIndex(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base_object, std::shared_ptr<WhyRValue> index) :
	WhyLValue(type)
{
	this->base_object = base_object;
	this->index = index;
}

std::string WhyArrayIndex::pp()
{
	return "(MemoryModel.deref_offset " + this->base_object->pp() + " (" + this->index->pp() + " * " +
		std::to_string(safety_cast<WhyPointerType>(this->base_object->get_type())->get_base_type()->get_size()) + "))";
}

WhyArrayRange::WhyArrayRange(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base_object, std::shared_ptr<WhyRValue> start, std::shared_ptr<WhyRValue> end) :
	WhyLValue(type)
{
	this->base_object = base_object;
	this->start = start;
	this->end = end;
}

std::string WhyArrayRange::pp()
{
	throw CastorException("Should never call WhyArrayRange's pretty-printer!");
}

std::string WhyArrayRange::generate_writes()
{
	auto size = std::to_string(safety_cast<WhyPointerType>(this->base_object->get_type())->get_base_type()->get_size());
	return "(__r2wmli < " + this->base_object->pp() + " + (" + this->start->pp() + " * " + size + ") \\/ __r2wmli > " +
		this->base_object->pp() + " + (" + this->end->pp() + " * " + size + ")) /\\ ";
}

WhyPointerDereference::WhyPointerDereference(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base_item) :
	WhyLValue(type)
{
	this->base_item = base_item;
}

std::string WhyPointerDereference::pp()
{
	return "(MemoryModel.deref " + this->base_item->pp() + ")";
}

WhyReference::WhyReference(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base_item) :
	WhyLValue(type)
{
	this->base_item = base_item;
}

std::string WhyReference::pp()
{
	return this->base_item->pp();
}

WhyFieldReference::WhyFieldReference(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base_item, int offset) :
	WhyLValue(type)
{
	this->base_item = base_item;
	this->offset = offset;
}

std::string WhyFieldReference::pp()
{
	return "(MemoryModel.offset " + this->base_item->pp() + " " + std::to_string(this->offset) + ")";
}

WhyFieldReferenceRValue::WhyFieldReferenceRValue(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base_item, int offset) :
	WhyRValue(type)
{
	this->base_item = base_item;
	this->offset = offset;
}

std::string WhyFieldReferenceRValue::pp()
{
	return "(Array.([]) " + this->base_item->pp() + " " + std::to_string(this->offset) + ")";
}

template <typename T>
WhyLiteral<T>::WhyLiteral(std::shared_ptr<WhyType> type, T literal) :
	WhyRValue(type)
{
	this->literal = literal;
}

template <typename T>
T WhyLiteral<T>::get_literal()
{
	return this->literal;
}

template <typename T>
std::string WhyLiteral<T>::pp()
{
	return "(" + std::to_string(this->literal) + " : int)";
}

template <>
std::string WhyLiteral<bool>::pp()
{
	return this->literal ? "true" : "false";
}

template class WhyLiteral<int8_t>;

template class WhyLiteral<uint8_t>;

template class WhyLiteral<int16_t>;

template class WhyLiteral<uint16_t>;

template class WhyLiteral<int32_t>;

template class WhyLiteral<uint32_t>;

template class WhyLiteral<int64_t>;

template class WhyLiteral<uint64_t>;

template class WhyLiteral<bool>;

WhyArbitraryLiteral::WhyArbitraryLiteral(int minimum) :
	WhyLiteral(std::make_shared<WhyU64Type>(), minimum) { }

std::string WhyArbitraryLiteral::pp()
{
	return "(ArithmeticModel.to_uint64 (ArithmeticModel.arbitrary " + std::to_string(this->literal) + "))";
}

WhyBinaryOperation::WhyBinaryOperation(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyRValue(type)
{
	this->lhs = lhs;
	this->rhs = rhs;
}

std::string WhyBinaryOperation::get_suffix()
{
	auto type = safety_cast<WhyIntegralType>(this->get_type());

	return (type->get_is_signed() ? "s" : "u") + std::to_string(type->get_bits());
}

WhyIncrementOp::WhyIncrementOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base) :
	WhyRValue(type)
{
	this->base = base;
}

std::string WhyIncrementOp::pp()
{
	auto base_var = std::make_shared<WhyVariableReference>(
			std::make_shared<WhyVariable>(this->type, "__transient"));

	return "(let __transient = " + this->base->pp() + " in " +
		"let __premodification = " +
		WhyMemoryGetExpr(this->type, base_var).pp() + " in " +
		"let __ = " + WhyCompoundAssignOp<WhyAddOp>(this->type, base_var,
			std::make_shared<WhyLiteral<int>>(std::make_shared<WhyS32Type>(), 1)).pp() +
		" in __premodification)";
}

WhyDecrementOp::WhyDecrementOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> base) :
	WhyRValue(type)
{
	this->base = base;
}

std::string WhyDecrementOp::pp()
{
	auto base_var = std::make_shared<WhyVariableReference>(
			std::make_shared<WhyVariable>(this->type, "__transient"));

	return "(let __transient = " + this->base->pp() + " in " +
		"let __premodification = " +
		WhyMemoryGetExpr(this->type, base_var).pp() + " in " +
		"let __ = " + WhyCompoundAssignOp<WhySubtractOp>(this->type, base_var,
			std::make_shared<WhyLiteral<int>>(std::make_shared<WhyS32Type>(), 1)).pp() +
		" in __premodification)";
}

WhyUnaryOperation::WhyUnaryOperation(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base) :
	WhyRValue(type)
{
	this->base = base;
}

std::string WhyUnaryOperation::get_suffix()
{
	auto type = safety_cast<WhyIntegralType>(this->get_type());

	return (type->get_is_signed() ? "s" : "u") + std::to_string(type->get_bits());
}

WhyAssignOp::WhyAssignOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyLValue(type)
{
	this->lhs = lhs;
	this->rhs = rhs;
}

std::string WhyAssignOp::pp()
{
	if (std::dynamic_pointer_cast<WhyClassType>(this->lhs->get_type()))
		return "(MemoryModel.set_slice " + this->lhs->pp() + " " + this->rhs->pp() + ")";
	else if (std::dynamic_pointer_cast<WhyBoolType>(this->lhs->get_type()))
		return "(MemoryModel.set " + this->lhs->pp() + " (ArithmeticModel.bool_2_var " + this->rhs->pp() + "))";
	else
		return "(MemoryModel.set " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

template <typename T>
WhyCompoundAssignOp<T>::WhyCompoundAssignOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyLValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyLValue(type)
{
	this->lhs = lhs;

	auto variable = std::make_shared<WhyVariable>(type, "__tempassign");
	auto variable_reference = std::make_shared<WhyVariableReference>(variable);

	this->base_assignment = std::make_shared<WhyAssignOp>(type,
			variable_reference,
			std::make_shared<T>(
				type,
				std::make_shared<WhyMemoryGetExpr>(variable->get_type(), variable_reference),
				rhs));
}

template class WhyCompoundAssignOp<WhyAddOp>;
template class WhyCompoundAssignOp<WhySubtractOp>;
template class WhyCompoundAssignOp<WhyMultiplyOp>;
template class WhyCompoundAssignOp<WhyDivideOp>;
template class WhyCompoundAssignOp<WhyModuloOp>;
template class WhyCompoundAssignOp<WhyBitwiseAndOp>;
template class WhyCompoundAssignOp<WhyBitwiseOrOp>;
template class WhyCompoundAssignOp<WhyBitwiseXorOp>;
template class WhyCompoundAssignOp<WhyBitwiseLeftShiftOp>;
template class WhyCompoundAssignOp<WhyBitwiseArithmeticRightShiftOp>;
template class WhyCompoundAssignOp<WhyBitwiseLogicalRightShiftOp>;

template <typename T>
std::string WhyCompoundAssignOp<T>::pp()
{
	return "(let __tempassign = " + this->lhs->pp() + " in " + this->base_assignment->pp() + ")";
}

template <typename T>
WhyCommaOp<T>::WhyCommaOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<T> rhs) :
	T(type)
{
	this->lhs = lhs;
	this->rhs = rhs;
}

template <typename T>
std::string WhyCommaOp<T>::pp()
{
	return "(let __discardlhs = " + this->lhs->pp() + " in " + this->rhs->pp() + ")";
}

template class WhyCommaOp<WhyLValue>;
template class WhyCommaOp<WhyRValue>;

WhyAddOp::WhyAddOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(type, lhs, rhs)
{
}

std::string WhyAddOp::pp()
{
	if (auto type = std::dynamic_pointer_cast<WhyPointerType>(this->lhs->get_type());
			type && !std::dynamic_pointer_cast<WhyPointerType>(this->rhs->get_type()))
		return "(" + this->lhs->pp() + " + (" + this->rhs->pp() + " * " + std::to_string(type->get_base_type()->get_size()) + "))";
	else if (auto type = std::dynamic_pointer_cast<WhyPointerType>(this->rhs->get_type());
			type && !std::dynamic_pointer_cast<WhyPointerType>(this->lhs->get_type()))
		return "(" + this->rhs->pp() + " + (" + this->lhs->pp() + " * " + std::to_string(type->get_base_type()->get_size()) + "))";
	else if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		return "(" + this->lhs->pp() + " + " + this->rhs->pp() + ")";
	else
		return "(ArithmeticModel.add_" + this->get_suffix() + " " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

WhySubtractOp::WhySubtractOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(type, lhs, rhs)
{
}

std::string WhySubtractOp::pp()
{
	if (auto type = std::dynamic_pointer_cast<WhyPointerType>(this->lhs->get_type());
			type && !std::dynamic_pointer_cast<WhyPointerType>(this->rhs->get_type()))
		return "(" + this->lhs->pp() + " - (" + this->rhs->pp() + " * " + std::to_string(type->get_base_type()->get_size()) + "))";
	else if (auto type = std::dynamic_pointer_cast<WhyPointerType>(this->rhs->get_type());
			type && !std::dynamic_pointer_cast<WhyPointerType>(this->lhs->get_type()))
		return "(" + this->rhs->pp() + " - (" + this->lhs->pp() + " * " + std::to_string(type->get_base_type()->get_size()) + "))";
	else if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		return "(" + this->lhs->pp() + " - " + this->rhs->pp() + ")";
	else
		return "(ArithmeticModel.sub_" + this->get_suffix() + " " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

WhyMultiplyOp::WhyMultiplyOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(type, lhs, rhs)
{
}

std::string WhyMultiplyOp::pp()
{
	if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		return "(" + this->lhs->pp() + " * " + this->rhs->pp() + ")";
	else
		return "(ArithmeticModel.mul_" + this->get_suffix() + " " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

WhyDivideOp::WhyDivideOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(type, lhs, rhs)
{
}

std::string WhyDivideOp::pp()
{
	if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		return "(" + this->lhs->pp() + " / " + this->rhs->pp() + ")";
	else
		return "(ArithmeticModel.div_" + this->get_suffix() + " " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

WhyModuloOp::WhyModuloOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(type, lhs, rhs)
{
}

std::string WhyModuloOp::pp()
{
	if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		return "(ComputerDivision.mod " + this->lhs->pp() + " " + this->rhs->pp() + ")";
	else
		return "(ArithmeticModel.mod_" + this->get_suffix() + " " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

WhyLessThanOp::WhyLessThanOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(std::make_shared<WhyBoolType>(), lhs, rhs)
{
}

std::string WhyLessThanOp::pp()
{
	return "(" + this->lhs->pp() + " < " + this->rhs->pp() + ")";
}

WhyLessEqualsOp::WhyLessEqualsOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(std::make_shared<WhyBoolType>(), lhs, rhs)
{
}

std::string WhyLessEqualsOp::pp()
{
	return "(" + this->lhs->pp() + " <= " + this->rhs->pp() + ")";
}

WhyGreaterThanOp::WhyGreaterThanOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(std::make_shared<WhyBoolType>(), lhs, rhs)
{
}

std::string WhyGreaterThanOp::pp()
{
	return "(" + this->lhs->pp() + " > " + this->rhs->pp() + ")";
}

WhyGreaterEqualsOp::WhyGreaterEqualsOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(std::make_shared<WhyBoolType>(), lhs, rhs)
{
}

std::string WhyGreaterEqualsOp::pp()
{
	return "(" + this->lhs->pp() + " >= " + this->rhs->pp() + ")";
}

WhyEqualsOp::WhyEqualsOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(std::make_shared<WhyBoolType>(), lhs, rhs)
{
}

std::string WhyEqualsOp::pp()
{
	if (std::dynamic_pointer_cast<WhyClassType>(this->lhs->get_type()))
		return "(MemoryModel.eq " + this->lhs->pp() + " " + this->rhs->pp() + ")";
	else
		return "(" + this->lhs->pp() + " = " + this->rhs->pp() + ")";
}

WhyNotEqualsOp::WhyNotEqualsOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(std::make_shared<WhyBoolType>(), lhs, rhs)
{
}

std::string WhyNotEqualsOp::pp()
{
	if (std::dynamic_pointer_cast<WhyClassType>(this->lhs->get_type()))
		return "(not (MemoryModel.eq " + this->lhs->pp() + " " + this->rhs->pp() + "))";
	else
		return "(" + this->lhs->pp() + " <> " + this->rhs->pp() + ")";
}

WhyImpliesOp::WhyImpliesOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(std::make_shared<WhyBoolType>(), lhs, rhs)
{
}

std::string WhyImpliesOp::pp()
{
	return "(" + this->lhs->pp() + " -> " + this->rhs->pp() + ")";
}

WhyAndOp::WhyAndOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(std::make_shared<WhyBoolType>(), lhs, rhs)
{
}

std::string WhyAndOp::pp()
{
	return "(" + this->lhs->pp() + " /\\ " + this->rhs->pp() + ")";
}

WhyOrOp::WhyOrOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(std::make_shared<WhyBoolType>(), lhs, rhs)
{
}

std::string WhyOrOp::pp()
{
	return "(" + this->lhs->pp() + " \\/ " + this->rhs->pp() + ")";
}

WhyShortCircuitAndOp::WhyShortCircuitAndOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(std::make_shared<WhyBoolType>(), lhs, rhs)
{
}

std::string WhyShortCircuitAndOp::pp()
{
	return "(" + this->lhs->pp() + " && " + this->rhs->pp() + ")";
}

WhyShortCircuitOrOp::WhyShortCircuitOrOp(std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(std::make_shared<WhyBoolType>(), lhs, rhs)
{
}

std::string WhyShortCircuitOrOp::pp()
{
	return "(" + this->lhs->pp() + " || " + this->rhs->pp() + ")";
}

WhyBitwiseAndOp::WhyBitwiseAndOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(type, lhs, rhs)
{
}

std::string WhyBitwiseAndOp::pp()
{
	if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		throw UnsupportedFeatureException("Bitwise arithmetic in No Overflow mode");
	else
		return "(ArithmeticModel.and_" + this->get_suffix() + " " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

WhyBitwiseOrOp::WhyBitwiseOrOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(type, lhs, rhs)
{
}

std::string WhyBitwiseOrOp::pp()
{
	if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		throw UnsupportedFeatureException("Bitwise arithmetic in No Overflow mode");
	else
		return "(ArithmeticModel.or_" + this->get_suffix() + " " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

WhyBitwiseXorOp::WhyBitwiseXorOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(type, lhs, rhs)
{
}

std::string WhyBitwiseXorOp::pp()
{
	if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		throw UnsupportedFeatureException("Bitwise arithmetic in No Overflow mode");
	else
		return "(ArithmeticModel.xor_" + this->get_suffix() + " " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

WhyBitwiseLeftShiftOp::WhyBitwiseLeftShiftOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(type, lhs, rhs)
{
}

std::string WhyBitwiseLeftShiftOp::pp()
{
	if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		throw UnsupportedFeatureException("Bitwise arithmetic in No Overflow mode");
	else
		return "(ArithmeticModel.lsl_" + this->get_suffix() + " " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

WhyBitwiseArithmeticRightShiftOp::WhyBitwiseArithmeticRightShiftOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(type, lhs, rhs)
{
}

std::string WhyBitwiseArithmeticRightShiftOp::pp()
{
	if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		throw UnsupportedFeatureException("Bitwise arithmetic in No Overflow mode");
	else
		return "(ArithmeticModel.asr_" + this->get_suffix() + " " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

WhyBitwiseLogicalRightShiftOp::WhyBitwiseLogicalRightShiftOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> lhs, std::shared_ptr<WhyRValue> rhs) :
	WhyBinaryOperation(type, lhs, rhs)
{
}

std::string WhyBitwiseLogicalRightShiftOp::pp()
{
	if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		throw UnsupportedFeatureException("Bitwise arithmetic in No Overflow mode");
	else
		return "(ArithmeticModel.lsr_" + this->get_suffix() + " " + this->lhs->pp() + " " + this->rhs->pp() + ")";
}

WhyNegationOp::WhyNegationOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base) :
	WhyUnaryOperation(type, base)
{
}

std::string WhyNegationOp::pp()
{
	if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		return "(-" + this->base->pp() + ")";
	else
		return "(ArithmeticModel.ineg_" + this->get_suffix() + " " + this->base->pp() + ")";
}

WhyBoolNegationOp::WhyBoolNegationOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base) :
	WhyUnaryOperation(type, base)
{
}

std::string WhyBoolNegationOp::pp()
{
	return "(not " + this->base->pp() + ")";
}

WhyBitNegationOp::WhyBitNegationOp(std::shared_ptr<WhyType> type, std::shared_ptr<WhyRValue> base) :
	WhyUnaryOperation(type, base)
{
}

std::string WhyBitNegationOp::pp()
{
	if (std::dynamic_pointer_cast<WhyUnboundedIntegralType>(this->type))
		throw UnsupportedFeatureException("Bitwise arithmetic in No Overflow mode");
	else
		return "(ArithmeticModel.neg_" + this->get_suffix() + " " + this->base->pp() + ")";
}

std::string __WhyFunctionRef::pp()
{
	return "";
}

template <typename T>
WhyFunctionCall<T>::WhyFunctionCall(std::shared_ptr<WhyType> type, WhyName name, std::vector<std::shared_ptr<WhyExpression>> params) :
	T(type)
{
	this->name = name;
	this->params = params;
	this->in_vc = false;
}

template <typename T>
WhyName WhyFunctionCall<T>::get_name()
{
	return this->name;
}

template <typename T>
void WhyFunctionCall<T>::set_in_vc()
{
	this->in_vc = true;
}

template <typename T>
bool WhyFunctionCall<T>::get_in_vc()
{
	return this->in_vc;
}

template <typename T>
void WhyFunctionCall<T>::add_param(std::shared_ptr<WhyExpression> param)
{
	this->params.insert(this->params.begin(), param);
}

template <typename T>
std::string WhyFunctionCall<T>::pp_in_vc()
{
	std::string ret = "(" + this->name + " ";

	for (auto p : params)
	{
		if (std::dynamic_pointer_cast<WhyRValue>(p))
			ret += p->pp();
		else if (std::dynamic_pointer_cast<WhyClassType>(p->get_type()))
			throw CastorException("Cannot pass an object (" + p->pp() + ") to a predicate in a verification condition!");
		else
		{
			auto lv = safety_cast<WhyLValue>(p);
			ret += WhyMemoryGetExpr(lv->get_type(), lv).pp();
		}
		ret += " ";
	}

	ret += ")";

	return ret;
}

template <typename T>
std::string WhyFunctionCall<T>::pp()
{
	if (this->in_vc)
		return this->pp_in_vc();

	std::string ret = "(" + this->name + " ";

	if (params.size() == 0) ret += "()";

	for (auto p : params)
	{
		if (std::dynamic_pointer_cast<WhyReference>(p))
		{
			ret += p->pp();
		}
		else if (std::dynamic_pointer_cast<WhyRValue>(p))
		{
			if (std::dynamic_pointer_cast<WhyClassType>(p->get_type()))
				ret += "(MemoryModel.create_slice " + p->pp() + ")";
			else if (std::dynamic_pointer_cast<WhyBoolType>(p->get_type()))
				ret += "(MemoryModel.create_val (ArithmeticModel.bool_2_var " + p->pp() + "))";
			else
				ret += "(MemoryModel.create_val " + p->pp() + ")";
		}
		else if (std::dynamic_pointer_cast<WhyArrayType>(p->get_type()))
		{
			ret += "(MemoryModel.create_val " +
				WhyMemoryGetExpr(p->get_type(), safety_cast<WhyLValue>(p)).pp() + ")";
		}
		else
		{
			auto lv = safety_cast<WhyLValue>(p);

			ret += "(MemoryModel.copy " + lv->pp() + " " +
				std::to_string(lv->get_type()->get_size()) + ")";
		}
		ret += " ";
	}

	ret += ")";

	return ret;
}

WhyConstructor::WhyConstructor(std::shared_ptr<WhyType> type, WhyName name, std::vector<std::shared_ptr<WhyExpression>> params) :
	WhyRValue(type)
{
	this->name = name;
	this->params = params;
}

WhyName WhyConstructor::get_name()
{
	return this->name;
}

std::string WhyConstructor::pp()
{
	std::string ret;
       
	ret += "(let __allocd = (MemoryModel.alloca ";
	ret += std::to_string(this->type->get_size()) + ") in (" + this->name + " ";

	for (auto p : params)
	{
		if (std::dynamic_pointer_cast<WhyReference>(p))
		{
			ret += p->pp();
		}
		else if (std::dynamic_pointer_cast<WhyRValue>(p))
		{
			if (std::dynamic_pointer_cast<WhyClassType>(p->get_type()))
				ret += "(MemoryModel.create_slice " + p->pp() + ")";
			else if (std::dynamic_pointer_cast<WhyBoolType>(p->get_type()))
				ret += "(MemoryModel.create_val (ArithmeticModel.bool_2_var " + p->pp() + "))";
			else
				ret += "(MemoryModel.create_val " + p->pp() + ")";
		}
		else if (std::dynamic_pointer_cast<WhyArrayType>(p->get_type()))
		{
			ret += "(MemoryModel.create_val " +
				WhyMemoryGetExpr(p->get_type(), safety_cast<WhyLValue>(p)).pp() + ")";
		}
		else
		{
			auto lv = safety_cast<WhyLValue>(p);

			ret += "(MemoryModel.copy " + lv->pp() + " " +
				std::to_string(lv->get_type()->get_size()) + ")";
		}
	}

	ret += " (MemoryModel.create_val __allocd.addr)); (MemoryModel.get_slice tape read __allocd ";
	ret += std::to_string(this->type->get_size()) + "))";

	return ret;
}

WhyStatement::WhyStatement()
{
	this->continuation = nullptr;
}

void WhyStatement::set_continuation(std::shared_ptr<WhyStatement> continuation)
{
	this->continuation = continuation;
}

WhyStatementCollection::WhyStatementCollection(std::shared_ptr<WhyStatement> collection)
{
	this->collection = collection;
}

std::string WhyStatementCollection::pp()
{
	auto rest = this->pp_continuation();

	if (rest.length())
		return "(let old_read = read in let old_write = write in (" + this->collection->pp() + "); read <- old_read; write <- old_write);" + rest;
	else
		return "(let old_read = read in let old_write = write in (" + this->collection->pp() + "); read <- old_read; write <- old_write)";
}

WhyLoop::WhyLoop(std::shared_ptr<WhyRValue> cond, std::shared_ptr<WhyStatement> body, std::vector<std::shared_ptr<WhyVerificationCondition>> vcs, std::vector<std::shared_ptr<WhyVariable>> vars)
{
	this->cond = cond;
	this->body = body;
	this->vcs = vcs;
	this->vars = vars;
}

std::string WhyLoop::pp()
{
	std::string ret = "(";
	ret += "\nlabel LoopStart in while " + this->cond->pp() + " do\n";
	ret += generate_vcs(this->vars, true);
	for (auto vc : vcs)
		ret += vc->pp();
	ret += "(" + this->body->pp();
	ret += ") done);\n" + this->pp_continuation();
	return ret;
}

std::vector<std::shared_ptr<WhyVariable>> WhyLoop::get_vars()
{
	return this->vars;
}

WhyIf::WhyIf(std::shared_ptr<WhyRValue> cond, std::shared_ptr<WhyStatement> then, std::shared_ptr<WhyStatement> els)
{
	this->cond = cond;
	this->then = then;
	this->els = els;
}

std::string WhyIf::pp()
{
	std::string ret = "(if " + this->cond->pp() + " then (" + this->then->pp() + ")";
	if (this->els) ret += " else (" + this->els->pp() + ")";
	ret += ");\n" + this->pp_continuation();
	return ret;
}

WhyLabelStmt::WhyLabelStmt(std::shared_ptr<WhyLabel> label)
{
	this->label = label;
}

std::string WhyLabelStmt::pp()
{
	return "(label " + this->label->pp() + " in ();\n" + this->pp_continuation() + ")";
}

std::vector<std::shared_ptr<WhyNode>> WhyLabelStmt::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->label, this->continuation });
}

WhyAssert::WhyAssert(std::shared_ptr<WhyRValue> assertion, std::string debug_str)
{
	this->assertion = assertion;
	this->debug_str = debug_str;
}

std::string WhyAssert::pp()
{
	return "assert { [@expl:" + this->debug_str + "] " + this->assertion->pp() + " };" + this->pp_continuation();
}

WhyAssume::WhyAssume(std::shared_ptr<WhyRValue> assumption) : WhyAssert(assumption, "assumes clause") { }

std::string WhyAssume::pp()
{
	return "assume { " + this->assertion->pp() + "};" + this->pp_continuation();
}

WhyDiscardResultStmt::WhyDiscardResultStmt(std::shared_ptr<WhyExpression> base_item)
{
	this->base_item = base_item;
}

std::string WhyDiscardResultStmt::pp()
{
	return "(let _ = " + this->base_item->pp() + " in ());"
		+ this->pp_continuation();
}

std::string WhyStatement::pp_continuation()
{
	if (this->continuation)
		return "\n" + this->continuation->pp();
	else
		return "";
}

WhyEmptyStmt::WhyEmptyStmt() = default;

std::string WhyEmptyStmt::pp()
{
	if (auto str = this->pp_continuation(); str != "")
		return str;
	else
		return "()";
}

WhyEmptyExpr::WhyEmptyExpr() : WhyRValue(std::make_shared<WhyUnitType>()) { };

std::string WhyEmptyExpr::pp()
{
	return "()";
}

WhyVariableDecl::WhyVariableDecl(std::shared_ptr<WhyVariable> var, std::shared_ptr<WhyExpression> initial_value)
{
	this->var = var;

	if (initial_value)
	{
		if (std::dynamic_pointer_cast<WhyClassType>(initial_value->get_type()) ||
				std::dynamic_pointer_cast<WhyReference>(initial_value))
			this->initial_value = initial_value;
		else if (auto lv = std::dynamic_pointer_cast<WhyLValue>(initial_value))
			this->initial_value = std::make_shared<WhyMemoryGetExpr>(lv->get_type(), lv);
		else
			this->initial_value = initial_value;
	}
	else
		this->initial_value = initial_value;
}

std::shared_ptr<WhyVariable> WhyVariableDecl::get_variable()
{
	return this->var;
}

std::string WhyVariableDecl::pp()
{
	std::string ret;

	if (std::dynamic_pointer_cast<WhyReference>(this->initial_value))
	{
		ret += "let " + this->var->get_name() + " = " + this->initial_value->pp() + " in ";
	}
	else if (auto clas = std::dynamic_pointer_cast<WhyClassType>(this->var->get_type()))
	{
		if (!this->initial_value)
			throw CastorException("Currently need to explicitly assign objects to their initial value!");

		if (std::dynamic_pointer_cast<WhyRValue>(this->initial_value))
		{
			ret += "let " + this->var->get_name() + " = (MemoryModel.alloca ";
			ret += std::to_string(clas->get_size()) + ") in let _ = (MemoryModel.set_slice ";
			ret += this->var->get_name() + " " + this->initial_value->pp() + ") in";
		}
		else
		{
			ret += "let " + this->var->get_name() + " = (MemoryModel.copy ";
			ret += this->initial_value->pp() + " " + std::to_string(clas->get_size()) + ") in ";
		}
	}
	else if (auto arr = std::dynamic_pointer_cast<WhyArrayType>(this->var->get_type()))
	{
		ret += "let " + this->var->get_name() + " = (MemoryModel.alloca ";
		ret += std::to_string(arr->get_size()) + ") in ";
	}
	else if (std::dynamic_pointer_cast<WhyIntegralType>(this->var->get_type()))
	{
		ret += "let " + this->var->get_name() + " = (MemoryModel.create_val ";
		if (this->initial_value)
			ret += this->initial_value->pp();
		else
			ret += "(any int)";
		ret += ") in ";
	}
	else if (std::dynamic_pointer_cast<WhyBoolType>(this->var->get_type()))
	{
		ret += "let " + this->var->get_name() + " = (MemoryModel.create_val ";
		if (this->initial_value)
			ret += "(ArithmeticModel.bool_2_var " + this->initial_value->pp() + ")";
		else
			ret += "(any int)";
		ret += ") in ";
	}
	else
	{
		throw CastorException("Unable to initialize variable with this type (what is this type?)");
	}

	if (this->var->get_const() && !this->var->get_type()->get_reference())
	{
		ret += "(MemoryModel.set_const " + this->var->get_name() + " " + std::to_string(this->var->get_type()->get_size()) + "); ";
	}

	return ret + "(); " + this->pp_continuation();
}

WhyGlobalVariableDecl::WhyGlobalVariableDecl(std::shared_ptr<WhyVariable> var)
{
	this->var = var;
}

std::shared_ptr<WhyVariable> WhyGlobalVariableDecl::get_variable()
{
	return this->var;
}

std::string WhyGlobalVariableDecl::pp()
{
	return "val function " + this->var->get_name() + " : " + this->var->get_type()->pp() + "\n";
}

WhyReturn::WhyReturn(std::shared_ptr<WhyExpression> base_item)
{
	this->base_item = base_item;
}

std::string WhyReturn::pp()
{
	std::string ret = "__return_val <- ";
	if (std::dynamic_pointer_cast<WhyReference>(this->base_item) || std::dynamic_pointer_cast<WhyRValue>(this->base_item))
		ret += this->base_item->pp();
	else
		ret += WhyMemoryGetExpr(this->base_item->get_type(), safety_cast<WhyLValue>(this->base_item)).pp();
	ret += "; break Funcreturn__";
	return ret;
}

WhyLemma::WhyLemma(WhyName name, std::shared_ptr<WhyRValue> vc)
{
	this->name = name;
	this->vc = vc;
}

std::string WhyLemma::pp()
{
	return "lemma " + this->name + ": [@expl:(lemma) " + this->name + "] " + this->vc->pp() + "\n";
}

WhyAxiom::WhyAxiom(WhyName name, std::shared_ptr<WhyRValue> vc) : WhyLemma(name, vc) { }

std::string WhyAxiom::pp()
{
	return "axiom " + this->name + ": [@expl:(axiom) " + this->name + "] " + this->vc->pp() + "\n";
}

WhyVerificationCondition::WhyVerificationCondition(std::shared_ptr<WhyRValue> vc, std::string debug_str)
{
	this->vc = vc;
	this->debug_str = debug_str;
}

WhyEnsures::WhyEnsures(std::shared_ptr<WhyRValue> vc, std::string debug_str) : WhyVerificationCondition(vc, debug_str) { }

std::string WhyEnsures::pp()
{
	return "ensures { [@expl:" + this->debug_str + "] " + this->vc->pp() + " }\n";
}

WhyRequires::WhyRequires(std::shared_ptr<WhyRValue> vc, std::string debug_str) : WhyVerificationCondition(vc, debug_str) { }

std::string WhyRequires::pp()
{
	return "requires { [@expl:" + this->debug_str + "] " + this->vc->pp() + " }\n";
}

WhyInvariant::WhyInvariant(std::shared_ptr<WhyRValue> vc, std::string debug_str) : WhyVerificationCondition(vc, debug_str) { }

std::string WhyInvariant::pp()
{
	return "invariant { [@expl:" + this->debug_str + "] " + this->vc->pp() + " }\n";
}

WhyVariant::WhyVariant(std::shared_ptr<WhyRValue> vc, std::string debug_str) : WhyVerificationCondition(vc, debug_str) { }

std::string WhyVariant::pp()
{
	return "variant { [@expl:" + this->debug_str + "] " + this->vc->pp() + " }\n";
}

WhyWrites::WhyWrites(std::vector<std::shared_ptr<WhyLValue>> vars, std::string debug_str) : WhyVerificationCondition(nullptr, debug_str)
{
	this->vars = vars;
	this->in_loop = false;
}

void WhyWrites::set_in_loop()
{
	this->in_loop = true;
}

std::string WhyWrites::pp()
{
	return generate_writes(this->vars, this->in_loop, GenerateWrites, debug_str);
}

WhyFrees::WhyFrees(std::vector<std::shared_ptr<WhyLValue>> vars, std::string debug_str) : WhyVerificationCondition(nullptr, debug_str)
{
	this->vars = vars;
	this->in_loop = false;
}

void WhyFrees::set_in_loop()
{
	this->in_loop = true;
}

std::string WhyFrees::pp()
{
	return generate_writes(this->vars, this->in_loop, GenerateFrees, debug_str);
}

WhyEmptyNode::WhyEmptyNode() = default;

std::string WhyEmptyNode::pp()
{
	return "";
}

WhyCase::WhyCase(std::shared_ptr<WhyRValue> expr)
{
	this->expr = expr;
}

std::string WhyCase::pp()
{
	if (this->expr)
		return "(if (__comparer = " + this->expr->pp() + " || not __firsttime) then (__firsttime <- false; " + this->pp_continuation() + "));";
	else
		return "(__firsttime <- false; " + this->pp_continuation() + ");";
}

std::vector<std::shared_ptr<WhyNode>> WhyCase::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->continuation, this->expr });
}

WhyBreak::WhyBreak() = default;

std::string WhyBreak::pp()
{
	return "(break);" + this->pp_continuation();
}

std::vector<std::shared_ptr<WhyNode>> WhyBreak::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->continuation });
}

WhyContinue::WhyContinue() = default;

std::string WhyContinue::pp()
{
	return "(continue);" + this->pp_continuation();
}

std::vector<std::shared_ptr<WhyNode>> WhyContinue::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->continuation });
}

WhySwitch::WhySwitch(std::shared_ptr<WhyRValue> expr, std::vector<std::shared_ptr<WhyCase>> cases)
{
	this->expr = expr;
	this->cases = cases;
}

std::string WhySwitch::pp()
{
	std::string ret = "(label Switch__ in (while true do\n";
	ret += "variant { 1 } invariant { tape = tape at Switch__ /\\ read = read at Switch__ /\\ write = write at Switch__ }\n";
	ret += "let __comparer = " + this->expr->pp() + " in\n";
	ret += "let ref __firsttime = true in (\n";

	for (auto c : this->cases)
		ret += c->pp() + "\n";

	ret += "(break Switch__)) done));\n" + this->pp_continuation();

	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhySwitch::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret({ this->continuation, this->expr });
	for (auto c : this->cases)
		ret.push_back(c);
	return ret;
}

WhyFunction::WhyFunction(std::shared_ptr<WhyType> return_type, std::string name, std::vector<std::shared_ptr<WhyVariable>> vars, std::vector<std::shared_ptr<WhyVariable>> globals, std::vector<std::shared_ptr<WhyVerificationCondition>> vcs, std::shared_ptr<WhyStatement> body, bool is_ref_function)
{
	this->return_type = return_type;
	this->name = name;
	this->vars = vars;
	this->vcs = vcs;
	this->body = body;
	this->globals = globals;
	this->is_ref_function = is_ref_function;
	this->is_mutually_recursive = false;
	this->ir_name = name;
}

WhyName WhyFunction::get_name()
{
	return this->name;
}

std::string WhyFunction::get_ir_name()
{
	return this->ir_name;
}

void WhyFunction::set_mutual()
{
	this->is_mutually_recursive = true;
}

std::vector<std::shared_ptr<WhyVariable>> WhyFunction::get_vars()
{
	return this->vars;
}

std::string WhyFunction::pp()
{
	std::vector<std::shared_ptr<WhyVariable>> vc_vars = this->vars;
	vc_vars.insert(vc_vars.end(), this->globals.begin(), this->globals.end());

	std::string return_type = this->is_ref_function ? "MemoryModel.var"
			 : std::dynamic_pointer_cast<WhyClassType>(this->return_type) ? "array int"
			 : std::dynamic_pointer_cast<WhyBoolType>(this->return_type) ? "bool"
			 : std::dynamic_pointer_cast<WhyUnitType>(this->return_type) ? "()"
			 : "int";

	std::string ret;

	if (this->is_mutually_recursive)
		ret = "with " + this->name + " ";
	else if (!this->body)
		ret = "val " + this->name + " ";
	else
		ret = "let rec " + this->name + " ";
	if (vars.size())
		for (auto v : vars)
			ret += "(" + v->get_name() + " : " + v->get_type()->pp() + ") ";
	else
		ret += "()";
	ret += " : " + return_type + "\n";
	for (auto vc : vcs)
		ret += vc->pp() + "\n";
	ret += generate_vcs(vc_vars);
	ret += generate_contracts(this->return_type, this->is_ref_function);

	if (this->body)
	{
		ret += "=";
		if (settings.method == Settings::Method::sp)
			ret += "[@vc:sp]";
		ret += "\nlet ref __return_val = ";
		if (std::dynamic_pointer_cast<WhyUnitType>(this->return_type))
			ret += "()";
		else
			ret += "(any " + return_type + ")";
		ret += " in\nlet old_read = read in let old_write = write in label Funcreturn__ in\n";
		ret += "while true do variant { 1 } invariant { tape = old tape /\\ read = old read /\\ write = old write } (\n";
		ret += this->body->pp();
		ret += ");\nbreak\ndone;\nread <- old_read; write <- old_write;\n__return_val\n";
	}
	else
	{
		ret += "writes { read, write, tape }\n";
	}

	return ret;
}

WhyProgram::WhyProgram(std::vector<std::shared_ptr<WhyGlobalVariableDecl>> vars, std::vector<std::shared_ptr<WhyFunction>> funcs, std::vector<std::shared_ptr<WhyLemma>> lemmas)
{
	this->vars = vars;
	this->funcs = funcs;
	this->lemmas = lemmas;
}

std::vector<std::shared_ptr<WhyFunction>> WhyProgram::get_funcs()
{
	return this->funcs;
}

std::vector<std::shared_ptr<WhyGlobalVariableDecl>> WhyProgram::get_vars()
{
	return this->vars;
}

std::vector<std::shared_ptr<WhyLemma>> WhyProgram::get_lemmas()
{
	return this->lemmas;
}

std::string WhyProgram::pp()
{
	std::string ret = "module SynthesizedProgram\n";
	ret += "use ArithmeticModel.ArithmeticModel\n";
	ret += "use MemoryModel.MemoryModel\n";
	ret += "use int.Int\n";
	ret += "use array.Array\n";
	ret += "use map.Map\n";
	ret += "use bool.Bool\n";
	if (settings.extra_lemmas) ret += "use Lemmas.Lemmas\n";
	if (settings.bitvec_lemmas) ret += "use BitVector.BitVector\n";

	for (auto v : vars)
		ret += v->pp() + "\n";

	for (auto l : lemmas)
		ret += l->pp() + "\n";

	for (auto f : funcs)
		ret += f->pp() + "\n";

	ret += "end";

	return ret;
}

WhyNullptr::WhyNullptr() : WhyRValue(std::make_shared<WhyPointerType>(std::make_shared<WhyU8Type>())) { }

std::string WhyNullptr::pp()
{
	return "MemoryModel.nullptr";
}

std::vector<std::shared_ptr<WhyNode>> WhyNullptr::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>();
}

std::vector<std::shared_ptr<WhyNode>> WhyType::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>();
}

std::vector<std::shared_ptr<WhyNode>> __WhyVariableList::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>();
}

std::vector<std::shared_ptr<WhyNode>> __WhyFunctions::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>();
}

std::vector<std::shared_ptr<WhyNode>> WhyExpression::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>();
}

template <typename T>
std::vector<std::shared_ptr<WhyNode>> WhyCast<T>::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_object });
}

std::vector<std::shared_ptr<WhyNode>> __WhyFunctionRef::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>();
}

template <typename T>
std::vector<std::shared_ptr<WhyNode>> WhyLiteral<T>::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>();
}

std::vector<std::shared_ptr<WhyNode>> WhyBinaryOperation::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->lhs, this->rhs });
}

std::vector<std::shared_ptr<WhyNode>> WhyAssignOp::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->lhs, this->rhs });
}

template <typename T>
std::vector<std::shared_ptr<WhyNode>> WhyCompoundAssignOp<T>::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->lhs, this->base_assignment });
}

template <typename T>
std::vector<std::shared_ptr<WhyNode>> WhyCommaOp<T>::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->lhs, this->rhs });
}

std::vector<std::shared_ptr<WhyNode>> WhyUnaryOperation::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base });
}

std::vector<std::shared_ptr<WhyNode>> WhyIncrementOp::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base });
}

std::vector<std::shared_ptr<WhyNode>> WhyDecrementOp::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base });
}

template <typename T>
std::vector<std::shared_ptr<WhyNode>> WhyFunctionCall<T>::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret;
	for (auto p : params)
		ret.push_back(p);
	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhyConstructor::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret;
	for (auto p : params)
		ret.push_back(p);
	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhyMemoryGetExpr::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item });
}

std::vector<std::shared_ptr<WhyNode>> WhyAddressOfExpr::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item });
}

std::vector<std::shared_ptr<WhyNode>> WhyValid::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret;
	for (auto b : base_items)
		ret.push_back(b);
	for (auto b : vars)
		ret.push_back(b);
	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhyNth::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base, this->idx });
}

std::vector<std::shared_ptr<WhyNode>> WhyFreed::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret;
	for (auto b : base_items)
		ret.push_back(b);
	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhyOld::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item });
}

std::vector<std::shared_ptr<WhyNode>> WhyUnchanged::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item });
}

std::vector<std::shared_ptr<WhyNode>> WhyAliasOf::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item, this->aliased_item });
}

std::vector<std::shared_ptr<WhyNode>> WhyAt::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item, this->label });
}

std::vector<std::shared_ptr<WhyNode>> WhyValidArray::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item, this->size });
}

std::vector<std::shared_ptr<WhyNode>> WhySeparated::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret;
	for (auto b : base_items)
		ret.push_back(b);
	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhyVariable::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>();
}

std::vector<std::shared_ptr<WhyNode>> WhyVariableReference::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->variable });
}

std::vector<std::shared_ptr<WhyNode>> WhyPointerDereference::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item });
}

std::vector<std::shared_ptr<WhyNode>> WhyReference::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item });
}

std::vector<std::shared_ptr<WhyNode>> WhyFieldReference::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item });
}

std::vector<std::shared_ptr<WhyNode>> WhyFieldReferenceRValue::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item });
}

std::vector<std::shared_ptr<WhyNode>> WhyStatement::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->continuation });
}

std::vector<std::shared_ptr<WhyNode>> WhyStatementCollection::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->continuation, this->collection });
}

std::vector<std::shared_ptr<WhyNode>> WhyDiscardResultStmt::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->continuation, this->base_item });
}

std::vector<std::shared_ptr<WhyNode>> WhyVariableDecl::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->continuation, this->var, this->initial_value });
}

std::vector<std::shared_ptr<WhyNode>> WhyGlobalVariableDecl::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->continuation, this->var });
}

std::vector<std::shared_ptr<WhyNode>> WhyAssert::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->continuation, this->assertion });
}

std::vector<std::shared_ptr<WhyNode>> WhyReturn::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->continuation, this->base_item });
}

std::vector<std::shared_ptr<WhyNode>> WhyLoop::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret({ this->continuation, this->cond, this->body });
	for (auto vc : vcs)
		ret.push_back(vc);
	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhyIf::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->continuation, this->cond, this->then, this->els });
}

std::vector<std::shared_ptr<WhyNode>> WhyVerificationCondition::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->vc });
}

std::vector<std::shared_ptr<WhyNode>> WhyLemma::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->vc });
}

std::vector<std::shared_ptr<WhyNode>> WhyWrites::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret;
	for (auto v : vars)
		ret.push_back(v);
	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhyFrees::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret;
	for (auto v : vars)
		ret.push_back(v);
	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhyEmptyNode::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>();
}

std::vector<std::shared_ptr<WhyNode>> WhyFunction::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret({ this->body });
	for (auto v : vars)
		ret.push_back(v);
	for (auto vc : vcs)
		ret.push_back(vc);
	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhyProgram::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret;
	for (auto f : funcs)
		ret.push_back(f);
	for (auto v : vars)
		ret.push_back(v);
	for (auto l : lemmas)
		ret.push_back(l);
	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhyQuantifier::traverse()
{
	std::vector<std::shared_ptr<WhyNode>> ret({ this->expr });
	for (auto v : vars)
		ret.push_back(v);
	return ret;
}

std::vector<std::shared_ptr<WhyNode>> WhyArrayIndex::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_object, this->index });
}

std::vector<std::shared_ptr<WhyNode>> WhyArrayRange::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_object, this->start, this->end });
}

#ifdef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
std::vector<std::shared_ptr<WhyNode>> WhyNew::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->constructed_object });
}

std::vector<std::shared_ptr<WhyNode>> WhyDelete::traverse()
{
	return std::vector<std::shared_ptr<WhyNode>>({ this->base_item });
}
#endif

