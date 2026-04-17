// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <rose.h>
#include <Rose/CommandLine.h>
#include <Rose/Diagnostics.h>
#include <Sawyer/Message.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <boost/dll/runtime_symbol_info.hpp>

#include "settings.hxx"
#include "irgenerator.hxx"
#include "whygenerator.hxx"
#include "fill_types.hxx"
#include "debug_info.hxx"
#include "validate_ir.hxx"
#include "symbol_table.hxx"
#include "set_metadata.hxx"
#include "order_functions.hxx"
#include "exception.hxx"
#include "normalization.hxx"
#include "messaging.hxx"

// We want to make sure that all of the info that we expect from CMake has been populated.
// This includes the path for Why3, Why3find, and the replacement header files.
#ifndef WHY3
	#error "Did not specify Why3 path, cannot compile!"
#endif
#ifndef WHY3FIND
	#error "Did not specify Why3find path, cannot compile!"
#endif
#ifndef REPLACEMENT_INCLUDES
	#error "Did not specify replacement includes path, cannot compile!"
#endif

using namespace Rose;
using namespace Sawyer::Message::Common;

///
/// @brief A short slogan for the Castor verification tool
///
static const char *purpose = "Verify C++ using a pragma-based automated verification paradigm.";
///
/// @brief A short description of the Castor verification tool
///
static const char *description =
	"Castor enables automated verification of idiomatic C++.\n"
	"Many C++ features are supported with Castor which are not\n"
	"supported in any other tool, such as dynamic memory allocation,\n"
	"templates, classes, and more. After writing C++, you can write\n"
	"verification conditions using pragmas, and Castor translates\n"
	"the source code to the Why3 platform, enabling the use of SMT\n"
	"solvers to formally verify the source code.\n"
	;

#ifndef TOOL_VERSION_STRING
	#warning "Did not specify Castor version, defaulting to \"0.0.0-dev\""
	#define TOOL_VERSION_STRING "0.0.0-dev"
#endif

///
/// @brief Enum which specifies all of the possible subcommands that Castor can be invoked with
///
enum class InvokeAction { prove, parse, compile, help, version };

///
/// @brief Sawyer logging facility
///
Sawyer::Message::Facility mlog;

///
/// @brief Some Castor settings, populated from the command line
///
Settings settings;

///
/// @brief The debug information table, populated later
///
FillDebugInfo debug_table;

///
/// @brief Runs the verifiers on a string of Why3 code, reporting successes and failures
///
/// @param why3 The Why3 source code
/// @param file_name The input C++ filename
/// @param temp_dir The Castor temporary directory
/// @param action Which subcommand Castor has been called with
/// @param base The Why3 AST
///
/// @return A success/error code
///
int run_verifiers(std::string why3, std::string file_name, std::string temp_dir, InvokeAction action, std::shared_ptr<Why3::WhyNode> base);

///
/// @brief Unparses the SAGE into a format intended for HUMAN DEBUGGING. NOT intended to go into a compiler!
///
/// @param root The SAGE AST
/// @param filename The input C++ filename
///
/// @return The unparsed SAGE tree
///
std::string unparse_sage(SgProject* root, std::string filename);

///
/// @brief Creates the Castor temporary directory
///
/// @return The location of the temporary directory
///
std::string create_temp_directory();

///
/// @brief Parses the error output from Why3find in the case that the provers could not prove the file
///
/// @param file The location of the Why3find log file
/// @param base The Why3 AST
///
/// @return A success/error code
///
int parse_error(std::string file, std::shared_ptr<Why3::WhyNode> base);

///
/// @brief Parses the error output from Why3 in the case that smoke was detected
///
/// @param file The location of the Why3 log file
/// @param base The Why3 AST
///
/// @return A success/error code
///
int parse_smoke_error(std::string file, std::shared_ptr<Why3::WhyNode> base);

namespace std
{
	///
	/// @brief Overloads ostream's << operator to support Settings::Method
	///
	/// This is necessary to use it in Sawyer
	///
	/// @param os The output stream
	/// @param method The Settings::Method method
	/// @return Returns os
	///
	std::ostream& operator<<(std::ostream& os, const Settings::Method& method)
	{
		if (method == Settings::Method::wp)
			os << "WP";
		else if (method == Settings::Method::sp)
			os << "SP";

		return os;
	}
}

