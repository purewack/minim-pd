#include "gfx.h"

void lcd_clear(){
  for(int i=0; i<128; i++){
    lcd.fbuf_top[i] = 0;
    lcd.fbuf_bot[i] = 0;
  }
}
void lcd_drawHline(int x, int y, int w){
  if(y<0) return;
  if(y>63) return;
  for(int i=x; i<x+w; i++){
    if(i<0) continue;
    if(i>127) continue;

    if(y<32)
      lcd.fbuf_top[i] |= (1<<y);
    else
      lcd.fbuf_bot[i] |= (1<<(y-32));
  }
}
void lcd_drawVline(int x, int y, int h){  
  if(x<0) return;
  if(x>127) return;
  int e = y+h > 64 ? 64 : y+h;
  for(int i=y; i<e; i++){
    if(i<0) continue;
    if(i>63) continue;

    if(i<32)
      lcd.fbuf_top[x] |= (1<<(i));
    else
      lcd.fbuf_bot[x] |= (1<<(i-32));
  }
}
void lcd_drawLine(int x, int y, int x2, int y2){
  const int dy = (y2-y);
  const int dx = (x2-x);
  const int nx = dx < 0 ? dx*-1 : dx;
  const int ny = dy < 0 ? dy*-1 : dy;

  if(dx == 0)
    return lcd_drawVline(x,y,dy);

  if(dy == 0)
    return lcd_drawHline(x,y,dx);

  if(nx > ny){
    if(dx < 0){
      for(int i=0; i>dx; i--){
          int yy = ((i)*dy)/dx;
          lcd_drawHline((i+x),yy+y,1);
      }
    }
    else{
      for(int i=0; i<dx; i++){
          int yy = ((i)*dy)/dx;
          lcd_drawHline((i+x),yy+y,1);
      }
    }
    
  }
  else{
    if(dy < 0){
      for(int i=0; i>dy; i--){
          int xx = ((i)*dx)/dy;
          lcd_drawHline(xx+x,(i+y),1);
      }
    }
    else{
      for(int i=0; i<dy; i++){
          int xx = ((i)*dx)/dy;
          lcd_drawHline(xx+x,(i+y),1);
      }
    }
  }
}

void lcd_drawRectSize(int x, int y, int w, int h){
  //lcd_draw_rect_base(x,y,w,h,0);
  lcd_drawHline(x,y,w);
  lcd_drawVline(x,y,h);
  lcd_drawHline(x,y+h-1,w);
  lcd_drawVline(x+w-1,y,h);
}
