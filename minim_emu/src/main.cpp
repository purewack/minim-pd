#include "minim.h"

int main(){
  audio_init();

  initGFXSystem();
  postCommand("g=1[cls;recto:0:0:20:20;rectf:20:20:30:30]g=2[cls;recto:20:20:100:20;rectf:110:110:50:50]");
  std::string in;
  std::cin >> in;
  endGFXSystem();

  audio_end();
  return 0;
}