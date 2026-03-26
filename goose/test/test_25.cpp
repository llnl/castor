// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "PEG/Expressions/NonTerminalExpression.h"
#include "PEG/Expressions/SeqNExpression.h"
#include "PEG/Expressions/TerminalExpression.h"
#include "PEG/ParseResult.h"
#include <PEG/Parser.h>
#include <cassert>
#include <iostream>
#include <string>

/**
 *
 * E <- E1 + E2 / E2 * E2 / 'n'
 *
 */

PEG::GrammarPtr
make_grammar() {
    std::string expression = "E";
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> rules;

    rules.insert(std::make_pair(
        expression, std::vector<PEG::ExpressionPtr>(
                        {PEG::Expressions::SeqNExpression::instance(
                             std::vector<PEG::ExpressionPtr>(
                                 {PEG::Expressions::NonTerminalExpression::instance(expression, 1),
                                  PEG::Expressions::TerminalExpression::instance("+"),
                                  PEG::Expressions::NonTerminalExpression::instance(expression, 2)}
                             )
                         ),
                         PEG::Expressions::SeqNExpression::instance(
                             std::vector<PEG::ExpressionPtr>(
                                 {PEG::Expressions::NonTerminalExpression::instance(expression, 2),
                                  PEG::Expressions::TerminalExpression::instance("*"),
                                  PEG::Expressions::NonTerminalExpression::instance(expression, 2)}
                             )
                         ),
                         PEG::Expressions::TerminalExpression::instance("n")}
                    )
    ));

    return PEG::Grammar::instance(expression, rules);
}


int
main() {
    auto parser = PEG::Parser(make_grammar());

    const std::string input_0 = "n+n+n";
    auto const result_0 = parser.parse(input_0);

    const std::string input_1 = "n*n*n";
    auto const result_1 = parser.parse(input_1);

    const std::string input_2 = "n+n*n";
    auto const result_2 = parser.parse(input_2);

    const std::string input_3 = "n*n+n";
    auto const result_3 = parser.parse(input_3);

    std::cout << std::endl;
    parser.grammar->print();
    std::cout << "parsed" << std::endl;

    assert(result_0->isSuccess() && result_1->isFullSuccess() && result_2->isSuccess() && result_3->isFullSuccess());

    return 0;
}