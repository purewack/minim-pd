import { useEffect, useRef, useState } from 'react';
import ContextScreen from './Minim'
import { FlowMidi, InjectMidiPanel } from './Midi';
import './style/app.css';

function App() {
  const [draws, setDraws] = useState([...Array(6).fill({
    renderCount:0,
    parseError:null
  })])
  const [midiStream, setMidiStream] = useState('')
  const [midiInjectPanel, setMidiInjectPanel] = useState(false)

  const checkUpdatesAndErrors = ()=>{
    const d = [...window.ControlSurface.showParseUpdates().values()]
    let dr = [...draws]
    let error = false
    for(let i=0; i<6; i++){
      dr[i].renderCount += d[i].renderCount
      const ee = window.ControlSurface.showParseErrors(i)
      dr[i].parseError = ee >= 0 ? ee : null 
      error |= ee >= 0
    }
    setDraws(dr)
    return error
  }

  return (
    <div className="App">
      <div className='MenuTitle'>
        <span>MINIM</span>
        <FlowMidi onClick={()=>setMidiInjectPanel(s=>!s)}/>
        <span>Quit</span>
      </div>

      {<InjectMidiPanel 
        style={{display: midiInjectPanel ? 'block' : 'none'}}
        streamParser={window.ControlSurface.parseMIDIStream}
        checkStream={checkUpdatesAndErrors} 
        setStream={(s)=>{setMidiStream(s)}} 
        stream={midiStream}
      />}
      
      <div className='MinimScreenArray'>
        <ContextScreen draws={draws[1]} contextNumber={1} horizontal={false}/>
        <ContextScreen draws={draws[2]} contextNumber={2} horizontal={false}/>
        <ContextScreen draws={draws[3]} contextNumber={3} horizontal={false}/>
        <ContextScreen draws={draws[4]} contextNumber={4} horizontal={false}/>
        <ContextScreen draws={draws[5]} contextNumber={5} horizontal={false}/>
        <ContextScreen draws={draws[0]} contextNumber={0} horizontal={true}/>
      </div>

    </div>
  );
}


export default App;