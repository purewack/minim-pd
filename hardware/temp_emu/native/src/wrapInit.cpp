#include "wrapMINIM.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
   MINIM::DisplayList::Init(env, exports);
   MINIM::BufferPainter::Init(env, exports);
   MINIM::ControlSurface::Init(env, exports);
   return exports;
}