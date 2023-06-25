import { useEffect, useState } from 'react';
import ContextScreen from './Minim'
import SettingsMidi, { ContextDisplayLinks, ContextDisplayList, MidiFlowIndicator, InjectMidiPanel } from './Midi';
import './style/app.css';

function App() {
  const [draws, setDraws] = useState([...Array(6).fill(0)])
  const [errors, setErrors] = useState([...Array(6).fill(null)])
  const [midiStream, setMidiStream] = useState('')
  const [inspectContext, setInspectContext] = useState(null)
  const screenOrder = [1,2,3,4,5,0]

  const [optPanel, setOptPanel] = useState(false)

  const [ioPanel, setIoPanel] = useState(false)
  const [inDevice, setInDevice] = useState(null)
  const [outDevice, setOutDevice] = useState(null)

  const [darkMode, setDark] = useState(false)
  const setDarkMode = (v)=>{
    setDark(v)
    setTimeout(()=>{setDraws(d => d.map(v => v+1))},300)
  }
  useEffect(()=>{
    const darklisten = event => {
      setDarkMode(event.matches)
    }
    if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
      setDarkMode(true)
    }
  
    window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', darklisten);
    return ()=>{
      window.matchMedia('(prefers-color-scheme: dark)').removeEventListener('change', darklisten);
    }
  },[])

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
    <div className={darkMode ? "App ThemeDark" : "App"}>
      <div className='MenuTitle'>
        <button className='MinimButton'>
          MINIM
          <div className='OptionPanel'>
            <button onClick={()=>{window.app.dev()}}>Dev tools</button>
            <button onClick={()=>{setDarkMode(t=>!t)}}>Theme</button>
          </div>
        </button>
        <MidiFlowIndicator onClick={()=>{
          setIoPanel(i => !i)
        }}>
          <SettingsMidi inputCallback={(ev)=>{
            console.log(ev.data)
            let s = ''
            for(let i=0; i<ev.data.length; i++){
              s += ev.data[i] + ' '
            }
            window.ControlSurface.parseMIDIStream(ev.data)
            checkUpdatesAndErrors()
            setMidiStream(s)
          }}/>
        </MidiFlowIndicator>
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

          <div className='MinimScreenInspect' >
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
                <h2>DisplayList:</h2>
                <ContextDisplayList 
                  displayListArray={window.ControlSurface.getDisplayListAtContext(inspectContext)}
                  linksArray={window.ControlSurface.getLinksAtContext(inspectContext)}
                />
                <h2>Links:</h2>
                <ContextDisplayLinks
                  displayListArray={window.ControlSurface.getDisplayListAtContext(inspectContext)}
                  linksArray={window.ControlSurface.getLinksAtContext(inspectContext)}
                  onNewLinkValue={(link,idx,value)=>{
                    window.ControlSurface.parseMIDIStreamA([(0x90 | inspectContext),idx,value])
                    checkUpdatesAndErrors()
                  }}
                />
            </div>
          </div>
          : null}

        </div>
      </div>
    </div>
  );
}


export default App;