// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_NotExpression_H
#define PEG_NotExpression_H

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <string>

namespace PEG {

namespace Expressions {

/// @brief Not Expression - Parse the negation of an expression
///
/// i.e. If the expression succeeds, then the result is a fail. However, if the expression does not succeed, then the
/// parse succeeds and resets position to before the parse.
class NotExpression final: public Expression {

public:
    ParseResultPtr parse(GrammarPtr grammar, const std::string &input, const int &position) override;

    ExpressionPtr expr;

    NotExpression(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory) :
        Expression(std::move(valueFactory)), expr(std::move(expr)) {}

    static ExpressionPtr instance(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory);

    static ExpressionPtr instance(ExpressionPtr expr);

    ~NotExpression() override = default;

    std::string info() override;

    EXPRESSION_TYPE get_expr_type() override;

    std::string print() override;

    std::vector<std::string> get_non_terms() override;

    bool isNullable(const std::set<std::string> &nullable_non_terminals) override { return true; }

    std::set<std::string> find_first(
        const std::unordered_map<std::string, std::set<std::string>> &non_terminal_map_first,
        const std::set<std::string> &nullable_non_terminals
    ) override {
        return expr->find_first(non_terminal_map_first, nullable_non_terminals);
    }
};
} // namespace Expressions
} // namespace PEG

#endif