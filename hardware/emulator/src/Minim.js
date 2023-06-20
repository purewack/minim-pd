import { StreamCodeBlocks, symbolToBlock } from './Midi'
import './style/screens.css'
import {useEffect, useRef, useState} from 'react'

function plotCSContext(canvas, contextNumber, horizontal){
    const context = canvas.getContext('2d')
    const sty = getComputedStyle(canvas)
    context.fillStyle = sty.backgroundColor
    context.fillRect(0, 0, context.canvas.width, context.canvas.height)
    for(let y=0; y<64; y++){
        for(let x=0; x<128; x++){
            if(!window.ControlSurface.getPixelAtContext(contextNumber,x,y)) continue
            context.fillStyle = sty.color
            if(horizontal)
                context.fillRect(x,y,1,1);
            else 
                context.fillRect(64-y,x,1,1);
        }
    }
}

export default function ContextScreen({contextNumber, draws, horizontal = true, detail = false}){
    const canvasRef = useRef()

    useEffect(() => {
        if(canvasRef.current) plotCSContext(canvasRef.current,contextNumber,horizontal);
    }, [contextNumber, draws, horizontal])

    return (
        <div className={(draws.parseError ? 'ContextScreen Error ' : 'ContextScreen ') + 'C' + contextNumber }>
            <canvas 
                width={horizontal ? 128 : 64}
                height={horizontal ? 64 : 128}
                ref={canvasRef}
            />
            <span className='ContextScreenInfo'>{draws.renderCount}</span>
            {draws.parseError ? <span className='ContextScreenError'>Parse error at byte:{draws.parseError}</span> : null}
            {detail ? <div className='ContextDetail'>
                <h1>Context[{contextNumber}] Inspect</h1>
                <StreamCodeBlocks blockArray={
                    window.ControlSurface.getDisplayListAtContext(contextNumber).values().map(v => symbolToBlock(v))
                }/>
            </div> : null}
        </div>
    )
}
