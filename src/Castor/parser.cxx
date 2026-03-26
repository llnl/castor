// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "parser.hxx"
#include "helper.cxx"
#include "exception.hxx"
#include "messaging.hxx"
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Bootstrap/BootstrapParser.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/Parser.h>
#include <fstream>
#include <sstream>
#include <utility>
#include <charconv>
#include <climits>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/dll/runtime_symbol_info.hpp>

#ifndef SOURCE_DIR
	#error "Did not specify source directory, cannot compile!"
#endif

using namespace Sawyer::Message::Common;

namespace VCParser
{

///
/// @brief Loads the grammar based on the parse type.
///
/// @param parse_type Either LValueFunc or RValueFunc
/// @return The string representing the grammar
///
std::string load(ParseType parse_type)
{
	std::string grammar_source;
	if (parse_type == RValueFunc)
	{
		grammar_source = "/grammars/vc_grammar_rvalue_func.peg";
	}
	else
	{
		grammar_source = "/grammars/vc_grammar_lvalue_func.peg";
	}

	std::ifstream ins;
	auto grammar_str = std::string(SOURCE_DIR) + grammar_source;
	ins.open(grammar_str, std::ifstream::in);
	std::ostringstream sstr;
	sstr << ins.rdbuf();
	return sstr.str();
}

std::shared_ptr<PEG::Parser> rparser; ///< GoosePEG rvalue parser object

std::shared_ptr<PEG::Parser> lparser; ///< GoosePEG lvalue parser object

///
/// @brief Represents the notion of a pointer-to-IR-node value for semantic actions
///
class PEGIRValue : public PEG::ValueInterface
{
public:
	std::shared_ptr<IRNode> c_value; ///< Internal pointer-to-IR-node value

	///
	/// @brief Constructor
	///
	/// @param c_value A pointer-to-ir-node
	///
	PEGIRValue(const std::shared_ptr<IRNode>& c_value) : c_value(c_value) { }
};

///
/// @brief Represents the notion of an arbitrary value for semantic actions
///
/// @tparam T The type of the arbitrary value
///
template <typename T>
class PEGValue : public PEG::ValueInterface
{
public:
	std::remove_reference_t<T> c_value; ///< Internal pointer to the arbitrary value

	///
	/// @brief Constructor
	///
	/// @param c_value An arbitrary value to initialize this class with
	///
	PEGValue(const std::remove_reference_t<T>& c_value) : c_value(c_value) { }
};

///
/// @brief Gets an IR node based on the parse tree and an index into it
///
/// @tparam T The type of the IR Node
/// @param parseTree The ParseTree object from GoosePEG
/// @param idx The index into parseTree
/// @return A shared pointer to T
///
template <typename T>
inline std::shared_ptr<T> get_ir_value(const std::vector<PEG::ValueInterfacePtr>& parseTree, int idx)
{
	if (idx >= parseTree.size())
		throw CastorException("Internal error in VC parser(1)");
	auto c_value = safety_cast<PEGIRValue>(
				safety_cast<PEG::ParseTree::PlainParseTree>(parseTree[idx])
				->children[0])->c_value;
	return safety_cast<T>(c_value);
}

///
/// @brief Gets an arbitrary value based on the parse tree and an index into it
///
/// @tparam T The type of the arbitrary value
/// @param parseTree The ParseTree object from GoosePEG
/// @param idx The index into parseTree
/// @return The fetched arbitrary value
///
template <typename T>
inline T get_value(const std::vector<PEG::ValueInterfacePtr>& parseTree, int idx)
{
	if (idx >= parseTree.size())
		throw CastorException("Internal error in VC parser(2)");
	auto c_value = safety_cast<PEGValue<T>>(
				safety_cast<PEG::ParseTree::PlainParseTree>(parseTree[idx])
				->children[0])->c_value;
	return c_value;
}

///
/// @brief Semantic action for just passing a value up the tree
///
/// This version passes up the 0th value
///
class LiftHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		if (auto ppt = std::dynamic_pointer_cast<PEG::ParseTree::PlainParseTree>(children[0]))
			return safety_cast<PEGIRValue>(ppt->children[0]);
		else
			return safety_cast<PEGIRValue>(children[0]);
	}
};

