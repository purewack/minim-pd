import React from 'react';
import {useEffect, useRef, useState} from 'react'


export default function ContextScreen({contextNumber, errorWhere, horizontal = true, className,...restProps}){
    const canvasRef = useRef()    
    const animRef = useRef()
    const pixelRef = useRef(new Uint8Array(128*64))
    const [draws, setDraws] = useState(0)

    useEffect(() => {
        if(contextNumber === null || contextNumber === undefined) return
        const draw = ()=>{
            if(draws == 0 || window.ControlSurface.shouldUpdate(contextNumber)){
                pixelRef.current = window.ControlSurface.asArray(contextNumber)
                setDraws(d => d+1)
            }

            if(!canvasRef.current)return
            const context = canvasRef.current.getContext('2d')
            const sty = getComputedStyle(canvasRef.current)
            context.fillStyle = sty.backgroundColor
            context.fillRect(0, 0, context.canvas.width, context.canvas.height)
            
            if(pixelRef.current){
                for(let y=0; y<64; y++){
                    for(let x=0; x<128; x++){
                        if(pixelRef.current[x + y*128]){
                            context.fillStyle = sty.color
                            if(horizontal)
                                context.fillRect(x,y,1,1);
                            else
                                context.fillRect(64-y,x,1,1);
                        }
                    }
                }
            }
            animRef.current = requestAnimationFrame(draw)
        }
        // window.ControlSurface.parseDisplayListAtContext(contextNumber)
        draw()
        
        return ()=>{
            cancelAnimationFrame(animRef.current)
        }
    }, [contextNumber, horizontal, draws])

    return (
        <div className={(errorWhere !== null ? 'ContextScreen Error ' : 'ContextScreen ') + 'C' + contextNumber + ' ' + className }
            {...restProps}
        >
            <canvas 
                width={horizontal ? 128 : 64}
                height={horizontal ? 64 : 128}
                ref={canvasRef}
            />
            <span className='ContextScreenInfo'>{draws}</span>
            {errorWhere !== null ? <span className='ContextScreenError'>Parse error at displayList byte:{errorWhere}</span> : null}
        </div>
    )
}
