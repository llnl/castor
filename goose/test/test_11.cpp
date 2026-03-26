// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "libPEG.h"
#include <cassert>
#include <iostream>
#include <unordered_map>

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
make_grammar() {
    // std::vector<std::string> rules = {"expr <- 2"};
    // Grammar grammar(rules);

    std::unordered_map<std::string, std::vector<PEG::ExpressionPtr>> p_rules;
    // rules["expr"] = g_rule;

    const auto value_factory_interface = std::make_shared<PEG::ParseTree::PlainParseTreeFactory>();

    std::string p_vc = "VC";

    p_rules.insert(std::make_pair(
        "VC",
        std::vector<PEG::ExpressionPtr>(
            {PEG::Expressions::NonTerminalExpression::instance("LoopVariant"), PEG::Expressions::NonTerminalExpression::instance("LoopInvariant"),
             PEG::Expressions::NonTerminalExpression::instance("Writes"), PEG::Expressions::NonTerminalExpression::instance("Requires"),
             PEG::Expressions::NonTerminalExpression::instance("Ensures"), PEG::Expressions::NonTerminalExpression::instance("Assert")}
        )
    ));

    p_rules.insert(std::make_pair(
        "Expression",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::NonTerminalExpression::instance("Atom"),
            PEG::Expressions::RepeatExpression::instance(PEG::Expressions::SequenceExpression::instance(
                PEG::Expressions::NonTerminalExpression::instance("Infix_Operator"), PEG::Expressions::NonTerminalExpression::instance("Atom")
            ))
        )})
    ));

    p_rules.insert(std::make_pair(
        "Atom",
        std::vector<PEG::ExpressionPtr>({
            PEG::Expressions::NonTerminalExpression::instance("Call"),
            PEG::Expressions::NonTerminalExpression::instance("WriteableExpression"),
            PEG::Expressions::SequenceExpression::instance(
                PEG::Expressions::NonTerminalExpression::instance("Prefix_Operator"), PEG::Expressions::NonTerminalExpression::instance("Atom")
            ),
            PEG::Expressions::SeqNExpression::instance(std::vector<PEG::ExpressionPtr>(
                {PEG::Expressions::TerminalExpression::instance("("), PEG::Expressions::NonTerminalExpression::instance("Expression"),
                 PEG::Expressions::TerminalExpression::instance("(")}
            )),
            PEG::Expressions::NonTerminalExpression::instance("Literal"),
            PEG::Expressions::NonTerminalExpression::instance("Quantifier"),
            PEG::Expressions::NonTerminalExpression::instance("Result"),
            PEG::Expressions::NonTerminalExpression::instance("AddressOf"),
            PEG::Expressions::NonTerminalExpression::instance("Label"),
        })
    ));

    p_rules.insert(std::make_pair(
        "Call", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SeqNExpression::instance(std::vector<PEG::ExpressionPtr>({
                    PEG::Expressions::NonTerminalExpression::instance("Identifier"),
                    PEG::Expressions::TerminalExpression::instance("("),
                    PEG::Expressions::NonTerminalExpression::instance("ExpressionList"),
                    PEG::Expressions::TerminalExpression::instance(")"),
                }))})
    ));

    p_rules.insert(std::make_pair(
        "ExpressionList",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SeqNExpression::instance(std::vector<PEG::ExpressionPtr>(
            {PEG::Expressions::NonTerminalExpression::instance("Expression"),
             PEG::Expressions::RepeatExpression::instance(PEG::Expressions::SequenceExpression::instance(
                 PEG::Expressions::TerminalExpression::instance(","), PEG::Expressions::NonTerminalExpression::instance("Expression")
             ))}
        ))})
    ));

    p_rules.insert(std::make_pair(
        "Index", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SeqNExpression::instance(std::vector<PEG::ExpressionPtr>({
                     PEG::Expressions::NonTerminalExpression::instance("Identifier"),
                     PEG::Expressions::TerminalExpression::instance("["),
                     PEG::Expressions::NonTerminalExpression::instance("IndexRange"),
                     PEG::Expressions::TerminalExpression::instance("]"),
                 }))})
    ));

    p_rules.insert(std::make_pair(
        "IndexRange",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::NonTerminalExpression::instance("Expression"),
            PEG::Expressions::OptionExpression::instance(PEG::Expressions::SequenceExpression::instance(
                PEG::Expressions::TerminalExpression::instance(".."), PEG::Expressions::NonTerminalExpression::instance("Expression")
            ))
        )})
    ));

    p_rules.insert(std::make_pair(
        "Quantifier",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SeqNExpression::instance(std::vector<PEG::ExpressionPtr>(
            {PEG::Expressions::NonTerminalExpression::instance("Quant"), PEG::Expressions::NonTerminalExpression::instance("VarList"),
             PEG::Expressions::TerminalExpression::instance("."), PEG::Expressions::NonTerminalExpression::instance("Expression")}
        ))})
    ));

    p_rules.insert(std::make_pair(
        "Quant", std::vector<PEG::ExpressionPtr>(
                     {PEG::Expressions::TerminalExpression::instance("forall"), PEG::Expressions::TerminalExpression::instance("exists")}
                 )
    ));

    p_rules.insert(std::make_pair(
        "VarList", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                       PEG::Expressions::NonTerminalExpression::instance("VarDecl"),
                       PEG::Expressions::RepeatExpression::instance(PEG::Expressions::SequenceExpression::instance(
                           PEG::Expressions::TerminalExpression::instance(","), PEG::Expressions::NonTerminalExpression::instance("VarDecl")
                       ))
                   )})
    ));

    p_rules.insert(std::make_pair(
        "VarDecl", std::vector<PEG::ExpressionPtr>({
                       PEG::Expressions::NonTerminalExpression::instance("Identifier"),
                       PEG::Expressions::NonTerminalExpression::instance("Identifier"),
                   })
    ));

    p_rules.insert(std::make_pair(
        "Literal", std::vector<PEG::ExpressionPtr>(
                       {PEG::Expressions::NonTerminalExpression::instance("Boolean"), PEG::Expressions::NonTerminalExpression::instance("Number"),
                        PEG::Expressions::NonTerminalExpression::instance("Builtin")}
                   )
    ));

    p_rules.insert(std::make_pair(
        "Boolean", std::vector<PEG::ExpressionPtr>(
                       {PEG::Expressions::TerminalExpression::instance("true"), PEG::Expressions::TerminalExpression::instance("false")}
                   )
    ));

    p_rules.insert(
        std::make_pair("Result", std::vector<PEG::ExpressionPtr>({PEG::Expressions::TerminalExpression::instance("result")}))
    );

    p_rules.insert(std::make_pair(
        "AddressOf",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::TerminalExpression::instance("&"), PEG::Expressions::NonTerminalExpression::instance("WriteableExpression")
        )})
    ));

    p_rules.insert(std::make_pair(
        "Label", std::vector<PEG::ExpressionPtr>({
                     PEG::Expressions::TerminalExpression::instance("@"),
                     PEG::Expressions::NonTerminalExpression::instance("Identifier"),
                 })
    ));

    p_rules.insert(std::make_pair(
        "WriteableExpression", std::vector<PEG::ExpressionPtr>({
                                   PEG::Expressions::NonTerminalExpression::instance("Index"),
                                   PEG::Expressions::NonTerminalExpression::instance("Identifier"),
                                   PEG::Expressions::NonTerminalExpression::instance("PointerDereference"),
                                   PEG::Expressions::NonTerminalExpression::instance("FieldReference"),
                               })
    ));

    p_rules.insert(std::make_pair(
        "PointerDereference",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::TerminalExpression::instance("*"), PEG::Expressions::NonTerminalExpression::instance("WriteableExpression")
        )})
    ));

    p_rules.insert(std::make_pair(
        "FieldReference",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SeqNExpression::instance(std::vector<PEG::ExpressionPtr>(
            {PEG::Expressions::TerminalExpression::instance("["), PEG::Expressions::NonTerminalExpression::instance("WriteableExpression"),
             PEG::Expressions::NonTerminalExpression::instance("]"), PEG::Expressions::NonTerminalExpression::instance("."),
             PEG::Expressions::NonTerminalExpression::instance("Identifier")}
        ))})
    ));

    p_rules.insert(std::make_pair(
        "WriteablExpressionList",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::NonTerminalExpression::instance("WriteableExpression"),
            PEG::Expressions::RepeatExpression::instance(PEG::Expressions::SequenceExpression::instance(
                PEG::Expressions::TerminalExpression::instance(","), PEG::Expressions::NonTerminalExpression::instance("WriteableExpression")
            ))
        )})
    ));

    p_rules.insert(std::make_pair(
        "LoopVariant",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::TerminalExpression::instance("variant"), PEG::Expressions::NonTerminalExpression::instance("Expression")
        )})
    ));

    p_rules.insert(std::make_pair(
        "LoopInvariant",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::TerminalExpression::instance("invariant"), PEG::Expressions::NonTerminalExpression::instance("Expression")
        )})
    ));

    p_rules.insert(std::make_pair(
        "Writes", std::vector<PEG::ExpressionPtr>(
                      {PEG::Expressions::SequenceExpression::instance(
                           PEG::Expressions::TerminalExpression::instance("writes"),
                           PEG::Expressions::NonTerminalExpression::instance("WriteableExpressionList")
                       ),
                       PEG::Expressions::TerminalExpression::instance("no_write")}
                  )
    ));

    p_rules.insert(std::make_pair(
        "Requires",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
            PEG::Expressions::TerminalExpression::instance("requires"), PEG::Expressions::NonTerminalExpression::instance("Expression")
        )})
    ));

    p_rules.insert(std::make_pair(
        "Ensures", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                       PEG::Expressions::TerminalExpression::instance("ensures"), PEG::Expressions::NonTerminalExpression::instance("Expression")
                   )})
    ));

    p_rules.insert(std::make_pair(
        "Assert", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                      PEG::Expressions::TerminalExpression::instance("assert"), PEG::Expressions::NonTerminalExpression::instance("Expression")
                  )})
    ));

    p_rules.insert(std::make_pair(
        "Identifier", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SeqNExpression::instance(std::vector<PEG::ExpressionPtr>({
                          PEG::Expressions::NotExpression::instance(PEG::Expressions::NonTerminalExpression::instance("Keyword")),
                          PEG::Expressions::NotExpression::instance(PEG::Expressions::NonTerminalExpression::instance("Builtin")),
                          PEG::Expressions::NonTerminalExpression::instance("Name"),
                      }))})
    ));

    p_rules.insert(std::make_pair(
        "Keyword", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                       PEG::Expressions::NonTerminalExpression::instance("Keyword_Suffix"),
                       PEG::Expressions::NotExpression::instance(PEG::Expressions::NonTerminalExpression::instance("Name"))
                   )})
    ));

    p_rules.insert(std::make_pair(
        "Keyword_Suffix",
        std::vector<PEG::ExpressionPtr>(
            {PEG::Expressions::TerminalExpression::instance("true"), PEG::Expressions::TerminalExpression::instance("false"),
             PEG::Expressions::TerminalExpression::instance("result"), PEG::Expressions::TerminalExpression::instance("exists"),
             PEG::Expressions::TerminalExpression::instance("forall"), PEG::Expressions::TerminalExpression::instance("variant"),
             PEG::Expressions::TerminalExpression::instance("invariant"), PEG::Expressions::TerminalExpression::instance("writes"),
             PEG::Expressions::TerminalExpression::instance("requires"), PEG::Expressions::TerminalExpression::instance("ensures")}
        )
    ));

    p_rules.insert(std::make_pair(
        "Builtin",
        std::vector<PEG::ExpressionPtr>(
            {PEG::Expressions::TerminalExpression::instance("min_sint8"), PEG::Expressions::TerminalExpression::instance("max_sint8"),
             PEG::Expressions::TerminalExpression::instance("min_uint8"), PEG::Expressions::TerminalExpression::instance("max_uint8"),
             PEG::Expressions::TerminalExpression::instance("min_sint16"), PEG::Expressions::TerminalExpression::instance("max_sint16"),
             PEG::Expressions::TerminalExpression::instance("min_uint16"), PEG::Expressions::TerminalExpression::instance("max_uint16"),
             PEG::Expressions::TerminalExpression::instance("min_sint32"), PEG::Expressions::TerminalExpression::instance("max_sint32"),
             PEG::Expressions::TerminalExpression::instance("min_uint32"), PEG::Expressions::TerminalExpression::instance("max_uint32"),
             PEG::Expressions::TerminalExpression::instance("min_sint64"), PEG::Expressions::TerminalExpression::instance("max_sint64"),
             PEG::Expressions::TerminalExpression::instance("min_uint64"), PEG::Expressions::TerminalExpression::instance("max_uint64")}
        )
    ));

    const auto alphabets_A_Z = peg_non_terminal_from_alphabets(p_rules, 'A', 'Z', value_factory_interface);
    const auto alphabets_a_z = peg_non_terminal_from_alphabets(p_rules, 'a', 'z', value_factory_interface);
    // const auto AZaz_;
    p_rules.insert(std::make_pair(
        "AZaz_", std::vector<PEG::ExpressionPtr>({alphabets_a_z, alphabets_A_Z, PEG::Expressions::TerminalExpression::instance("_")})
    ));
    const auto digits_0_9 = peg_sequence_from_digits(p_rules, '0', '9', value_factory_interface);
    // const auto AZaz09_;
    p_rules.insert(std::make_pair(
        "AZaz09_", std::vector<PEG::ExpressionPtr>(
                       {alphabets_a_z, alphabets_A_Z, digits_0_9, PEG::Expressions::TerminalExpression::instance("_")}
                   )
    ));

    p_rules.insert(std::make_pair(
        "Name", std::vector<PEG::ExpressionPtr>({PEG::Expressions::SequenceExpression::instance(
                    PEG::Expressions::NonTerminalExpression::instance("AZaz_"),
                    PEG::Expressions::RepeatExpression::instance(PEG::Expressions::NonTerminalExpression::instance("AZaz09_"))
                )})
    ));

    p_rules.insert(
        std::make_pair("Number", std::vector<PEG::ExpressionPtr>({PEG::Expressions::PlusExpression::instance(digits_0_9)}))
    );

    p_rules.insert(std::make_pair(
        "Prefix_Operator",
        std::vector<PEG::ExpressionPtr>({PEG::Expressions::TerminalExpression::instance("!"), PEG::Expressions::TerminalExpression::instance("-")}
        )
    ));

    p_rules.insert(std::make_pair(
        "Infix_Operator", std::vector<PEG::ExpressionPtr>({
                              PEG::Expressions::TerminalExpression::instance("->"),  PEG::Expressions::TerminalExpression::instance("<->"),
                              PEG::Expressions::TerminalExpression::instance(">="),  PEG::Expressions::TerminalExpression::instance("<="),
                              PEG::Expressions::TerminalExpression::instance(">>"),  PEG::Expressions::TerminalExpression::instance("<<"),
                              PEG::Expressions::TerminalExpression::instance("\\/"), PEG::Expressions::TerminalExpression::instance("/\\"),
                              PEG::Expressions::TerminalExpression::instance("=="),  PEG::Expressions::TerminalExpression::instance("!="),
                              PEG::Expressions::TerminalExpression::instance("-"),   PEG::Expressions::TerminalExpression::instance("+"),
                              PEG::Expressions::TerminalExpression::instance("/"),   PEG::Expressions::TerminalExpression::instance(">"),
                              PEG::Expressions::TerminalExpression::instance("<"),   PEG::Expressions::TerminalExpression::instance("*"),
                              PEG::Expressions::TerminalExpression::instance("^"),   PEG::Expressions::TerminalExpression::instance("|"),
                              PEG::Expressions::TerminalExpression::instance("&"),
                          })
    ));

    ;

    return PEG::Grammar::instance(p_vc, p_rules);
}

int
main() {
    std::cout << "hello, world to test pragma!" << std::endl;

    const auto plain_parse_result_factory = PEG::ParseTree::PlainParseTreeFactory::instance<PEG::ParseTree::PlainParseTreeFactory>();

    auto parser = PEG::Parser(make_grammar());
    // const std::string input = "requires{a.addr<>b.addr/\\tape[a.addr]<>b.addr}";
    const std::string input;
    const auto parseResult = parser.parse(input);

    std::cout << "result: ";
    std::cout << "\t isSuccess :" << parseResult->isSuccess();
    std::cout << "\n\t isFullSuccess :" << parseResult->isFullSuccess();
    std::cout << "\n\t parseTree :";
    // PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(parseResult->valueOutput)->print(0);
    std::cout << "\n\t remainder : " << parseResult->position;
    // parseResult->output();
    std::cout << "\n\t input : " << input;

    // assert(parseResult->isFullSuccess());
    return !(!parseResult->isSuccess() && !parseResult->isFullSuccess());
}