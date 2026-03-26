// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expressions/NonTerminalExpression.h>
#include <PEG/Expressions/SeqNExpression.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Parser.h>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

/**
 *
 * E <- N + E / N - E / N
 * N <- 1 / 2 / 3 / 4 / ...
 *
 */

PEG::GrammarPtr
make_grammar() {
    std::string expression = "E";
    const std::string number = "N";
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> rules;

    const auto value_factory_interface = PEG::ParseTree::PlainParseTreeFactory::instance<PEG::ParseTree::PlainParseTreeFactory>();

    rules.insert(std::make_pair(
        expression, std::vector<PEG::ExpressionPtr>(
                        {PEG::Expressions::SeqNExpression::instance(
                             std::vector<PEG::ExpressionPtr>(
                                 {PEG::Expressions::NonTerminalExpression::instance(number, value_factory_interface),
                                  PEG::Expressions::TerminalExpression::instance("+", value_factory_interface),
                                  PEG::Expressions::NonTerminalExpression::instance(expression, value_factory_interface)}
                             ),
                             value_factory_interface
                         ),
                         PEG::Expressions::SeqNExpression::instance(
                             std::vector<PEG::ExpressionPtr>(
                                 {PEG::Expressions::NonTerminalExpression::instance(number, value_factory_interface),
                                  PEG::Expressions::TerminalExpression::instance("-", value_factory_interface),
                                  PEG::Expressions::NonTerminalExpression::instance(expression, value_factory_interface)}
                             ),
                             value_factory_interface
                         ),
                         PEG::Expressions::NonTerminalExpression::instance(number, value_factory_interface)}
                    )
    ));

    rules.insert(std::make_pair(
        number, std::vector<PEG::ExpressionPtr>({
                    PEG::Expressions::TerminalExpression::instance("1", value_factory_interface),
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

    return PEG::Grammar::instance(expression, rules);
}

int
main() {
    const auto plain_parse_tree_factory = PEG::ParseTree::PlainParseTreeFactory::instance<PEG::ParseTree::PlainParseTreeFactory>();

    auto calculator_grammar = make_grammar();
    auto parser = PEG::Parser(calculator_grammar);
    const std::string input = "2+3-5";

    const auto parse_result = parser.parse(input);
    calculator_grammar->print();
    parse_result->output();
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(parse_result->valueOutput)->print(0);
    std::cout << "input : " << input << std::endl;

    assert(parse_result->isFullSuccess());

    return 0;
}