///
/// @brief Semantic action for just passing a value up the tree
///
/// This version passes up the 1st value
///
class LiftHandler1 : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		if (auto ppt = std::dynamic_pointer_cast<PEG::ParseTree::PlainParseTree>(children[1]))
			return safety_cast<PEGIRValue>(ppt->children[0]);
		else
			return safety_cast<PEGIRValue>(children[1]);
	}
};

///
/// @brief Semantic actions for parsing Expression nodes from the input grammar
///
/// This version is right-associative
///
class ExpressionHandlerRight : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		std::vector<std::shared_ptr<IRExpression>> exprs({ get_ir_value<IRExpression>(children, 0) });
		std::vector<std::string> ops;

		auto rep = safety_cast<PEG::ParseTree::PlainParseTree>(children[1]);

		if (rep->label != "Empty")
		{
			for (int i = 0; i < rep->children.size(); i++)
			{
				auto& right = safety_cast<PEG::ParseTree::PlainParseTree>(rep->children[i])->children;
				auto op = get_value<std::string>(right, 0);
				auto expr = get_ir_value<IRExpression>(right, 1);
				ops.push_back(op);
				exprs.push_back(expr);
			}
		}

		if (exprs.size() > 1) 
		{
			auto atom1 = exprs[exprs.size() - 1];

			for (int i = exprs.size() - 2; i >= 0; i--)
			{
				std::shared_ptr<IRNode> return_val;

				auto atom2 = exprs[i];

				std::string op = ops[i];
				boost::trim(op);

				if (op == "=>")
					atom1 = std::make_shared<IRImpliesOp>(std::make_shared<IRUnknownType>(), atom2, atom1);
				else if (op == "<->")
					atom1 = std::make_shared<IREqualsOp>(std::make_shared<IRUnknownType>(), atom2, atom1);
			}

			return std::make_shared<PEGIRValue>(atom1);
		}
		else
		{
			return std::make_shared<PEGIRValue>(exprs[0]);
		}
	}
};

///
/// @brief Semantic actions for parsing Expression nodes from the input grammar
///
/// This version is left-associative
///
class ExpressionHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		std::vector<std::shared_ptr<IRExpression>> exprs({ get_ir_value<IRExpression>(children, 0) });
		std::vector<std::string> ops;

		auto rep = safety_cast<PEG::ParseTree::PlainParseTree>(children[1]);

		if (rep->label != "Empty")
		{
			for (int i = 0; i < rep->children.size(); i++)
			{
				auto& right = safety_cast<PEG::ParseTree::PlainParseTree>(rep->children[i])->children;
				auto op = get_value<std::string>(right, 0);
				auto expr = get_ir_value<IRExpression>(right, 1);
				ops.push_back(op);
				exprs.push_back(expr);
			}
		}

		if (exprs.size() > 1) 
		{
			auto atom1 = exprs[0];

			for (int i = 0; i < exprs.size() - 1; i++)
			{
				std::shared_ptr<IRNode> return_val;

				auto atom2 = exprs[i + 1];

				std::string op = ops[i];
				boost::trim(op);

				if (op == "/\\")
					atom1 = std::make_shared<IRAndOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "\\/")
					atom1 = std::make_shared<IROrOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == ">=")
					atom1 = std::make_shared<IRGreaterEqualsOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == ">")
					atom1 = std::make_shared<IRGreaterThanOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "<=")
					atom1 = std::make_shared<IRLessEqualsOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "<")
					atom1 = std::make_shared<IRLessThanOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "==")
					atom1 = std::make_shared<IREqualsOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "!=")
					atom1 = std::make_shared<IRNotEqualsOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "+")
					atom1 = std::make_shared<IRAdditionOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "-")
					atom1 = std::make_shared<IRSubtractionOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "*")
					atom1 = std::make_shared<IRMultiplyOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "/")
					atom1 = std::make_shared<IRDivideOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "%")
					atom1 = std::make_shared<IRModuloOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "<<")
					atom1 = std::make_shared<IRBitLShiftOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == ">>")
					atom1 = std::make_shared<IRBitRShiftOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "^")
					atom1 = std::make_shared<IRBitXorOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "&")
					atom1 = std::make_shared<IRBitAndOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
				else if (op == "|")
					atom1 = std::make_shared<IRBitOrOp>(std::make_shared<IRUnknownType>(), atom1, atom2);
			}

			return std::make_shared<PEGIRValue>(atom1);
		}
		else
		{
			return std::make_shared<PEGIRValue>(exprs[0]);
		}
	}
};

