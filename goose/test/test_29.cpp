// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/Parser.h>
#include <iostream>
#include <string>

int
main() {
    /*
     *  Test case as a reminder that comments should end in a newline
     *  This usually means that comments can't be put at the end of the file
     */
    const std::string input_grammar_template = R"N(
VC     <- 'LoopInvariant / LoopVariant / Writes / Requires / Ensures / Assert / Assume / Frees'

#eeeeeeee)N";

    // the above grammar will fail to parse but the following change will make it parse
    // i.e., PEG::Parser::from_str(input_grammar_template) will crash
    // while PEG::Parser::from_str(input_grammar) will succeed fully

    const std::string input_grammar = input_grammar_template + '\n';

    const auto parser = PEG::Parser::from_str(input_grammar);

    parser->grammar->print();

    return 0;
}