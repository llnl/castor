// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expression.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResultType/FailParseResult.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <PEG/Value/NothingFactoryInterface.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <valarray>
#include <vector>

namespace PEG {

Grammar::MapKV Grammar::L_table;

Grammar::Grammar(
    const std::string &gram_start, const std::unordered_map<std::string, std::vector<ExpressionPtr>> &gram_rules
) {
    gram.start = gram_start;
    gram.rules = gram_rules;

    const auto valueFactory = NothingFactory::instance<NothingFactory>();

    const std::string spacing = "_spacing_";

    gram.rules.insert(std::make_pair(
        white_space, std::vector<ExpressionPtr>(
                         {Expressions::TerminalExpression::instance(" ", valueFactory),
                          Expressions::TerminalExpression::instance("\t", valueFactory),
                          Expressions::TerminalExpression::instance("\r\n", valueFactory),
                          Expressions::TerminalExpression::instance("\n", valueFactory),
                          Expressions::TerminalExpression::instance("\r", valueFactory)}
                     )
    ));

    nullable_non_terminals = find_all_nullable();
    left_recursive_non_terminals = find_all_left_recursion();

    if (left_recursive_non_terminals.empty()) {
        doLeftRecursion = false;
    }
}

int
Grammar::parse_whitespace(const std::string &input, const int &position_before_space) {
    // TODO: refactor into a proper RepeatExpression based parsing
    auto position = position_before_space;
    const auto ws_rules = gram.rules.find(white_space)->second;
    int new_position = position;
    for (const auto &ws_rule : ws_rules) {
        const auto ws_result = ws_rule->parse(shared_from_this(), input, position);
        if (ws_result->isSuccess()) {
            new_position = ws_result->position;
        }
    }
    while (new_position > position) { // is fixpoint reached?
        position = new_position;
        for (const auto &ws_rule : ws_rules) {
            const auto ws_result = ws_rule->parse(shared_from_this(), input, position);
            if (ws_result->isSuccess()) {
                new_position = ws_result->position;
            }
        }
    }
    return new_position;
}

ParseResultPtr
Grammar::parse_from_non_terminal(
    const std::string &non_terminal, const int &precedence_level, const std::string &input, const int &position,
    ValueFactoryInterfacePtr value_factory
) {
    if (!doLeftRecursion) {
        return parse_from_non_terminal(false, non_terminal, precedence_level, input, position, value_factory);
    }

    const auto isLeftRecursive = left_recursive_non_terminals.find(non_terminal) != left_recursive_non_terminals.end();
    return parse_from_non_terminal(isLeftRecursive, non_terminal, precedence_level, input, position, value_factory);
}

ParseResultPtr
Grammar::parse_from_non_terminal(
    const bool &isLeftRecursive, const std::string &non_terminal, const int &precedence_level, const std::string &input,
    const int &position, const ValueFactoryInterfacePtr value_factory
) {
    // sanity checks:
    // check if grammar is non-empty
    if (gram.rules.empty()) {
        return ParseResultType::FailParseResult::instance(position);
    }
    // check if non-terminal non-empty rules in grammar
    if (gram.rules.find(non_terminal) == gram.rules.end()) {
        return ParseResultType::FailParseResult::instance(position);
    }

    if (isLeftRecursive) {
        const auto in_table = L_table.find({non_terminal, position});
        if (in_table != L_table.end()) {
            // only applies if inside a left recursion
            const auto memo_key = in_table->first;
            const auto memo_value = in_table->second;

            auto output = memo_value.first;
            auto const precedence = memo_value.second;

            if (!output->isSuccess()) {
                return output;
            }

            if (precedence_level < precedence) {
                return ParseResultType::FailParseResult::instance(position);
            }

            return output;
        }
    }

    // const int position = parse_whitespace(input, position_before_space);

    const std::vector<ExpressionPtr> &literal_rules = gram.rules.find(non_terminal)->second;
    for (size_t rule_no = 0; rule_no < literal_rules.size(); rule_no++) {
        // TODO: make distance calc work
        // int rule_no = std::distance<std::vector<ExpressionPtr>::iterator>(expr_rules.begin(), iter)

        // handling left-recursion, start off with marking our current non_terminal as failed result in memo table
        if (isLeftRecursive) {
            L_table[{non_terminal, position}] = {ParseResultType::FailParseResult::instance(position), 0};
        }

        const ExpressionPtr &iter = literal_rules[rule_no];
        const auto result = iter->parse(shared_from_this(), input, position);
        if (result->isSuccess()) {

            const int new_position = result->position;

            const ValueInterfacePtr parseResultTreeNonTerminal =
                value_factory->createParseTree("non_terminal", rule_no, non_terminal, {result->valueOutput});

            auto output = ParseResultType::SuccessParseResult::instance(
                parseResultTreeNonTerminal,
                // parse_whitespace(input, new_position),
                new_position, input.size()
            );

            if (!isLeftRecursive) {
                return output;
            }

            // return output;
            // handling left recursion, don't return the result right away
            // do a separate INC function that:
            //      checks if you can proceed one more step with updated memo table value
            //      if no, then just return the current value
            //      if yes,
            //          confirm that the value is indeed updated (position changed, not reaching fixed point)?
            //          then recurse on INC with the updated memo table value
            L_table[{non_terminal, position}] = {output, precedence_level};
            const auto next_position = output->position;
            const auto new_output =
                inc(output, non_terminal, precedence_level, input, position, value_factory, next_position);
            L_table.erase({non_terminal, position});
            return new_output;
        }

        if (isLeftRecursive) {
            L_table.erase({non_terminal, position});
        }
    }

    if (isLeftRecursive) {
        L_table.erase({non_terminal, position});
    }

    return ParseResultType::FailParseResult::instance(position);
}

ParseResultPtr
Grammar::inc(
    ParseResultPtr current_output, const std::string &non_terminal, const int &precedence_level,
    const std::string &input, const int &position, const ValueFactoryInterfacePtr value_factory,
    const int &next_position
) {
    // S. Medeiros et al. / Science of Computer Programming 96 (2014) 177
    // L_table[{non_terminal, position}] = std::move(current_output); // <- before inc starts at rule lvar1

    // const auto new_result = parse_from_non_terminal(non_terminal, input, position, value_factory);
    ParseResultPtr new_result = ParseResultType::FailParseResult::instance(next_position);

    // std::cout << "INC: trying another non-term " << non_terminal << std::endl;
    const std::vector<ExpressionPtr> &literal_rules = gram.rules.find(non_terminal)->second;
    for (size_t rule_no = 0; rule_no < literal_rules.size(); rule_no++) {
        const ExpressionPtr &iter = literal_rules[rule_no];
        const auto result = iter->parse(shared_from_this(), input, position);
        if (result->isSuccess()) {
            // std::cout << "INC parsed one more left-recursive " << non_terminal << std::endl;

            const int new_position = result->position;

            const ValueInterfacePtr parseResultTreeNonTerminal =
                value_factory->createParseTree("non_terminal", rule_no, non_terminal, {result->valueOutput});

            const auto output = ParseResultType::SuccessParseResult::instance(
                parseResultTreeNonTerminal,
                // parse_whitespace(input, new_position),
                new_position, input.size()
            );

            new_result = output;
            break; // since we're still a PEG
        }
    }

    if (!new_result->isSuccess()) {
        // rule inc.2
        // always available since it was set by L139 before reaching here
        return L_table[{non_terminal, position}].first;
    }

    if (new_result->position <= next_position) {
        // rule inc.3
        // < means it had backward progress
        // = means it had empty progress
        // again, value in the unordered_map is always set by L139
        return L_table[{non_terminal, position}].first;
    }

    // rule inc.1
    // one additional parse was possible, and its _successful_ AND has _forward-progress_
    const int new_position = new_result->position;
    L_table[{non_terminal, position}] = {new_result, precedence_level};
    const auto new_output =
        inc(new_result, non_terminal, precedence_level, input, position, value_factory, new_position);
    L_table[{non_terminal, position}] = {std::move(current_output), precedence_level};
    return new_output;
}

GrammarPtr
Grammar::instance(
    const std::string &gram_start, std::unordered_map<std::string, std::vector<ExpressionPtr>> gram_rules
) {
    return std::shared_ptr<Grammar>(new Grammar(gram_start, gram_rules));
}

void
Grammar::print() const {
    std::cout << std::endl << "Grammar " << std::endl;
    std::cout << "\t" << " start - " << gram.start << std::endl;
    std::cout << "\t" << " rules - " << std::endl;
    for (const auto &iter : gram.rules) {
        if (iter.first == white_space) {
            continue;
        }
        std::string out = iter.first;
        std::transform(out.begin(), out.end(), out.begin(), ::toupper);
        std::cout << "\t\t" << out;
        std::cout << " <- ";
        int i = 0;
        for (const auto &rule : iter.second) {
            std::cout << rule->print();
            std::cout << " / (rule-" << i << ")" << std::endl;
            std::cout << "\t\t\t";
            i++;
        }
        std::cout << std::endl;
    }
}

bool
Grammar::check_grammar() const {
    // check if all non-terminals have rules

    std::vector<std::string> lhs;
    std::set<std::string> rhs;
    for (const auto &rule_pair : gram.rules) {
        if (rule_pair.first == white_space) {
            continue;
        }
        const auto non_terminal_string = rule_pair.first;
        lhs.push_back(non_terminal_string);
        const auto rules = rule_pair.second;
        for (const auto &rule : rules) {
            auto rule_out = rule->get_non_terms();
            for (const auto &iter : rule_out) {
                rhs.insert(iter);
            }
        }
    }

    if (lhs.size() != rhs.size()) {
        if (lhs.size() == 1 && rhs.size() == 0) {
            // only one production
            return true;
        }
        if (lhs.size() == rhs.size() + 1) {
            // all non-terminals + one start terminal
            // TODO: but is it actually the start terminal tho
            return true;
        }
        return false;
    }

    return true;
}

std::unordered_map<std::string, std::set<std::string>>
Grammar::find_all_non_terminal_first(
    const std::unordered_map<std::string, std::set<std::string>> &current_map,
    const std::set<std::string> &nullable_non_terminals
) const {
    std::unordered_map<std::string, std::set<std::string>> result;

    const auto rules = gram.rules;
    for (const auto &[non_terminal, non_terminal_rules] : rules) {
        if (non_terminal == white_space) {
            continue;
        }
        std::set<std::string> non_terminal_result;
        for (const auto &rule : non_terminal_rules) {
            const auto rule_result = rule->find_first(current_map, nullable_non_terminals);
            for (const auto &iter_rule_result : rule_result) {
                non_terminal_result.insert(iter_rule_result);
            }
        }
        result.insert({non_terminal, non_terminal_result});
    }

    return result;
}

std::set<std::string>
Grammar::find_all_left_recursion() const {
    std::set<std::string> result;

    std::unordered_map<std::string, std::set<std::string>> non_terminal_map;
    int count = 0;
    for (const auto &iter : gram.rules) {
        if (iter.first == white_space) {
            continue;
        }
        non_terminal_map.insert({iter.first, std::set<std::string>()});
    }
    auto new_non_terminal_map = find_all_non_terminal_first(non_terminal_map, nullable_non_terminals);
    int new_count = 0;
    for (const auto &[fst, snd] : new_non_terminal_map) {
        new_count += snd.size();
    }
    while (new_count > count) {
        count = new_count;
        new_non_terminal_map = find_all_non_terminal_first(new_non_terminal_map, nullable_non_terminals);
        new_count = 0;
        for (const auto &[fst, snd] : new_non_terminal_map) {
            new_count += snd.size();
        }
    }

    for (const auto &[fst, snd] : new_non_terminal_map) {
        if (snd.find(fst) != snd.end()) {
            result.insert(fst);
        }
    }

    return result;
}

std::set<std::string>
Grammar::find_all_nullable() {
    auto count = 0;
    std::set<std::string> current_result;
    current_result = find_nullable_non_terminals(current_result);

    auto new_count = current_result.size();
    while (new_count > count) {
        count = new_count;
        current_result = find_nullable_non_terminals(current_result);
        new_count = current_result.size();
    }

    return current_result;
}

std::set<std::string>
Grammar::find_nullable_non_terminals(const std::set<std::string> &current_result) {
    std::set<std::string> new_result;
    for (const auto &[non_term_literal, snd] : gram.rules) {
        auto result = false;
        for (const auto &iter : snd) {
            result = result || iter->isNullable(current_result);
        }
        if (result) {
            new_result.insert(non_term_literal);
        }
    }
    return new_result;
}

} // end namespace PEG