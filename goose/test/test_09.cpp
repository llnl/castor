// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/BasicTypes.h>
#include <PEG/Expressions/NonTerminalExpression.h>
#include <PEG/Expressions/SeqNExpression.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/ParseResult.h>
#include <PEG/Parser.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <PEG/Value/ValueInterface.h>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace PEG {
/*
 */

/**
 *
 * E <- N + E / N - E / N
 * N <- 1 / 2 / 3 / 4 / ...
 *
 */

class CalculatorValue;
using CalculatorPtr = std::shared_ptr<CalculatorValue>;

class CalculatorValue: public ValueInterface {
public:
    ~CalculatorValue() override = default;

    template <class Self, typename... Args> static CalculatorPtr instance(Args... args) {
        return std::make_shared<Self>(args...);
    }
};

class IntCalcVal final: public CalculatorValue {
public:
    int value;

    explicit IntCalcVal(const int &val) : value(val) {}
};

class OpCalcVal final: public CalculatorValue {
public:
    char operation;

    explicit OpCalcVal(const char &operation) : operation(operation) {}

    CalculatorPtr doOperation(const IntCalcVal &val1, const IntCalcVal &val2) const {
        const int a = val1.value;
        const int b = val2.value;
        if (operation == '+') {
            return std::make_shared<IntCalcVal>(a + b);
        }
        if (operation == '-') {
            return std::make_shared<IntCalcVal>(a - b);
        }
        assert(false);
    }
};

class CalculatorFactory final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        if (label == "non_terminal") {
            // position - rule no
            // value - name of the non-terminal that was used
            // children - input parsed so far constructed into value-interface/parse-tree
            // value can be either E or N
            assert(!children.empty());
            return ValueInterface::downcast<CalculatorValue>(children[0]);
        }
        if (label == "terminal") {
            // value - contains a number
            //         or an operation
            if (value == "+" || value == "-") {
                return CalculatorValue::instance<OpCalcVal>(value[0]);
            }
            const auto val = std::stoi(value);
            // return std::shared_ptr<CalculatorValue>(new IntCalcVal(val));
            return CalculatorValue::instance<IntCalcVal>(val);
        }
        if (label == "SeqN") {
            // children - 3 in this case
            //  hacky but first is a val, second is an op, third is a val
            assert(children.size() == 3);
            const auto child_a = ValueInterface::downcast<IntCalcVal>(children[0]);
            const auto child_op = ValueInterface::downcast<OpCalcVal>(children[1]);
            const auto child_b = ValueInterface::downcast<IntCalcVal>(children[2]);
            return child_op->doOperation(*child_a, *child_b);
        }
        assert(false);
    }
};

class CalculatorPlus final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        std::cout << "Calculator Plus Override Called" << std::endl;
        assert(children.size() == 3);
        const auto child_a = ValueInterface::downcast<IntCalcVal>(children[0]);
        // const auto child_op = ValueInterface::downcast<OpCalcVal>(children[1]);
        const auto child_b = ValueInterface::downcast<IntCalcVal>(children[2]);
        // return child_op->doOperation(*child_a, *child_b);
        return CalculatorValue::instance<IntCalcVal>(child_a->value + child_b->value);
    }
};

class CalculatorMinus final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        assert(children.size() == 3);
        const auto child_a = ValueInterface::downcast<IntCalcVal>(children[0]);
        // const auto child_op = ValueInterface::downcast<OpCalcVal>(children[1]);
        const auto child_b = ValueInterface::downcast<IntCalcVal>(children[2]);
        // return child_op->doOperation(*child_a, *child_b);
        return CalculatorValue::instance<IntCalcVal>(child_a->value - child_b->value);
    }
};

class CalculatorNumberOne final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        std::cout << "Calculator NumberOne Override Called" << std::endl;
        return CalculatorValue::instance<IntCalcVal>(1);
    }
};

} // namespace PEG

int
main() {
    auto value_factory_interface = PEG::CalculatorFactory::instance<PEG::CalculatorFactory>();

    auto plain_parse_tree_factory = PEG::ValueFactoryInterface::instance<PEG::ParseTree::PlainParseTreeFactory>();

    std::string expression = "E";
    const std::string number = "N";
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> rules;

    auto calculator_plus = PEG::CalculatorPlus::instance<PEG::CalculatorPlus>();
    auto calculator_minus = PEG::CalculatorMinus::instance<PEG::CalculatorMinus>();
    auto calculator_number = PEG::CalculatorNumberOne::instance<PEG::CalculatorNumberOne>();

    /**
     *
     *term E
     *term N
     *
     *action calculator_plus
     *
     * E <- N + E   - calculator_plus
     *    / N - E
     *    / N
     *
     */
    rules.insert(std::make_pair(
        expression,
        std::vector<PEG::ExpressionPtr>(
            {PEG::Expressions::SeqNExpression::instance(
                 std::vector<PEG::ExpressionPtr>(
                     {PEG::Expressions::NonTerminalExpression::instance(number, value_factory_interface),
                      PEG::Expressions::TerminalExpression::instance("+", value_factory_interface),
                      PEG::Expressions::NonTerminalExpression::instance(expression, value_factory_interface)}
                 ),
                 calculator_plus // plus semantic action
             ),
             PEG::Expressions::SeqNExpression::instance(
                 std::vector<PEG::ExpressionPtr>(
                     {PEG::Expressions::NonTerminalExpression::instance(number, value_factory_interface),
                      PEG::Expressions::TerminalExpression::instance("-", value_factory_interface),
                      PEG::Expressions::NonTerminalExpression::instance(expression, value_factory_interface)}
                 ),
                 calculator_minus // minus
             ),
             PEG::Expressions::NonTerminalExpression::instance(number, value_factory_interface)} // default, parses the number -> int
        )
    ));

    // N <- 1 / 2 / 3 / 4 / ...
    rules.insert(std::make_pair(
        number, std::vector<PEG::ExpressionPtr>({
                    PEG::Expressions::TerminalExpression::instance("1", calculator_number),
                    PEG::Expressions::TerminalExpression::instance("2", value_factory_interface),
                    PEG::Expressions::TerminalExpression::instance("3", value_factory_interface),
                    PEG::Expressions::TerminalExpression::instance("4", value_factory_interface),
                    PEG::Expressions::TerminalExpression::instance("5", value_factory_interface),
                    PEG::Expressions::TerminalExpression::instance("6", value_factory_interface),
                    PEG::Expressions::TerminalExpression::instance("7", value_factory_interface),
                    PEG::Expressions::TerminalExpression::instance("8", value_factory_interface),
                    PEG::Expressions::TerminalExpression::instance("9", value_factory_interface),
                    PEG::Expressions::TerminalExpression::instance("0", value_factory_interface),
                })
    ));

    const auto grammar = PEG::Grammar::instance(expression, rules);

    auto parser = PEG::Parser(grammar);
    const auto input = "1+5-2"; // 4
    const auto parse_result = parser.parse(input, value_factory_interface);
    grammar->print();
    parse_result->output();
    const auto output_value = PEG::ValueInterface::downcast<PEG::IntCalcVal>(parse_result->valueOutput)->value;
    std::cout << "calculator value : " << output_value << std::endl;
    std::cout << "input : " << input << std::endl;

    assert(output_value == 1 + 5 - 2);
    return 0;
}