///
/// @brief Semantic actions for parsing prefix nodes from the input grammar
///
class PrefixHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto op = get_value<std::string>(children, 0);
		auto atom = get_ir_value<IRExpression>(children, 1);
		boost::trim(op);
	
		if (op == "-")
			return std::make_shared<PEGIRValue>(
					std::make_shared<IRNegationOp>(
						std::make_shared<IRUnknownType>(), atom));
		else if (op == "!")
			return std::make_shared<PEGIRValue>(
					std::make_shared<IRBoolNegationOp>(
						std::make_shared<IRUnknownType>(), atom));
		else if (op == "~")
			return std::make_shared<PEGIRValue>(
					std::make_shared<IRBitNegationOp>(
						std::make_shared<IRUnknownType>(), atom));
		else
			return std::make_shared<PEGIRValue>(atom);
	}
};

///
/// @brief Semantic actions for parsing quantifier nodes from the input grammar
///
class QuantifierHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		int type = get_value<int>(children, 0);
		auto var_list = get_value<std::vector<std::shared_ptr<IRVariable>>>(children, 1);
		auto expr = get_ir_value<IRExpression>(children, 3);

		if (type == 0)
			return std::make_shared<PEGIRValue>(std::make_shared<IRForall>(var_list, expr));
		else
			return std::make_shared<PEGIRValue>(std::make_shared<IRExists>(var_list, expr));
	}
};

///
/// @brief Semantic actions for parsing quantifier nodes from the input grammar
///
/// This version determines the type of quantifier
///
class QuantHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return value == "forall" ? std::make_shared<PEGValue<int>>(0) : std::make_shared<PEGValue<int>>(1);
	}
};

///
/// @brief Semantic actions for parsing boolean nodes from the input grammar
///
class BooleanHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(
				std::make_shared<IRBoolLiteral>(value == "true"));
	}
};
///
/// @brief Semantic actions for parsing pointer dereference nodes from the input grammar
///
class PointerDereferenceHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto base = get_ir_value<IRExpression>(children, 1);
		return std::make_shared<PEGIRValue>(
				std::make_shared<IRPointerDereference>(
					std::make_shared<IRUnknownType>(), base));
	}
};

///
/// @brief Semantic actions for parsing address-of nodes from the input grammar
///
class AddressOfHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto base = get_ir_value<IRLValue>(children, 1);
		return std::make_shared<PEGIRValue>(
				std::make_shared<IRAddressOf>(
					std::make_shared<IRUnknownType>(), base));
	}
};

///
/// @brief Semantic actions for parsing variable lists from the input grammar
///
class VarListHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		std::vector<std::shared_ptr<IRVariable>> vars({ get_ir_value<IRVariable>(children, 0) });

		auto rep = safety_cast<PEG::ParseTree::PlainParseTree>(children[1]);

		if (rep->label != "Empty")
		{
			for (int i = 0; i < rep->children.size(); i++)
			{
				auto& right = safety_cast<PEG::ParseTree::PlainParseTree>(rep->children[i])->children;
				auto variable = get_ir_value<IRVariable>(right, 1);
				vars.push_back(variable);
			}
		}

		return std::make_shared<PEGValue<std::vector<std::shared_ptr<IRVariable>>>>(vars);
	}
};

