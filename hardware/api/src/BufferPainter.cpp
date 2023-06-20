#include "gfx.h"
#include "fonttiny.h"

int API::BufferPainter::accessBuffer(uint8_t* buf){
  int pixels = 0;
  for(int yy=0; yy<32; yy++){
    for(int xx=0; xx<128; xx++){
      buf[pixels++] = ((this->fbuf_top[xx] & (1<<yy)) ? 1 : 0);
    }
  }
  for(int yy=0; yy<32; yy++){
    for(int xx=0; xx<128; xx++){
      buf[pixels++] = ((this->fbuf_bot[xx] & (1<<yy)) ? 1 : 0);
    }
  }
  return pixels;
}
uint8_t API::BufferPainter::getPixel(unsigned char x, unsigned char y){
  x %= 128;
  y %= 64;
  if(y >= 32) return (this->fbuf_bot[x] & (1<<(y-32))) ? 1 : 0;
  return (this->fbuf_top[x] & (1<<(y))) ? 1 : 0;
}
void API::BufferPainter::resetScaleRotate(){
  this->modexor = 0;
  this->rotated = 0;
  this->scale = 1;
}
void API::BufferPainter::clear(){
  resetScaleRotate();
  for(int i=0; i<128; i++){
    this->fbuf_top[i] = 0;
    this->fbuf_bot[i] = 0;
  }
}
void API::BufferPainter::drawHline(int x, int y, int w){
if(w < 0) {w *= -1; x -= w;}
  if(y<0) return;
  if(y>63) return;
  for(int i=x; i<x+w; i++){
    if(i<0) continue;
    if(i>127) continue;

    auto bb = (y<32) ? this->fbuf_top : this->fbuf_bot;
    auto bbpx = (1<<(y%32));
    if(this->modexor)
      bb[i] ^= bbpx;
    else
      bb[i] |= bbpx;
  }
}
void API::BufferPainter::drawVline(int x, int y, int h){  
if(h < 0) {h *= -1; y -= h-1;}
  if(x<0) return;
  if(x>127) return;
  int e = y+h > 63 ? 63 : y+h;
  for(int i=y; i<e; i++){
    if(i<0) continue;
    if(i>63) continue;

    auto bb = (i<32) ? this->fbuf_top : this->fbuf_bot;
    auto bbpx = (1<<(i%32));
    if(this->modexor)
      bb[x] ^= bbpx;
    else
      bb[x] |= bbpx;
  }
}
void API::BufferPainter::drawLine(int x, int y, int x2, int y2){
  
    if(this->rotated){
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
    return this->drawVline(x,y,dy);

  if(dy == 0)
    return this->drawHline(x,y,dx);

  if(nx > ny){
    if(dx < 0){
      for(int i=0; i>dx; i--){
          int yy = ((i)*dy)/dx;
          this->drawHline((i+x),yy+y,1);
      }
    }
    else{
      for(int i=0; i<dx; i++){
          int yy = ((i)*dy)/dx;
          this->drawHline((i+x),yy+y,1);
      }
    }
    
  }
  else{
    if(dy < 0){
      for(int i=0; i>dy; i--){
          int xx = ((i)*dx)/dy;
          this->drawHline(xx+x,(i+y),1);
      }
    }
    else{
      for(int i=0; i<dy; i++){
          int xx = ((i)*dx)/dy;
          this->drawHline(xx+x,(i+y),1);
      }
    }
  }
}

void API::BufferPainter::drawRectSize(int x, int y, int w, int h){
  //this->modexor = 0;
  if(this->rotated){
    this->drawHline(y,63-x,h);//W
    this->drawHline(y,63-x-w+1,h);//E
    this->drawVline(y,63-x-w+2,w-2);//N
    this->drawVline(y+h-1,63-x-w+2,w-2);//S
  }
  else{
    this->drawHline(x,y,w);//N
    this->drawVline(x,y+1,h-1);//W
    this->drawHline(x+1,y+h-1,w-1);//S
    this->drawVline(x+w-1,y+1,h-2);//E
  }
}

void API::BufferPainter::fillSection(int xoff, int yoff, int xlen, int ylen){
  if(this->rotated){
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
    for(int i=xoff; i<xoff+xlen; i++){
      if(i<0) continue;
      if(i>127) return;
      if(this->modexor)
        this->fbuf_bot[i] ^= (bb<<(yoff));
      else
        this->fbuf_bot[i] |= (bb<<(yoff));
    }
  }
  else{
    uint32_t yy = yoff+ylen;
    auto ylen2 = yy > 32 ? yy - 32 : 0;
    uint32_t bt = ylen >= 32 ? 0xffffffff : ((1<<ylen)-1);
    uint32_t bb = ylen2 >= 32 ? 0xffffffff : ((1<<ylen2)-1);

    for(int i=xoff; i<xoff+xlen; i++){
      if(i<0) continue;
      if(i>127) return;
      if(this->modexor){
        this->fbuf_top[i] ^= (bt<<yoff);
        this->fbuf_bot[i] ^= (bb);
      }
      else{
        this->fbuf_top[i] |= (bt<<yoff);
        this->fbuf_bot[i] |= (bb);
      }
    }
  }
  
}

void API::BufferPainter::drawPixel(int x, int y, int tx, int ty){

  if(this->rotated){
    tx += this->scale;
    int a = tx;
    tx = ty;
    ty = -a;

    a = x;
    x = y;
    y = (64/this->scale)-a;
  }

  auto sxx = this->scale*x;
  auto syy = this->scale*y;
  if(syy+(this->scale-1) < 0) return;
  if(sxx+(this->scale-1) < 0) return;

  //repeat for scaled x columns 
  for(int sx=sxx; sx<this->scale+sxx; sx++){
    //create pixel scaled y tall
    if(tx+sx >= 128) return;
    if(tx+sx < 0) continue;
    for(int s=0; s<this->scale; s++){
      if(ty+syy < 0) continue;
      if(ty+syy >= 64) return;
      auto bb = (syy+s+ty<32) ? this->fbuf_top : this->fbuf_bot;
      auto bbpx = (1<<((syy+s+ty)%32));
      if(this->modexor)
        bb[sx+tx] ^= bbpx;
      else
        bb[sx+tx] |= bbpx;
    }
  }
}


// void API::BufferPainter::drawBitmap(int x, int y, int w, int h, int bpc, int blen, const uint8_t* buf){
//   int nw = w < 0;
//   int nh = h < 0;
//   if(nw) w = -w;
//   if(nh) h = -h;
//   for(int xx=0; xx<w; xx++){
//     int xxx = nw ? w-xx : xx;
//     for(int yy=0; yy<h; yy++){
//       int yyy = nh ? h-yy : yy;
//       uint8_t bb = (xxx*bpc) + (yy>>3)%bpc;
//       uint8_t ybyte = 0;
//       ybyte |= (buf[bb%blen]);
      
//       if((!nh && ybyte & (1<<(yy%8))) || (nh && ybyte & (1<<(8-(yy%8)))))
//         this->drawPixel(xxx,yy,x,y);
//     }
//   }
// }

// #ifdef _MINIM_TARGET_BUILD
// #include "gfx/fonttiny.cpp"
// #endif

// void BufferPainter::drawChar(char ch, int x, int y){
//   ch = (ch < ' ' || ch > 126) ? 0 : (ch-' '+1);
//   this->drawBitmap(x,y,
//     fonttiny_wide,
//     fonttiny_tall,
//     fonttiny_tall>>3,
//     fonttiny_wide,
//     &fonttiny_data[int(ch)*fonttiny_wide]
//   );
// }


// void BufferPainter::drawString(const char* str, int x, int y){
//   int ii = 0;
// 	while(str[ii] != 0){
// 		this->drawChar(str[ii], x + ii*fonttiny_wide*this->scale, y);
//     ii++;
// 	}
// }