// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

// traversal.cxx
#include "irgenerator.hxx"
#include "class_table.hxx"
#include "function_table.hxx"
#include "settings.hxx"
#include "types.hxx"
#include "parser.hxx"
#include "helper.cxx"
#include "messaging.hxx"
#include <AstProcessing.h>
#include <iostream>

FunctionTable function_table; ///< Function table object
ClassTable class_table;       ///< Class table object

extern Settings settings;

bool isAnyReferenceType(SgType* t)
{
	if (isSgReferenceType(t))
		return true;
	if (isSgRvalueReferenceType(t))
		return true;
	if (isSgTypedefType(t))
		return isAnyReferenceType(isSgTypedefType(t)->get_base_type());
	if (isSgModifierType(t))
		return isAnyReferenceType(isSgModifierType(t)->get_base_type());
	return false;
}

///
/// @brief Functor, so that instances of this class can be used as a function
///
/// @param project SAGE AST
/// @return IR AST
///
std::shared_ptr<IRNode> IRGenerator::operator()(SgNode* project)
{
	auto result = traverse(project, { Nothing, nullptr, nullptr, false, false, false } );
	
	if (auto program = std::dynamic_pointer_cast<IRProgram>(result)) // if we created a program
	{
		auto globals = program->get_globals(); // we need to get the globals
		auto scope = program->get_scope();     // and the global scope
		tidy_ast(globals);		       // and clean up the AST
		auto tidied_program = std::make_shared<IRProgram>(globals, scope); // before returning the new program
		return tidied_program;
	}
	else
	{
		return result; // otherwise we just return what we had
	}
}

///
/// @brief Downwards traversal.
///
/// @param astNode SAGE node to look at
/// @param inheritedValue The inherited values
/// @return The new inherited values to pass on down
///
IRGeneratorIV IRGenerator::evaluateInheritedAttribute(SgNode* astNode, IRGeneratorIV inheritedValue)
{
	// these are commonly used, so we'll copy them to locals for brevity later on
	auto scope = inheritedValue.current_scope;
	auto new_type = inheritedValue.new_type;
	auto is_ref = inheritedValue.is_ref;
	auto class_var = inheritedValue.class_var;

	if (auto sage_scope = isSgScopeStatement(astNode)) // any time we encounter a SAGE scope statement
	{
		scope = std::make_shared<R2WML_Scope>(scope); // we need to create a corresponding IR scope
		R2WML_Scope::register_sage_scope(scope, sage_scope); // and register the SAGE scope
	}
	else if (auto decl = isSgFunctionDeclaration(astNode)) // these have to be handled specially, since a function declaration isn't a scope
	{
		auto sage_scope = decl->get_functionParameterScope();

		scope = std::make_shared<R2WML_Scope>(scope);
		R2WML_Scope::register_sage_scope(scope, sage_scope);
	}

	if (isSgClassDefinition(astNode))
		class_var = true;
	else if (isSgFunctionDeclaration(astNode))
		class_var = false;

	if (auto ast = isSgNewExp(astNode)) // if we're parsing a `new` expression, we need to capture its type information and pass it down
					    // trust me-- this makes things easier later
	{
		new_type = getIRTypeFromSgType(ast->get_specified_type());
	}

	if (auto ast = isSgPragma(astNode)) // if we see a pragma, we need to make a note on if its associated function returns a reference or not
	{
		SgFunctionDeclaration* associated_function;

		if (find_function(ast, associated_function)) // find the associated function
		{
			is_ref = isAnyReferenceType(associated_function->get_type()->get_return_type()); // check if it returns a reference
		}
	}

	if (auto ast = isSgMemberFunctionDeclaration(astNode)) // we need to add `this` to the symbol table if we see a member function
	{
		if (!SageInterface::isStatic(ast->get_firstNondefiningDeclaration()) && !SageInterface::isStatic(ast)) // ...except if it's static
		{
			auto name = scope->get_unique_name("this");
			auto type = getIRTypeFromSgType(safety_cast_raw<SgClassDeclaration*>(
							ast->get_associatedClassDeclaration())->get_type()); // get the class type

			if (safety_cast_raw<SgMemberFunctionType*>(ast->get_type())->isConstFunc()) // check if const function
				type = std::make_shared<IRConstType>(type); // make the "this" pointer a pointer-to-const

			type = std::make_shared<IRPointerType>(type); // create the actual pointer type

			scope->register_variable("this", std::make_shared<IRVariable>(type, name, "this", true)); // this gets the proper type for `this` and adds it to the symbol table
		}
	}

	if (isSgFunctionParameterList(astNode)) // pass down a FunctionParameterList
	{
		return { FunctionParameterList, scope, new_type, is_ref, inheritedValue.is_constexpr, class_var };
	}
	else if (auto var = isSgVariableDeclaration(astNode)) // pass down a VariableDeclaration
	{
		return { VariableDeclaration, scope, new_type, is_ref, var->get_is_constexpr(), class_var };
	}
	else // for anything else, just copy inheritedValue.status
	{
		return { inheritedValue.status, scope, new_type, is_ref, inheritedValue.is_constexpr, class_var };
	}
}

