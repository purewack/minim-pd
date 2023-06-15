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
  


export function FlowMidi(){
  return <div className='MidiFlow'>
    <span className='In'/>
    <span className='Out'/>
    <span>
    MIDI
    </span>
  </div>
}


export function InjectMidiPanel({stream, setStream, checkStream}){
  const inputRef = useRef()

  return <div className='InjectMidiPanel' >
    <span>Stream:</span>
    <form onSubmit={(e)=>{
      e.preventDefault()
      const textStream = e.target.inputStream.value
      const textArray = textStream.replace(/\s+/g, ' ').trim().split(' ');
      const numArray = textArray.map(v => parseInt(v))
      // numArray.forEach(v => {
      //   if(v > 255 || v < 0) throw Error('invalid input stream at: ' + v)
      // });
      window.ControlSurface.parseMIDIStream(new Uint8Array(numArray))
      checkStream()
      setStream(textStream)
    }}>
      <input ref={inputRef} type='text' name="inputStream" placeholder={stream} className='inputStream'/>
      <input type="submit" name="inject" value="Inject"/>
      <button type="submit" name="start" onClick={()=>{
        if(inputRef.current){
          inputRef.current.value += ' '
        }
      }}>+Start</button>
      <button type="submit" name="start">+End</button>
    </form>
  </div>
}