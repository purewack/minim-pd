import { useEffect, useRef, useState } from 'react';
import MinimScreen from './Minim'
const { ipcRenderer } = window.require('electron');

function App() {
  const [menu, setMenu] = useState('midi io')

  return (
    <div className="App">
      {/* <MinimScreen />
      <MinimScreen />
      <MinimScreen />
      <MinimScreen />
      <MinimScreen /> */}
      {/* <InjectMidiPanel /> */}
      <div className='MenuTitle'>
        <span>{menu}</span>
        <FlowMidi />
      </div>
      <MinimScreen />
    </div>
  );
}

export function SettingsMidi({className, ...restProps}){
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

function FlowMidi(){
  return <div className='MidiFlow' onClick={()=>{
    ipcRenderer.send('openMIDIIO')
  }}>
    <span className='In'/>
    <span className='Out'/>
    <span>
    MIDI
    </span>
  </div>
}

function InjectMidiPanel(){
  return <form>
    <textarea name="inputStream" />
    <input type="submit" name="Inject" onClick={(e)=>{e.preventDefault()}}/>
  </form>
}

export default App;