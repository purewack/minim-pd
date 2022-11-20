#include "minim.h"

// midiprobe.cpp
#include <iostream>
#include <cstdlib>
#include "RtMidi.h"
#include "RtAudio.h"

int main()
{
  RtMidiIn  *midiin = 0;
  RtMidiOut *midiout = 0;
  // RtMidiIn constructor
  try {
    midiin = new RtMidiIn();
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    exit( EXIT_FAILURE );
  }
  // Check inputs.
  unsigned int nPorts = midiin->getPortCount();
  std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
  std::string portName;
  for ( unsigned int i=0; i<nPorts; i++ ) {
    try {
      portName = midiin->getPortName(i);
    }
    catch ( RtMidiError &error ) {
      error.printMessage();
      goto cleanup;
    }
    std::cout << "  Input Port #" << i+1 << ": " << portName << '\n';
  }
  // RtMidiOut constructor
  try {
    midiout = new RtMidiOut();
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    exit( EXIT_FAILURE );
  }
  // Check outputs.
  nPorts = midiout->getPortCount();
  std::cout << "\nThere are " << nPorts << " MIDI output ports available.\n";
  for ( unsigned int i=0; i<nPorts; i++ ) {
    try {
      portName = midiout->getPortName(i);
    }
    catch (RtMidiError &error) {
      error.printMessage();
      goto cleanup;
    }
    std::cout << "  Output Port #" << i+1 << ": " << portName << '\n';
  }
  std::cout << '\n';
  // Clean up
 cleanup:
  delete midiin;
  delete midiout;


  RtAudio audio;
  // Determine the number of devices available
  unsigned int devices = audio.getDeviceCount();
  // Scan through devices for various capabilities
  RtAudio::DeviceInfo info;
  for ( unsigned int i=0; i<devices; i++ ) {
    info = audio.getDeviceInfo( i );
    if ( info.probed == true ) {
      // Print, for example, the maximum number of output channels for each device
      std::cout << "device = " << i;
      std::cout << ": maximum output channels = " << info.outputChannels << "\n";
    }
  }


  return 0;
}
