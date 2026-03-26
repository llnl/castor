// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == a + b
int add(int a, int b);

#pragma castor ensures result == checked(a + b)
int add(int a, int b) { return a + b; }
