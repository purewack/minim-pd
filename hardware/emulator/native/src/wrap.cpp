#include "wrap.h"

Napi::String GFX::dummyWrap(const Napi::CallbackInfo& info){
  const char* str = gfx_dummy();
  return Napi::String::New(info.Env(), str);
}

Napi::Object GFX::Init(Napi::Env env, Napi::Object exports) 
{
  //export Napi function
  exports.Set("dummy", Napi::Function::New(env, GFX::dummyWrap));
  return exports;
}