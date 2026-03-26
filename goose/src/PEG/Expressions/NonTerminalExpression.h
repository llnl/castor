// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_NonTerminalExpression_H
#define PEG_NonTerminalExpression_H

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <algorithm>
#include <string>

namespace PEG {

/// Namespace that contains all the different kinds of expression that are implemented as subclasses to the Expression.
namespace Expressions {

/// Non-Terminal Expression: defined by production rules that give alternations to try while attempting to parse inputs.
class NonTerminalExpression final: public Expression {

public:
    ParseResultPtr parse(GrammarPtr grammar, const std::string &input, const int &position) override;

    std::string literal;

    int precedence;

    NonTerminalExpression(std::string symbol, ValueFactoryInterfacePtr valueFactory, int precedence_level) :
        Expression(std::move(valueFactory)), literal(std::move(symbol)), precedence(precedence_level) {}

    static ExpressionPtr
    instance(const std::string &symbol, ValueFactoryInterfacePtr valueFactory, int precedence_level);

    static ExpressionPtr instance(const std::string &symbol, ValueFactoryInterfacePtr valueFactory);

    static ExpressionPtr instance(const std::string &symbol);

    static ExpressionPtr instance(const std::string &symbol, const int &precedence_level);

    ~NonTerminalExpression() override = default;

    std::string info() override;

    EXPRESSION_TYPE get_expr_type() override;

    std::string print() override;

    std::vector<std::string> get_non_terms() override;

    bool isNullable(const std::set<std::string> &nullable_non_terminals) override;

    std::set<std::string> find_first(
        const std::unordered_map<std::string, std::set<std::string>> &non_terminal_map_first,
        const std::set<std::string> &nullable_non_terminals
    ) override;
};
} // namespace Expressions

} // namespace PEG

#endif