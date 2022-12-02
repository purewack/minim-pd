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

#include <vector>

#define AUDIO_CLIP_SIZE 8388608 //8MB 8*1024*1024*4bytes ~ 43s @ 48kHz

inline RtAudio adac;
inline RtMidiIn* midiin;
inline std::vector<unsigned char> midiin_bytes;

void audio_end();
int audio_init();
int rtaudio_process (void *out_b, void *in_b, unsigned int nBufferFrames,
           double streamTime, RtAudioStreamStatus status, void *data );

#define MGFX_
#define MGFX_OK 0

int initGFXSystem();
int endGFXSystem();

int initMidiSystem();
int endMidiSystem();

void processSystems();