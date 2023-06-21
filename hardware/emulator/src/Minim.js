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

export default function ContextScreen({contextNumber, draws, error, horizontal = true, detail = false, ...restProps}){
    const canvasRef = useRef()

    useEffect(() => {
        if(canvasRef.current) plotCSContext(canvasRef.current,contextNumber,horizontal);
    }, [contextNumber, draws, horizontal])

    return (
        <div className={(error ? 'ContextScreen Error ' : 'ContextScreen ') + 'C' + contextNumber }
            {...restProps}
        >
            <canvas 
                width={horizontal ? 128 : 64}
                height={horizontal ? 64 : 128}
                ref={canvasRef}
            />
            <span className='ContextScreenInfo'>{draws}</span>
            {error ? <span className='ContextScreenError'>Parse error at displayList byte:{error}</span> : null}
        </div>
    )
}
