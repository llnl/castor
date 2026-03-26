// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expressions/EmptyExpression.h>
#include <PEG/Expressions/NonTerminalExpression.h>
#include <PEG/Expressions/NotExpression.h>
#include <PEG/Expressions/PlusExpression.h>
#include <PEG/Expressions/RepeatExpression.h>
#include <PEG/Expressions/SequenceExpression.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <PEG/Parser.h>
#include <cassert>
#include <iostream>

PEG::ExpressionPtr
peg_sequence_from_digits(
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> &rules, const int start, const int end,
    const PEG::ValueFactoryInterfacePtr value_factory_interface
) {
    // peg_sequence_from_digits(0, 2) {
    if (end < start) {
        return PEG::Expressions::EmptyExpression::instance(value_factory_interface);
    }
    std::string digit = std::string("Digit_") + std::to_string(start) + "_" + std::to_string(end);
    auto w = std::vector<PEG::ExpressionPtr>(
        {PEG::Expressions::TerminalExpression::instance(std::to_string(start), value_factory_interface)}
    );
    for (int i = start + 1; i <= end; ++i) {
        w.push_back(PEG::Expressions::TerminalExpression::instance(std::to_string(i), value_factory_interface));
    }

    rules.insert(std::make_pair(digit, w));

    return PEG::Expressions::NonTerminalExpression::instance(digit, value_factory_interface);
}

// peg_sequence_from_alphabets('A', 'Z')
PEG::ExpressionPtr
peg_non_terminal_from_alphabets(
    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> &rules, const char start, const char end,
    const PEG::ValueFactoryInterfacePtr value_factory_interface
) {
    // auto value_factory_interface = PEG::PlainParseTreeFactory();
    if (end < start) {
        return PEG::Expressions::EmptyExpression::instance(value_factory_interface);
    }

    // std::to_string(start) not the same as std::string(1, start)
    std::string non_terminal_char = "Alphabet_" + std::string(1, start) + "_" + end;
    auto x = std::vector<PEG::ExpressionPtr>(
        {PEG::Expressions::TerminalExpression::instance(std::string(1, start), value_factory_interface)}
    );
    for (char i = start + 1; i <= end; ++i) {
        x.push_back(PEG::Expressions::TerminalExpression::instance(std::string(1, i), value_factory_interface));
    }
    rules.insert(std::make_pair(non_terminal_char, x));

    return PEG::Expressions::NonTerminalExpression::instance(non_terminal_char, value_factory_interface);
}

