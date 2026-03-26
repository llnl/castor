| no | testcase                                                                                       | Result |
|----|------------------------------------------------------------------------------------------------|--------|
| 01 | requires min_int(T) <= this->first /\ this->first < max_int(T) - 1                             |        |
| 12 | requires forall sint32 i. 0 <= i /\ i < len => min_sint32 <= arr[i] /\ arr[i] < max_sint32 - 1 | S      |
| 02 | ensures this->second == old(this->second) + 1                                                  |        |
| 17 | ensures (a & (1 << b)) != 0 <-> result != 0                                                    |        |
| 05 | assert !(exists uint8: i, uint8: j. i < j)                                                     | S      |
| 06 | assert forall sint64: i. to_sint64(i) == i                                                     | S      |
| 07 | assert !(forall uint8: i, sint8: j. i < j)                                                     | S      |
| 10 | assert arr[2].first == 5 /\ arr[2].second == 6                                                 |        |
| 11 | assert obj.arr[0] == obj.arr[1]                                                                |        |
| 16 | assert forall sint32: a. 0 <= a /\ a < ARRLEN => arr[a] == 0                                   |        |
| 08 | invariant 1 <= i /\ i <= 11                                                                    |        |
| 15 | invariant forall sint32: a. 0 <= a /\ a < i => arr[a] == old(arr[a]) + 1                       | SSS    |
| 03 | ensures result == I * (I + 1) / 2                                                              |        |
| 04 | ensures **a == old(**a) * **b                                                                  |        |
| 09 | variant 10 - i                                                                                 |        |
| 13 | writes arr[0 .. len - 1], newref, i                                                            |        |
| 14 | no_write                                                                                       |        |
| 18 | ensures *a == old(*a) ^ b                                                                      |        |
