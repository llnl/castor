// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/Expressions/EmptyExpression.h>
#include <PEG/Expressions/NonTerminalExpression.h>
#include <PEG/Expressions/SequenceExpression.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <fstream>
#include <iostream>
#include <string>

PEG::ExpressionPtr seq(PEG::ExpressionPtr expr1, PEG::ExpressionPtr expr2) {
    return PEG::Expressions::SequenceExpression::instance(std::move(expr1), std::move(expr2));
}
PEG::ExpressionPtr non_term(const std::string &nt) {
    return PEG::Expressions::NonTerminalExpression::instance(nt);
}
PEG::ExpressionPtr term(const std::string &t) {
    return PEG::Expressions::TerminalExpression::instance(t);
}
PEG::ExpressionPtr empty() {
    return PEG::Expressions::EmptyExpression::instance();
}

int
main() {
    auto input_grammar = PEG::Grammar::instance(
        "L",
        {{"L",                                // <-
          {seq(non_term("L"), non_term("N")), // /
           empty()}},
         {"N", {term("0")}}}
    );

    auto parser = PEG::Parser(input_grammar);

    parser.grammar->print();

    const auto answer = parser.grammar->find_all_left_recursion();

    std::cout << "left recursive terminals: {";
    for (const auto &i : answer) {
        std::cout << i << ", ";
    }
    std::cout << "}" << std::endl;

    const std::string input = "000";
    const auto result = parser.parse(input);

    result->output();
    if(result->isSuccess()) {
        PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput)->print(0);
    }
    if(!result->isFullSuccess()) {
        std::cout << "remainder: " << input.substr(result->position) << std::endl;
    }
    std::cout << result->isSuccess() << " && " << result->isFullSuccess() << std::endl;
    std::cout << input << std::endl;
}