PEG::GrammarPtr
make_grammar(const PEG::ValueFactoryInterfacePtr value_factory_interface) {
    // std::vector<std::string> rules = {"expr <- 2"};
    // Grammar grammar(rules);

    std::string g_vc = "VC";

    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> g_rules;
    // rules["expr"] = g_rule;
    g_rules.insert(std::make_pair(
        g_vc, std::vector<PEG::ExpressionPtr>(
                  {PEG::Expressions::NonTerminalExpression::instance("LoopVariant", value_factory_interface),
                   PEG::Expressions::NonTerminalExpression::instance("LoopInvariant", value_factory_interface),
                   PEG::Expressions::NonTerminalExpression::instance("Writes", value_factory_interface),
                   PEG::Expressions::NonTerminalExpression::instance("Requires", value_factory_interface),
                   PEG::Expressions::NonTerminalExpression::instance("Ensures", value_factory_interface),
                   PEG::Expressions::NonTerminalExpression::instance("Valid", value_factory_interface),
                   PEG::Expressions::NonTerminalExpression::instance("Separated", value_factory_interface)}
              )
    ));

    g_rules.insert(std::make_pair(
        "Expression", std::vector<PEG::ExpressionPtr>(
                          {PEG::Expressions::NonTerminalExpression::instance("PredicateCall", value_factory_interface),
                           PEG::Expressions::NonTerminalExpression::instance("Quantifier", value_factory_interface),
                           PEG::Expressions::NonTerminalExpression::instance("TEST", value_factory_interface)}
                      )
    ));

    g_rules.insert(std::make_pair(
        "TEST",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::NonTerminalExpression::instance("Atom", value_factory_interface),
            PEG::Expressions::RepeatExpression::instance(
                PEG::Expressions::SequenceExpression::instance(
                    PEG::Expressions::TerminalExpression::instance(",", value_factory_interface),
                    PEG::Expressions::NonTerminalExpression::instance("Atom", value_factory_interface), value_factory_interface
                ),
                value_factory_interface
            ),
            value_factory_interface
        )})
    ));

    g_rules.insert(std::make_pair(
        "Atom", std::vector<PEG::ExpressionPtr>(
                    {PEG::Expressions::SequenceExpression::instance(
                         PEG::Expressions::SequenceExpression::instance(
                             PEG::Expressions::TerminalExpression::instance("(", value_factory_interface),
                             PEG::Expressions::NonTerminalExpression::instance("Expression", value_factory_interface),
                             value_factory_interface
                         ),
                         PEG::Expressions::TerminalExpression::instance(")", value_factory_interface), value_factory_interface
                     ),
                     PEG::Expressions::NonTerminalExpression::instance("Literal", value_factory_interface),
                     PEG::Expressions::NonTerminalExpression::instance("WriteableExpression", value_factory_interface),
                     PEG::Expressions::SequenceExpression::instance(
                         PEG::Expressions::NonTerminalExpression::instance("Prefix_Operator", value_factory_interface),
                         PEG::Expressions::NonTerminalExpression::instance("Atom", value_factory_interface), value_factory_interface
                     )}
                )
    ));

    g_rules.insert(std::make_pair(
        "PredicateCall",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::SequenceExpression::instance(
                PEG::Expressions::SequenceExpression::instance(
                    PEG::Expressions::NonTerminalExpression::instance("VariableReference", value_factory_interface),
                    PEG::Expressions::TerminalExpression::instance("(", value_factory_interface), value_factory_interface
                ),
                PEG::Expressions::NonTerminalExpression::instance("ExpressionList", value_factory_interface), value_factory_interface
            ),
            PEG::Expressions::TerminalExpression::instance(")", value_factory_interface), value_factory_interface
        )})
    ));

    g_rules.insert(std::make_pair(
        "ExpressionList",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::NonTerminalExpression::instance("Expression", value_factory_interface),
            PEG::Expressions::RepeatExpression::instance(
                PEG::Expressions::SequenceExpression::instance(
                    PEG::Expressions::TerminalExpression::instance(",", value_factory_interface),
                    PEG::Expressions::NonTerminalExpression::instance("VariableReference", value_factory_interface),
                    value_factory_interface
                ),
                value_factory_interface
            ),
            value_factory_interface
        )})
    ));

    g_rules.insert(std::make_pair(
        "Quantifier",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::SequenceExpression::instance(
                PEG::Expressions::SequenceExpression::instance(
                    PEG::Expressions::NonTerminalExpression::instance("Quant", value_factory_interface),
                    PEG::Expressions::NonTerminalExpression::instance("VarList", value_factory_interface), value_factory_interface
                ),
                PEG::Expressions::TerminalExpression::instance(".", value_factory_interface), value_factory_interface
            ),
            PEG::Expressions::NonTerminalExpression::instance("Expression", value_factory_interface), value_factory_interface
        )})
    ));

    g_rules.insert(std::make_pair(
        "Quant", std::vector<PEG::ExpressionPtr>(
                     {PEG::Expressions::TerminalExpression::instance("forall", value_factory_interface),
                      PEG::Expressions::TerminalExpression::instance("exists", value_factory_interface)}
                 )
    ));

    g_rules.insert(std::make_pair(
        "VarList", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                       PEG::Expressions::NonTerminalExpression::instance("VariableReference", value_factory_interface),
                       PEG::Expressions::RepeatExpression::instance(
                           PEG::Expressions::SequenceExpression::instance(
                               PEG::Expressions::TerminalExpression::instance(",", value_factory_interface),
                               PEG::Expressions::NonTerminalExpression::instance("VariableReference", value_factory_interface),
                               value_factory_interface
                           ),
                           value_factory_interface
                       ),
                       value_factory_interface
                   )})
    ));

    g_rules.insert(std::make_pair(
        "Literal",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::NonTerminalExpression::instance("Number", value_factory_interface)})
    ));

    g_rules.insert(std::make_pair(
        "WriteableExpression", std::vector<PEG::ExpressionPtr>(
                                   {PEG::Expressions::NonTerminalExpression::instance("VariableReference", value_factory_interface),
                                    PEG::Expressions::NonTerminalExpression::instance("PointerDereference", value_factory_interface)}
                               )
    ));

    g_rules.insert(std::make_pair(
        "VariableReference",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::NonTerminalExpression::instance("Identifier", value_factory_interface)})
    ));

    g_rules.insert(std::make_pair(
        "PointerDereference", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                                  PEG::Expressions::TerminalExpression::instance("*", value_factory_interface),
                                  PEG::Expressions::NonTerminalExpression::instance("WriteableExpression", value_factory_interface),
                                  value_factory_interface
                              )})
    ));

    g_rules.insert(std::make_pair(
        "WriteableExpressionList",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::NonTerminalExpression::instance("WriteableExpression", value_factory_interface),
            PEG::Expressions::RepeatExpression::instance(
                PEG::Expressions::SequenceExpression::instance(
                    PEG::Expressions::TerminalExpression::instance(",", value_factory_interface),
                    PEG::Expressions::NonTerminalExpression::instance("WriteableExpression", value_factory_interface),
                    value_factory_interface
                ),
                value_factory_interface
            ),
            value_factory_interface
        )})
    ));

    g_rules.insert(std::make_pair(
        "LoopVariant",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::TerminalExpression::instance("variant", value_factory_interface),
            PEG::Expressions::NonTerminalExpression::instance("Expression", value_factory_interface), value_factory_interface
        )})
    ));

    g_rules.insert(std::make_pair(
        "LoopInvariant",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::TerminalExpression::instance("invariant", value_factory_interface),
            PEG::Expressions::NonTerminalExpression::instance("Expression", value_factory_interface), value_factory_interface
        )})
    ));

    g_rules.insert(std::make_pair(
        "Writes", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                      PEG::Expressions::TerminalExpression::instance("writes", value_factory_interface),
                      PEG::Expressions::NonTerminalExpression::instance("WriteableExpressionList", value_factory_interface),
                      value_factory_interface
                  )})
    ));

    g_rules.insert(std::make_pair(
        "Requires",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::TerminalExpression::instance("requires", value_factory_interface),
            PEG::Expressions::NonTerminalExpression::instance("Expression", value_factory_interface), value_factory_interface
        )})
    ));

    g_rules.insert(std::make_pair(
        "Ensures",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::TerminalExpression::instance("ensures", value_factory_interface),
            PEG::Expressions::NonTerminalExpression::instance("Expression", value_factory_interface), value_factory_interface
        )})
    ));

    g_rules.insert(std::make_pair(
        "Valid", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                     PEG::Expressions::TerminalExpression::instance("valid", value_factory_interface),
                     PEG::Expressions::NonTerminalExpression::instance("WriteableExpressionList", value_factory_interface),
                     value_factory_interface
                 )})
    ));

    g_rules.insert(std::make_pair(
        "Separated", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                         PEG::Expressions::TerminalExpression::instance("separated", value_factory_interface),
                         PEG::Expressions::NonTerminalExpression::instance("WriteableExpressionList", value_factory_interface),
                         value_factory_interface
                     )})
    ));

    g_rules.insert(std::make_pair(
        "Identifier",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::NotExpression::instance(
                PEG::Expressions::NonTerminalExpression::instance("Keyword", value_factory_interface), value_factory_interface
            ),
            PEG::Expressions::NonTerminalExpression::instance("Name", value_factory_interface), value_factory_interface
        )})
    ));

    g_rules.insert(std::make_pair(
        "KeywordExtra", std::vector<PEG::ExpressionPtr>(
                            {PEG::Expressions::TerminalExpression::instance("exists", value_factory_interface),
                             PEG::Expressions::TerminalExpression::instance("forall", value_factory_interface),
                             PEG::Expressions::TerminalExpression::instance("variant", value_factory_interface),
                             PEG::Expressions::TerminalExpression::instance("invariant", value_factory_interface),
                             PEG::Expressions::TerminalExpression::instance("writes", value_factory_interface),
                             PEG::Expressions::TerminalExpression::instance("requires", value_factory_interface),
                             PEG::Expressions::TerminalExpression::instance("ensures", value_factory_interface),
                             PEG::Expressions::TerminalExpression::instance("valid", value_factory_interface),
                             PEG::Expressions::TerminalExpression::instance("separated", value_factory_interface)}
                        )
    ));

    g_rules.insert(std::make_pair(
        "Keyword",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::NonTerminalExpression::instance("KeywordExtra", value_factory_interface),
            PEG::Expressions::NotExpression::instance(
                PEG::Expressions::NonTerminalExpression::instance("Name", value_factory_interface), value_factory_interface
            ),
            value_factory_interface
        )})
    ));

    auto digitsNonTerm = peg_sequence_from_digits(g_rules, 0, 9, value_factory_interface);

    g_rules.insert(std::make_pair(
        "Number",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::PlusExpression::instance(digitsNonTerm, value_factory_interface)})
    ));

    g_rules.insert(std::make_pair(
        "Name",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::NonTerminalExpression::instance("Char_", value_factory_interface),
            PEG::Expressions::RepeatExpression::instance(
                PEG::Expressions::NonTerminalExpression::instance("CharDigit_", value_factory_interface), value_factory_interface
            ),
            value_factory_interface
        )})
    ));

    auto smallAlphabetsNonTerm = peg_non_terminal_from_alphabets(g_rules, 'a', 'z', value_factory_interface);
    auto capitalAlphabetsNonTerm = peg_non_terminal_from_alphabets(g_rules, 'A', 'Z', value_factory_interface);

    g_rules.insert(std::make_pair(
        "Char_", std::vector<PEG::ExpressionPtr>(
                     {smallAlphabetsNonTerm, capitalAlphabetsNonTerm,
                      PEG::Expressions::TerminalExpression::instance("_", value_factory_interface)}
                 )
    ));

    g_rules.insert(std::make_pair(
        "CharDigit_", std::vector<PEG::ExpressionPtr>(
                          {smallAlphabetsNonTerm, capitalAlphabetsNonTerm, digitsNonTerm,
                           PEG::Expressions::TerminalExpression::instance("_", value_factory_interface)}
                      )
    ));

    g_rules.insert(std::make_pair(
        "Prefix_Operator", std::vector<PEG::ExpressionPtr>(
                               {PEG::Expressions::TerminalExpression::instance("!", value_factory_interface),
                                PEG::Expressions::TerminalExpression::instance("-", value_factory_interface)}
                           )
    ));

    g_rules.insert(std::make_pair(
        "Infix_Operator", std::vector<PEG::ExpressionPtr>({
                              PEG::Expressions::TerminalExpression::instance("->", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance("<->", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance(">=", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance("<=", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance("\\/", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance("/\\", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance("==", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance("!=", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance("-", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance("+", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance("/", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance(">", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance("<", value_factory_interface),
                              PEG::Expressions::TerminalExpression::instance("*", value_factory_interface),
                          })
    ));

    ;

    return PEG::Grammar::instance(g_vc, g_rules);
}

int
main() {
    std::cout << "hello, world to test pragma!" << std::endl;

    const auto plain_parse_result_factory = PEG::ParseTree::PlainParseTreeFactory::instance<PEG::ParseTree::PlainParseTreeFactory>();

    auto parser = PEG::Parser(make_grammar(plain_parse_result_factory));
    // const std::string input = "requires{a.addr<>b.addr/\\tape[a.addr]<>b.addr}";
    const std::string input = "requires id ( forall x . 5)";
    const auto parseResult = parser.parse(input);

    std::cout << "result: ";
    std::cout << "\n\t isSuccess :" << parseResult->isSuccess();
    std::cout << "\n\t isFullSuccess :" << parseResult->isFullSuccess();
    std::cout << "\n\t parseTree :";
    // PEG::ValueInterface::downcast<PEG::PlainParseTree>(parseResult->valueOutput)->print(0);
    std::cout << "\n\t remainder : \"" << input.substr(parseResult->position);
    // parseResult->output();
    std::cout << "\"\n\t input : " << input;

    assert(parseResult->isFullSuccess());
    return 0;
}