// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_DotExpression_H
#define PEG_DotExpression_H

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <string>

namespace PEG {

namespace Expressions {

/// Dot Expression - parse a single character as long as its valid.
class DotExpression final: public Expression {

public:
    ParseResultPtr parse(GrammarPtr grammar, const std::string &input, const int &position) override;

    explicit DotExpression(ValueFactoryInterfacePtr valueFactory) : Expression(std::move(valueFactory)) {}

    static ExpressionPtr instance(ValueFactoryInterfacePtr valueFactory);

    static ExpressionPtr instance();

    ~DotExpression() override = default;

    std::string info() override;

    EXPRESSION_TYPE get_expr_type() override;

    std::string print() override;

    std::vector<std::string> get_non_terms() override;

    bool isNullable(const std::set<std::string> &nullable_non_terminals) override { return false; }

    std::set<std::string> find_first(
        const std::unordered_map<std::string, std::set<std::string>> &non_terminal_map_first,
        const std::set<std::string> &nullable_non_terminals
    ) override {
        return {};
    }
};
} // namespace Expressions

} // namespace PEG

#endif