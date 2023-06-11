#include "wrapMINIM.h"

Napi::FunctionReference MINIM::DisplayList::constructor;

Napi::Object MINIM::DisplayList::Init(Napi::Env env, Napi::Object exports) 
{
  Napi::HandleScope scope(env);

  Napi::Function buf = DefineClass(env, "DisplayList", {
    InstanceMethod("asArray", &MINIM::DisplayList::asArray),
    InstanceMethod("clear", &MINIM::DisplayList::clear),
    InstanceMethod("add", &MINIM::DisplayList::add),
    InstanceMethod("post", &MINIM::DisplayList::post),
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
  auto commands = this->list->getBufferCopy();
  return Napi::Buffer<uint8_t>::Copy(env,commands.data(),commands.size());
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
    for(int i = 0; i<b.Length(); i++)
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
        auto values = std::vector<char>();
        for(int i=at; i<(at+count); i++)
            values.push_back(this->list->links[i]);
        return Napi::Buffer<char>::Copy(info.Env(),values.data(),values.size());
    }
    return info.Env().Undefined();
}