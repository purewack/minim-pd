#include <napi.h>
#include <iostream>

#include "../../../api/gfx.h"

using namespace std;
namespace MINIM{
    class ControlSurface : public Napi::ObjectWrap<ControlSurface> {
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports); //Init function for setting the export key to JS
        ControlSurface(const Napi::CallbackInfo& info); //Constructor to initialise

    private:
        static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
        BufferPainter* gfx; //internal instance of actualclass used to perform actual operations.
     
        Napi::Value asArray(const Napi::CallbackInfo& info);
        Napi::Value getPixel(const Napi::CallbackInfo& info);
        void _reset(const Napi::CallbackInfo& info);
        void _clear(const Napi::CallbackInfo& info);
        Napi::Value _drawHline(const Napi::CallbackInfo& info);
        Napi::Value _drawVline(const Napi::CallbackInfo& info);
        Napi::Value _drawLine(const Napi::CallbackInfo& info);
        Napi::Value _drawRectSize(const Napi::CallbackInfo& info);
        Napi::Value _fillSection(const Napi::CallbackInfo& info);
        // void _drawBitmap(const Napi::CallbackInfo& info);
        // void _drawChar(const Napi::CallbackInfo& info);
        // void _drawString(const Napi::CallbackInfo& info);
    };
};

Napi::Object InitAll(Napi::Env env, Napi::Object exports);
NODE_API_MODULE(minim, InitAll);