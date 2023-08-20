import { Children, useEffect, useRef, useState } from 'react';
import './style/midi.css'

export default function SettingsMidi({className, inputCallback, onNewOutput, ...restProps}){
    const [midiAccess, setMidiAccess] = useState(null)
    const [portList, setPortsList] = useState(null)
    const [inPort, setInPort] = useState(null)
    const [outPort, setOutPort] = useState(null)
    useEffect(()=>{
      if(midiAccess) return
      navigator.requestMIDIAccess({sysex:true}).then((midi)=>{
        setMidiAccess(midi); 
        midi.onstatechange = (event) => {
          // Print information about the (dis)connected MIDI controller
          console.log(event.port.name, event.port.manufacturer, event.port.state);
        };
      }, (msg)=>{
        console.error(`Failed to get MIDI access - ${msg}`);
      });    
    },[])

    useEffect(()=>{
      if(!midiAccess) return
      let ins = []
      let outs = []
      for (const entry of midiAccess.inputs) {
        ins.push(entry[1].name);
        if(inPort === entry[1].name){
          entry[1].onmidimessage = inputCallback
          console.log('set ', inPort, entry)
        }
        else 
          entry.onmidimessage = null
      }
      
      for (const entry of midiAccess.outputs){
        outs.push(entry[1].name);
        if(outPort === entry[1].name){
          onNewOutput?.(entry[1])
          console.log('set ', outPort, entry)
        }
      }
      
      setPortsList({input: [...ins], output: [...outs]})
    },[inPort,outPort,midiAccess])
  
    return <div className={className + ' SettingsMidi'}>
       {portList && <>
        <ul className='InputList'>
          {portList?.input?.length ? <>
            <p>Inputs</p>
            {portList.input.map((p,i) => <li 
              key={p + `_${i}`} 
              className={inPort === p ? 'Option Selected' :'Option'} 
              onClick={()=>{
                setInPort(p)
              }}
            >
              {p}
            </li>)}
          </>
          : 
          <p>-No inputs available-</p>
          }
        </ul>
        <p>{'>'}Emu{'>'}</p>
        <ul className='OutputList'>
          {portList?.output?.length ? <>
            <p>Outputs</p>
            {portList.output.map((p,i) => <li 
              key={p + `_${i}`} 
              className={outPort === p ? 'Option Selected' :'Option'} 
              onClick={()=>{
                setOutPort(p)
              }}
            >
              {p}
            </li>)}
          </>
          : 
          <p>-No outputs available-</p>
          }
        </ul>
      </>}
    </div>
  }
  
export function MidiFlowIndicator({inDevice, outDevice, children, ...restProps}){
  return <button className='MidiFlowIndicator' {...restProps}>
    <span className={'In '  + inDevice}/>
    <span className={'Out ' + outDevice}/>
    <span>
    MIDI
    </span>
    {children}
  </button>
}





const textToSymbolArray = (text)=> text.replace(/\s+/g, ' ').trim().split(' ').map(v => parseInt(v))

export function blocksToTextStream(blocks){
  if(!blocks) return ''
  let st = ''
  blocks.forEach(c => {
    st += window.ControlSurface.getSymbol(c.name) + ' '

    if(c.name === 'string')
      st += c.maxArguments + c.arguments[2].length + ' '
    else if(!(c.name === 'start' || c.name === 'end'))
      st += c.maxArguments + ' '
    c.arguments.forEach((a,i) => {
      if(c.name === 'string' && i==c.maxArguments-1){
        for(let si=0; si<a.length; si++){
          st += a.charCodeAt(si)
          st += ' '
        }
        st += '0 '
      }
      else{
        st += a
        st += ' '
      }
    })
  })
  return st
}

export function symbolToBlock(symbol){
  const name = window.ControlSurface.getName(symbol)
  const c = window.ControlSurface.getAPICommands(name)
  return { 
    ...c,
    arguments: (name === 'scale' ? [1] : [...Array(c.arguments).fill('0')]),
    maxArguments: c.arguments
  }
}



export function ContextDisplayList({displayListArray, linksArray, className, ...restProps}){

  const [list,setList] = useState([])
  
  useEffect(()=>{
    let l = []
    window.ControlSurface.parseMIDICommandsA(displayListArray,(type,where)=>{
      const c = window.ControlSurface.getAPICommands(type)
      const args = [...Array(c.arguments)].map((v,i)=> {
        const ii = where+2+i
        let link = false
        linksArray.forEach(v => {
          if(v===ii){
            link = true
            return
          }
        })
        return {
          value: displayListArray[ii],
          isLinked: link 
        }})
      l.push({...c, arguments:args})
    })
    setList(l)
  },[displayListArray,linksArray])

  return <ol className={'ContextDisplayList displaylist ' + className} {...restProps}>
    {list.map((c,i) => <li className={' ' + c.type} key={`displaylist_cmd_${i}`}>
      <span>
        {c.name}
        {'( '}
          {c.arguments.map((v,j) => 
            <span key={`displaylist_arg_${j}`} className={v.isLinked ? 'linked' : null}>
              {v.value} {' '}
            </span>
          )}
        {')'}
      </span>  
    </li>)}
  </ol>
}

