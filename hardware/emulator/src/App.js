import { useState } from 'react';
import ContextScreen from './Minim'
import { ContextDisplayList, FlowMidi, InjectMidiPanel } from './Midi';
import './style/app.css';

function App() {
  const [draws, setDraws] = useState([...Array(6).fill(0)])
  const [errors, setErrors] = useState([...Array(6).fill(null)])
  const [midiStream, setMidiStream] = useState('')
  const [inspectContext, setInspectContext] = useState(null)
  const screenOrder = [1,2,3,4,5,0]

  const checkUpdatesAndErrors = ()=>{
    const d = window.ControlSurface.showParseUpdates()
    const e = window.ControlSurface.showParseErrors()
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
        <button onClick={()=>{
          window.app.end()
        }}>Quit</button>
      </div>
      
      <div className='AppContent'>
        {<InjectMidiPanel 
          onInject={(symbols,code,stream)=>{
            // console.log(symbols)
            setMidiStream(stream)
            window.ControlSurface.parseMIDIStream(new Uint8Array(symbols),(c)=>{
              console.log(c)
            })
            window.ControlSurface.parseMIDIStream(new Uint8Array(symbols))
            checkUpdatesAndErrors()
          }} 
          stream={midiStream}
        />}
        
        <div className='MinimScreenArray'>
          {screenOrder.map(v => 
            <ContextScreen 
              key={`screen_context_${v}`}
              contextNumber={v} 
              draws={draws[v]}
              error={errors[v]}
              horizontal={v === 0}
              onClick={()=>{setInspectContext(v)}}
            />
          )}
          {inspectContext !== null ?

          <div className='MinimScreenInspect' 
          onClick={()=>{setInspectContext(null)}}>
            <ContextScreen 
              className={inspectContext === 0 ? 'Hor' : ''}
              key={`screen_context_${inspectContext}`}
              contextNumber={inspectContext} 
              draws={draws[inspectContext]}
              error={errors[inspectContext]}
              horizontal={inspectContext === 0}
              onClick={()=>{setInspectContext(null)}}
            />
            <div className={'ContextDetail'}>
                <h1>Context[{inspectContext}] Inspect</h1>
                <ContextDisplayList stream={window.ControlSurface.getDisplayListAtContext(inspectContext)}/>
            </div>
          </div>
          : null}

        </div>
      </div>
    </div>
  );
}


export default App;