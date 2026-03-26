// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/ParseTree/PlainParseTree.h>
#include <PEG/ParseTree/PlainParseTreeFactory.h>
#include <vector>

PEG::ValueInterfacePtr
PEG::ParseTree::PlainParseTreeFactory::createParseTree(
    const std::string &label, const int &position, const std::string &value,
    const std::vector<ValueInterfacePtr> &children
) {
    return PlainParseTree::instance(label, position, value, children);
}