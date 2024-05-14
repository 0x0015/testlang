#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>
#include "../ast.hpp"
#include "id.hpp"
#include "literal.hpp"

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	struct type_t{
		static constexpr auto recur_type = dsl::recurse<type_t>;
		struct builtin_type_t{
			struct void_type{
				static constexpr auto rule = LEXY_KEYWORD("void", id);
				static constexpr auto value = lexy::constant(ast::type::builtin_type::void_type);
			};
			struct int_type{
				static constexpr auto rule = LEXY_KEYWORD("int", id);
				static constexpr auto value = lexy::constant(ast::type::builtin_type::int_type);
			};
			struct float_type{
				static constexpr auto rule = LEXY_KEYWORD("float", id);
				static constexpr auto value = lexy::constant(ast::type::builtin_type::float_type);
			};
			struct bool_type{
				static constexpr auto rule = LEXY_KEYWORD("bool", id);
				static constexpr auto value = lexy::constant(ast::type::builtin_type::bool_type);
			};
			static constexpr auto rule = dsl::p<void_type> | dsl::p<int_type> | dsl::p<float_type> | dsl::p<bool_type>;
			static constexpr auto value = lexy::forward<ast::type::builtin_type>;
		};
		static constexpr auto builtin_type = dsl::p<builtin_type_t>;
		//note: add some peeks to make sure that tuple and array will parse correctly
		struct tuple_type_t{
			//empty tuple is not a valid type (it is a void type)
			static constexpr auto rule = dsl::parenthesized(dsl::list(recur_type, dsl::sep(dsl::comma)));
			static constexpr auto value = lexy::as_list<ast::type::tuple_type>;
		};
		static constexpr auto tuple_type = dsl::p<tuple_type_t>;	
		struct array_type_t{
			struct array_type_impl_t : lexy::scan_production<int>{
				static constexpr auto isBuiltin = LEXY_LIT("void") | LEXY_LIT("int") | LEXY_LIT("float") | LEXY_LIT("bool");
				template<typename Context, typename Reader> static scan_result scan(lexy::rule_scanner<Context, Reader>& scanner){
					std::cout<<"in scan"<<std::endl;
					if(!scanner.branch(LEXY_LIT("int"))){
					    if(!scanner.branch(LEXY_LIT("(")))
					        return lexy::scan_failed;
					    int parens = 1;
					    std::string tupleToParse = "(";
					    while(parens > 0){
					        if(!scanner){
					            std::cout<<"Scanner never found end of parenthases"<<std::endl;
					            return lexy::scan_failed;
					        }
					        if(scanner.branch(LEXY_LIT("("))){
					            parens++;
					        }else if(scanner.branch(LEXY_LIT(")"))){
					            parens--;
					        }else if(scanner.branch(dsl::ascii::character)){
					            //none
					        }else{
					            std::cout<<"Scan failed for unknown reason"<<std::endl;
					            return lexy::scan_failed;
					        }
					    }
					}
					if(!scanner.branch(dsl::square_bracketed(dsl::digits<>)))
					    return lexy::scan_failed;
					do{} while(scanner.branch(dsl::square_bracketed(dsl::digits<>)));
					return 0;
				}
			};
			static constexpr auto rule = dsl::peek(dsl::p<array_type_impl_t>) >> builtin_type;
			static constexpr auto value = lexy::construct<ast::type::builtin_type>;
		};
		static constexpr auto array_type = dsl::p<array_type_t>;
		

		static constexpr auto rule = array_type | builtin_type | tuple_type;
		static constexpr auto value = lexy::construct<ast::type>;
	};
	constexpr auto type = dsl::p<type_t>;
}
}
