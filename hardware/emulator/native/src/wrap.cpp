#include "wrap.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
   return MINIM::BufferPainter::Init(env, exports);
}