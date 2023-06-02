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
    InstanceMethod("hello", &BufferPainterWrap::hello)
  });

  constructor = Napi::Persistent(bufpaint);
  constructor.SuppressDestruct();

  exports.Set("BufferPainter", bufpaint);
  return exports;
}

Napi::Value GFX::BufferPainterWrap::hello(const Napi::CallbackInfo& info){
  return Napi::String::New(info.Env(),"Hello");
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
  if (!info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::Error::New(info.Env(), "Expected numbers as arguments (x,y)")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  auto x = info[0].As<Napi::Number>().Int32Value() % 128;
  auto y = info[1].As<Napi::Number>().Int32Value() % 64;
  if(x < 32)
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
