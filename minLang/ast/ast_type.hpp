#pragma once
#include <memory>
#include <variant>
#include <vector>

namespace minLang{
namespace ast{
	struct type{
		enum builtin_type{
			none_type,
			void_type,
			uint8_type,
			uint16_type,
			uint32_type,
			uint64_type,
			int8_type,
			int16_type,
			int32_type,
			int64_type,
			float32_type,
			float64_type,
			bool_type
		};
		struct array_type{
			std::shared_ptr<type> ty;
			unsigned int length;
			array_type() = default;
			array_type(const type& t_ty) : ty(std::make_shared<type>(t_ty)){}
			array_type(const type& t_ty, unsigned int len) : ty(std::make_shared<type>(t_ty)), length(len){}
		};
		using tuple_type = std::vector<type>;
		std::variant<builtin_type, array_type, tuple_type> ty;
		type() = default;
		type(builtin_type builtin_ty) : ty(builtin_ty){}
		type(const array_type& array_ty) : ty(array_ty){}
		type(const tuple_type& tuple_ty) : ty(tuple_ty){}
		constexpr bool operator==(const type& other) const{
			if(ty.index() != other.ty.index())
				return false;
			if(std::holds_alternative<builtin_type>(ty)){
				return std::get<builtin_type>(ty) == std::get<builtin_type>(other.ty);
			}
			if(std::holds_alternative<array_type>(ty)){
				const auto& a1 = std::get<array_type>(ty);
				const auto& a2 = std::get<array_type>(other.ty);
				return (a1.ty->operator==(*a2.ty));
			}
			if(std::holds_alternative<tuple_type>(ty)){
				const auto& t1 = std::get<tuple_type>(ty);
				const auto& t2 = std::get<tuple_type>(other.ty);
				if(t1.size() != t2.size()) return false;
				for(unsigned int i=0;i<t1.size();i++){
					if(! t1[i].operator==(t2[i])) return false;
				}
				return true;
			}
			return false;
		}
		unsigned int getSize() const;
		static type fromString(const std::string_view type);
		std::string toString() const;
		ast::type clone() const;
		std::size_t hash() const;
		static ast::type fullyDealias(const ast::type& ty);
	};
}
}
