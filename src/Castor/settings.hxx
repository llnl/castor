// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

// settings.h
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <string>

///
/// @brief Settings that can be set from the command line
///
struct Settings {
	enum class Method { wp, sp, unknown };
	enum class Version { cpp17, cpp14, cpp11, unknown };

	int time;          ///< Timeout for the provers (default=5)
	bool rerun;        ///< Whether or not to force rebuild proofs (default=false)
	bool only_sage;    ///< Whether or not to only generate the SAGE (default=false)
	bool only_ir;      ///< Whether or not to only generate the IR (default=false)
	bool only_why3;    ///< Whether or not to only generate the WhyML (default=false)
	bool extra_lemmas; ///< Whether or not to use the Lemmas.mlw files (default=false)
	bool bitvec_lemmas;///< Whether or not to use the BitVector.mlw files (default=false)
	bool smoke_tests;  ///< Whether or not to run smoke tests (default=false)
	bool overflow_checking; ///< Whether or not to simulate machine integer overflow when verifying (default=true)
	int debug;         ///< Whether or not to enable debug output (default=0)
	std::string outputFile; ///< Name of the output file (default=a.o)
	Method method;     ///< Which predicate calculus method to use (default=wp)
	Version version;   ///< Which C++ version to use

	///
	/// @brief Constructor
	///
	Settings() :
		time(5),
		rerun(false),
		only_ir(false),
		only_why3(false),
		extra_lemmas(false),
		bitvec_lemmas(false),
		smoke_tests(false),
		overflow_checking(true),
		debug(0),
		outputFile("a.o"),
		method(Method::wp),
		version(Version::cpp17)
	{
	}
};

#endif // __SETTINGS_H__
