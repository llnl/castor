// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expressions/DotExpression.h>
#include <PEG/ParseResultType/FailParseResult.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <iostream>
#include <string>

#define UNUSED(expr)                                                                                                   \
    do {                                                                                                               \
        (void)(expr);                                                                                                  \
    } while (0)

PEG::ParseResultPtr
PEG::Expressions::DotExpression::parse(GrammarPtr grammar, const std::string &input, const int &position) {
    UNUSED(grammar);

    // Dot parse a single terminal symbol
    // TODO: Dot currently just parses the first character and returns the rest
    // maybe it might make more sense to just parse a single terminal
    // although tbh that's the same thing...

    if (input.empty() || position < 0) {
        return ParseResultType::FailParseResult::instance(position);
    }

    const int size = input.size();
    if (position >= size) {
        return ParseResultType::FailParseResult::instance(position);
    }

    const auto parse_tree_output =
        value_factory->createParseTree("dot", -1, std::string(1, input[position]), std::vector<ValueInterfacePtr>());
    return ParseResultType::SuccessParseResult::instance(parse_tree_output, position + 1, input.size());
}

std::string
PEG::Expressions::DotExpression::info() {
    return "Dot expression";
}

PEG::ExpressionPtr
PEG::Expressions::DotExpression::instance(ValueFactoryInterfacePtr valueFactory) {
    return std::make_shared<DotExpression>(std::move(valueFactory));
}

PEG::ExpressionPtr
PEG::Expressions::DotExpression::instance() {
    return instance(ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>());
}

PEG::EXPRESSION_TYPE
PEG::Expressions::DotExpression::get_expr_type() {
    return PEG_DOT;
}

std::string
PEG::Expressions::DotExpression::print() {
    return ".";
}

std::vector<std::string>
PEG::Expressions::DotExpression::get_non_terms() {
    return {};
}