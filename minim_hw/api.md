# Commands

Example command:
```
    g[2/r[0;0;100;200;1/g[1/c[0;0;20;0
```
Prettyfied:
```
    g[2/
    r[0;0;100;200;1/
    g[1/
    c[0;0;20;0
```

Breakdown of example
`g` is graphics command flag, `[` denotes begining of command argument list
`2` is the first argument in this case, `;` is argument separator, `r[0;0;100;200;1` is another argument which is also another command. `/` is a command separator

### Graphics

```g[{display}/```

Set Graphics context for display {display}

```l[{x};{y};{x2};{y2}/```

Draw a line from {x},{y} to {x2},{y2}

```r[{x};{y};{w};{h};{fill}/```

Draw a rectangle of {w}*{h} at {x},{y}. Fill flag 0 for filled in, otherwise draw outline of px wide based of {fill} number

```t[{x};{y};{size};{text}/```

Draw a string of lower chars of {size} at x,y. Fill flag 0 for filled in, otherwise draw outline of px wide based of {fill} number

