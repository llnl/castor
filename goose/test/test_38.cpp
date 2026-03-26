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
    // verification-condition grammar that does not properly handle precedence
    const std::string input_grammar = R"N(
VC     <- LoopInvariant
    / LoopVariant
    / Writes
    / Requires
    / Ensures
    / Assert
    / Assume
    / Frees

LoopVariant   <- 'variant' Expression
LoopInvariant <- 'invariant' Expression
Writes        <- 'writes' WriteableExpressionList / 'no_write'
Requires      <- 'requires' Expression
Ensures       <- 'ensures' Expression
Assert        <- 'assert' Expression
Assume        <- 'assume' Expression
Frees         <- 'frees' WriteableExpressionList / 'no_free'

WriteableExpressionList <- WriteableExpression (',' WriteableExpression)*

# Expression <- Infix_Expression(Atom, Infix_Operator)

Expression <- Atom (Infix_Operator Atom)*

# Expression <- Expression^2 '+' Expression^3

Atom       <- Call
    / WriteableExpression
    / Prefix_Operator Atom
    / '(' Expression ')'
    / Literal
    / Quantifier
    / Result
    / AddressOf
    / Label

Call  <- Identifier '(' ExpressionList ')'
ExpressionList <- Expression (',' Expression)*

Quantifier <- Quant VarList '.' Expression
Quant      <- 'forall' / 'exists'
VarList    <- VarDecl (',' VarDecl)*
VarDecl    <- Identifier ':' Identifier

Literal <- Boolean / Number / Builtin
Boolean <- 'true' / 'false'
Number <- [0-9]+

Result <- 'result'

AddressOf <- '&' WriteableExpression

Label <- '@' Identifier

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

Identifier <- !Keyword Name
Keyword    <-
    'true' / 'false' / 'result' / 'exists'
    / 'forall' / 'variant' / 'invariant'
    / 'writes' / 'requires' / 'ensures' / Builtin
Builtin    <- 'min_sint8' / 'max_sint8' / 'min_uint8' / 'max_uint8'
    / 'min_sint16' / 'max_sint16' / 'min_uint16' / 'max_uint16'
    / 'min_sint32' / 'max_sint32' / 'min_uint32' / 'max_uint32'
    / 'min_sint64' / 'max_sint64' / 'min_uint64' / 'max_uint64'

Name   <- [A-Za-z_][A-Za-z0-9_]*

Prefix_Operator <- [!-]

Infix_Operator  <-
    '*' / '/' !'\\'
    / '+'   / '-'
    / '^'   / '|' / '&'
    / '<<'  / '>>'
    / '>='  / '<=' / '>' / '<' !'->' / '==' / '!='
    / '\\/' / '/\\'
    / '=>'  / '<->'

#Infix_Expression(A, O) <- A (O A)* {
#  precedence
#    R -> <->
#    L /\ \/
#    L >= <= > < == !=
#    L << >>
#    L ^ | &
#    L + -
#    L * /
#}
)N";

    const auto parser = PEG::Parser::from_str(input_grammar);

    parser->grammar->print();

    const auto answer = parser->grammar->find_all_left_recursion();

    std::cout << "left recursive terminals: {";
    for (const auto &i : answer) {
        std::cout << i << ", ";
    }
    std::cout << "}" << std::endl;

    // const std::string input = "asserts forall sint64: i. to_sint64(i) == i";
    const std::string input = "assert forall sint64: i. to_sint64(i) == i";
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