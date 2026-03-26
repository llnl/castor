// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Bootstrap/BootstrapParser.h>
#include <PEG/Expressions/AndExpression.h>
#include <PEG/Expressions/DotExpression.h>
#include <PEG/Expressions/EmptyExpression.h>
#include <PEG/Expressions/NonTerminalExpression.h>
#include <PEG/Expressions/NotExpression.h>
#include <PEG/Expressions/OptionExpression.h>
#include <PEG/Expressions/PlusExpression.h>
#include <PEG/Expressions/RepeatExpression.h>
#include <PEG/Expressions/SeqNExpression.h>
#include <PEG/Expressions/SequenceExpression.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Value/NothingFactoryInterface.h>
#include <PEG/Value/ValueInterface.h>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

namespace PEG {

std::unordered_map<std::string, ValueFactoryInterfacePtr> Bootstrap::BootstrapParser::action_map;

class RegexValue final: public ValueInterface {
public:
    char letter;

    explicit RegexValue(const char &letter) : letter(letter) {}

    ~RegexValue() override = default;
};

class RegexTerminalFactory final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        return std::make_shared<RegexValue>(value[0]);
    }
};

class RegexNonTerminalFactory final: public ValueFactoryInterface {
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // assert(children.size() == 1);
        return ValueInterface::downcast<RegexValue>(children[0]);
    }
};

class NonTerminalLift final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        return children[0];
    }
};

class TerminalFromRegexFactory final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // assert(label == "repeat"); position, value are unused
        if (label == "Empty") {
            return ParseTree::PlainParseTreeFactory().createParseTree(label, position, value, children);
        }
        std::string out;
        for (const auto &iter : children) {
            out += ValueInterface::downcast<RegexValue>(iter)->letter;
        }
        return ParseTree::PlainParseTreeFactory().createParseTree("regex", -1, out, std::vector<ValueInterfacePtr>());
    }
};

class StringFromRegexFactory final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // assert(label == "repeat"); position, value are unused
        if (label == "Empty") {
            return ParseTree::PlainParseTreeFactory().createParseTree(label, position, value, children);
        }
        std::string out;
        for (const auto &iter : children) {
            out += ValueInterface::downcast<ParseTree::PlainParseTree>(iter)->value;
        }
        return ParseTree::PlainParseTreeFactory().createParseTree(
            "identifier", -1, out, std::vector<ValueInterfacePtr>()
        );
    }
};

class SpaceInSequenceFactory: public ValueFactoryInterface {
public:
    int spacingIndex;

    explicit SpaceInSequenceFactory(const int &spacingIndex) : spacingIndex(spacingIndex) {}

    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        auto new_children = std::vector<ValueInterfacePtr>(children.size() - 1);
        std::copy_n(children.begin(), spacingIndex, new_children.begin());
        std::copy(children.begin() + spacingIndex + 1, children.end(), new_children.begin());
        return ParseTree::PlainParseTreeFactory().createParseTree(label, position, value, new_children);
    }
};

class TerminalFromSeqN final: public SpaceInSequenceFactory {
public:
    explicit TerminalFromSeqN(const int &spacingIndex) : SpaceInSequenceFactory(spacingIndex) {}

    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // std::cout << "Spacing is at Index: " << spacingIndex << std::endl;
        std::string out;
        for (int i = 0; i < children.size(); i++) {
            if (i == spacingIndex) {
                continue;
            }
            const auto &iter = children[i];
            out += ValueInterface::downcast<ParseTree::PlainParseTree>(iter)->value;
        }
        return ParseTree::PlainParseTreeFactory().createParseTree(
            "identifier", -1, out, std::vector<ValueInterfacePtr>()
        );
    }
};

// [0-9]
ExpressionPtr
peg_sequence_from_digits(
    std::unordered_map<std::string, std::vector<ExpressionPtr>> &rules, const int &start, const int &end
) {
    const auto value_factory_interface_ptr = ValueFactoryInterface::instance<RegexTerminalFactory>();
    // peg_sequence_from_digits(0, 2) {
    if (end < start) {
        return Expressions::EmptyExpression::instance();
    }
    std::string digit = std::string("Digit_") + std::to_string(start) + "_" + std::to_string(end);
    auto w = std::vector<ExpressionPtr>({});
    for (int i = start; i <= end; ++i) {
        w.push_back(Expressions::TerminalExpression::instance(std::to_string(i), value_factory_interface_ptr));
    }

    rules.insert(std::make_pair(digit, w));

    const auto trampoline_factory = ValueFactoryInterface::instance<RegexNonTerminalFactory>();

    return Expressions::NonTerminalExpression::instance(digit, trampoline_factory);
}

// [A-Z], [a-z]
ExpressionPtr
peg_non_terminal_from_alphabets(
    std::unordered_map<std::string, std::vector<ExpressionPtr>> &rules, const char &start, const char &end
) {
    const ValueFactoryInterfacePtr terminal_to_regex = ValueFactoryInterface::instance<RegexTerminalFactory>();
    if (end < start) {
        return Expressions::EmptyExpression::instance();
    }

    // std::to_string(start) not the same as std::string(1, start)
    std::string non_terminal_char = "ASCII_" + std::string(1, start) + "_" + end;
    auto x = std::vector<ExpressionPtr>({});
    for (char i = start; i <= end; ++i) {
        x.push_back(Expressions::TerminalExpression::instance(std::string(1, i), terminal_to_regex));
    }
    rules.insert(std::make_pair(non_terminal_char, x));

    const auto non_terminal_lift_terminal_factory = ValueFactoryInterface::instance<RegexNonTerminalFactory>();

    return Expressions::NonTerminalExpression::instance(non_terminal_char, non_terminal_lift_terminal_factory);
}

