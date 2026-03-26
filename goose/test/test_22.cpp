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
        Expr <- Expr '+' 'n' / 'n'
    )N";

    const auto parser = PEG::Parser::from_str(input_grammar);

    parser->grammar->print();

    const std::string input = "n+n+n";
    auto const result = parser->parse(input);
    std::cout << std::endl;
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput)->print(0);
    std::cout << "input: " << input << std::endl;
    return 0;
}