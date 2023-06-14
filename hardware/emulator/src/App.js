import { useEffect, useRef, useState } from 'react';
import ContextScreen from './Minim'
import { FlowMidi, InjectMidiPanel } from './MidiPrefs';

function App() {
  const [draws, setDraws] = useState([0,0,0,0,0,0])
  const [errorCmds, setErrorCmds] = useState([null,null,null,null,null,null])
  const [midiStream, setMidiStream] = useState('')

  const checkUpdatesAndErrors = ()=>{
    const d = [...window.ControlSurface.showParseUpdates().values()]
    let dr = [...draws]
    let er = [-1,-1,-1,-1,-1,-1]
    for(let i=0; i<6; i++){
      dr[i] += d[i]
      const ee = window.ControlSurface.showParseErrors(i)
      er[i] = ee >= 0 ? ee : null 
    }
    setDraws(dr)
    setErrorCmds(er)
  }

  return (
    <div className="App">
      <div className='MenuTitle'>
        {/* <span>MINIM Panel</span> */}
        <FlowMidi />
      </div>

      <span className='CurrentStream'>Stream: {midiStream}</span>
      
      <InjectMidiPanel streamCheck={checkUpdatesAndErrors} onNewStream={(s)=>{setMidiStream(s)}}/>
      
      <div className='MinimScreenArray'>
        <ContextScreen draws={draws[1]} errorAt={errorCmds[1]} contextNumber={1} horizontal={false}/>
        <ContextScreen draws={draws[2]} errorAt={errorCmds[2]} contextNumber={2} horizontal={false}/>
        <ContextScreen draws={draws[3]} errorAt={errorCmds[3]} contextNumber={3} horizontal={false}/>
        <ContextScreen draws={draws[4]} errorAt={errorCmds[4]} contextNumber={4} horizontal={false}/>
        <ContextScreen draws={draws[5]} errorAt={errorCmds[5]} contextNumber={5} horizontal={false}/>
        <ContextScreen draws={draws[0]} errorAt={errorCmds[0]} contextNumber={0} horizontal={true}/>
      </div>

    </div>
  );
}


export default App;