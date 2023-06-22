const {contextBridge, ipcRenderer} = require('electron')
const {ControlSurface} = require('../native/build/Release/minim.node')
const _cs = new ControlSurface()


contextBridge.exposeInMainWorld('app', {
    end: ()=>{ipcRenderer.send('end')}
})

// const sharedPainter = new BufferPainter()
contextBridge.exposeInMainWorld('ControlSurface', {
    parseMIDIStream: (streamUint8Array, parseHook = undefined) => _cs.parseMIDIStream(streamUint8Array, parseHook),
    parseMIDIStreamA: (array, parseHook = undefined) => _cs.parseMIDIStream(new Uint8Array(array), parseHook),
    parseMIDICommands: (streamUint8Array, parseHook = undefined) => _cs.parseMIDICommands(streamUint8Array, parseHook),
    parseMIDICommandsA: (array, parseHook = undefined) => _cs.parseMIDICommands(new Uint8Array(array), parseHook),
    
    showParseUpdates: ()=> [..._cs.showParseUpdates().values()],
    showParseErrors: ()=> [..._cs.showParseErrors().values()],
    showParseErrorLocation: (context)=>_cs.showParseErrorLocation(context),

    getPixelAtContext: (context, x,y)=> _cs.getPixelAtContext(context,x,y),
    getDisplayListAtContext:(context)=> [..._cs.getDisplayListAtContext(context).values()],
    showLinksAtContext: (context,count)=> [..._cs.showLinksAtContext(context,count).values()],
    parseDisplayListAtContext: (context)=> _cs.parseDisplayListAtContext(context).values(),
    
    getAPICommands: (name)=> {
        if(name === undefined)
            return [...ControlSurface.commands]
        return ControlSurface.commands[ControlSurface.nameToIdx[name]]
    },
    getSymbol: (name)=>{
        return ControlSurface.nameToSymbol[name]
    },
    getName: (symbol)=>{
        return ControlSurface.symbolToName[symbol]
    }
})
// contextBridge.exposeInMainWorld('BufferPainter', {
//     clear: ()=> {sharedPainter.clear()},
//     getPixel:(x,y) => sharedPainter.getPixel(x,y),
//     drawLine:(x,y,x2,y2,xo = false) => {sharedPainter.drawLine(x,y,x2,y2,xo)},
//     drawRect:(x,y,w,h,fill = false, xo = false) => {sharedPainter.drawRect(x,y,w,h,fill,xo)},
// })
