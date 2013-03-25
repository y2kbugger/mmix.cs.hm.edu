To remove warnings C4244: conversion from '__w64 int' to 'int', 
possible loss of data
when doing pointer arithmetic
@x
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif
@y
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif
#pragma warning(disable : 4244 )
@z



For a tetra t, -t is still an unsigned number so we replace the following statements

@x
    new_O=incr(cool_O,-x-1);
@y
    new_O=incr(cool_O,-(int)x-1);
@z

max is already defined
@x
@d max(x,y) ((x)<(y)? (y):(x))
@y
@z

@x
  data->y.o.l &= -data->b.o.l;
@y
  data->y.o.l &= -(int)data->b.o.l;
@z

