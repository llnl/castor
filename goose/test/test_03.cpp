// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expressions/NonTerminalExpression.h>
#include <PEG/Expressions/SequenceExpression.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Parser.h>
#include <PEG/Value/ValueInterface.h>
#include <cassert>
#include <iostream>

PEG::GrammarPtr
make_grammar(const PEG::ValueFactoryInterfacePtr value_factory_interface) {
    // expr -> 2 / next / 4 + last
    // next -> 1 / last
    // last -> 3

    std::string g_start = "expr";
    std::string g_next = "next";
    std::string g_last = "last";
    PEG::ExpressionPtr g_term_1 = PEG::Expressions::TerminalExpression::instance("2", value_factory_interface);
    PEG::ExpressionPtr g_non_term_1 = PEG::Expressions::NonTerminalExpression::instance(g_next, value_factory_interface);
    PEG::ExpressionPtr g_term_2 = PEG::Expressions::TerminalExpression::instance("1", value_factory_interface);
    PEG::ExpressionPtr g_non_term_2 = PEG::Expressions::NonTerminalExpression::instance("last", value_factory_interface);
    PEG::ExpressionPtr g_term_3 = PEG::Expressions::TerminalExpression::instance("3", value_factory_interface);

    PEG::ExpressionPtr g_term_4 = PEG::Expressions::SequenceExpression::instance(
        PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::TerminalExpression::instance("4", value_factory_interface),
            PEG::Expressions::TerminalExpression::instance("+", value_factory_interface), value_factory_interface
        ),
        PEG::Expressions::NonTerminalExpression::instance(g_last, value_factory_interface), value_factory_interface
    );

    std::vector<PEG::ExpressionPtr> g_rule_1({g_term_1, g_non_term_1, g_term_4});
    std::vector<PEG::ExpressionPtr> g_rule_2({g_term_2, g_non_term_2});
    std::vector<PEG::ExpressionPtr> g_rule_3({g_term_3});

    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> g_rules;
    g_rules.insert(std::make_pair(g_start, g_rule_1));
    g_rules[g_next] = g_rule_2;
    g_rules.insert(std::make_pair(g_last, g_rule_3));

    return PEG::Grammar::instance(g_start, g_rules);
}

int
main() {
    std::cout << "hello, world to test sequence!" << std::endl;

    const auto plain_parse_result_factory = PEG::ParseTree::PlainParseTreeFactory::instance<PEG::ParseTree::PlainParseTreeFactory>();

    auto parser = PEG::Parser(make_grammar(plain_parse_result_factory));
    const std::string input = "4+3";
    const auto parseResult = parser.parse(input);
    parseResult->output();
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(parseResult->valueOutput)->print(0);
    std::cout << " input : " << input << std::endl;

    assert(parseResult->isFullSuccess());
    return 0;
}