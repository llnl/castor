// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma castor ensures param == 0 => result == 1
#pragma castor ensures param == 1 => result == 0
#pragma castor ensures param == 2 => result == 2
#pragma castor ensures param == 3 => result == 2
#pragma castor ensures param < 0 \/ param > 3 => result == param
int naive_switch(int param)
{
	int t = 0;

	switch (auto p = param)
	{
		case 0:
			t = 0;
			t = 1;
			break;
		case 1:
			return 0;
		case 2:
			t = 2;
			break;
		case 3:
			t += 1;
			t += 1;
			break;
		default:
			return p;
	}

	return t;
}

#pragma castor ensures param == 1 => result == 2
#pragma castor ensures param == 3 => result == 1
#pragma castor ensures param != 1 /\ param != 3 => result == 0
int no_default(int param)
{
	int t = 0;

	switch (param)
	{
		case 1:
			t = 2;
			break;
		case 3:
			return 1;
	}

	return t;
}

#pragma castor ensures param == 1 => result == 0
#pragma castor ensures param == 2 => result == 4
#pragma castor ensures param == 3 => result == 2
#pragma castor ensures param == 4 => result == param + 1
#pragma castor ensures param < 1 \/ param > 4 => result == 1
int fall_through(int param)
{
	int t = 0;

	switch (param)
	{
		case 1:
			t = 0;
			break;
		case 2:
			t = -1;
			return 4;
		case 3:
			t += 2;
			break;
		case 4:
			t = param;
		default:
			t += 1;
			break;
	}

	return t;
}
