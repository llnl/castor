// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <PEG/Expressions/NotExpression.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseResultType/FailParseResult.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <iostream>
#include <string>

PEG::ParseResultPtr
PEG::Expressions::NotExpression::parse(GrammarPtr grammar, const std::string &input, const int &position) {
    // not lets expr parse, and if it succeeds it fails
    //   if it fails, then it succeeds and the remainder is reset to the input

    const auto result_expr = expr->parse(grammar, input, position);

    if (result_expr->isSuccess()) {
        return ParseResultType::FailParseResult::instance(result_expr->position);
    }

    const auto result_tree = value_factory->createParseTree("not", -1, "", std::vector<ValueInterfacePtr>());
    return ParseResultType::SuccessParseResult::instance(result_tree, position, input.size());
}

std::string
PEG::Expressions::NotExpression::info() {
    return "not expression of (" + expr->info() + ")";
}

PEG::ExpressionPtr
PEG::Expressions::NotExpression::instance(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory) {
    return std::make_shared<NotExpression>(std::move(expr), std::move(valueFactory));
}

PEG::ExpressionPtr
PEG::Expressions::NotExpression::instance(ExpressionPtr expr) {
    return instance(std::move(expr), ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>());
}

PEG::EXPRESSION_TYPE
PEG::Expressions::NotExpression::get_expr_type() {
    return PEG_NOT;
}

std::string
PEG::Expressions::NotExpression::print() {
    return "!(" + expr->print() + ")";
}

std::vector<std::string>
PEG::Expressions::NotExpression::get_non_terms() {
    return expr->get_non_terms();
}