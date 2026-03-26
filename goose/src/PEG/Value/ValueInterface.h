// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_ValueInterface_H
#define PEG_ValueInterface_H

#include <PEG/BasicTypes.h>

namespace PEG {

/// @brief An abstract factory class that represents the value generated after a successful parse of input.
///
/// By default, the output value is going to an object of class PlainParseTree - this is a simple dummy
/// object that contains the output as a tuple (label, position, value, children) and can be printed.
/// Since the base class of value, i.e. this class, doesn't put any restrictions on what can or cannot be
/// done on the value.
class ValueInterface {

public:
    /// A default destructor.
    virtual ~ValueInterface() = default;

    /// A templated function to dynamically convert an abstract class pointer to a pointer to the
    /// appropriate subclass.
    ///
    /// @tparam Self A subclass that publicly inherits ValueInterface.
    /// @param child An abstract class shared pointer to an object of class \p Self that publicly inherits
    /// from ValueInterface.
    /// @return A shared pointer to an object of class \p Self.
    template <class Self> static std::shared_ptr<Self> downcast(const std::shared_ptr<ValueInterface> child) {
        return std::dynamic_pointer_cast<Self>(child);
    }
};

} // namespace PEG

#endif