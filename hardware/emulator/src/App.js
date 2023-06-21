import { useEffect, useRef, useState } from 'react';
import ContextScreen from './Minim'
import { FlowMidi, InjectMidiPanel, StreamCodeBlocks, symbolToBlock } from './Midi';
import './style/app.css';

function App() {
  const [draws, setDraws] = useState([...Array(6).fill(0)])
  const [errors, setErrors] = useState([...Array(6).fill(null)])
  const [midiStream, setMidiStream] = useState('')
  const [inspectContext, setInspectContext] = useState(null)
  const screenOrder = [1,2,3,4,5,0]

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
        <FlowMidi />
        <span>Quit</span>
      </div>

      {<InjectMidiPanel 
        onInject={(symbols,code)=>{
          // console.log(symbols)
          window.ControlSurface.parseMIDIStream(new Uint8Array(symbols))
          checkUpdatesAndErrors()
        }} 
        stream={midiStream}
      />}
      
      {/* {inspectContext === null ?  */}
      <div className='MinimScreenArray'>
        {screenOrder.map(v => 
        
        inspectContext === v ?

        <div className='MinimScreenInspect'>
          <ContextScreen 
            key={`screen_context_${v}`}
            contextNumber={v} 
            draws={draws[v]}
            error={errors[v]}
            horizontal={v === 0}
            onClick={()=>{setInspectContext(null)}}
          />
          <div className='ContextDetail'>
              <h1>Context[{inspectContext}] Inspect</h1>
              <StreamCodeBlocks blockArray={[...window.ControlSurface.getDisplayListAtContext(inspectContext).values()].map(v => symbolToBlock(v))}/>
              <button onClick={()=>{setInspectContext(null)}}>Back</button>
          </div>
        </div>

        : 
        
        <ContextScreen 
          key={`screen_context_${v}`}
          contextNumber={v} 
          draws={draws[v]}
          error={errors[v]}
          horizontal={v === 0}
          onClick={()=>{setInspectContext(v)}}
        />)}
      </div>

    </div>
  );
}


export default App;