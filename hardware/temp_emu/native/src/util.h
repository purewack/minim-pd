#pragma once
#include <vector>
#include <map>
#include <string>
namespace API
{
    std::vector<std::map<std::string, std::string>> generateCommandDescriptors();
    std::vector<std::map<std::string, std::string>> generateContextDescriptors();
} 