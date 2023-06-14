const {ControlSurface} = require('../../native/build/Release/minim.node');

describe('ControlSurface', ()=>{
    test('parse stream single context', ()=>{
        const stream = [
            0xF0,0x00,0x7F,0x7F,0,
            108, 0, 0, 20, 21,
            0xF7
        ];
        const result = [108, 0, 0, 20, 21, 0];
        
        const cs = new ControlSurface();
        const draws = cs.parseMIDIStream(new Uint8Array(stream));
        const data = [...cs.getCommandListAtContext(0).values()];
        const dataEmpty = [...cs.getCommandListAtContext(1).values()];
        
        expect(data).toEqual(expect.arrayContaining(result));
        expect(dataEmpty).toEqual([])
        expect(draws).toBe(1);
    })

    test('parse stream 2 contexts', ()=>{
        const stream = [
            0xF0,0x00,0x7F,0x7F,0,
            108, 0, 0, 20, 21,
            0xF0,0x00,0x7F,0x7F,1,
            114, 2, 3, 4, 5, 0,
            0xF7
        ];
        const result0 = [108, 0, 0, 20, 21,];
        const result1 = [114, 2, 3, 4, 5, 0,];

        const cs = new ControlSurface();
        const draws = cs.parseMIDIStream(new Uint8Array(stream));
        const data0 = [...cs.getCommandListAtContext(0).values()];
        const data1 = [...cs.getCommandListAtContext(1).values()];
        
        expect(draws).toBe(2);
        expect(data0).toEqual(expect.arrayContaining(result0));
        expect(data1).toEqual(expect.arrayContaining(result1));
    })

    test('parse stream multi-context', ()=>{
        const stream = [
            0xF0,0x00,0x7F,0x7F,0,
            108, 0, 0, 20, 21,
            
            0xF0,0x00,0x7F,0x7F,1,
            108, 2, 3, 4, 5,
            
            0xF0,0x00,0x7F,0x7F,3,
            108, 20, 30, 40, 50,

            0xF0,0x00,0x7F,0x7F,2,
            108, 9, 8, 7, 6,

            0xF7
        ];
        const result0 = [108, 0, 0, 20, 21,];
        const result1 = [108, 2, 3, 4, 5];
        const result2 = [108, 9, 8, 7, 6];
        const result3 = [108, 20, 30, 40, 50,];

        const cs = new ControlSurface();
        const draws = cs.parseMIDIStream(new Uint8Array(stream));
        const data0 = [...cs.getCommandListAtContext(0).values()];
        const data1 = [...cs.getCommandListAtContext(1).values()];
        const data2 = [...cs.getCommandListAtContext(2).values()];
        const data3 = [...cs.getCommandListAtContext(3).values()];
        
        expect(draws).toBe(4);
        expect(data0).toEqual(expect.arrayContaining(result0));
        expect(data1).toEqual(expect.arrayContaining(result1));
        expect(data2).toEqual(expect.arrayContaining(result2));
        expect(data3).toEqual(expect.arrayContaining(result3));
    })

    test('parse stream multi-context with noteOn events', ()=>{
        const stream = [
            0x90,5,10,
            0xF0,0x00,0x7F,0x7F,0,
            108, 0, 0, 20, 21,
            0xF0,0x00,0x7F,0x7F,1,
            108, 2, 3, 4, 5,
            0x90,10,15
        ];
        const result0 = [108, 0, 0, 20, 21];
        const result1 = [108, 2, 3, 4, 5];

        const cs = new ControlSurface();
        const draws = cs.parseMIDIStream(new Uint8Array(stream));
        const data0 = [...cs.getCommandListAtContext(0).values()];
        const data1 = [...cs.getCommandListAtContext(1).values()];
        
        expect(data0).toEqual(expect.arrayContaining(result0));
        expect(data1).toEqual(expect.arrayContaining(result1));
        expect(draws).toBe(4);
    })

    test('check parsed buffer contents',()=>{
        const stream = [
            0x90,5,10,
            0xF0,0x00,0x7F,0x7F,1,
            114, 0, 0, 2, 2, 1,
            108, 2, 2, 5, 5,
            0x90,10,15
        ];
        const commands = [
            114, 0, 0, 2, 2, 1,
            108, 2, 2, 5, 5,
        ]
        const buf = [
            1,1,0,0,0,
            1,1,0,0,0,
            0,0,1,0,0,
            0,0,0,1,0,
            0,0,0,0,1
        ]

        const cs = new ControlSurface();
        const draws = cs.parseMIDIStreamUpdate(new Uint8Array(stream));
        const cmds = [...cs.getCommandListAtContext(1).values()];
        const pixels = [...cs.asArrayAtContext(1,5).values()]
        expect(draws).toBe(3);
        expect(cmds).toEqual(expect.arrayContaining(commands))
        expect(pixels).toEqual(expect.arrayContaining(buf))
    })

    test('variable map',()=>{
        const cs = new ControlSurface();

        let draws = cs.parseMIDIStreamUpdate(new Uint8Array([
            0xF0,0x00,0x7F,0x7F,2,
            114, 0, 0, 2, 2, 1,
            108, 2, 2, 5, 5,
            86,0,2,
            0xF7
        ]));
        expect(draws).toBe(1);
        expect([...cs.getCommandListAtContext(2).values()]).toEqual(expect.arrayContaining([
            114, 0, 0, 2, 2, 1,
            108, 2, 2, 5, 5,
        ]))
        draws = cs.parseMIDIStream(new Uint8Array([
            0x92,0,69
        ]))
        expect(draws).toBe(1);
        expect([...cs.getCommandListAtContext(2).values()]).toEqual(expect.arrayContaining([
            114, 0, 69, 2, 2, 1,
            108, 2, 2, 5, 5,
        ]))
        
    })

    test('multi-variable map single context',()=>{
        const cs = new ControlSurface();

        let draws = cs.parseMIDIStreamUpdate(new Uint8Array([
            0xF0,0x00,0x7F,0x7F,2,
            114, 0, 0, 2, 2, 1,
            108, 2, 2, 5, 5,
            86,0,2,
            86,0,10,
            0xF7
        ]));
        expect(draws).toBe(1);
        expect([...cs.getCommandListAtContext(2).values()]).toEqual(expect.arrayContaining([
            114, 0, 0, 2, 2, 1,
            108, 2, 2, 5, 5,
        ]))
        draws = cs.parseMIDIStream(new Uint8Array([
            0x92,0,69,
            0x92,1,120
        ]))
        expect(draws).toBe(2);
        expect([...cs.getCommandListAtContext(2).values()]).toEqual(expect.arrayContaining([
            114, 0, 69, 2, 2, 1,
            108, 2, 2, 5, 120,
        ]))
    })

    test('multi-variable map multi-context',()=>{
        const cs = new ControlSurface();

        let draws = cs.parseMIDIStreamUpdate(new Uint8Array([
            0xF0,0x00,0x7F,0x7F,2,
            114, 0, 0, 2, 2, 1,
            108, 2, 2, 5, 5,
            86,0,2,
            86,0,10,
            0xF0,0x00,0x7F,0x7F,1,
            114, 1, 2, 3, 4, 5,
            86,0,5,
            0xF7
        ]));
        expect(draws).toBe(2);
        expect([...cs.getCommandListAtContext(2).values()]).toEqual(expect.arrayContaining([
            114, 0, 0, 2, 2, 1,
            108, 2, 2, 5, 5,
        ]))
        expect([...cs.getCommandListAtContext(1).values()]).toEqual(expect.arrayContaining([
            114, 1, 2, 3, 4, 5,
        ]))
        draws = cs.parseMIDIStream(new Uint8Array([
            0x92,0,69,
            0x92,1,120,
            0x91,0,42
        ]))
        expect(draws).toBe(3);
        expect([...cs.getCommandListAtContext(2).values()]).toEqual(expect.arrayContaining([
            114, 0, 69, 2, 2, 1,
            108, 2, 2, 5, 120,
        ]))
        expect([...cs.getCommandListAtContext(1).values()]).toEqual(expect.arrayContaining([
            114, 1, 2, 3, 4, 42,
        ]))
        expect([...cs.showParseUpdates().values()]).toEqual(expect.arrayContaining([
            0,1,1,0,0,0
        ]))
    })

    test('invalid command stream',()=>{
        const cs = new ControlSurface();

        cs.parseMIDIStreamUpdate(new Uint8Array([
            0xF0,0x00,0x7F,0x7F,2,
            114, 0, 0, 2, 2, 1,
            20, 2, 2, 5, 5,
            0xF7
        ]));
        expect([...cs.showParseErrors().values()]).toEqual(expect.arrayContaining([
            0,0,1,0,0,0
        ]))

        cs.parseMIDIStreamUpdate(new Uint8Array([
            0xF0,0x00,0x7F,0x7F,2,
            114, 0, 0, 2, 2, 1,
            114, 2, 2, 5, 5,
            0xF7
        ]));
        expect([...cs.showParseErrors().values()]).toEqual(expect.arrayContaining([
            0,0,1,0,0,0
        ]))
    })
})