template <class SpaceChild>
ValueFactoryInterfacePtr
get_spacing_factory(int spacing_index) {
    return std::make_shared<SpaceChild>(spacing_index);
}

using SequenceLeft = NonTerminalLift;

class SequenceRight final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        return children[1];
    }
};

class CollectRepeats final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        if (label == "Empty") {
            return ParseTree::PlainParseTree::instance(label, position, value, children);
        }
        if (children.size() == 1) {
            const auto new_value = ValueInterface::downcast<ParseTree::PlainParseTree>(children[0])->value;
            return ParseTree::PlainParseTree::instance("char", position, new_value, children);
        }
        std::string out;
        for (const auto &iter : children) {
            out += ValueInterface::downcast<ParseTree::PlainParseTree>(iter)->value;
        }
        return ParseTree::PlainParseTree::instance("string", position, out, std::vector<ValueInterfacePtr>());
    }
};

using SkipNulls = SequenceRight;

class ExpressionValue final: public ParseTree::PlainParseTree {
public:
    ExpressionPtr expr;

    explicit ExpressionValue(
        ExpressionPtr expr, const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) : PlainParseTree(label, position, value, children), expr(std::move(expr)) {}
};

class ParserValueFactory final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        //
        return ParseTree::PlainParseTreeFactory().createParseTree(label, position, value, children);
    }
};

class ExpressionValueFactory: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        //
        return ParseTree::PlainParseTreeFactory().createParseTree(label, position, value, children);
    }
};

class NonTerminalFactory final: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // called on Primary <- Identifier ('^'[0-9][0-9]?)? !LEFTARROW Spacing
        //           children      0            1               2          3
        const auto &seq_left = children[0];
        const auto identifier = ValueInterface::downcast<ParseTree::PlainParseTree>(seq_left)->value;

        const auto optional_precedence = ValueInterface::downcast<ParseTree::PlainParseTree>(children[1]);

        if (optional_precedence->label != "Empty") {
            const auto precedence_parse =
                ValueInterface::downcast<ParseTree::PlainParseTree>(optional_precedence->children[0]);
            const int &precedence = std::stoi(precedence_parse->value);
            ExpressionPtr non_terminal_precedence_expr =
                Expressions::NonTerminalExpression::instance(identifier, precedence);
            return std::make_shared<ExpressionValue>(
                non_terminal_precedence_expr, "non-term+" + label, position, value, children
            );
        }

        // TODO: handle extra Spacing?

        ExpressionPtr non_terminal_expr = Expressions::NonTerminalExpression::instance(identifier);
        return std::make_shared<ExpressionValue>(non_terminal_expr, "non-term", position, value, children);
    }
};

class EmbedExprFactory final: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // Primary <- OPEN Sequence CLOSE
        const auto &seq_second = children[1];
        const auto second_tree = ValueInterface::downcast<ParseTree::PlainParseTree>(seq_second);
        const auto expr = ValueInterface::downcast<ExpressionValue>(second_tree->children[0])->expr;
        return std::make_shared<ExpressionValue>(expr, label, position, value, children);
    }
};

class LiteralFactory final: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        const auto &lift = children[0];
        const auto lift_tree = ValueInterface::downcast<ParseTree::PlainParseTree>(lift);
        const auto terminal = lift_tree->value;
        ExpressionPtr terminal_expr = Expressions::TerminalExpression::instance(terminal);
        return std::make_shared<ExpressionValue>(terminal_expr, "lit+" + label, position, value, children);
    }
};

class DotFactory final: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        ExpressionPtr dot_expr = Expressions::DotExpression::instance();
        return std::make_shared<ExpressionValue>(dot_expr, "dot+" + label, position, value, children);
    }
};

class SuffixFactory final: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // suffix can be optional
        // if no suffix, return expr as is
        // else, create a new appropriate expression
        const auto &seq_left = children[0];
        const auto &seq_right = children[1];
        ExpressionPtr expr = ValueInterface::downcast<ExpressionValue>(seq_left)->expr;
        const auto option_output = ValueInterface::downcast<ParseTree::PlainParseTree>(seq_right);
        if (option_output->label != "Option") { // "Option" or "Empty"
            return std::make_shared<ExpressionValue>(expr, label, position, value, children);
        }
        // option_output is not empty
        ExpressionPtr new_expr;
        const auto option_child = option_output->children[0];
        const auto child_tree = ValueInterface::downcast<ParseTree::PlainParseTree>(option_child);
        if (child_tree->value == "QUESTION") {
            new_expr = Expressions::OptionalExpression::instance(expr);
        } else if (child_tree->value == "STAR") {
            new_expr = Expressions::RepeatExpression::instance(expr);
        } else if (child_tree->value == "PLUS") {
            new_expr = Expressions::PlusExpression::instance(expr);
        }
        return std::make_shared<ExpressionValue>(new_expr, label, position, value, children);
    }
};

