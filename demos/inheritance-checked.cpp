// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

class Rectangle {
	unsigned int side1_length;
	unsigned int side2_length;

public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->side1_length == side1
	#pragma castor ensures this->side2_length == side2
	#pragma castor writes this->side1_length, this->side2_length
	Rectangle(unsigned int side1, unsigned int side2)
		: side1_length(side1), side2_length(side2) { }

	#pragma castor invariant valid(this)
	#pragma castor ensures result == checked(to_uint32(2) * this->side1_length + \
					  to_uint32(2) * this->side2_length)
	#pragma castor no_write
	unsigned int get_perimeter() {
		return 2 * side1_length + 2 * side2_length;
	}
};

class Square : public Rectangle {
public:
	#pragma castor invariant valid(this)
	#pragma castor ensures this->side1_length == side
	#pragma castor ensures this->side2_length == side
	#pragma castor writes this->side1_length, this->side2_length
	Square(unsigned int side) : Rectangle(side, side) { }

	#pragma castor invariant valid(this)
	#pragma castor invariant this->side1_length == this->side2_length
	#pragma castor ensures exists uint32: x. checked(x * to_uint32(4)) == result
	#pragma castor no_write
	unsigned int get_perimeter() {
		return this->Rectangle::get_perimeter();
	}
};
