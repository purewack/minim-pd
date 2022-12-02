#include "minim.h"
#include "gfx.h"
#include "cmd.h"
gfx_t gfx;

#define INT(X) (std::stoi(X))

int initGFXSystem(){
	data_buf = (uint8_t*)malloc(sizeof(uint8_t)*512);
    InitWindow(200*4,200,"M I N I M - Emulator");
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();
	return 0;
}

int endGFXSystem(){
    CloseWindow();
	free(data_buf);
	return 0;
}

int context = 0;
void cmd_gfx_on_context(int ctx){
	context = ctx;
}
void cmd_gfx_on_draw(){
    std::cout << "cmd_gfx_on_draw"<< std::endl;
}

int initMidiSystem(){
	midiin = new RtMidiIn();
	midiin->openVirtualPort("MINIM Emu Input");
  	midiin->ignoreTypes( false, false, false );
	return 0;
}
int endMidiSystem(){
	delete midiin;
	return 0;
}

bool sysex = false;
uint8_t sysex_buf[512];
int sysex_buf_len = 0;

void collectSysex(unsigned char b){
	std::cout << "Collecting :" << std::to_string(b) << std::endl;
	if(b == 0xf7 && sysex){
		sysex = false;
		std::cout << "Finish SYSEX "<< std::endl;
		std::cout << "------Begin Parse"<< std::endl;
		parseCommand(sysex_buf,sysex_buf_len);
		std::cout << "------End Parse"<< std::endl;
	}
	else{
		sysex_buf[sysex_buf_len] = b;
		sysex_buf_len++;
	}
}

void processSystems(){
	midiin->getMessage( &midiin_bytes);
	if(midiin_bytes.size() <= 0) return;

    std::cout << "New midi bytes\n{"<< std::endl;
    for(auto a : midiin_bytes)
      std::cout << "\t[" << std::to_string(a) << "]" << std::endl;

    std::cout << "}\nByte dump end"<< std::endl;

    for(auto bb : midiin_bytes){
      if(!sysex){
        if(bb == 0xf0 && !sysex){
          sysex = true;
          sysex_buf_len = 0;
          std::cout << "Enter SYSEX"<< std::endl;
          collectSysex(bb);
        }
      }
      else{
        collectSysex(bb);
      } 
    }
}

int main(){
#ifndef _SRC_MINIM_EMU
	audio_init();
	std::string in;
	std::cin >> in;
	audio_end();
#else
	initGFXSystem();
	initMidiSystem();
	while(!WindowShouldClose()){
	processSystems();
	BeginDrawing();
	ClearBackground(BLACK);
	EndDrawing();
	}
	endMidiSystem();
	endGFXSystem();
#endif
  	return 0;
}