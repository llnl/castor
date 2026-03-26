// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/Expressions/EmptyExpression.h>
#include <PEG/Grammar.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/Parser.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <PEG/Value/ValueInterface.h>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <unordered_map>

class NumLift final: public PEG::ValueFactoryInterface {
public:
    PEG::ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<PEG::ValueInterfacePtr> &children
    ) override {
        return children[0];
    }
};

int
main(int argc, char *argv[]) {

    auto verbose = false;
    auto has_grammar = false;
    std::string grammar_file;
    auto has_input = false;
    std::string input_file;
    while (true) {
        static struct option long_options[] = {
            {"verbose", no_argument, nullptr, 'v'},
            {"grammar", required_argument, nullptr, 'g'},
            {"input", required_argument, nullptr, 'i'},
            {nullptr, 0, nullptr, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "g:i:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'v':
            std::cout << "option -v / --verbose \n";
            verbose = true;
            break;
        case 'g':
            std::cout << "option -g / --grammar with value " << optarg << " \n";
            if (!has_grammar) {
                grammar_file = optarg;
                has_grammar = true;
            }
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

    if (!has_grammar) {
        std::cout << " no grammar? " << std::endl;
        return -1;
    }

    std::ifstream input_file_stream(grammar_file);
    if (input_file_stream.fail()) {
        std::cerr << "Error opening grammar file " << grammar_file << std::endl;
        return -1;
    }
    std::string input_grammar;
    input_grammar.assign(std::istreambuf_iterator<char>(input_file_stream), std::istreambuf_iterator<char>());

    std::cout << " grammar file " << grammar_file << std::endl;

    std::string input;
    if (!has_input) {
        std::cout << " no input ? that's ok! " << std::endl;
    } else {
        std::ifstream input_stream(input_file);
        if (input_stream.fail()) {
            std::cerr << "Error opening input file " << input_file << std::endl;
            return -1;
        }
        input.assign(std::istreambuf_iterator<char>(input_stream), std::istreambuf_iterator<char>());
        std::cout << " input file " << input_file << std::endl;
    }

    auto calc_action_map = std::unordered_map<std::string, PEG::ValueFactoryInterfacePtr>();
    const auto parser = PEG::Parser::from_str(input_grammar, calc_action_map);

    std::cout << parser->grammar->check_grammar();

    if (has_input) {
        const auto result = parser->parse(input);
        std::ostringstream output_stream;
        auto tree = PEG::ValueInterface::downcast<PEG::ParseTree::PlainParseTree>(result->valueOutput);
        tree->printJSON(output_stream);
        std::ofstream output_file(input_file + ".out.json");
        output_file << output_stream.str();
        output_file.close();
        std::cout << output_stream.str() << std::endl;
    } else {
        parser->grammar->print();
    }

    return 0;
}