export function ContextDisplayLinks({displayListArray, linksArray, onNewLinkValue, className, ...restProps}){

  return <ol className={'ContextDisplayLinks ' + className}>
    {linksArray.map((l,i) => 
      <li key={`linkslider_${l}_${i}`}><input type='range' defaultValue={displayListArray[l]} min={0} max={127} step={1} onChange={(e)=>{
        e.preventDefault()
        onNewLinkValue?.(l,i,parseInt(e.target.value))
      }}></input></li>
    )}
  </ol>
}


export function StreamCodeBlocks({blockArray, onNewArgument, onRemove}){
  const [isLinking, setIsLinking] = useState(false)

  return <div className='StreamCodeBlocks'>
    {blockArray && blockArray.map((c,i) => 
      <span 
        key={i + c.name} 
        data-name={c.name}
        className={'codeblock ' + c.type}
      >
        
        <p><span>{c.name}</span><span className='remove' onClick={()=>{onRemove(i)}}>X</span></p>
        <p><span>{'(' }</span>
        {c.name === 'link' ? 
          
          <div className='help' data-help="Link to a variable">
            <input 
              type='number'
              className={'address'} 
              value={0}  
              onChange={(e)=>{

              }} 
            />
          </div>
          
          :

          c.name === 'fill' ? 
                    
          <div className='help' data-help="Use XOR fill mode">
            <input 
              type='checkbox'
              className={'address'} 
              onChange={(e)=>{
                onNewArgument(i,0,e.target.checked ? 1 : 0);
              }} 
            />
          </div>
          
          : c.arguments.map((a,j) => 
              <div className='help' 
              key={`arg_${i}_${j}`} 
              data-help={window.ControlSurface.getAPICommands(c.name)[`arg_${j}`]}>
              <input 
                data-argindex={j}
                data-name={c.name}
                type='text'
                className={'argument'} 
                value={a}  
                onChange={(e)=>{
                  onNewArgument(i,j,e.target.value);
                }} 
              />
              </div>
          )}
        <span>{')'}</span></p> 
        
      </span>)}
  </div>
}

export function InjectMidiPanel({stream, onInject, ...restProps}){
  const inputRef = useRef()
  const [code, setCode] = useState([])
  const [insertHead, setInsertHead] = useState(0)

  useEffect(()=>{
    if(inputRef.current)inputRef.current.value = stream
  }, [stream])

  return <>

    <form className='StreamView' onSubmit={(e)=>{
      e.preventDefault()
      // const textStream = e.target.inputStream.value
      onInject(textToSymbolArray(e.target.inputStream.value),code,e.target.inputStream.value)
    }}>
      <input type="submit" name="inject" value="Inject" className='btnInject'/>
      <input ref={inputRef} type='text' name="inputStream" className='inputStream' placeholder='numeric stream' />
      <input type="button" name="clear" value="CLEAR" className='btnClear' onClick={(e)=>{
        e.preventDefault()
        setCode([])
        inputRef.current.value = ''
      }}/>
    </form>

    <StreamCodeBlocks blockArray={code} 
      newAt={insertHead}
      onNewArgument={(i,j,v)=>{
        setCode(c => {
          let cc = [...c]
          cc[i].arguments[j] = v
          inputRef.current.value = blocksToTextStream(cc)
          return cc
        })
      }}
      onRemove={(i)=>{
        setCode(c => [...c.filter((v,ii) => ii !== i)])
        setCode(c => {
          inputRef.current.value = blocksToTextStream(c)
          return c
        })
      }}
    />
    
    <div className='CommandButtons'>
      {window.ControlSurface.getAPICommands().map(c => 
        <button key={'btn_'+c.name} 
        className='CommandButton help'
        data-help={window.ControlSurface.getAPICommands(c.name).tooltip}
        onClick={()=>{
          setCode(cd => {
            let ccc = [...cd]
            ccc.splice(insertHead, 0, symbolToBlock(c.symbol))
            inputRef.current.value = blocksToTextStream(ccc)
            return ccc
          })
          setInsertHead(h => h+1)
        }}>{c.name}</button>
      )}
      <span>:</span>

      <button className='CommandButton' onClick={()=>{
        let str = ''
        str += window.ControlSurface.getSymbol('start') + ' '
        str += '0 '
        str += window.ControlSurface.getSymbol('rect') + ' '
        str += '5 0 0 15 15 0 '
        str += window.ControlSurface.getSymbol('rect') + ' '
        str += '5 15 15 15 15 1 '
        str += window.ControlSurface.getSymbol('start') + ' '
        str += '1 '
        str += window.ControlSurface.getSymbol('rect') + ' '
        str += '5 0 0 15 15 0 '
        str += window.ControlSurface.getSymbol('rect') + ' '
        str += '5 15 15 15 15 1 '
        str += window.ControlSurface.getSymbol('end')
        inputRef.current.value = str;
      }}>Insert Test Stream</button>

      </div>
  </>
}
