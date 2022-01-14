#include <iostream>
#include <string>
#include "one.hpp"

void cpppost(){
    std::string st = "hello from cpp";
    post("%s",st.c_str());
}
