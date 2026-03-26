// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/ParseTree/PlainParseTree.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void
PEG::ParseTree::PlainParseTree::indentation(const int indent) {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
}

void
PEG::ParseTree::PlainParseTree::print(const int indent) const {
    // TODO: have enums for the labels and then switch case/pattern match
    //  different types (terminal, non-term, seq, etc) instead of
    //  butchering/scope-creeping this print to accommodating everything
    indentation(indent);

    std::cout << label;
    if (position != -1) {
        std::cout << " " + value + "/" << position;
    }
    std::cout << " [";
    if (!children.empty()) {
        for (const auto &iter : children) {
            std::cout << std::endl;
            if (iter == nullptr) {
                continue;
            }
            downcast<PlainParseTree>(iter)->print(indent + 1);
        }

        indentation(indent);
    } else {
        std::cout << value;
    }
    std::cout << "]" << std::endl;
}

void
PEG::ParseTree::PlainParseTree::printJSON(std::ostringstream &printout) const {
    // label, position, value, children
    printout << '{';
    printout << "\"label\":" << "\"" << label << "\"";
    printout << ",";
    printout << "\"position\":" << position;
    printout << ",";
    if (value == "/\\") {
        printout << "\"value\":" << "\"" << "/\\\\" << "\"";
    } else if (value == "\\/") {
        printout << "\"value\":" << "\"" << "\\\\/" << "\"";
    } else {
        printout << "\"value\":" << "\"" << value << "\"";
    }
    printout << ",";
    printout << "\"children\":[";
    for (const auto &iterator : children) {
        downcast<PlainParseTree>(iterator)->printJSON(printout);
        if (&iterator != &children.back()) {
            printout << ',';
        }
    }
    printout << ']';
    printout << '}';
}

void
PEG::ParseTree::PlainParseTree::print() const {
    std::ostringstream printout;
    printJSON(printout);
    std::cout << printout.str();
}

PEG::ValueInterfacePtr
PEG::ParseTree::PlainParseTree::instance(
    const std::string &label, const int &position, const std::string &value,
    const std::vector<ValueInterfacePtr> &children
) {
    return std::make_shared<PlainParseTree>(label, position, value, children);
}