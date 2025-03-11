#pragma once
#include "ast_type.hpp"
#include <iostream>
#include <vector>

namespace minLang{
namespace ast{
	struct literal{
		type ty;
		using builtin_literal = std::variant<int64_t, int32_t, uint64_t, uint32_t, double, float, bool>;
		struct array_literal{
			std::vector<literal> vals;
		};
		struct tuple_literal{
			std::vector<literal> vals;
		};
		std::variant<builtin_literal, array_literal, tuple_literal> value;
		literal() = default;
		literal(int64_t v) : value(builtin_literal{v}), ty(type::int64_type){}
		literal(uint64_t v) : value(builtin_literal{v}), ty(type::uint64_type){}
		literal(int32_t v) : value(builtin_literal{v}), ty(type::int32_type){}
		literal(uint32_t v) : value(builtin_literal{v}), ty(type::uint32_type){}
		literal(float v) : value(builtin_literal{v}), ty(type::float32_type){}
		literal(double v) : value(builtin_literal{v}), ty(type::float64_type){}
		literal(bool v) : value(builtin_literal{v}), ty(type::bool_type){}
		literal(const array_literal& v) : value(v){
			if(v.vals.size() == 0){
				ty = type::none_type;
				return;
			}
			ty = type::array_type{v.vals[0].ty, (unsigned int)v.vals.size()};
			for(unsigned int i=0;i<v.vals.size();i++){
				if(v.vals[i].ty != v.vals[0].ty){
					std::cerr<<"Error: all elements of array literal must be of same type (element "<<i<<" was of type: "<<v.vals[i].ty.toString()<<", expected: "<<v.vals[0].ty.toString()<<")"<<std::endl;
					ty = type::none_type;
				}
			}
		}	
		literal(const tuple_literal& v) : value(v){	
			if(v.vals.size() == 0){
				ty = type::none_type;
				return;
			}
			ty = type::tuple_type{};
			auto& tuple_ty = std::get<type::tuple_type>(ty.ty);
			tuple_ty.reserve(v.vals.size());
			for(const auto& o : v.vals)
				tuple_ty.push_back(o.ty);
		}
		literal(const literal& lit) : value(lit.value), ty(lit.ty){}
		std::string toString() const;
	};
}
}
