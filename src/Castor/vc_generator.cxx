// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "vc_generator.hxx"
#include "whyml.hxx"
#include "helper.cxx"
#include "whygenerator.hxx"

///
/// @brief Generates bound constraints for class member variables (e.g., is_sint32)
///
/// @param type The class type in question
/// @param addr The address of the object to generate constraints for
/// @param global_offset The offset to begin offsetting from addr
/// @return The Why3 verification condition
///
std::string generate_class_bounds(std::shared_ptr<WhyClassType> type, std::shared_ptr<WhyRValue> addr, int global_offset)
{
	auto members = type->get_offset_table().get_members();
	std::string ret = "(";

	std::function<void(int)> enumerator;
	
	for (auto m : members)
	{
		auto offset = m.first;
		auto type = m.second;
	
		if (auto inte = std::dynamic_pointer_cast<WhyIntegralType>(type); inte && !std::dynamic_pointer_cast<WhyUnboundedIntegralType>(type))
		{
			ret += std::string("(is_") + (inte->get_is_signed() ? "s" : "u") + "int" + std::to_string(inte->get_bits()) + " ";
			ret += "tape[" + addr->pp() + " + " + std::to_string(offset + global_offset) + "]) /\\ ";
		}
		else if (auto clas = std::dynamic_pointer_cast<WhyClassType>(type))
		{
			ret += generate_class_bounds(clas, addr, offset + global_offset) + " /\\ ";
		}
	}

	ret += "true)";
	return ret;
}

///
/// @brief Generates common verification conditions for functions and loops.
///
/// An example of a verification condition generated here might be that two non-pointer function parameters do not alias the same value.
///
/// @param vars List of variables in play
/// @param in_loop Whether or not we're generating loop VCs (default=false)
/// @return The Why3 verification conditions
///
std::string generate_vcs(std::vector<std::shared_ptr<WhyVariable>> vars, bool in_loop)
{
	auto keyword = std::string(in_loop ? "invariant " : "requires "); // select right keyword

	std::string ret;

	for (auto v : vars)
	{
		auto size = v->get_type()->get_size();

		if (!v->get_const()) // v is not const
		{
			ret += keyword + " { [@expl:Parameter is readable and non-const] ";
			// require that the variable's memory is all readable and writable
			if (size == 1)
				ret += v->get_name() + ".addr <> 0 /\\ read.data[" + v->get_name() + ".addr] /\\ write.data[" + v->get_name() + ".addr] }\n";
			else
				ret += v->get_name() + ".addr <> 0 /\\ forall __r2wmli:int. " + v->get_name() + ".addr <= __r2wmli < " + v->get_name() + ".addr + " + std::to_string(size) + " -> read.data[__r2wmli] /\\ write.data[__r2wmli] }\n";
		}
		else // v is const
		{
			ret += keyword + " { [@expl:Parameter is readable] ";
			// require that the variable's memory is all readable
			if (size == 1)
				ret += v->get_name() + ".addr <> 0 /\\ read.data[" + v->get_name() + ".addr] }\n";
			else
				ret += v->get_name() + ".addr <> 0 /\\ forall __r2wmli:int. " + v->get_name() + ".addr <= __r2wmli < " + v->get_name() + ".addr + " + std::to_string(size) + " -> read.data[__r2wmli] }\n";
		}

		if (auto init = v->get_literal_init())
		{
			ret += keyword + " { [@expl:Global variable is set] ";
			if (std::dynamic_pointer_cast<WhyLiteral<bool>>(init))
				ret += "tape[" + v->get_name() + ".addr] = ArithmeticModel.bool_2_var (" + init->pp() + ") }\n";
			else
				ret += "tape[" + v->get_name() + ".addr] = (" + init->pp() + ") }\n";
		}
	}

	for (auto v : vars)
		// need to say something about the type of integers (that they're in-bounds)
		if (auto type = std::dynamic_pointer_cast<WhyIntegralType>(v->get_type()); type && !std::dynamic_pointer_cast<WhyUnboundedIntegralType>(v->get_type()))
			ret += keyword + " { [@expl:Integral parameter is in bounds] is_" +
				(type->get_is_signed() ? "s" : "u") +
				"int" + std::to_string(type->get_bits()) +
				" tape[" + v->get_name() + ".addr] }\n";
		// and the types of integers in class parameters
		else if (auto class_type = std::dynamic_pointer_cast<WhyClassType>(v->get_type()))
			ret += keyword + " { [@expl:Object is legally constructed]" + generate_class_bounds(class_type,
					std::make_shared<WhyAddressOfExpr>(std::make_shared<WhyPointerType>(v->get_type()),
						std::make_shared<WhyVariableReference>(v)), 0) + " }\n";


	// here we iterate through all pairs of variables and ensure that they aren't overlapping/aliasing the same value
	// this is usually fine, though has possibly unwanted side effects when the source code is using reference types
	// this might be something we need to change in the future
	for (int i = 0; i < vars.size(); i++)
	{
		for (int j = 0; j < i; j++)
		{
			// skip this for reference types
			
			// for loops, a reference parameter may alias another variable in scope
			if (in_loop && (vars[i]->get_type()->get_reference() || vars[j]->get_type()->get_reference()))
				continue;

			// for functions, a reference parameter may alias another reference parameter, but never a non-reference parameter
			if (!in_loop && (vars[i]->get_type()->get_reference() && vars[j]->get_type()->get_reference()))
				continue;

			// if they're both size 1, we can just assert that their addresses aren't equal
			if (vars[i]->get_type()->get_size() == 1 && vars[j]->get_type()->get_size() == 1)
				ret += keyword + " { [@expl:Parameters do not alias] " + vars[i]->get_name() + ".addr <> " + vars[j]->get_name() + ".addr }";
			// otherwise, we need to make sure to assert that their memory ranges don't overlap
			else
				ret += keyword + " { [@expl:Parameters do not alias] " + vars[i]->get_name() + ".addr > " + vars[j]->get_name() + ".addr + " +
					std::to_string(vars[j]->get_type()->get_size() - 1) + " \\/ " + vars[i]->get_name() + ".addr + " +
					std::to_string(vars[i]->get_type()->get_size() - 1) + " < " + vars[j]->get_name() + ".addr }";
		}
	}

	return ret;
}

