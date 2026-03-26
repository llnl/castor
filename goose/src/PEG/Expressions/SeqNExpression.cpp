// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <PEG/Expressions/SeqNExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseResultType/FailParseResult.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <iostream>
#include <string>

PEG::ParseResultPtr
PEG::Expressions::SeqNExpression::parse(GrammarPtr grammar, const std::string &input, const int &position) {
    // input -> expr1 -> fail/success(remainder)
    // - if fail stop
    // success.remainder -> expr2 -> fail/success
    // - if fail stop
    // take both parse trees, and merge it into one

    if (seq_n_expressions.empty()) {
        return ParseResultType::FailParseResult::instance(position);
    }

    int new_position = position;
    auto seq_n_result_p_tree = std::vector<ValueInterfacePtr>();
    for (const auto &iter : seq_n_expressions) {
        new_position = grammar->parse_whitespace(input, new_position);
        const auto parse_temp_result = iter->parse(grammar, input, new_position);
        if (!(parse_temp_result->isSuccess())) {
            return ParseResultType::FailParseResult::instance(parse_temp_result->position);
        }
        new_position = parse_temp_result->position;
        seq_n_result_p_tree.push_back(parse_temp_result->valueOutput);
    }

    const ValueInterfacePtr parseTreeSeqN = value_factory->createParseTree("SeqN", -1, "", seq_n_result_p_tree);
    return ParseResultType::SuccessParseResult::instance(parseTreeSeqN, new_position, input.size());
}

std::string
PEG::Expressions::SeqNExpression::info() {
    std::string n_sequence_string;
    for (const auto &iter : seq_n_expressions) {
        n_sequence_string += iter->info();
        n_sequence_string += ", ";
    }
    return "n-sequence expression of (" + n_sequence_string + ")";
}

PEG::ExpressionPtr
PEG::Expressions::SeqNExpression::instance(
    const std::vector<ExpressionPtr> &expressions, ValueFactoryInterfacePtr valueFactory
) {
    return std::make_shared<SeqNExpression>(expressions, std::move(valueFactory));
}

PEG::ExpressionPtr
PEG::Expressions::SeqNExpression::instance(const std::vector<ExpressionPtr> &expressions) {
    return instance(expressions, ValueFactoryInterface::instance<ParseTree::PlainParseTreeFactory>());
}

PEG::EXPRESSION_TYPE
PEG::Expressions::SeqNExpression::get_expr_type() {
    return PEG_SEQ_N;
}

std::string
PEG::Expressions::SeqNExpression::print() {
    std::string out;
    for (const auto &iter : seq_n_expressions) {
        out += iter->print();
        out += " ";
    }
    return out;
}

std::vector<std::string>
PEG::Expressions::SeqNExpression::get_non_terms() {
    std::vector<std::string> out;
    for (const auto &iter : seq_n_expressions) {
        auto iter_non_terms = iter->get_non_terms();
        out.insert(out.end(), iter_non_terms.begin(), iter_non_terms.end());
    }
    return out;
}

bool
PEG::Expressions::SeqNExpression::isNullable(const std::set<std::string> &nullable_non_terminals) {
    bool out = true;
    for (const auto &iter : seq_n_expressions) {
        out = out && iter->isNullable(nullable_non_terminals);
    }
    return out;
}

std::set<std::string>
PEG::Expressions::SeqNExpression::find_first(
    const std::unordered_map<std::string, std::set<std::string>> &non_terminal_map_first,
    const std::set<std::string> &nullable_non_terminals
) {
    if (seq_n_expressions.empty()) {
        return {};
    }
    std::set<std::string> result = seq_n_expressions[0]->find_first(non_terminal_map_first, nullable_non_terminals);
    auto stillNullable = seq_n_expressions[0]->isNullable(nullable_non_terminals);
    if (!stillNullable) {
        return result;
    }
    for (int i = 1; i < seq_n_expressions.size(); ++i) {
        const auto next_result = seq_n_expressions[i]->find_first(non_terminal_map_first, nullable_non_terminals);
        for (const auto &element : next_result) {
            result.insert(element);
        }
        stillNullable = seq_n_expressions[i]->isNullable(nullable_non_terminals);
        if (!stillNullable) {
            break;
        }
    }
    return result;
}
