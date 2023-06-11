const {ControlSurface} = require('../../native/build/Release/minim.node');

describe('ControlSurface', ()=>{
    test('parse stream single context', ()=>{
        const stream = [
            0xF0,0x00,0x7F,0x7F,0,
            108, 0, 0, 20, 21, 0,
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
            108, 0, 0, 20, 21, 0,
            0xF0,0x00,0x7F,0x7F,1,
            1, 2, 3, 4, 5, 6,
            0xF7
        ];
        const result0 = [108, 0, 0, 20, 21, 0];
        const result1 = [1, 2, 3, 4, 5, 6];

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
            108, 0, 0, 20, 21, 0,
            
            0xF0,0x00,0x7F,0x7F,1,
            1, 2, 3, 4, 5, 6,
            
            0xF0,0x00,0x7F,0x7F,3,
            10, 20, 30, 40, 50, 60,

            0xF0,0x00,0x7F,0x7F,2,
            10, 9, 8, 7, 6, 5,

            0xF7
        ];
        const result0 = [108, 0, 0, 20, 21, 0];
        const result1 = [1, 2, 3, 4, 5, 6];
        const result2 = [10, 9, 8, 7, 6, 5];
        const result3 = [10, 20, 30, 40, 50, 60];

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
            108, 0, 0, 20, 21, 0,
            0xF0,0x00,0x7F,0x7F,1,
            1, 2, 3, 4, 5, 6,
            0x90,10,15
        ];
        const result0 = [108, 0, 0, 20, 21, 0];
        const result1 = [1, 2, 3, 4, 5, 6];

        const cs = new ControlSurface();
        const draws = cs.parseMIDIStream(new Uint8Array(stream));
        const data0 = [...cs.getCommandListAtContext(0).values()];
        const data1 = [...cs.getCommandListAtContext(1).values()];
        
        expect(data0).toEqual(expect.arrayContaining(result0));
        expect(data1).toEqual(expect.arrayContaining(result1));
        expect(draws).toBe(4);
    })
})