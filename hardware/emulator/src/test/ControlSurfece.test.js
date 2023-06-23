const {ControlSurface} = require('../../native/build/Release/minim.node');

const CMD = (cmd)=>parseInt(ControlSurface.nameToSymbol[cmd])

describe('ControlSurface', ()=>{
    test('parse stream single context', ()=>{
        const stream = [
            0xF0,0x00,0x7F,0x7F,0,
            CMD('line'),4, 0, 0, 20, 21,
            0xF7
        ];
        const result = [CMD('line'),4, 0, 0, 20, 21];
        
        const cs = new ControlSurface();
        const draws = cs.parseMIDIStream(new Uint8Array(stream));
        const data = [...cs.getDisplayListAtContext(0).values()];
        const dataEmpty = [...cs.getDisplayListAtContext(1).values()];
        
        expect(data).toEqual(expect.arrayContaining(result));
        expect(dataEmpty).toEqual([])
        expect(draws).toBe(1);
    })

    test('parse stream 2 contexts', ()=>{
        const stream = [
            0xF0,0x00,0x7F,0x7F,0,
            CMD('line'),4, 0, 0, 20, 21,
            0xF0,0x00,0x7F,0x7F,1,
            CMD('rect'),5, 2, 3, 4, 5, 0,
            0xF7
        ];
        const result0 = [CMD('line'),4, 0, 0, 20, 21,];
        const result1 = [CMD('rect'),5, 2, 3, 4, 5, 0,];

        const cs = new ControlSurface();
        const draws = cs.parseMIDIStream(new Uint8Array(stream));
        const data0 = [...cs.getDisplayListAtContext(0).values()];
        const data1 = [...cs.getDisplayListAtContext(1).values()];
        
        expect(draws).toBe(2);
        expect(data0).toEqual(expect.arrayContaining(result0));
        expect(data1).toEqual(expect.arrayContaining(result1));
    })

    test('parse stream multi-context', ()=>{
        const stream = [
            0xF0,0x00,0x7F,0x7F,0,
            CMD('line'),4, 0, 0, 20, 21,
            
            0xF0,0x00,0x7F,0x7F,1,
            CMD('line'),4, 2, 3, 4, 5,
            
            0xF0,0x00,0x7F,0x7F,3,
            CMD('line'),4, 20, 30, 40, 50,

            0xF0,0x00,0x7F,0x7F,2,
            CMD('line'),4, 9, 8, 7, 6,

            0xF7
        ];
        const result0 = [CMD('line'),4, 0, 0, 20, 21,];
        const result1 = [CMD('line'),4, 2, 3, 4, 5];
        const result2 = [CMD('line'),4, 9, 8, 7, 6];
        const result3 = [CMD('line'),4, 20, 30, 40, 50,];

        const cs = new ControlSurface();
        const draws = cs.parseMIDIStream(new Uint8Array(stream));
        const data0 = [...cs.getDisplayListAtContext(0).values()];
        const data1 = [...cs.getDisplayListAtContext(1).values()];
        const data2 = [...cs.getDisplayListAtContext(2).values()];
        const data3 = [...cs.getDisplayListAtContext(3).values()];
        
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
            CMD('line'),4, 0, 0, 20, 21,
            0xF0,0x00,0x7F,0x7F,1,
            CMD('line'),4, 2, 3, 4, 5,
            0x90,10,15
        ];
        const result0 = [CMD('line'),4, 0, 0, 20, 21];
        const result1 = [CMD('line'),4, 2, 3, 4, 5];

        const cs = new ControlSurface();
        const draws = cs.parseMIDIStream(new Uint8Array(stream));
        const data0 = [...cs.getDisplayListAtContext(0).values()];
        const data1 = [...cs.getDisplayListAtContext(1).values()];
        
        expect(data0).toEqual(expect.arrayContaining(result0));
        expect(data1).toEqual(expect.arrayContaining(result1));
        expect(draws).toBe(4);
    })

    test('check parsed buffer contents',()=>{
        const stream = [
            0x90,5,10,
            0xF0,0x00,0x7F,0x7F,0,
            CMD('rect'),5, 0, 0, 2, 2, 1,
            CMD('line'),4, 2, 2, 5, 5,
            0x90,10,15
        ];
        const commands = [
            CMD('rect'),5, 0, 0, 2, 2, 1,
            CMD('line'),4, 2, 2, 5, 5,
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
        const cmds = [...cs.getDisplayListAtContext(0).values()];
        const pixels = [...cs.asArrayAtContext(0,5).values()]
        expect(draws).toBe(3);
        expect(cmds).toEqual(expect.arrayContaining(commands))
        expect(pixels).toEqual(expect.arrayContaining(buf))
    })

    test('variable map',()=>{
        const cs = new ControlSurface();

        let draws = cs.parseMIDIStreamUpdate(new Uint8Array([
            0xF0,0x00,0x7F,0x7F,2,
            CMD('rect'),5,0, 0, 2, 2, 1,
            CMD('line'),4, 2, 2, 5, 5,
            CMD('link'),2,0,1,
            0xF7
        ]));
        expect(draws).toBe(1);
        expect([...cs.getDisplayListAtContext(2).values()]).toEqual(expect.arrayContaining([
            CMD('rect'),5, 0, 0, 2, 2, 1,
            CMD('line'),4, 2, 2, 5, 5,
        ]))
        draws = cs.parseMIDIStream(new Uint8Array([
            0x92,0,69
        ]))
        expect(draws).toBe(1);
        expect([...cs.getDisplayListAtContext(2).values()]).toEqual(expect.arrayContaining([
            CMD('rect'),5, 0, 69, 2, 2, 1,
            CMD('line'),4, 2, 2, 5, 5,
        ]))
        
    })

    test('multi-variable map single context',()=>{
        const cs = new ControlSurface();

        let draws = cs.parseMIDIStreamUpdate(new Uint8Array([
            0xF0,0x00,0x7F,0x7F,2,
            CMD('rect'),5, 0, 0, 2, 2, 1,
            CMD('line'),4, 2, 2, 5, 5,
            CMD('link'),2,0,1,
            CMD('link'),2,0,8,
            0xF7
        ]));
        expect(draws).toBe(1);
        expect([...cs.getDisplayListAtContext(2).values()]).toEqual(expect.arrayContaining([
            CMD('rect'),5, 0, 0, 2, 2, 1,
            CMD('line'),4, 2, 2, 5, 5,
        ]))
        draws = cs.parseMIDIStream(new Uint8Array([
            0x92,0,69,
            0x92,1,120
        ]))
        expect(draws).toBe(2);
        expect([...cs.getDisplayListAtContext(2).values()]).toEqual(expect.arrayContaining([
            CMD('rect'),5, 0, 69, 2, 2, 1,
            CMD('line'),4, 2, 2, 5, 120,
        ]))
    })

    test('multi-variable map multi-context',()=>{
        const cs = new ControlSurface();

        let draws = cs.parseMIDIStreamUpdate(new Uint8Array([
            0xF0,0x00,0x7F,0x7F,2,
            CMD('rect'),5, 0, 0, 2, 2, 1,
            CMD('line'),4, 2, 2, 5, 5,
            CMD('link'),2,0,1,
            CMD('link'),2,0,8,
            0xF0,0x00,0x7F,0x7F,1,
            CMD('rect'),5, 1, 2, 3, 4, 5,
            CMD('link'),2,0,4,
            0xF7
        ]));
        expect(draws).toBe(2);
        expect([...cs.getDisplayListAtContext(2).values()]).toEqual(expect.arrayContaining([
            CMD('rect'),5, 0, 0, 2, 2, 1,
            CMD('line'),4, 2, 2, 5, 5,
        ]))
        expect([...cs.getDisplayListAtContext(1).values()]).toEqual(expect.arrayContaining([
            CMD('rect'),5, 1, 2, 3, 4, 5,
        ]))
        draws = cs.parseMIDIStream(new Uint8Array([
            0x92,0,69,
            0x92,1,120,
            0x91,0,42
        ]))
        expect(draws).toBe(3);
        expect([...cs.getDisplayListAtContext(2).values()]).toEqual(expect.arrayContaining([
            CMD('rect'),5, 0, 69, 2, 2, 1,
            CMD('line'),4, 2, 2, 5, 120,
        ]))
        expect([...cs.getDisplayListAtContext(1).values()]).toEqual(expect.arrayContaining([
            CMD('rect'),5, 1, 2, 3, 4, 42,
        ]))
        expect([...cs.showParseUpdates().values()]).toEqual(expect.arrayContaining([
            0,1,1,0,0,0
        ]))
    })

    test('invalid command stream',()=>{
        const cs = new ControlSurface();

        cs.parseMIDIStreamUpdate(new Uint8Array([
            0xF0,0x00,0x7F,0x7F,2,
            CMD('rect'),5, 0, 0, 2, 2, 1,
            20, 2, 2, 5, 5,
            0xF7
        ]));
        expect([...cs.showParseErrors().values()]).toEqual(expect.arrayContaining([
            0,0,1,0,0,0
        ]))

        cs.parseMIDIStreamUpdate(new Uint8Array([
            0xF0,0x00,0x7F,0x7F,2,
            CMD('rect'),5, 0, 0, 2, 2, 1,
            CMD('rect'),5, 2, 2, 5, 5,
            0xF7
        ]));
        expect([...cs.showParseErrors().values()]).toEqual(expect.arrayContaining([
            0,0,1,0,0,0
        ]))
    })

    test('command parse, context independent', ()=>{
        const cs = new ControlSurface();
        let names = []
        cs.parseMIDICommands(new Uint8Array([
            CMD('rect'),5, 0, 0, 2, 2, 1
        ]), (n)=>{
            names.push(n)
        });
        expect(names).toEqual(expect.arrayContaining(['rect']))
    })
})