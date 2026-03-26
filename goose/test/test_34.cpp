// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <fstream>
#include <iostream>
#include <string>

int
main() {

    // TODO: deduce a way to read input_grammar from a file
    // TODO: such that it works with CMake/CLion, and across MacOS\Clang++ & GNU/Linux\G++

    const std::string input_grammar_template = R"N(
Quantifier <- Quant VarList '.' Expression
Quant      <- 'forall' / 'exists'
VarList    <- VarDecl (',' VarDecl)*

Expression <- Atom (Infix_Operator Atom)*

Atom       <- Call
    / Prefix_Operator Atom
    / '(' Expression ')'
    / Quantifier
    / Identifier

Call  <- Identifier '(' ExpressionList ')'
ExpressionList <- Expression (',' Expression)*

Identifier <- !Keyword Name
Keyword    <- 'true' / 'false' / 'result' / 'exists' / 'forall'
    / 'variant' / 'invariant' / 'writes' / 'requires' / 'ensures' / Builtin
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

    const auto input_grammar = input_grammar_template + R"N(
#VarDecl    <- Identifier Identifier
VarDecl    <- Identifier ':' Identifier
    )N";

    const auto parser = PEG::Parser::from_str(input_grammar);

    parser->grammar->print();

    const std::string input = "forall sint64: i. to_sint64(i) == i";
    const auto result = parser->parse(input);

    result->output();
    if(result->isSuccess()) {
        PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput)->print(0);
    }
    if(!result->isFullSuccess()) {
        std::cout << "remainder: " << input.substr(result->position) << std::endl;
    }
    std::cout << result->isSuccess() << " && " << result->isFullSuccess() << std::endl;
    std::cout << input << std::endl;
    return result->isFullSuccess() == true ? 0 : 1;
}