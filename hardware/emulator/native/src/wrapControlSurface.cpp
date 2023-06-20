#include "wrapMINIM.h"
#include "api.h"
#include "util.h"
#include <iostream>

Napi::FunctionReference MINIM::ControlSurface::constructor;


Napi::Object MINIM::ControlSurface::Init(Napi::Env env, Napi::Object exports) 
{
  Napi::HandleScope scope(env);

  Napi::Function buf = DefineClass(env, "ControlSurface", {
    InstanceMethod("asArrayAtContext", &MINIM::ControlSurface::asArrayAtContext),
    InstanceMethod("getPixelAtContext", &MINIM::ControlSurface::getPixelAtContext),
    InstanceMethod("getDisplayListAtContext", &MINIM::ControlSurface::getDisplayListAtContext),
    InstanceMethod("parseDisplayListAtContext", &MINIM::ControlSurface::parseDisplayListAtContext),
    InstanceMethod("showLinksAtContext", &MINIM::ControlSurface::showLinksAtContext),

    InstanceMethod("parseMIDIStream", &MINIM::ControlSurface::parseMidiStream),
    InstanceMethod("parseMIDIStreamUpdate", &MINIM::ControlSurface::parseMidiStreamUpdate),
    InstanceMethod("parseMIDICommands", &MINIM::ControlSurface::parseMidiCommands),
    InstanceMethod("showParseUpdates", &MINIM::ControlSurface::showParseUpdates),
    InstanceMethod("showParseErrors", &MINIM::ControlSurface::showParseErrors),
  });

  constructor = Napi::Persistent(buf);
  constructor.SuppressDestruct();

  auto cmdList = API::generateCommandDescriptors();
  auto listArray = Napi::Array::New(env);
  unsigned int j=0;

  auto contextList = API::generateContextDescriptors();
  for(auto i:contextList){
    auto cmd = Napi::Object::New(env);
    cmd["name"] = i["name"];
    cmd["symbol"] = i["symbol"];
    cmd["arguments"] = std::stoi(i["arguments"]);
    cmd["type"] = i["type"];
    listArray[j++] = cmd;
  }

  for(auto i:cmdList){
    auto cmd = Napi::Object::New(env);
    cmd["name"] = i["name"];
    cmd["symbol"] = i["symbol"];
    cmd["arguments"] = std::stoi(i["arguments"]);
    cmd["type"] = i["type"];
    listArray[j++] = cmd;
  }
  
  buf["commands"] = listArray;

  auto nameArray = Napi::Object::New(env);
  j=0;
  for(auto i:contextList){
    nameArray[i["name"]] = uint32_t(j++);
  }
  for(auto i:cmdList){
    nameArray[i["name"]] = uint32_t(j++);
  }
  buf["nameToIdx"] = nameArray;

  auto symbolArray = Napi::Object::New(env);
  j=0;
  for(auto i:contextList){
    symbolArray[i["symbol"]] = uint32_t(j++);
  }
  for(auto i:cmdList){
    symbolArray[i["symbol"]] = uint32_t(j++);
  }
  buf["symbolToIdx"] = symbolArray;

  auto nameToSymbolArray = Napi::Object::New(env);
  j=0;
  for(auto i:contextList){
    nameToSymbolArray[i["name"]] = i["symbol"];
  }
  for(auto i:cmdList){
    nameToSymbolArray[i["name"]] = i["symbol"];
  }
  buf["nameToSymbol"] = nameToSymbolArray;

  auto symbolToName = Napi::Object::New(env);
  j=0;
  for(auto i:contextList){
    symbolToName[i["symbol"]] = i["name"];
  }
  for(auto i:cmdList){
    symbolToName[i["symbol"]] = i["name"];
  }
  buf["symbolToName"] = symbolToName;

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
        Napi::Error::New(info.Env(), "Expected more than 0 arguments, context number, ?x,?y,?w,?h")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
     
    unsigned int context = info[0].As<Napi::Number>().Uint32Value() % 6;
    this->cs->parseDisplayList(context);
    
    unsigned int x = 0;
    unsigned int y = 0;
    unsigned int w = 128;
    unsigned int h = 64;

    if (info.Length() == 5) {
        x = info[1].As<Napi::Number>().Uint32Value() % 128;
        y = info[2].As<Napi::Number>().Uint32Value() % 64;
        w = info[3].As<Napi::Number>().Uint32Value() % 128;
        h = info[4].As<Napi::Number>().Uint32Value() % 64;
    }
    else if (info.Length() == 3) {
        w = info[1].As<Napi::Number>().Uint32Value() % 128;
        h = info[2].As<Napi::Number>().Uint32Value() % 64;
    }
    else if (info.Length() == 2) {
        w = h = (info[1].As<Napi::Number>().Uint32Value() % 64);
    }
    

    // for(unsigned int y=0; y<8; y++){
    //     for(unsigned int x=0; x<8; x++)
    //         std::cout << (this->cs->gfx.getPixel(x,y) ? "1" : "0");
    //     std::cout << std::endl;
    // }
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

    this->cs->parseDisplayList(context);
    return Napi::Number::New(info.Env(), this->cs->gfx.getPixel(x,y));
}
Napi::Value MINIM::ControlSurface::getDisplayListAtContext(const Napi::CallbackInfo& info){
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  if( info.Length() != 1){
      Napi::Error::New(info.Env(), "Expected 1 argument, context number")
          .ThrowAsJavaScriptException();
      return info.Env().Undefined();
  }
  
  auto context = (info[0].As<Napi::Number>().Uint32Value());
  if(context > 5) {
      Napi::Error::New(info.Env(), "Invalid context number")
          .ThrowAsJavaScriptException();
      return info.Env().Undefined();
  } 

    unsigned char buf[CMD_BYTE_COUNT_MAX];
    auto cc =  this->cs->cmdList[context].accessBuffer(buf);
  auto list = std::vector<uint8_t>();
  for(int i=0; i<cc; i++)
    list.push_back(buf[i]);
  return Napi::Buffer<uint8_t>::Copy(info.Env(),list.data(),list.size());
}

