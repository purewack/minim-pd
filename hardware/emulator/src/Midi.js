import { useEffect, useRef, useState } from 'react';
import './style/midi.css'

export default function SettingsMidi({className, ...restProps}){
    const [midiAccess, setMidiAccess] = useState(null)
    const [portList, setPortsList] = useState(null)
    const [inPort, setInPort] = useState(null)
    const [outPort, setOutPort] = useState(null)
    const [fold, setFold] = useState(true)
  
    useEffect(()=>{
      navigator.requestMIDIAccess().then((midiAccess)=>{
        console.log("MIDI ready!", midiAccess);
        setMidiAccess(midiAccess); 
        let ins = []
        let outs = []
        for (const entry of midiAccess.inputs) 
          ins.push(entry[1].name);
        
        for (const entry of midiAccess.outputs) 
          outs.push(entry[1].name);
        
        setPortsList({input: [...ins], output: [...outs]})
      }, (msg)=>{
        console.error(`Failed to get MIDI access - ${msg}`);
      });    
  
    },[])
  
    return <div className={className + ' MidiSettings'}>
       {portList && <div className='MidiIO'>
        <div className='InputList'>
          <h1>-{'>'} To DSP</h1> 
          {portList?.input.map((p,i) => <p className={'Option'} key={p + `_${i}`}>{`${i}: ${p}`}</p>)}
        </div>
        <div className='OutputList'>
          <h1>From DSP -{'>'}</h1> 
          {portList?.output.map((p,i) => <p className={'Option'} key={p + `_${i}`}>{`${i}: ${p}`}</p>)}
        </div>
      </div>}
    </div>
  }
  
export function FlowMidi({...restProps}){
  return <div className='MidiFlow' {...restProps}>
    <span className='In'/>
    <span className='Out'/>
    <span>
    MIDI
    </span>
  </div>
}





const textToSymbolArray = (text)=> text.replace(/\s+/g, ' ').trim().split(' ').map(v => parseInt(v))

// export function textStreamToBlock(textStream){
//   let blocks = []
//   const textArray = cleanStreamWhitespace(textStream)
//   const numArray = textArray.map(v => parseInt(v))
//   window.ControlSurface.parseMIDICommands(new Uint8Array(numArray),(str,where)=>{
//     console.log(str,where)
//     if(!str.contains('err'))
//       blocks.push(symbolToBlock(str))
//   })
//   window.ControlSurface.parseMIDIStream(new Uint8Array(numArray))
//   return blocks
// }

export function blocksToTextStream(blocks){
  if(!blocks) return ''
  let st = ''
  blocks.forEach(c => {
    st += window.ControlSurface.getSymbol(c.name) + ' '
    c.arguments.forEach(a => {
      st += a
      st += ' '
    })
  })
  return st
}

export function symbolToBlock(symbol, args){
  const name = window.ControlSurface.getName(symbol)
  const c = window.ControlSurface.getAPICommands(name)
  return { 
    ...c,
    arguments: (args ? args : [...Array(c.arguments).fill('0')]),
    maxArguments: c.arguments
  }
}





export function StreamCodeBlocks({blockArray, onNewArgument, onRemove}){
  
  return <div className='codeblocks'>
    {blockArray && blockArray.map((c,i) => 
      <span 
        key={i + c.name} 
        className={'codeblock ' + c.type}
      >
        <p><span>{c.name}</span><span className='remove' onClick={()=>{onRemove(i)}}>X</span></p>
        <p><span>{'(' }</span>
        {c.arguments.map((a,j) => 
          <input className={'argument'} key={`arg_${i}_${j}`} value={a} type='text' onChange={(e)=>{
            onNewArgument(i,j,e.target.value);
          }}></input>
        )}
        <span>{')'}</span></p>
      </span>)}
  </div>
}

export function InjectMidiPanel({stream, onInject, ...restProps}){
  const inputRef = useRef()
  const [code, setCode] = useState([])
  const [insertHead, setInsertHead] = useState(0)

  return <div className='InjectMidiPanel' {...restProps} >

    <form onSubmit={(e)=>{
      e.preventDefault()
      // const textStream = e.target.inputStream.value
      onInject(textToSymbolArray(e.target.inputStream.value),code)
    }}>
      <input type="submit" name="inject" value="Inject" className='btnInject'/>
      <input ref={inputRef} type='text' name="inputStream" className='inputStream' placeholder='numeric stream' />
      <input type="button" name="make" value="Make" className='btnMake' onClick={(e)=>{
        e.preventDefault()
        inputRef.current.value = blocksToTextStream(code)
      }}/>
      <input type="button" name="clear" value="CLEAR" className='btnClear' onClick={(e)=>{
        e.preventDefault()
        setCode([])
        inputRef.current.value = ''
      }}/>
    </form>

    <StreamCodeBlocks blockArray={code} 
      newAt={insertHead}
      onNewArgument={(i,j,v)=>{
        setCode(c => {
          let cc = [...c]
          cc[i].arguments[j] = v
          inputRef.current.value = blocksToTextStream(cc)
          return cc
        })
      }}
      onRemove={(i)=>{
        setCode(c => [...c.filter((v,ii) => ii !== i)])
        setCode(c => {
          inputRef.current.value = blocksToTextStream(c)
          return c
        })
      }}
    />
    
    <div className='inputCommands'>
      {window.ControlSurface.getAPICommands().map(c => 
        <button key={'btn_'+c.name} onClick={()=>{
          setCode(cd => {
            let ccc = [...cd]
            ccc.splice(insertHead, 0, symbolToBlock(c.symbol))
            inputRef.current.value = blocksToTextStream(ccc)
            return ccc
          })
          setInsertHead(h => h+1)
        }}>{c.name}</button>
      )}
      {/* <span>:</span>
      <button onClick={()=>{
        let str = ''
        str += window.ControlSurface.getSymbol('start') + ' '
        str += '1 '
        str += window.ControlSurface.getSymbol('rect') + ' '
        str += '0 0 30 30 1 '
        str += window.ControlSurface.getSymbol('end')
        inputRef.current.value = str;
      }}>Insert Test Stream</button> */}
      </div>
  </div>
}
