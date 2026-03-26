// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <PEG/Expressions/OptionExpression.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <iostream>
#include <string>

PEG::ParseResultPtr
PEG::Expressions::OptionExpression::parse(GrammarPtr grammar, const std::string &input, const int &position) {
    // "Option[E]" can be desugared as "E / Empty"
    // which can be understood as: try to parse E, if not just parse nothing and return

    const auto result_expr = option_expr->parse(grammar, input, position);

    if (!result_expr->isSuccess()) {
        const ValueInterfacePtr emptyResult =
            value_factory->createParseTree("Empty", -1, "", std::vector<ValueInterfacePtr>());
        return ParseResultType::SuccessParseResult::instance(emptyResult, position, input.size());
    }

    const auto result_tree =
        value_factory->createParseTree("Option", -1, "", std::vector<ValueInterfacePtr>({result_expr->valueOutput}));
    const auto new_position = result_expr->position;

    return ParseResultType::SuccessParseResult::instance(result_tree, new_position, input.size());
}

std::string
PEG::Expressions::OptionExpression::info() {
    return "Optional expression of (" + option_expr->info() + ")";
}

PEG::ExpressionPtr
PEG::Expressions::OptionExpression::instance(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory) {
    return std::make_shared<OptionExpression>(std::move(expr), std::move(valueFactory));
}

PEG::ExpressionPtr
PEG::Expressions::OptionExpression::instance(ExpressionPtr expr) {
    return instance(std::move(expr), ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>());
}

PEG::EXPRESSION_TYPE
PEG::Expressions::OptionalExpression::get_expr_type() {
    return PEG_OPTION;
}

std::string
PEG::Expressions::OptionalExpression::print() {
    return "(" + option_expr->print() + ")?";
}

std::vector<std::string>
PEG::Expressions::OptionalExpression::get_non_terms() {
    return option_expr->get_non_terms();
}