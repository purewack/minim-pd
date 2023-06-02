#include <napi.h>
#include <iostream>

#include "../../../api/gfx.h"

using namespace std;
namespace GFX{
    Napi::String dummyWrap(const Napi::CallbackInfo& info);
    Napi::Object Init(Napi::Env env, Napi::Object exports);
    NODE_API_MODULE(gfx, Init)
}