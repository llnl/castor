// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#define array_separated(A, B, C, N) ((A + N <= B \/ B + N <= A) /\ (B + N <= C \/ C + N <= B) /\ (A + N <= C \/ C + N <= A))

#pragma castor invariant valid_array(a, n) /\ valid_array(b, n) /\ valid_array(c, n)
#pragma castor invariant array_separated(a, b, c, n)
#pragma castor requires 0 < n
#pragma castor writes c[0 .. n - 1]
#pragma castor ensures forall sint32: idx. 0 <= idx /\ idx < n => c[idx] == checked(a[idx] + b[idx])
void impl_pre_transform(int* a, int* b, int* c, int n) {
	#pragma castor invariant valid_array(a, n) /\ valid_array(b, n) /\ valid_array(c, n)
	#pragma castor invariant array_separated(a, b, c, n)
	#pragma castor invariant 0 <= i /\ i <= n /\ 0 < n
	#pragma castor invariant forall sint32: idx. 0 <= idx /\ idx < i => c[idx] == checked(a[idx] + b[idx])
	#pragma castor writes c[0 .. n - 1], i
	#pragma castor variant n - i
	for (int i = 0; i < n; i += 1) {
		c[i] = a[i] + b[i];
	}
}

#pragma castor invariant valid_array(a, n) /\ valid_array(b, n) /\ valid_array(c, n)
#pragma castor invariant array_separated(a, b, c, n)
#pragma castor requires 0 < n /\ n < max_sint32
#pragma castor writes c[0 .. n - 1]
#pragma castor ensures forall sint32: idx. 0 <= idx /\ idx < n => c[idx] == checked(a[idx] + b[idx])
void impl_post_transform(int* a, int* b, int* c, int n) {
	#pragma castor invariant valid_array(a, n) /\ valid_array(b, n) /\ valid_array(c, n)
	#pragma castor invariant array_separated(a, b, c, n)
	#pragma castor invariant 0 <= i /\ i <= n + 1 /\ 0 < n /\ n < max_sint32
	#pragma castor invariant forall sint32: idx. 0 <= idx /\ idx < i /\ idx < n => c[idx] == checked(a[idx] + b[idx])
	#pragma castor writes c[0 .. n - 1], i
	#pragma castor variant n - i
	for (int i = 0; i < n; i += 2) {
		c[i] = a[i] + b[i];

		if (i + 1 < n) {
			c[i + 1] = a[i + 1] + b[i + 1];
		}
	}
}

#pragma castor requires valid_array(a, n) /\ valid_array(b, n) /\ valid_array(c, n)
#pragma castor requires array_separated(a, b, c, n)
#pragma castor requires 0 < n /\ n < max_sint32
void equivalent(int* a, int* b, int* c, int n) {
	impl_pre_transform (a, b, c, n);
	#pragma castor assert unchanged(a) /\ unchanged(b) /\ unchanged(c) /\ unchanged(n)
state_pre:
	impl_post_transform(a, b, c, n);
state_post:
	;
	#pragma castor assert forall sint32: idx. 0 <= idx /\ idx < n => at(c[idx], @state_pre) == at(c[idx], @state_post)
}
