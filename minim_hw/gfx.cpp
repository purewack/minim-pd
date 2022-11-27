#include "gfx.h"
#include <Arduino.h>

void gfx_clear(){
  for(int i=0; i<128; i++){
    gfx.fbuf_top[i] = 0;
    gfx.fbuf_bot[i] = 0;
  }
}
void gfx_drawHline(int x, int y, int w){
if(w < 0) {w *= -1; x -= w;}
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
if(h < 0) {h *= -1; y -= h;}
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
    if(gfx.rotated){
        auto a = y;
        y = -x+63;
        x = a;
        a = y2;
        y2 = -x2+63;
        x2 = a;
    }

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
  gfx_drawLine(x,y,x+w,y);
  gfx_drawLine(x,y,x,y+h);
  gfx_drawLine(x,y+h-1,x+w,y+h);
  gfx_drawLine(x+w-1,y,x+w,y+h);
}

void gfx_fillSection(int yoff, int ylen, int xoff, int xlen, int fill){
    if(gfx.rotated){
        auto a = yoff;
        yoff = 64-xoff-xlen;
        xoff = a;
         a = ylen;
        ylen = xlen;
        xlen = a;
    }

  if(yoff < 0){
    ylen -= -yoff;
    yoff = 0;
  }

  if(yoff > 32){
    yoff -= 32;
    int bb = ((1<<ylen)-1);
    for(int i=xoff; i<xoff+xlen; i++){
      if(fill)
      gfx.fbuf_bot[i] |= (bb<<(yoff));
      else
      gfx.fbuf_bot[i] &= (~(bb<<(yoff)));
    }
  }
  else{
    int yy = yoff+ylen;
    int bt = ((1<<ylen)-1);
    int bb = yy > 32 ? ((1<<(yy-32))-1) : 0;
    for(int i=xoff; i<xoff+xlen; i++){
      if(fill){
        gfx.fbuf_top[i] |= (bt<<yoff);
        gfx.fbuf_bot[i] |= bb;
      }
      else{
        gfx.fbuf_top[i] &= (~(bt<<yoff));
        gfx.fbuf_bot[i] &= (~bb);
      }
    }
  }
  
}

void drawPixel(int x, int y, int scale, int tx, int ty){

  auto sxx = scale*x;
  auto syy = scale*y;
  if(syy+(scale-1) < 0) return;
  if(sxx+(scale-1) < 0) return;

  //repeat for scaled x columns 
  for(int sx=sxx; sx<scale+sxx; sx++){
    //create pixel scaled y tall
    if(tx+sx >= 128) return;
    if(tx+sx < 0) return;
    for(int s=0; s<scale; s++){
      if(syy+s+ty<32)
        gfx.fbuf_top[sx+tx] |= (1<<(syy+s+ty));
      else
        gfx.fbuf_bot[sx+tx] |= (1<<(syy+s+ty-32));
    }
  }
}

bool isPixel8(int x, int y, const uint8_t* buf, int len){
  return buf[x%len] & (1<<(y%8));
}

void gfx_drawBitmap8(int x, int y, int w, int h, int scale, int blen, const uint8_t* buf){

  // if(gfx.rotated){
  //   auto a = x;
  //   y = 64-x-w;
  //   x = y;
  //   a = w;
  //   w = h;
  //   h = a;
  // }

  // if(x < 0){
  //   w += x;
  //   x = 0;
  // }
  // if(y < 0){
  //   h += y;
  //   y = 0;
  // }
  
  // auto dd = (x+w)-128;
  // if(dd) w -= dd;

  // dd = (y+h)-64;
  // if(dd) h -= dd;
  auto ox = 0;//x*(scale-1);
  auto oy = 0;//y*(scale-1);
  for(int xx=0; xx<w; xx++){
    for(int yy=0; yy<h; yy++){
      if(isPixel8(xx,yy,buf,blen))
        drawPixel(xx,yy,scale,x,y);
    }
  }
}