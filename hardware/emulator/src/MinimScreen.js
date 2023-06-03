import './MinimScreen.css'
import {useEffect, useRef} from 'react'
const {BufferPainter} = window.require('./native/build/Release/gfx.node');
const sharedPainter = new BufferPainter()

function plotBuffer(context, horizontal){
    context.fillStyle = '#000000'
    context.fillRect(0, 0, context.canvas.width, context.canvas.height)

    if(horizontal){
        for(let y=0; y<64; y++){
            for(let x=0; x<128; x++){
                if(!sharedPainter.getPixel(x,y)) continue
                context.fillStyle = 'white'
                context.fillRect(x,y,1,1);
            }
        }
    }
    else{
        // for(let y=0; y<128; y++){
        //     for(let x=0; x<64; x++){
        //         if(!sharedPainter.getPixel(x,y)) continue
        //         context.fillStyle = 'white'
        //         context.fillRect(x,y,1,1);
        //     }
        // }
    }
}

export default function MinimScreen({horizontal = true}){
    const canvasRef = useRef()
    useEffect(() => {
        const canvas = canvasRef.current
        const context = canvas.getContext('2d')
        sharedPainter._clear()
        sharedPainter._drawLine(0,0,127,63);
        sharedPainter._drawLine(0,63,127,0);
        sharedPainter._drawHline(0,32,127);
        sharedPainter._drawRectSize(0,0,128,64);
        plotBuffer(context, horizontal)
    }, [])

    return (
        <canvas 
            className='MinimScreen'
            width={horizontal ? 128 : 64}
            height={horizontal ? 64 : 128}
            ref={canvasRef}
        />
    )
}