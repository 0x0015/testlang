#include "parse.hpp"
#include "tokenize/basicTokenize.hpp"
#include "tokenize/mediumTokenize.hpp"
#include "parse/parseContext.hpp"

std::optional<ast::context> parser::parseAll(){
	for(const auto& file : files){
		auto basicToks = basicTokenizeFile(file);
		if(!basicToks)
			return std::nullopt;
		auto mediumToks = parseBrackets(*basicToks);
		if(!mediumToks)
			return std::nullopt;
		auto parsedContext = parseContext(*mediumToks);
		if(!parsedContext)
			return std::nullopt;
		return parsedContext;
		//for now just return the first successfully parsed context.
		//At some point we will need to figure out how to merge contexts or something
	}

	return std::nullopt;
}

