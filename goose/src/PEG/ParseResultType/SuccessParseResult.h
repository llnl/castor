// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_SuccessParseResult_H
#define PEG_SuccessParseResult_H

#include <PEG/BasicTypes.h>
#include <PEG/ParseResult.h>

namespace PEG {
namespace ParseResultType {

/// Class that represents a Successful Parse Result.
class SuccessParseResult final: public ParseResult {

public:
    void output() override;

    bool isSuccess() override;

    SuccessParseResult(ValueInterfacePtr value_output, const int &position, const int &input_length) :
        ParseResult(std::move(value_output), position, position == input_length) {};

    ~SuccessParseResult() override = default;

    static ParseResultPtr instance(ValueInterfacePtr value_output, const int &position, const int &input_length);
};

using SuccessParseResultPtr = std::shared_ptr<SuccessParseResult>;

} // namespace ParseResultType
} // namespace PEG

#endif