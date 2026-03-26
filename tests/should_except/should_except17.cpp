// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

struct HasStaticMember
{
	static bool member = false;
};

void tries_to_assert_static_member()
{
	auto member = HasStaticMember::member;
	#pragma castor assert !member
}
