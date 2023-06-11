#include "wrapMINIM.h"

Napi::FunctionReference MINIM::ControlSurface::constructor;

Napi::Object MINIM::ControlSurface::Init(Napi::Env env, Napi::Object exports) 
{
  Napi::HandleScope scope(env);

  Napi::Function buf = DefineClass(env, "ControlSurface", {
    InstanceMethod("asArrayAtContext", &MINIM::ControlSurface::asArrayAtContext),
    InstanceMethod("getPixelAtContext", &MINIM::ControlSurface::getPixelAtContext),
    InstanceMethod("getCommandListAtContext", &MINIM::ControlSurface::getCommandListAtContext),
    InstanceMethod("parseMIDIStream", &MINIM::ControlSurface::parseMidiStream),
  });

  constructor = Napi::Persistent(buf);
  constructor.SuppressDestruct();

  exports.Set("ControlSurface", buf);
  return exports;
}

MINIM::ControlSurface::ControlSurface(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MINIM::ControlSurface>(info)  {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    this->cs = new API::ControlSurfaceAPI5();
}

Napi::Value MINIM::ControlSurface::asArrayAtContext(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if( info.Length() != 1){
        Napi::Error::New(info.Env(), "Expected 1 argument, context number")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    auto b = MINIM::BufferPainter::asArrayFromBufferPainter(&this->cs->gfx);
    return Napi::Buffer<uint8_t>::Copy(info.Env(),b.data(),b.size());
}
Napi::Value MINIM::ControlSurface::getPixelAtContext(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if( info.Length() != 3){
        Napi::Error::New(info.Env(), "Expected 3 arguments, context number, (x,y)")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    
    auto context = (info[0].As<Napi::Number>().Uint32Value());
    auto x = (info[1].As<Napi::Number>().Uint32Value());
    auto y = (info[2].As<Napi::Number>().Uint32Value());

    this->cs->parseCommandList(context);
    return Napi::Number::New(info.Env(), this->cs->gfx.getPixel(x,y));
}
Napi::Value MINIM::ControlSurface::getCommandListAtContext(const Napi::CallbackInfo& info){
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  if( info.Length() != 1){
      Napi::Error::New(info.Env(), "Expected 1 argument, context number")
          .ThrowAsJavaScriptException();
      return info.Env().Undefined();
  }
  
  auto context = (info[0].As<Napi::Number>().Uint32Value());
  if(context < 0 || context > 6) {
      Napi::Error::New(info.Env(), "Invalid context number")
          .ThrowAsJavaScriptException();
      return info.Env().Undefined();
  } 
  auto list = this->cs->cmdList[context].getBufferCopy();
  return Napi::Buffer<uint8_t>::Copy(info.Env(),list.data(),list.size());
}
Napi::Value MINIM::ControlSurface::parseMidiStream(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if( info.Length() != 1){
        Napi::Error::New(info.Env(), "Expected 1 argument, midi stream Int8Array")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    if( !info[0].IsBuffer() ){
        Napi::Error::New(info.Env(), "Expected Uint8Array")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    
    Napi::Uint8Array buf = info[0].As<Napi::Uint8Array>();
    auto data = reinterpret_cast<uint8_t*>(buf.Data());
    auto len = buf.ByteLength() / sizeof(uint8_t);
    // std::string s = "console.log(\'data";
    // for(int i=0; i<len; i++){
    //   s +=  std::to_string(data[i]) + ", ";
    // }
    // s += "\')";
    // info.Env().RunScript(s);
    // info.Env().RunScript("console.log(\'" + std::string(this->cs->MidiStreamHasSysex(data,len) == 1 ? "Has Sysex" : "No Sysex flag in stream") + "\')");
    return Napi::Number::New(info.Env(), this->cs->parseMidiStream(data,len));
}
Napi::Value MINIM::ControlSurface::parseMidiStreamUpdate(const Napi::CallbackInfo& info){
    auto v = this->parseMidiStream(info);
    this->cs->updateRequiredContexts();
    return v;
}