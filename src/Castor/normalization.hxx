// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <rose.h>

///
/// @brief Normalizes the SAGE AST using CodeThorn transformations
///
/// @param root The root AST node
/// @param debug Whether or not to enable debug output
/// 
void normalize(SgNode* root, bool debug);
