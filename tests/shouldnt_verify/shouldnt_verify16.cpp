// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

int global_with_init = 5;

#pragma castor ensures global_with_init == 5
void tries_to_assert_about_global() { }
