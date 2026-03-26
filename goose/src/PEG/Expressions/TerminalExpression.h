// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_TerminalExpression_H
#define PEG_TerminalExpression_H

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <string>

namespace PEG {
namespace Expressions {

/// Terminal Expression - succeeds if a prefix of the given input matches the string of characters as is.
class TerminalExpression final: public Expression {

public:
    ParseResultPtr parse(GrammarPtr grammar, const std::string &input, const int &position) override;

    std::string terminal;

    explicit TerminalExpression(std::string input, ValueFactoryInterfacePtr valueFactory) :
        Expression(std::move(valueFactory)), terminal{std::move(input)} {}

    static ExpressionPtr instance(const std::string &input, ValueFactoryInterfacePtr valueFactory);

    static ExpressionPtr instance(const std::string &input);

    ~TerminalExpression() override = default;

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