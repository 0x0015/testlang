#include <iostream>
#include "parse/parse.hpp"

#include <lexy/action/parse_as_tree.hpp>
#include <lexy/action/parse.hpp>
#include <lexy/action/trace.hpp>
#include <lexy/input/file.hpp>
#include <lexy_ext/report_error.hpp>

#include "builtins/builtins.hpp"
#include "functionChecker.hpp"
#include "typeChecker.hpp"

#include "interpreter/interpreter.hpp"

int main(){
	auto file = lexy::read_file<lexy::utf8_encoding>("simpleTest.txt");
	if(!file)
		return 1;
	auto input = file.buffer();

	//lexy::parse_tree_for<decltype(input)> tree;
	//{
	//	[[maybe_unused]] auto result = lexy::parse_as_tree<grammer::production>(tree, input, lexy_ext::report_error);
	//	lexy::visualize(stdout, tree, {lexy::visualize_fancy});
	//}
	lexy::trace<grammer::production>(stdout, input);
	auto result = lexy::parse<grammer::production>(input, lexy_ext::report_error);

	bool errored = false;
	errored = errored || result.is_error();

	if(!result.has_value()){
		return 1;//unrecoverable state (no parsed output to work off of)
	}
	

	auto val = result.value();
	addBuiltins(val);

	errored = errored || !checkFunctionsDefined(val);
	errored = errored || !checkConflictingFunctionDefinitions(val);
	errored = errored || !checkTypeUsesValid(val);

	if(!errored){
		val.dump();
		interpreter::interpret(val, "main");
	}
	

	return errored;
}
