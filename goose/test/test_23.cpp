// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "PEG/ParseResult.h"
#include "PEG/ParseTree/PlainParseTree.h"
#include "PEG/Value/ValueInterface.h"

#include <PEG/Parser.h>
#include <iostream>
#include <string>

/**
 *
 * E <- E + 'n' / 'n'
 *
 */

int
main() {
    const std::string input_grammar = R"N(
        E <- M '+' E / M
        M <- M '-' 'n' / 'n'
    )N";

    const auto parser = PEG::Parser::from_str(input_grammar);

    parser->grammar->print();

    const std::string input_0 = "n+n+n";
    auto const result_0 = parser->parse(input_0);

    std::cout << std::endl;

    const std::string input_1 = "n-n-n";
    const auto result_1 = parser->parse(input_1);

    std::cout << std::endl;
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result_0->valueOutput)->print(0);
    std::cout << "input_0: " << input_0 << std::endl;
    std::cout << std::endl;
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result_1->valueOutput)->print(0);
    std::cout << "input_1: " << input_1 << std::endl;

    return 0;
}