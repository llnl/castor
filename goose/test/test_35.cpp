// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "PEG/Bootstrap/BootstrapParser.h"

#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <fstream>
#include <iostream>
#include <string>

int
main() {
    const std::string input_grammar_template = R"N(
Expression <- Atom (Infix_Operator Atom)*

Atom       <-
      WriteableExpression
    / Prefix_Operator Atom
    / '(' Expression ')'
    / Quantifier
    / Literal

Quantifier <- Quant VarList '.' Expression
Quant      <- 'forall' / 'exists'
VarList    <- VarDecl (',' VarDecl)*
VarDecl    <- Identifier ":" Identifier

Literal <- Boolean / Number / Builtin
Boolean <- 'true' / 'false'
Number <- [0-9]+

WriteableExpression <- Index
    / PointerDereference
    / FieldReference
    / FieldArrowReference
    / Identifier
PointerDereference  <- '*' WriteableExpression
FieldReference      <- WriteableExpression '.' Identifier
FieldArrowReference      <- WriteableExpression '->' Identifier

Index      <- WriteableExpression '[' IndexRange ']'
IndexRange <- Expression ('..' Expression)?

Identifier <- !Keyword !Builtin Name
Keyword_Reserved <-
    'true' / 'false' / 'result' / 'exists'
    / 'forall' / 'variant' / 'invariant'
    / 'writes' / 'requires' / 'ensures'
Builtin    <- 'min_sint8' / 'max_sint8' / 'min_uint8' / 'max_uint8'
    / 'min_sint16' / 'max_sint16' / 'min_uint16' / 'max_uint16'
    / 'min_sint32' / 'max_sint32' / 'min_uint32' / 'max_uint32'
    / 'min_sint64' / 'max_sint64' / 'min_uint64' / 'max_uint64'

Name   <- [A-Za-z_][A-Za-z0-9_]*

Prefix_Operator <- [!-]

Infix_Operator  <-
    '*' / '/' # Note: !'\' is not handled
    / '+'   / '-'
    / '^'   / '|' / '&'
    / '<<'  / '>>'
    / '>='  / '<=' / '>' / '<' / '==' / '!='
    / '\\/' / '/\\'
    / '->'  / '<->'
)N";

    const std::string input = R"N(forall sint64: i. true)N";

    const std::string not_not_grammar = input_grammar_template + R"N(
Keyword <- Keyword_Reserved !Name
    )N";

    const auto not_not_parser = PEG::Parser::from_str(not_not_grammar);
    not_not_parser->grammar->print();
    const auto not_not_result = not_not_parser->parse(input);
    not_not_result->output();
    if(not_not_result->isSuccess()) {
        PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(not_not_result->valueOutput)->print(0);
    }
    if(!not_not_result->isFullSuccess()) {
        std::cout << "remainder: \"" << input.substr(not_not_result->position) << "\"" << std::endl;
    }

    PEG::GrammarRuleValue::rules.clear();

    std::cout << std::endl << "------------------------------------" << std::endl;
    std::cout << std::endl << "MODIFIED grammar output" << std::endl;

    const std::string input_grammar = input_grammar_template + R"N(
Keyword    <- Keyword_Reserved
    )N";

    const auto parser = PEG::Parser::from_str(input_grammar);
    // parser->grammar.print();
    const auto result = parser->parse(input);
    result->output();
    if(result->isSuccess()) {
        PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput)->print(0);
    }
    if(!result->isFullSuccess()) {
        std::cout << "remainder: \"" << input.substr(result->position) << "\"" << std::endl;
    }

    std::cout << std::endl << "results: " << std::endl;
    std::cout << "not_not_grammar output" << std::endl;
    std::cout << not_not_result->isSuccess() << " && " << not_not_result->isFullSuccess() << std::endl;
    std::cout << "MODIFIED grammar output" << std::endl;
    std::cout << result->isSuccess() << " && " << result->isFullSuccess() << std::endl;
    std::cout << input << std::endl;

    return !(not_not_result->isFullSuccess()) && result->isFullSuccess() ? 0 : 1;
}