// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expressions/EmptyExpression.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <iostream>
#include <memory>
#include <string>

#define UNUSED(expr)                                                                                                   \
    do {                                                                                                               \
        (void)(expr);                                                                                                  \
    } while (0)

PEG::ParseResultPtr
PEG::Expressions::EmptyExpression::parse(GrammarPtr grammar, const std::string &input, const int &position) {
    UNUSED(grammar);
    // Empty parse nothing
    // except it'll still succeed by just returning the input as the remainder
    // TODO: re think this

    const auto parse_tree_output = value_factory->createParseTree("Empty", -1, "", std::vector<ValueInterfacePtr>());
    return ParseResultType::SuccessParseResult::instance(parse_tree_output, position, input.size());
}

std::string
PEG::Expressions::EmptyExpression::info() {
    return "Empty expression";
}

PEG::ExpressionPtr
PEG::Expressions::EmptyExpression::instance(ValueFactoryInterfacePtr valueFactory) {
    return std::make_shared<EmptyExpression>(std::move(valueFactory));
}

PEG::ExpressionPtr
PEG::Expressions::EmptyExpression::instance() {
    return instance(ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>());
}

PEG::EXPRESSION_TYPE
PEG::Expressions::EmptyExpression::get_expr_type() {
    return PEG_EMPTY;
}

std::string
PEG::Expressions::EmptyExpression::print() {
    return "ε";
}

std::vector<std::string>
PEG::Expressions::EmptyExpression::get_non_terms() {
    return {};
}