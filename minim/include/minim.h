#pragma once

#include <iostream>
#include "raylib.h"
#include "z_libpd.h"

#if defined(_WIN32)           
	#define NOGDI             // All GDI defines and routines
	#define NOUSER            // All USER defines and routines
    // Type required before windows.h inclusion
    typedef struct tagMSG *LPMSG;
#endif

#include "RtAudio.h"
#include "RtMidi.h"

#if defined(_WIN32)           // raylib uses these names as function parameters
	#undef near
	#undef far
#endif

#if defined(_WIN32)  
#include <stdexcept>
#include <limits>
#endif


int initGFXSystem();
int endGFXSystem();
int postGFXCommand(std::string s);