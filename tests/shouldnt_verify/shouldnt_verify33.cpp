// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor axiom inconsistency: false

#pragma castor ensures result == 1
int returns_zero() { return 0; }

#pragma castor lemma arithmetic: 2 + 2 == 5
