import './Minim.css'
import {useEffect, useRef, useState} from 'react'

function plotCSContext(context, contextNumber, horizontal){
    context.fillStyle = '#000000'
    context.fillRect(0, 0, context.canvas.width, context.canvas.height)
    for(let y=0; y<64; y++){
        for(let x=0; x<128; x++){
            if(!window.ControlSurface.getPixelAtContext(contextNumber,x,y)) continue
            context.fillStyle = 'white'

            if(horizontal)
                context.fillRect(x,y,1,1);
            else 
                context.fillRect(64-y,x,1,1);
        }
    }
}

export default function ContextScreen({contextNumber, horizontal = true, draws, errorAt}){
    const canvasRef = useRef()
    const update = (canvas)=>{plotCSContext(canvas.getContext('2d'),contextNumber,horizontal);}

    useEffect(() => {
        const cv = canvasRef.current
        if(cv) update(cv)
    }, [contextNumber, draws])

    return (
        <div className={(errorAt ? 'ContextScreen Error ' : 'ContextScreen ') + 'C' + contextNumber }>
        <canvas 
            width={horizontal ? 128 : 64}
            height={horizontal ? 64 : 128}
            ref={canvasRef}
            onClick={()=>{update(canvasRef.current)}}
        />
        <span className='ContextScreenInfo'>{draws}</span>
        {errorAt ? <span className='ContextScreenError'>Parse error at byte:{errorAt}</span> : null}
        </div>
    )
}