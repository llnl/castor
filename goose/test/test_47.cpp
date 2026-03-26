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
    const std::string input_grammar = R"N(
json <- element
value <- object
       / array
       / string
       / number
       / "true"
       / "false"
       / "null"

object <- '{' members? '}'
members <- member (',' members)*
member <- string ':' element

array <- '[' elements? ']'
elements <- element (',' elements)*
element <- value

string <- ["] character* ["]
character <- !character_extra . / '\\' escape
character_extra <- ["] / [\\]
escape <- ["] / '\\' / '/' / 'b' / 'f' / 'n' / 'r' / 't' / 'u' hex hex hex hex
hex <- digit / [A-F] / [a-f]

number <- '-'? integer fraction? exponent?

integer <- [1-9] digits / digit
digit <- '0' / [1-9]
digits <- digit+

fraction <- '.' digits
exponent <- 'E' sign? digits / 'e' sign? digits
sign <- '+' / '-'

#ws <- "" / ' ' / '\n' / '\r' / '\t'
)N";

    const auto parser = PEG::Parser::from_str(input_grammar);

    // empty the white_space rules since the parser already defines spacing
    parser->grammar->print();

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

    const auto result = parser->parse(input);

    result->output();
    if(!result->isFullSuccess()) {
        std::cout << "remainder: " << input.substr(result->position) << std::endl;
    }
    std::cout << result->isSuccess() << " && " << result->isFullSuccess() << std::endl;
    // std::cout << input << std::endl;
    return result->isFullSuccess() == true ? 0 : 1;
}