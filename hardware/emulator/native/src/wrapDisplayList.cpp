#include "wrapMINIM.h"
#include "api.h"

Napi::FunctionReference MINIM::DisplayList::constructor;

Napi::Object MINIM::DisplayList::Init(Napi::Env env, Napi::Object exports) 
{
  Napi::HandleScope scope(env);

  Napi::Function buf = DefineClass(env, "DisplayList", {
    InstanceMethod("asArray", &MINIM::DisplayList::asArray),
    InstanceMethod("clear", &MINIM::DisplayList::clear),
    InstanceMethod("add", &MINIM::DisplayList::add),
    InstanceMethod("post", &MINIM::DisplayList::post),
    InstanceMethod("getCommandAt", &MINIM::DisplayList::getCommandAt),
    InstanceMethod("modifyAt", &MINIM::DisplayList::modifyAt),
    InstanceMethod("link", &MINIM::DisplayList::link),
    InstanceMethod("unlink", &MINIM::DisplayList::unlink),
    InstanceMethod("showLinks", &MINIM::DisplayList::showLinks),
  });

  constructor = Napi::Persistent(buf);
  constructor.SuppressDestruct();

  exports.Set("DisplayList", buf);
  return exports;
}

MINIM::DisplayList::DisplayList(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MINIM::DisplayList>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  this->list = new API::DisplayList();
}

Napi::Value MINIM::DisplayList::clear(const Napi::CallbackInfo& info){
    this->list->clear();
    return info.Env().Undefined();
}

Napi::Value MINIM::DisplayList::asArray(const Napi::CallbackInfo& info){
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  uint8_t buf[CMD_BYTE_COUNT_MAX];
  auto count = this->list->copyBuffer(buf);
  auto commands = std::vector<uint8_t>();
  for(int i=0; i<count; i++)
    commands.push_back(buf[i]);
  return Napi::Buffer<uint8_t>::Copy(env,commands.data(),commands.size());
}
Napi::Value MINIM::DisplayList::getCommandAt(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if (info.Length() != 1){
        Napi::Error::New(info.Env(), "Expected at location")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    auto at = (info[0].As<Napi::Number>().Uint32Value());
    if(at > this->list->getCount()){
        Napi::Error::New(info.Env(), "Invalid at location")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    auto cmd = this->list->getCommandAt(at);
    return Napi::Number::New(info.Env(), cmd);
}

Napi::Value MINIM::DisplayList::add(const Napi::CallbackInfo& info){ 
    if (info.Length() != 1){
        Napi::Error::New(info.Env(), "Expected 1 byte to be passed to add to the list")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    this->list->add(uint8_t(info[0].As<Napi::Number>().Int32Value()));
    return info.Env().Undefined();
}

Napi::Value MINIM::DisplayList::post(const Napi::CallbackInfo& info){ 
    if (info.Length() != 1){
        Napi::Error::New(info.Env(), "Expected array of bytes to be passed to add to the list")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    this->list->clear();
    Napi::Array b = info[0].As<Napi::Array>();
    for(unsigned int i = 0; i<b.Length(); i++)
    {
      Napi::Value v = b[i];
      if (v.IsNumber())
      {
        int value = (int)v.As<Napi::Number>();
        this->list->add(value);
      }
    }
    return info.Env().Undefined();
}


Napi::Value MINIM::DisplayList::link(const Napi::CallbackInfo& info){ 
    if (info.Length() != 2){
        Napi::Error::New(info.Env(), "Expected 2 arguments, listAt->slotAt")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    auto listAt = (info[0].As<Napi::Number>().Uint32Value());
    auto slotAt = (info[1].As<Napi::Number>().Uint32Value());
    this->list->link(listAt,slotAt);
    return info.Env().Undefined();
}
Napi::Value MINIM::DisplayList::unlink(const Napi::CallbackInfo& info){ 
    if (info.Length() != 2){
        Napi::Error::New(info.Env(), "Expected 1 argument slotAt")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    auto slotAt = (info[0].As<Napi::Number>().Uint32Value());
    this->list->unlink(slotAt);
    return info.Env().Undefined();
}
Napi::Value MINIM::DisplayList::modifyAt(const Napi::CallbackInfo& info){ 
    if (info.Length() != 2){
        Napi::Error::New(info.Env(), "Expected at index and a byte to be passed to modify in the list")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
    auto at = (info[0].As<Napi::Number>().Uint32Value());
    auto value = (info[1].As<Napi::Number>().Uint32Value());
    this->list->modifyAt(at,value);
    return info.Env().Undefined();
}

Napi::Value MINIM::DisplayList::showLinks(const Napi::CallbackInfo& info){
    if(info.Length() == 2){
        auto at    = (info[0].As<Napi::Number>().Int32Value());
        auto count = (info[1].As<Napi::Number>().Int32Value());
        int16_t buf[CMD_LINK_COUNT_MAX];
        this->list->copyLinkBuffer(buf);
        auto values = std::vector<int16_t>();
        for(int i=at; i<(at+count); i++)
            values.push_back(buf[i]);
        return Napi::Buffer<int16_t>::Copy(info.Env(),values.data(),values.size());
    }
    return info.Env().Undefined();
}