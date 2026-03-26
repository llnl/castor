// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures result == 5
int returns_5_on_assumption() {
	int return_value;
	#pragma castor assume return_value == 5
	return return_value;
}

void introduces_contradiction()
{
	#pragma castor assume false
	#pragma castor assert 5 == 6
}

