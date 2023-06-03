#include "wrap.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
   return MINIM::ControlSurface::Init(env, exports);
 }

Napi::FunctionReference MINIM::ControlSurface::constructor;

Napi::Object MINIM::ControlSurface::Init(Napi::Env env, Napi::Object exports) 
{
  Napi::HandleScope scope(env);

  Napi::Function cs = DefineClass(env, "ControlSurface", {
    InstanceMethod("getPixel", &ControlSurface::getPixel),
    InstanceMethod("asArray", &ControlSurface::asArray),
    InstanceMethod("_reset", &ControlSurface::_reset),
    InstanceMethod("_clear", &ControlSurface::_clear),
    InstanceMethod("_drawHline", &ControlSurface::_drawHline),
    InstanceMethod("_drawVline", &ControlSurface::_drawVline),
    InstanceMethod("_drawLine", &ControlSurface::_drawLine),
    InstanceMethod("_drawRectSize", &ControlSurface::_drawRectSize),
    InstanceMethod("_fillSection", &ControlSurface::_fillSection),
  });

  constructor = Napi::Persistent(cs);
  constructor.SuppressDestruct();

  exports.Set("ControlSurface", cs);
  return exports;
}

MINIM::ControlSurface::ControlSurface(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MINIM::ControlSurface>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  // int length = info.Length();
  // if (length != 1 || !info[0].IsNumber()) {
  //   Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  // }

  Napi::Number value = info[0].As<Napi::Number>();
  this->gfx = new BufferPainter();
}

