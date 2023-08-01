# API 5 Concepts

Commands are stored in a command pool (DisplayList) per graphics context (screen), that means each display can have an image on screen different from other screens' as well as have variables which can be sent without sending all DisplayList code.

These commands can be sent as raw midi sysex messages to the control surface to manipulate display contents.

Example code stream:
```
    0xF0 0x00 0x7F 0x7F 2 r 0 0 100 200 1 0xF0 0x00 0x7F 0x7F 1 s 2 4 hello 0xF7
```
Prettyfied:
```
    (0xF0 0x00 0x7F 0x7F) 2
        r 0 0 100 200 1
    (0xF0 0x00 0x7F 0x7F) 1
        s 2 4 "hello"
    (0xF7)
```
Expained raw code Stream:
<pre>
    <i style='color:green'>sysex start and context address {<b>0xF0 0x00 0x7F 0x7F</b>}</i> 
        2 <i>(Context 2)</i>
        114 <i>(rect)</i>
        0,0,100,200,1 <i>(x=0, y=0, width=100, height=200, fill=true)</i>
    <i style='color:green'>sysex start and context address  {<b>0xF0 0x00 0x7F 0x7F</b>}</i> 
        1 <i>(Context 2)</i>
        115 <i>(text)</i>
        2,4 <i>(x=2, y=4)</i>
        104,101,108,108,111,0 <i>(ascii 'hello' null terminated)</i>
    <i style='color:orange'>sysex end flag {<b>0xF7</b> or any <b>status</b> byte}</i>
</pre>

Meaning:

`g` is graphics command flag,argument list is assumed and not checked for count of arguments so take heed.
`2` is the first argument in this case

Upon recieving a `0xF0 0x00 0x7F 0x7F` string, the parser switches to sysex mode until recieving another status byte, typically a sysex end flag `0xF7`. All bytes (exluding id tag `0x00 0x7F 0x7F`) are interpreted as commands.


## Display List Commands

`S{factor}` - Set Graphics drawing scale for bitmaps and text to {factor} integer value

`X{mode}` - Set Graphics drawing mode, if non zero use XOR drawing mode, otherwise normal mode

`l{x,y,x2,y2}` - Draw a line from {x},{y} to {x2},{y2}

`r{x,y,w,h,fill}` - Draw a rectangle of {w}*{h} at {x},{y}. Fill flag 0 for filled in, otherwise draw outline of px wide based of {fill} number

`s{x,y,text}` - Draw a string of lower chars of {size} at x,y. Fill flag 0 for filled in, otherwise draw outline of px wide based of {fill} number

## Shared image pool

Onboard is a 4KB memory pool for storing (monochrome) bitmaps. Each context can access the same buffer and draw images using appropriate offsets.

Data should be sent in a long string of 4bit nibbles in Little Endian format, prefaced with a starting address to start uploading.

*Warning* any bytes uploaded past the memory limit will be automatically dropped and ignored.

`0xF0 0x00 0x7F 0x7E {startLSB, startMSB ,countLSB, countMSB, data...}` - {0xF0 0x00 0x7F 0x7E} is the sysex string which should be used to enter upload mode.
{startLSB, startMSB} is the offset within the shared buffer from which data placement should start.
{countLSB, countMSB} is the number of data nibbles which follow
*LSB + MSB bytes from a 14bit number used in internal calculations.*


# Display List Manipulation

`V{addressMSB, addressLSB}` - register memory location defined by addressMSB + addressLSB bits (14bit). the link is auto increased per context in the order of registration, e.g. link Display List @ address 4 -link-> slot 0, link Display List @ address 2 -link-> slot 1 etc ...

`0x90 {slot, value}` - noteOn message can alter memory if memory location was previously linked/bound, channel = context (LSB nibble of 0x9<span style="color:limegreen">0</span>)


# Technical 

## Memory Limits
`20KB` RAM:
* `minimum 4KB ` for system variables, USB stack, io etc...
* `128*4*2 = 1KB` for shared screen buffer
* `1kb ` command buffer per context ` * 5 = 5KB`
* `128b ` variable pool per context ` * 5 = 620b` 
* `128b ` command link per context ` * 5 = 620b` 
* `8KB ` shared image buffer pool 

Estimated minim memory usage (without system):
`1KB + 5KB + 620b + 620b + 8KB =` *15576 bytes*

## Transfer Limits
https://www.lim.di.unimi.it/IEEE/LYON/NETWD.HTM
* Midi baud rate = `31250 baud`.
* Byte rate = `31250 baud / 10bits = 3125 bytes/s`.
* dt per serial byte = `1s / 31250baud = 320us`

## Frametime limits
* `20fps dt = 1 / 20 = 50ms`
* max midi bytes within 1 frame of `20fps = 50ms / 320us ~= 156 bytes`
* `25fps dt = 1 / 25 = 40ms`
* max midi bytes within 1 frame of `25fps = 40ms / 320us ~= 125 bytes`
* `30fps dt = 1 / 30 = 33ms`
* max midi bytes within 1 frame of `30fps = 33ms / 320us ~= 104 bytes`
