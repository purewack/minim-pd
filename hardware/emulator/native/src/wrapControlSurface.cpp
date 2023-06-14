#include "wrapMINIM.h"
#include "../../../api/api.h"

Napi::FunctionReference MINIM::ControlSurface::constructor;

Napi::Object MINIM::ControlSurface::Init(Napi::Env env, Napi::Object exports) 
{
  Napi::HandleScope scope(env);

  Napi::Function buf = DefineClass(env, "ControlSurface", {
    InstanceMethod("asArrayAtContext", &MINIM::ControlSurface::asArrayAtContext),
    InstanceMethod("getPixelAtContext", &MINIM::ControlSurface::getPixelAtContext),
    InstanceMethod("getCommandListAtContext", &MINIM::ControlSurface::getCommandListAtContext),
    InstanceMethod("parseMIDIStream", &MINIM::ControlSurface::parseMidiStream),
    InstanceMethod("parseMIDIStreamUpdate", &MINIM::ControlSurface::parseMidiStreamUpdate),
    InstanceMethod("parseCommandListAtContext", &MINIM::ControlSurface::parseCommandListAtContext),
    InstanceMethod("showLinksAtContext", &MINIM::ControlSurface::showLinksAtContext),
    InstanceMethod("showParseUpdates", &MINIM::ControlSurface::showParseUpdates),
    InstanceMethod("showParseErrors", &MINIM::ControlSurface::showParseErrors),
  });

    buf["SYSEX_ID"] = std::string(CMD_SYSEX_STRING);
    buf["SYSEX_START"] = CMD_SYSEX_START;
    buf["SYSEX_END"]   = CMD_SYSEX_END;
    buf["NOTE_ON"]  = NOTE_ON;
    buf["NOTE_OFF"] = NOTE_OFF;
    buf["CMD_ALTER"] = CMD_SYMBOL_F_ALTER;
    buf["CMD_LINK"] = CMD_SYMBOL_F_LINK;
    buf["CMD_LINE"] = CMD_SYMBOL_F_LINE;
    buf["CMD_RECT"] = CMD_SYMBOL_F_RECT;
    buf["CMD_STRING"] = CMD_SYMBOL_F_STRING;
    buf["CMD_BITMAP"] = CMD_SYMBOL_F_BITMAP;
    buf["CMD_UPLOAD"] = CMD_SYMBOL_F_UPLOAD;

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
    if( info.Length() < 1){
        Napi::Error::New(info.Env(), "Expected more than 0 arguments, context number")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    
    int context = info[0].As<Napi::Number>().Int32Value() % 6;

    int x = 0;
    int y = 0;
    int w = 128;
    int h = 64;

    if (info.Length() == 5) {
        x = info[1].As<Napi::Number>().Int32Value() % 128;
        y = info[2].As<Napi::Number>().Int32Value() % 64;
        w = info[3].As<Napi::Number>().Int32Value() % 128;
        h = info[4].As<Napi::Number>().Int32Value() % 64;
    }
    else if (info.Length() == 3) {
        w = info[1].As<Napi::Number>().Int32Value() % 128;
        h = info[2].As<Napi::Number>().Int32Value() % 64;
    }
    else if (info.Length() == 2) {
        w = h = (info[1].As<Napi::Number>().Int32Value() % 64);
    }

    // int cmds = this->cs->parseCommandList(context);
    // int realCmds = this->cs->cmdList[context].getCount();
    // info.Env().RunScript("console.log(\'cmds:" + std::to_string(cmds) + " real:" + std::to_string(realCmds) + "\')");
    auto pixels = MINIM::BufferPainter::asArrayFromBufferPainter(&this->cs->gfx, x,y,w,h);
    return Napi::Buffer<uint8_t>::Copy(info.Env(),pixels.data(),pixels.size());
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
  if(context < 0 || context > 5) {
      Napi::Error::New(info.Env(), "Invalid context number")
          .ThrowAsJavaScriptException();
      return info.Env().Undefined();
  } 
//   std::string s = "console.log(\'cmds ";
//     for(int i=0; i<16; i++){
//       s +=  std::to_string(this->cs->cmdList[context].getCommandAt(i)) + ", ";
//     }
//     s += " context: " + std::to_string(context); 
//     s += "\')";

//     info.Env().RunScript(s);
  auto list = this->cs->cmdList[context].getBufferCopy();
  return Napi::Buffer<uint8_t>::Copy(info.Env(),list.data(),list.size());
}

Napi::Value MINIM::ControlSurface::parseCommandListAtContext(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if( info.Length() != 1){
        Napi::Error::New(info.Env(), "Expected 1 argument, context number")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    auto context = (info[0].As<Napi::Number>().Uint32Value());
    if(context < 0 || context > 5) {
        Napi::Error::New(info.Env(), "Invalid context number")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    } 
    auto cmds = this->cs->parseCommandList(context);
    return Napi::Number::New(info.Env(), cmds);
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
    // std::string s = "console.log(\'data ";
    // for(int i=0; i<len; i++){
    //   s +=  std::to_string(data[i]) + ", ";
    // }
    // s += " len " + std::to_string(len);
    // s += "\')";
    // info.Env().RunScript(s);
    // info.Env().RunScript("console.log(\'" + std::string(this->cs->MidiStreamHasSysex(data,len) == 1 ? "Has Sysex" : "No Sysex flag in stream") + "\')");
    auto draws = this->cs->parseMidiStream(data,len);
    // auto updates = this->cs->updateContextsFlag;
    // info.Env().RunScript("console.log(\'" + std::to_string(updates) + "\')");
    
    return Napi::Number::New(info.Env(), draws);
}
Napi::Value MINIM::ControlSurface::parseMidiStreamUpdate(const Napi::CallbackInfo& info){
    auto draws = this->parseMidiStream(info);
    this->cs->updateRequiredContexts();
    return draws;
}

Napi::Value MINIM::ControlSurface::showLinksAtContext(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if( info.Length() != 2){
        Napi::Error::New(info.Env(), "Expected 3 arguments: context number, count")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    auto context = (info[0].As<Napi::Number>().Uint32Value());
    if(context < 0 || context > 5) {
        Napi::Error::New(info.Env(), "Invalid context number")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    } 

    auto count = (info[1].As<Napi::Number>().Uint32Value());
    auto values = this->cs->cmdList[context].getLinkBufferCopy();
    return Napi::Buffer<unsigned char>::Copy(info.Env(),values.data(),count);
}


Napi::Value MINIM::ControlSurface::showParseUpdates(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    // if( info.Length() != 1){
    //     Napi::Error::New(info.Env(), "Expected context number")
    //         .ThrowAsJavaScriptException();
    //     return info.Env().Undefined();
    // }

    // auto context = (info[0].As<Napi::Number>().Uint32Value());
    // if(context < 0 || context > 5) {
    //     Napi::Error::New(info.Env(), "Invalid context number")
    //         .ThrowAsJavaScriptException();
    //     return info.Env().Undefined();
    // } 

    auto updateBits = this->cs->updateContextsFlag;
    auto updateList = std::vector<uint8_t>();
    for(int i=0; i<6; i++)
        updateList.push_back(((1<<i) & updateBits) ? 1 : 0);
    
    return Napi::Buffer<uint8_t>::Copy(info.Env(),updateList.data(),updateList.size());
}

Napi::Value MINIM::ControlSurface::showParseErrors(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if( info.Length() == 1){
        auto context = (info[0].As<Napi::Number>().Uint32Value());
        if(context < 0 || context > 5) {
            Napi::Error::New(info.Env(), "Invalid context number")
                .ThrowAsJavaScriptException();
            return info.Env().Undefined();
        } 
        auto er = this->cs->errorLocation[context];
        if(er >= 0)
            return Napi::Number::New(info.Env(),er);
        else 
            return info.Env().Null();
    }

    auto errorsBits = this->cs->errorContextsFlag;
    auto errorsList = std::vector<uint8_t>();
    for(int i=0; i<6; i++)
        errorsList.push_back(((1<<i) & errorsBits) ? 1 : 0);
    
    return Napi::Buffer<uint8_t>::Copy(info.Env(),errorsList.data(),errorsList.size());
}
