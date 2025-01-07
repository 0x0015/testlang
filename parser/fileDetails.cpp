#include "fileDetails.hpp"
#include <iostream>

void fileSourceInfo::print() const{
	std::cout<<filename<<" "<<fileLocation<<"-"<<fileLocation+fileSize;
}
