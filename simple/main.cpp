#include <iostream>
#include "parse.hpp"

#include <lexy/action/parse_as_tree.hpp>
#include <lexy/action/parse.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

int main(){
	std::string input_str = "#ffaa";
	auto input = lexy::string_input(input_str);

	lexy::parse_tree_for<decltype(input)> tree;
	//auto result = lexy::parse_as_tree<grammer::production>(tree, input, lexy_ext::report_error);
	auto result = lexy::parse<grammer::color>(input, lexy_ext::report_error);

	lexy::visualize(stdout, tree, {lexy::visualize_fancy});

	if(result.has_value()){
		auto col = result.value();
		std::printf("#%02x%02x%02x\n", col.r, col.g, col.b);
	}

	return 0;
}
