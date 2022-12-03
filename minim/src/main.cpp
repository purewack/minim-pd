#include "minim.h"
#include "gfx.h"
#include "cmd.h"
#include <string>
#include <chrono>
gfx_t gfx;

#define INT(X) (std::stoi(X))

RenderTexture ctx[6];
int SS = 2;

int initGFXSystem(){
	data_buf = (uint8_t*)malloc(sizeof(uint8_t)*512);
    InitWindow(80*SS*8,128*SS,"M I N I M - Emulator");
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();
	gfx_defaults();

	for(int i=0; i<6; i++){
		ctx[i] = LoadRenderTexture(128,64);
		BeginTextureMode(ctx[i]);
			ClearBackground(BLACK);
		EndTextureMode();
	}
	return 0;
}

int endGFXSystem(){
	for(int i=0; i<6; i++)
		UnloadRenderTexture(ctx[i]);
	
    CloseWindow();
	free(data_buf);
	return 0;
}

int context = 0;
void cmd_gfx_on_context(int ctx){
	context = ctx;
	if(ctx == 5) gfx.rotated = 0;
	else gfx.rotated = 1;
}

int draw_count = 0;
void cmd_gfx_on_draw(){
	BeginTextureMode(ctx[context]);
		ClearBackground(BLACK);
		for(int x=0; x<128; x++){
			for(int y=0; y<32; y++){
				if(gfx.fbuf_top[x] & (1<<y))
					DrawPixel(x,y,WHITE);
				if(gfx.fbuf_bot[x] & (1<<y))
					DrawPixel(x,y+32,WHITE);
			}
		}
	EndTextureMode();
}

int initMidiSystem(){
	midiin = new RtMidiIn();
#ifdef WIN32
	unsigned int nPorts = midiin->getPortCount();
	if ( nPorts == 0 ) {
		std::cout << "No ports available!\n";
	}
	else{
		std::string portName;
  		for ( unsigned int i=0; i<nPorts; i++ ) {
			try {
				portName = midiin->getPortName(i);
				std::cout << "  Input Port #" << i+1 << ": " << portName << '\n';
			}
			catch ( RtMidiError &error ) {
				error.printMessage();
			}
		}
		midiin->openPort( 0 );
	}
#else
	midiin->openVirtualPort("MINIM Emu Input");
#endif
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

void processMidi(){
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
	initMidiSystem();
	initGFXSystem();
	while(!WindowShouldClose()){
		processMidi();
		BeginDrawing();
			ClearBackground(BLACK);
			for(int i=0; i<5; i++){
				DrawTexturePro(ctx[i].texture,
							(Rectangle){0,0,128,-64},
							(Rectangle){i*70*SS,0,128*SS,64*SS},
							(Vector2){0,128}, 90.0f, WHITE);
				DrawRectangleLines(i*70*SS,0,64*SS,128*SS,YELLOW);
			}
			
			DrawTexturePro(ctx[5].texture,
						(Rectangle){0,0,128,-64},
						(Rectangle){5*75*SS,0,128*SS*2,64*SS*2},
						(Vector2){0,0}, 0.0f, WHITE);
			DrawRectangleLines(5*75*SS,0,128*SS*2,64*SS*2,YELLOW);
		EndDrawing();
	}
	endMidiSystem();
	endGFXSystem();
#endif
  	return 0;
}