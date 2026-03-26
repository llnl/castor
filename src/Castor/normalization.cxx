// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "normalization.hxx"
#include <CodeThornLib.h>
#include <Normalization.h>
#include <NormalizationCxx.h>
#include <Sawyer/Message.h>

///
/// @brief Normalizes the SAGE AST using CodeThorn transformations
///
/// @param root The root AST node
/// @param debug Whether or not to enable debug output
/// 
void normalize(SgNode* root, bool debug)
{
	CodeThorn::CodeThornLib::configureRose();
	CodeThorn::Normalization norm;

	if (!debug)
		CodeThorn::Normalization::logger[Sawyer::Message::INFO].disable();

	CodeThorn::normalizeCxx1(norm, root);

	CodeThorn::CxxTransformStats stats;
	CodeThorn::normalizeCtorDtor(root, stats);
	CodeThorn::normalizeObjectDestruction(root, stats);
}
