# Commands

Example command:
```
    x{2}G{2}r{0}{0}{100}{200}G{1}s{2}{4}hello
```
Decimal Midi Stream:
```
    implied sysex start flag {0xF0}
        120,2, 
        71,2, 
        114,0,0,100,200, 
        71,1, 
        115,2,104,101,108,108,111
    implied sysex end flag {0xF7}
```
Prettyfied:
```
    x 2
    G 2
    r 0 0 100 200 1
    G 1
    s 2 4 "hello"
```

Breakdown of example
`g` is graphics command flag,argument list is assumed and not checked for count of arguments so take heed.
`2` is the first argument in this case, `{}` is argument separator only for visual purposes to denote integer value separation in this example.

### Graphics

```G{display}``` - Set Graphics context for display {display}  

```S{factor}``` - Set Graphics drawing scale for bitmaps and text to {factor} integer value

```I{mode}``` - Set Graphics drawing mode, if non zero use XOR drawing mode, otherwise normal mode

```c``` - Clear draw buffer

```u``` - Send draw buffer to current context

```l{x}{y}{x2}{y2}``` - Draw a line from {x},{y} to {x2},{y2}

```r{x}{y}{w}{h}{fill}``` - Draw a rectangle of {w}*{h} at {x},{y}. Fill flag 0 for filled in, otherwise draw outline of px wide based of {fill} number

```s{x}{y}{text}``` - Draw a string of lower chars of {size} at x,y. Fill flag 0 for filled in, otherwise draw outline of px wide based of {fill} number

