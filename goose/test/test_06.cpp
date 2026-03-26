// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Parser.h>
#include <PEG/Value/ValueInterface.h>
#include <cassert>
#include <iostream>

// g++ -Wall -O3 -I.. -std=c++14 peg.cpp -o peg

PEG::GrammarPtr
make_grammar(const PEG::ValueFactoryInterfacePtr value_factory_interface) {
    // std::vector<std::string> rules = {"expr <- 2"};
    // Grammar grammar(rules);

    std::string g_start = "expr";
    const PEG::ExpressionPtr g_term = PEG::Expressions::TerminalExpression::instance("2", value_factory_interface);

    std::vector<PEG::ExpressionPtr> g_rule({g_term});

    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> g_rules;
    // rules["expr"] = g_rule;
    g_rules.insert(std::make_pair(g_start, g_rule));

    return PEG::Grammar::instance(g_start, g_rules);
}

int
main() {
    std::cout << "hello, world to test terminal!" << std::endl;

    const auto plain_parse_result_factory = PEG::ParseTree::PlainParseTreeFactory::instance<PEG::ParseTree::PlainParseTreeFactory>();

    auto parser = PEG::Parser(make_grammar(plain_parse_result_factory));
    const auto parseResult = parser.parse("2");
    const auto parseTree = parseResult->valueOutput;
    const auto plainParseTree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(parseTree);
    plainParseTree->print(0);
    parseResult->output();
    std::cout << " input : " << "2" << std::endl;

    assert(parseResult->isFullSuccess());

    return 0;
}