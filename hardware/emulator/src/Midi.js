import { Children, useEffect, useRef, useState } from 'react';
import './style/midi.css'

export default function SettingsMidi({className, inputCallback, outputDispatchArray, ...restProps}){
    const [midiAccess, setMidiAccess] = useState(null)
    const [portList, setPortsList] = useState(null)
    const [inPort, setInPort] = useState(null)
    const [outPort, setOutPort] = useState(null)
  
    useEffect(()=>{
      navigator.requestMIDIAccess({sysex:true}).then((midiAccess)=>{
        console.log("MIDI ready!", midiAccess);
        setMidiAccess(midiAccess); 
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
        
        for (const entry of midiAccess.outputs) 
          outs.push(entry[1].name);
        
        setPortsList({input: [...ins], output: [...outs]})
      }, (msg)=>{
        console.error(`Failed to get MIDI access - ${msg}`);
      });    
  
    },[inPort,outPort,inputCallback])
  
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
    if(!(c.name === 'start' || c.name === 'end'))
      st += c.maxArguments + ' '
    c.arguments.forEach(a => {
      st += a
      st += ' '
    })
  })
  return st
}

export function symbolToBlock(symbol, args){
  const name = window.ControlSurface.getName(symbol)
  const c = window.ControlSurface.getAPICommands(name)
  return { 
    ...c,
    arguments: (args ? args : [...Array(c.arguments).fill('0')]),
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
    {list.map((c,i) => <li className={' ' + c.type}>
      <span>
        {c.name}
        {'( '}
          {c.arguments.map((v,j) => 
            <span className={v.isLinked ? 'linked' : null}>
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
      <li><input type='range' defaultValue={displayListArray[l]} min={0} max={127} step={1} onChange={(e)=>{
        e.preventDefault()
        onNewLinkValue?.(l,i,parseInt(e.target.value))
      }}></input></li>
    )}
  </ol>
}


export function StreamCodeBlocks({blockArray, onNewArgument, onRemove}){
  
  return <div className='StreamCodeBlocks'>
    {blockArray && blockArray.map((c,i) => 
      <span 
        key={i + c.name} 
        className={'codeblock ' + c.type}
      >
        
        <p><span>{c.name}</span><span className='remove' onClick={()=>{onRemove(i)}}>X</span></p>
        <p><span>{'(' }</span>
        {c.arguments.map((a,j) => 
            <div className='help' 
            data-help={window.ControlSurface.getAPICommands(c.name)[`arg_${j}`]}>
            <input 
              key={`arg_${i}_${j}`} 
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
        className='CommandButton tooltipped'
        data-tooltip={window.ControlSurface.getAPICommands(c.name).tooltip}
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
        str += '1 '
        str += window.ControlSurface.getSymbol('rect') + ' '
        str += '5 0 0 30 30 1 '
        str += window.ControlSurface.getSymbol('end')
        inputRef.current.value = str;
      }}>Insert Test Stream</button>
      </div>
  </>
}