///
/// @brief Semantic actions for parsing expression lists from the input grammar
///
class ExpressionListHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		std::vector<std::shared_ptr<IRExpression>> exprs({ get_ir_value<IRExpression>(children, 0) });

		auto rep = safety_cast<PEG::ParseTree::PlainParseTree>(children[1]);

		if (rep->label != "Empty")
		{
			for (int i = 0; i < rep->children.size(); i++)
			{
				auto& right = safety_cast<PEG::ParseTree::PlainParseTree>(rep->children[i])->children;
				auto expr = get_ir_value<IRExpression>(right, 1);
				exprs.push_back(expr);
			}
		}

		return std::make_shared<PEGValue<std::vector<std::shared_ptr<IRExpression>>>>(exprs);
	}
};

///
/// @brief Semantic actions for parsing call nodes from the input grammar
///
class CallHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto var = get_ir_value<IRVariableReference>(children, 0);
		std::string func_name = var->get_name();
		auto expr_list = get_value<std::vector<std::shared_ptr<IRExpression>>>(children, 2);
		return std::make_shared<PEGIRValue>(
				std::make_shared<IRFunctionCallExpr<IRRValue>>(
					std::make_shared<IRUnknownType>(),
					std::make_shared<IRFunctionRefExpr>(func_name),
					expr_list));
	}
};

///
/// @brief Semantic actions for parsing variable declarations from the input grammar
///
class VarDeclHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		std::string type = get_ir_value<IRVariableReference>(children, 0)->get_name();
		std::string var = get_ir_value<IRVariableReference>(children, 2)->get_name();

		std::shared_ptr<IRType> computed_type =
			(type == "sint64") ? safety_cast<IRType>(std::make_shared<IRS64Type>()) :
			(type == "uint64") ? safety_cast<IRType>(std::make_shared<IRU64Type>()) :
			(type == "sint32") ? safety_cast<IRType>(std::make_shared<IRS32Type>()) :
			(type == "uint32") ? safety_cast<IRType>(std::make_shared<IRU32Type>()) :
			(type == "sint16") ? safety_cast<IRType>(std::make_shared<IRS16Type>()) :
			(type == "uint16") ? safety_cast<IRType>(std::make_shared<IRU16Type>()) :
			(type == "sint8") ? safety_cast<IRType>(std::make_shared<IRS8Type>()) :
			(type == "uint8") ? safety_cast<IRType>(std::make_shared<IRU8Type>()) :
			(type == "int") ? safety_cast<IRType>(std::make_shared<IRUnboundedIntegralType>()) :
			(type == "bool") ? safety_cast<IRType>(std::make_shared<IRBoolType>()) :
			throw UnknownTypeException(type);
	
		return std::make_shared<PEGIRValue>(std::make_shared<IRVariable>(computed_type, var, var, false));
	}
};

///
/// @brief Semantic actions for parsing min_sint8 from the input grammar
///
class MinS8Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRS8Literal>(INT8_MIN));
	}
};

///
/// @brief Semantic actions for parsing max_sint8 from the input grammar
///
class MaxS8Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRS8Literal>(INT8_MAX));
	}
};

///
/// @brief Semantic actions for parsing min_uint8 from the input grammar
///
class MinU8Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRU8Literal>(0));
	}
};

///
/// @brief Semantic actions for parsing max_uint8 from the input grammar
///
class MaxU8Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRU8Literal>(UINT8_MAX));
	}
};

///
/// @brief Semantic actions for parsing min_sint16 from the input grammar
///
class MinS16Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRS16Literal>(INT16_MIN));
	}
};

///
/// @brief Semantic actions for parsing max_sint16 from the input grammar
///
class MaxS16Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRS16Literal>(INT16_MAX));
	}
};

///
/// @brief Semantic actions for parsing min_uint16 from the input grammar
///
class MinU16Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRU16Literal>(0));
	}
};

///
/// @brief Semantic actions for parsing max_uint16 from the input grammar
///
class MaxU16Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRU16Literal>(UINT16_MAX));
	}
};

///
/// @brief Semantic actions for parsing min_sint32 from the input grammar
///
class MinS32Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRS32Literal>(INT32_MIN));
	}
};

///
/// @brief Semantic actions for parsing max_sint32 from the input grammar
///
class MaxS32Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRS32Literal>(INT32_MAX));
	}
};

