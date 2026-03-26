// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <string>

int
main(int argc, char *argv[]) {
    // verification-condition grammar that does properly handle precedence
    const std::string input_grammar = R"N(
VC     <- LoopInvariant
    / LoopVariant
    / Writes
    / Requires
    / Ensures
    / Assert
    / Assume
    / Frees

LoopVariant   <- 'variant ' Expression
LoopInvariant <- 'invariant ' Expression
Writes        <- 'writes ' WriteableExpressionList / 'no_write'
Requires      <- 'requires ' Expression
Ensures       <- 'ensures ' Expression
Assert        <- 'assert ' Expression
Assume        <- 'assume ' Expression
Frees         <- 'frees ' WriteableExpressionList / 'no_free'

WriteableExpressionList <- WriteableExpression (',' WriteableExpression)*

Expression <-
      Expression^2 '=>' Expression^2
    / Expression^2 '<->' Expression^2

    / Expression^2 '/\\' Expression^3
    / Expression^2 '\\/' Expression^3

    / Expression^3 '>=' Expression^4
    / Expression^3 '<=' Expression^4
    / Expression^3 '>' Expression^4
    / Expression^3 '<' !'->' Expression^4
    / Expression^3 '==' Expression^4
    / Expression^3 '!=' Expression^4

    / Expression^4 '>>' Expression^5
    / Expression^4 '<<' Expression^5

    / Expression^5 '^' Expression^6
    / Expression^5 '|' Expression^6
    / Expression^5 '&' Expression^6

    / Expression^6 '+' Expression^7
    / Expression^6 '-' Expression^7

    / Expression^7 '*' Expression^8
    / Expression^7 '/' !'\\' Expression^8

    / Atom

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

)N";

    const auto parser = PEG::Parser::from_str(input_grammar);

    // parser->grammar.print();

    const auto answer = parser->grammar->find_all_left_recursion();

    auto verbose = false;
    auto has_input = false;
    std::string input_file;
    while (true) {
        static struct option long_options[] = {
            {"verbose", no_argument, nullptr, 'v'},
            {"input", required_argument, nullptr, 'i'},
            {nullptr, 0, nullptr, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "i:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'v':
            std::cout << "option -v / --verbose \n";
            verbose = true;
            break;
        case 'i':
            std::cout << "option -i / --input with value " << optarg << " \n";
            if (!has_input) {
                input_file = optarg;
                has_input = true;
            }
            break;
        case '?':
            /* get opt long already printed an error */
                return 0;
        default:
            return -1;
        }
    }

    if (optind < argc) {
        std::cout << "non option argv elements";
        while (optind < argc) {
            std::cout << argv[optind++];
        }
        std::cout << std::endl;
    }

    if (verbose) {
        std::cout << " running in verbose mode " << std::endl;
    }

    std::string input;
    if (!has_input) {
        std::cout << " no input ? " << std::endl;
        return -1;
    }

    std::ifstream input_stream(input_file);
    if (input_stream.fail()) {
        std::cerr << "Error opening input file " << input_file << std::endl;
        return -1;
    }
    input.assign(std::istreambuf_iterator<char>(input_stream), std::istreambuf_iterator<char>());
    std::cout << " input file " << input_file << std::endl;

    // const std::string input = "assert !(forall max_uint8 i, max_sint8 j. i < j)";
    // const std::string input = "asserts forall sint64: i. to_sint64(i) == i";
    // const std::string input = "assert forall sint32: i. to_sint32(i) == i";
    const auto result = parser->parse(input);

    result->output();
    if(result->isSuccess()) {
        std::ostringstream output_stream;
        auto tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput);
        tree->printJSON(output_stream);
        std::ofstream output_file(input_file + ".tree.json");
        output_file << output_stream.str();
        output_file.close();
        std::cout << output_stream.str() << std::endl;
    }
    if(!result->isFullSuccess()) {
    std::cout << "remainder: " << input.substr(result->position) << std::endl;
    }
    std::cout << result->isSuccess() << " && " << result->isFullSuccess() << std::endl;
    std::cout << input << std::endl;
    return result->isFullSuccess() == true ? 0 : 1;
}