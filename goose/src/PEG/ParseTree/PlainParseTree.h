// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_PlainParseTree_H
#define PEG_PlainParseTree_H

#include <PEG/BasicTypes.h>
#include <PEG/Value/ValueInterface.h>
#include <string>
#include <utility>
#include <vector>

namespace PEG {

namespace ParseTree {

/// @brief Class that implements ValueInterface and defines a default parse tree output behavior
///
/// Objects of this class are meant to be the default output of parsing if no other factory or transformation is
/// specified.
class PlainParseTree: public ValueInterface {

public:
    std::string label; ///< The label that describes the kind of Expression that was parsed.
    ///< When label is a non-terminal, this describes the index in the list of production rules of a non-terminal
    /// i.e. it tells us which rule was used at this non-terminal to parse the rest of the input.
    int position;
    std::string value; ///< When label is a terminal, this gives us the content that was parsed.
    ///< This contains the list of child expressions that constitute an Expression (like for Seq, SeqN) or the contents
    /// that were parsed (often multiple times like in Repeat or Plus).
    std::vector<ValueInterfacePtr> children;

    PlainParseTree(
        std::string label, const int &position, std::string value, const std::vector<ValueInterfacePtr> &children
    ) : label(std::move((label))), position(position), value(std::move((value))), children(children) {}

    /// Helper function to output indentation into std::cout
    /// @param indent The current indentation level.
    static void indentation(int indent);

    /// Pretty print a string representation of parse structure using indentation
    /// @param indent The current indentation level.
    void print(int indent) const;

    /// Print the current parsed output structure as a JSON into the input stream.
    /// TODO: maybe use some serialization/deserialization library instead of manually writing (sometimes invalid) JSON.
    void printJSON(std::ostringstream &printout) const;

    /// Print the current object as a JSON element into std::cout.
    void print() const;

    /// A default destructor.
    ~PlainParseTree() override = default;

    /// Instantiate a shared pointer to a PlainParseTree object.
    /// @param label The label of the Expression that was parsed.
    /// @param position If non-terminal, the position index of the rule used in the list of production rules of
    /// non-terminal.
    /// @param value If terminal, the value that was just parsed.
    /// @param children For certain subclass of Expression, this contains the constituents/child sub-expression.
    /// @return An abstract class shared pointer an object of class PlainParseTree.
    static ValueInterfacePtr instance(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    );
};
} // namespace ParseTree
} // namespace PEG

#endif