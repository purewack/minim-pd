#include "minim.h"
#include <string>
#include <sstream>
#include <vector>

#define INT(X) (std::stoi(X))

int initGFXSystem(){
    InitWindow(640,480,"M I N I M - Simulator");
}

int endGFXSystem(){
    CloseWindow();
}

int parseGFXCommand(std::string c, int n){
    
    if(c == "cls"){
        ClearBackground(BLACK);
        return 0;
    }
    else{
        std::stringstream cc(c);
        std::string item;
        std::vector<std::string> cmd;
        while (std::getline(cc, item, ':')) {
            cmd.push_back(item);
        }
        std::cout << "{" << std::endl;
        for(int ii=0; ii<cmd.size(); ii++)
            std::cout << cmd[ii] << " , ";
        std::cout << "}" << std::endl;
        if(cmd[0] == "recto"){
            if(cmd.size() != 5) return -1;
            else{
                DrawRectangleLines(INT(cmd[1]),INT(cmd[2]),INT(cmd[3]),INT(cmd[4]),WHITE);
            }
        }
        if(cmd[0] == "rectf"){
            if(cmd.size() != 5) return -1;
            else{
                DrawRectangle(INT(cmd[1]),INT(cmd[2]),INT(cmd[3]),INT(cmd[4]),WHITE);
            }
        }
    }
}

int postGFXCommand(std::string s){
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, ';')) {
        elems.push_back(item);
    }
    BeginDrawing();
        for(int ii=0; ii<elems.size(); ii++)
            parseGFXCommand(elems[ii],ii);
    EndDrawing();
    return 0;
}