///
/// @brief Semantic actions for parsing min_uint32 from the input grammar
///
class MinU32Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRU32Literal>(0));
	}
};

///
/// @brief Semantic actions for parsing max_uint32 from the input grammar
///
class MaxU32Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRU32Literal>(UINT32_MAX));
	}
};

///
/// @brief Semantic actions for parsing min_sint64 from the input grammar
///
class MinS64Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRS64Literal>(INT64_MIN));
	}
};

///
/// @brief Semantic actions for parsing max_sint64 from the input grammar
///
class MaxS64Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRS64Literal>(INT64_MAX));
	}
};

///
/// @brief Semantic actions for parsing min_uint64 from the input grammar
///
class MinU64Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRU64Literal>(0));
	}
};

///
/// @brief Semantic actions for parsing max_uint64 from the input grammar
///
class MaxU64Handler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRU64Literal>(UINT64_MAX));
	}
};

///
/// @brief Semantic actions for parsing lemma clauses from the input grammar
///
class LemmaHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto name = get_value<std::string>(children, 1);
		auto vc = get_ir_value<IRExpression>(children, 3);
		return std::make_shared<PEGIRValue>(std::make_shared<IRLemma>(name, vc));
	}
};

///
/// @brief Semantic actions for parsing axiom clauses from the input grammar
///
class AxiomHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto name = get_value<std::string>(children, 1);
		auto vc = get_ir_value<IRExpression>(children, 3);
		return std::make_shared<PEGIRValue>(std::make_shared<IRAxiom>(name, vc));
	}
};

///
/// @brief Semantic actions for parsing ensures clauses from the input grammar
///
class EnsuresHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto vc = get_ir_value<IRExpression>(children, 1);
		return std::make_shared<PEGIRValue>(std::make_shared<IREnsures>(vc));
	}
};

///
/// @brief Semantic actions for parsing requires clauses from the input grammar
///
class RequiresHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto vc = get_ir_value<IRExpression>(children, 1);
		return std::make_shared<PEGIRValue>(std::make_shared<IRRequires>(vc));
	}
};

///
/// @brief Semantic actions for parsing loop variant clauses from the input grammar
///
class VariantHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto vc = get_ir_value<IRExpression>(children, 1);
		return std::make_shared<PEGIRValue>(std::make_shared<IRLoopVariant>(vc));
	}
};

///
/// @brief Semantic actions for parsing loop invariant clauses from the input grammar
///
class InvariantHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto vc = get_ir_value<IRExpression>(children, 1);
		return std::make_shared<PEGIRValue>(std::make_shared<IRLoopInvariant>(vc));
	}
};

///
/// @brief Semantic actions for parsing writes clauses from the input grammar
///
class WritesHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		std::vector<std::shared_ptr<IRExpression>> vars;

		if (children.size() > 1)
			vars = get_value<std::vector<std::shared_ptr<IRExpression>>>(children, 1);

		return std::make_shared<PEGIRValue>(std::make_shared<IRWrites>(vars));
	}
};
	
///
/// @brief Semantic actions for parsing frees clauses from the input grammar
///
class FreesHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
#ifndef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
		log("WARNING: Using frees clause without having compiled with dynamic memory allocation enabled.\n"
		"You don't need to use frees clauses in this mode!", LogType::WARN, 0);
#endif

		std::vector<std::shared_ptr<IRExpression>> vars;

		if (children.size() > 1)
			vars = get_value<std::vector<std::shared_ptr<IRExpression>>>(children, 1);

		return std::make_shared<PEGIRValue>(std::make_shared<IRFrees>(vars));
	}
};

///
/// @brief Semantic actions for parsing assert clauses from the input grammar
///
class AssertHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto vc = get_ir_value<IRExpression>(children, 1);
		return std::make_shared<PEGIRValue>(std::make_shared<IRAssert>(vc));
	}
};

///
/// @brief Semantic actions for parsing assume clauses from the input grammar
///
class AssumeHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto vc = get_ir_value<IRExpression>(children, 1);
		return std::make_shared<PEGIRValue>(std::make_shared<IRAssume>(vc));
	}
};

