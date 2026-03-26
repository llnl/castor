// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Bootstrap/BootstrapParser.h>
#include <PEG/Expressions/EmptyExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <PEG/Value/ValueInterface.h>
#include <cassert>
#include <iostream>
#include <ostream>
#include <unordered_map>

int
main() {
    const auto bootstrap_parser = PEG::Bootstrap::BootstrapParser("", std::unordered_map<std::string, PEG::ValueFactoryInterfacePtr>());
    const auto grammar = bootstrap_parser.peg_grammar;
    grammar->gram.rules[grammar->white_space] = std::vector<PEG::ExpressionPtr>();
    auto parser = PEG::Parser(grammar);

    const std::string input_0 = R"N(
    # Expr <- Num + Expr / Num
    # Expr <- Num '+' Expr / Num
    # Expr <- Num '+' Expr / &(Num) Num
    Expr <- Num '+' Expr / Num
    Num <- 'what' / 'where' / 'who'
    )N";
    grammar->print();

    if (grammar->check_grammar()) {
        std::cout << "grammar is good";
    } else {
        std::cout << "grammar is not good";
    }

    std::cout << "input size: " << input_0.size() << std::endl;
    const auto result_0 = parser.parse(input_0, PEG::ValueFactoryInterface::instance<PEG::Bootstrap::Lift>());
    // TODO: should memo table be cleared
    const auto output = PEG::ValueInterface::downcast<PEG::Bootstrap::GrammarValue>(result_0->valueOutput);
    output->grammar->print();
    std::cout << "\n result_0: " << result_0->isFullSuccess() << std::endl;

    auto output_grammar = output->grammar;
    auto output_parser = PEG::Parser(output_grammar);
    auto result = output_parser.parse("what+where");
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput)->print(0);

    assert(result_0->isFullSuccess());

    return 0;
}