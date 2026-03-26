// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_ValueFactoryInterface_H
#define PEG_ValueFactoryInterface_H

#include <PEG/BasicTypes.h>
#include <vector>

namespace PEG {

/// @brief An abstract factory class to help create ValueInterface objects
///
/// A ParseTree represents the output of a successful parse where the input is transformed into one
/// with a structure that represents a tree.
class ValueFactoryInterface {
public:
    /// Given the constituents to a successful parse of a portion of input, create a value object.
    /// @param label The kind of Expression that matched the parse.
    /// @param position When used by a Non-Terminal Expression, tells us the index of the rule that was used to parse
    /// this. -1 otherwise
    /// @param value The value parsed if any. Usually filled by a successful parse of a Terminal or Dot Expression.
    /// @param children Either the constituents of an Expression (Sequence/SeqN/NonTerminal) or all the parsed
    /// structures from a single a Star/Plus/Optional.
    /// @return An abstract class shared pointer to an object of a class that publicly inherits ValueInterface.
    virtual ValueInterfacePtr createParseTree(
        const std::string &label, const int &position, const std::string &value,
        const std::vector<ValueInterfacePtr> &children
    ) = 0;

    /// A default destructor.
    virtual ~ValueFactoryInterface() = default;

    /// A templated instantiation of the Factory.
    /// @tparam ValueFactoryInheritor A subclass of ValueFactoryInterface.
    /// @return An abstract class shared pointer to an object of a class that publicly inherits ValueFactoryInterface.
    template <class ValueFactoryInheritor> static ValueFactoryInterfacePtr instance() {
        return std::make_shared<ValueFactoryInheritor>();
    }
};

} // namespace PEG

#endif