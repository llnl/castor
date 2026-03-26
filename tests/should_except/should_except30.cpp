// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

struct Foo {} ;

#pragma castor ensures f == old(f) + 1
void increment_foo(Foo& f) { }