///
/// @brief Generates a list of contracts based on a function's return type
///
/// For example, if we're returning an object, we need to specify how much data is coming out
///
/// @param return_type The return type of the function
/// @param is_lvalue Whether or not the function is an lvalue function
///
std::string generate_contracts(std::shared_ptr<WhyType> return_type, bool is_lvalue)
{
	if (!std::dynamic_pointer_cast<WhyClassType>(return_type) || is_lvalue)
		return "";

	auto clas = safety_cast<WhyClassType>(return_type);

	std::string ret = "ensures { [@expl:Returned object has expected size] result.length = " + std::to_string(clas->get_size()) + " }";

	return ret;
}

///
/// @brief Called if lv's size in the Why3 memory model is greater than 1
///
/// This is sometimes called by generate_valid if its lv's size is greater than 1.
/// In this case, we need to write a slightly different verification condition.
/// This can also be called by valid_array, in which case the verification condition is similar.
///
/// @param vars List of variables that currently exist
/// @param lv Variable to assert is valid
/// @param size The size of the chunk of memory to assert is valid
/// @return The Why3 clause generated
///
std::string generate_valid_contiguous(std::vector<std::shared_ptr<WhyVariable>> vars, std::shared_ptr<WhyExpression> lv, std::shared_ptr<WhyRValue> size)
{
	std::string ret, base;
	auto base_type = safety_cast<WhyPointerType>(lv->get_type())->get_base_type();

	auto varref = std::dynamic_pointer_cast<WhyVariableReference>(lv);

	if (std::dynamic_pointer_cast<WhyLValue>(lv) && !(varref && varref->get_in_quantifier()))
	{
		base = "tape[" + lv->pp() + ".addr]";
		// make sure that a pointer's address doesn't lie within the range it supposedly points to
		ret += "(((" + lv->pp() + ".addr < " + base + ") \\/ (" + lv->pp() + ".addr > " + base + " + (" + size->pp() + " - 1))) /\\ ";
	}
	else
	{
		// pointer can't point to itself if it's an rvalue
		base = lv->pp();
		ret += "(";
	}

	for (auto v : vars)
	{
		if (v->get_name() == lv->pp()) continue;

		if (v->get_type()->get_reference()) continue;
	
		auto rsize = v->get_type()->get_size();
		
		// ensure that other variables aren't aliased in the valid range
		ret += "((" + base + " + (" + size->pp() + " - 1) < " +
			v->get_name() + ".addr) \\/ (" + base + " > " +
			v->get_name() + ".addr + " + std::to_string(rsize - 1) + "))";

		ret += " /\\ ";
	}

	// if we're pointing to an integral type, we need to ensure what we're pointing to is in range
	if (auto inte = std::dynamic_pointer_cast<WhyIntegralType>(base_type); inte && !std::dynamic_pointer_cast<WhyUnboundedIntegralType>(base_type))
	{
		ret += "(forall __r2wmli:int. " + base + " <= __r2wmli < " + base + " + " + size->pp() + " -> " +
			std::string("(is_") + (inte->get_is_signed() ? "s" : "u") + "int" + std::to_string(inte->get_bits()) + " tape[__r2wmli])) /\\";
	}

	// ensure that the whole range is valid
	ret += "(forall __r2wmli:int. " + base + " <= __r2wmli < " + base + " + " + size->pp() + " -> read.data[__r2wmli] /\\ ";

	// if lv is a variable reference to non-const variable, ensure it's writeable
	if (!base_type->get_const())
		ret += "write.data[__r2wmli]) /\\ ";

	// if lv is a pointer to a class, make sure we generate the class bounds
	if (auto class_type = std::dynamic_pointer_cast<WhyClassType>(base_type))
		ret += generate_class_bounds(class_type, WhyGenerator::makeRValue(lv), 0) + " /\\ ";

	// ensure that the start address is nonnegative
	ret += "(0 < " + base + "))";

	return ret;
}

