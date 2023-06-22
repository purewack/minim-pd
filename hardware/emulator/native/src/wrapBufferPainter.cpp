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
  uint8_t buf[128*64];
  gfx->accessBuffer(buf);
  auto section = std::vector<uint8_t>();
  for(int yy=y; yy<h+y; yy++){
    for(int xx=x; xx<w+x; xx++){
      section.push_back(buf[xx + yy*128]);
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
  // auto xo = this->gfx->modexor;
  // std::string s = "console.log(\'clear xor:";
  //   s += xo ? "yes" : "no";
  //   s += "\')";
  //   info.Env().RunScript(s);
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
  if (info.Length() < 4) {
    Napi::Error::New(info.Env(), "Expected > 4 arguments")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }


  auto x   = info[0].As<Napi::Number>().Int32Value();
  auto y   = info[1].As<Napi::Number>().Int32Value();
  auto x2  = info[2].As<Napi::Number>().Int32Value();
  auto y2  = info[3].As<Napi::Number>().Int32Value();
  
  auto useXor = info.Length() >= 5 ? info[4].As<Napi::Boolean>().Value() : false;
  if(useXor) this->gfx->modexor = 1;
  
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
  auto fill = info.Length() >= 5 ? info[4].As<Napi::Boolean>().Value() : false;

  auto useXor = info.Length() >= 6 ? info[5].As<Napi::Boolean>().Value() : false;
  if(useXor) this->gfx->modexor = 1;
  
  if(fill)
    this->gfx->fillSection(x,y,w,h);
  else
    this->gfx->drawRectSize(x,y,w,h);

  // std::string s = "console.log(\'drawRect :";
  //   s += fill ? "fill" : "nofill";
  //   s += " - ";
  //   s += useXor ? "xor" : "noxor";
  //   s += "\')";
  //   info.Env().RunScript(s);
  return info.Env().Undefined();
}