// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_SeqNExpression_H
#define PEG_SeqNExpression_H

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <string>
#include <vector>

namespace PEG {
namespace Expressions {

/// @brief SeqN Expression: Like Sequence Expression but a list of Expression instead of Two.
///
/// i.e. given an ordered list of expression, starting with the first expression, try parsing the input with the
/// expression, and if it succeeds, try parse the remainder of the input with the next expression.
///
/// SeqN parse only succeeds if all its constituent expressions consecutively successfully parse the remainder input.
class SeqNExpression final: public Expression {

public:
    ParseResultPtr parse(GrammarPtr grammar, const std::string &input, const int &position) override;

    std::vector<ExpressionPtr> seq_n_expressions;

    SeqNExpression(const std::vector<ExpressionPtr> &expressions, ValueFactoryInterfacePtr valueFactory) :
        Expression(std::move(valueFactory)), seq_n_expressions((expressions)) {}

    static ExpressionPtr instance(const std::vector<ExpressionPtr> &expressions, ValueFactoryInterfacePtr valueFactory);

    static ExpressionPtr instance(const std::vector<ExpressionPtr> &expressions);

    ~SeqNExpression() override = default;

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