class PrefixFactory final: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // prefix can be optional
        // if no prefix, return expr as is
        // else, create a new appropriate expression
        const auto &seq_left = children[0];
        const auto &seq_right = children[1];
        ExpressionPtr expr = ValueInterface::downcast<ExpressionValue>(seq_right)->expr;
        const auto option_output = ValueInterface::downcast<ParseTree::PlainParseTree>(seq_left);
        if (option_output->label == "Empty") { // "Option" or "Empty"
            return std::make_shared<ExpressionValue>(expr, label, position, value, children);
        }
        // option_output is not empty
        ExpressionPtr new_expr;
        const auto option_child = option_output->children[0];
        const auto child_tree = ValueInterface::downcast<ParseTree::PlainParseTree>(option_child);
        if (child_tree->value == "AND") {
            new_expr = AndExpression::instance(expr);
        } else if (child_tree->value == "NOT") {
            new_expr = Expressions::NotExpression::instance(expr);
        }
        return std::make_shared<ExpressionValue>(new_expr, label, position, value, children);
    }
};

class Sequence2SeqNFactory final: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        const auto repeat_tree = ValueInterface::downcast<ParseTree::PlainParseTree>(children[0]);
        const auto option_tree = ValueInterface::downcast<ParseTree::PlainParseTree>(children[1]);

        bool has_option = false;
        ValueFactoryInterfacePtr optional_action;
        if (option_tree->label != "Empty") {
            const auto identifier = ValueInterface::downcast<ParseTree::PlainParseTree>(option_tree->children[0]);
            const auto action_name = identifier->value;
            const auto in_map = Bootstrap::BootstrapParser::action_map.find(action_name);
            // assert(in_map != BootstrapParser::action_map.end());
            if (in_map != Bootstrap::BootstrapParser::action_map.end()) {
                const auto action = in_map->second;
                optional_action = action;
                has_option = true;
            } else {
                // std::cout << "revert to default: cannot find action " << action_name << std::endl;
                has_option = false;
            }
        }

        if (repeat_tree->children.size() == 1) {
            const auto expr_value = ValueInterface::downcast<ExpressionValue>(repeat_tree->children[0]);
            if (has_option) {
                expr_value->expr->value_factory = optional_action;
            }
            return expr_value;
        }

        std::vector<ExpressionPtr> expr_list;
        for (const auto &iter : repeat_tree->children) {
            const auto child_tree = ValueInterface::downcast<ExpressionValue>(iter);
            expr_list.push_back(child_tree->expr);
        }
        ExpressionPtr seq_n_expr;
        if (has_option) {
            seq_n_expr = Expressions::SeqNExpression::instance(expr_list, optional_action);
        } else {
            seq_n_expr = Expressions::SeqNExpression::instance(expr_list);
        }
        return std::make_shared<ExpressionValue>(seq_n_expr, label, position, value, children);
    }
};

class RuleValue: public ParseTree::PlainParseTree {
public:
    std::vector<ExpressionPtr> rules;

    explicit RuleValue(
        const std::vector<ExpressionPtr> &rules, const std::string &label, const int &position,
        const std::string &value, const std::vector<ValueInterfacePtr> &children
    ) : PlainParseTree(label, position, value, children), rules(rules) {}
};

class CollectRules final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // Sequence ( / Sequence )*
        const auto &first_sequence = children[0];
        const auto first_seq_tree = ValueInterface::downcast<ParseTree::PlainParseTree>(first_sequence);
        const auto first_expr = ValueInterface::downcast<ExpressionValue>(first_seq_tree->children[0]);
        const auto first_rule = first_expr->expr;

        const auto &rest_of_sequences = ValueInterface::downcast<ParseTree::PlainParseTree>(children[1]);
        if (rest_of_sequences->label == "Empty") {
            return std::make_shared<RuleValue>(
                std::vector<ExpressionPtr>({first_rule}), label, position, value, children
            );
        }
        // there are more rules
        auto new_rules = std::vector<ExpressionPtr>({first_rule});
        for (const auto &iter : rest_of_sequences->children) {
            // Sequence of (SLASH Sequence)
            const auto iter_tree = ValueInterface::downcast<ParseTree::PlainParseTree>(iter);
            const auto iter_slash = ValueInterface::downcast<ParseTree::PlainParseTree>(iter_tree->children[0]);
            const auto iter_sequence = ValueInterface::downcast<ParseTree::PlainParseTree>(iter_tree->children[1]);
            const auto iter_expr = ValueInterface::downcast<ExpressionValue>(iter_sequence->children[0]);
            const auto iter_rule = iter_expr->expr;
            new_rules.push_back(iter_rule);
        }
        return std::make_shared<RuleValue>(new_rules, label, position, value, children);
    }
};

// initialize
std::unordered_map<std::string, std::vector<ExpressionPtr>> GrammarRuleValue::rules;
bool GrammarRuleValue::doLeftRecursion;

