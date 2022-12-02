#include "minim.h"
#include "gfx.h"
#include "cmd.h"
gfx_t gfx;

#define INT(X) (std::stoi(X))

RenderTexture ctx[5];
int SS = 2;

int initGFXSystem(){
	data_buf = (uint8_t*)malloc(sizeof(uint8_t)*512);
    InitWindow(64*SS*6,64*SS,"M I N I M - Emulator");
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();

	for(int i=0; i<5; i++){
		ctx[i] = LoadRenderTexture(128,128);

		BeginTextureMode(ctx[i]);
			ClearBackground(BLACK);
		EndTextureMode();
	}
	return 0;
}

int endGFXSystem(){
	for(int i=0; i<5; i++)
		UnloadRenderTexture(ctx[i]);
	
    CloseWindow();
	free(data_buf);
	return 0;
}

int context = 0;
void cmd_gfx_on_context(int ctx){
	context = ctx;
}
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
	initGFXSystem();
	initMidiSystem();
	while(!WindowShouldClose()){
		processMidi();
		BeginDrawing();
			ClearBackground(BLACK);
			for(int i=0; i<5; i++){
				DrawTexturePro(ctx[i].texture,
							(Rectangle){0,0,128,-64},
							(Rectangle){i*64*SS,0,128*SS,64*SS},
							(Vector2){0,0}, 0.0f, WHITE);
			}
			
		EndDrawing();
	}
	endMidiSystem();
	endGFXSystem();
#endif
  	return 0;
}