///
/// @brief Material implication operator
///
/// @param p Antecedent
/// @param q Consequent
///
/// @return p -> q under a classical logic interpretation
///
constexpr inline bool IF(bool p, bool q) { return !p || q; }

///
/// @brief Handles input from the command line and guides the compiler through its many passes
///
/// @param argc The number of arguments from the command line
/// @param argv The arguments from the command line
///
/// @return A success/error code
///
int main(int argc, char **argv) {
	// The very first thing we do is initialize ROSE with some basic information
	ROSE_INITIALIZE;
	Rose::CommandLine::versionString = TOOL_VERSION_STRING " using " + Rose::CommandLine::versionString;
	Rose::Diagnostics::initAndRegister(&::mlog, "Castor");
	Rose::Diagnostics::initialize();

	// We get the subcommand that Castor was invoked with.
	// In the case of no subcommand, default to "help"
	std::string command = argc >= 2 ? argv[1] : "help";
	InvokeAction action;

	// Switch over the subcommands, setting the action enum approperiately
	if (command == "prove")
		action = InvokeAction::prove;
	else if (command == "parse")
		action = InvokeAction::parse;
	else if (command == "help" || command == "--help")
		action = InvokeAction::help;
	else if (command == "version" || command == "--version")
		action = InvokeAction::version;
	else if (command == "compile")
		action = InvokeAction::compile;
	else
	{
		// If a subcommand is unknown, note it as a fatal error and default to "help"
		log("Unknown subcommand \"" + command + "\"", LogType::FATAL, 0);
		action = InvokeAction::help;
	}

	// Create a Sawyer command line facility for parsing the data from each subcommand
	using namespace Sawyer::CommandLine;
	Parser p = CommandLine::createEmptyParserStage(purpose, description);

	// "castor prove" and "castor compile" use the same command line switches
	if (action == InvokeAction::prove || action == InvokeAction::compile)
	{
		// Create the switch group
		SwitchGroup toolSwitches("Castor proof-related switches");
		toolSwitches.name("castor");

		// --help is used to show all of the possible flags for this subcommand
		toolSwitches.insert(Switch("help", 'h')
			.doc("Show this documentation.")
			.action(showHelpAndExit(0)));
		// --time / -t is used to provide a timeout to Why3find
		toolSwitches.insert(Switch("time", 't')
			.argument("seconds", positiveIntegerParser(settings.time))
			.doc("Duration to attempt an SMT solver before applying tactics"));
		// --rerun / -r is translated to -f in Why3find
		toolSwitches.insert(Switch("rerun", 'r')
			.intrinsicValue("true", booleanParser(settings.rerun))
			.doc("Force rerun the provers instead of checking cache"));
		// --additional-lemmas is used to include the Lemmas.mlw theory file
		toolSwitches.insert(Switch("additional-lemmas")
			.intrinsicValue("true", booleanParser(settings.extra_lemmas))
			.doc("Use additional memory model lemmas; may help prove some files, but may hinder others"));
		// --bitwise-lemmas is used to include the BitVector.mlw theory file
		toolSwitches.insert(Switch("bitwise-lemmas")
			.intrinsicValue("true", booleanParser(settings.bitvec_lemmas))
			.doc("Use additional bitwise arithmetic lemmas; may help prove some files, but may hinder others"));
		// --smoke-tests is used to enable Why3 smoke testing
		toolSwitches.insert(Switch("smoke-tests")
			.intrinsicValue("true", booleanParser(settings.smoke_tests))
			.doc("Checks if functions may have been proving using inconsistent global context; does not check for inconsistent function preconditions"));
		// --no-overflow is used to disable overflow checking
		toolSwitches.insert(Switch("no-overflow")
			.intrinsicValue("false", booleanParser(settings.overflow_checking))
			.doc("Disables simulation of machine integers with overflow in favor of mathematical, unbounded integers"));
		// default debug level, if enabled, is 1
		std::string debug_default = "1";
		// --debug enables debug output, optionally with simple or verbose output
		toolSwitches.insert(Switch("debug")
			.argument("level", nonNegativeIntegerParser(settings.debug), debug_default)
			.doc("Set the debug output level (0=off, 1=simple debug output, 2=verbose debug output)"));
		// --transformer is used to select the predicate transformer
		toolSwitches.insert(Switch("transformer")
			.argument("transformer", enumParser<Settings::Method>(settings.method)
				->with("wp", Settings::Method::wp)
				->with("sp", Settings::Method::sp))
			.doc("Specifies the predicate transformer to use. Value must be one of: sp (=strongest postcondition), wp (=weakest precondition)"));
		// --standard is used to set the C++ standard version
		toolSwitches.insert(Switch("standard")
			.argument("version", enumParser<Settings::Version>(settings.version)
				->with("c++17", Settings::Version::cpp17)
				->with("17", Settings::Version::cpp17)
				->with("c++14", Settings::Version::cpp14)
				->with("14", Settings::Version::cpp14)
				->with("c++11", Settings::Version::cpp11)
				->with("11", Settings::Version::cpp11))
			.doc("Specifies the C++ version to use. Value must be one of: c++11, 11, c++14, 14, c++17, 17"));

		// little synopsis
		p.doc("Synopsis", "@prop{programName} prove [@v{switches}] @v{file_names}...");
		p.with(toolSwitches);
		p.errorStream(mlog[FATAL]); // print error messages as fatal
	}

	// "castor parse" is used if you only want castor to translate a C++ file without invoking provers.
	// I use this all the time in debugging Castor.
	if (action == InvokeAction::parse)
	{
		// Create the switch group
		SwitchGroup toolSwitches("Castor parse-related switches");
		toolSwitches.name("castor");

		// --help is used to show all of the possible flags for this subcommand
		toolSwitches.insert(Switch("help", 'h')
			.doc("Show this documentation")
			.action(showHelpAndExit(0)));
		// --generate-sage is used to generate the normalized SAGE before terminating, outputting SAGE in the process
		toolSwitches.insert(Switch("generate-sage")
			.intrinsicValue("true", booleanParser(settings.only_sage))
			.doc("Generate the SAGE, output to stdout, then halt compilation"));
		// --generate-ir is used to generate the IR before terminating, outputting IR in the process
		toolSwitches.insert(Switch("generate-ir")
			.intrinsicValue("true", booleanParser(settings.only_ir))
			.doc("Generate the IR, output to stdout, then halt compilation"));
		// --generate-whyml is used to generate the WhyML before terminating, outputting WhyML in the process
		toolSwitches.insert(Switch("generate-whyml")
			.intrinsicValue("true", booleanParser(settings.only_why3))
			.doc("Generate the WhyML, output to stdout, then halt compilation"));
		// --no-overflow is used to disable overflow checking
		toolSwitches.insert(Switch("no-overflow")
			.intrinsicValue("false", booleanParser(settings.overflow_checking))
			.doc("Disables simulation of machine integers with overflow in favor of mathematical, unbounded integers"));
		// default debug level, if enabled, is 1
		std::string debug_default = "1";
		// --debug enables debug output, optionally with simple or verbose output
		toolSwitches.insert(Switch("debug")
			.argument("level", nonNegativeIntegerParser(settings.debug), debug_default)
			.doc("Set the debug output level (0=off, 1=simple debug output, 2=verbose debug output)"));
		// --standard is used to set the C++ standard version
		toolSwitches.insert(Switch("standard")
			.argument("version", enumParser<Settings::Version>(settings.version)
				->with("c++17", Settings::Version::cpp17)
				->with("17", Settings::Version::cpp17)
				->with("c++14", Settings::Version::cpp14)
				->with("14", Settings::Version::cpp14)
				->with("c++11", Settings::Version::cpp11)
				->with("11", Settings::Version::cpp11))
			.doc("Specifies the C++ version to use. Value must be one of: c++11, 11, c++14, 14, c++17, 17"));

		// little synopsis
		p.doc("Synopsis", "@prop{programName} parse [@v{switches}] @v{file_names}...");
		p.with(toolSwitches);
		p.errorStream(mlog[FATAL]); // print error messages as fatal
	}

	// "castor help" is very simple and doesn't even use Sawyer... just outputs which subcommands are enabled
	if (action == InvokeAction::help)
	{
		std::cout << "Castor subcommands:"                                                    << std::endl;
		std::cout << "  prove   - Prove an annotated C++ file."                               << std::endl;
		std::cout << "  parse   - Parse an annotated C++ file."                               << std::endl;
		std::cout << "  compile - After successfully proving a file, compile into .o format." << std::endl;
		std::cout << "  help    - Display this menu."                                         << std::endl;
		std::cout << "  version - Display Castor and ROSE version information."               << std::endl;
		// terminate
		return 0;
	}

	// "castor version" is also very simple
	if (action == InvokeAction::version)
	{
		std::cout << Rose::CommandLine::versionString << std::endl;
		// terminate
		return 0;
	}

	// Parse the command-line and get the non-switch, positional arguments at the end
	std::vector<std::string> backend_args = p.parse(argc, argv).apply().skippedArgs();

	// Remove "parse" "prove" or "compile" from the backend arguments, those are only used as Castor subcommands
	for (int i = 0; i < backend_args.size(); i++)
		if (backend_args[i] == "parse" || backend_args[i] == "prove" || backend_args[i] == "compile")
			backend_args.erase(backend_args.begin() + i--);
		else if (backend_args[i].substr(0, 10) == "--standard")
			settings.version = Settings::Version::unknown;
		else if (backend_args[i].substr(0, 13) == "--transformer")
			settings.method = Settings::Method::unknown;

	// add some extra command line switches to make sure Castor is getting the right input program
	auto more_flags = CommandlineProcessing::generateArgListFromString(
		std::string("-DSKIP_ROSE_BUILTIN_DECLARATIONS -c -isystem ") + REPLACEMENT_INCLUDES + " " +
		std::string("-std=") + (settings.version == Settings::Version::cpp17 ? "c++17" : settings.version == Settings::Version::cpp14 ? "c++14" : "c++11"));

	// insert "more_flags" to the backend args
	backend_args.insert(backend_args.begin(), more_flags.begin(), more_flags.end());
	// insert "castor" to the backend args
	backend_args.insert(backend_args.begin(), argv[0]);

	// create the temporary directory
	auto temp_dir = create_temp_directory();

	// This is used in case Castor receives a header file. We create a temporary C++ file so that the ROSE frontend accepts it, but must remember to remove it later
	std::vector<std::string> to_remove;

	// Iterate through the backend arguments
	for (int i = 0; i < backend_args.size(); i++)
	{
		log("Including frontend argument: " + backend_args[i], LogType::INFO, 1);
	
		// If we're looking at a filename
		if (backend_args[i].find(".") != std::string::npos)
		{
			// Get the file extension
			std::filesystem::path input_file_path(backend_args[i]);
			auto extension = boost::algorithm::to_lower_copy(input_file_path.extension().native());
	
			// If it's a header file
			if (extension == ".h" || extension == ".hxx" || extension == ".hpp" || extension == ".h++")
			{
				log("Input file " + backend_args[i] + " is a header file. Creating C++ source file...", LogType::INFO, 1);

				// Create a new name for the file with a .cpp extension
				auto new_name = temp_dir + input_file_path.stem().native() + ".cpp";
	
				// Add it to remove later on
				to_remove.push_back(new_name);
	
				// Remove any file with this name that may already exist in the temp directory
				// Then copy over the actual input file to the temporary .cpp file
				system(("rm -rf " + new_name).c_str());
				system(("cp " + backend_args[i] + " " + new_name).c_str());
				backend_args[i] = new_name;
				log("Copied to " + new_name, LogType::INFO, 1);
			}
		}
	}

 
	// Finally! We're ready to begin compilation

	log("Starting translation...", LogType::INFO, 1);

	// These are a bunch of functors which correspond to different stages in compilation to Why3
	IRGenerator ir_generator;
	FillTypes type_filler;
	ValidateIR validate_ir;
	WhyGenerator why3_generator;
	SetMetadata metadata_setter;
	GenerateFunctionCallGraph call_graph_generator;

	// PASS 1: Generate the SAGE AST, enabling constant folding
	SgProject *root = frontend(backend_args, true);

	// If we have an error code, return the error code
	if (auto error = root->get_frontendErrorCode(); error != 0)
		return error;

	// If we've parsed multiple source files, Castor can't handle this, so exit
	if (root->get_sourceFileNameList().size() != 1)
	{
		log("Did not receive the correct number of input files (required: 1)\nAborting...", LogType::FATAL, 0);
		return 1;
	}

	// Check to make sure the C++ versions are internally consistent
	if (!(IF(settings.version == Settings::Version::cpp11, root->get_Cxx11_only()) &&   // Ensure if internal version is C++11 then SAGE is C++11
		IF(settings.version == Settings::Version::cpp14, root->get_Cxx14_only()) && // Ensure if internal version is C++14 then SAGE is C++14
		IF(settings.version == Settings::Version::cpp17, root->get_Cxx17_only())))  // Ensure if internal version is C++17 then SAGE is C++17
	{
		log("Use --standard flag instead of -std flag for setting C++ standard\nAborting...", LogType::FATAL, 0);
		return 2;
	}

	// Don't progress if an unknown C++ version was specified
	if (settings.version == Settings::Version::unknown)
	{
		log("Unknown C++ version specified by --standard\nAborting...", LogType::FATAL, 0);
		return 3;
	}

	// Don't progress if an unknown predicate transformer was specified
	if (settings.method == Settings::Method::unknown)
	{
		log("Unknown predicate transformer specified by --transformer\nAborting...", LogType::FATAL, 0);
		return 4;
	}

	log("SAGE generated...", LogType::INFO, 1);

	// At this point we can remove those temporary .cpp files
	for (auto rem : to_remove)
	{
		log("Removing temp file " + rem, LogType::INFO, 1);
		system(("rm " + rem).c_str());
	}

	// PASS 2: Normalize the Sage AST
	normalize(root, settings.debug);

	log("SAGE normalized...", LogType::INFO, 1);

	//SageInterface::constantFolding(root);

	//log("Folded constants...", LogType::INFO, 1);

	// Get the file name
	std::string file_name = root->get_sourceFileNameList()[0];
	boost::filesystem::path trimmed_path(file_name);

	try
	{
		// If --generate-sage was used, we've completed generating the SAGE, so this if statement triggers
		if (settings.only_sage)
		{
			// Output a couple newlines
			std::cout << std::endl << std::endl;
			// Output the unparsed SAGE
			std::cout << unparse_sage(root, trimmed_path.filename().string()) << std::endl;
			// exit
			return 0;
		}

		auto ir = ir_generator(root);    // PASS 3: Generate IR
		log("IR generated...", LogType::INFO, 1);
		ir = type_filler(ir);            // PASS 4: Set IR types and simplify
		log("IR types populated...", LogType::INFO, 1);
		debug_table(ir);                 // PASS 5: Assign debug info to certain verification condtiions
		log("IR debug info added...", LogType::INFO, 1);
		validate_ir(ir);                 // PASS 6: Validate IR
		log("IR validated...", LogType::INFO, 1);

		// If --generate-ir was used, we've completed generating the IR, so this if statement triggers
		if (settings.only_ir)
		{
			// Output a couple newlines
			std::cout << std::endl << std::endl;
			// Output the unparsed IR
			std::cout << ir->pp() << std::endl;
			// exit
			return 0;
		}

		auto whyml = why3_generator(ir);                                // PASS 7: Generate Why3 AST
		log("Why3 generated...", LogType::INFO, 1);
		whyml = metadata_setter(whyml);                                 // PASS 8: Set Why3 AST metadata
		log("Why3 metadata set...", LogType::INFO, 1);
		auto program = safety_cast<WhyProgram>(whyml);
		auto call_graph = call_graph_generator(program);                // PASS 9: Generate function call graph
		log("Call graph generated...", LogType::INFO, 1);
		auto final_whyml = generate_correct_order(call_graph, program); // PASS 10: Correctly order functions
		log("Ordered functions successfully...", LogType::INFO, 1);
		auto output = final_whyml->pp();                                // Is this considered a pass? Generate the WhyML string
		log("Why3 source obtained...", LogType::INFO, 1);

		// If --generate-whyml was not used, output some debug information
		if (!settings.only_why3 && (action == InvokeAction::prove || action == InvokeAction::compile))
		{
			log("C++ version used: "              + std::string(settings.version == Settings::Version::cpp17 ? "C++17" :
								settings.version == Settings::Version::cpp14 ? "C++14" :
								settings.version == Settings::Version::cpp11 ? "C++11" :
								"UNKNOWN!"), LogType::INFO, 1);
			log("Running Why3find with timeout: " + std::to_string(settings.time), LogType::INFO, 1);
			log("Backend predicate transformer: " + std::string(settings.method == Settings::Method::wp ? "weakest precondition" :
								settings.method == Settings::Method::sp ? "strongest postcondition" :
								"UNKNOWN!"), LogType::INFO, 1);
			log("Rerunning already proven VCs? "  + std::string(settings.rerun ? "yes" : "no"), LogType::INFO, 1);
			log("Using memory model lemmas? "     + std::string(settings.extra_lemmas ? "yes" : "no"), LogType::INFO, 1);
			log("Using bitvector lemmas? "        + std::string(settings.bitvec_lemmas ? "yes" : "no"), LogType::INFO, 1);
		}

		// We still invoke run_verifiers even if the subcommand is "castor parse", because this function is responsible for
		// pretty-printing the WhyML. It makes for a nicer-looking output
		if (run_verifiers(output, trimmed_path.filename().string(), temp_dir, action, final_whyml))
		{
			// If there was an error code, selectively choose an error message
			if (action == InvokeAction::prove || action == InvokeAction::compile)
				log("Could not prove " + trimmed_path.filename().string() + "...", LogType::ERROR, 0);
			else
				log("Could not parse " + trimmed_path.filename().string() + "...", LogType::ERROR, 0);
			// exit
			return 1;
		}
		else
		{
			// If there wasn't an error code, selectively choose a success message
			if (action == InvokeAction::prove || action == InvokeAction::compile)
				log("Successfully proved " + trimmed_path.filename().string() + "!", LogType::INFO, 0);
			else
				log("Successfully parsed " + trimmed_path.filename().string() + "!", LogType::INFO, 0); 
		}

		// TODO: THIS IS BUGGY: WE DON'T WANT TO COMPILE NORMALIZED SAGE! We want to output the original SAGE from the frontend!
		// If the subcommand is "castor compile", we pass off the SAGE AST to the ROSE backend.
		if (action == InvokeAction::compile)
		{
			// Compile the SAGE
			int result = backend(root);
			// If there's an error code, selectively choose an error message
			if (result)
				log("Could not compile " + trimmed_path.filename().string() + "...", LogType::ERROR, 0);
			else
				log("Successfully compiled " + trimmed_path.filename().string() + "!", LogType::INFO, 0);
			// exit
			return result;
		}
	}
	// If an exception was thrown anywhere in the compiler, we catch it here
	catch (std::exception& e)
	{
		// Log it as a fatal error and exit
		log(e.what(), LogType::FATAL, 0);
		return 1;
	}
}

