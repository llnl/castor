// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor requires max_int(T) > 0
template <typename T>
void foo() { }

template void foo<int>();