///
/// @brief Semantic actions for parsing names from the input grammar
///
class NameHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		std::string out = safety_cast<PEG::ParseTree::PlainParseTree>(children[0])->value;

		auto rep = safety_cast<PEG::ParseTree::PlainParseTree>(children[1]);

		if (rep->label != "Empty")
			for (auto child : rep->children)
				out += safety_cast<PEG::ParseTree::PlainParseTree>(child)->value;

		return std::make_shared<PEGValue<std::string>>(out);
	}
};

///
/// @brief Semantic actions for parsing an identifier from the input grammar
///
class IdentifierHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(
				std::make_shared<IRVariableReference>(
					std::make_shared<IRVariable>(
						std::make_shared<IRUnknownType>(),
						get_value<std::string>(children, 1),
						get_value<std::string>(children, 1),
						false)));
	}
};

///
/// @brief Semantic actions for parsing numbers from the input grammar
///
class NumberHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		std::string str;
		int32_t first_try;
		int64_t second_try;
		uint64_t third_try;

		auto rep = safety_cast<PEG::ParseTree::PlainParseTree>(children[0]);

		//for (auto child : rep->children)
		for (auto child : children)
			str += safety_cast<PEG::ParseTree::PlainParseTree>(child)->value;

		auto c_str = str.c_str();

		auto result = std::from_chars(c_str, c_str + str.length(), first_try);
		if (result.ec == std::errc{})
			return std::make_shared<PEGIRValue>(std::make_shared<IRS32Literal>(first_try));

		result = std::from_chars(c_str, c_str + str.length(), second_try);
		if (result.ec == std::errc{})
			return std::make_shared<PEGIRValue>(std::make_shared<IRS64Literal>(second_try));

		result = std::from_chars(c_str, c_str + str.length(), third_try);
		if (result.ec == std::errc{})
			return std::make_shared<PEGIRValue>(std::make_shared<IRU64Literal>(third_try));

		throw CastorException("Cannot parse literal \"" + str + "\"... doesn't fit into any supported literal datatype!");
	}
};

///
/// @brief Semantic actions for parsing rvalue result nodes from the input grammar
///
class RResultHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRResult<IRRValue>>());
	}
};

///
/// @brief Semantic actions for parsing lvalue result nodes from the input grammar
///
class LResultHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRResult<IRLValue>>());
	}
};

///
/// @brief Semantic actions for parsing index range nodes from the input grammar
///
class IndexRangeHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto begin = get_ir_value<IRExpression>(children, 0);
		std::shared_ptr<IRExpression> end = nullptr;

		auto opt = safety_cast<PEG::ParseTree::PlainParseTree>(children[1]);

		if (opt->label != "Empty")
			end = get_ir_value<IRExpression>(
					safety_cast<PEG::ParseTree::PlainParseTree>(
						opt->children[0])->children, 1);

		return std::make_shared<PEGValue<std::pair<std::shared_ptr<IRExpression>, std::shared_ptr<IRExpression>>>>(
				std::make_pair(begin, end));
	}
};

///
/// @brief Semantic actions for parsing label nodes from the input grammar
///
class LabelHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(
				std::make_shared<IRLabelReference>(
					get_ir_value<IRVariableReference>(children, 1)->get_name()));
	}
};

///
/// @brief Semantic actions for parsing a ghost code marker
///
class GhostHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGIRValue>(std::make_shared<IRGhostStatement>());
	}
};

///
/// @brief Semantic actions for parsing prefix operator nodes from the input grammar
///
/// This version just gets the character
///
class PrefixCharHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		return std::make_shared<PEGValue<std::string>>(safety_cast<PEG::ParseTree::PlainParseTree>(children[0])->value);
	}
};

///
/// @brief Semantic actions for parsing binary operator nodes from the input grammar
///
/// This version just gets the character
///
class OperatorHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		if (value != "")
			return std::make_shared<PEGValue<std::string>>(value);
		else
			return std::make_shared<PEGValue<std::string>>(safety_cast<PEG::ParseTree::PlainParseTree>(children[0])->value);
	}
};

