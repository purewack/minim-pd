const {BufferPainter} = require('../../native/build/Release/minim.node');

describe('BufferPainter',()=>{
    
    test('single pixel',()=>{
        const gfx = new BufferPainter() 
        gfx.clear();
        expect(gfx.getPixel(0,0)).toBe(0);
        gfx.drawLine(0,0,1,1);
        expect(gfx.getPixel(0,0)).toBe(1);
    })

    describe('4x4 section at (0,0)',()=>{
        test('horizontal line',()=>{
            const gfx = new BufferPainter()    
            gfx.clear();
            gfx.drawLine(0,0,4,0);
            {
                const data = [...gfx.asArray(4).values()]
                expect(data.length).toBe(16);
                expect(data).toEqual(expect.arrayContaining([
                    1,1,1,1,
                    0,0,0,0,
                    0,0,0,0,
                    0,0,0,0
                ]));
            }
        })

        test('diagonal',()=>{
            const gfx = new BufferPainter()   
            gfx.clear();
            gfx.drawLine(0,0,4,4);
            {
                const data = [...gfx.asArray(4).values()]
                expect(data.length).toBe(16);
                expect(data).toEqual(expect.arrayContaining([
                    1,0,0,0,
                    0,1,0,0,
                    0,0,1,0,
                    0,0,0,1
                ]));
            }
        })

        test('box',()=>{
            const gfx = new BufferPainter()

            gfx.clear();
            gfx.drawRect(0,0,3,3,true);
            {
                const data = [...gfx.asArray(4).values()]
                expect(data.length).toBe(16);
                expect(data).toEqual(expect.arrayContaining([
                    1,1,1,0,
                    1,1,1,0,
                    1,1,1,0,
                    0,0,0,0
                ]));
            }

            gfx.clear();
            gfx.drawRect(0,0,3,3,false);
            {
                const data = [...gfx.asArray(4).values()]
                expect(data.length).toBe(16);
                expect(data).toEqual(expect.arrayContaining([
                    1,1,1,0,
                    1,0,1,0,
                    1,1,1,0,
                    0,0,0,0
                ]));
            }
        })
    })


    describe('4x4 section at (1,0)',()=>{
        test('horizontal line',()=>{
            const gfx = new BufferPainter()    
            gfx.clear();
            gfx.drawLine(0,0,4,0);
            {
                const data = [...gfx.asArray(1,0,4,4).values()]
                expect(data.length).toBe(16);
                expect(data).toEqual(expect.arrayContaining([
                    1,1,1,0,
                    0,0,0,0,
                    0,0,0,0,
                    0,0,0,0
                ]));
            }
        })

        test('diagonal',()=>{
            const gfx = new BufferPainter()   
            gfx.clear();
            gfx.drawLine(0,0,4,4);
            {
                const data = [...gfx.asArray(1,0,4,4).values()]
                expect(data.length).toBe(16);
                expect(data).toEqual(expect.arrayContaining([
                    0,0,0,0,
                    1,0,0,0,
                    0,1,0,0,
                    0,0,1,0
                ]));
            }
        })

        test('box',()=>{
            const gfx = new BufferPainter()

            gfx.clear();
            gfx.drawRect(0,0,3,3,true);
            {
                const data = [...gfx.asArray(1,0,4,4).values()]
                expect(data.length).toBe(16);
                expect(data).toEqual(expect.arrayContaining([
                    1,1,0,0,
                    1,1,0,0,
                    1,1,0,0,
                    0,0,0,0
                ]));
            }

            gfx.clear();
            gfx.drawRect(0,0,3,3,false);
            {
                const data = [...gfx.asArray(1,0,4,4).values()]
                expect(data.length).toBe(16);
                expect(data).toEqual(expect.arrayContaining([
                    1,1,0,0,
                    0,1,0,0,
                    1,1,0,0,
                    0,0,0,0
                ]));
            }
        })
    })

    describe('16x16', () => { 
        test('fill all',()=>{
            const gfx = new BufferPainter()
            gfx.clear()
            gfx.drawRect(0,0,16,16,true);
            {
                const data = [...gfx.asArray(0,0,16,16).values()]
                expect(data.length).toBe(16*16);
                expect(data).toEqual(expect.arrayContaining(
                    Array.from(new Array(16*16), () => 1)));
            }
        })
    })
})