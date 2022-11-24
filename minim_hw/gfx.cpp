#include "gfx.h"

void gfx_clear(){
  for(int i=0; i<128; i++){
    gfx.fbuf_top[i] = 0;
    gfx.fbuf_bot[i] = 0;
  }
}
void gfx_drawHline(int x, int y, int w){
  if(y<0) return;
  if(y>63) return;
  for(int i=x; i<x+w; i++){
    if(i<0) continue;
    if(i>127) continue;

    if(y<32)
      gfx.fbuf_top[i] |= (1<<y);
    else
      gfx.fbuf_bot[i] |= (1<<(y-32));
  }
}
void gfx_drawVline(int x, int y, int h){  
  if(x<0) return;
  if(x>127) return;
  int e = y+h > 64 ? 64 : y+h;
  for(int i=y; i<e; i++){
    if(i<0) continue;
    if(i>63) continue;

    if(i<32)
      gfx.fbuf_top[x] |= (1<<(i));
    else
      gfx.fbuf_bot[x] |= (1<<(i-32));
  }
}
void gfx_drawLine(int x, int y, int x2, int y2){
  const int dy = (y2-y);
  const int dx = (x2-x);
  const int nx = dx < 0 ? dx*-1 : dx;
  const int ny = dy < 0 ? dy*-1 : dy;

  if(dx == 0)
    return gfx_drawVline(x,y,dy);

  if(dy == 0)
    return gfx_drawHline(x,y,dx);

  if(nx > ny){
    if(dx < 0){
      for(int i=0; i>dx; i--){
          int yy = ((i)*dy)/dx;
          gfx_drawHline((i+x),yy+y,1);
      }
    }
    else{
      for(int i=0; i<dx; i++){
          int yy = ((i)*dy)/dx;
          gfx_drawHline((i+x),yy+y,1);
      }
    }
    
  }
  else{
    if(dy < 0){
      for(int i=0; i>dy; i--){
          int xx = ((i)*dx)/dy;
          gfx_drawHline(xx+x,(i+y),1);
      }
    }
    else{
      for(int i=0; i<dy; i++){
          int xx = ((i)*dx)/dy;
          gfx_drawHline(xx+x,(i+y),1);
      }
    }
  }
}

void gfx_drawRectSize(int x, int y, int w, int h){
  //gfx_draw_rect_base(x,y,w,h,0);
  gfx_drawHline(x,y,w);
  gfx_drawVline(x,y,h);
  gfx_drawHline(x,y+h-1,w);
  gfx_drawVline(x+w-1,y,h);
}