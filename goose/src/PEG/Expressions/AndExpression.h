// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_AndExpression_H
#define PEG_AndExpression_H

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <string>

namespace PEG {

/// @brief And Expression, when you want to test if an Expression is satisfied at a position. If yes, then you reset
/// back to the position. If not, then it fails.
class AndExpression final: public Expression {

public:
    ParseResultPtr parse(GrammarPtr grammar, const std::string &input, const int &position) override;

    ExpressionPtr not_not_expr;

    AndExpression(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory);

    static ExpressionPtr instance(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory);

    static ExpressionPtr instance(ExpressionPtr expr);

    ~AndExpression() override = default;

    std::string info() override;

    EXPRESSION_TYPE get_expr_type() override;

    std::string print() override;

    std::vector<std::string> get_non_terms() override;

    bool isNullable(const std::set<std::string> &nullable_non_terminals) override { return true; }

    std::set<std::string> find_first(
        const std::unordered_map<std::string, std::set<std::string>> &non_terminal_map_first,
        const std::set<std::string> &nullable_non_terminals
    ) override {
        return not_not_expr->find_first(non_terminal_map_first, nullable_non_terminals);
    }
};

} // namespace PEG

#endif