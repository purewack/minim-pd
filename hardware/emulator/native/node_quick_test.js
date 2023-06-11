const {ControlSurface} = require('./build/Release/minim.node');

const cs = new ControlSurface()
cs.parseMIDIStream(new Uint8Array([0xf0,0,0x7f,0x7f,0,0xf7]));