///
/// @brief Upwards traversal.
///
/// @param astNode SAGE node to look at
/// @param inheritedValue The inherited values
/// @param list Synthesized attributes so far
/// @return The new synthesized value
///
std::shared_ptr<IRNode> IRGenerator::evaluateSynthesizedAttribute(SgNode* astNode, IRGeneratorIV inheritedValue, SynthesizedAttributesList list)
{
	auto scope = inheritedValue.current_scope; // copy the scope for easy reference later

	if (!astNode) // this shouldn't trigger, but I've seen it before
		      // return an empty node if astNode is null
	{
		return std::make_shared<IREmptyNode>();
	}

	if (auto ast = isSgProject(astNode)) // for SgProject, we just pass up the last value
	{
		if (list.size())
			return list[0];
		else
			return std::make_shared<IREmptyNode>();
	}
	else if (auto ast = isSgClassDefinition(astNode))
	{
		auto name = ast->get_qualified_name();

		std::vector<std::shared_ptr<IRVariableDeclarationStmt>> vars;
		std::vector<std::shared_ptr<IRFunction>> funcs;

		for (int i = 0; i < list.size(); i++)
		{
			if (auto node = std::dynamic_pointer_cast<IRVariableDeclarationStmt>(list[i])) // push a member variable to the `vars` vector
			{
				vars.push_back(node);
			}
			else if (auto node = std::dynamic_pointer_cast<IRFunction>(list[i])) // push a member function to the `funcs` vector
			{
				funcs.push_back(node);
			}
			else if (auto vc = std::dynamic_pointer_cast<IRLoopInvariant>(list[i])) // this looks like a loop invariant, but is really the
												// `invariant` used on functions
												// this takes some special care
			{
				auto require = std::make_shared<IRRequires>(vc->get_vc()); // parse the requires clause
				auto ensure = std::make_shared<IREnsures>(vc->get_vc());   // parse the ensures clause
				auto str = vc->get_str();
				auto require_str = "castor requires" + str.substr(str.find("invariant") + 9); // get the requires clause string
				auto ensure_str = "castor ensures" + str.substr(str.find("invariant") + 9);   // get the ensures clause string
				require->attach_str(require_str); // set them
				ensure->attach_str(ensure_str);

				attach_ver(require, i + 1, list); // attach them to the next function
				attach_ver(ensure, i + 1, list);
			}
			else if (auto vc = std::dynamic_pointer_cast<IRLemma>(list[i]))
			{
				throw CastorException("Cannot declare a lemma within a class!");
			}
			else if (auto node = std::dynamic_pointer_cast<IRVerificationCondition>(list[i])) // for all other verification conditions
			{
				attach_ver(node, i + 1, list); // ...we just attach them to the next function
			}
		}

		if (ast->get_inheritances().size() > 1) // if more than 1 base class, throw an exception
			throw UnsupportedFeatureException("Multiple inheritance");
		else if (ast->get_inheritances().size() == 1) // if this is a derived class
		{
			auto inherited_class_name = ast->get_inheritances()[0] // get the class it inherits from
				->get_base_class()
				->get_qualified_name();                        // and get its name
			auto inherited_members = class_table.get_variables(inherited_class_name); // look up the inherited members in the table
			class_table.set_parent(name, inherited_class_name);

			for (int i = inherited_members.size() - 1; i >= 0; i--) // iterate through the inherited members
										// this is done in reverse order so that if any variables
										// are shadowed, the most recent one is registered first
			{
				scope->register_variable(inherited_members[i]->get_var()->get_frontend_name(),
						inherited_members[i]->get_var()); // register them in the symbol table
			}

			vars.insert(vars.begin(), inherited_members.begin(), inherited_members.end()); // insert them into this class's members
		}

		// if the class has no member variables, it has size 0
		// the memory model doesn't support size 0 data, so we add a placeholder boolean
		// in order to pad the class out
		if (vars.size() == 0)
			vars.push_back(std::make_shared<IRVariableDeclarationStmt>(
						std::make_shared<IRVariable>(
							std::make_shared<IRBoolType>(),
							"__struct_non_zero_size_placeholder",
							"__struct_non_zero_size_placeholder",
							true),
						nullptr, false));

		auto class_obj = std::make_shared<IRClass>(vars, funcs, name, scope); // we have a class!
		class_table[name] = class_obj; // register it with the class table
		return class_obj;
	}
	else if (auto ast = isSgFunctionDeclaration(astNode)) // this handles function declarations
	{
		if (settings.version == Settings::Version::cpp17 && ast->get_is_constexpr())
			throw UnsupportedFeatureException("Constexpr-qualified functions"); // we don't support constexpr functions

		if (ast->get_functionModifier().isVirtual())
			throw UnsupportedFeatureException("Virtual functions"); // we don't support virtual functions
	
		if (ast->get_functionModifier().isMarkedDelete()) // default-deleted constructors disappear
			return std::make_shared<IREmptyNode>();

		// first, let's get the function's data
		auto block = std::dynamic_pointer_cast<IRStatementBlock>(list[2]);
		auto var_list = safety_cast<IRVariableList>(list[0]);
		auto name = get_func_name(ast->get_qualified_name(), var_list->get_vars(), ast->get_mangled_name());

		auto type = ast->get_type()->get_return_type();

		if (auto member = isSgMemberFunctionDeclaration(astNode)) // if it's a member function
		{
			// ... and it's not static
			if (!SageInterface::isStatic(member->get_firstNondefiningDeclaration()) && !SageInterface::isStatic(member))
			{
				// we add `this` to its parameter list
				auto thisvar = scope->lookup("this");
				var_list->add_var(thisvar, 0);
			}
		}

		std::vector<std::shared_ptr<IRVariable>> template_args;

		// if it's a template instantiation, we need to call getTemplateArgs, check the documentation over there
		if (auto t = isSgTemplateInstantiationFunctionDecl(astNode))
			template_args = getTemplateArgs(t, scope); 
		else if (auto t = isSgTemplateInstantiationMemberFunctionDecl(astNode))
			template_args = getTemplateArgs(t, scope);
		// if it's a template non-instantiation, we need to call getTemplateParams, check the documentation over there
		else if (auto t = isSgTemplateFunctionDeclaration(astNode))
			template_args = getTemplateParams(t); 
		else if (auto t = isSgTemplateMemberFunctionDeclaration(astNode))
			template_args = getTemplateParams(t); 

		std::vector<bool> refs;

		// check each parameter to see if it's a reference
		for (auto name : ast->get_parameterList()->get_args())
			refs.push_back(isAnyReferenceType(name->get_type()));

		bool is_ref = isAnyReferenceType(type); // check the function's return value for a reference

		auto return_type = getIRTypeFromSgType(type);

		//if (std::dynamic_pointer_cast<IRClassType>(return_type) && !is_ref)
		//	throw CastorException("Cannot yet return objects from functions!");

		// make the function!
		auto ret = std::make_shared<IRFunction>(
			name, var_list, block,
			return_type,
			isSgTemplateFunctionDeclaration(astNode) || isSgTemplateMemberFunctionDeclaration(astNode),
			isSgTemplateInstantiationFunctionDecl(astNode) || isSgTemplateInstantiationMemberFunctionDecl(astNode),
			template_args, scope, is_ref);

		function_table.register_function(name, refs, is_ref, ret); // register the function in the function table

		return ret;
	}
	else if (auto ast = isSgPragma(astNode)) // if we see a pragma
	{
		std::string raw_pragma = ast->get_pragma(); // get the string
		
		log("Parsing verification condition: " + raw_pragma, LogType::INFO, 2);

		// ...and parse it
		auto parsed = VCParser::parse_ver(raw_pragma, inheritedValue.is_ref ? VCParser::LValueFunc : VCParser::RValueFunc);

		if (auto vc = std::dynamic_pointer_cast<IRVerificationCondition>(parsed)) // if it's a traditional verification conditions
		{
			vc->attach_str(raw_pragma); // attach the raw string to it and return
			return vc;
		}
		else if (auto vc = std::dynamic_pointer_cast<IRAssert>(parsed)) // if it's an assert
		{
			vc->attach_str(raw_pragma); // attach the raw string to it and return
			return vc;
		}
		else
		{
			return safety_cast<IRStatement>(parsed); // this should be another kind of statement VC
		}
	}
	else if (auto ast = isSgConstructorInitializer(astNode)) // constructor initializer
	{
		if (auto clas = ast->get_class_decl()) // check if we're creating a class
		{
			if (auto decl = ast->get_declaration()) // check for the constructor declaration
			{
				auto params = safety_cast<__IRExpressionListWrapper>(list[0])->params; // get the parameters
				auto name = get_func_name(decl->get_qualified_name(), params, decl->get_mangled_name()); // get the constructor's name
				auto ret = std::make_shared<IRFunctionCallExpr<IRRValue>>(
					getIRTypeFromSgType(clas->get_type()),
					std::make_shared<IRFunctionRefExpr>(name), params); // create a function call to the constructor
				ret->set_is_constructor(); // mark it as a constructor
				return ret;
			}
			else // we have a class but not an explicit constructor
			{
				throw CastorException("Need to explicitly write constructor for " +
						safety_cast_raw<SgClassType*>(ast->get_type())->get_name() + " initialization!");
			}
		}
		else // we don't have a class
		{
			auto type = inheritedValue.new_type; // get the type

			// strip const qualifiers
			while (auto const_type = std::dynamic_pointer_cast<IRConstType>(type))
				type = const_type->get_base_type();

			// get the params
			auto params = list.size() ? safety_cast<__IRExpressionListWrapper>(list[0])->params : std::vector<std::shared_ptr<IRExpression>>();

			// if we're creating a new integral or boolean, we either pass up the value given or a default value
			if ((std::dynamic_pointer_cast<IRIntegralType>(type) || std::dynamic_pointer_cast<IRBoolType>(type)) && params.size())
				return params[0];
			else if (std::dynamic_pointer_cast<IRIntegralType>(type))
				return std::make_shared<IRS32Literal>(0);
			else if (std::dynamic_pointer_cast<IRBoolType>(type))
				return std::make_shared<IRBoolLiteral>(false);
			else if (!type)
				throw CastorException("Received null type when trying to figure out how to construct object!");
			else // not an integral or boolean... what type is it?
				throw CastorException("I don't know how to construct this object! Has type " + type->pp());
		}
	}
	else if (auto ast = isSgIfStmt(astNode)) // if statements are easy
	{
		std::shared_ptr<IRExpression> cond;
		std::shared_ptr<IRVariableDeclarationStmt> decl;
		if (!(cond = std::dynamic_pointer_cast<IRExpression>(list[0]))) // sometimes the condition is a statement, we need an expression
		{
			if (auto exprstmt = std::dynamic_pointer_cast<IRExpressionStmt>(list[0])) // check if we have an expression statement
				cond = exprstmt->get_expr();
			else if (decl = std::dynamic_pointer_cast<IRVariableDeclarationStmt>(list[0])) // otherwise we should have a variable declaration statement
			{
				cond = std::make_shared<IRVariableReference>(decl->get_var()); // get the variable reference

				if (!std::dynamic_pointer_cast<IRBoolType>(cond->get_type())) // if it's not a boolean type
					cond = std::make_shared<IRCast<IRRValue>>(std::make_shared<IRBoolType>(), cond); // insert a conversion
			}
		}
		auto then = safety_cast<IRStatement>(list[1]);
		auto els = std::dynamic_pointer_cast<IRStatement>(list[2]);

		auto ifstatement = std::make_shared<IRIfStatement>(cond, then, els);

		if (!decl) // if we don't have a variable declaration
		{
			return ifstatement; // just return the if statement
		}
		else // otherwise
		{
			std::vector<std::shared_ptr<IRStatement>> stmts({ decl, ifstatement });   // collect our statements
			return std::make_shared<IRStatementBlock>(stmts, scope); // and return a statement block
		}
	}
	else if (auto ast = isSgWhileStmt(astNode)) // while loops are easy
	{
		std::shared_ptr<IRExpression> cond;
		std::shared_ptr<IRVariableDeclarationStmt> decl;
		if (!(cond = std::dynamic_pointer_cast<IRExpression>(list[0]))) // sometimes the condition is a statement, we need an expression
		{
			if (auto exprstmt = std::dynamic_pointer_cast<IRExpressionStmt>(list[0])) // check if we have an expression statement
				cond = exprstmt->get_expr();
			else if (decl = std::dynamic_pointer_cast<IRVariableDeclarationStmt>(list[0])) // otherwise we should have a variable declaration statement
			{
				cond = std::make_shared<IRAssignOp>(decl->get_var()->get_type(),
						std::make_shared<IRVariableReference>(decl->get_var()),
						decl->get_initial_value()); // get the condition (in the form "x = a" given "T x = a")

				if (!std::dynamic_pointer_cast<IRBoolType>(cond->get_type())) // if it's not a boolean type
					cond = std::make_shared<IRCast<IRRValue>>(std::make_shared<IRBoolType>(), cond); // insert a conversion

				decl = std::make_shared<IRVariableDeclarationStmt>(decl->get_var(), nullptr, decl->get_is_static()); // re-create the variable declaration without an initializer
			}
		}
		auto body = safety_cast<IRStatement>(list[1]);

		auto loopstatement = std::make_shared<IRLoopStmt>(scope, cond, body);

		if (!decl) // if we don't have a variable declaration
		{
			return loopstatement; // just return the while statement
		}
		else // otherwise
		{
			std::vector<std::shared_ptr<IRStatement>> stmts({ decl, loopstatement });   // collect our statements
			return std::make_shared<IRStatementBlock>(stmts, scope); // and return a statement block
		}
	}
	else if (auto ast = isSgDoWhileStmt(astNode))
	{
		auto body = safety_cast<IRStatement>(list[0]);
		auto cond = std::dynamic_pointer_cast<IRExpression>(list[1]);
		if (!cond)
			cond = safety_cast<IRExpressionStmt>(list[1])->get_expr(); // unlike similar idioms in for/while, variable declarations are not permitted
										   // in do-while. nevertheless, we receive an expression statement, so we're just
										   // going to pull the inner expression out

		auto loop = std::make_shared<IRLoopStmt>(scope, cond, body);
		std::vector<std::shared_ptr<IRStatement>> stmts({ body, loop });
		return std::make_shared<IRStatementBlock>(stmts, scope);
	}
	else if (isSgForInitStatement(astNode)) // just pass up the synthesized value
	{
		return list[0];
	}
	else if (auto ast = isSgForStatement(astNode)) // for loops!
	{
		std::shared_ptr<IRExpression> cond;
		std::shared_ptr<IRVariableDeclarationStmt> decl;
		std::shared_ptr<IRStatement> pre;
		std::shared_ptr<IRStatement> post;
		std::shared_ptr<IRStatement> body;
		if (!(cond = std::dynamic_pointer_cast<IRExpression>(list[1]))) // sometimes the condition is a statement, we need an expression
		{
			if (auto exprstmt = std::dynamic_pointer_cast<IRExpressionStmt>(list[1])) // check if we have an expression statement
				cond = exprstmt->get_expr();
			else if (decl = std::dynamic_pointer_cast<IRVariableDeclarationStmt>(list[1])) // otherwise we should have a variable declaration statement
			{
				cond = std::make_shared<IRAssignOp>(decl->get_var()->get_type(),
						std::make_shared<IRVariableReference>(decl->get_var()),
						decl->get_initial_value()); // get the condition (in the form "x = a" given "T x = a")

				if (!std::dynamic_pointer_cast<IRBoolType>(cond->get_type())) // if it's not a boolean type
					cond = std::make_shared<IRCast<IRRValue>>(std::make_shared<IRBoolType>(), cond); // insert a conversion

				decl = std::make_shared<IRVariableDeclarationStmt>(decl->get_var(), nullptr, decl->get_is_static()); // re-create the variable declaration without an initializer
			}
		}
		if (!(pre = std::dynamic_pointer_cast<IRStatement>(list[0])))
			pre = std::make_shared<IRExpressionStmt>(safety_cast<IRExpression>(list[0])); // if the pre or post aren't statements, make them statements
		if (!(post = std::dynamic_pointer_cast<IRStatement>(list[2])))
			post = std::make_shared<IRExpressionStmt>(safety_cast<IRExpression>(list[2]));

		if (!post)
			body = safety_cast<IRStatement>(list[3]); // if there is no post, make the body the synthesized value
		else
			// if there is a post, we need to make a new statement block
			body = std::make_shared<IRStatementBlock>(std::vector<std::shared_ptr<IRStatement>>({ safety_cast<IRStatement>(list[3]), post }), scope);

		auto loopstatement = std::make_shared<IRLoopStmt>(scope, cond, body);

		std::vector<std::shared_ptr<IRStatement>> stmts; // this is the statements to create a new block out of

		if (pre)
			stmts.push_back(pre); // add pre, if it's set
		if (decl)
			stmts.push_back(decl); // add decl, if it's set
		stmts.push_back(loopstatement); // add the loop statement

		return std::make_shared<IRStatementBlock>(stmts, scope); // return the new statement block
	}
	else if (auto ast = isSgRangeBasedForStatement(astNode)) // range-based for loops!
	{
		auto pre_1 = safety_cast<IRVariableDeclarationStmt>(list[1]); // auto& __range = container
		auto pre_2 = safety_cast<IRVariableDeclarationStmt>(list[2]); // auto __begin = __range.begin()
		auto pre_3 = safety_cast<IRVariableDeclarationStmt>(list[3]); // auto __end = __range.end()
		auto cond = safety_cast<IRExpression>(list[4]);               // __begin != __end
		auto post = safety_cast<IRExpression>(list[5]);               // ++__begin
		auto iter = safety_cast<IRVariableDeclarationStmt>(list[0]);  // auto& item = *__begin
		auto body = safety_cast<IRStatement>(list[6]);

		static int __range_ctr = 0; // because we're replacing variables in the symbol table, and because we're creating these bottom-up,
		static int __begin_ctr = 0; // we can't rely on the symbol table for name mangling, since the symbol table relies on mangling variable
		static int __end_ctr = 0;   // names from the top-down.

		auto __range = IRVariable(pre_1->get_var()->get_type(), "__range_" + std::to_string(__range_ctr++), "__range", true);
		auto __begin = IRVariable(pre_2->get_var()->get_type(), "__begin_" + std::to_string(__begin_ctr++), "__begin", true);
		auto __end   = IRVariable(pre_3->get_var()->get_type(), "__end_"   + std::to_string(__end_ctr++),   "__end",   true);

		if (!std::dynamic_pointer_cast<IRPointerType>(__begin.get_type()))
			throw UnsupportedFeatureException("Range-based for loops with non-pointer iterators");

		auto __range_oldname = pre_1->get_var()->get_frontend_name();
		auto __begin_oldname = pre_2->get_var()->get_frontend_name();
		auto __end_oldname   = pre_3->get_var()->get_frontend_name();

		*(pre_1->get_var()) = __range;
		*(pre_2->get_var()) = __begin;
		*(pre_3->get_var()) = __end;

		std::vector<std::shared_ptr<IRStatement>> body_stmts({ iter, body, std::make_shared<IRExpressionStmt>(post) });
		auto full_body = std::make_shared<IRStatementBlock>(body_stmts, scope);

		auto symbol_table = scope->get_symbol_table();
		auto parent_scope = scope->get_parent_scope();

		auto loop_scope = std::make_shared<R2WML_Scope>(parent_scope);

		for (const auto& variable_pair : symbol_table) // see earlier, "iter" doesn't exist yet, and "pre_1" is an alias
							       // we need to remove them from the scope
		{
			auto variable_name = variable_pair.first == __range_oldname ? __range.get_frontend_name() :
						variable_pair.first == __begin_oldname ? __begin.get_frontend_name() :
						variable_pair.first == __end_oldname ? __end.get_frontend_name() :
						variable_pair.first;
			if (variable_name != iter->get_var()->get_frontend_name())
				loop_scope->register_variable(variable_name, variable_pair.second);
		}

		if (auto scoped = std::dynamic_pointer_cast<IRScopedStatement>(body))
			scoped->get_scope()->set_parent_scope(loop_scope);

		auto loop = std::make_shared<IRLoopStmt>(loop_scope, cond, full_body);

		auto vc1 = safety_cast<IRLoopVariant>(VCParser::parse_ver("castor variant __end - __begin", VCParser::RValueFunc));
		vc1->attach_str("Range-based for loop termination clause");

		auto vc2 = safety_cast<IRLoopInvariant>(VCParser::parse_ver("castor invariant __begin == __end \\/ valid(__begin)", VCParser::RValueFunc));
		vc2->attach_str("Range-based for loop valid iterator clause");

		auto vc3_quantvar = std::make_shared<IRVariable>(pre_2->get_var()->get_type(), "__r2wmli", "__r2wmli", false);
		// forall int: __r2wmli. __begin <= __r2wmli /\ __r2wmli < __end => valid(__r2wmli)
		auto vc3 = std::make_shared<IRLoopInvariant>(std::make_shared<IRForall>(std::vector<std::shared_ptr<IRVariable>>({ vc3_quantvar }),
						std::make_shared<IRImpliesOp>(
							std::make_shared<IRBoolType>(),
							std::make_shared<IRAndOp>(
								std::make_shared<IRBoolType>(),
								std::make_shared<IRLessEqualsOp>(
									std::make_shared<IRBoolType>(),
									std::make_shared<IRVariableReference>(pre_2->get_var()),
									std::make_shared<IRVariableReference>(vc3_quantvar)),
								std::make_shared<IRLessThanOp>(
									std::make_shared<IRBoolType>(),
									std::make_shared<IRVariableReference>(vc3_quantvar),
									std::make_shared<IRVariableReference>(pre_3->get_var()))),
							std::make_shared<IRFunctionCallExpr<IRRValue>>(
								std::make_shared<IRUnknownType>(),
								std::make_shared<IRFunctionRefExpr>("valid"),
								std::vector<std::shared_ptr<IRExpression>>({ std::make_shared<IRVariableReference>(vc3_quantvar) })))));
		/*auto vc3 = safety_cast<IRLoopInvariant>(VCParser::parse_ver("castor invariant forall int: __r2wmli. __begin <= __r2wmli /\ __r2wmli < __end => valid(__r2wmli)",
					VCParser::RValueFunc));*/
		vc3->attach_str("Range-based for loop valid range clause");

		auto vc4 = safety_cast<IRLoopInvariant>(VCParser::parse_ver("castor invariant at(__begin, @LoopStart) <= __begin /\\ __begin <= __end", VCParser::RValueFunc));
		vc4->attach_str("Range-based for loop iterator bounds clause");

		auto vc5 = safety_cast<IRLoopInvariant>(VCParser::parse_ver("castor invariant __end == at(__end, @LoopStart)", VCParser::RValueFunc));
		vc5->attach_str("Range-based for loop end iterator is immutable clause");

		loop->add_vc(vc1);
		loop->add_vc(vc3);
		loop->add_vc(vc4);
		loop->add_vc(vc5);
		loop->add_vc(vc2);

		std::vector<std::shared_ptr<IRStatement>> stmts({ pre_1, pre_2, pre_3, loop });

		auto ret = std::make_shared<IRStatementBlock>(stmts, scope);
		return ret;
	}
	else if (auto ast = isSgPntrArrRefExp(astNode)) // indexing into an array
	{
		// There is a ROSE bug which pops up here, where indexing into an array like "0[arr]" results
		// in the incorrect type being returned from the "ast" pointer. This will sometimes cause an
		// exception elsewhere in Castor as part of its internal AST consistency checking.
		//
		// Conceivably, this could result in totally incorrect Why3 code generation...

		auto sum = std::make_shared<IRAdditionOp>(
				std::make_shared<IRPointerType>(getIRTypeFromSgType(ast->get_type())),
				safety_cast<IRExpression>(list[0]), safety_cast<IRExpression>(list[1]));

		return std::make_shared<IRPointerDereference>(getIRTypeFromSgType(ast->get_type()), sum);
	}
	else if (auto ast = isSgSizeOfOp(astNode)) // sizeof
	{
		std::shared_ptr<IRType> type;

		if (auto operand = ast->get_operand_type())
			type = getIRTypeFromSgType(operand);
		else if (auto expr = ast->get_operand_expr())
			type = getIRTypeFromSgType(expr->get_type());
		else
			throw CastorException("I don't know how to handle the contents of this sizeof.");

		return std::make_shared<IRSizeOf>(type);
	}
	else if (auto ast = isSgInitializedName(astNode)) // creating a variable
	{
		if (ast->isFrontendSpecific()) // if this is something ROSE inserted, let's ignore it
			return std::make_shared<IREmptyNode>();

		auto name = ast->get_qualified_name() + "_" + std::to_string(scope->get_unique_suffix(ast->get_name())); // get a unique name
		auto frontend_name = ast->get_name(); // grab the name to use in the frontend
		auto type = getIRTypeFromSgType(ast->get_type());

		if (auto vardecl = dynamic_cast<SgVariableDeclaration*>(ast->get_declaration());
				vardecl && vardecl->get_is_constexpr()) // if this was declared as constexpr
			type = std::make_shared<IRConstType>(type); // make it a const type

		auto var = std::make_shared<IRVariable>(type, name, frontend_name, true); // create the variable

		if (inheritedValue.status == VariableDeclaration) // if this is a variable declaration
		{
			SgType* type = ast->get_type(); // get its type

			bool is_static = SageInterface::isStatic(ast->get_declaration()); // check if it's a static variable

			scope->register_variable(frontend_name, var); // register it in the symbol table

			if (inheritedValue.class_var && !is_static) // if it's a class member variable
			{
				if (std::dynamic_pointer_cast<IRConstType>(getIRTypeFromSgType(type))) // first make sure it's not const
					throw UnsupportedFeatureException("Const-qualified class member variables");
				else
					scope->set_class_variable(frontend_name); // then mark it as a class variable
			}

			if (isAnyReferenceType(type)) // If it's a reference type, we need to insert an IRReference.
			{
				if (auto lv = std::dynamic_pointer_cast<IRLValue>(list[0])) // If we have an lvalue initializer, directly bind to that
				{
					auto type = lv->get_type();

					return std::make_shared<IRVariableDeclarationStmt>(var,
							std::make_shared<IRReference>(type, lv), is_static);
				}
				else // otherwise, temporarily materialize an object, then directly bind
				{
					auto rv = safety_cast<IRRValue>(list[0]);
					auto type = rv->get_type();

					return std::make_shared<IRVariableDeclarationStmt>(var,
							std::make_shared<IRReference>(type,
								std::make_shared<IRMaterialize>(rv)), is_static);
				}
			}
			else
			{
				return std::make_shared<IRVariableDeclarationStmt>(var,
						std::dynamic_pointer_cast<IRExpression>(list[0]), is_static);
			}
		}
		else if (inheritedValue.status == FunctionParameterList) // this is a function parameter
		{
			scope->register_variable(frontend_name, var); // just register the variable in the symbol table and pass it up

			return var;
		}
		else // not a variable declaration or function pointer, we can ignore this for now
		{
			return std::make_shared<IREmptyNode>();
		}
	}
	else if (auto ast = isSgFunctionParameterList(astNode)) // funcion parameter list
	{
		std::vector<std::shared_ptr<IRVariable>> varvec;
		for (auto l : list)
		{
			varvec.push_back(safety_cast<IRVariable>(l)); // pack up all the variables in an IRVariableList
		}
		return std::make_shared<IRVariableList>(varvec);
	}
	else if (auto ast = isSgVarRefExp(astNode)) // variable reference
	{
		auto var = R2WML_Scope::lookup(ast->get_symbol()); // look up the variable in the symbol table

		if (!var) // no variable exists, so we'll create something with an unknown type. perhaps the type filler can make sense of it
			  // if the type filler can't make sense of it, an exception will be thrown later on
			return std::make_shared<IRVariableReference>(std::make_shared<IRVariable>(std::make_shared<IRUnknownType>(), ast->get_symbol()->get_name(),
						ast->get_symbol()->get_name(), true));
		else // otherwise we're good to go
			return std::make_shared<IRVariableReference>(var);
	}
	else if (auto ast = isSgNonrealRefExp(astNode)) // this pops up in non-instantiated template functions
	{
		auto var = R2WML_Scope::lookup(ast->get_symbol()); // look up the variable in the symbol table

		if (!var) // no variable exists, so we'll create something with an unknown type. perhaps the type filler can make sense of it
			  // if the type filler can't make sense of it, an exception will be thrown later on
			return std::make_shared<IRVariableReference>(std::make_shared<IRVariable>(std::make_shared<IRUnknownType>(), ast->get_symbol()->get_name(),
						ast->get_symbol()->get_name(), true));
		else // otherwise we're good to go
			return std::make_shared<IRVariableReference>(var);
	}
	else if (auto ast = isSgThisExp(astNode)) // `this
	{
		auto var = scope->lookup("this"); // just look it up in the symbol table and return that
		if (!var) throw UnknownVariableException("this");
		return std::make_shared<IRVariableReference>(var);
	}
	else if (isSgBinaryOp(astNode)) // binary operations are offloaded to another function
	{
		return parseBinaryOp(astNode, list, scope);
	}
	else if (auto ast = isSgReturnStmt(astNode)) // return statements are easy
	{
		return std::make_shared<IRReturnStmt>(safety_cast<IRExpression>(list[0]));
	}
	else if (auto ast = isSgPointerDerefExp(astNode)) // pointer dereferencing is easy
	{
		return std::make_shared<IRPointerDereference>(
				getIRTypeFromSgType(ast->get_type()),
				safety_cast<IRExpression>(list[0]));
	}
	else if (auto ast = isSgBasicBlock(astNode)) // we have a basic block
	{
		std::vector<std::shared_ptr<IRStatement>> stmts;

		for (int i = 0; i < list.size(); i++)
		{
			auto l = list[i];

			if (std::dynamic_pointer_cast<IREmptyStatement>(l))
			{
				continue;
			}
			else if (auto label = std::dynamic_pointer_cast<__IRLabelWrapper>(l)) // labels have an associated statement, but we don't want that in the IR
			{
				stmts.push_back(safety_cast<IRStatement>(label->label));     // create separate statements for the label
				stmts.push_back(safety_cast<IRStatement>(label->statement)); // and its associated statement
			}
			else if (auto ghost = std::dynamic_pointer_cast<IRGhostStatement>(l))
			{
				if (i + 1 >= list.size())
					throw CastorException("Cannot have ghost marker at the end of a block!");

				ghost->attach_stmt(safety_cast<IRStatement>(list[++i]));
				stmts.push_back(ghost);
			}
			else
			{
				stmts.push_back(safety_cast<IRStatement>(l));
			}
		}

		attach_invariants(stmts); // attach invariants (e.g. `variant` and `invariant` for loops

		auto ret = std::make_shared<IRStatementBlock>(stmts, scope);

		return ret;
	}
	else if (isSgPragmaDeclaration(astNode) ||
		isSgSourceFile(astNode) ||
		isSgFileList(astNode) ||
		isSgFunctionDefinition(astNode) ||
		isSgAssignInitializer(astNode) ||
		isSgCtorInitializerList(astNode) ||
		isSgNamespaceDeclarationStatement(astNode)) // for all of these, we just pass up the synthesized value
	{
		return (list.size() && list[0]) ? list[0] : std::make_shared<IREmptyNode>();
	}
	else if (auto ast = isSgClassDeclaration(astNode)) // for classes, we just wanna make sure it's not a union
	{
		if (ast->get_class_type() == SgClassDeclaration::class_types::e_union)
			throw UnsupportedFeatureException("Unions");
		else
			return (list.size() && list[0]) ? list[0] : std::make_shared<IREmptyNode>();
	}
	else if (auto ast = isSgExprStatement(astNode)) // expression-statements
	{
		return std::make_shared<IRExpressionStmt>(safety_cast<IRExpression>(list[0]));
	}
	else if (auto ast = isSgNullExpression(astNode)) // null
	{
		return std::make_shared<IRNull>(); 
	}
	else if (isSgTypedefDeclaration(astNode) ||
		isSgEnumDeclaration(astNode) ||
		isSgTemplateInstantiationDirectiveStatement(astNode) ||
		isSgUsingDirectiveStatement(astNode)) // all of these are desugared elsewhere, so here we can ignore them
	{
		return std::make_shared<IREmptyNode>();
	}
	else if (isSgStaticAssertionDeclaration(astNode))
	{
		return std::make_shared<IREmptyStatement>();
	}
	else if (isSgGlobal(astNode) || isSgNamespaceDefinitionStatement(astNode)) // for global or namespace definitions
	{
		std::vector<std::shared_ptr<IRStatement>> globals;

		std::vector<std::shared_ptr<IRNode>> copied_list = list; // copy the synthesized attributes to a vector

		for (int i = 0; i < copied_list.size(); i++)
		{
			if (auto g = std::dynamic_pointer_cast<IRProgram>(copied_list[i])) // if we encounter an IRProgram, this is a case where
											   // we encountered a namespace. namespaces go away in the
											   // IR, so we just need to pull out the data and put it in
											   // copied_list
			{
				auto globals = g->get_globals();
				copied_list.erase(copied_list.begin() + i);
				copied_list.insert(copied_list.begin() + i, globals.begin(), globals.end());
				i += globals.size() - 1;
			}
		}

		for (int i = 0; i < copied_list.size(); i++) // iterate through our global items
		{
			if (auto vc = std::dynamic_pointer_cast<IRLoopInvariant>(copied_list[i])) // this is an `invariant` used on a function
			{
				auto require = std::make_shared<IRRequires>(vc->get_vc()); // create the requires and ensures clauses
				auto ensure = std::make_shared<IREnsures>(vc->get_vc());
				auto str = vc->get_str();
				auto require_str = "castor requires" + str.substr(str.find("invariant") + 9); // get the new raw strings
				auto ensure_str = "castor ensures" + str.substr(str.find("invariant") + 9);
				require->attach_str(require_str); // attach the strings
				ensure->attach_str(ensure_str);

				attach_ver(require, i + 1, copied_list); // attach the VCs
				attach_ver(ensure, i + 1, copied_list);
			}
			else if (auto vc = std::dynamic_pointer_cast<IRVerificationCondition>(copied_list[i]);
					vc && !std::dynamic_pointer_cast<IRLemma>(copied_list[i])) // verification condition
			{
				attach_ver(vc, i + 1, copied_list); // attach them to the next function
			}
			else if (auto g = std::dynamic_pointer_cast<IRStatement>(copied_list[i])) // we can push this to our globals array
			{
				globals.push_back(g);
			}
		}

		attach_impl(globals); // attach function implementations to their prototypes

		return std::make_shared<IRProgram>(globals, scope);
	}
	else if (auto ast = isSgFunctionRefExp(astNode)) // function references
	{
		auto decl = ast->get_symbol()->get_declaration(); // get the definition
		auto name = safety_cast<IRFunction>(IRGenerator()(decl))->get_name(); // this is dirty... we re-parse the function so that we can obtain its name
		return std::make_shared<IRFunctionRefExpr>(name);
	}
	else if (auto ast = isSgExprListExp(astNode)) // expression list
	{
		if (list.size() && std::dynamic_pointer_cast<__IRExpressionListWrapper>(list[0])) // if we already have an expression list, just pass it up
			return list[0];

		auto ret = std::make_shared<__IRExpressionListWrapper>();
		for (auto l : list)
		{
			ret->params.push_back(safety_cast<IRExpression>(l)); // otherwise we create an expression list wrapper with all our synthesized values
		}
		return ret;
	}
	else if (auto ast = isSgFunctionCallExp(astNode)) // function call expression
	{
		auto exprlist = safety_cast<__IRExpressionListWrapper>(list[1])->params; // grab our parameters

		if (isAnyReferenceType(ast->get_type())) // if this is an lvalue function
		{
			if (auto ref = std::dynamic_pointer_cast<IRFunctionRefExpr>(list[0])) // call a function if it's just a function call
				return std::make_shared<IRFunctionCallExpr<IRLValue>>(
					getIRTypeFromSgType(ast->get_type()), ref,
					exprlist);
			else if (auto ref = std::dynamic_pointer_cast<IRFieldReference>(list[0])) // call a member function if it's a member function call
				return std::make_shared<IRMemberFunctionCallExpr<IRLValue>>(
					getIRTypeFromSgType(ast->get_type()),
					ref->get_base_object(),
					std::make_shared<IRFunctionRefExpr>(ref->get_field_name()),
					exprlist);
			else
				return std::make_shared<IRFunctionCallExpr<IRLValue>>( // I can't remember what triggers this... TODO fill this in
					getIRTypeFromSgType(ast->get_type()),
					std::make_shared<IRFunctionRefExpr>(safety_cast<IRVariableReference>(list[0])
						->get_name()),
					exprlist);
		}
		else // this is the same as earlier, just with rvalue functions
		{
			if (auto ref = std::dynamic_pointer_cast<IRFunctionRefExpr>(list[0]))
				return std::make_shared<IRFunctionCallExpr<IRRValue>>(
					getIRTypeFromSgType(ast->get_type()), ref,
					exprlist);
			else if (auto ref = std::dynamic_pointer_cast<IRFieldReference>(list[0]))
				return std::make_shared<IRMemberFunctionCallExpr<IRRValue>>(
					getIRTypeFromSgType(ast->get_type()),
					ref->get_base_object(),
					std::make_shared<IRFunctionRefExpr>(ref->get_field_name()),
					exprlist);
			else
				return std::make_shared<IRFunctionCallExpr<IRRValue>>(
					getIRTypeFromSgType(ast->get_type()),
					std::make_shared<IRFunctionRefExpr>(safety_cast<IRVariableReference>(list[0])
						->get_name()),
					exprlist);
		}
	}
	else if (auto ast = isSgMemberFunctionRefExp(astNode)) // member function reference
	{
		auto decl = ast->get_symbol()->get_declaration(); // get the declaration
		auto name = safety_cast<IRFunction>(IRGenerator()(decl))->get_name(); // same nasty thing as earlier, re-parse the function to get its name
		return std::make_shared<IRFunctionRefExpr>(name);
	}
	else if (auto ast = isSgCastExp(astNode)) // casts are easy
	{
		auto kind = ast->get_cast_type();
		auto type = ast->get_type();
		auto base_type = ast->get_operand_i()->get_type();

		try
		{
			if (are_same_type_without_const(getIRTypeFromSgType(type), getIRTypeFromSgType(base_type)))
				return safety_cast<IRExpression>(list[0]);
		}
		catch (std::exception &e)
		{
			// This can trigger sometimes when upcasting an object to a parent class type.
			// It seems like an SgFunctionType makes its way in somewhere. The rest of Castor
			// can handle this just fine, so it's unknown why this only appears here.
			log("Encountered an unknown type trying to see if a cast can be optimized away. Trying to continue...", LogType::WARN, 2);
		}

		if (kind == SgCastExp::cast_type_enum::e_C_style_cast || kind == SgCastExp::cast_type_enum::e_static_cast)
			if (isAnyReferenceType(type))
				return std::make_shared<IRCast<IRLValue>>(getIRTypeFromSgType(type), safety_cast<IRExpression>(list[0]));
			else
				return std::make_shared<IRCast<IRRValue>>(getIRTypeFromSgType(type), safety_cast<IRExpression>(list[0]));
		else if (kind == SgCastExp::cast_type_enum::e_const_cast)
			return safety_cast<IRExpression>(list[0]);
		else
			throw UnsupportedFeatureException("reinterpret_cast and dynamic_cast");
	}
	else if (auto ast = isSgAddressOfOp(astNode)) // address-of is easy
	{
		return std::make_shared<IRAddressOf>(getIRTypeFromSgType(ast->get_type()), safety_cast<IRLValue>(list[0]));
	}
	else if (auto ast = isSgVariableDeclaration(astNode)) // for variable declarations, pass up the synthesized value
	{
		//if (ast->get_is_constexpr()) // constexpr isn't supported
		//	throw UnsupportedFeatureException("Constexpr-qualification");

		return list[1];
	}
#ifdef SUPER_DANGEROUS_DYNAMIC_MEMORY_ALLOC
	else if (auto ast = isSgNewExp(astNode)) // new expressions are easy
	{
		return std::make_shared<IRNewExpr>(getIRTypeFromSgType(ast->get_type()), safety_cast<IRExpression>(list[1]));
	}
	else if (auto ast = isSgDeleteExp(astNode)) // delete is easy
	{
		return std::make_shared<IRDeleteExpr>(safety_cast<IRLValue>(list[0]));
	}
#else
	else if (isSgNewExp(astNode) || isSgDeleteExp(astNode))
	{
		throw UnsupportedFeatureException("Dynamic memory allocation");
	}
#endif
	else if (auto ast = isSgMinusOp(astNode)) // negation is easy
	{
		auto expr = safety_cast<IRExpression>(list[0]);

		return std::make_shared<IRNegationOp>(getIRTypeFromSgType(ast->get_type()), expr);
	}
	else if (auto ast = isSgBitComplementOp(astNode)) // bit complement is easy
	{
		auto expr = safety_cast<IRExpression>(list[0]);

		return std::make_shared<IRBitNegationOp>(getIRTypeFromSgType(ast->get_type()), expr);
	}
	else if (auto ast = isSgNotOp(astNode)) // boolean not is easy
	{
		auto expr = safety_cast<IRExpression>(list[0]);

		return std::make_shared<IRBoolNegationOp>(getIRTypeFromSgType(ast->get_type()), expr);
	}
	else if (auto ast = isSgPlusPlusOp(astNode)) // increment
	{
		auto expr = safety_cast<IRExpression>(list[0]);
		
		if (ast->get_mode() == SgUnaryOp::Sgop_mode::prefix)
		{
			if (std::dynamic_pointer_cast<IRBoolType>(expr->get_type())) // support pre-C++17 bool::operator++
			{
				return std::make_shared<IRAssignOp>(getIRTypeFromSgType(ast->get_type()),
						safety_cast<IRLValue>(expr),
						std::make_shared<IRBoolLiteral>(true));
			}
			else // the usual case
			{
				return std::make_shared<IRAdditionAssignOp>(getIRTypeFromSgType(ast->get_type()),
						safety_cast<IRLValue>(expr),
						std::make_shared<IRS32Literal>(1));
			}
		}
		else
		{
			return std::make_shared<IRIncrementOp>(getIRTypeFromSgType(ast->get_type()), safety_cast<IRLValue>(expr));
		}
	}
	else if (auto ast = isSgMinusMinusOp(astNode)) // decrement
	{
		auto expr = safety_cast<IRExpression>(list[0]);
		
		if (ast->get_mode() == SgUnaryOp::Sgop_mode::prefix)
		{
			return std::make_shared<IRSubtractionAssignOp>(getIRTypeFromSgType(ast->get_type()),
						safety_cast<IRLValue>(expr),
						std::make_shared<IRS32Literal>(1));
		}
		else
		{
			return std::make_shared<IRDecrementOp>(getIRTypeFromSgType(ast->get_type()), safety_cast<IRLValue>(expr));
		}
	}
	else if (auto ast = isSgLabelStatement(astNode)) // labels
	{
		auto label_ref = std::make_shared<IRLabelReference>(ast->get_label()); // create a label reference
		auto label = std::make_shared<IRLabel>(label_ref); // create the label
		auto statement = safety_cast<IRStatement>(list[0]); // grab its associated statement

		return std::make_shared<__IRLabelWrapper>(label, statement); // create a label wrapper, we'll decompose this later
	}
	else if (auto ast = isSgConditionalExp(astNode)) // ternary operations are easy
	{
		auto condition = safety_cast<IRExpression>(list[0]);
		auto then = safety_cast<IRExpression>(list[1]);
		auto els = safety_cast<IRExpression>(list[2]);

		if (ast->isLValue())
			return std::make_shared<IRTernary<IRLValue>>(getIRTypeFromSgType(ast->get_type()), condition, then, els);
		else
			return std::make_shared<IRTernary<IRRValue>>(getIRTypeFromSgType(ast->get_type()), condition, then, els);
	}
	/*else if (auto ast = isSgAggregateInitializer(astNode))
	{
		return list[0];
	}*/
	else if (isSgValueExp(astNode)) // we have a special handler for literals
	{
		return parseLiteral(astNode, list);
	}
	else if (auto ast = isSgCaseOptionStmt(astNode)) // case labels
	{
		if (auto body = std::dynamic_pointer_cast<IRStatement>(list[1]))
			return std::make_shared<IRCase>(safety_cast<IRExpression>(list[0]),
					std::vector<std::shared_ptr<IRStatement>>({ body }));
		else
			return std::make_shared<IRCase>(safety_cast<IRExpression>(list[0]),
					std::vector<std::shared_ptr<IRStatement>>());
	}
	else if (auto ast = isSgDefaultOptionStmt(astNode)) // default case label
	{
		if (auto body = std::dynamic_pointer_cast<IRStatement>(list[0]))
			return std::make_shared<IRDefault>(std::vector<std::shared_ptr<IRStatement>>({ body }));
		else
			return std::make_shared<IRDefault>(std::vector<std::shared_ptr<IRStatement>>());
	}
	else if (auto ast = isSgBreakStmt(astNode))
	{
		return std::make_shared<IRBreak>();
	}
	else if (auto ast = isSgContinueStmt(astNode))
	{
		throw UnsupportedFeatureException("Continue statements");
		//return std::make_shared<IRContinue>();
	}
	else if (auto ast = isSgSwitchStatement(astNode)) // switch statement
	{
		std::shared_ptr<IRCase> active_case;
		auto statements = safety_cast<IRStatementBlock>(list[1])->get_statements(); // get the statements in the block
		std::vector<std::shared_ptr<IRCase>> cases;

		for (auto s : statements) // iterate over the statements in the block
		{
			if (auto case_ = std::dynamic_pointer_cast<IRCase>(s)) // if it's a case, set that as our active case
			{
				active_case = case_;
				cases.push_back(case_);
			}
			else // otherwise we need to add this statement to the active case label
				active_case->add_statement(safety_cast<IRStatement>(s));
		}

		std::shared_ptr<IRExpression> cond;
		std::shared_ptr<IRVariableDeclarationStmt> decl;
		if (!(cond = std::dynamic_pointer_cast<IRExpression>(list[0]))) // sometimes the condition is a statement, we need an expression
		{
			if (auto exprstmt = std::dynamic_pointer_cast<IRExpressionStmt>(list[0])) // check if we have an expression statement
				cond = exprstmt->get_expr();
			else if (decl = std::dynamic_pointer_cast<IRVariableDeclarationStmt>(list[0])) // otherwise we should have a variable declaration statement
			{
				cond = std::make_shared<IRVariableReference>(decl->get_var()); // get the variable reference
			}
		}

		auto switchstatement = std::make_shared<IRSwitchStmt>(cond, cases, scope);

		if (!decl) // if we don't have a variable declaration
		{
			return switchstatement; // just return the switch statement
		}
		else // otherwise
		{
			std::vector<std::shared_ptr<IRStatement>> stmts({ decl, switchstatement });   // collect our statements
			return std::make_shared<IRStatementBlock>(stmts, scope); // and return a statement block
		}
	}
	else // anything else, throw an exception
	{
		throw UnknownSgNodeException(astNode->class_name());
	}
}

