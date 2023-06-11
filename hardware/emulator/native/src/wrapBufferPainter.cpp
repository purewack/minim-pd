#include "wrapMINIM.h"

Napi::FunctionReference MINIM::BufferPainter::constructor;

Napi::Object MINIM::BufferPainter::Init(Napi::Env env, Napi::Object exports) 
{
  Napi::HandleScope scope(env);

  Napi::Function buf = DefineClass(env, "BufferPainter", {
    InstanceMethod("asArray", &MINIM::BufferPainter::asArray),
    InstanceMethod("getPixel", &MINIM::BufferPainter::getPixel),
    InstanceMethod("drawPixel", &MINIM::BufferPainter::drawPixel),
    InstanceMethod("drawLine", &MINIM::BufferPainter::drawLine),
    InstanceMethod("drawRect", &MINIM::BufferPainter::drawRect),
    InstanceMethod("clear", &MINIM::BufferPainter::clear),
  });

  constructor = Napi::Persistent(buf);
  constructor.SuppressDestruct();

  exports.Set("BufferPainter", buf);
  return exports;
}

MINIM::BufferPainter::BufferPainter(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MINIM::BufferPainter>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  this->gfx = new API::BufferPainter();
}

Napi::Value MINIM::BufferPainter::getPixel(const Napi::CallbackInfo& info){
  if (info.Length() != 2) {
    Napi::Error::New(info.Env(), "Expected (x,y)")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  auto x = info[0].As<Napi::Number>().Int32Value() % 128;
  auto y = info[1].As<Napi::Number>().Int32Value() % 64;
  
  return Napi::Number::New(info.Env(), this->gfx->getPixel(x,y));
}

std::vector<uint8_t> MINIM::BufferPainter::asArrayFromBufferPainter(API::BufferPainter* gfx, int x, int y,int w,int h){
  auto pixels = gfx->getBufferCopy();
  auto section = std::vector<uint8_t>();
  for(int yy=y; yy<h+y; yy++){
    for(int xx=x; xx<w+x; xx++){
      section.push_back(pixels[xx + yy*128]);
    }
  }
  return section;
}

Napi::Value MINIM::BufferPainter::asArray(const Napi::CallbackInfo& info){
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  
  int x = 0;
  int y = 0;
  int w = 128;
  int h = 64;

  if (info.Length() == 4) {
    x = info[0].As<Napi::Number>().Int32Value() % 128;
    y = info[1].As<Napi::Number>().Int32Value() % 64;
    w = info[2].As<Napi::Number>().Int32Value() % 128;
    h = info[3].As<Napi::Number>().Int32Value() % 64;
  }

  else if (info.Length() == 2) {
    w = info[0].As<Napi::Number>().Int32Value() % 128;
    h = info[1].As<Napi::Number>().Int32Value() % 64;
  }
  
  else if (info.Length() == 1) {
    w = h = (info[0].As<Napi::Number>().Int32Value() % 64);
  }

  else if (info.Length() != 0){
    Napi::Error::New(info.Env(), "Expected (x,y,w,h) or (w,h) or (dimension) or no params for 128*64 array")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  
  auto pixels = MINIM::BufferPainter::asArrayFromBufferPainter(this->gfx, x,y,w,h);

  return Napi::Buffer<uint8_t>::Copy(env,pixels.data(),pixels.size());
}


Napi::Value MINIM::BufferPainter::clear(const Napi::CallbackInfo& info){
  this->gfx->clear();
  return info.Env().Undefined();
}
Napi::Value MINIM::BufferPainter::drawPixel(const Napi::CallbackInfo& info){
  if (info.Length() != 2) {
    Napi::Error::New(info.Env(), "Expected 2 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }


  auto x   = info[0].As<Napi::Number>().Int32Value();
  auto y   = info[1].As<Napi::Number>().Int32Value();

  this->gfx->drawHline(x,y,1);  
  return info.Env().Undefined();
}
Napi::Value MINIM::BufferPainter::drawLine(const Napi::CallbackInfo& info){
  if (info.Length() != 4) {
    Napi::Error::New(info.Env(), "Expected 4 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }


  auto x   = info[0].As<Napi::Number>().Int32Value();
  auto y   = info[1].As<Napi::Number>().Int32Value();
  auto x2  = info[2].As<Napi::Number>().Int32Value();
  auto y2  = info[3].As<Napi::Number>().Int32Value();

  this->gfx->drawLine(x,y,x2,y2);  
  return info.Env().Undefined();
}
Napi::Value MINIM::BufferPainter::drawRect(const Napi::CallbackInfo& info){
  if (info.Length() < 4) {
    Napi::Error::New(info.Env(), "Expected 4 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  auto x    = info[0].As<Napi::Number>().Int32Value();
  auto y    = info[1].As<Napi::Number>().Int32Value();
  auto w    = info[2].As<Napi::Number>().Int32Value();
  auto h    = info[3].As<Napi::Number>().Int32Value();
  auto fill = info.Length() == 5 ? info[4].As<Napi::Boolean>().Value() : false;

  if(fill)
    this->gfx->fillSection(x,y,w,h);
  else
    this->gfx->drawRectSize(x,y,w,h);
  return info.Env().Undefined();
}

/*
#include "wrap.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
   return MINIM::BufferPainter::Init(env, exports);
 }

Napi::FunctionReference MINIM::BufferPainter::constructor;

Napi::Object MINIM::BufferPainter::Init(Napi::Env env, Napi::Object exports) 
{
  Napi::HandleScope scope(env);

  Napi::Function bufpaint = DefineClass(env, "BufferPainter", {
    InstanceMethod("getPixel", &BufferPainter::getPixel),
    InstanceMethod("asArray", &BufferPainter::asArray),
    InstanceMethod("_reset", &BufferPainter::_reset),
    InstanceMethod("_clear", &BufferPainter::_clear),
    InstanceMethod("_drawHline", &BufferPainter::_drawHline),
    InstanceMethod("_drawVline", &BufferPainter::_drawVline),
    InstanceMethod("_drawLine", &BufferPainter::_drawLine),
    InstanceMethod("_drawRectSize", &BufferPainter::_drawRectSize),
    InstanceMethod("_fillSection", &BufferPainter::_fillSection),
  });

  constructor = Napi::Persistent(bufpaint);
  constructor.SuppressDestruct();

  exports.Set("BufferPainter", bufpaint);
  return exports;
}

MINIM::BufferPainter::BufferPainter(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MINIM::BufferPainter>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  // int length = info.Length();
  // if (length != 1 || !info[0].IsNumber()) {
  //   Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  // }

  Napi::Number value = info[0].As<Napi::Number>();
  for(int i=0; i<6; i++)
    this->gfx[i] = new BufferPainter();
}

Napi::Value MINIM::BufferPainter::getPixel(const Napi::CallbackInfo& info){
  if (info.Length() != 3) {
    Napi::Error::New(info.Env(), "Expected (x,y)")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  auto ctx = info[0].As<Napi::Number>().Int32Value() % 6;
  auto x = info[1].As<Napi::Number>().Int32Value() % 128;
  auto y = info[2].As<Napi::Number>().Int32Value() % 64;
  if(y < 32)
    return Napi::Boolean::New(info.Env(), this->gfx[ctx]->fbuf_top[x] & (1<<y));
  else
    return Napi::Boolean::New(info.Env(), this->gfx[ctx]->fbuf_bot[x] & (1<<(y-32)));
}

Napi::Value MINIM::BufferPainter::asArray(const Napi::CallbackInfo& info){
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  if (info.Length() != 1) {
    Napi::Error::New(info.Env(), "Expected context number")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  auto ctx = info[0].As<Napi::Number>().Int32Value() % 6;
  auto gfx = this->gfx[ctx];

  auto top = gfx->fbuf_top;
  auto bot = gfx->fbuf_bot;
  
  auto buf = Napi::ArrayBuffer::New(env,128*64);
  auto data = reinterpret_cast<int8_t*>(buf.Data());
  for(int i=0; i<2; i++){
    auto fbuf = i == 0 ? gfx->fbuf_top : gfx->fbuf_bot;
    for(int x=0; x<128; x++){
      for(int y=0; y<32; y++){
        data[x*32 + y*i] = fbuf[x] & (1<<y);
      }
    }
  }
  return buf;
}


Napi::Value  MINIM::BufferPainter::_reset(const Napi::CallbackInfo& info){
  auto ctx = info[0].As<Napi::Number>().Int32Value() % 6;
  auto gfx = this->gfx[ctx];
  gfx->resetScaleRotate();
  return info.Env().Undefined();
}
Napi::Value  MINIM::BufferPainter::_clear(const Napi::CallbackInfo& info){
  auto ctx = info[0].As<Napi::Number>().Int32Value() % 6;
  auto gfx = this->gfx[ctx];
  gfx->clear();
  return info.Env().Undefined();
}
Napi::Value MINIM::BufferPainter::_drawHline(const Napi::CallbackInfo& info){
  if (info.Length() != 4) {
    Napi::Error::New(info.Env(), "Expected 4 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  auto ctx = info[0].As<Napi::Number>().Int32Value() % 6;
  auto gfx = this->gfx[ctx];

  auto x = info[1].As<Napi::Number>().Int32Value();
  auto y = info[2].As<Napi::Number>().Int32Value();
  auto w = info[3].As<Napi::Number>().Int32Value();

  gfx->drawHline(x,y,w);
  return info.Env().Undefined();
}
Napi::Value MINIM::BufferPainter::_drawVline(const Napi::CallbackInfo& info){
  if (info.Length() != 4) {
    Napi::Error::New(info.Env(), "Expected 4 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  auto ctx = info[0].As<Napi::Number>().Int32Value() % 6;
  auto gfx = this->gfx[ctx];
  auto x = info[1].As<Napi::Number>().Int32Value();
  auto y = info[2].As<Napi::Number>().Int32Value();
  auto h = info[3].As<Napi::Number>().Int32Value();

  gfx->drawVline(x,y,h);
  return info.Env().Undefined();
}
Napi::Value MINIM::BufferPainter::_drawLine(const Napi::CallbackInfo& info){
  if (info.Length() != 5) {
    Napi::Error::New(info.Env(), "Expected 5 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  auto ctx = info[0].As<Napi::Number>().Int32Value() % 6;
  auto gfx = this->gfx[ctx];
  auto x   = info[1].As<Napi::Number>().Int32Value();
  auto y   = info[2].As<Napi::Number>().Int32Value();
  auto x2  = info[3].As<Napi::Number>().Int32Value();
  auto y2  = info[4].As<Napi::Number>().Int32Value();

  gfx->drawLine(x,y,x2,y2);  
  return info.Env().Undefined();
}
Napi::Value MINIM::BufferPainter::_drawRectSize(const Napi::CallbackInfo& info){
  if (info.Length() != 5) {
    Napi::Error::New(info.Env(), "Expected 5 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  auto ctx = info[0].As<Napi::Number>().Int32Value() % 6;
  auto gfx = this->gfx[ctx];
  auto x   = info[1].As<Napi::Number>().Int32Value();
  auto y   = info[2].As<Napi::Number>().Int32Value();
  auto w   = info[3].As<Napi::Number>().Int32Value();
  auto h   = info[4].As<Napi::Number>().Int32Value();

  gfx->drawRectSize(x,y,w,h);
  return info.Env().Undefined();
}
Napi::Value MINIM::BufferPainter::_fillSection(const Napi::CallbackInfo& info){
  if (info.Length() != 5) {
    Napi::Error::New(info.Env(), "Expected 5 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  auto ctx = info[0].As<Napi::Number>().Int32Value() % 6;
  auto gfx = this->gfx[ctx];
  auto x   = info[1].As<Napi::Number>().Int32Value();
  auto y   = info[2].As<Napi::Number>().Int32Value();
  auto w   = info[3].As<Napi::Number>().Int32Value();
  auto h   = info[4].As<Napi::Number>().Int32Value();

  gfx->fillSection(x,y,w,h);
  return info.Env().Undefined();
}

*/