Napi::Value MINIM::ControlSurface::parseDisplayListAtContext(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if( info.Length() != 1){
        Napi::Error::New(info.Env(), "Expected 1 argument, context number")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    auto context = (info[0].As<Napi::Number>().Uint32Value());
    if(context > 5) {
        Napi::Error::New(info.Env(), "Invalid context number")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    } 
    auto cmds = this->cs->parseDisplayList(context);
    return Napi::Number::New(info.Env(), cmds);
}


void onParseElement(void* data, const char* element, int where){
    auto parsedata = reinterpret_cast<MINIM::ParseData*>(data);
    auto args = std::vector<Napi::Value>();
    args.push_back(Napi::String::New(*(parsedata->env),element));
    args.push_back(Napi::Number::New(*(parsedata->env),where));
    parsedata->callback->Call(args);
}


Napi::Value MINIM::ControlSurface::parseMidiStream(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if( info.Length() < 1){
        Napi::Error::New(info.Env(), "Expected 1 argument: (Uint8Array midiStream), or (Uint8Array midiStream, (string onElement)=>{})")
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

    int draws = 0;
    if( info.Length() == 2 && !info[1].IsUndefined()){
        if( !info[1].IsFunction() ){
            Napi::Error::New(info.Env(), "Expected parseHook function")
                .ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        auto fn = info[1].As<Napi::Function>();
        ParseData pdata;
        pdata.env = &env;
        pdata.callback = &fn;
        API::ParseArgs args = {onParseElement,reinterpret_cast<void*>(&pdata)};
        draws = this->cs->parseMidiStream(data,len,args);
    }else 
        draws = this->cs->parseMidiStream(data,len);

    return Napi::Number::New(info.Env(), draws);
}
Napi::Value MINIM::ControlSurface::parseMidiStreamUpdate(const Napi::CallbackInfo& info){
    auto draws = this->parseMidiStream(info);
    this->cs->updateRequiredContexts();
    return draws;
}

Napi::Value MINIM::ControlSurface::parseMidiCommands(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if( info.Length() < 1){
        Napi::Error::New(info.Env(), "Expected 1 argument: (Uint8Array midiStream), or (Uint8Array midiStream, (string onElement)=>{})")
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

    int draws = 0;
    if( info.Length() == 3 && !info[1].IsUndefined()){
        if( !info[1].IsFunction() ){
            Napi::Error::New(info.Env(), "Expected parseHook function")
                .ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        auto fn = info[1].As<Napi::Function>();

        auto offset = !info[2].IsNumber() ? 0 : info[2].As<Napi::Number>().Uint32Value();
        
        ParseData pdata;
        pdata.env = &env;
        pdata.callback = &fn;
        API::ParseArgs args = {onParseElement,reinterpret_cast<void*>(&pdata)};
        draws = this->cs->parseMidiCommands(offset, data,len,args);
    }
    return Napi::Number::New(info.Env(), draws);
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
    if(context > 5) {
        Napi::Error::New(info.Env(), "Invalid context number")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    } 

    auto count = (info[1].As<Napi::Number>().Uint32Value());
    unsigned char buf[CMD_LINK_COUNT_MAX];
    auto cc = this->cs->cmdList[context].accessLinkBuffer(buf);
    auto values = std::vector<unsigned char>();
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
        if(context > 5) {
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
