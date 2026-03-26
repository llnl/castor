// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PEG_FailParseResult_H
#define PEG_FailParseResult_H

#include <PEG/BasicTypes.h>
#include <PEG/ParseResult.h>
#include <string>

namespace PEG {

namespace ParseResultType {

/// Class that represents a Parse Failure.
class FailParseResult final: public ParseResult {

public:
    void output() override;

    bool isSuccess() override;

    explicit FailParseResult(const int &position) : ParseResult(position, false) {}

    ~FailParseResult() override = default;

    // TODO: maybe don't abuse input as an error message and have it be separate.
    //       that + success headers could just be merged into single parent header with three classes
    //       we ain't java (one class per file)

    static ParseResultPtr instance(const int &position);
};

} // namespace ParseResultType
} // namespace PEG
#endif