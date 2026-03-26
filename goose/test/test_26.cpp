// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "PEG/Expressions/NonTerminalExpression.h"
#include "PEG/ParseResult.h"
#include "PEG/ParseTree/PlainParseTree.h"
#include "PEG/Value/ValueInterface.h"
#include <PEG/Parser.h>
#include <cassert>
#include <iostream>
#include <string>

/**
 *
 * E <- E1 + E2 / E2 * E2 / 'n'
 *
 */

int
main() {
    const std::string input_grammar = R"N(
        E <- E '+' E^2 / E^2 '*' E^2 / 'n'
    )N";

    const auto parser = PEG::Parser::from_str(input_grammar);

    parser->grammar->print();

    const std::string input_0 = "n+n+n";
    auto const result_0 = parser->parse(input_0);

    const std::string input_1 = "n*n*n";
    auto const result_1 = parser->parse(input_1);

    const std::string input_2 = "n+n*n";
    auto const result_2 = parser->parse(input_2);

    const std::string input_3 = "n*n+n";
    auto const result_3 = parser->parse(input_3);

    std::cout << std::endl;
    parser->grammar->print();
    std::cout << std::endl << "input: " << input_0 << std::endl;
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result_0->valueOutput)->print(0);
    std::cout << std::endl << "input: " << input_1 << std::endl;
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result_1->valueOutput)->print(0);
    std::cout << std::endl << "input: " << input_2 << std::endl;
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result_2->valueOutput)->print(0);
    std::cout << std::endl << "input: " << input_3 << std::endl;
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result_3->valueOutput)->print(0);
    std::cout << "parsed" << std::endl;

    assert(result_0->isSuccess() && result_1->isFullSuccess() && result_2->isSuccess() && result_3->isFullSuccess());

    return 0;
}