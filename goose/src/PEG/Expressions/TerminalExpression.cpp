// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/ParseResultType/FailParseResult.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <iostream>
#include <string>

#define UNUSED(expr)                                                                                                   \
    do {                                                                                                               \
        (void)expr;                                                                                                    \
    } while (0)

namespace PEG {

ParseResultPtr
Expressions::TerminalExpression::parse(GrammarPtr grammar, const std::string &input, const int &position) {
    UNUSED(grammar); // https://stackoverflow.com/a/1486931

    if (std::equal(terminal.begin(), terminal.end(), input.begin() + position)) {
        // input starts with terminal
        const ValueInterfacePtr parseResultTree =
            value_factory->createParseTree("terminal", -1, terminal, std::vector<ValueInterfacePtr>());
        const int new_position = position + terminal.size();
        return ParseResultType::SuccessParseResult::instance(parseResultTree, new_position, input.size());
    }

    return ParseResultType::FailParseResult::instance(position);
}

std::string
Expressions::TerminalExpression::info() {
    return "terminal expression (" + terminal + ")";
}

ExpressionPtr
Expressions::TerminalExpression::instance(const std::string &input, ValueFactoryInterfacePtr valueFactory) {
    return std::make_shared<TerminalExpression>(input, std::move(valueFactory));
}

ExpressionPtr
Expressions::TerminalExpression::instance(const std::string &input) {
    return instance(input, ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>());
}

EXPRESSION_TYPE
Expressions::TerminalExpression::get_expr_type() {
    return PEG_TERMINAL;
}

std::string
Expressions::TerminalExpression::print() {
    return "'" + terminal + "'";
}

std::vector<std::string>
Expressions::TerminalExpression::get_non_terms() {
    return {};
}

} // namespace PEG