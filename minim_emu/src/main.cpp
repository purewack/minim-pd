#include "minim.h"

int main(){
  //audio_init();

  initGFXSystem();
  processMIDISystem();
  endGFXSystem();

  //audio_end();
  return 0;
}