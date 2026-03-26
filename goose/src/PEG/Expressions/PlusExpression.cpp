// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "PEG/Grammar.h"

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <PEG/Expressions/PlusExpression.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseResultType/FailParseResult.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <iostream>
#include <string>

PEG::ParseResultPtr
PEG::Expressions::PlusExpression::parse(GrammarPtr grammar, const std::string &input, const int &position) {
    // Plus parses expr one of more times
    // Plus[E] is desugared into Sequence[E, Repeat[E]]

    auto result_expr = expr->parse(grammar, input, position);

    if (!result_expr->isSuccess()) {
        return ParseResultType::FailParseResult::instance(result_expr->position);
    }

    auto plus_value_children = std::vector<ValueInterfacePtr>();
    int new_position = result_expr->position;
    while (result_expr->isSuccess()) {
        new_position = result_expr->position;
        plus_value_children.push_back(result_expr->valueOutput);
        new_position = grammar->parse_whitespace(input, new_position);
        result_expr = expr->parse(grammar, input, new_position);
    }

    const ValueInterfacePtr result_tree =
        value_factory->createParseTree("Plus", -1, "", std::vector<ValueInterfacePtr>({plus_value_children}));

    return ParseResultType::SuccessParseResult::instance(result_tree, new_position, input.size());
}

std::string
PEG::Expressions::PlusExpression::info() {
    return "Plus (One or more) expression of (" + expr->info() + ")";
}

PEG::ExpressionPtr
PEG::Expressions::PlusExpression::instance(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory) {
    return std::make_shared<PlusExpression>(std::move(expr), std::move(valueFactory));
}

PEG::ExpressionPtr
PEG::Expressions::PlusExpression::instance(ExpressionPtr expr) {
    return instance(std::move(expr), ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>());
}

PEG::EXPRESSION_TYPE
PEG::Expressions::PlusExpression::get_expr_type() {
    return PEG_PLUS;
}

std::string
PEG::Expressions::PlusExpression::print() {
    return "[" + expr->print() + "]+";
}

std::vector<std::string>
PEG::Expressions::PlusExpression::get_non_terms() {
    return expr->get_non_terms();
}