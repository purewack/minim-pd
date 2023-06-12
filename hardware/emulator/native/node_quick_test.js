const {ControlSurface} = require('./build/Release/minim.node');

const cs = new ControlSurface()
const midi = new Uint8Array([
    0x90,5,10,
    0xF0,0x00,0x7F,0x7F,1,
    114, 0, 0, 2, 2, 1,
    108, 2, 2, 5, 5,
    86,0,1,
    0x90,10,15
]);
cs.parseMIDIStreamUpdate(midi)
let cmds = cs.parseCommandListAtContext(1);
let draws = cs.parseMIDIStreamUpdate(midi);
// let ar = [...cs.asArrayAtContext(1,5).values()]
let list = [...cs.getCommandListAtContext(1).values()]
console.log(draws, cmds, list)
console.log('test map')

let links = [...cs.showLinksAtContext(1,8).values()];

cs.parseMIDIStreamUpdate(new Uint8Array([0x91,0,69]))
list = [...cs.getCommandListAtContext(1).values()]
console.log(list,links)
console.log('end test')