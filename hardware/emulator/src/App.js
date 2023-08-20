import { useEffect, useState } from 'react';
import ContextScreen from './Minim'
import SettingsMidi, { ContextDisplayLinks, ContextDisplayList, MidiFlowIndicator, InjectMidiPanel } from './Midi';
import './style/app.css';

async function asyncDisplayStream(streamRaw, setter){
  let s = ''
  for(let i=0; i<streamRaw.length; i++){
    s += streamRaw[i] + ' '
  }
  setter(s)
}

function commitStream(streamUint8, onError){
  let anyError = false
  window.ControlSurface.parseMIDIStream(streamUint8)
  window.ControlSurface.showParseErrors()
  .forEach((hasError,where) => {
    if(hasError === 1) {
      onError?.(where,window.ControlSurface.showParseErrorLocation(where))
      anyError |= true
    }
    else{
      onError?.(where,null)
    }
  })
  return anyError
}

function App() {
  const screenOrder = [1,2,3,4,5,0]
  const [errors, setErrors] = useState([...Array(6).fill(null)])
  const [midiStream, setMidiStream] = useState('')
  const [realOutputInjectPort,  setRealOutputInjectPort] = useState();
  const [inspectContext, setInspectContext] = useState(null)

  const [optPanel, setOptPanel] = useState(false)

  const [ioPanel, setIoPanel] = useState(false)
  const [inDevice, setInDevice] = useState(null)
  const [outDevice, setOutDevice] = useState(null)

  const [darkMode, setDarkMode] = useState(false)
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

  const errorHandle = (context, where)=>{
    setErrors(e => {
      let ee = [...e]
      ee[context] = where
      return ee
    })
  }

  return (
    <div className={darkMode ? "App ThemeDark" : "App"}>
      <div className='MenuTitle'>
        <div className='DropDown'>
          <button className='MinimButton'>
            MINIM
          </button>
          <div className='DropDownContent'>
            <button onClick={()=>{window.app.dev()}}>Dev tools</button>
            <button role='button' onClick={()=>{setDarkMode(t=>!t)}}>Theme</button>
          </div>
        </div>

        <MidiFlowIndicator onClick={()=>{
          setIoPanel(i => !i)
        }}>
          <SettingsMidi inputCallback={(ev)=>{
            //incoming real midi data
            commitStream(ev.data,errorHandle)
            asyncDisplayStream(ev.data,setMidiStream)
          }}
          onNewOutput={(port)=>{
            setRealOutputInjectPort(port)
          }}
          />
        </MidiFlowIndicator>
        <button onClick={()=>{
          window.app.end()
        }}>Quit</button>

      </div>
      
      <div className='AppContent'>
        {<InjectMidiPanel 
          onInject={(symbols,code,stream)=>{
            //incoming injected midi data
            const ss = new Uint8Array(symbols)
            commitStream(ss,errorHandle)
            setMidiStream(stream)
            if(realOutputInjectPort) realOutputInjectPort.send(ss)
          }} 
          stream={midiStream}
        />}
        
        <div className='MinimScreenArray'>
          {screenOrder.map(v => inspectContext === v ?
  
            <div className='MinimScreenInspect' key={`inspect_${inspectContext}`} >
              <ContextScreen 
                className={inspectContext === 0 ? 'Hor' : ''}
                key={`screen_context_${inspectContext}`}
                contextNumber={inspectContext} 
                horizontal={inspectContext === 0}
                errorWhere={errors[inspectContext]}
                onClick={()=>{setInspectContext(null)}}
              />
              <div className={'ContextDetail'}>
                  <h1>Context[{inspectContext}] Inspect</h1>
                  <h2>Code:</h2>
                  <code>{window.ControlSurface.getDisplayListAtContext(inspectContext).map((s,i) => 
                    <span key={`code_${inspectContext}_${i}`}>{s} </span>  
                  )}</code>
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
                      commitStream(new Uint8Array([(0x90 | inspectContext),idx,value]),errorHandle)
                      setMidiStream(`${0x90 | inspectContext} ${idx} ${value}`)
                    }}
                  />
              </div>
            </div>
            
          : 
            
            <ContextScreen 
              key={`screen_context_${v}`}
              contextNumber={v} 
              horizontal={v === 0}
              errorWhere={errors[v]}
              onClick={()=>{setInspectContext(v)}}
            />

          )}

        </div>
      </div>
    </div>
  );
}


export default App;