///
/// @brief Creates the Castor temporary directory
///
/// @return The location of the temporary directory
///
std::string create_temp_directory()
{
	int return_code;
	// Get the location of where we want to put the temporary directory
	auto temp_dir = std::string(getenv("HOME")) + std::string("/.castor/temp/");

	// Create the directory
	return_code = system(("mkdir -p " + temp_dir).c_str());
	if (return_code) throw CastorException("Unable to make temporary directory!");

	std::string exe_path = SOURCE_DIR;

	// Copy over the arithmetic model
	return_code = system(("cp " + exe_path + "/theories/ArithmeticModel.mlw " + temp_dir).c_str());
	if (return_code) throw CastorException("Couldn't copy the theory files to the temporary directory!");
	// Copy over the memory model
	return_code = system(("cp " + exe_path + "/theories/MemoryModel.mlw " + temp_dir).c_str());
	if (return_code) throw CastorException("Couldn't copy the theory files to the temporary directory!");
	// Copy over the memory model lemmas
	return_code = system(("cp " + exe_path + "/theories/Lemmas.mlw " + temp_dir).c_str());
	if (return_code) throw CastorException("Couldn't copy the theory files to the temporary directory!");
	// Copy over the bitvector lemmas
	return_code = system(("cp " + exe_path + "/theories/BitVector.mlw " + temp_dir).c_str());
	if (return_code) throw CastorException("Couldn't copy the theory files to the temporary directory!");
	// Copy over why3find.json
	return_code = system(("cp " + exe_path + "/theories/why3find.json " + temp_dir).c_str());
	if (return_code) throw CastorException("Couldn't copy the config file to the temporary directory!");

	log("Created temp directory: " + temp_dir, LogType::INFO, 1);

	return temp_dir;
}