///
/// @brief Generate the Why3 for a `valid` clause
///
/// @param vars List of variables that currently exist
/// @param lv Variable to assert is valid
/// @return The Why3 clause generated
///
std::string generate_valid(std::vector<std::shared_ptr<WhyVariable>> vars, std::shared_ptr<WhyExpression> lv)
{
	std::string ret, base;

	// if the size of what we're pointing to is >1, we instead call generate_valid_contiguous
	if (auto size = safety_cast<WhyPointerType>(lv->get_type())->get_base_type()->get_size(); size > 1)
		return generate_valid_contiguous(vars, lv, std::make_shared<WhyLiteral<int32_t>>(std::make_shared<WhyS32Type>(), size));

	auto varref = std::dynamic_pointer_cast<WhyVariableReference>(lv);

	if (std::dynamic_pointer_cast<WhyLValue>(lv) && !(varref && varref->get_in_quantifier()))
	{
		// if lvalue, pointer should not point to itself
		base = "tape[" + lv->pp() + ".addr]";
		ret += "((" + lv->pp() + ".addr <> " + base + ") /\\ ";
	}
	else
	{
		// pointer can't point to itself if it's an rvalue
		base = lv->pp();
		ret += "(";
	}

	for (auto v : vars)
	{
		if (v->get_name() == lv->pp()) continue;

		if (v->get_type()->get_reference()) continue;
	
		auto size = v->get_type()->get_size();
	
		// ensure that other variables aren't aliased in the valid range
		ret += "((" + base + " < " + v->get_name() + ".addr) \\/ (" + base + " > " + v->get_name() + ".addr + " + std::to_string(size - 1) + "))";

		ret += " /\\ ";
	}
	
	auto base_type = safety_cast<WhyPointerType>(lv->get_type())->get_base_type();

	// if we're pointing to an integral type, we need to ensure what we're pointing to is in range
	if (auto inte = std::dynamic_pointer_cast<WhyIntegralType>(base_type); inte && !std::dynamic_pointer_cast<WhyUnboundedIntegralType>(base_type))
	{
		ret += std::string("(is_") + (inte->get_is_signed() ? "s" : "u") + "int" +
			std::to_string(inte->get_bits()) + " tape[" + base + "]) /\\";
	}

	// ensure the data is readable
	ret += "read.data[" + base + "] /\\ ";
	
	// if non-const, ensure it's writeable
	if (!base_type->get_const())
		ret += "write.data[" + base + "] /\\ ";

	// if lv is a pointer to a class, make sure we generate the class bounds
	if (auto class_type = std::dynamic_pointer_cast<WhyClassType>(safety_cast<WhyPointerType>(lv->get_type())->get_base_type()))
		ret += generate_class_bounds(class_type, WhyGenerator::makeRValue(lv), 0) + " /\\ ";

	// ensure the starting address is non-zero
	ret += "0 < " + base + ")";

	return ret;
}

