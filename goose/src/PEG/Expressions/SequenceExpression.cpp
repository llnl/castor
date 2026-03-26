// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <PEG/Expressions/SequenceExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseResultType/FailParseResult.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <iostream>
#include <string>

PEG::ParseResultPtr
PEG::Expressions::SequenceExpression::parse(GrammarPtr grammar, const std::string &input, const int &position) {
    // input -> expr1 -> fail/success(remainder)
    // - if fail stop
    // success.remainder -> expr2 -> fail/success
    // - if fail stop
    // take both parse trees, and merge it into one

    const int &new_position = grammar->parse_whitespace(input, position);

    const auto result1 = expr_first->parse(grammar, input, new_position);

    if (!result1->isSuccess()) {
        return ParseResultType::FailParseResult::instance(new_position);
    }

    // what happens when remainder is "" !? (nothing)
    // const std::string input_second = grammar.parse_whitespace(result1->remainder);
    const auto next_position = grammar->parse_whitespace(input, result1->position);

    const auto result2 = expr_second->parse(grammar, input, next_position);

    if (result2->isSuccess()) {
        const ValueInterfacePtr parseTreeSequence =
            value_factory->createParseTree("sequence", -1, "", {result1->valueOutput, result2->valueOutput});
        const std::vector<ValueInterfacePtr> p_tree_result({result1->valueOutput, result2->valueOutput});
        return ParseResultType::SuccessParseResult::instance(parseTreeSequence, result2->position, input.size());
    }

    return ParseResultType::FailParseResult::instance(next_position);
}

std::string
PEG::Expressions::SequenceExpression::info() {
    return "sequence expression of (" + expr_first->info() + "," + expr_second->info() + ")";
}

PEG::ExpressionPtr
PEG::Expressions::SequenceExpression::instance(
    ExpressionPtr expr_1, ExpressionPtr expr_2, ValueFactoryInterfacePtr valueFactory
) {
    return std::make_shared<SequenceExpression>(std::move(expr_1), std::move(expr_2), std::move(valueFactory));
}

PEG::ExpressionPtr
PEG::Expressions::SequenceExpression::instance(ExpressionPtr expr_1, ExpressionPtr expr_2) {
    return instance(
        std::move(expr_1), std::move(expr_2), ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>()
    );
}

PEG::EXPRESSION_TYPE
PEG::Expressions::SequenceExpression::get_expr_type() {
    return PEG_SEQUENCE;
}

std::string
PEG::Expressions::SequenceExpression::print() {
    return expr_first->print() + " " + expr_second->print();
}

std::vector<std::string>
PEG::Expressions::SequenceExpression::get_non_terms() {
    auto f = expr_first->get_non_terms();
    auto s = expr_second->get_non_terms();
    std::vector<std::string> result(f.begin(), f.end());
    result.insert(result.end(), s.begin(), s.end());
    return result;
}

std::set<std::string>
PEG::Expressions::SequenceExpression::find_first(
    const std::unordered_map<std::string, std::set<std::string>> &non_terminal_map_first,
    const std::set<std::string> &nullable_non_terminals
) {
    std::set<std::string> result = expr_first->find_first(non_terminal_map_first, nullable_non_terminals);
    if (expr_first->isNullable(nullable_non_terminals)) {
        const auto next_result = expr_second->find_first(non_terminal_map_first, nullable_non_terminals);
        for (const auto &element : next_result) {
            result.insert(element);
        }
    }
    return result;
}
