#include "gfx.h"

uint8_t* data_buf;

void gfx_defaults(){
  gfx.modexor = 0;
  gfx.rotated = 1;
  gfx.scale = 1;
}
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

    auto bb = (y<32) ? gfx.fbuf_top : gfx.fbuf_bot;
    auto bbpx = (1<<(y%32));
    if(gfx.modexor)
      bb[i] ^= bbpx;
    else
      bb[i] |= bbpx;
  }
}
void gfx_drawVline(int x, int y, int h){  
if(h < 0) {h *= -1; y -= h-1;}
  if(x<0) return;
  if(x>127) return;
  int e = y+h > 63 ? 63 : y+h;
  for(int i=y; i<e; i++){
    if(i<0) continue;
    if(i>63) continue;

    auto bb = (i<32) ? gfx.fbuf_top : gfx.fbuf_bot;
    auto bbpx = (1<<(i%32));
    if(gfx.modexor)
      bb[x] ^= bbpx;
    else
      bb[x] |= bbpx;
  }
}
void gfx_drawLine(int x, int y, int x2, int y2){
  
    if(gfx.rotated){
        auto a = y;
        y = 64-x;
        x = a;
        a = y2;
        y2 = 64-x2;
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
  //gfx.modexor = 0;
  if(gfx.rotated){
    gfx_drawHline(y,63-x,h);//W
    gfx_drawHline(y,63-x-w+1,h);//E
    gfx_drawVline(y,63-x-w+2,w-2);//N
    gfx_drawVline(y+h-1,63-x-w+2,w-2);//S
  }
  else{
    gfx_drawHline(x,y,w);//N
    gfx_drawVline(x,y+1,h-1);//W
    gfx_drawHline(x+1,y+h-1,w-1);//S
    gfx_drawVline(x+w-1,y+1,h-2);//E
  }

  //gfx_drawLine(x,y,w,y);//N
  // gfx_drawLine(x,y+1,x,h-1);//W
  // gfx_drawLine(x+1,y+h-1,w-1,y+h-1);//S
  // gfx_drawLine(x+w-1,y+1,x+w-1,h-2);//E
  
}

void gfx_fillSection(int xoff, int yoff, int xlen, int ylen){
  if(gfx.rotated){
      int32_t a = yoff;
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

  if(yoff >= 32){
    yoff -= 32;
    uint32_t bb = ylen >= 32 ? 0xffffffff : ((1<<ylen)-1);
    for(uint32_t i=xoff; i<xoff+xlen; i++){
      if(i<0) continue;
      if(i>127) return;
      if(gfx.modexor)
        gfx.fbuf_bot[i] ^= (bb<<(yoff));
      else
        gfx.fbuf_bot[i] |= (bb<<(yoff));
    }
  }
  else{
    uint32_t yy = yoff+ylen;
    auto ylen2 = yy > 32 ? yy - 32 : 0;
    uint32_t bt = ylen >= 32 ? 0xffffffff : ((1<<ylen)-1);
    uint32_t bb = ylen2 >= 32 ? 0xffffffff : ((1<<ylen2)-1);

    for(uint32_t i=xoff; i<xoff+xlen; i++){
      if(i<0) continue;
      if(i>127) return;
      if(gfx.modexor){
        gfx.fbuf_top[i] ^= (bt<<yoff);
        gfx.fbuf_bot[i] ^= (bb);
      }
      else{
        gfx.fbuf_top[i] |= (bt<<yoff);
        gfx.fbuf_bot[i] |= (bb);
      }
    }
  }
  
}

void drawPixel(int x, int y, int tx, int ty){

  if(gfx.rotated){
    tx += gfx.scale;
    int a = tx;
    tx = ty;
    ty = -a;

    a = x;
    x = y;
    y = (64/gfx.scale)-a;
  }

  auto sxx = gfx.scale*x;
  auto syy = gfx.scale*y;
  if(syy+(gfx.scale-1) < 0) return;
  if(sxx+(gfx.scale-1) < 0) return;

  //repeat for scaled x columns 
  for(int sx=sxx; sx<gfx.scale+sxx; sx++){
    //create pixel scaled y tall
    if(tx+sx >= 128) return;
    if(tx+sx < 0) continue;
    for(int s=0; s<gfx.scale; s++){
      if(ty+syy < 0) continue;
      if(ty+syy >= 64) return;
      auto bb = (syy+s+ty<32) ? gfx.fbuf_top : gfx.fbuf_bot;
      auto bbpx = (1<<((syy+s+ty)%32));
      if(gfx.modexor)
        bb[sx+tx] ^= bbpx;
      else
        bb[sx+tx] |= bbpx;
    }
  }
}


void gfx_drawBitmap(int x, int y, int w, int h, int bpc, int blen, const uint8_t* buf){
  int nw = w < 0;
  int nh = h < 0;
  if(nw) w = -w;
  if(nh) h = -h;
  for(int xx=0; xx<w; xx++){
    int xxx = nw ? w-xx : xx;
    for(int yy=0; yy<h; yy++){
      int yyy = nh ? h-yy : yy;
      uint8_t bb = (xxx*bpc) + (yy>>3)%bpc;
      uint8_t ybyte = 0;
      ybyte |= (buf[bb%blen]);
      
      if(!nh && ybyte & (1<<(yy%8)) || nh && ybyte & (1<<(8-(yy%8))))
        drawPixel(xxx,yy,x,y);
    }
  }
}

#ifndef _SRC_MINIM_EMU
#include "gfx/fonttiny.cpp"
#endif

void gfx_drawChar(char ch, int x, int y){
  ch = (ch < ' ' || ch > 126) ? 0 : (ch-' '+1);
  gfx_drawBitmap(x,y,
    fonttiny_wide,
    fonttiny_tall,
    fonttiny_tall>>3,
    fonttiny_wide,
    &fonttiny_data[int(ch)*fonttiny_wide]
  );
}


void gfx_drawString(const char* str, int x, int y){
  int ii = 0;
	while(str[ii] != 0){
		gfx_drawChar(str[ii], x + ii*fonttiny_wide*gfx.scale, y);
    ii++;
	}
}