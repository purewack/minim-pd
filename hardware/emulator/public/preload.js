const {contextBridge} = require('electron')
const {BufferPainter, ControlSurface} = require('../native/build/Release/minim.node')
const _cs = new ControlSurface()
// const sharedPainter = new BufferPainter()

contextBridge.exposeInMainWorld('ControlSurface', {
    parseMIDIStream: (streamUint8Array, parseHook = undefined) => _cs.parseMIDIStream(streamUint8Array, parseHook),
    parseMIDIStreamUpdate: (streamUint8Array) => _cs.parseMIDIStreamUpdate(streamUint8Array),

    getPixelAtContext: (context, x,y)=> _cs.getPixelAtContext(context,x,y),
    
    getCommandListAtContext:(context)=> _cs.getCommandListAtContext(context),
    parseCommandListAtContext: (context)=> _cs.parseCommandListAtContext(context),
    showParseUpdates: ()=> _cs.showParseUpdates(),
    showParseErrors: (context = undefined)=> _cs.showParseErrors(context),

    showLinksAtContext: (context,count)=> _cs.showLinksAtContext(context,count),
    
    getAPICommands: (name)=> {
        if(name === undefined)
            return [...ControlSurface.commands]
        return ControlSurface.commands[ControlSurface.indexes[name]]
    },
    getSymbol: (name)=>{
        return ControlSurface.commands[ControlSurface.names[name]].symbol
    }
})
// contextBridge.exposeInMainWorld('BufferPainter', {
//     clear: ()=> {sharedPainter.clear()},
//     getPixel:(x,y) => sharedPainter.getPixel(x,y),
//     drawLine:(x,y,x2,y2,xo = false) => {sharedPainter.drawLine(x,y,x2,y2,xo)},
//     drawRect:(x,y,w,h,fill = false, xo = false) => {sharedPainter.drawRect(x,y,w,h,fill,xo)},
// })