class DefinitionFactory final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // Definition <- Identifier LEFTARROW Expression
        const auto &seq_left = children[0]; // identifier
        const auto identifier = ValueInterface::downcast<ParseTree::PlainParseTree>(seq_left);
        const auto rule_string = identifier->value;

        const auto &seq_right = children[2]; // expression
        const auto right = ValueInterface::downcast<ParseTree::PlainParseTree>(seq_right);
        const auto rules = ValueInterface::downcast<RuleValue>(right->children[0])->rules;

        // check if there are already rules under the identifier
        const auto in_table = GrammarRuleValue::rules.find(rule_string);
        if (in_table != GrammarRuleValue::rules.end()) {
            // if so, just append to the existing list
            auto current_rule = in_table->second;
            current_rule.insert(std::end(current_rule), std::begin(rules), std::end(rules));
            GrammarRuleValue::rules[rule_string] = current_rule;
        } else {
            GrammarRuleValue::rules.insert({rule_string, rules});
        }

        return std::make_shared<GrammarRuleValue>(rule_string, label, position, value, children);
    }
};

class GrammarFactory final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // Grammar <- Spacing Definition+ EndOfFile
        const auto &seq_second = children[1];
        const auto &seq_second_tree = ValueInterface::downcast<ParseTree::PlainParseTree>(seq_second);
        // Plus has at least one child
        const auto first_child = ValueInterface::downcast<ParseTree::PlainParseTree>(seq_second_tree->children[0]);
        const auto definition_0 = ValueInterface::downcast<GrammarRuleValue>(first_child->children[0]);
        const auto start_string = definition_0->start_rule;
        const auto rules = GrammarRuleValue::rules;
        auto grammar = Grammar::instance(start_string, rules);

        // since Spacing is null, it needs to be removed
        constexpr int spacingIndex = 0;
        auto new_children = std::vector<ValueInterfacePtr>(children.size() - 1);
        std::copy(children.begin() + spacingIndex + 1, children.end(), new_children.begin());
        return std::make_shared<Bootstrap::GrammarValue>(grammar, label, position, value, new_children);
    }
};

class CharExtraFactory final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        const auto seq_right = ValueInterface::downcast<ParseTree::PlainParseTree>(children[1]);
        std::string new_value;
        if (seq_right->value == "'") {
            new_value = '\'';
        }
        if (seq_right->value == "\"") {
            new_value = '\"';
        }
        if (seq_right->value == "\\") {
            new_value = '\\';
        }
        if (seq_right->value == "n") {
            new_value = '\n';
        }
        if (seq_right->value == "r") {
            new_value = '\r';
        }
        if (seq_right->value == "t") {
            new_value = '\t';
        }
        if (seq_right->value == "[") {
            new_value = '[';
        }
        if (seq_right->value == "]") {
            new_value = ']';
        }
        if (new_value.empty()) {
            std::cout << "invalid escape character: " << seq_right->value << std::endl;
            assert(false);
        }
        return ParseTree::PlainParseTree::instance("CharExtra", position, new_value, children);
    }
};

class CharTerm final: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        const auto &lift = ValueInterface::downcast<ParseTree::PlainParseTree>(children[0]);
        auto expr = Expressions::TerminalExpression::instance(lift->value);
        return std::make_shared<ExpressionValue>(expr, "CharTerm", lift->position, lift->value, lift->children);
    }
};

class CharNonTerm final: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // Char '-' Char
        const auto start = ValueInterface::downcast<ParseTree::PlainParseTree>(children[0])->value;
        const auto end = ValueInterface::downcast<ParseTree::PlainParseTree>(children[3])->value;
        // assert(start.size() > 1); assert(end.size() > 2);
        auto expr = peg_non_terminal_from_alphabets(GrammarRuleValue::rules, start[0], end[0]);
        expr->value_factory = instance<TerminalFromRegexFactory>();
        return std::make_shared<ExpressionValue>(expr, "CharNonTerm", position, value, children);
    }
};

class RangeClassFactory: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // Class <- '[' (!']' Range)* ']' Spacing
        const auto &range = children[0];
        const auto &repeat_tree = ValueInterface::downcast<ParseTree::PlainParseTree>(range);
        // TODO: (!']' Range)* could return empty.... but why
        // !assert(repeat_tree.label == 'repeat')
        if (repeat_tree->children.size() == 1) {
            const auto repeat_child = repeat_tree->children[0];
            const auto expr = ValueInterface::downcast<ExpressionValue>(repeat_child)->expr;
            // const auto terminal_from_regex = instance<TerminalFromRegexFactory>();
            // expr->value_factory = terminal_from_regex;
            return std::make_shared<ExpressionValue>(expr, label, position, value, children);
        }
        // [0-9a-zA-Z_] => [0-9] / [a-z] / [A-Z] / '_'
        std::vector<ExpressionPtr> range_children_rules;
        std::string non_term_name;
        for (const auto &iter : repeat_tree->children) {
            const auto iter_expr_value = ValueInterface::downcast<ExpressionValue>(iter);
            auto expr = iter_expr_value->expr;
            if (iter_expr_value->label == "CharTerm") {
                non_term_name += std::dynamic_pointer_cast<Expressions::TerminalExpression>(expr)->terminal;
                non_term_name += "-";
            } else {
                non_term_name += std::dynamic_pointer_cast<Expressions::NonTerminalExpression>(expr)->literal;
                non_term_name += "-";
            }
            range_children_rules.push_back(expr);
        }
        non_term_name += "range_class";
        GrammarRuleValue::rules.insert({non_term_name, range_children_rules});
        const auto terminal_from_seq_n = get_spacing_factory<TerminalFromSeqN>(-1);
        const auto expr = Expressions::NonTerminalExpression::instance(non_term_name, terminal_from_seq_n);
        return std::make_shared<ExpressionValue>(expr, label, position, value, children);
    }
};

