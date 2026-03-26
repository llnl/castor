// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_PlainParseTreeFactory_H
#define PEG_PlainParseTreeFactory_H

#include <PEG/BasicTypes.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <vector>

namespace PEG {

/// Namespace that defines the default parse tree.
namespace ParseTree {

/// Factory class that creates the default parse tree output.
class PlainParseTreeFactory final: public ValueFactoryInterface {
public:
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override;

    ~PlainParseTreeFactory() override = default;
};
} // namespace ParseTree
} // namespace PEG

#endif