///
/// @brief Generates a separated clause
///
/// @param vars List of variables that currently exist
/// @param lvs List of variables to assert are separated
/// @return The Why3 clause generated
///
std::string generate_separated(std::vector<std::shared_ptr<WhyVariable>> vars, std::vector<std::shared_ptr<WhyExpression>> lvs)
{
	std::string ret = "(";

	// iterate through all pairs of variables in the separated clause
	for (int i = 0; i < lvs.size(); i++)
		for (int j = 0; j < i; j++)
		{
			std::string basei, basej;
			if (std::dynamic_pointer_cast<WhyLValue>(lvs[i]))
				basei = "tape[" + lvs[i]->pp() + ".addr]";
			else
				basei = "(" + lvs[i]->pp() + ")";
			if (std::dynamic_pointer_cast<WhyLValue>(lvs[j]))
				basej = "tape[" + lvs[j]->pp() + ".addr]";
			else
				basej = "(" + lvs[j]->pp() + ")";

			// get the size of what each variable is pointing to
			auto isize = safety_cast<WhyPointerType>(lvs[i]->get_type())->get_base_type()->get_size();
			auto jsize = safety_cast<WhyPointerType>(lvs[j]->get_type())->get_base_type()->get_size();

			// ensure that the ranges don't overlap
			// if both pointed-to sizes are 1, using <> is sufficient
			if (isize == 1 && jsize == 1)
				ret += "(" + basei + " <> " + basej + ") /\\ ";
			else
				ret += "(" + basei + " + " + std::to_string(isize - 1) + " < " + basej + " \\/ " +
					     basei + " > " + basej + " + " + std::to_string(jsize - 1) + ") /\\ ";
		}

	ret += "true)"; // the string ends in /\, so just add a "true" on the end
	return ret;
}

///
/// @brief Generates either a `writes` or a `frees` clause
///
/// @param lvs List of variables to assert we're writing to or freeing
/// @param in_loop Whether or not this clause is for a loop
/// @param to_write Whether or not we're generating a `writes` or a `frees`
/// @param debug_str The debug identifier to propogate Why3 results back to the user
/// @return The Why3 clause generated
///
std::string generate_writes(std::vector<std::shared_ptr<WhyLValue>> lvs, bool in_loop, WritesGenerator to_write, std::string debug_str)
{
	std::string ret = in_loop ? "invariant " : "ensures ";
	ret += " { [@expl:" + debug_str + "] forall __r2wmli:int. (";

	// iterate through the list of variables
	for (auto lv : lvs)
	{
		auto size = lv->get_type()->get_size();

		// if the variable is an array range, let it generate its own writes clause
		if (auto arlv = std::dynamic_pointer_cast<WhyArrayRange>(lv))
			ret += arlv->generate_writes();
		// if it's a size 1, this generates simpler VCs
		else if (size == 1)
		{
			//if (!in_loop)
			//	ret += "__r2wmli <> old(" + WhyAddressOfExpr(nullptr, lv).pp() + ") /\\ ";
			//else
				ret += "__r2wmli <> " + WhyAddressOfExpr(nullptr, lv).pp() + " /\\ ";
		}
		// make sure that __r2wmli doesn't include this variable
		else if (size > 1)
		{
			//if (!in_loop)
			//	ret += "(__r2wmli < old(" + WhyAddressOfExpr(nullptr, lv).pp() + ") \\/ __r2wmli > old(" + 
			//		WhyAddressOfExpr(nullptr, lv).pp() + ") + " + std::to_string(size - 1) + ") /\\ ";
			//else
				ret += "(__r2wmli < " + WhyAddressOfExpr(nullptr, lv).pp() + " \\/ __r2wmli > " + 
					WhyAddressOfExpr(nullptr, lv).pp() + " + " + std::to_string(size - 1) + ") /\\ ";
		}
	}

	if (to_write == GenerateWrites)
	{
		// assert that all remaining values of __r2wmli aren't modified
		if (in_loop)
			ret += "(read.data[__r2wmli] at LoopStart)) -> tape[__r2wmli] = (tape[__r2wmli] at LoopStart) }";
		else
			ret += "(old read.data[__r2wmli])) -> tape[__r2wmli] = old tape[__r2wmli] }";
	}
	else
	{
		// assert that all remaining values of __r2wmli don't change their valid status
		if (in_loop)
			ret += "(read.data[__r2wmli] at LoopStart)) -> read.data[__r2wmli] /\\ write.data[__r2wmli] = write.data[__r2wmli] at LoopStart }";
		else
			ret += "(old read.data[__r2wmli])) -> read.data[__r2wmli] /\\ write.data[__r2wmli] = old write.data[__r2wmli] }";
	}

	ret += "\n";

	return ret;
}

