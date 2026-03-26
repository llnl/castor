// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures b == old(b) + 1
void increment_bool(bool& b) {
	++b;
}
