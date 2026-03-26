// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expressions/NonTerminalExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <cassert>
#include <iostream>
#include <string>

namespace PEG {
PEG::ParseResultPtr
PEG::Expressions::NonTerminalExpression::parse(GrammarPtr grammar, const std::string &input, const int &position) {
    const int next_position = grammar->parse_whitespace(input, position);
    return grammar->parse_from_non_terminal(literal, precedence, input, next_position, value_factory);
}

PEG::ExpressionPtr
PEG::Expressions::NonTerminalExpression::instance(
    const std::string &symbol, ValueFactoryInterfacePtr valueFactory, int precedence_level
) {
    return std::make_shared<NonTerminalExpression>(symbol, std::move(valueFactory), precedence_level);
}

PEG::ExpressionPtr
PEG::Expressions::NonTerminalExpression::instance(const std::string &input, ValueFactoryInterfacePtr valueFactory) {
    return std::make_shared<NonTerminalExpression>(input, std::move(valueFactory), 1);
}

PEG::ExpressionPtr
PEG::Expressions::NonTerminalExpression::instance(const std::string &input) {
    return instance(input, ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>(), 1);
}

PEG::ExpressionPtr
PEG::Expressions::NonTerminalExpression::instance(const std::string &symbol, const int &precedence_level) {
    return instance(symbol, ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>(), precedence_level);
}

std::string
PEG::Expressions::NonTerminalExpression::info() {
    return "non terminal expression (" + literal + ")";
}

PEG::EXPRESSION_TYPE
PEG::Expressions::NonTerminalExpression::get_expr_type() {
    return PEG_NON_TERMINAL;
}

std::string
PEG::Expressions::NonTerminalExpression::print() {
    std::string out = literal;
    // std::transform(out.begin(), out.end(), out.begin(), ::toupper);
    if (precedence > 1) {
        out += "#" + std::to_string(precedence);
    }
    return out;
}

std::vector<std::string>
PEG::Expressions::NonTerminalExpression::get_non_terms() {
    return {literal};
}

bool
PEG::Expressions::NonTerminalExpression::isNullable(const std::set<std::string> &nullable_non_terminals) {
    return nullable_non_terminals.find(literal) == nullable_non_terminals.end();
}

std::set<std::string>
PEG::Expressions::NonTerminalExpression::find_first(
    const std::unordered_map<std::string, std::set<std::string>> &non_terminal_map_first,
    const std::set<std::string> &nullable_non_terminals
) {
    const auto result_value = non_terminal_map_first.find(literal);
    if (result_value == non_terminal_map_first.end()) {
        std::cout << "non-terminal " << literal << " does NOT have production rules!!" << std::endl;
        assert(result_value == non_terminal_map_first.end());
    }
    auto result = result_value->second;
    if (result.find(literal) == result.end()) {
        result.insert(literal);
    }
    return result;
}

} // namespace PEG