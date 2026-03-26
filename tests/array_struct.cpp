// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

template <typename T>
struct Pair
{
	T first;
	T second;
};

#pragma castor ensures result == 21
int tester()
{
	Pair<int> arr[3];
	arr[0].first = 1;
	0[arr].second = 2;
	arr[1].first = 3;
	1[arr].second = 4;
	arr[2].first = 5;
	2[arr].second = 6;

	#pragma castor assert arr[0].first == 1 /\ arr[0].second == 2
	#pragma castor assert arr[1].first == 3 /\ arr[1].second == 4
	#pragma castor assert arr[2].first == 5 /\ arr[2].second == 6

	return arr[0].first + arr[0].second +
		arr[1].first + arr[1].second +
		arr[2].first + arr[2].second;
}

#pragma castor ensures result == 21
int tester_2()
{
	Pair<int> arr[3];
	(arr + 0)->first = 1;
	(0 + arr)->second = 2;
	(arr + 1)->first = 3;
	(1 + arr)->second = 4;
	(arr + 2)->first = 5;
	(2 + arr)->second = 6;

	#pragma castor assert arr[0].first == 1 /\ arr[0].second == 2
	#pragma castor assert arr[1].first == 3 /\ arr[1].second == 4
	#pragma castor assert arr[2].first == 5 /\ arr[2].second == 6

	return arr[0].first + arr[0].second +
		arr[1].first + arr[1].second +
		arr[2].first + arr[2].second;
}

