// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <fstream>
#include <iostream>
#include <string>

int
main() {
    const std::string input_grammar = R"N(
    S <- A B / C B
    A <- B C / C A / 'a'
    B <- C A / D B / 'b'
    C <- B A / A D / 'a'
    D <- A C / B D / 'b'
)N";

    const auto parser = PEG::Parser::from_str(input_grammar);

    parser->grammar->print();

    // parser->grammar.doLeftRecursion = false;

    const auto answer = parser->grammar->find_all_left_recursion();

    std::cout << "left recursive terminals: {";
    for (const auto &i : answer) {
        std::cout << i << ", ";
    }
    std::cout << "}" << std::endl;

    return !(answer == std::set<std::string>{"A", "B", "C", "D"});
}