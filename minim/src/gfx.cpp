#include "minim.h"
#include <string>
#include <sstream>
#include <vector>

#define INT(X) (std::stoi(X))

std::vector<std::string> split(std::string s, char delim){
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

void show(std::vector<std::string> ss){
    std::cout << "{" << std::endl;
    for(int ii=0; ii<ss.size(); ii++)
        std::cout << "["<<ii<<"] = " << ss[ii] << "\n";
    std::cout << "}" << std::endl;
}

int initGFXSystem(){
    InitWindow(200*4,200,"M I N I M - Simulator");
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();
}

int endGFXSystem(){
    CloseWindow();
}

int parseGFXCommand(int screen, std::string cmds, int n){
    int xx = screen*200;
    if(cmds == "cls"){
        DrawRectangle(xx,0,200,200,BLACK);
        DrawRectangleLines(xx,0,200,200,BLUE);
        return 0;
    }
    else{
        auto cmd = split(cmds,':');
        show(cmd);
        if(cmd[0] == "recto"){
            if(cmd.size() != 5) return -1;
            else{
                DrawRectangleLines(xx + INT(cmd[1]),INT(cmd[2]),INT(cmd[3]),INT(cmd[4]),WHITE);
            }
        }
        if(cmd[0] == "rectf"){
            if(cmd.size() != 5) return -1;
            else{
                DrawRectangle(xx + INT(cmd[1]),INT(cmd[2]),INT(cmd[3]),INT(cmd[4]),WHITE);
            }
        }
        return 0;
    }
    return 1;
}

int postCommand(std::string s){
    auto elems = split(s,']');
    show(elems);

    BeginDrawing();
    for(int ii=0; ii<elems.size(); ii++){
        auto ss = elems[ii];

        auto gfx = split(ss,'[');
        show(gfx);

        auto context = split(gfx[0],'=');
        auto calls = split(gfx[1],';');
        show(calls);

        if(context[0] == "g"){
            int screen = std::stoi(context[1]);
            
                for(int ii=0; ii<calls.size(); ii++)
                    parseGFXCommand(screen,calls[ii],ii);
        
        }
    }    
    EndDrawing();
    
    return 0;
}