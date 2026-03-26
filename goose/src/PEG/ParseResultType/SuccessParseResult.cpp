// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseResultType/SuccessParseResult.h>
#include <iostream>
#include <string>

namespace PEG {

void
ParseResultType::SuccessParseResult::output() {
    std::cout << "output: " << (isFullSuccess() ? "full " : "") << "success" << std::endl;
}

bool
ParseResultType::SuccessParseResult::isSuccess() {
    return true;
}

ParseResultPtr
ParseResultType::SuccessParseResult::instance(
    ValueInterfacePtr value_output, const int &position, const int &input_length
) {
    return std::make_shared<SuccessParseResult>(std::move(value_output), position, input_length);
}

} // namespace PEG