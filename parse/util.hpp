#pragma once
#include <lexy/dsl.hpp>

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;
	namespace util{
		template<class R, class Sep> constexpr auto optList(R r, Sep s){
			return dsl::opt(dsl::list(r, s));
		}
	}
}
}
