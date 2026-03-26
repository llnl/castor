// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_RepeatExpression_H
#define PEG_RepeatExpression_H

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <string>

namespace PEG {

namespace Expressions {

/// Repeat Expression - parse an Expression zero or more times.
class RepeatExpression final: public Expression {

public:
    ParseResultPtr parse(GrammarPtr grammar, const std::string &input, const int &position) override;

    ExpressionPtr expr;

    RepeatExpression(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory) :
        Expression(std::move(valueFactory)), expr(std::move(expr)) {}

    static ExpressionPtr instance(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory);

    static ExpressionPtr instance(ExpressionPtr expr);

    ~RepeatExpression() override = default;

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