///
/// @brief Runs the verifiers on a string of Why3 code, reporting successes and failures
///
/// @param why3 The Why3 source code
/// @param file_name The input C++ filename
/// @param temp_dir The Castor temporary directory
/// @param action Which subcommand Castor has been called with
/// @param base The Why3 AST
///
/// @return A success/error code
///
int run_verifiers(std::string why3, std::string file_name, std::string temp_dir, InvokeAction action, std::shared_ptr<Why3::WhyNode> base)
{
	int return_code;
	// The Why3 file will just be the C++ file name with the .mlw extension appended
	auto output_file_name = file_name + ".mlw";

	// Create a temporary file
	std::ofstream tempfile(temp_dir + "temp.mlw");

	// Output the Why3 string to the temporary file and close the file
	tempfile << why3 << std::endl;
	tempfile.close();

	// Generate a "why3 pp" call
	auto why3pp_call = std::string(WHY3) + " pp " + temp_dir + "temp.mlw > " + temp_dir + output_file_name;

	// Call "why3 pp"
	return_code = system(why3pp_call.c_str());
	if (return_code) throw CastorException("Couldn't pretty-print the output code! Did I create a syntax error?");

	log("Why3 pretty-printed...", LogType::INFO, 1);

	// If --generate-whyml was used, we're done. Output the pretty-printed WhyML and exit
	if (settings.only_why3)
	{
		std::stringstream pretty_printed;
		std::ifstream infile(temp_dir + output_file_name);
		pretty_printed << infile.rdbuf();
		std::cout << std::endl << std::endl;
		std::cout << pretty_printed.str() << std::endl;
		return 0;
	}

	// If we need to prove the file, this is where the provers are called
	if (action == InvokeAction::prove || action == InvokeAction::compile)
	{
		// Create a log file
		auto log_file = temp_dir + "/output.log";
		log("Attempting to prove file...", LogType::INFO, 0);

		// Generate a "why3find prove" call
		// alternative tactics to maybe add:
		// - eliminate_definition
		// - simplify_array
		auto why3find_call = std::string(WHY3FIND) + " prove --root " + temp_dir +
					" --tactic split_vc,remove_unused -t=" + std::to_string(settings.time) +
					(settings.rerun ? " -f" : "") + " -x -s " + output_file_name + " &> " + 
					log_file;

		// Call "why3find prove"
		return_code = system(why3find_call.c_str());
		// If unsuccessful, parse the error
		if (return_code) return parse_error(log_file, base);
		log("Proved file successfully", LogType::INFO, 0);
	
		// If --smoke-tests was used, we need to smoke test the result of "why3find prove"
		if (settings.smoke_tests)
		{
			log("Smoke testing...", LogType::INFO, 0);
			// Generate a "why3 replay" call
			auto smoke_test_call = std::string(WHY3) + " replay --smoke-detector " +
						temp_dir + file_name + " -L " + temp_dir + " &>> " + log_file;

			// Call "why3 replay"
			return_code = system(smoke_test_call.c_str());
			// If unsuccessful, parse the error
			if (return_code) return parse_smoke_error(log_file, base);
			log("No smoke detected", LogType::INFO, 0);
		}
	}

	return 0;
}

