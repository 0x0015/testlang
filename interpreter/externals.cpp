#include "interpreter.hpp"
#include <filesystem>
#include <dlfcn.h>
#include "ffi.h"
#include "../parse/fileIO.hpp"
#include "../parse/parseUtil.hpp"

namespace interpreter{

const char* addToLibToPathNeed(std::string_view path){
	if(path.empty())
		return "";
	if(std::filesystem::path(path).is_absolute()){
		return "";
	}else{
		if(path.size() > 3 && path.substr(0, 3) == "lib"){
			return "";
		}else{
			return "lib";
		}
	}
}
//add more to this if needed
#if defined(_WIN32) || defined(_WIN64)
constexpr auto soExtention = ".dll";
constexpr unsigned int soExtentionLen = 4;
#else
constexpr auto soExtention = ".so";
constexpr unsigned int soExtentionLen = 3;
#endif

const char* addExtensionToPathNeed(std::string_view path){
	if(path.size() <= soExtentionLen)
		return soExtention;
	if(path.find(soExtention) != std::string_view::npos){
		return "";
	}
	return soExtention;
}

ffi_type getFfiType(const ast::type& ty){
	if(std::holds_alternative<ast::type::builtin_type>(ty.ty)){
		switch(std::get<ast::type::builtin_type>(ty.ty)){
			case ast::type::none_type:
			case ast::type::void_type:
				return ffi_type_void;
			case ast::type::int_type:
				return ffi_type_sint;
			case ast::type::float_type:
				return ffi_type_float;
			case ast::type::bool_type:
				return ffi_type_uint8;
			default:
				return ffi_type_void;
		}
	}else{
		//this should handle arrays/tuples.  It will not for now.
		return ffi_type_void;
	}
}

//don't touch this unless it really has something goof'd
std::optional<std::string> parseGNUldScript(std::string_view filename){
	auto file = readFile(filename, true);
	if(!file){
		file = readFile("/usr/lib/" + std::string(filename), true);  //blind grasp for hope
		if(!file)
			return std::nullopt;
	}
	if(file->find("GROUP") == std::string::npos)
		return std::nullopt;
	//quick and dirty parsing
	auto groupPos = file->find("GROUP");
	auto bodySec = file->substr(groupPos);
	auto parenPos = bodySec.find("(");
	if(parenPos == std::string_view::npos)
		return std::nullopt;
	bodySec = bodySec.substr(parenPos);
	auto spaceSplit = strSplit(bodySec, " ");
	if(!spaceSplit.empty())
		return std::string(spaceSplit.front());

	return std::nullopt;
}

}

std::optional<interpreter::interpreter::externalFunctionHandler> interpreter::loadExternalFunctions(std::span<const ast::function> funcs, std::span<const std::string> linkLibs){
	interpreter::externalFunctionHandler handler;
	bool loadFailed = false;
	for(const auto& str : linkLibs){
		const auto& objPath = addToLibToPathNeed(str) + str + addExtensionToPathNeed(str);
		void* handle = dlopen(objPath.c_str(), RTLD_LAZY);
		if(handle == nullptr){
			//wait, maybe it's a gnu ld script
			auto realObjPath = parseGNUldScript(objPath);
			if(!realObjPath){
				loadFailed = true;
				std::cerr<<"Failed to load external lib \""<<str<<"\": "<<dlerror()<<std::endl;
			}else{
				handle = dlopen(realObjPath->c_str(), RTLD_LAZY);
			}
		}
		handler.dlHandles[objPath] = handle;
	}
	if(loadFailed)
		return std::nullopt;

	for(const auto& func : funcs){
		if(func.status == ast::function::positionStatus::external){
			void* handle = nullptr;
			int implsFound = 0;
			for(auto& [dlname, dlhandle] : handler.dlHandles){
				void* tmpHandle = dlsym(dlhandle, func.name.c_str());
				if(tmpHandle != nullptr){
					//symbol was found!
					handle = tmpHandle;
					implsFound++;
				}
			}
			if(implsFound == 0){
				loadFailed = true;
				std::cerr<<"Failed to find external function \""<<func.name<<"\" in loaded external libs"<<std::endl;
			}else{
				auto& funcDetail = handler.funcHandles[func.name];
				funcDetail.handle = handle;

				//now for ffi bs
				funcDetail.rtype = getFfiType(func.ty);
				funcDetail.atypes.resize(func.args.size());
				funcDetail.atypePtrs.resize(func.args.size());
				for(unsigned int i=0;i<func.args.size();i++){
					funcDetail.atypes[i] = getFfiType(func.args[i].ty);
					funcDetail.atypePtrs[i] = &funcDetail.atypes[i];
				}
				auto status = ffi_prep_cif(&funcDetail.cif, FFI_DEFAULT_ABI, func.args.size(), &funcDetail.rtype, funcDetail.atypePtrs.data());
				if(status != FFI_OK){
					std::cerr<<"Failed to create ffi cif for function"<<std::endl;
					loadFailed = true;
				}
			}
		}
	}

	if(loadFailed)
		return std::nullopt;

	return handler;
}

void interpreter::interpreter::externalFunctionHandler::handleExternal(const ast::function& func, const ast::call& call, interpreter& M){
	std::cout<<"Handling external functin!"<<std::endl;
	unsigned int retSize = std::min<unsigned int>(4, func.ty.getSize());
	unsigned int argSize = sizeof(void*) * func.args.size();
	//for(const auto& arg : func.args){
	//	argSize += sizeof(void*);
	//}

	auto& funcDetail = M.externalHandler.funcHandles[func.name];
	std::vector<uint8_t> RAVec(retSize + argSize);//could be an alloca if I was really crazy.  I'm not
	void** argArr = (void**)((uint8_t*)RAVec.data() + retSize);
	for(unsigned int i=0;i<func.args.size();i++){
		const auto& carg = std::get<ast::varName>(call.args[i].value).name;
		argArr[i] = (void*) (M.stack.data() + (M.functionExecutions.back().variablePtrs[carg]));
	}
	ffi_call(&funcDetail.cif, (void(*)())funcDetail.handle, (void*)RAVec.data(), argArr);
}


