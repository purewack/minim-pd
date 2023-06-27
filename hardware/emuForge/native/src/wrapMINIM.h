#include <napi.h>
#include <iostream>

#include "gfx.h"
#include "surface.h"

namespace API{
    void autoBindCommands(Napi::Env env, Napi::Function& obj);
}

using namespace std;
namespace MINIM{
    class BufferPainter : public Napi::ObjectWrap<BufferPainter> {
    private:
        API::BufferPainter* gfx;
        static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
           
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports); //Init function for setting the export key to JS
        BufferPainter(const Napi::CallbackInfo& info); //Constructor to initialise

        static std::vector<uint8_t> asArrayFromBufferPainter(API::BufferPainter* gfx, int x=0, int y=0,int w=128,int h=64);
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

        Napi::Value asArray(const Napi::CallbackInfo & info);
        Napi::Value clear(const Napi::CallbackInfo & info);
        Napi::Value add(const Napi::CallbackInfo & info);
        Napi::Value post(const Napi::CallbackInfo & info);
        Napi::Value getCommandAt(const Napi::CallbackInfo & info);
        Napi::Value modifyAt(const Napi::CallbackInfo & info);
        Napi::Value link(const Napi::CallbackInfo & info);
        Napi::Value unlink(const Napi::CallbackInfo & info);

        Napi::Value showLinks(const Napi::CallbackInfo & info);
    };


    class ControlSurface : public Napi::ObjectWrap<ControlSurface> {
    private:
        static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
        API::ControlSurfaceAPI5* cs;
        
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports); //Init function for setting the export key to JS
        ControlSurface(const Napi::CallbackInfo& info); //Constructor to initialise

        Napi::Value asArray(const Napi::CallbackInfo& info);
        Napi::Value getPixelAtContext(const Napi::CallbackInfo& info);
        
        Napi::Value updateContext(const Napi::CallbackInfo& info);
        Napi::Value forceRefreshOne(const Napi::CallbackInfo& info);
        Napi::Value forceRefreshAll(const Napi::CallbackInfo& info);
        Napi::Value shouldUpdate(const Napi::CallbackInfo& info);
        Napi::Value parseDisplayListAtContext(const Napi::CallbackInfo& info);
        Napi::Value getDisplayListAtContext(const Napi::CallbackInfo& info);
        Napi::Value getLinksAtContext(const Napi::CallbackInfo& info);
        
        Napi::Value parseMidiStream(const Napi::CallbackInfo& info);
        Napi::Value parseMidiCommands(const Napi::CallbackInfo& info);
        
        Napi::Value showParseUpdates(const Napi::CallbackInfo& info);
        Napi::Value showParseErrors(const Napi::CallbackInfo& info);
        Napi::Value showParseErrorLocation(const Napi::CallbackInfo& info);
        Napi::Value hasError(const Napi::CallbackInfo& info);
    };

    struct ParseData{
        Napi::Function* callback;
        Napi::Env* env;
    };
};

Napi::Object InitAll(Napi::Env env, Napi::Object exports);
NODE_API_MODULE(minim, InitAll);