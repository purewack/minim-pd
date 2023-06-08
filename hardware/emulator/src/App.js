import { useEffect, useRef, useState } from 'react';
import MinimScreen from './Minim'

function App() {
  const [midiAccess, setMidiAccess] = useState(null)
  const [portList, setPortsList] = useState(null)
  const [inPort, setInPort] = useState(null)
  const [outPort, setOutPort] = useState(null)

  useEffect(()=>{
    navigator.requestMIDIAccess().then((midiAccess)=>{
      console.log("MIDI ready!", midiAccess);
      setMidiAccess(midiAccess); // store in the global (in real usage, would probably keep in an object instance)
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

  return (
    <div className="App">
    <MinimScreen />
      {portList && <>
        <p>Inputs: </p> 
        {portList?.input.map((p,i) => <p key={p + `_${i}`}>{`${i}: ${p}`}</p>)}
        <p>Output: </p> 
        {portList?.output.map((p,i) => <p key={p + `_${i}`}>{`${i}: ${p}`}</p>)}
      </>}
    </div>
  );
}

export default App;