// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <PEG/Expressions/RepeatExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <iostream>
#include <string>

PEG::ParseResultPtr
PEG::Expressions::RepeatExpression::parse(GrammarPtr grammar, const std::string &input, const int &position) {
    // Repeat lets expr parse until it fails
    //  once expr parse fails, repeat succeeds with whatever is the most recent remainder,
    //  or it succeeds by parsing nothing

    auto result_expr = expr->parse(grammar, input, position);

    if (!result_expr->isSuccess()) {
        const auto valueOutput = value_factory->createParseTree("Empty", -1, "", std::vector<ValueInterfacePtr>());
        return ParseResultType::SuccessParseResult::instance(valueOutput, position, input.size());
    }

    if (result_expr->position == position) {
        const auto output = value_factory->createParseTree("repeat", -1, "", {result_expr->valueOutput});
        return ParseResultType::SuccessParseResult::instance(output, position, input.size());
    }

    auto repeat_value_children = std::vector<ValueInterfacePtr>();

    auto next_position = result_expr->position;
    while (result_expr->isSuccess()) {
        next_position = result_expr->position; // A
        next_position = grammar->parse_whitespace(input, next_position);
        repeat_value_children.push_back(result_expr->valueOutput);
        result_expr = expr->parse(grammar, input, next_position);
    }

    const ValueInterfacePtr result = value_factory->createParseTree("repeat", -1, "", repeat_value_children);
    return ParseResultType::SuccessParseResult::instance(result, next_position, input.size());
}

std::string
PEG::Expressions::RepeatExpression::info() {
    return "Repeating expressions of (" + expr->info() + ")";
}

PEG::ExpressionPtr
PEG::Expressions::RepeatExpression::instance(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory) {
    return std::make_shared<RepeatExpression>(std::move(expr), std::move(valueFactory));
}

PEG::ExpressionPtr
PEG::Expressions::RepeatExpression::instance(ExpressionPtr expr) {
    return instance(std::move(expr), ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>());
}

PEG::EXPRESSION_TYPE
PEG::Expressions::RepeatExpression::get_expr_type() {
    return PEG_REPEAT;
}

std::string
PEG::Expressions::RepeatExpression::print() {
    return "[" + expr->print() + "]*";
}

std::vector<std::string>
PEG::Expressions::RepeatExpression::get_non_terms() {
    return expr->get_non_terms();
}