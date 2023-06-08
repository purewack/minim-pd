#include "minim.h"
#include "gfx.h"
#include "cmd.h"
#include "api.h"
#include <string>
#include <chrono>
#include <unistd.h>

#define INT(X) (std::stoi(X))
gfx_t gfx;
RenderTexture ctx[6]; //emulator screen contexts
int _scale = 2; //emulator screen scale /px

bool sysex = false;
uint8_t sysex_buf[512];
int sysex_buf_len = 0;

int context = 0;
int draw_count = 0;

int initGFXSystem(){
	char str[64];
	snprintf(str,64,"M I N I M - Emulator API version[%d]",MINIM_API_VER);
	data_buf = (uint8_t*)malloc(sizeof(uint8_t)*512);
    InitWindow(80*_scale*8,128*_scale,str);
	SetTargetFPS(120);
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
void hookOnGfxContextChange(int ctx){
	context = ctx;
	if(ctx == 5) gfx.rotated = 0;
	else gfx.rotated = 1;
}

void hookOnGfxDraw(){
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

void hookOnSleep(int ms){}
void hookOnUploadBootBegin(){}
void hookOnUploadBootByte(unsigned char c){}
void hookOnUploadBootEnd(){}
void hookOnParse(int){}

int initMidiSystem(){
	midiin = new RtMidiIn();
//#ifdef WIN32
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
// #else
// 	midiin->openVirtualPort("MINIM Emu Input");
//#endif
	midiin->setBufferSize(8196,8);
  	midiin->ignoreTypes( false, false, false );
	return 0;
}
int endMidiSystem(){
	delete midiin;
	return 0;
}


void collectSysex(unsigned char b){
	if(b == 0xf7 && sysex){
		sysex = false;
		std::cout << "Finish SYSEX "<< std::endl;
		std::cout << "------Begin Parse"<< std::endl;
		parseCommand(sysex_buf,sysex_buf_len);
		std::cout << "------End Parse"<< std::endl;
	}
	else if(sysex){
		sysex_buf[sysex_buf_len] = b;
		sysex_buf_len++;
	}
}

int current_byte = 0;
void processMidi(){
	midiin->getMessage( &midiin_bytes);
	if(midiin_bytes.size() <= 0) return;

    std::cout << "New midi bytes\n{\t"<< std::endl;
    for(auto a : midiin_bytes)
      std::cout << "[" << std::to_string(a) << "]";

    std::cout << "\n}\nByte dump end"<< std::endl;
	// int nc = 0;
	// int bc[4];
	// bool ex = false;
    // for(auto bb : midiin_bytes){
    //   if(!sysex){
    //     if(bb == 0xf0 && !sysex){
    //       sysex = true;
	// 	  ex = true;
    //       sysex_buf_len = 0;
    //       std::cout << "Enter SYSEX"<< std::endl;
    //       collectSysex(bb);
    //     }
    //   }
    //   else if(sysex){
    //     collectSysex(bb);
    //   } 
    // }

	// if(!ex){
	// 	if(midiin_bytes[0] == 0xB1){
	// 		current_byte = midiin_bytes[1];
	// 	}
	// 	else if(midiin_bytes[0] == 0xB0){
	// 		auto a = setCByte(current_byte, midiin_bytes[1], midiin_bytes[2]);
	// 		printf("set CB: %d\n",a);
	// 	}
	// }

	// for(int f=0; f<6; f++){
	// 	if(frames[f].isFramed){
	// 		parseCommand(frames[f].cmd_bytes,frames[f].cmd_count);
	// 		frames[f].fps_counter++;
	// 		printf("frame[%d]@%d\n",f,frames[f].fps_counter);
	// 	}
	// }
}

int main(){
#ifdef _MINIM_TARGET_BUILD
	audio_init();
	std::string in;
	std::cin >> in;
	audio_end();
#else
	initMidiSystem();
	// initGFXSystem();
	// while(!WindowShouldClose()){
	while(1){
		processMidi();
		usleep(10000);
		// BeginDrawing();
		// 	ClearBackground(BLACK);
		// 	for(int i=0; i<5; i++){
		// 		DrawTexturePro(ctx[i].texture,
		// 					(Rectangle){0,0,128,-64},
		// 					(Rectangle){i*70*_scale,0,128*_scale,64*_scale},
		// 					(Vector2){0,128}, 90.0f, WHITE);
		// 		DrawRectangleLines(i*70*_scale,0,64*_scale,128*_scale,YELLOW);
		// 	}
			
		// 	DrawTexturePro(ctx[5].texture,
		// 				(Rectangle){0,0,128,-64},
		// 				(Rectangle){5*75*_scale,0,128*_scale*2,64*_scale*2},
		// 				(Vector2){0,0}, 0.0f, WHITE);
		// 	DrawRectangleLines(5*75*_scale,0,128*_scale*2,64*_scale*2,YELLOW);
		// EndDrawing();
	}
	endMidiSystem();
	// endGFXSystem();
#endif
  	return 0;
}