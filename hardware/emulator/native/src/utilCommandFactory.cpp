#include <vector>
#include <map>
#include <string>
#include <sstream>
#include "api.h"
#include "util.h"
#include "describe.h"

#define AUTO_BIND(SYM) {\
    auto cmd = std::map<std::string, std::string>();\
    cmd["symbol"] = std::to_string(SYM);\
    cmd["arguments"] = std::to_string(SYM##_ARG);\
    cmd["name"] = std::string(SYM##_ABR);\
    cmd["type"] = std::string(SYM##_TYPE);\
    cmd["tooltip"] = std::string(SYM##_DESC);\
    std::string args[ SYM##_ARG ] = SYM##_ARGV ;\
    for(int i=0; i< SYM##_ARG; i++){\
        cmd["arg_" + std::to_string(i)] = args[i];\
    }\
    cmdList.push_back(cmd);\
} 
#define AUTO_BIND_X(SYM,NAME,ARG,TYPE,TIP) {\
    auto cmd = std::map<std::string, std::string>();\
    cmd["symbol"] = SYM;\
    cmd["arguments"] = ARG;\
    cmd["name"] = NAME;\
    cmd["type"] = TYPE;\
    cmd["tooltip"] = TIP;\
    contextCmds.push_back(cmd);\
} 

std::vector<std::map<std::string, std::string>> API::generateCommandDescriptors(){
    auto cmdList = std::vector<std::map<std::string, std::string>>();
    
    // AUTO_BIND(CMD_SYMBOL_ALTER);
    AUTO_BIND(CMD_SYMBOL_LINK);
    AUTO_BIND(CMD_SYMBOL_SCALE);
    AUTO_BIND(CMD_SYMBOL_XOR);
    AUTO_BIND(CMD_SYMBOL_LINE);
    AUTO_BIND(CMD_SYMBOL_RECT);
    AUTO_BIND(CMD_SYMBOL_STRING);
    AUTO_BIND(CMD_SYMBOL_BITMAP);

    return cmdList;
}

std::vector<std::map<std::string, std::string>> API::generateContextDescriptors(){
    auto contextCmds = std::vector<std::map<std::string, std::string>>();

    std::string startStr = "";
    startStr += std::to_string(CMD_SYSEX_ID[0]) + " ";
    startStr += std::to_string(CMD_SYSEX_ID[1]) + " ";
    startStr += std::to_string(CMD_SYSEX_ID[2]) + " ";
    startStr += std::to_string(CMD_SYSEX_ID[3]);
    AUTO_BIND_X(startStr,"start",std::to_string(1),"status","Start token of sysex stream, number following the token is the context number");

    startStr = "";
    startStr += std::to_string(CMD_SYSEX_ID[0]) + " ";
    startStr += std::to_string(CMD_SYSEX_ID[1]) + " ";
    startStr += std::to_string(CMD_SYSEX_ID[2]) + " ";
    startStr += std::to_string(CMD_SYSEX_DEBUG);
    AUTO_BIND_X(startStr,"debug",std::to_string(1),"status", "See Start token");
    AUTO_BIND_X(std::to_string(CMD_SYSEX_END),"end",std::to_string(0),"status","End context transmission token");

    return contextCmds;
}