///
/// @brief Semantic actions for parsing postfix expressions
///
class PostfixHandler : public PEG::ValueFactoryInterface
{
public:
	PEG::ValueInterfacePtr createParseTree(
			const std::string& label, const int& position, const std::string& value,
			const std::vector<PEG::ValueInterfacePtr>& children) override
	{
		auto base = get_ir_value<IRExpression>(children, 0);

		auto rep = safety_cast<PEG::ParseTree::PlainParseTree>(children[1]);

		if (rep->label != "Empty")
		{
			for (int i = 0; i < rep->children.size(); i++)
			{
				auto right = safety_cast<PEG::ParseTree::PlainParseTree>(rep->children[i]);
				auto& terms = safety_cast<PEG::ParseTree::PlainParseTree>(right->children[0])->children;

				auto str = safety_cast<PEG::ParseTree::PlainParseTree>(terms[0])->value;
				boost::trim(str);

				if (str == ".")
				{
					auto varref = get_ir_value<IRVariableReference>(terms, 1);

					base = std::make_shared<IRFieldReference>(std::make_shared<IRUnknownType>(),
							base, varref->get_name());
				}
				else if (str == "->")
				{
					auto varref = get_ir_value<IRVariableReference>(terms, 1);

					base = std::make_shared<IRFieldReference>(std::make_shared<IRUnknownType>(),
							std::make_shared<IRPointerDereference>(std::make_shared<IRUnknownType>(), base),
							varref->get_name());
				}
				else if (str == "[")
				{
					auto pair = get_value<std::pair<std::shared_ptr<IRExpression>, std::shared_ptr<IRExpression>>>(terms, 1);

					if (pair.second == nullptr)
						base = std::make_shared<IRArrayIndex>(std::make_shared<IRUnknownType>(),
								safety_cast<IRLValue>(base), pair.first);
					else
						base = std::make_shared<IRArrayRangeIndex>(std::make_shared<IRUnknownType>(),
								safety_cast<IRLValue>(base), pair.first, pair.second);
				}
			}
		}

		return std::make_shared<PEGIRValue>(base);
	}
};

