import { useEffect, useRef, useState } from 'react';
import ContextScreen from './Minim'
import { FlowMidi, InjectMidiPanel, StreamCodeBlocks, symbolToBlock } from './Midi';
import './style/app.css';

function App() {
  const [draws, setDraws] = useState([...Array(6).fill(0)])
  const [errors, setErrors] = useState([...Array(6).fill(null)])
  const [midiStream, setMidiStream] = useState('')
  const [midiInjectPanel, setMidiInjectPanel] = useState(false)
  const [inspectContext, setInspectContext] = useState(null)

  const onInspect = (c)=>{
    setInspectContext(c)
  }

  const checkUpdatesAndErrors = ()=>{
    const d = [...window.ControlSurface.showParseUpdates().values()]
    const e = [...window.ControlSurface.showParseErrors().values()]
    let dr = [...draws]
    let er = [...errors]
    let error = false
    dr.forEach((v,i)=>{
      dr[i] += d[i]
      er[i] = e[i] ? window.ControlSurface.showParseErrorLocation(i) : null 
      error |= e[i]
    })
    setDraws(dr)
    setErrors(er)
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
        onInject={(symbols,code)=>{
          // console.log(symbols)
          window.ControlSurface.parseMIDIStream(new Uint8Array(symbols))
          checkUpdatesAndErrors()
        }}
        // streamParser={window.ControlSurface.parseMIDIStream}
        // checkStream={checkUpdatesAndErrors} 
        // setStream={(s)=>{setMidiStream(s)}} 
        stream={midiStream}
      />}
      
      {inspectContext === null ? 
      <div className='MinimScreenArray'>
        <ContextScreen draws={draws[1]} error={errors[1]} contextNumber={1} horizontal={false} onClick={()=>{setInspectContext(1)}}/>
        <ContextScreen draws={draws[2]} error={errors[2]} contextNumber={2} horizontal={false} onClick={()=>{setInspectContext(2)}}/>
        <ContextScreen draws={draws[3]} error={errors[3]} contextNumber={3} horizontal={false} onClick={()=>{setInspectContext(3)}}/>
        <ContextScreen draws={draws[4]} error={errors[4]} contextNumber={4} horizontal={false} onClick={()=>{setInspectContext(4)}}/>
        <ContextScreen draws={draws[5]} error={errors[5]} contextNumber={5} horizontal={false} onClick={()=>{setInspectContext(5)}}/>

        <ContextScreen draws={draws[0]} error={errors[0]} contextNumber={0} horizontal={true} onClick={()=>{setInspectContext(0)}}/>
      </div>
      : 
      <div className='MinimScreenInspect'>
        <ContextScreen 
          draws={draws[inspectContext]} 
          error={errors[inspectContext]} 
          contextNumber={inspectContext} 
          horizontal={(inspectContext === 0)} 
          detail={true}
        />
        <div className='ContextDetail'>
            <h1>Context[{inspectContext}] Inspect</h1>
            <StreamCodeBlocks blockArray={[...window.ControlSurface.getDisplayListAtContext(inspectContext).values()].map(v => symbolToBlock(v))}/>
            <button onClick={()=>{setInspectContext(null)}}>Back</button>
        </div>
      </div>}

    </div>
  );
}


export default App;