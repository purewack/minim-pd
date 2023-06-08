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

```s{factor}``` - Set Graphics drawing scale for bitmaps and text to {factor} integer value

```x{mode}``` - Set Graphics drawing mode, if non zero use XOR drawing mode, otherwise normal mode

```l{x,y,x2,y2}``` - Draw a line from {x},{y} to {x2},{y2}

```r{x,y,w,h,fill}``` - Draw a rectangle of {w}*{h} at {x},{y}. Fill flag 0 for filled in, otherwise draw outline of px wide based of {fill} number

```s{x,y,text}``` - Draw a string of lower chars of {size} at x,y. Fill flag 0 for filled in, otherwise draw outline of px wide based of {fill} number

## Display List Manipulation

```G{ context, start, count, bytes... }``` - Set Graphics context for display list {context} and upload display list commands 

```A {mem slot, value}``` - Alter memory slot of graphicsList, useful for animation

```a {mem_msb, mem_lsb, value}``` - Alter memory location of graphicsList directly

# Technical 

## Memory Limits
`20kb` RAM:
* `minimum 4kb ` for system variables, USB stack, io etc...
* `128*4*2 = 1kb` for shared screen buffer
* `1kb ` command buffer per context ` * 5 = 5kb`
* `128b ` variable pool per context ` * 5 = 620b` 
* `8kb ` shared image buffer pool 

Estimated minim memory usage (without system):
`1kb + 5kb + 620b + 8kb =` *14956 bytes*

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
