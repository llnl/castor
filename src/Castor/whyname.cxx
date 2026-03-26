// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <string>
#include <cctype>

#ifndef WHYNAME
#define WHYNAME

///
/// @brief Wrapper class for Why3 identifiers.
///
/// This class will do a deterministic, automatic conversion from identifiers in the IR to identifiers legal in Why3
///
class WhyName
{
private:
	std::string name;

	///
	/// @brief This does the conversion from IR identifiers to Why3-legal identifiers
	///
	/// @param name IR identifier
	/// @return Why3-legal identifier
	///
	std::string parse_name(std::string name)
	{
		std::string ret;

		// this is part of the name mangling
		// we generate a name and try to be human-readable
		for (auto c : name)
			if (std::isalnum(c))
				ret += c;
			else if (c == '~')
				ret += "_destructor_";
			else if (c == '+')
				ret += "_plus_";
			else if (c == '-')
				ret += "_minus_";
			else if (c == '*')
				ret += "_star_";
			else if (c == '/')
				ret += "_slash_";
			else if (c == '=')
				ret += "_equals_";
			else if (c == '&')
				ret += "_ref_";
			else if (c == '<')
				ret += "L";
			else if (c == '>')
				ret += "R";
			else if (c == '^')
				ret += "_xor_";
			else if (c == '|')
				ret += "_or_";
			else
				ret += '_';

		return ret;
	}

public:
	///
	/// @brief Default constructor, initializes name to the empty string
	///
	WhyName()
	{
		this->name = "";
	}

	///
	/// @brief Constructor for std::string
	///
	/// @param name std::string object to convert
	///
	WhyName(std::string name)
	{
		this->name = parse_name(name);
	}

	///
	/// @brief Constructor for const char*
	///
	/// @param name const char* to convert
	WhyName(const char* name)
	{
		this->name = parse_name(name);
	}

	///
	/// @brief Conversion from WhyName to strings
	///
	/// @return String representation of this object
	///
	operator std::string() const
	{
		return this->name;
	}

	///
	/// @brief Concatenation bewteen two WhyName objects
	///
	/// @param name Second argument of a concatenation
	/// @return The concatenated name, as a string
	///
	std::string operator+(WhyName& name)
	{
		return this->name + name;
	}

	///
	/// @brief Concatenation between a WhyName and a const char*
	///
	/// @param lhs Left-hand side
	/// @param rhs Right-hand side
	/// @return The concatenated name, as a string
	///
	friend std::string operator+(const WhyName& lhs, const char* rhs);

	///
	/// @brief Concatenation between a const char* and a WhyName
	///
	/// @param lhs Left-hand side
	/// @param rhs Right-hand side
	/// @return The concatenated name, as a string
	///
	friend std::string operator+(const char* lhs, const WhyName& rhs);

	///
	/// @brief Concatenation bewteen a WhyName and a std::string
	///
	/// @param lhs Left-hand side
	/// @param rhs Right-hand side
	/// @return The concatenated name, as a string
	///
	friend std::string operator+(const WhyName& lhs, const std::string& rhs);

	///
	/// @brief Concatenation between a std::string and a WhyName
	///
	/// @param lhs Left-hand side
	/// @param rhs Right-hand side
	/// @return The concatenated name, as a string
	///
	friend std::string operator+(const std::string& lhs, const WhyName& rhs);

	///
	/// @brief Provides an ordering for sorting WhyName objects
	///
	/// @param name WhyName to compare to
	/// @result The ordering given
	///
	bool operator<(const WhyName& name) const
	{
		return this->name < name.name;
	}

	///
	/// @brief Equality between a WhyName and a const char*
	///
	/// @param rhs Right-hand side
	/// @result Whether or not they're equal
	///
	bool operator==(const char* rhs)
	{
		return this->name == rhs;
	}

	///
	/// @brief Equality between a WhyName and a std::string
	///
	/// @param rhs Right-hand side
	/// @result Whether or not they're equal
	///
	bool operator==(const std::string& rhs)
	{
		return this->name == rhs;
	}
};

///
/// @brief Concatenation between a WhyName and a const char*
///
/// @param lhs Left-hand side
/// @param rhs Right-hand side
/// @return The concatenated name, as a string
///
inline std::string operator+(const WhyName& lhs, const char* rhs)
{
	return lhs.name + rhs;
}

///
/// @brief Concatenation between a const char* and a WhyName
///
/// @param lhs Left-hand side
/// @param rhs Right-hand side
/// @return The concatenated name, as a string
///
inline std::string operator+(const char* lhs, const WhyName& rhs)
{
	return lhs + rhs.name;
}

///
/// @brief Concatenation between a WhyName and a std::string
///
/// @param lhs Left-hand side
/// @param rhs Right-hand side
/// @return The concatenated name, as a string
///
inline std::string operator+(const WhyName& lhs, const std::string& rhs)
{
	return lhs.name + rhs;
}

///
/// @brief Concatenation between a std::string and a WhyName
///
/// @param lhs Left-hand side
/// @param rhs Right-hand side
/// @return The concatenated name, as a string
///
inline std::string operator+(const std::string& lhs, const WhyName& rhs)
{
	return lhs + rhs.name;
}

#endif
