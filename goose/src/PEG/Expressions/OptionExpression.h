// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_OptionExpression_H
#define PEG_OptionExpression_H

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <string>

namespace PEG {
namespace Expressions {

/// @brief Option Expression - try to parse an expression zero or one time
///
/// i.e. a successful parse that is either empty or makes forward progress.
class OptionExpression final: public Expression {

public:
    ParseResultPtr parse(GrammarPtr grammar, const std::string &input, const int &position) override;

    ExpressionPtr option_expr;

    OptionExpression(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory) :
        Expression(std::move(valueFactory)), option_expr{std::move(expr)} {}

    static ExpressionPtr instance(ExpressionPtr expr, ValueFactoryInterfacePtr valueFactory);

    static ExpressionPtr instance(ExpressionPtr expr);

    ~OptionExpression() override = default;

    std::string info() override;

    EXPRESSION_TYPE get_expr_type() override;

    std::string print() override;

    std::vector<std::string> get_non_terms() override;

    bool isNullable(const std::set<std::string> &nullable_non_terminals) override { return true; }

    std::set<std::string> find_first(
        const std::unordered_map<std::string, std::set<std::string>> &non_terminal_map_first,
        const std::set<std::string> &nullable_non_terminals
    ) override {
        return option_expr->find_first(non_terminal_map_first, nullable_non_terminals);
    }
};

using OptionalExpression = OptionExpression;

} // namespace Expressions
} // namespace PEG

#endif