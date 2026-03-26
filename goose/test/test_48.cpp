// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "PEG/Expressions/DotExpression.h"
#include "PEG/Expressions/NotExpression.h"
#include "PEG/Expressions/OptionExpression.h"
#include "PEG/Expressions/PlusExpression.h"
#include "PEG/Expressions/RepeatExpression.h"
#include "PEG/Expressions/SeqNExpression.h"
#include <PEG/Expressions/EmptyExpression.h>
#include <PEG/Expressions/NonTerminalExpression.h>
#include <PEG/Expressions/SequenceExpression.h>
#include <PEG/Expressions/TerminalExpression.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <string>

PEG::ExpressionPtr seq(PEG::ExpressionPtr expr1, PEG::ExpressionPtr expr2) {
    return PEG::Expressions::SequenceExpression::instance(std::move(expr1), std::move(expr2));
}
PEG::ExpressionPtr non_term(const std::string &nt) {
    return PEG::Expressions::NonTerminalExpression::instance(nt);
}
PEG::ExpressionPtr term(const std::string &t) {
    return PEG::Expressions::TerminalExpression::instance(t);
}
PEG::ExpressionPtr empty() {
    return PEG::Expressions::EmptyExpression::instance();
}
PEG::ExpressionPtr seq_n(const std::vector<PEG::ExpressionPtr> &list_expr) {
    return PEG::Expressions::SeqNExpression::instance(list_expr);
}

int
main(int argc, char *argv[]) {
    const auto input_grammar = PEG::Grammar::instance(
        "json",
        {
            {"json", {non_term("element")}},
            {"value",
             {
                 non_term("object"),
                 non_term("array"),
                 non_term("string"),
                 non_term("number"),
                 term("true"),
                 term("false"),
                 term("null"),
             }},
            {"object",
             {seq_n({term("{"), PEG::Expressions::OptionalExpression::instance(non_term("members")), term("}")})}},
            {"members",
             {seq(non_term("member"), PEG::Expressions::RepeatExpression::instance(seq(term(","), non_term("members"))))}
            },
            {"member", {seq_n({non_term("string"), term(":"), non_term("element")})}},
            {"array",
             {seq_n({term("["), PEG::Expressions::OptionalExpression::instance(non_term("elements")), term("]")})}},
            {"elements",
             {seq(
                 non_term("element"), PEG::Expressions::RepeatExpression::instance(seq(term(","), non_term("elements")))
             )}},
            {"element", {non_term("value")}},
            {"string",
             {seq_n({term("\""), PEG::Expressions::RepeatExpression::instance(non_term("character")), term("\"")})}},
            {"character",
             {seq_n(
                  {PEG::Expressions::NotExpression::instance(non_term("char_extra")),
                   PEG::Expressions::DotExpression::instance()}
              ),
              seq(term("\\"), non_term("escape"))}},
            {"char_extra", {term("\""), term("\\")}},
            {"escape",
             {
                 term("\""),
                 term("\\"),
                 term("/"),
                 term("b"),
                 term("f"),
                 term("n"),
                 term("r"),
                 term("t"),
                 seq_n({term("u"), non_term("hex"), non_term("hex"), non_term("hex"), non_term("hex")}),
             }},
            {"hex",
             {
                 non_term("digit"),
                 term("A"),
                 term("B"),
                 term("C"),
                 term("D"),
                 term("E"),
                 term("F"),
                 term("a"),
                 term("b"),
                 term("c"),
                 term("d"),
                 term("e"),
                 term("f"),
             }},
            {"number",
             {seq_n(
                 {non_term("integer"), PEG::Expressions::OptionalExpression::instance(non_term("fraction")),
                  PEG::Expressions::OptionalExpression::instance(non_term("exponent"))}
             )}},
            {"integer",
             {seq_n({
                  PEG::Expressions::OptionalExpression::instance(term("-")),
                  non_term("onenine"),
                  PEG::Expressions::PlusExpression::instance(non_term("digit")),
              }),
              seq(PEG::Expressions::OptionalExpression::instance(term("-")), non_term("digit"))}},
            {"digit", {term("0"), non_term("onenine")}},
            {"onenine",
             {
                 term("1"),
                 term("2"),
                 term("3"),
                 term("4"),
                 term("5"),
                 term("6"),
                 term("7"),
                 term("8"),
                 term("9"),
             }},
            {"fraction", {seq(term("."), PEG::Expressions::PlusExpression::instance(non_term("digit")))}},
            {"exponent",
             {seq_n(
                  {term("E"), PEG::Expressions::OptionalExpression::instance(non_term("sign")),
                   PEG::Expressions::PlusExpression::instance(non_term("digit"))}
              ),
              seq_n(
                  {term("e"), PEG::Expressions::OptionalExpression::instance(non_term("sign")),
                   PEG::Expressions::PlusExpression::instance(non_term("digit"))}
              )}},
            {"sign", {term("+"), term("-")}},
            // {"ws", {term(" "), term("\n"), term("\r"), term("\t")}},
        }
    );

    auto parser = PEG::Parser(input_grammar);

    // parser.grammar->print();

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

    if (!has_input) {
        std::cout << " no input ? " << std::endl;
        return -1;
    }

    std::string input;
    std::ifstream input_stream(input_file);
    if (input_stream.fail()) {
        std::cerr << "Error opening input file " << input_file << std::endl;
        return -1;
    }
    input.assign(std::istreambuf_iterator<char>(input_stream), std::istreambuf_iterator<char>());

    const auto result = parser.parse(input);

    result->output();
    if(!result->isFullSuccess()) {
    std::cout << "remainder: " << input.substr(result->position) << std::endl;
    }
    std::cout << result->isSuccess() << " && " << result->isFullSuccess() << std::endl;
    return result->isFullSuccess() == true ? 0 : 1;
}