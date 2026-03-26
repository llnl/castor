// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "order_functions.hxx"
#include "helper.cxx"
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>
#include <vector>
#include <algorithm>

///
/// @brief Traverses over the tree, then aggregates the set into a graph
/// @param program The WhyProgram to investigate
/// @return The total graph representing the call graph
///
TotalGraph GenerateFunctionCallGraph::operator()(std::shared_ptr<WhyProgram> program)
{
	auto funcs = program->get_funcs(); // grab the program functions

	CallGraph call_graph;
	std::map<WhyName, CallGraph::vertex_descriptor> name_to_vertex;
	std::map<WhyName, std::set<WhyName>> function_calls;

	for (auto func : program->get_funcs()) // iterate over the program functions
	{
		auto name = func->get_name(); // grab their name
		auto calls = traverse(func, name); // generate the set of called functions

		function_calls[name] = calls; // add it to the map for later

		CallGraph::vertex_descriptor vertex = boost::add_vertex(call_graph); // create a vertex in the graph
		call_graph[vertex].name = name; // set its name to the current function name
		name_to_vertex[name] = vertex; // update the other map
	}

	for (auto call : function_calls) // iterate through all the functions and their function calls
	{
		auto key = call.first;  // the function name
		auto val = call.second; // the set of functions it calls

		for (auto v : val) // iterate over the set and add an edge to the graph
			boost::add_edge(name_to_vertex[key], name_to_vertex[v], call_graph);
	}

	return std::make_pair(call_graph, name_to_vertex);
}

///
/// @brief Downwards traversal (unused)
///
/// @param astNode The AST node
/// @param inheritedValue Passed down
/// @return = inheritedValue
///
WhyName GenerateFunctionCallGraph::evaluateInheritedAttribute(std::shared_ptr<WhyNode> astNode, WhyName inheritedValue)
{
	return inheritedValue;
}

///
/// @brief Checks if a function is called, and if so, add it to the set
///
/// @param astNode The AST node we're visiting
/// @param inheritedValue The current function name
/// @param list Function calls encountered so far
/// @return All of the functions encountered so far
///
std::set<WhyName> GenerateFunctionCallGraph::evaluateSynthesizedAttribute(std::shared_ptr<WhyNode> astNode, WhyName inheritedValue, SynthesizedAttributesList list)
{
	std::set<WhyName> all_calls;

	// merge all the incoming sets
	for (auto l : list)
		all_calls.merge(l);

	// if we see an lvalue function
	if (auto func = std::dynamic_pointer_cast<WhyFunctionCall<WhyLValue>>(astNode))
	{
		if (!func->get_in_vc()) // add it to the set if it's not in a VC
			all_calls.insert(func->get_name());

		return all_calls;
	}
	// if we see an rvalue function
	else if (auto func = std::dynamic_pointer_cast<WhyFunctionCall<WhyRValue>>(astNode))
	{
		if (!func->get_in_vc()) // add it to the set if it's not in a VC
			all_calls.insert(func->get_name());

		return all_calls;
	}
	// if we see a constructor
	else if (auto con = std::dynamic_pointer_cast<WhyConstructor>(astNode))
	{
		// add it to the set
		all_calls.insert(con->get_name());
		return all_calls;
	}
	else
	{
		return all_calls;
	}
}

///
/// @brief This generates the proper order to emit Why3 functions
///
/// We also annotate mutually recursive functions that should use "with" instead of "let rec".
///
/// This function operates by creating a condensation graph of the call graph in order to get a DAG.
/// Once we have a condensation graph, we perform a topological sort in order to determine
/// the order in which we emit functions into Why3. The condensation graph is important for handling
/// recursion and mutual recursion. If a single function equates to a strongly connected component,
/// it is not mutually recursive, and we can emit it alone. If multiple functions are in a strongly
/// connected component, they are mutually recursive, and we should mark them so. That way, the
/// pretty-printers can properly emit them into Why3 using "with".
///
/// @param graph The call graph generated from GenerateFunctionCallGraph
/// @param program The current WhyProgram to reorder
/// @return The new WhyProgram, reordered
///
std::shared_ptr<WhyNode> generate_correct_order(TotalGraph graph, std::shared_ptr<WhyProgram> program)
{
	auto g = std::get<0>(graph); // get the graph
	std::vector<int> component(boost::num_vertices(g)); // begin creating a list of components
	using IndexMap = boost::property_map<CallGraph, boost::vertex_index_t>::type;
	IndexMap indexMap = get(boost::vertex_index, g); // get the map of indices

	// create a property map for the strong_components function
	boost::iterator_property_map<std::vector<int>::iterator, IndexMap> componentMap(component.begin(), indexMap);

	int num = boost::strong_components(g, componentMap); // generate the strongly connected components

	// now we create the condensation graph
	using CondensationGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;
	CondensationGraph cg(num);

	// we need this to iterate over the edges
	using EdgeIterator = boost::graph_traits<CallGraph>::edge_iterator;
	EdgeIterator ei, ei_end;

	for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) // iterate over the edges in the graph
	{
		int u = source(*ei, g); // get its source
		int v = target(*ei, g); // and its target

		int comp_u = component[u]; // then get where the vertex is in the component graph
		int comp_v = component[v];

		if (comp_u != comp_v) // if two vertices are in different components
			boost::add_edge(comp_u, comp_v, cg); // we add the edge to the condensation graph
	}

	std::vector<int> topo_order;
	boost::topological_sort(cg, std::back_inserter(topo_order)); // now we perform the topological sort over the condensation graph

	// at this point we need to map back from vertex IDs to function names
	// this map maps component IDs to vectors of function names which lie in that component
	std::map<int, std::vector<WhyName>> component_to_names;
	for (auto vp = vertices(g); vp.first != vp.second; ++vp.first) // we iterate through the vertices of the graph
	{
		int comp_idx = component[*vp.first]; // get their location in the condensation graph
		component_to_names[comp_idx].push_back(g[*vp.first].name); // and push them to their corresponding vector
	}

	auto funcs = program->get_funcs(); // now we get the WhyProgram's functions
	std::map<WhyName, std::shared_ptr<WhyFunction>> function_map; // and a map from names to function pointers

	for (auto f : funcs)
		function_map[f->get_name()] = f; // fill out the function map, so that we can look up a function based on its name

	std::vector<std::shared_ptr<WhyFunction>> proper_order; // this will contain the proper order of the functions

	for (auto t : topo_order) // iterate through the topological order of the condensation graph
	{
		bool is_first = true; // for mutually recursive functions, the first declared function still uses "let"
				      // we need to tell the 2nd and beyond functions to use "with"
		for (auto name : component_to_names[t]) // iterate through the functions in a strongly connected component
		{
			auto func = function_map[name]; // get the actual function
			if (!is_first) func->set_mutual(); // potentially tell it to use "with"
			proper_order.push_back(func); // and add it to the proper order
			is_first = false;
		}
	}

	auto vars = program->get_vars(); // now we grab the program's global variables
	auto lemmas = program->get_lemmas(); // and the program lemmas

	return std::make_shared<WhyProgram>(vars, proper_order, lemmas); // and create a new WhyProgram with all the functions in the right place
}
