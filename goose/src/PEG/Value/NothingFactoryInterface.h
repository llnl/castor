// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_NothingFactoryInterface_H
#define PEG_NothingFactoryInterface_H

#include <PEG/BasicTypes.h>
#include <PEG/Value/ValueFactoryInterface.h>
#include <vector>

namespace PEG {

/// A ValueFactory implementation that does, as its name implies, nothing.
class NothingFactory final: public ValueFactoryInterface {
public:
    /// @brief A function that ignores its input parse structure and just returns nothing.
    ///
    /// This is useful in cases where we'd like to parse something and then ignore
    /// what was parsed - such as comments, whitespace, and more.
    ///
    /// However, care must be given in how to handle upstream parse structures that
    /// might contain nothing factory outputs in their children's output and its
    /// basically a dangling shared pointer or simply a NULL dereference error waiting
    /// to happen.
    ///
    /// @param label unused.
    /// @param position unused.
    /// @param value unused.
    /// @param children unused.
    /// @return A shared pointer to an uninitialized NULL value.
    ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) override {
        return {};
    };

    ~NothingFactory() override = default;
};

} // namespace PEG

#endif