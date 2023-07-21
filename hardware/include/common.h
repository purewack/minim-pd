#pragma once
#include <Arduino.h>
#include <USBMIDI.h>
#include <USBComposite.h>
#include <libmaple/i2c.h>
#include "../api/include/surface.h"
#include "../api/include/libdarray.h"
#include "../include/io.h"

extern USBCompositeSerial logger;
extern USBMIDI usbmidi;


namespace MINIM {

class ControlSurface: public API::ControlSurfaceAPI5{
    private:
        void beginOledSingle();
        void postToDisplays();
        void populateContextBuffer();
        unsigned int contextAddress;
        i2c_dev* contextDevice;
        i2c_msg imsg;
        
        sarray_t<uint8_t> displayCommands;
        sarray_t<uint8_t> midiString;

        unsigned int altering = 0;
    
    public:
        unsigned int midiBase = 36;
        bool isSysex();
        void initMemory();
        void initGPIO();
        void initDisplays();
        void collectMidi(uint8_t* b, int blen, int offset);
        void forceDrawContext(int ctx);
        void pollControls(hw_t & io);
        void pollDisplays();
        void logDisplayList(int ctx);
};

}

extern MINIM::ControlSurface cs;