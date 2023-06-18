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


export function InjectMidiPanel({stream, streamParser, setStream, checkStream, ...restProps}){
  const inputRef = useRef()
  const [code, setCode] = useState([])
  const cleanStreamWhitespace = (stream)=> stream.replace(/\s+/g, ' ').trim().split(' ')

  const buildStream = (cc)=>{
    inputRef.current.value = '';
    cc.forEach(c => {
      inputRef.current.value += c.symbol + ' ';
      c.arguments.forEach(a => {
        inputRef.current.value +=  '- '
      })
    })
  }

  return <div className='InjectMidiPanel' {...restProps} >

    <form onSubmit={(e)=>{
      e.preventDefault()
      const textStream = e.target.inputStream.value
      const textArray = cleanStreamWhitespace(textStream)
      const numArray = textArray.map(v => parseInt(v))
      // numArray.forEach(v => {
      //   if(v > 255 || v < 0) throw Error('invalid input stream at: ' + v)
      // });
      streamParser(new Uint8Array(numArray),(str)=>{
        console.log(str)
      })
      streamParser(new Uint8Array(numArray))
      checkStream()
      setStream(textStream)
    }}>
      <input type="submit" name="inject" value="Inject"/>
      <input ref={inputRef} type='text' name="inputStream" placeholder='numeric stream' className='inputStream'/>
      <input type="submit" name="inject" value="CLEAR" onClick={(e)=>{
        e.preventDefault()
        setCode([])
        inputRef.current.value = ''
      }}/>
    </form>

    <div className='codeblocks'>
        {code.map((c,i) => <span key={i + c.name} className={'codeblock ' + c?.type}>{c?.name}{c.arguments.length ? '(' + c.arguments + ')' : null}</span>)}
      </div>
    
    <div className='inputCommands'>
      {window.ControlSurface.getAPICommands().map(c => 
        <button key={'btn_'+c.name} onClick={()=>{
          setCode(arr => {
            let cc = [...arr]
              cc.push({name: c.name, type:c.type, symbol:c.symbol, arguments:[...Array(c.arguments).fill('-')], maxArguments:c.arguments})
              buildStream(cc)
            return cc;
          })
        }}>{c.name}</button>
      )}
      <span>:</span>
      <button onClick={()=>{
        let str = ''
        str += window.ControlSurface.getSymbol('start') + ' '
        str += '1 '
        str += window.ControlSurface.getSymbol('rect') + ' '
        str += '0 0 30 30 1 '
        str += window.ControlSurface.getSymbol('end')
        inputRef.current.value = str;
      }}>Insert Test Stream</button>
      </div>
  </div>
}