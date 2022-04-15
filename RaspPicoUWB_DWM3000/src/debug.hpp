#pragma once

#include <iostream>
#define DUMP_VAR(x) { \
    std::cout "DUMP " << << __FILE__ << "::" << __LINE__  << "::" << #x << "=<" << x << ">" <<std::endl;\
}

#define ERROR_VAR(x) { \
    std::cout <<"ERROR " << __FILE__ << "::" << __LINE__  << "::" << #x << "=<" << x << ">" <<std::endl;\
}

