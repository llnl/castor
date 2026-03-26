// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <exception>

#ifndef __WMLEXCEPT
#define __WMLEXCEPT

///
/// @brief This is the base class for all exceptions in Castor
///
class CastorException : public std::exception
{
private:
	///
	/// @brief The message output to the user.
	///
	std::string msg;

public:
	///
	/// @brief Default constructor.
	///
	CastorException(std::string msg = "Encountered exception converting C++ to WhyML.")
	{
		this->msg = msg;
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief This exception is thrown when encountering an unknown SgNode.
///
class UnknownSgNodeException : public CastorException
{
private:
	std::string msg;

public:
	///
	/// @brief Constructor
	///
	UnknownSgNodeException(std::string node)
	{
		this->msg = std::string("I don't know about this Sage node -> " + node);
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief This exception is thrown when encountering an explicitly unsupported feature.
///
class UnsupportedFeatureException : public CastorException
{
private:
	std::string msg;

public:
	///
	/// @brief Constructor
	///
	UnsupportedFeatureException(std::string feature)
	{
		this->msg = std::string(feature + " is an unsupported feature!");
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief This exception is thrown when attempting to calculate the `sizeof` an invalid type,
/// such as IRNonRealType or IRUnknownType.
///
class UnknownSizeException : public CastorException
{
private:
	std::string msg;

public:
	///
	/// @brief Constructor
	///
	UnknownSizeException()
	{
		this->msg = std::string("Cannot calculate sizeof of nonreal or unknown type!");
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief This exception is thrown on syntax errors when parsing verification conditions.
///
class SyntaxErrorException : public CastorException
{
private:
	std::string msg;

public:
	///
	/// @brief Constructor
	///
	SyntaxErrorException(std::string string)
	{
		this->msg = std::string("Syntax error encountered while parsing -> " + string);
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief This exception is thrown when encountering an unknown IRNode in the backend.
///
class UnknownIRNodeException : public CastorException
{
private:
	std::string msg;

public:
	///
	/// @brief Constructor
	///
	UnknownIRNodeException(std::string node)
	{
		this->msg = std::string("I don't know about this IR node -> " + node);
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief This exception is primarily thrown from `safety_cast` and `safety_cast_raw`.
///
class InvalidCastException : public CastorException
{
private:
	std::string msg;

public:
	///
	/// @brief Constructor
	///
	InvalidCastException(std::string T, std::string U)
	{
		this->msg = std::string("Expecting node of type " + T + ", but did not receive one.");
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief This exception is thrown when encountering a variable in the verification language that was not
/// defined in the symbol table.
///
class UnknownVariableException : public CastorException
{
private:
	std::string msg;

public:
	///
	/// @brief Constructor
	///
	UnknownVariableException(std::string var_name)
	{
		this->msg = std::string("I don't know this variable -> " + var_name);
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief We don't throw this exception enough. This is thrown when an invalid number of arguments are passed
/// to a function.
///
class IncorrectArgumentCountException : public CastorException
{
private:
	std::string msg;
	
public:
	///
	/// @brief Constructor
	///
	IncorrectArgumentCountException(int got, int expected, std::string name)
	{
		this->msg = std::string("Expected " + std::to_string(got) + " argument(s) to function \"" + name + "\", got " + std::to_string(expected) + ".");
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief This exception is thrown when the type of an intermediate expression cannot be determined.
/// This usually happens in the IRNodes generated from the verification language.
///
class CannotInferTypeException : public CastorException
{
private:
	std::string msg;

public:
	///
	/// @brief Constructor
	///
	CannotInferTypeException(std::string expr)
	{
		this->msg = "I can't infer the type of this IR expression -> " + expr;
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief Thrown when a type is unknown.
/// 
class UnknownTypeException : public CastorException
{
private:
	std::string msg;
	
public:
	///
	/// @brief Constructor
	///
	UnknownTypeException(std::string got)
	{
		this->msg = std::string("I don't know this type -> " + got);
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief Thrown if an argument is invalid.
///
class InvalidArgumentException : public CastorException
{
private:
	std::string msg;

public:
	///
	/// @brief Constructor
	///
	InvalidArgumentException(std::string msg)
	{
		this->msg = msg;
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

///
/// @brief Thrown if labels are used in the verification language anywhere you shouldn't.
///
class ImproperLabelUsageException : public CastorException
{
public:
	///
	/// @brief Constructor
	///
	ImproperLabelUsageException() = default;

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return "Cannot use labels in this context!";
	}
};

///
/// @brief Thrown if the expected type of an expression doesn't match the actual type of an expression.
///
class TypeMismatchException : public CastorException
{
private:
	std::string msg;

public:
	///
	/// @brief Constructor
	///
	TypeMismatchException(std::string expr, std::string found_type, std::string expected_type)
	{
		this->msg = std::string("Type mismatch in expression: \"" + expr + "\", expected type \"" + expected_type + "\" but found type \"" + found_type + "\"");
	}

	///
	/// @brief Returns the error message.
	///
	/// @return The error message.
	///
	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
};

#endif
