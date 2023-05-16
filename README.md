# Bytecode-Virtual-Machine

### Currently in-progress

Terminal Input
```
> 2 + 7 * 14 
```

Terminal Output
```
== code ==
0000    1 OP_CONSTANT         0 '2'
0002    | OP_CONSTANT         1 '7'
0004    | OP_CONSTANT         2 '14'
0006    | OP_MULTIPLY
0007    | OP_ADD
0008    2 OP_RETURN

0000    1 OP_CONSTANT         0 '2'
                [ 2 ]
0002    | OP_CONSTANT         1 '7'
                [ 2 ][ 7 ]
0004    | OP_CONSTANT         2 '14'
                [ 2 ][ 7 ][ 14 ]
0006    | OP_MULTIPLY
                [ 2 ][ 98 ]
0007    | OP_ADD
                [ 100 ]
0008    2 OP_RETURN
100
```
