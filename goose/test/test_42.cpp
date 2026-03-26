// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "PEG/Value/ValueFactoryInterface.h"
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <string>

class FlattenName final: public PEG::ValueFactoryInterface {
public:
    PEG::ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<PEG::ValueInterfacePtr> &children
    ) override {
        // all parse outputs are ValueInterface objects
        // all parse outputs of meta-grammar are PlainParseTree objects, which is a subclass of ValueInterface
        // containing:
        //  label (type of Expression that was parsed: Seq, Repeat, Empty, Not),
        //  position (only used for Non-Terminals: specifies which alternation/choice was chosen, 0-indexed)
        //  value (meant to store output values of Terminal strings)
        //  children (a list of ValueInterface object that are embedded within this tree)

        // most rules are formed using a sequence of expressions, each expression's output being stored in the parent's children
        // For Example,
        //  Name   <- [A-Za-z_] [A-Za-z0-9_]*  @lift
        // we're concerned just with [A-Za-z_] [A-Za-z0-9_]*
        // which is represented as Sequence({ [A-Za-z_] , [A-Za-z0-9]* });
        // and therefore has an output tree:
        //  PlainParseTree(
        //      label // "Sequence"
        //      position // unused
        //      value // unused
        //      children // two objects, one for [A-Za-z_] (a single char), and one for the repeat/star
        //               // the latter is actually Repeat([A-Za-z0-9])
        //               // which can be Empty or contain a list of characters
        //  )

        // child_0 contains a single character
        // child_1 contains the rest of the characters, _if any_
        std::string out;

        const auto &child_0 = children[0];

        const auto &child_0_tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(child_0);

        out += child_0_tree->value;

        const auto &child_1 = children[1];
        const auto &child_1_tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(child_1);

        if(child_1_tree->label == "Empty") {
            return PEG::ParseTree::PlainParseTree::instance("Name", -1, out, {});
        }

        // Repeat contains more characters
        for(const auto &iter : child_1_tree->children) {
            const auto &iter_tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(iter);
            out += iter_tree->value;
        }
        return PEG::ParseTree::PlainParseTree::instance("Name", -1, out, {});
    }
};

class IDHandler final: public PEG::ValueFactoryInterface {
public:
    PEG::ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<PEG::ValueInterfacePtr> &children
    ) override {
        // Identifer <- !Keyword Name
        // Internally: Sequence( { Not(NonTerm(Keyword)), NonTerm(Name) })
        // Not Expressions do not consume any input and can be safely ignored
        // Name should contain the value as created by FlattenName above
        // NonTerm(Name).children[0]
        const auto &name = children[1];
        const auto &name_tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(name);
        return name_tree->children[0];
    }
};

int
main(int argc, char *argv[]) {
    auto verbose = false;
    auto has_output_file = false;
    std::string output_file;
    while (true) {
        static struct option long_options[] = {
            {"verbose", no_argument, nullptr, 'v'},
            {"output", required_argument, nullptr, 'o'},
            {nullptr, 0, nullptr, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "o:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'v':
            std::cout << "option -v / --verbose \n";
            verbose = true;
            break;
        case 'o':
            std::cout << "option -o / --output with value " << optarg << " \n";
            if (!has_output_file) {
                output_file = optarg;
                has_output_file = true;
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

    if (!has_output_file) {
        std::cout << " no output dir ? " << std::endl;
        return -1;
    }


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

Identifier <- !Keyword Name @idHandler
Keyword    <-
    'true' / 'false' / 'result' / 'exists'
    / 'forall' / 'variant' / 'invariant'
    / 'writes' / 'requires' / 'ensures' / Builtin
Builtin    <- 'min_sint8' / 'max_sint8' / 'min_uint8' / 'max_uint8'
    / 'min_sint16' / 'max_sint16' / 'min_uint16' / 'max_uint16'
    / 'min_sint32' / 'max_sint32' / 'min_uint32' / 'max_uint32'
    / 'min_sint64' / 'max_sint64' / 'min_uint64' / 'max_uint64'

Name   <- [A-Za-z_][A-Za-z0-9_]*  @nameHandler

Prefix_Operator <- [!-]

)N";

    auto action_map = std::unordered_map<std::string, PEG::ValueFactoryInterfacePtr>();
    action_map.insert({"nameHandler", std::make_shared<FlattenName>()});
    action_map.insert({"idHandler", std::make_shared<IDHandler>()});

    const auto parser = PEG::Parser::from_str(input_grammar, action_map);

    parser->grammar->print();

    // const std::string input = "assert !(forall max_uint8 i, max_sint8 j. i < j)";
    // const std::string input = "asserts forall sint64: i. to_sint64(i) == i";
    const std::string input = "assert forall sint32: i. to_sint32(i) == i";
    const auto result = parser->parse(input);

    result->output();
    if(result->isSuccess()) {
        const auto cwd = output_file + ".tree.json";
        std::ofstream output_file_stream(cwd);

        auto tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput);
        std::ostringstream output_stream;
        tree->printJSON(output_stream);

        output_file_stream << output_stream.str();
        output_file_stream.close();
        // std::cout << output_stream.str() << std::endl;
        PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput)->print(0);
    }

    if(!result->isFullSuccess()) {
    std::cout << "remainder: " << input.substr(result->position) << std::endl;
    }
    std::cout << result->isSuccess() << " && " << result->isFullSuccess() << std::endl;
    std::cout << input << std::endl;
    return result->isFullSuccess() == true ? 0 : 1;
}