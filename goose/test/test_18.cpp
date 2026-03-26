// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Bootstrap/BootstrapParser.h>
#include <PEG/Expressions/EmptyExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <PEG/Value/ValueInterface.h>
#include <cassert>
#include <iostream>
#include <ostream>
#include <unordered_map>

int
main() {
    const auto bootstrap_parser = PEG::Bootstrap::BootstrapParser("", std::unordered_map<std::string, PEG::ValueFactoryInterfacePtr>());
    const auto grammar = bootstrap_parser.peg_grammar;
    grammar->gram.rules[grammar->white_space] = std::vector<PEG::ExpressionPtr>();
    grammar->doLeftRecursion = false;
    auto parser = PEG::Parser(grammar);

    const std::string input = R"N(
    # Hierarchical syntax
    Grammar <- Spacing Definition+ EndOfFile
    Definition <- Identifier LEFTARROW Expression

    Expression <- Sequence (SLASH Sequence)*
    Sequence <- Prefix+ # TODO
    Prefix <- (Prefix_Extra)? Suffix
    Prefix_Extra <- AND / NOT
    Suffix <- Primary (Suffix0)?
    Suffix0 <- QUESTION / STAR / PLUS

    Primary <- Identifier !LEFTARROW
    Primary <- OPEN Expression CLOSE
    Primary <- Literal
    Primary <- Class
    Primary <- DOT

    Identifier <- IdentStart IdentCont* Spacing
    IdentStart <- [a-zA-Z_]
    IdentCont <- IdentStart
             / [0-9]

    Literal <- ['] (!['] Char)+ ['] Spacing
            /   ["] (!["] Char)+ ["] Spacing
    Class <- '[' (!'[' Range)+ ']' Spacing
    Range <- Char '-' Char / Char
    Char <- '\\' [nrt'"\[\]\\]
    #        / '\\' [0-2][0-7][0-7]
    #        / '\\' [0-7][0-7]?
            / !'\\' .

    LEFTARROW <- '<-' Spacing
    SLASH <- '/' Spacing
    AND <- '&' Spacing
    NOT <- '!' Spacing
    QUESTION <- '?' Spacing
    STAR <- '*' Spacing
    PLUS <- "+" Spacing
    OPEN <- '(' Spacing
    CLOSE <- ')' Spacing
    DOT <- '.' Spacing

    Spacing <- (Spacing_Extra)*
    Spacing_Extra <- Space / Comment
    Space <- ' ' / '\t' / EndOfLine
    Comment <- '#' (!EndOfLine .)* EndOfLine
    EndOfLine <- '\r\n' / '\n' / '\r'
    EndOfFile <- !.
    )N";
    grammar->print();

    if (grammar->check_grammar()) {
        std::cout << "grammar is good";
    } else {
        std::cout << "grammar is not good";
    }

    std::cout << "input size: " << input.size() << std::endl;
    const auto lift = PEG::ValueFactoryInterface::instance<PEG::Bootstrap::Lift>();
    const auto result_0 = parser.parse(input, lift);
    // TODO: should memo table be cleared
    const auto new_grammar = PEG::ValueInterface::downcast<PEG::Bootstrap::GrammarValue>(result_0->valueOutput);

    // TODO: handle new_grammar::check_grammar() failing
    //  replace Spacing rule above with #Spacing <- (Space / Comment)*
    std::cout << "output parse tree: ";
    new_grammar->print(0);
    std::cout << std::endl;
    std::cout << "output is a grammar value with following output: ";
    new_grammar->grammar->print();
    std::cout << "\n result_0: " << result_0->isFullSuccess() << std::endl;

    assert(result_0->isFullSuccess());

    return 0;
}