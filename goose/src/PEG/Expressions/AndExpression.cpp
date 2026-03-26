// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <PEG/Expressions/AndExpression.h>
#include <PEG/Expressions/NotExpression.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseResultType/FailParseResult.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <iostream>
#include <string>

PEG::AndExpression::AndExpression(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory) :
    Expression(std::move(valueFactory)) {
    // TODO: maybe its not a good idea to pass in value factories like that, should use the default
    const auto plain = ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>();
    not_not_expr =
        Expressions::NotExpression::instance(Expressions::NotExpression::instance(std::move(expr), plain), plain);
}

PEG::ParseResultPtr
PEG::AndExpression::parse(GrammarPtr grammar, const std::string &input, const int &position) {
    // And lets expr parse, and if it succeeds, the remainder is reset to the input
    //  if it fails, then it just returns fail

    const auto result_expr = not_not_expr->parse(grammar, input, position);

    if (!result_expr->isSuccess()) {
        // std::cout << "expr not found" << std::endl;
        return ParseResultType::FailParseResult::instance(result_expr->position);
    }

    const auto parse_tree_output = value_factory->createParseTree("And", -1, "", std::vector<ValueInterfacePtr>());
    return ParseResultType::SuccessParseResult::instance(parse_tree_output, position, input.size());
}

std::string
PEG::AndExpression::info() {
    // TODO: maybe keep a copy of original expr just for error/debug/reporting (also, Line 31 above)
    return "And expression of (" + not_not_expr->info() + ")";
}

PEG::ExpressionPtr
PEG::AndExpression::instance(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory) {
    return std::make_shared<AndExpression>(std::move(expr), std::move(valueFactory));
}

PEG::ExpressionPtr
PEG::AndExpression::instance(ExpressionPtr expr) {
    return instance(std::move(expr), ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>());
}

PEG::EXPRESSION_TYPE
PEG::AndExpression::get_expr_type() {
    return PEG_AND;
}

std::string
PEG::AndExpression::print() {
    const auto not_not_expr_as_not = std::dynamic_pointer_cast<Expressions::NotExpression>(not_not_expr);
    const auto not_expr_as_not = std::dynamic_pointer_cast<Expressions::NotExpression>(not_not_expr_as_not->expr);
    const auto expr = std::dynamic_pointer_cast<Expression>(not_expr_as_not->expr);
    return "&(" + expr->print() + ") ";
}

std::vector<std::string>
PEG::AndExpression::get_non_terms() {
    return not_not_expr->get_non_terms();
}