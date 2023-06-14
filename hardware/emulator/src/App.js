import { useEffect, useRef, useState } from 'react';
import ContextScreen from './Minim'
import { FlowMidi } from './MidiPrefs';

function App() {
  const [draws, setDraws] = useState([0,0,0,0,0,0])

  return (
    <div className="App">
      <div className='MenuTitle'>
        {/* <span>MINIM Panel</span> */}
        <FlowMidi />
        <button onClick={()=>{
        const cmds = new Uint8Array([
          0xF0,0x00,0x7F,0x7F,2,
          108, 0,0, 20,20,
          114, 0,0,10,10,1,
          0xF7,
        ])
        window.ControlSurface.parseMIDIStream(cmds)
        const v = [...window.ControlSurface.showParseUpdates().values()]
        v.forEach((i,v) => {
          setDraws(d => {
            let dd = [...d]
            dd[v] += i
            return [...dd]
          })
        })
      }}>Test Stream</button>
      </div>
      
      <div>
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