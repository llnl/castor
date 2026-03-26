// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <PEG/BasicTypes.h>
#include <PEG/ParseResult.h>
#include <PEG/ParseResultType/FailParseResult.h>
#include <iostream>
#include <string>

namespace PEG {

void
ParseResultType::FailParseResult::output() {
    printf("result: failed \n");
}

bool
ParseResultType::FailParseResult::isSuccess() {
    return false;
}

ParseResultPtr
ParseResultType::FailParseResult::instance(const int &position) {
    return std::make_shared<FailParseResult>(position);
}

} // namespace PEG