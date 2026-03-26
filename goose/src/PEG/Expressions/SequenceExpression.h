// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_SequenceExpression_H
#define PEG_SequenceExpression_H

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <string>

namespace PEG {
namespace Expressions {

/// Sequence Expression: parse two expressions consecutively - try the first one and then if it succeeds, parse the
/// second.
class SequenceExpression final: public Expression {

public:
    ParseResultPtr parse(GrammarPtr grammar, const std::string &input, const int &position) override;

    ExpressionPtr expr_first;
    ExpressionPtr expr_second;

    SequenceExpression(ExpressionPtr expr_1, ExpressionPtr expr_2, ValueFactoryInterfacePtr valueFactory) :
        Expression(std::move(valueFactory)), expr_first(std::move(expr_1)), expr_second(std::move(expr_2)) {}

    static ExpressionPtr instance(ExpressionPtr expr_1, ExpressionPtr expr_2, ValueFactoryInterfacePtr valueFactory);

    static ExpressionPtr instance(ExpressionPtr expr_1, ExpressionPtr expr_2);

    ~SequenceExpression() override = default;

    std::string info() override;

    EXPRESSION_TYPE get_expr_type() override;

    std::string print() override;

    std::vector<std::string> get_non_terms() override;

    bool isNullable(const std::set<std::string> &nullable_non_terminals) override {
        return expr_first->isNullable(nullable_non_terminals) && expr_second->isNullable(nullable_non_terminals);
    }

    std::set<std::string> find_first(
        const std::unordered_map<std::string, std::set<std::string>> &non_terminal_map_first,
        const std::set<std::string> &nullable_non_terminals
    ) override;
};
} // namespace Expressions
} // namespace PEG

#endif