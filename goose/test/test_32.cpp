// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <cassert>
#include <iostream>
#include <string>

int
main() {
    const std::string input_grammar = "O <- '\\\\/' / '/\\\\'";
    const std::string raw_grammar = R"N(O <- '\\/' / '/\\')N";

    std::cout << input_grammar << std::endl;
    std::cout << raw_grammar << std::endl;

    const auto parser = PEG::Parser::from_str(input_grammar);
    const auto raw_parser = PEG::Parser::from_str(raw_grammar);

    parser->grammar->print();
    raw_parser->grammar->print();

    // const std::string input = R"N(\/)N";
    // const std::string input = "\\/";
    const std::string conjunction = "\\/";
    const std::string disjunction = "/\\";
    std::cout << "our input for today: " << conjunction << " and  " << disjunction << std::endl;
    const auto conj_result = parser->parse(conjunction);
    const auto disj_result = parser->parse(disjunction);
    const auto raw_conj_result = raw_parser->parse(conjunction);
    const auto raw_disj_result = raw_parser->parse(disjunction);

    conj_result->output();
    if(conj_result->isSuccess()) {
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(conj_result->valueOutput)->print(0);
    }
    if(!conj_result->isFullSuccess()) {
    std::cout << "conjunction: " << conjunction << std::endl;
    std::cout << "remainder: " << conjunction.substr(conj_result->position) << std::endl;
    }
    std::cout << conj_result->isSuccess() << " && " << conj_result->isFullSuccess() << std::endl;

    disj_result->output();
    if(disj_result->isSuccess()) {
        PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(disj_result->valueOutput)->print(0);
    }
    if(!disj_result->isFullSuccess()) {
        std::cout << "conjunction: " << conjunction << std::endl;
        std::cout << "remainder: " << conjunction.substr(disj_result->position) << std::endl;
    }
    std::cout << disj_result->isSuccess() << " && " << disj_result->isFullSuccess() << std::endl;

    raw_conj_result->output();
    if(raw_conj_result->isSuccess()) {
        PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(raw_conj_result->valueOutput)->print(0);
    }
    if(!raw_conj_result->isFullSuccess()) {
        std::cout << "conjunction: " << conjunction << std::endl;
        std::cout << "remainder: " << conjunction.substr(raw_conj_result->position) << std::endl;
    }
    std::cout << raw_conj_result->isSuccess() << " && " << raw_conj_result->isFullSuccess() << std::endl;

    raw_disj_result->output();
    if(raw_disj_result->isSuccess()) {
        PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(raw_disj_result->valueOutput)->print(0);
    }
    if(!raw_disj_result->isFullSuccess()) {
        std::cout << "conjunction: " << conjunction << std::endl;
        std::cout << "remainder: " << conjunction.substr(raw_disj_result->position) << std::endl;
    }
    std::cout << raw_disj_result->isSuccess() << " && " << raw_disj_result->isFullSuccess() << std::endl;

    assert(conj_result->isFullSuccess() && disj_result->isFullSuccess()
        && raw_conj_result->isFullSuccess() && raw_disj_result->isFullSuccess());
    return 0;
}