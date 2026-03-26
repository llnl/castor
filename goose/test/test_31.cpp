// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <iostream>
#include <string>

int
main() {
    const std::string input_grammar =
        R"N(
Name   <- Number / [A-Za-z_][A-Za-z0-9_]*
Number <- [0-9]+
)N";

    const auto parser = PEG::Parser::from_str(input_grammar);

    parser->grammar->print();

    const std::string input = "1";
    const auto result = parser->parse(input);

    result->output();
    if(result->isSuccess()) {
        PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput)->print(0);
    }
    if(!result->isFullSuccess()) {
        std::cout << "remainder: " << input.substr(result->position) << std::endl;
    }
    std::cout << result->isSuccess() << " && " << result->isFullSuccess() << std::endl;
    return 0;
}