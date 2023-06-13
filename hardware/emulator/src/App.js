import { useEffect, useRef, useState } from 'react';
import ContextScreen from './Minim'
import { FlowMidi } from './MidiPrefs';

function App() {
  const [menu, setMenu] = useState('Minim panel')

  return (
    <div className="App">
      <div className='MenuTitle'>
        <span>{menu}</span>
        <FlowMidi />
      </div>
      <button onClick={()=>{
        const cmds = new Uint8Array([
          0xF0,0x00,0x7F,0x7F,2,
          108, 0,0, 20,20,
          114, 0,0,10,10,1,
          0xF7,
        ])
        window.ControlSurface.parseMIDIStreamUpdate(cmds)
      }}>Test Stream</button>
      <div>
      <ContextScreen contextNumber={1} horizontal={false}/>
      <ContextScreen contextNumber={2} horizontal={false}/>
      <ContextScreen contextNumber={3} horizontal={false}/>
      <ContextScreen contextNumber={4} horizontal={false}/>
      <ContextScreen contextNumber={5} horizontal={false}/>
      <ContextScreen contextNumber={0} horizontal={true}/>
      </div>
    </div>
  );
}


export default App;