///
/// @brief Unparses the SAGE into a format intended for HUMAN DEBUGGING. NOT intended to go into a compiler!
///
/// @param root The SAGE AST
/// @param filename The input C++ filename
///
/// @return The unparsed SAGE tree
///
std::string unparse_sage(SgProject* root, std::string filename)
{
	// Generate a RoseAst object
	RoseAst ast(root);

	// Iterate over the SAGE nodes
	for (auto iter = ast.begin(); iter != ast.end(); iter++)
	{
		SgNode* node = *iter;

		// If we see a template instantiation, mark for unparsing
		if (SageInterface::isTemplateInstantiationNode(node))
		{
			auto fileinfo = node->get_file_info();

			if (fileinfo)
			{
				fileinfo->setTransformation();
				fileinfo->setOutputInCodeGeneration();

				if (auto located = isSgLocatedNode(node))
				{
					located->setTransformation();
					located->setOutputInCodeGeneration();
				}
			}
		}

		// If we see a cast expression, mark for unparsing
		if (auto ast = isSgCastExp(node))
		{
			ast->unsetCompilerGenerated();
		}

		// Mark all include files for unparsing
		if (auto ast = isSgFile(node))
		{
			ast->set_unparse_includes(true);
		}
	}

	// Unparse the project
	unparseProject(root);

	// ROSE unparses to a file, so we'll read that file
	std::stringstream full_output;
	std::ifstream input_from_output("rose_" + filename);
	full_output << input_from_output.rdbuf();
	input_from_output.close();

	// Then promptly delete the file
	int result = system(("rm rose_" + filename).c_str());
	if (result)
		throw CastorException("Could not remove expected backend temp file: rose_" + filename);

	// And return the unparsed SAGE as a string
	return full_output.str();
}

