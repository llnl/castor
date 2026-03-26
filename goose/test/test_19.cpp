// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Bootstrap/BootstrapParser.h>
#include <PEG/Expressions/EmptyExpression.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <PEG/Value/ValueInterface.h>
#include <cassert>
#include <iostream>
#include <ostream>
#include <unordered_map>

class CalculatorValue final: public PEG::ParseTree::PlainParseTree {
public:
    int val;

    CalculatorValue(
        const int &calc_val, const std::string &label, const int &position, const std::string &value,
        const std::vector<PEG::ValueInterfacePtr> &children
    ) : PlainParseTree(label, position, value, children), val(calc_val) {}
};

class NumFactory final: public PEG::ValueFactoryInterface {
public:
    PEG::ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<PEG::ValueInterfacePtr> &children
    ) override {
        std::cout << "NUMBER FACTORY" << std::endl;
        int calc_val = std::stoi(value);
        return std::make_shared<CalculatorValue>(calc_val, label, position, value, children);
    }
};

class OpFactory final: public PEG::ValueFactoryInterface {
public:
    enum OPERATION { PLUS, MINUS };
    OPERATION op;

    explicit OpFactory(const OPERATION &operation) : op(operation) {}

    PEG::ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<PEG::ValueInterfacePtr> &children
    ) override {
        std::cout << "SUM FACTORY" << std::endl;
        // Num '-' Expr
        // Num
        const auto num_tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(children[0]);
        const auto num_calc_value = PEG::ValueInterface::downcast<CalculatorValue>(num_tree->children[0]);
        const auto value_lhs = num_calc_value->val;

        // children[1] is '-'

        // Expr
        const auto expr_tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(children[2]);
        const auto num_inside_expr = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(expr_tree->children[0]);
        const auto expr_calc_value = PEG::ValueInterface::downcast<CalculatorValue>(num_inside_expr->children[0]);
        const auto value_rhs = expr_calc_value->val;

        int op_val;
        if (op == PLUS) {
            op_val = value_lhs + value_rhs;
        } else if (op == MINUS) {
            op_val = value_lhs - value_rhs;
        }
        return std::make_shared<CalculatorValue>(op_val, label, position, value, children);
    }
};

class NumLift final: public PEG::ValueFactoryInterface {
public:
    PEG::ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<PEG::ValueInterfacePtr> &children
    ) override {
        return children[0];
    }
};

int
main() {
    const std::string input_grammar = R"N(
    Expr <- Num '+' Expr @sum
         / Num '-' Expr @minus
         / Num
    Num <- '0' @num
        / '1' @num
        / '2' @num
        / '3' @num
        / '4' @num
        / '5' @num
        / '6' @num
        / '7' @num
        / '8' @num
        / '9' @num
    )N";

    auto calc_action_map = std::unordered_map<std::string, PEG::ValueFactoryInterfacePtr>();
    calc_action_map.insert({"num", std::shared_ptr<PEG::ValueFactoryInterface>(new NumFactory())});
    calc_action_map.insert({"lift", std::make_shared<NumLift>()});
    calc_action_map.insert({"sum", std::make_shared<OpFactory>(OpFactory::PLUS)});
    calc_action_map.insert({"minus", std::make_shared<OpFactory>(OpFactory::MINUS)});

    const auto parser = PEG::Parser::from_str(input_grammar, calc_action_map);
    const std::string input = "2-4";
    const auto result = parser->parse(input);
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput)->print(0);

    const auto valueOutput = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput);
    const auto calc_val_output = PEG::ValueInterface::downcast<CalculatorValue>(valueOutput->children[0]);
    const auto value = calc_val_output->val;
    std::cout << std::endl << "VALUE: " << value << std::endl;

    assert(result->isFullSuccess());

    return 0;
}