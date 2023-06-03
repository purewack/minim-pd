#include "wrap.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
   return GFX::BufferPainterWrap::Init(env, exports);
 }

Napi::FunctionReference GFX::BufferPainterWrap::constructor;

Napi::Object GFX::BufferPainterWrap::Init(Napi::Env env, Napi::Object exports) 
{
  Napi::HandleScope scope(env);

  Napi::Function bufpaint = DefineClass(env, "BufferPainterWrap", {
    InstanceMethod("getPixel", &BufferPainterWrap::getPixel),
    InstanceMethod("asArray", &BufferPainterWrap::asArray),
    InstanceMethod("_reset", &BufferPainterWrap::_reset),
    InstanceMethod("_clear", &BufferPainterWrap::_clear),
    InstanceMethod("_drawHline", &BufferPainterWrap::_drawHline),
    InstanceMethod("_drawVline", &BufferPainterWrap::_drawVline),
    InstanceMethod("_drawLine", &BufferPainterWrap::_drawLine),
    InstanceMethod("_drawRectSize", &BufferPainterWrap::_drawRectSize),
    InstanceMethod("_fillSection", &BufferPainterWrap::_fillSection),
  });

  constructor = Napi::Persistent(bufpaint);
  constructor.SuppressDestruct();

  exports.Set("BufferPainter", bufpaint);
  return exports;
}

GFX::BufferPainterWrap::BufferPainterWrap(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GFX::BufferPainterWrap>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  // int length = info.Length();
  // if (length != 1 || !info[0].IsNumber()) {
  //   Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
  // }

  Napi::Number value = info[0].As<Napi::Number>();
  this->gfx = new BufferPainter();
}

Napi::Value GFX::BufferPainterWrap::getPixel(const Napi::CallbackInfo& info){
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

Napi::Value GFX::BufferPainterWrap::asArray(const Napi::CallbackInfo& info){
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


Napi::Value GFX::BufferPainterWrap::_reset(const Napi::CallbackInfo& info){
  this->gfx->reset();
  return info.Env().Undefined();
}
Napi::Value GFX::BufferPainterWrap::_clear(const Napi::CallbackInfo& info){
  this->gfx->clear();
  return info.Env().Undefined();
}
Napi::Value GFX::BufferPainterWrap::_drawHline(const Napi::CallbackInfo& info){
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
Napi::Value GFX::BufferPainterWrap::_drawVline(const Napi::CallbackInfo& info){
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
Napi::Value GFX::BufferPainterWrap::_drawLine(const Napi::CallbackInfo& info){
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
Napi::Value GFX::BufferPainterWrap::_drawRectSize(const Napi::CallbackInfo& info){
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
Napi::Value GFX::BufferPainterWrap::_fillSection(const Napi::CallbackInfo& info){
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
