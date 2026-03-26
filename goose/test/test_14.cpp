// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expressions/EmptyExpression.h>
#include <PEG/Expressions/NonTerminalExpression.h>
#include <PEG/Expressions/PlusExpression.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Parser.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <PEG/Value/ValueInterface.h>
#include <cassert>
#include <iostream>
#include <ostream>
#include <unordered_map>

PEG::ExpressionPtr peg_sequence_from_digits(
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> &rules, const int &start, const int &end
);

// peg_sequence_from_alphabets('A', 'Z')
PEG::ExpressionPtr peg_non_terminal_from_alphabets(
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> &rules, const char &start, const char &end
);

class RegexValue final: public PEG::ValueInterface {
public:
    char letter;

    explicit RegexValue(const char &letter) : letter(letter) {}

    ~RegexValue() override = default;
};

class RegexTerminalFactory final: public PEG::ValueFactoryInterface {
public:
    PEG::ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<PEG::ValueInterfacePtr> &children
    ) override {
        return std::make_shared<RegexValue>(value[0]);
    }
};

class RegexNonTerminalFactory final: public PEG::ValueFactoryInterface {
    PEG::ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<PEG::ValueInterfacePtr> &children
    ) override {
        // assert(children.size() == 1);
        return PEG::ValueInterface::downcast<RegexValue>(children[0]);
    }
};

class TerminalFromRegexFactory final: public PEG::ValueFactoryInterface {
public:
    PEG::ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<PEG::ValueInterfacePtr> &children
    ) override {
        // assert(label == "repeat"); position, value are unused
        if (label == "Empty") {
            return PEG::ParseTree::PlainParseTreeFactory().createParseTree(label, position, value, children);
        }
        std::string out;
        for (const auto &iter : children) {
            out += PEG::ValueInterface::downcast<RegexValue>(iter)->letter;
        }
        return PEG::ParseTree::PlainParseTreeFactory().createParseTree("terminal", -1, out, std::vector<PEG::ValueInterfacePtr>());
    }
};

// E <- [A-Z]* / N
// N <- [a-z]+ / D
// D <- [0-9]+
// rewritten as
// E <- (A_Z)+ / N
// A_Z <- A / B / ....... / Y / Z
// N <- (a_z)+ / D
// a_z <- a / b / ....... / y / z
// D <- (digit)+
// digit <- 0 / 1 / .... / 8 / 9
PEG::GrammarPtr
make_grammar() {
    std::string start = "E";
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> rules;

    const auto regex_to_parse_value = PEG::ValueFactoryInterface::instance<TerminalFromRegexFactory>();

    rules.insert(std::make_pair(
        "E", std::vector<PEG::ExpressionPtr>(
                 {PEG::Expressions::PlusExpression::instance(peg_non_terminal_from_alphabets(rules, 'A', 'Z'), regex_to_parse_value),
                  PEG::Expressions::NonTerminalExpression::instance("N")}
             )
    ));

    rules.insert(std::make_pair(
        "N", std::vector<PEG::ExpressionPtr>(
                 {PEG::Expressions::PlusExpression::instance(peg_non_terminal_from_alphabets(rules, 'a', 'z'), regex_to_parse_value),
                  PEG::Expressions::NonTerminalExpression::instance("D")}
             )
    ));

    rules.insert(std::make_pair(
        "D", std::vector<PEG::ExpressionPtr>(
                 {PEG::Expressions::PlusExpression::instance(peg_sequence_from_digits(rules, 0, 9), regex_to_parse_value)}
             )
    ));

    return PEG::Grammar::instance(start, rules);
}

int
main() {
    const auto grammar = make_grammar();
    auto parser = PEG::Parser(grammar);

    const std::string input_1 = "abcdefghijk";

    grammar->print();

    const auto result_1 = parser.parse(input_1);
    std::cout << "\n result_1: " << result_1->isFullSuccess() << std::endl;
    // TODO: clearing memo table doesnt work?
    PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result_1->valueOutput)->print(0);

    assert(result_1->isFullSuccess());

    return 0;
}

PEG::ExpressionPtr
peg_sequence_from_digits(
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> &rules, const int &start, const int &end
) {
    const auto value_factory_interface_ptr = PEG::ValueFactoryInterface::instance<RegexTerminalFactory>();
    // peg_sequence_from_digits(0, 2) {
    if (end < start) {
        return PEG::Expressions::EmptyExpression::instance();
    }
    std::string digit = std::string("Digit_") + std::to_string(start) + "_" + std::to_string(end);
    auto w = std::vector<PEG::ExpressionPtr>({});
    for (int i = start; i <= end; ++i) {
        w.push_back(PEG::Expressions::TerminalExpression::instance(std::to_string(i), value_factory_interface_ptr));
    }

    rules.insert(std::make_pair(digit, w));

    const auto value_factory_interface = PEG::ValueFactoryInterface::instance<RegexNonTerminalFactory>();

    return PEG::Expressions::NonTerminalExpression::instance(digit, value_factory_interface);
}

// peg_sequence_from_alphabets('A', 'Z')
PEG::ExpressionPtr
peg_non_terminal_from_alphabets(
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> &rules, const char &start, const char &end
) {
    const PEG::ValueFactoryInterfacePtr terminal_to_regex =
        PEG::ValueFactoryInterface::instance<RegexTerminalFactory>();
    if (end < start) {
        return PEG::Expressions::EmptyExpression::instance();
    }

    // std::to_string(start) not the same as std::string(1, start)
    std::string non_terminal_char = "Alphabet_" + std::string(1, start) + "_" + end;
    auto x = std::vector<PEG::ExpressionPtr>({});
    for (char i = start; i <= end; ++i) {
        x.push_back(PEG::Expressions::TerminalExpression::instance(std::string(1, i), terminal_to_regex));
    }
    rules.insert(std::make_pair(non_terminal_char, x));

    const auto non_terminal_lift_terminal_factory = PEG::ValueFactoryInterface::instance<RegexNonTerminalFactory>();

    return PEG::Expressions::NonTerminalExpression::instance(non_terminal_char, non_terminal_lift_terminal_factory);
}