///
/// @brief Configures the parser for parsing.
///
void configure()
{
	static bool configured = false;

	if (configured)
		return;

	auto action_map = std::unordered_map<std::string, PEG::ValueFactoryInterfacePtr>();
	action_map.insert({ "IdentifierHandler", std::make_shared<IdentifierHandler>() });
	action_map.insert({ "LiftHandler", std::make_shared<LiftHandler>() });
	action_map.insert({ "LiftHandler1", std::make_shared<LiftHandler1>() });
	action_map.insert({ "ExpressionHandler", std::make_shared<ExpressionHandler>() });
	action_map.insert({ "ExpressionHandlerRight", std::make_shared<ExpressionHandlerRight>() });
	action_map.insert({ "PrefixHandler", std::make_shared<PrefixHandler>() });
	action_map.insert({ "PrefixCharHandler", std::make_shared<PrefixCharHandler>() });
	action_map.insert({ "OperatorHandler", std::make_shared<OperatorHandler>() });
	action_map.insert({ "LabelHandler", std::make_shared<LabelHandler>() });
	action_map.insert({ "QuantifierHandler", std::make_shared<QuantifierHandler>() });
	action_map.insert({ "QuantHandler", std::make_shared<QuantHandler>() });
	action_map.insert({ "BooleanHandler", std::make_shared<BooleanHandler>() });
	action_map.insert({ "PointerDereferenceHandler", std::make_shared<PointerDereferenceHandler>() });
	action_map.insert({ "AddressOfHandler", std::make_shared<AddressOfHandler>() });
	action_map.insert({ "VarListHandler", std::make_shared<VarListHandler>() });
	action_map.insert({ "ExpressionListHandler", std::make_shared<ExpressionListHandler>() });
	action_map.insert({ "CallHandler", std::make_shared<CallHandler>() });
	action_map.insert({ "VarDeclHandler", std::make_shared<VarDeclHandler>() });
	action_map.insert({ "MinS8Handler", std::make_shared<MinS8Handler>() });
	action_map.insert({ "MaxS8Handler", std::make_shared<MaxS8Handler>() });
	action_map.insert({ "MinU8Handler", std::make_shared<MinU8Handler>() });
	action_map.insert({ "MaxU8Handler", std::make_shared<MaxU8Handler>() });
	action_map.insert({ "MinS16Handler", std::make_shared<MinS16Handler>() });
	action_map.insert({ "MaxS16Handler", std::make_shared<MaxS16Handler>() });
	action_map.insert({ "MinU16Handler", std::make_shared<MinU16Handler>() });
	action_map.insert({ "MaxU16Handler", std::make_shared<MaxU16Handler>() });
	action_map.insert({ "MinS32Handler", std::make_shared<MinS32Handler>() });
	action_map.insert({ "MaxS32Handler", std::make_shared<MaxS32Handler>() });
	action_map.insert({ "MinU32Handler", std::make_shared<MinU32Handler>() });
	action_map.insert({ "MaxU32Handler", std::make_shared<MaxU32Handler>() });
	action_map.insert({ "MinS64Handler", std::make_shared<MinS64Handler>() });
	action_map.insert({ "MaxS64Handler", std::make_shared<MaxS64Handler>() });
	action_map.insert({ "MinU64Handler", std::make_shared<MinU64Handler>() });
	action_map.insert({ "MaxU64Handler", std::make_shared<MaxU64Handler>() });
	action_map.insert({ "EnsuresHandler", std::make_shared<EnsuresHandler>() });
	action_map.insert({ "LemmaHandler", std::make_shared<LemmaHandler>() });
	action_map.insert({ "AxiomHandler", std::make_shared<AxiomHandler>() });
	action_map.insert({ "RequiresHandler", std::make_shared<RequiresHandler>() });
	action_map.insert({ "VariantHandler", std::make_shared<VariantHandler>() });
	action_map.insert({ "InvariantHandler", std::make_shared<InvariantHandler>() });
	action_map.insert({ "WritesHandler", std::make_shared<WritesHandler>() });
	action_map.insert({ "FreesHandler", std::make_shared<FreesHandler>() });
	action_map.insert({ "AssertHandler", std::make_shared<AssertHandler>() });
	action_map.insert({ "AssumeHandler", std::make_shared<AssumeHandler>() });
	action_map.insert({ "NameHandler", std::make_shared<NameHandler>() });
	action_map.insert({ "IdentifierHandler", std::make_shared<IdentifierHandler>() });
	action_map.insert({ "NumberHandler", std::make_shared<NumberHandler>() });
	action_map.insert({ "RResultHandler", std::make_shared<RResultHandler>() });
	action_map.insert({ "LResultHandler", std::make_shared<LResultHandler>() });
	action_map.insert({ "GhostHandler", std::make_shared<GhostHandler>() });
	action_map.insert({ "IndexRangeHandler", std::make_shared<IndexRangeHandler>() });
	action_map.insert({ "PostfixHandler", std::make_shared<PostfixHandler>() });

	rparser = PEG::Parser::from_str(load(RValueFunc), action_map);
	lparser = PEG::Parser::from_str(load(LValueFunc), action_map);

	configured = true;
}

///
/// @brief Parses a verification condition.
///
/// @param inp The string representing the VC
/// @param parse_type Either LValueFunc or RValueFunc
/// @return The VC as an IR AST
///
std::shared_ptr<IRNode> parse_ver(std::string inp, ParseType parse_type)
{
	configure();

	std::shared_ptr<PEG::Parser> correct_parser;
	if (parse_type == RValueFunc)
	{
		correct_parser = rparser;
	}
	else
	{
		correct_parser = lparser;
	}

	auto result = correct_parser->parse(inp);

	if (result->isFullSuccess())
	{
		auto vc = safety_cast<IRNode>(
				safety_cast<PEGIRValue>(
					safety_cast<PEG::ParseTree::PlainParseTree>(
						result->valueOutput)->children[0])
				->c_value);
		return vc;
	}

	throw SyntaxErrorException(inp);
}

}
