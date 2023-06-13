import './Minim.css'
import {useEffect, useRef} from 'react'
const {BufferPainter} = window.require('./native/build/Release/minim.node');
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
        for(let y=0; y<128; y++){
            for(let x=0; x<64; x++){
                if(!sharedPainter.getPixel(x,y)) continue
                context.fillStyle = 'white'
                context.fillRect(y,x,1,1);
            }
        }
    }
}

export default function MinimScreen({context, horizontal = true}){
    const canvasRef = useRef()
    useEffect(() => {
        const canvas = canvasRef.current
        const context = canvas.getContext('2d')
        sharedPainter.clear()
        sharedPainter.drawLine(0,0,127,63);
        sharedPainter.drawLine(0,63,127,0);
        sharedPainter.drawLine(0,32,127,32);
        sharedPainter.drawRect(0,0,128,64,false);
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