#include <napi.h>
#include <iostream>

#include "../../../api/gfx.h"

using namespace std;
namespace MINIM{
    class BufferPainter : public Napi::ObjectWrap<BufferPainter> {
    private:
        API::BufferPainter* gfx;
        static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
           
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports); //Init function for setting the export key to JS
        BufferPainter(const Napi::CallbackInfo& info); //Constructor to initialise

        Napi::Value asArray(const Napi::CallbackInfo& info);
        Napi::Value getPixel(const Napi::CallbackInfo& info);
        Napi::Value drawPixel(const Napi::CallbackInfo& info);
        Napi::Value drawLine(const Napi::CallbackInfo& info);
        Napi::Value drawRect(const Napi::CallbackInfo& info);
        Napi::Value clear(const Napi::CallbackInfo& info);
        Napi::Value resetScaleRotate(const Napi::CallbackInfo& info);
    };

    class DisplayList : public Napi::ObjectWrap<DisplayList> {
    private:
        API::DisplayList* list;
        static Napi::FunctionReference constructor;
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports);
        DisplayList(const Napi::CallbackInfo& info);
    };

    // class ControlSurface : public Napi::ObjectWrap<ControlSurface> {
    // public:
    //     static Napi::Object Init(Napi::Env env, Napi::Object exports); //Init function for setting the export key to JS
    //     ControlSurface(const Napi::CallbackInfo& info); //Constructor to initialise

    // private:
    //     static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
    //     API::BufferPainter* gfx; //internal instance of actualclass used to perform actual operations.
     
    //     Napi::Value asArray(const Napi::CallbackInfo& info);
    //     Napi::Value getPixel(const Napi::CallbackInfo& info);
    //     void _reset(const Napi::CallbackInfo& info);
    //     void _clear(const Napi::CallbackInfo& info);
    //     Napi::Value _drawHline(const Napi::CallbackInfo& info);
    //     Napi::Value _drawVline(const Napi::CallbackInfo& info);
    //     Napi::Value _drawLine(const Napi::CallbackInfo& info);
    //     Napi::Value _drawRectSize(const Napi::CallbackInfo& info);
    //     Napi::Value _fillSection(const Napi::CallbackInfo& info);
    //     // void _drawBitmap(const Napi::CallbackInfo& info);
    //     // void _drawChar(const Napi::CallbackInfo& info);
    //     // void _drawString(const Napi::CallbackInfo& info);
    // };
};

Napi::Object InitAll(Napi::Env env, Napi::Object exports);
NODE_API_MODULE(minim, InitAll);