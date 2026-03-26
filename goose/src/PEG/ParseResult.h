// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_ParseResult_H
#define PEG_ParseResult_H

#include <PEG/BasicTypes.h>
#include <string>

namespace PEG {

/// A class that stores the Result of Parsing an input against a grammar.
class ParseResult {

public:
    /// Print the result of parsing
    virtual void output() = 0;

    ///< The position in the input string upto which the parsing was possible.
    /// The prefix of the input string from 0...position forms the parsed portion.
    /// If position is the size of the input string, then we have successfully parsed the entire
    /// string and this is referred to as a Full Success.
    int position;

    ///< The result value, if the parse was successful.
    ValueInterfacePtr valueOutput;

    ///< Whether the parse was a full success or not. A full success means that the entire input string was
    /// parsed. A successful parse can just parse a prefix of the input.
    bool is_full_success;

    /// A default destructor.
    virtual ~ParseResult() = default;

    /// Constructor for initializing a ParseResult with a Failed parse.
    /// @param position Index upto which the input string was parsed.
    /// @param is_full_success Whether the entire input string was parsed.
    /// TODO: remove explicit.
    explicit ParseResult(const int &position, const bool &is_full_success) :
        position(position), is_full_success(is_full_success) {}

    /// Constructor for initializing a ParseResult with a Successful parse.
    /// @param valueOutput The output value created from the parsed input.
    /// @param position The position in the input string upto which parsing was possible.
    /// @param is_full_success Whether the entire input string was parsed or not.
    ParseResult(ValueInterfacePtr valueOutput, const int &position, const bool &is_full_success) :
        position(position), valueOutput(std::move(valueOutput)), is_full_success(is_full_success) {}

    /// Check if the parse was successful.
    /// @return A boolean value that corresponds to true if successful parse and false otherwise.
    virtual bool isSuccess() = 0;

    /// Check if the parse was successful and the entire input was parsed.
    /// @return A boolean value that corresponds to true if successful parse and false otherwise.
    [[nodiscard]] bool isFullSuccess() const { return is_full_success; }
};

} // namespace PEG

#endif