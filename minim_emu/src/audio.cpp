#include "minim.h"

int rtaudio_process( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
    float *out_buffer = (float *) outputBuffer;
    float *in_buffer = (float *) inputBuffer;
    libpd_process_float(1,in_buffer,out_buffer);
    return 0;
}
void pdprint(const char *s) {
	std::cout << "[libpd]" << s << std::endl;
}

int audio_init(){

    std::cout << "[audio]" << std::endl;
        std::cout << "libpd" << std::endl;
            libpd_set_printhook(pdprint);
            libpd_init();
            libpd_add_to_search_path("../deps/libpd/pure-data/extra");
            libpd_add_to_search_path("../../");
            libpd_add_to_search_path("../res");
            libpd_init_audio(0, 2, 44100);
            libpd_start_message(1); 
            libpd_add_float(1.0f);
            libpd_finish_message("pd", "dsp");
            if (!libpd_openfile("main.pd","../res"))
                return -1;
            
            if ( adac.getDeviceCount() < 1 ) 
                return -1;
            
            RtAudio::StreamParameters parameters;
            parameters.deviceId = adac.getDefaultOutputDevice();
            parameters.nChannels = 2;
            parameters.firstChannel = 0;
            unsigned int sampleRate = 44100;
            unsigned int bufferFrames = 64; 
            double data[2] = {0,0};
            try {
                adac.openStream( &parameters, NULL, RTAUDIO_FLOAT32,
                                sampleRate, &bufferFrames, &rtaudio_process, NULL );
                adac.startStream();
                std::cout << "rtaudio start" << std::endl;
            }
            catch ( RtAudioError& e ) {
                e.printMessage();
            return -1;
            } 
    
   return 0;
}

void audio_end()
{
	std::cout << "audio" << std::endl;
        try {
            adac.stopStream();
   		std::cout << "rtaudio end" << std::endl;
        }
        catch (RtAudioError& e) {
            e.printMessage();
        }
        if ( adac.isStreamOpen() ) adac.closeStream();
}