///
/// @brief Parses the error output from Why3find in the case that the provers could not prove the file
///
/// @param file The location of the Why3find log file
/// @param base The Why3 AST
///
/// @return A success/error code
///
int parse_error(std::string file, std::shared_ptr<Why3::WhyNode> base)
{
	// Create a helper to look up the IR name of a Why3 name
	LookupIRName lookup_name;
	std::ifstream infile(file);
	std::string contents, part;

	// read the log file
	while (std::getline(infile, part))
		contents += part;

	// This regex matches any failed proofs and captures the @expl Why3 attribute associated with the failed proof
	auto regex = std::regex("([A-Za-z0-9_]+) \\[([A-Za-z0-9_\\ \\-\\(\\)]+)\\]");

	// Iterate over the matches
	for (std::smatch match; std::regex_search(contents, match, regex); )
	{
		std::string string = match[2];
		// Look it up in the debug table
		auto vc = debug_table.lookup(string);
		// Log to the user
		log("Failed to verify VC associated with function \"" + lookup_name(base, match[1]) + "\", VC is: " + vc, LogType::ERROR, 0);

		contents = match.suffix();
	}

	// Return an error code
	return 1;
}

///
/// @brief Parses the error output from Why3 in the case that smoke was detected
///
/// @param file The location of the Why3 log file
/// @param base The Why3 AST
///
/// @return A success/error code
///
int parse_smoke_error(std::string file, std::shared_ptr<Why3::WhyNode> base)
{
	// Create a helper to look up the IR name of a Why3 name
	LookupIRName lookup_name;
	std::ifstream infile(file);
	std::string contents, part;

	// read the log file
	while (std::getline(infile, part))
		contents += part;

	// This regex matches any failed replays and captures the Why3 name associated with the smoke
	auto regex = std::regex("goal '([A-Za-z0-9_]+)('vc)?.([0-9\.]+)', prover '[^']+': result is: Valid");

	// Iterate over the matches
	for (std::smatch match; std::regex_search(contents, match, regex); )
	{
		// Log to the user
		log("Smoke found in function or lemma \"" + std::string(lookup_name(base, match[1])) + "\" (vc:" + std::string(match[3]) + ")", LogType::ERROR, 0);

		contents = match.suffix();
	}

	// Return an error code
	return 1;
}
