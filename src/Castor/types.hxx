// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "ir.hxx"
#include <rose.h>
#include <memory>

using namespace IR;

///
/// @brief Returns the IR type associated with a SAGE type
///
/// @param type The SAGE type
/// @return The IR type
///
std::shared_ptr<IRType> getIRTypeFromSgType(SgType* type);
