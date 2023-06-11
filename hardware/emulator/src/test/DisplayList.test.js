const {DisplayList} = require('../../native/build/Release/minim.node');

describe('DisplayList',()=>{
    
    test('clear',()=>{
        const list = new DisplayList() 
        list.clear();
        const data = [...list.asArray().values()]
        expect(data.length).toBe(0);
    })

    test('add 0x80 to empty list', ()=>{
        const list = new DisplayList() 
        list.clear();
        list.add(0x80);
        const data = [...list.asArray().values()]
        expect(data.length).toBe(1);
        expect(data[0]).toBe(0x80);
    })

    describe('Byte stream', ()=>{
        const list = new DisplayList() 
        test('post 0x80 0x81 0x82 to empty list', ()=>{    
            list.clear();
            list.post([0x80,0x81,0x82]);
            const data = [...list.asArray().values()]
            expect(data.length).toBe(3);
            expect(data).toEqual(expect.arrayContaining([0x80,0x81,0x82]));
        })

        test('attempt modify list[2] = 0x11 without linkage', ()=>{
            list.modifyAt(0,0x11)
            const data = [...list.asArray().values()]
            expect(data.length).toBe(3);
            expect(data).toEqual(expect.arrayContaining([0x80,0x81,0x82]));
        })

        test('modify list[2] = 0x11 with linkage', ()=>{
            list.link(2,0);
            list.modifyAt(0,0x11)
            const data = [...list.asArray().values()]
            expect(data.length).toBe(3);
            expect(data).toEqual(expect.arrayContaining([0x80,0x81,0x11]));
            
            const links = [...list.showLinks(0,4).values()]
            expect(links).toEqual(expect.arrayContaining([2,255,255,255]));
        })
    })
})