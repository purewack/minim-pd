const {ControlSurface} = require('./build/Release/minim.node');

const cs = new ControlSurface()
const midi = new Uint8Array([
    0x90,5,10,
    0xF0,0x00,0x7F,0x7F,1,
    114, 0, 0, 2, 2, 1,
    108, 2, 2, 5, 5,
    0x90,10,15
]);
cs.parseMIDIStreamUpdate(midi)
const cmds = cs.parseCommandListAtContext(1);
const draws = cs.parseMIDIStreamUpdate(midi);
const ar = [...cs.asArrayAtContext(1,5).values()]
const list = [...cs.getCommandListAtContext(1).values()]
console.log(draws, cmds, list,  ar)
