const {contextBridge, ipcRenderer} = require('electron')
const path = require('path')
console.log(__dirname)
const {ControlSurface} = require('minim.node')
// const _cs = new ControlSurface()
// console.log(_cs)

contextBridge.exposeInMainWorld('app', {
    end: ()=>{ipcRenderer.send('end')},
    dev: ()=>{ipcRenderer.send('dev')}
})
contextBridge.exposeInMainWorld('ControlSurface', {
    parseMIDIStream: (streamUint8Array, parseHook = undefined) => {},
    parseMIDIStreamA: (array, parseHook = undefined) => {},
    parseMIDICommands: (streamUint8Array, parseHook = undefined) => {},
    parseMIDICommandsA: (array, parseHook = undefined) =>  {},
    
    showParseUpdates: ()=> [0,0,0,0,0,0],
    showParseErrors: ()=> [0,0,0,0,0,0],
    showParseErrorLocation: (context)=> -1,
    hasError: (context)=> false,

    asArray: (context)=> new Uint8Array(),
    shouldUpdate: (context)=> false,
    getDisplayListAtContext:(context)=> [],
    getLinksAtContext: (context,count = undefined)=> [],
    parseDisplayListAtContext: (context)=> [],
  
    getAPICommands: (name)=> {
        if(name === undefined)
            return []
        return {}
    },
    getSymbol: (name)=>{
        return 0
    },
    getName: (symbol)=>{
        return 'name'
    }
})

// const sharedPainter = new BufferPainter()
// contextBridge.exposeInMainWorld('ControlSurface', {
//     parseMIDIStream: (streamUint8Array, parseHook = undefined) => _cs.parseMIDIStream(streamUint8Array, parseHook),
//     parseMIDIStreamA: (array, parseHook = undefined) => _cs.parseMIDIStream(new Uint8Array(array), parseHook),
//     parseMIDICommands: (streamUint8Array, parseHook = undefined) => _cs.parseMIDICommands(streamUint8Array, parseHook),
//     parseMIDICommandsA: (array, parseHook = undefined) => _cs.parseMIDICommands(new Uint8Array(array), parseHook),
    
//     showParseUpdates: ()=> [..._cs.showParseUpdates().values()],
//     showParseErrors: ()=> [..._cs.showParseErrors().values()],
//     showParseErrorLocation: (context)=>_cs.showParseErrorLocation(context),
//     hasError: (context)=>_cs.hasError(context),

//     asArray: (context)=> _cs.asArray(context),
//     shouldUpdate: (context)=> _cs.shouldUpdate(context),
//     getDisplayListAtContext:(context)=> [..._cs.getDisplayListAtContext(context).values()],
//     getLinksAtContext: (context,count = undefined)=> {
//         let links = []
//         let b = _cs.getLinksAtContext(context,count)
//         for(let i=0; i<b.length/2; i++)
//             links.push(b.readInt16LE(i*2));
//         return links
//     },
//     parseDisplayListAtContext: (context)=> _cs.parseDisplayListAtContext(context).values(),
  
//     getAPICommands: (name)=> {
//         if(name === undefined)
//             return [...ControlSurface.commands]
//         return ControlSurface.commands[ControlSurface.nameToIdx[name]]
//     },
//     getSymbol: (name)=>{
//         return ControlSurface.nameToSymbol[name]
//     },
//     getName: (symbol)=>{
//         return ControlSurface.symbolToName[symbol]
//     }
// })
