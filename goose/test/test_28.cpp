// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/Parser.h>
#include <iostream>
#include <string>

int
main() {
    const std::string input_grammar = R"N(
        Prefix <- [!-]
        E <- N '+' N / Prefix E
        N <- 'n'
    )N";

    const auto parser = PEG::Parser::from_str(input_grammar);

    parser->grammar->print();

    return 0;
}