class What final: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        return ParseTree::PlainParseTree::instance("What", position, value, children);
    }
};

class ToDigitFactory final: public ExpressionValueFactory {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        // precedence detected
        // left recursion might be required
        GrammarRuleValue::doLeftRecursion = true;

        const auto digit = ValueInterface::downcast<RegexValue>(children[1]);
        const auto optional_digit = ValueInterface::downcast<ParseTree::PlainParseTree>(children[2]);
        if (optional_digit->label == "Empty") {
            return ParseTree::PlainParseTree::instance("Precedence", position, std::string() + digit->letter, children);
        }
        const auto tens_digit = (digit->letter);
        const auto ones_digit = ValueInterface::downcast<RegexValue>(optional_digit->children[0])->letter;
        const std::string precedence = std::string() + tens_digit + ones_digit;
        return ParseTree::PlainParseTree::instance("Precedence", position, precedence, children);
    }
};

GrammarPtr
make_peg_grammar() {
    std::string start = "Grammar";
    std::unordered_map<std::string, std::vector<ExpressionPtr>> rules;

    const auto terminal_from_regex = ValueFactoryInterface::instance<TerminalFromRegexFactory>();

    const auto string_from_regex = ValueFactoryInterface::instance<StringFromRegexFactory>();

    const auto lift = ValueFactoryInterface::instance<NonTerminalLift>();

    const auto seq_left = ValueFactoryInterface::instance<SequenceLeft>();

    const auto seq_right = ValueFactoryInterface::instance<SequenceRight>();

    const auto skip_nulls = ValueFactoryInterface::instance<SkipNulls>();

    const auto grammar_factory = ValueFactoryInterface::instance<GrammarFactory>();

    // Grammar <- Spacing Definition+ EndOfFile
    rules.insert(std::make_pair(
        "Grammar",
        std::vector<ExpressionPtr>({Expressions::SeqNExpression::instance(
            std::vector<ExpressionPtr>(
                {Expressions::NonTerminalExpression::instance(
                     "Spacing", ValueFactoryInterface::instance<NothingFactory>()
                 ),
                 Expressions::PlusExpression::instance(Expressions::NonTerminalExpression::instance("Definition")),
                 Expressions::NonTerminalExpression::instance("EndOfFile")}
            ),
            grammar_factory
        )})
    ));

    // Definition <- Identifier LEFTARROW Expression
    const auto definition_factory = ValueFactoryInterface::instance<DefinitionFactory>();
    rules.insert(std::make_pair(
        "Definition", std::vector<ExpressionPtr>({Expressions::SeqNExpression::instance(
                          std::vector<ExpressionPtr>({
                              Expressions::NonTerminalExpression::instance("Identifier", lift),
                              Expressions::NonTerminalExpression::instance("LEFTARROW"),
                              Expressions::NonTerminalExpression::instance("Expression"),
                          }),
                          definition_factory
                      )})
    ));

    // Expression <- Sequence (SLASH Sequence)*
    const auto rules_factory = ValueFactoryInterface::instance<CollectRules>();
    rules.insert(std::make_pair(
        "Expression", std::vector<ExpressionPtr>({Expressions::SeqNExpression::instance(
                          std::vector<ExpressionPtr>(
                              {Expressions::NonTerminalExpression::instance("Sequence"),
                               Expressions::RepeatExpression::instance(Expressions::SequenceExpression::instance(
                                   Expressions::NonTerminalExpression::instance("SLASH"),
                                   Expressions::NonTerminalExpression::instance("Sequence")
                               ))}
                          ),
                          rules_factory
                      )})
    ));

    // Sequence <- Prefix+ (ACTION Identifier)?
    const auto repeat_to_seq_factory = ValueFactoryInterface::instance<Sequence2SeqNFactory>();
    rules.insert(std::make_pair(
        "Sequence",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::PlusExpression::instance(Expressions::NonTerminalExpression::instance("Prefix", lift)),
            Expressions::OptionalExpression::instance(Expressions::SequenceExpression::instance(
                Expressions::NonTerminalExpression::instance("ACTION"),
                Expressions::NonTerminalExpression::instance("Identifier", lift), seq_right
            )),
            repeat_to_seq_factory
        )})
    ));

    // Prefix <- Prefix_Extra Suffix
    const auto prefix_factory = ValueFactoryInterface::instance<PrefixFactory>();
    rules.insert(std::make_pair(
        "Prefix",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::OptionalExpression::instance(Expressions::NonTerminalExpression::instance("Prefix_Extra", lift)
            ),
            Expressions::NonTerminalExpression::instance("Suffix", lift), prefix_factory
        )})
    ));

    // Prefix_Extra <- AND / NOT
    rules.insert(std::make_pair(
        "Prefix_Extra", std::vector<ExpressionPtr>({
                            Expressions::NonTerminalExpression::instance("AND"),
                            Expressions::NonTerminalExpression::instance("NOT"),
                        })
    ));

    // Suffix <- Primary Suffix_Extra
    const auto suffix_factory = ValueFactoryInterface::instance<SuffixFactory>();
    rules.insert(std::make_pair(
        "Suffix",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::NonTerminalExpression::instance("Primary", lift),
            Expressions::OptionalExpression::instance(Expressions::NonTerminalExpression::instance("Suffix_Extra", lift)
            ),
            suffix_factory
        )})
    ));

    // Suffix_Extra <- QUESTION / STAR / PLUS
    rules.insert(std::make_pair(
        "Suffix_Extra", std::vector<ExpressionPtr>(
                            {Expressions::NonTerminalExpression::instance("QUESTION"),
                             Expressions::NonTerminalExpression::instance("STAR"), // START / STAR
                             Expressions::NonTerminalExpression::instance("PLUS")}
                        )
    ));

    // Primary <- Identifier ('^'[0-9][0-9]?)? !LEFTARROW Spacing
    //          / OPEN Sequence CLOSE
    //          / Literal
    //          / Class
    //          / DOT
    const auto non_terminal_factory = ValueFactoryInterface::instance<NonTerminalFactory>();
    const auto embed_expr_factory = ValueFactoryInterface::instance<EmbedExprFactory>();
    const auto literal_factory = ValueFactoryInterface::instance<LiteralFactory>();
    const auto class_factory = ValueFactoryInterface::instance<RangeClassFactory>();
    const auto dot_factory = ValueFactoryInterface::instance<DotFactory>();
    const auto to_digit_factory = ValueFactoryInterface::instance<ToDigitFactory>();
    rules.insert(std::make_pair(
        "Primary",
        std::vector<ExpressionPtr>({
            Expressions::SeqNExpression::instance(
                {Expressions::NonTerminalExpression::instance("Identifier", lift),
                 Expressions::OptionalExpression::instance(Expressions::SeqNExpression::instance(
                     {Expressions::TerminalExpression::instance("^"), peg_sequence_from_digits(rules, 0, 9),
                      Expressions::OptionalExpression::instance(peg_sequence_from_digits(rules, 0, 9))},
                     to_digit_factory
                 )),
                 Expressions::NotExpression::instance(Expressions::NonTerminalExpression::instance("LEFTARROW")),
                 Expressions::NonTerminalExpression::instance("Spacing")},
                non_terminal_factory
            ), // NonTerminalExpression
            Expressions::SeqNExpression::instance(
                std::vector<ExpressionPtr>(
                    {Expressions::NonTerminalExpression::instance("OPEN"),
                     Expressions::NonTerminalExpression::instance("Sequence"),
                     Expressions::NonTerminalExpression::instance("CLOSE")}
                ),
                embed_expr_factory
            ),
            Expressions::NonTerminalExpression::instance("Literal", literal_factory), // meant to create terminal values
            Expressions::NonTerminalExpression::instance("Class", class_factory),     // meant to create regexes */
            Expressions::NonTerminalExpression::instance("DOT", dot_factory)          // meant to create dot expressions
        })
    ));

    // Identifier <- IdentStart IdentCont* Spacing
    const auto terminal_from_seq_n = get_spacing_factory<TerminalFromSeqN>(2);
    rules.insert(std::make_pair(
        "Identifier", std::vector<ExpressionPtr>({Expressions::SeqNExpression::instance(
                          std::vector<ExpressionPtr>({
                              Expressions::NonTerminalExpression::instance("IdentStart", terminal_from_regex),
                              Expressions::RepeatExpression::instance(
                                  Expressions::NonTerminalExpression::instance("IdentCont", lift), terminal_from_regex
                              ),
                              Expressions::NonTerminalExpression::instance(
                                  "Spacing", ValueFactoryInterface::instance<NothingFactory>()
                              ),
                          }),
                          terminal_from_seq_n
                      )})
    ));

    // IdentStart <- [A-Z] / [a-z] / '_'
    rules.insert(std::make_pair(
        "IdentStart",
        std::vector<ExpressionPtr>(
            {peg_non_terminal_from_alphabets(rules, 'A', 'Z'), peg_non_terminal_from_alphabets(rules, 'a', 'z'),
             Expressions::TerminalExpression::instance("_", ValueFactoryInterface::instance<RegexTerminalFactory>())}
        )
    ));

    // IdentCont <- IdentStart / [0-9]
    rules.insert(std::make_pair(
        "IdentCont",
        std::vector<ExpressionPtr>(
            {Expressions::NonTerminalExpression::instance("IdentStart", lift), peg_sequence_from_digits(rules, 0, 9)}
        )
    ));

    // Literal <- "'" !("'" Char) "'" Spacing
    //         /  "\"" !("\"" Char) "\"" Spacing
    const auto nothing = ValueFactoryInterface::instance<NothingFactory>();
    rules.insert(std::make_pair(
        "Literal",
        std::vector<ExpressionPtr>(
            {Expressions::SeqNExpression::instance(
                 std::vector<ExpressionPtr>(
                     {Expressions::TerminalExpression::instance("'", nothing),
                      Expressions::PlusExpression::instance(
                          Expressions::SequenceExpression::instance(
                              Expressions::NotExpression::instance(Expressions::TerminalExpression::instance("'")),
                              Expressions::NonTerminalExpression::instance("Char", lift), seq_right
                          ),
                          ValueFactoryInterface::instance<CollectRepeats>()
                      ),
                      Expressions::TerminalExpression::instance("'", nothing),
                      Expressions::NonTerminalExpression::instance("Spacing", nothing)}
                 ),
                 skip_nulls
             ),
             Expressions::SeqNExpression::instance(
                 std::vector<ExpressionPtr>(
                     {Expressions::TerminalExpression::instance("\""),
                      Expressions::PlusExpression::instance(
                          Expressions::SequenceExpression::instance(
                              Expressions::NotExpression::instance(Expressions::TerminalExpression::instance("\"")),
                              Expressions::NonTerminalExpression::instance("Char", lift), seq_right
                          ),
                          ValueFactoryInterface::instance<CollectRepeats>()
                      ),
                      Expressions::TerminalExpression::instance("\""),
                      Expressions::NonTerminalExpression::instance(
                          "Spacing", ValueFactoryInterface::instance<NothingFactory>()
                      )}
                 ),
                 skip_nulls
             )}
        )
    ));

    // Class <- '[' (!']' Range)+ ']' Spacing
    rules.insert(std::make_pair(
        "Class", std::vector<ExpressionPtr>({Expressions::SeqNExpression::instance(
                     std::vector<ExpressionPtr>(
                         {Expressions::TerminalExpression::instance("["),
                          Expressions::PlusExpression::instance(Expressions::SequenceExpression::instance(
                              Expressions::NotExpression::instance(Expressions::TerminalExpression::instance("]")),
                              Expressions::NonTerminalExpression::instance("Range", lift), seq_right
                          )),
                          Expressions::TerminalExpression::instance("]"),
                          Expressions::NonTerminalExpression::instance(
                              "Spacing", ValueFactoryInterface::instance<NothingFactory>()
                          )}
                     ),
                     skip_nulls
                 )})
    ));

    // Range <- Char '-' Char
    //       / Char
    const auto char_term = ValueFactoryInterface::instance<CharTerm>();
    const auto char_non_term = ValueFactoryInterface::instance<CharNonTerm>();
    rules.insert(std::make_pair(
        "Range", std::vector<ExpressionPtr>(
                     {Expressions::SeqNExpression::instance(
                          std::vector<ExpressionPtr>({
                              Expressions::NonTerminalExpression::instance("Char", char_term),
                              Expressions::TerminalExpression::instance("-"),
                              Expressions::NotExpression::instance(Expressions::TerminalExpression::instance("]")),
                              Expressions::NonTerminalExpression::instance("Char", char_term),
                          }),
                          char_non_term
                      ),
                      Expressions::NonTerminalExpression::instance("Char", char_term)}
                 )
    ));

    // Char <- '\\' Char_Extra
    //      \  !'\\' .
    rules.insert(std::make_pair(
        "Char", std::vector<ExpressionPtr>(
                    {Expressions::SequenceExpression::instance(
                         Expressions::TerminalExpression::instance(R"N(\)N"),
                         Expressions::NonTerminalExpression::instance("Char_Extra", lift),
                         ValueFactoryInterface::instance<CharExtraFactory>()
                     ),
                     Expressions::SequenceExpression::instance(
                         Expressions::NotExpression::instance(Expressions::TerminalExpression::instance("\\")),
                         Expressions::DotExpression::instance(), seq_right
                     )}
                )
    ));

    // Char_Extra <- "'" / "\"" / "\\" / "n" / "r" / "t" / "[" / "]"
    rules.insert(std::make_pair(
        "Char_Extra", std::vector<ExpressionPtr>({
                          Expressions::TerminalExpression::instance("'"),
                          Expressions::TerminalExpression::instance("\""),
                          Expressions::TerminalExpression::instance("\\"),
                          Expressions::TerminalExpression::instance("n"),
                          Expressions::TerminalExpression::instance("r"),
                          Expressions::TerminalExpression::instance("t"),
                          Expressions::TerminalExpression::instance("["),
                          Expressions::TerminalExpression::instance("]"),
                      })
    ));

    // LEFTARROW <- "<-" Spacing
    rules.insert(std::make_pair(
        "LEFTARROW",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::TerminalExpression::instance("<-"),
            Expressions::NonTerminalExpression::instance("Spacing", ValueFactoryInterface::instance<NothingFactory>()),
            seq_left
        )})
    ));

    // SLASH <- "/" Spacing
    rules.insert(std::make_pair(
        "SLASH",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::TerminalExpression::instance("/"),
            Expressions::NonTerminalExpression::instance("Spacing", ValueFactoryInterface::instance<NothingFactory>()),
            seq_left
        )})
    ));

    // ACTION <- "@" Spacing
    rules.insert(
        {"ACTION",
         std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
             Expressions::TerminalExpression::instance("@"),
             Expressions::NonTerminalExpression::instance("Spacing", ValueFactoryInterface::instance<NothingFactory>()),
             seq_left
         )})}
    );

    // AND <- "&" Spacing
    rules.insert(std::make_pair(
        "AND",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::TerminalExpression::instance("&"),
            Expressions::NonTerminalExpression::instance("Spacing", ValueFactoryInterface::instance<NothingFactory>()),
            seq_left
        )})
    ));

    // NOT <- "!" Spacing
    rules.insert(std::make_pair(
        "NOT",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::TerminalExpression::instance("!"),
            Expressions::NonTerminalExpression::instance("Spacing", ValueFactoryInterface::instance<NothingFactory>()),
            seq_left
        )})
    ));

    // QUESTION <- "?" Spacing
    rules.insert(std::make_pair(
        "QUESTION",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::TerminalExpression::instance("?"),
            Expressions::NonTerminalExpression::instance("Spacing", ValueFactoryInterface::instance<NothingFactory>()),
            seq_left
        )})
    ));

    // STAR <- "*" Spacing
    rules.insert(std::make_pair(
        "STAR",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::TerminalExpression::instance("*"),
            Expressions::NonTerminalExpression::instance("Spacing", ValueFactoryInterface::instance<NothingFactory>()),
            seq_left
        )})
    ));

    // PLUS <- "+" Spacing
    rules.insert(std::make_pair(
        "PLUS",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::TerminalExpression::instance("+"),
            Expressions::NonTerminalExpression::instance("Spacing", ValueFactoryInterface::instance<NothingFactory>()),
            seq_left
        )})
    ));

    // OPEN <- "(" Spacing
    rules.insert(std::make_pair(
        "OPEN",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::TerminalExpression::instance("("),
            Expressions::NonTerminalExpression::instance("Spacing", ValueFactoryInterface::instance<NothingFactory>()),
            seq_left
        )})
    ));

    // CLOSE <- ")" Spacing
    rules.insert(std::make_pair(
        "CLOSE",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::TerminalExpression::instance(")"),
            Expressions::NonTerminalExpression::instance("Spacing", ValueFactoryInterface::instance<NothingFactory>()),
            seq_left
        )})
    ));

    // DOT <- "." Spacing
    rules.insert(std::make_pair(
        "DOT",
        std::vector<ExpressionPtr>({Expressions::SequenceExpression::instance(
            Expressions::TerminalExpression::instance("."),
            Expressions::NonTerminalExpression::instance("Spacing", ValueFactoryInterface::instance<NothingFactory>()),
            seq_left
        )})
    ));

    // Spacing <- Spacing_Extra*
    rules.insert(std::make_pair(
        "Spacing",
        std::vector<ExpressionPtr>(
            {Expressions::RepeatExpression::instance(Expressions::NonTerminalExpression::instance("Spacing_Extra"))}
        )
    ));

    // Spacing_Extra <- Space / Comment
    rules.insert(std::make_pair(
        "Spacing_Extra", std::vector<ExpressionPtr>(
                             {Expressions::NonTerminalExpression::instance("Space"),
                              Expressions::NonTerminalExpression::instance("Comment")}
                         )
    ));

    // Comment <- '#' (!(EndOfLine .)*
    //         /  EndOfLine
    rules.insert(std::make_pair(
        "Comment",
        std::vector<ExpressionPtr>({Expressions::SeqNExpression::instance(std::vector<ExpressionPtr>(
            {Expressions::TerminalExpression::instance("#"),
             Expressions::RepeatExpression::instance(Expressions::SequenceExpression::instance(
                 Expressions::NotExpression::instance(Expressions::NonTerminalExpression::instance("EndOfLine")),
                 Expressions::DotExpression::instance()
             )),
             Expressions::NonTerminalExpression::instance("EndOfLine")}
        ))})
    ));

    // TODO: maybe repeat could just take a list of expressions...

    // Space <- " " / "\t" / EndOfLine
    rules.insert(std::make_pair(
        "Space", std::vector<ExpressionPtr>(
                     {Expressions::TerminalExpression::instance(" "), Expressions::TerminalExpression::instance("\t"),
                      Expressions::NonTerminalExpression::instance("EndOfLine")}
                 )
    ));

    // EndOfLine <- "\r" / "\n" / "\n\r"
    rules.insert(std::make_pair(
        "EndOfLine",
        std::vector<ExpressionPtr>(
            {Expressions::SequenceExpression::instance(
                 Expressions::TerminalExpression::instance("\r"), Expressions::TerminalExpression::instance("\n")
             ),
             Expressions::TerminalExpression::instance("\n"), Expressions::TerminalExpression::instance("\r")}
        )
    ));

    // EndOfFile <- !.
    rules.insert(std::make_pair(
        "EndOfFile",
        std::vector<ExpressionPtr>({Expressions::NotExpression::instance(Expressions::DotExpression::instance())})
    ));

    return Grammar::instance(start, rules);
}

Bootstrap::BootstrapParser::BootstrapParser(
    const std::string &input, std::unordered_map<std::string, ValueFactoryInterfacePtr> custom_action_map
) {
    action_map = std::move(custom_action_map);
    peg_grammar = make_peg_grammar();
}

} // namespace PEG