Napi::Value MINIM::ControlSurface::getPixel(const Napi::CallbackInfo& info){
  if (info.Length() != 2) {
    Napi::Error::New(info.Env(), "Expected (x,y)")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  auto x = info[0].As<Napi::Number>().Int32Value() % 128;
  auto y = info[1].As<Napi::Number>().Int32Value() % 64;
  if(y < 32)
    return Napi::Boolean::New(info.Env(), this->gfx->fbuf_top[x] & (1<<y));
  else
    return Napi::Boolean::New(info.Env(), this->gfx->fbuf_bot[x] & (1<<(y-32)));
}

Napi::Value MINIM::ControlSurface::asArray(const Napi::CallbackInfo& info){
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  auto top = this->gfx->fbuf_top;
  auto bot = this->gfx->fbuf_bot;
  
  auto buf = Napi::ArrayBuffer::New(env,128*64);
  auto data = reinterpret_cast<int8_t*>(buf.Data());
  for(int i=0; i<2; i++){
    auto fbuf = i == 0 ? this->gfx->fbuf_top : this->gfx->fbuf_bot;
    for(int x=0; x<128; x++){
      for(int y=0; y<32; y++){
        data[x*32 + y*i] = fbuf[x] & (1<<y);
      }
    }
  }
  return buf;
}


void MINIM::ControlSurface::_reset(const Napi::CallbackInfo& info){
  this->gfx->reset();
}
void MINIM::ControlSurface::_clear(const Napi::CallbackInfo& info){
  this->gfx->clear();
}
Napi::Value MINIM::ControlSurface::_drawHline(const Napi::CallbackInfo& info){
  if (info.Length() != 3) {
    Napi::Error::New(info.Env(), "Expected 3 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  auto x = info[0].As<Napi::Number>().Int32Value();
  auto y = info[1].As<Napi::Number>().Int32Value();
  auto w = info[2].As<Napi::Number>().Int32Value();

  this->gfx->drawHline(x,y,w);
  return info.Env().Undefined();
}
Napi::Value MINIM::ControlSurface::_drawVline(const Napi::CallbackInfo& info){
  if (info.Length() != 3) {
    Napi::Error::New(info.Env(), "Expected 3 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  auto x = info[0].As<Napi::Number>().Int32Value();
  auto y = info[1].As<Napi::Number>().Int32Value();
  auto h = info[2].As<Napi::Number>().Int32Value();

  this->gfx->drawVline(x,y,h);
  return info.Env().Undefined();
}
Napi::Value MINIM::ControlSurface::_drawLine(const Napi::CallbackInfo& info){
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
Napi::Value MINIM::ControlSurface::_drawRectSize(const Napi::CallbackInfo& info){
  if (info.Length() != 4) {
    Napi::Error::New(info.Env(), "Expected 4 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  auto x   = info[0].As<Napi::Number>().Int32Value();
  auto y   = info[1].As<Napi::Number>().Int32Value();
  auto w   = info[2].As<Napi::Number>().Int32Value();
  auto h   = info[3].As<Napi::Number>().Int32Value();

  this->gfx->drawRectSize(x,y,w,h);
  return info.Env().Undefined();
}
Napi::Value MINIM::ControlSurface::_fillSection(const Napi::CallbackInfo& info){
  if (info.Length() != 4) {
    Napi::Error::New(info.Env(), "Expected 4 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  auto x   = info[0].As<Napi::Number>().Int32Value();
  auto y   = info[1].As<Napi::Number>().Int32Value();
  auto w   = info[2].As<Napi::Number>().Int32Value();
  auto h   = info[3].As<Napi::Number>().Int32Value();

  this->gfx->fillSection(x,y,w,h);
  return info.Env().Undefined();
}

/*
#include "wrap.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
   return MINIM::ControlSurface::Init(env, exports);
 }

Napi::FunctionReference MINIM::ControlSurface::constructor;

Napi::Object MINIM::ControlSurface::Init(Napi::Env env, Napi::Object exports) 
{
  Napi::HandleScope scope(env);

  Napi::Function bufpaint = DefineClass(env, "ControlSurface", {
    InstanceMethod("getPixel", &ControlSurface::getPixel),
    InstanceMethod("asArray", &ControlSurface::asArray),
    InstanceMethod("_reset", &ControlSurface::_reset),
    InstanceMethod("_clear", &ControlSurface::_clear),
    InstanceMethod("_drawHline", &ControlSurface::_drawHline),
    InstanceMethod("_drawVline", &ControlSurface::_drawVline),
    InstanceMethod("_drawLine", &ControlSurface::_drawLine),
    InstanceMethod("_drawRectSize", &ControlSurface::_drawRectSize),
    InstanceMethod("_fillSection", &ControlSurface::_fillSection),
  });

  constructor = Napi::Persistent(bufpaint);
  constructor.SuppressDestruct();

  exports.Set("BufferPainter", bufpaint);
  return exports;
}

MINIM::ControlSurface::ControlSurface(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MINIM::ControlSurface>(info)  {
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

Napi::Value MINIM::ControlSurface::getPixel(const Napi::CallbackInfo& info){
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

Napi::Value MINIM::ControlSurface::asArray(const Napi::CallbackInfo& info){
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


Napi::Value  MINIM::ControlSurface::_reset(const Napi::CallbackInfo& info){
  auto ctx = info[0].As<Napi::Number>().Int32Value() % 6;
  auto gfx = this->gfx[ctx];
  gfx->reset();
  return info.Env().Undefined();
}
Napi::Value  MINIM::ControlSurface::_clear(const Napi::CallbackInfo& info){
  auto ctx = info[0].As<Napi::Number>().Int32Value() % 6;
  auto gfx = this->gfx[ctx];
  gfx->clear();
  return info.Env().Undefined();
}
Napi::Value MINIM::ControlSurface::_drawHline(const Napi::CallbackInfo& info){
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
Napi::Value MINIM::ControlSurface::_drawVline(const Napi::CallbackInfo& info){
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
Napi::Value MINIM::ControlSurface::_drawLine(const Napi::CallbackInfo& info){
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
Napi::Value MINIM::ControlSurface::_drawRectSize(const Napi::CallbackInfo& info){
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
Napi::Value MINIM::ControlSurface::_fillSection(const Napi::CallbackInfo& info){
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